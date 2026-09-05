/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OceanComponent.h"
#include "SkyboltSim/PropertyMetadata.h"
#include <SkyboltCommon/Units.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT(OceanComponent) {
	registry.type<OceanComponent>("OceanComponent")
		.superType<Component>()
		.property("seed", &OceanComponent::seed)
		.property("waveHeight", &OceanComponent::waveHeight)
		.property("windVelocityHeading", &OceanComponent::windVelocityHeading, {{PropertyMetadataNames::units, Units::Radians}});
}

OceanComponent::OceanComponent(std::shared_ptr<OceanHeightfieldGenerator> heightfieldGeneratorArg) :
	heightfieldGenerator(std::move(heightfieldGeneratorArg))
{
	assert(heightfieldGenerator);
}

void OceanComponent::update()
{
	if (!heightfieldGenerator) { return; }
	heightfieldGenerator->setWaveHeight(waveHeight);
	heightfieldGenerator->setWindVelocityHeading(windVelocityHeading);
	heightfieldGenerator->setSeed(seed);

	heightfieldGenerator->generate(mSimTime);
}

OceanSurfaceSampler& OceanComponent::getOceanSurfaceSampler() const
{
	if (!heightfieldGenerator)
	{
		static auto sampler = std::make_shared<sim::PlanarOceanSurfaceSampler>();
		return *sampler;
	}

	return *heightfieldGenerator->getSurfaceSampler();
}

} // namespace sim
} // namespace skybolt