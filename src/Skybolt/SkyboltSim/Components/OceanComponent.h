/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Chrono.h"
#include "SkyboltSim/Component.h"
#include "SkyboltSim/OceanSurfaceSampler.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt::sim {

class OceanHeightfieldGenerator
{
public:
	virtual ~OceanHeightfieldGenerator() {}

	//! @return true if the texture was generated or re-generated, or false if the previously generated texture should be used.
	virtual bool generate(sim::SecondsD time) = 0;

	//! @returns the total number of times he ocean surface has been generated
	virtual std::size_t getGenerationCount() const = 0;

	virtual float getSignificantWaveHeight() const = 0;
	virtual void setSignificantWaveHeight(float height) = 0;

	virtual float getWindVelocityHeading() const = 0;
	virtual void setWindVelocityHeading(float heading) = 0;

	virtual int getSeed() const = 0;
	virtual void setSeed(int seed) = 0;

	//! @returns an interface for sampling the ocean surface eometry. Result is never null.
	virtual skybolt::sim::OceanSurfaceSamplerPtr getSurfaceSampler() const
	{
		static auto sampler = std::make_shared<sim::PlanarOceanSurfaceSampler>();
		return sampler;
	}
};

class OceanComponent : public Component
{
public:
	//! If heightfieldGenerator is null, a planar ocean surface is used
	explicit OceanComponent(std::shared_ptr<OceanHeightfieldGenerator> heightfieldGenerator = nullptr);
	~OceanComponent() override = default;

	OceanSurfaceSampler& getOceanSurfaceSampler() const;

	int seed = 0;

	//! Significant wave height is the average trough-to-crest height of the highest 1/3 of waves.
	//! Also defined as 4 times the standard deviation of the surface elevation.
	//! See https://en.wikipedia.org/wiki/Significant_wave_height
	double significantWaveHeight = 1;

	double windVelocityHeading = 0;

	const std::shared_ptr<OceanHeightfieldGenerator> heightfieldGenerator; //!< May be null

protected:
	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::BeginStateUpdate, update)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void update();

	void setSimTime(SecondsD simTime) override { mSimTime = simTime; }

private:
	SecondsD mSimTime = 0;
};

SKYBOLT_REFLECT_EXTERN(OceanComponent)

} // namespace skybolt::sim