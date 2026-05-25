/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SunComponent.h"

#include <SkyboltCommon/Units.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include "SkyboltSim/PropertyMetadata.h"

namespace skybolt::sim {

SKYBOLT_REFLECT(AzimuthElevationLatLon) {
	registry.type<AzimuthElevationLatLon>("AzimuthElevationLatLon")
		.property("azimuth", &AzimuthElevationLatLon::azimuth, {{PropertyMetadataNames::units, Units::Radians}})
		.property("elevation", &AzimuthElevationLatLon::elevation, {{PropertyMetadataNames::units, Units::Radians}})
		.property("observer", &AzimuthElevationLatLon::observer);
}

SKYBOLT_REFLECT(SunComponent) {
	registry.type<SunComponent>("SunComponent")
		.superType<Component>()
		.property("directionOverride", &SunComponent::directionOverride);
}

sim::LatLon SunComponent::calcEclipticDirection(double julianDate) const
{
	if (!directionOverride)
	{
		return calcSunEclipticPosition(julianDate);
	}

	const AzimuthElevationLatLon& override = *directionOverride;
	AzimuthElevation azEl(override.azimuth, override.elevation);

	sim::LatLon equatorial = convertHorizontalToEquatorial(julianDate, azEl, override.observer);
	return convertEquatorialToEcliptic(julianDate, equatorial);
}

} // namespace skybolt::sim
