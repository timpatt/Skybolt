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

	sim::OceanSurfaceSampler& getSurfaceSampler() const
	{
		static auto sampler = std::make_shared<sim::PlanarOceanSurfaceSampler>();
		return *sampler;
	}

	bool generate(double time) override
	{
		// Stub functionality to "generate" if not already generated
		if (getGenerationCount())
		{
			// No need to re-generate
			return false;
		}

		return true;
	}

	int getTextureCount() const override { return 1; }

	osg::ref_ptr<osg::Texture2D> getTexture(int index) const override
	{
		return mTexture;
	}

	float getTextureWorldSize(int index) const override { return 100; } // Size doesn't matter as the ocean surface is flat

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

	sim::Vector3 mTextureOriginGeocentric = {};
};

} // namespace vis
} // namespace skybolt
