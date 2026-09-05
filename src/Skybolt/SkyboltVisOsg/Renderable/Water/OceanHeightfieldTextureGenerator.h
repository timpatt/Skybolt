/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/OceanSurfaceSampler.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/Components/OceanComponent.h>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

class OceanHeightfieldTextureGenerator : public sim::OceanHeightfieldGenerator
{
public:
	virtual ~OceanHeightfieldTextureGenerator() = default;

	virtual int getTextureCount() const = 0; //! Texture count must be constant for the lifetime of the object
	virtual osg::ref_ptr<osg::Texture2D> getTexture(int index) const = 0;
	virtual float getTextureWorldSize(int index) const = 0;

	//! @returns the geocentric origin of the texture.
	//! This may shift over time as the scene origin moves, so that the texture appears fixed relative to the planet.
	virtual const sim::Vector3& getTextureOriginGeocentric() const = 0;

	//! @returns the origin of the texture in north-east coordinates relative to the scene origin.
	//! This may shift over time as the scene origin moves, so that the texture appears fixed relative to the planet.
	virtual const glm::dvec2& getTextureOriginNorthEast() const = 0;

	virtual void updateSceneOriginGeocentric(const sim::Vector3& sceneOriginGeocentric) = 0;
};

} // namespace vis

} // namespace skybolt
