/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltSim/Spatial/LatLonAlt.h>
#include <glm/glm.hpp>
#include <optional>

namespace skybolt {
namespace vis {

class LlaToNedConverter
{
public:
	LlaToNedConverter(const sim::LatLon& origin, const std::optional<double>& planetRadiusForSurfaceDrop) :
		mOrigin(origin),
		mPlanetRadiusForSurfaceDrop(planetRadiusForSurfaceDrop)
	{
	}

	//! +x is north, +y is east
	glm::dvec2 latLonToCartesianNe(const sim::LatLon& position) const;

	//! +x is north, +y is east, +z is down
	glm::dvec3 latLonAltToCartesianNed(const sim::LatLonAlt& position) const;

	sim::LatLon cartesianNeToLatLon(const glm::dvec2& position) const;

	void setOrigin(const sim::LatLon& origin)
	{
		mOrigin = origin;
	}

	float calcPlanetSurfaceDrop(float distance) const;

private:
	sim::LatLon mOrigin;
	std::optional<double> mPlanetRadiusForSurfaceDrop;
};

} // namespace vis
} // namespace skybolt
