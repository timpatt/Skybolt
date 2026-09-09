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
		.property("significantWaveHeight", &OceanComponent::significantWaveHeight)
		.property("windVelocityHeading", &OceanComponent::windVelocityHeading, {{PropertyMetadataNames::units, Units::Radians}});
}

void OceanComponent::update()
{
	if (generate(mSimTime))
	{
		++mGenerationCount;
	}
}

} // namespace sim
} // namespace skybolt