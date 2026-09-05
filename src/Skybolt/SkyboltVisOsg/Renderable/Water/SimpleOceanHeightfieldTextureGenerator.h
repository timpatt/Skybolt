/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "OceanHeightfieldTextureGenerator.h"

namespace skybolt {
namespace vis {

class SimpleOceanHeightfieldTextureGenerator : public OceanHeightfieldTextureGenerator
{
public:
	SimpleOceanHeightfieldTextureGenerator();

	bool generate(double time) override
	{
		// Stub functionality to "generate" if not already generated
		if (mGenerationCount)
		{
			// No need to re-generate
			return false;
		}
		mGenerationCount = 1;
		return true;
	}

	std::size_t getGenerationCount() const override
	{
		return mGenerationCount;
	}

	float getWaveHeight() const override { return 0.0f; }
	void setWaveHeight(float height) override {};

	float getWindVelocityHeading() const override { return 0.0f; }
	void setWindVelocityHeading(float heading) override {};

	int getTextureCount() const override { return 1; }

	osg::ref_ptr<osg::Texture2D> getTexture(int index) const override
	{
		return mTexture;
	}

	float getTextureWorldSize(int index) const override { return 100; } // Size doesn't matter as the ocean surface is flat

	int getSeed() const override { return 0; }
	void setSeed(int seed) override {};

	const sim::Vector3& getTextureOriginGeocentric() const override { return mTextureOriginGeocentric; }
	const glm::dvec2& getTextureOriginNorthEast() const override
	{
		static const glm::dvec2 origin = {};
		return origin;
	}

	//! Updates the scene origin in geocentric coordinates. This is used to offset the texture coordinates so that the texture appears fixed relative to the planet, even though the scene origin may move.
	void updateSceneOriginGeocentric(const sim::Vector3& sceneOriginGeocentric) override { mTextureOriginGeocentric = sceneOriginGeocentric; }

private:
	osg::ref_ptr<osg::Texture2D> mTexture;
	std::size_t mGenerationCount;

	sim::Vector3 mTextureOriginGeocentric = {};
};

} // namespace vis
} // namespace skybolt
