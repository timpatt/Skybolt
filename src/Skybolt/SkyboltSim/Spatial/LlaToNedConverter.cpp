/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "LlaToNedConverter.h"
#include "GreatCircle.h"

namespace skybolt {
namespace vis {

glm::dvec2 LlaToNedConverter::latLonToCartesianNe(const sim::LatLon& position) const
{
	return sim::latLonToCartesianNe(mOrigin, position);
}

glm::dvec3 LlaToNedConverter::latLonAltToCartesianNed(const sim::LatLonAlt& position) const
{
	glm::dvec2 ne = sim::latLonToCartesianNe(mOrigin, toLatLon(position));
	double d = -position.alt;

	if (mPlanetRadiusForSurfaceDrop)
	{
		d += calcPlanetSurfaceDrop(glm::length(ne));
	}

	return glm::dvec3(ne.x, ne.y, d);
}

sim::LatLon LlaToNedConverter::cartesianNeToLatLon(const glm::dvec2& position) const
{
	return sim::cartesianNeToLatLon(mOrigin, position);
}

float LlaToNedConverter::calcPlanetSurfaceDrop(float distance) const
{
	return distance * distance / (2 * *mPlanetRadiusForSurfaceDrop);
}

} // namespace vis
} // namespace skybolt
