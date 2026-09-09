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
#include <functional>
#include <mutex>
#include <thread>

namespace skybolt {
namespace vis {

struct FftOceanHeightfieldTextureGeneratorConfig
{
	int textureSizePixels;
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
	FftOceanHeightfieldTextureGenerator(const FftOceanHeightfieldTextureGeneratorConfig& config)
	{
		mGeneratorResult = std::vector<glm::vec3>(config.textureSizePixels * config.textureSizePixels, glm::vec3(0,0,0));

		mGenerator.reset(new FftOceanGenerator([&] {
			FftOceanGeneratorConfig c;
			c.gravity = mGravity;
			c.seed = seed;
			c.textureSizePixels = config.textureSizePixels;
			c.textureWorldSize = textureWorldSize;
			c.significantWaveHeight = significantWaveHeight;
			c.windVelocityHeading = windVelocityHeading;
			c.waveSpectrumWindow = waveSpectrumWindow;
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
	}

	~FftOceanHeightfieldTextureGenerator()
	{
		mTerminateGeneratorThread = true;
		mGeneratorRequestCV.notify_one();
		mGeneratorThread.join();
	}

	const sim::OceanSurfaceSampler& getSurfaceSampler() const
	{
		static auto sampler = std::make_shared<sim::PlanarOceanSurfaceSampler>();
		return *sampler;
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
			}
			image->dirty();
			textureUpdated = true;
		}

		// Check if input changed
		GenerationRequest request = createGenerationRequest(time);
		bool changed = (!mPrevGenerationRequest || (*mPrevGenerationRequest != request));

		// If input state has changed, request a new texture to be generated
		if (changed)
		{
			mPrevGenerationRequest = request;
			{
				// Request a new image to be generated
				std::lock_guard<std::mutex> lock(mGeneratorRequestMutex);
				mGenerationRequest = request;
			}
			mGeneratorRequestCV.notify_one(); // Safe to call notify_one outside the mGeneratorRequestMutex lock
		}
		return textureUpdated;
	}

	void runGeneratorThread()
	{
		while (!mTerminateGeneratorThread)
		{
			// Wait for generation request
			{
				std::unique_lock<std::mutex> lock(mGeneratorRequestMutex); // Must lock before calling wait()
				mGeneratorRequestCV.wait(lock, [this] {return mGenerationRequest || mTerminateGeneratorThread; });
			}

			if (!mGenerationRequest || mTerminateGeneratorThread)
			{
				return;
			}

			{
				std::lock_guard<std::mutex> lock(mGeneratorResultMutex);

				mGenerator->setTextureWorldSize(mGenerationRequest->textureWorldSize);
				mGenerator->setSignificantWaveHeight(mGenerationRequest->significantWaveHeight);
				mGenerator->setWindVelocityHeading(mGenerationRequest->windVelocityHeading);
				mGenerator->setSeed(mGenerationRequest->seed);
				mGenerator->setWaveSpectrumWindow(mGenerationRequest->waveSpectrumWindow);
				mGenerator->calculate(mGenerationRequest->requestTime, std::span<glm::vec3>{mGeneratorResult.data(), mGeneratorResult.size()});
				mGeneratorHasResult = true;
				mGenerationRequest.reset();
			}
		}
	}

	int getTextureCount() const override { return 1; }

	osg::ref_ptr<osg::Texture2D> getTexture(int index) const
	{
		assert(index == 0);
		return mTexture;
	}

	float getTextureWorldSize(int index) const override { return textureWorldSize; }

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

		const double teleportDistance = 100 * textureWorldSize;
		if (originDistance > teleportDistance)
		{
			// Teleport the texture origin if the scene origin has moved a long way the texture origin.
			mTextureOriginGeocentric = sceneOriginGeocentric;
			mTextureOriginNorthEast = {};
			return;
		}
		else if (originDistance < textureWorldSize)
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
		displacementNed.x = std::fmod(displacementNed.x, (double)textureWorldSize);
		displacementNed.y = std::fmod(displacementNed.y, (double)textureWorldSize);
		displacementNed.z = 0; // Don't offset the texture in the vertical direction

		mTextureOriginGeocentric = sceneOriginGeocentric + ltpOrientation * displacementNed;
		mTextureOriginNorthEast = displacementNed;
	}

	struct GenerationRequest
	{
		sim::SecondsD requestTime;
		float textureWorldSize;
		float significantWaveHeight;
		float windVelocityHeading;
		WaveSpectrumWindow waveSpectrumWindow;
		int seed;

		bool operator==(const GenerationRequest& other) const = default;
	};

	GenerationRequest createGenerationRequest(sim::SecondsD requestTime) const
	{
		return GenerationRequest{
			.requestTime = requestTime,
			.textureWorldSize = textureWorldSize,
			.significantWaveHeight = (float)significantWaveHeight,
			.windVelocityHeading = (float)windVelocityHeading,
			.waveSpectrumWindow = waveSpectrumWindow,
			.seed = seed
		};
	}

	float textureWorldSize = 500;
	WaveSpectrumWindow waveSpectrumWindow;

public: // Component interface
	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(FftOceanHeightfieldTextureGenerator), typeid(OceanComponent)};
	}

private:
	const float mGravity = 9.8f;
	std::unique_ptr<FftOceanGenerator> mGenerator;
	osg::ref_ptr<osg::Texture2D> mTexture;

	sim::Vector3 mTextureOriginGeocentric = {};
	glm::dvec2 mTextureOriginNorthEast = {};

	std::optional<GenerationRequest> mPrevGenerationRequest;

	std::thread mGeneratorThread;
	std::atomic_bool mTerminateGeneratorThread = false;

	std::vector<glm::vec3> mGeneratorResult;
	std::mutex mGeneratorResultMutex;
	std::atomic_bool mGeneratorHasResult = false;

	std::mutex mGeneratorRequestMutex;
	std::condition_variable mGeneratorRequestCV;
	std::optional<GenerationRequest> mGenerationRequest;
};

SKYBOLT_REFLECT(FftOceanHeightfieldTextureGenerator)
{
	registry.type<FftOceanHeightfieldTextureGenerator>("FftOceanHeightfieldTextureGenerator")
		.superType<sim::OceanComponent>()
		.property("textureWorldSize", &FftOceanHeightfieldTextureGenerator::textureWorldSize);
}

} // namespace vis
} // namespace skybolt
