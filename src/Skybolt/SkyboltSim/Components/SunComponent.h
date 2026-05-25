/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/Spatial/AzimuthElevation.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <optional>

namespace skybolt::sim {

struct AzimuthElevationLatLon
{
	double azimuth; //!< in radians, 0 is north, positive is east
	double elevation; //!< in radians, 0 is on the horizon, positive is up
	sim::LatLon observer;
};

SKYBOLT_REFLECT_EXTERN(AzimuthElevationLatLon)

struct SunComponent : public Component
{
	//! When set, overrides the astronomically calculated azimuth and elevation of the sun
	std::optional<AzimuthElevationLatLon> directionOverride;

	//! @returns the sun direction in ecliptic coordinates based on given julianDate.
	//! If directionOverride is set, the returned value is based on the override instead of the astronomical position of the sun.
	sim::LatLon calcEclipticDirection(double julianDate) const;
};

SKYBOLT_REFLECT_EXTERN(SunComponent)

} // namespace skybolt::sim
