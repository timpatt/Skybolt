/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVisOsg/SkyboltVisOsgFwd.h"
#include "SkyboltVisOsg/Renderable/Water/WaterStateSet.h"

#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Chrono.h>

#include <osg/Group>

namespace skybolt {

namespace vis {


struct WaterMaterialConfig
{
	OceanHeightfieldTextureGeneratorPtr oceanHeightfieldTextureGenerator;
	const ShaderPrograms* programs;
	const NonNullPtr<Scene> scene;
};

class WaterMaterial : public osg::Group
{
public:
	WaterMaterial(const WaterMaterialConfig& config);

	void setWakes(const std::vector<Wake>& wakes);

	void update(double timeSeconds);

	osg::ref_ptr<osg::Texture2D> getHeightTexture(int cascadeIndex) const;
	osg::ref_ptr<osg::Texture2D> getNormalTexture(int cascadeIndex) const;
	osg::ref_ptr<osg::Texture2D> getFoamMaskTexture(int cascadeIndex) const;
	int getCascadeCount() const;

	sim::OceanSurfaceSamplerPtr getSurfaceSampler() const; //!< Never returns null

private:
	NonNullPtr<Scene> mScene;
	OceanHeightfieldTextureGeneratorPtr mOceanHeightfieldTextureGenerator;
	std::vector<osg::ref_ptr<GpuTextureGenerator>> mWaterSurfaceGpuTextureGenerators; //!< stored in execution order
	std::vector<osg::ref_ptr<class WaveFoamMaskGenerator>> mWaveFoamMaskGenerators;
	std::size_t mLastGenerationCount = 0;
};

} // namespace vis
} // namespace skybolt
