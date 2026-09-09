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

class OceanComponent : public Component
{
public:
	~OceanComponent() override = default;

	virtual const OceanSurfaceSampler& getSurfaceSampler() const = 0;

	//! @return true if the texture was generated or re-generated, or false if the previously generated texture should be used.
	virtual bool generate(sim::SecondsD time) = 0;

	//! @returns the total number of times he ocean surface has been generated
	virtual std::size_t getGenerationCount() const { return mGenerationCount; }

	int seed = 0;

	//! Significant wave height is the average trough-to-crest height of the highest 1/3 of waves.
	//! Also defined as 4 times the standard deviation of the surface elevation.
	//! See https://en.wikipedia.org/wiki/Significant_wave_height
	double significantWaveHeight = 1;

	double windVelocityHeading = 0;

protected:
	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::BeginStateUpdate, update)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void update();

	void setSimTime(SecondsD simTime) override { mSimTime = simTime; }

private:
	SecondsD mSimTime = 0;
	std::size_t mGenerationCount = 0;
};

class PlanarOceanComponent : public OceanComponent
{
	OceanSurfaceSampler& getSurfaceSampler() const override
	{
		static auto sampler = std::make_shared<sim::PlanarOceanSurfaceSampler>();
		return *sampler;
	}

	bool generate(sim::SecondsD time) override
	{
		return false;
	}

public: // Component interface
	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(PlanarOceanComponent), typeid(OceanComponent)};
	}
};

SKYBOLT_REFLECT_EXTERN(OceanComponent)

} // namespace skybolt::sim