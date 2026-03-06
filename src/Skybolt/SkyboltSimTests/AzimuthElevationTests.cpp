/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TestHelpers.h"
#include <catch2/catch.hpp>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltSim/Spatial/AzimuthElevation.h>

using namespace skybolt;
using namespace skybolt::sim;

TEST_CASE("AzimuthElevation to direction and back")
{
	AzimuthElevation azEl(0.1, 0.2);
	Vector3 dir = azimuthAndElevationToDirection(azEl);
	AzimuthElevation azEl2 = directionToAzimuthElevation(dir);
	
	CHECK(azEl.x == Approx(azEl.x));
	CHECK(azEl.y == Approx(azEl.y));
}

TEST_CASE("Zero azimuth and elevation is due north")
{
	Vector3 dir = azimuthAndElevationToDirection(AzimuthElevation(0, 0));
	
	CHECK(dir[0] == Approx(1));
	CHECK(dir[1] == Approx(0));
	CHECK(dir[2] == Approx(0));
}

TEST_CASE("Half pi azimuth and zero elevation is due east")
{
	Vector3 dir = azimuthAndElevationToDirection(AzimuthElevation(math::halfPiD(), 0));
	
	CHECK(dir[0] == Approx(0).margin(1e-8));
	CHECK(dir[1] == Approx(1));
	CHECK(dir[2] == Approx(0));
}

TEST_CASE("Half pi elevation is up")
{
	Vector3 dir = azimuthAndElevationToDirection(AzimuthElevation(0, math::halfPiD()));
	
	CHECK(dir[0] == Approx(0).margin(1e-8));
	CHECK(dir[1] == Approx(0));
	CHECK(dir[2] == Approx(-1));
}

TEST_CASE("Negative half pi elevation is down")
{
	Vector3 dir = azimuthAndElevationToDirection(AzimuthElevation(0, -math::halfPiD()));
	
	CHECK(dir[0] == Approx(0).margin(1e-8));
	CHECK(dir[1] == Approx(0));
	CHECK(dir[2] == Approx(1));
}