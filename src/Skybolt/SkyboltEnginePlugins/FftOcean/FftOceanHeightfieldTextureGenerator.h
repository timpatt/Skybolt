/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltVisOsg/Renderable/Water/OceanHeightfieldTextureGenerator.h>
#include "WaveSpectrumWindow.h"
#include "FftOceanGenerator.h"

#include <SkyboltSim/Spatial/Geocentric.h>

#include <osg/Image>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace skybolt {
namespace vis {

struct FftOceanHeightfieldTextureGeneratorConfig
{
	float textureWorldSize;
	int textureSizePixels;
	WaveSpectrumWindow waveSpectrumWindow;
	int seed = 0;
};

static glm::dvec2 calcTextureOriginNorthEast(const sim::Vector3& sceneOriginGeocentric, const sim::Vector3& textureOriginGeocentric)
{
	// Calculate north-east displacement of the texture origin relative to the scene origin.
	sim::Vector3 displacementGeocentric = textureOriginGeocentric - sceneOriginGeocentric;

	sim::Matrix3 ltpOrientation = sim::geocentricToLtpOrientation(sceneOriginGeocentric);
	sim::Vector3 displacementNed = glm::inverse(ltpOrientation) * displacementGeocentric;

	return glm::dvec2(displacementNed.x, displacementNed.y);
}

class FftOceanHeightfieldTextureGenerator : public OceanHeightfieldTextureGenerator
{
public:
	FftOceanHeightfieldTextureGenerator(const FftOceanHeightfieldTextureGeneratorConfig& config) :
		mWorldSize(config.textureWorldSize),
		mWaveHeight(0.5),
		mSeed(config.seed)
	{
		mWindVelocity = calcWindVelocity();

		mGeneratorResult = std::vector<glm::vec3>(config.textureSizePixels * config.textureSizePixels, glm::vec3(0,0,0));

		mGenerator.reset(new FftOceanGenerator([&] {
			FftOceanGeneratorConfig c;
			c.gravity = mGravity;
			c.seed = config.seed;
			c.textureSizePixels = config.textureSizePixels;
			c.textureWorldSize = config.textureWorldSize;
			c.windVelocity = mWindVelocity;
			c.waveSpectrumWindow = config.waveSpectrumWindow;
			return c;
			}()));

		osg::Image* image = new osg::Image();
		image->setDataVariance(osg::Image::DYNAMIC);
		image->allocateImage(config.textureSizePixels, config.textureSizePixels, 1, GL_RGB, GL_FLOAT);
		image->setInternalTextureFormat(GL_RGB32F_ARB);

		mTexture = new osg::Texture2D(image);
		mTexture->setFilter(osg::Texture2D::FilterParameter::MIN_FILTER, osg::Texture2D::FilterMode::LINEAR_MIPMAP_LINEAR);
		mTexture->setFilter(osg::Texture2D::FilterParameter::MAG_FILTER, osg::Texture2D::FilterMode::LINEAR);

		mGeneratorThread = std::thread([this] {runGeneratorThread(); });
		{
			std::lock_guard<std::mutex> lock(mGeneratorRequestMutex);
			mGeneratorRequest = true;
		}
		mGeneratorRequestCV.notify_one();

		generate(0);
	}

	~FftOceanHeightfieldTextureGenerator()
	{
		mTerminateGeneratorThread = true;
		mGeneratorRequestCV.notify_one();
		mGeneratorThread.join();
	}

	bool generate(sim::SecondsD time) override
	{
		bool textureUpdated = false;
		// If there's a result from the previous request, copy it to the texture
		if (mGeneratorHasResult)
		{
			osg::Image* image = mTexture->getImage();
			{
				// Copy image from generator thread to texture
				std::lock_guard<std::mutex> lock(mGeneratorResultMutex);
				memcpy(image->data(), mGeneratorResult.data(), mGeneratorResult.size() * sizeof(float) * 3);
				mGeneratorHasResult = false;
				++mGenerationCount;
			}
			image->dirty();
			textureUpdated = true;
		}

		// If input state has changed, request a new texture to be generated
		bool timeChanged = !mRequestTime || (time != *mRequestTime);
		if (timeChanged || mWindVelocityChanged || mSeedChanged)
		{
			mRequestTime = time;
			{
				// Request a new image to be generated
				std::lock_guard<std::mutex> lock(mGeneratorRequestMutex);
				mGeneratorRequest = true;
			}
			mGeneratorRequestCV.notify_one(); // Safe to call notify_one outside the mGeneratorRequestMutex lock
		}
		return textureUpdated;
	}

	std::size_t getGenerationCount() const override
	{
		return mGenerationCount;
	}

	void runGeneratorThread()
	{
		while (!mTerminateGeneratorThread)
		{
			// Wait for generation request
			{
				std::unique_lock<std::mutex> lock(mGeneratorRequestMutex); // Must lock before calling wait()
				mGeneratorRequestCV.wait(lock, [this] {return mGeneratorRequest || mTerminateGeneratorThread; });
			}

			if (mTerminateGeneratorThread)
			{
				return;
			}

			{
				std::lock_guard<std::mutex> lock(mGeneratorResultMutex);

				if (mWindVelocityChanged)
				{
					mGenerator->setWindVelocity(mWindVelocity);
					mWindVelocityChanged = false;
				}

				if (mSeedChanged)
				{
					mGenerator->setSeed(mSeed);
					mSeedChanged = false;
				}

				mGenerator->calculate(mRequestTime.value_or(0), std::span<glm::vec3>{mGeneratorResult.data(), mGeneratorResult.size()});
				mGeneratorHasResult = true;
				mGeneratorRequest = false;
			}
		}
	}

	float getWorldSize() const
	{
		return mWorldSize;
	}

	float getWaveHeight() const override { return mWaveHeight; }

	void setWaveHeight(float height) override
	{
		if (mWaveHeight != height)
		{
			mWaveHeight = height;
			mWindVelocity = calcWindVelocity();
			mWindVelocityChanged = true;
		}
	}

	float getWindVelocityHeading() const override { return mWindVelocityHeading; }

	void setWindVelocityHeading(float heading) override
	{
		if (mWindVelocityHeading != heading)
		{
			mWindVelocityHeading = heading;
			mWindVelocity = calcWindVelocity();
			mWindVelocityChanged = true;
		}
	}

	int getTextureCount() const override { return 1; }

	osg::ref_ptr<osg::Texture2D> getTexture(int index) const
	{
		assert(index == 0);
		return mTexture;
	}

	float getTextureWorldSize(int index) const override { return mWorldSize; }

	int getSeed() const override
	{
		return mSeed;
	}

	void setSeed(int seed) override
	{
		if (mSeed != seed)
		{
			mSeed = seed;
			mSeedChanged = true;
		}
	}

	const sim::Vector3& getTextureOriginGeocentric() const override
	{
		return mTextureOriginGeocentric;
	}

	const glm::dvec2& getTextureOriginNorthEast() const override
	{
		return mTextureOriginNorthEast;
	}

	void updateSceneOriginGeocentric(const sim::Vector3& sceneOriginGeocentric)
	{
		double originDistance = glm::distance(mTextureOriginGeocentric, sceneOriginGeocentric);

		const double teleportDistance = 100 * mWorldSize;
		if (originDistance > teleportDistance)
		{
			// Teleport the texture origin if the scene origin has moved a long way the texture origin.
			mTextureOriginGeocentric = sceneOriginGeocentric;
			mTextureOriginNorthEast = {};
			return;
		}
		else if (originDistance < mWorldSize)
		{
			// No need to update the geocentric texture origin if the scene origin is already close to the texture origin
			mTextureOriginNorthEast = calcTextureOriginNorthEast(sceneOriginGeocentric, mTextureOriginGeocentric);
			return;
		}

		// Move the texture origin in increments of mWorldSize to be close to scene origin.
		sim::Vector3 displacementGeocentric = mTextureOriginGeocentric - sceneOriginGeocentric;

		sim::Matrix3 ltpOrientation = sim::geocentricToLtpOrientation(sceneOriginGeocentric);
		sim::Vector3 displacementNed = glm::inverse(ltpOrientation) * displacementGeocentric;

		// Find the remainder of the displacement relative to the texture world size.
		// Apply remainder as an offset to the texture coordinates.
		displacementNed.x = std::fmod(displacementNed.x, (double)mWorldSize);
		displacementNed.y = std::fmod(displacementNed.y, (double)mWorldSize);
		displacementNed.z = 0; // Don't offset the texture in the vertical direction

		mTextureOriginGeocentric = sceneOriginGeocentric + ltpOrientation * displacementNed;
		mTextureOriginNorthEast = displacementNed;
	}

private:
	glm::vec2 calcWindVelocity() const {
		float windSpeed = FftOceanGenerator::calcWindSpeedFromMaxWaveHeight(mWaveHeight, mGravity);
		return math::vec2Rotate(glm::vec2(windSpeed, 0), mWindVelocityHeading);
	}

private:
	const float mGravity = 9.8f;
	std::unique_ptr<FftOceanGenerator> mGenerator;
	osg::ref_ptr<osg::Texture2D> mTexture;
	float mWorldSize;
	float mWaveHeight;
	float mWindVelocityHeading = 0;
	int mSeed = 0;

	sim::Vector3 mTextureOriginGeocentric = {};
	glm::dvec2 mTextureOriginNorthEast = {};

	std::thread mGeneratorThread;
	std::atomic_bool mTerminateGeneratorThread = false;

	std::vector<glm::vec3> mGeneratorResult;
	std::mutex mGeneratorResultMutex;
	std::atomic_bool mGeneratorHasResult = false;
	std::size_t mGenerationCount;

	std::mutex mGeneratorRequestMutex;
	std::condition_variable mGeneratorRequestCV;
	std::optional<sim::SecondsD> mRequestTime;
	bool mGeneratorRequest = false;

	std::atomic<glm::vec2> mWindVelocity;
	std::atomic_bool mWindVelocityChanged = false;
	std::atomic_bool mSeedChanged = false;
};

} // namespace vis
} // namespace skybolt
