/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltSim/Components/MainRotorComponent.h>
#include <SkyboltSim/SimMath.h>
#include <catch2/catch.hpp>

#include <cmath>

using namespace skybolt;
using namespace skybolt::sim;

// Aliases for test readability
static const Vector3 travelX(1, 0, 0);
static const Vector3 travelY(0, 1, 0);
static const Vector3 travelNegX(-1, 0, 0);

TEST_CASE("calculateBladeElementAirflow returns nullopt when blade has no forward airflow")
{
	SECTION("Zero rotational speed and zero helicopter velocity")
	{
		// Blade is stationary relative to air - no forward airflow through blade element
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(0, 0, 0), /*meanBladeSpeedRelHeli=*/0.0, travelX, /*bladePitch=*/0.1);
		CHECK(!result.has_value());
	}

	SECTION("Retreating blade speed exceeds rotational speed")
	{
		// Helicopter moving in +X at 200 m/s; blade rotating in -X at 100 m/s.
		// Net blade speed relative to wind is negative - retreating blade stall condition.
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(200, 0, 0), /*meanBladeSpeedRelHeli=*/100.0, travelNegX, /*bladePitch=*/0.1);
		CHECK(!result.has_value());
	}

	SECTION("Retreating blade speed exactly cancels rotational speed (zero boundary)")
	{
		// Helicopter speed exactly equals blade rotational speed on retreating side.
		// bladeSpeedRelWind == 0, which is treated as <= 0.
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(100, 0, 0), /*meanBladeSpeedRelHeli=*/100.0, travelNegX, /*bladePitch=*/0.1);
		CHECK(!result.has_value());
	}
}

TEST_CASE("calculateBladeElementAirflow returns correct blade speed relative to wind")
{
	SECTION("Pure hover: blade speed equals rotational speed")
	{
		// No helicopter velocity; blade speed relative to wind equals its rotational speed
		constexpr double meanBladeSpeed = 100.0;
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(0, 0, 0), meanBladeSpeed, travelX, /*bladePitch=*/0.1);
		REQUIRE(result.has_value());
		CHECK(result->bladeSpeedRelAirflow == Approx(meanBladeSpeed));
	}

	SECTION("Advancing blade: forward flight adds to blade speed")
	{
		// Helicopter moving in +X at 50 m/s; blade also traveling in +X.
		// Advancing blade experiences increased airspeed.
		constexpr double heliSpeed = 50.0;
		constexpr double rotationalSpeed = 100.0;
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(heliSpeed, 0, 0), rotationalSpeed, travelX, /*bladePitch=*/0.1);
		REQUIRE(result.has_value());
		CHECK(result->bladeSpeedRelAirflow == Approx(rotationalSpeed + heliSpeed));
	}

	SECTION("Retreating blade: forward flight reduces blade speed")
	{
		// Helicopter moving in +X at 50 m/s; blade traveling in -X.
		// Retreating blade experiences reduced (but still positive) airspeed.
		constexpr double heliSpeed = 50.0;
		constexpr double rotationalSpeed = 100.0;
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(heliSpeed, 0, 0), rotationalSpeed, travelNegX, /*bladePitch=*/0.1);
		REQUIRE(result.has_value());
		CHECK(result->bladeSpeedRelAirflow == Approx(rotationalSpeed - heliSpeed));
	}

	SECTION("Forward flight does not affect blade speed when blade travels perpendicular to flight direction")
	{
		// Helicopter moving in +X; blade traveling in +Y.
		// The +X helicopter velocity has no component in the +Y blade travel direction.
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(50, 0, 0), /*meanBladeSpeedRelHeli=*/100.0, travelY, /*bladePitch=*/0.1);
		REQUIRE(result.has_value());
		CHECK(result->bladeSpeedRelAirflow == Approx(100.0));
	}

	SECTION("Vertical (Z) inflow does not contribute to blade speed relative to wind")
	{
		// Only the in-plane (XY) velocity components determine the advancing/retreating blade speed.
		// A vertical inflow should not change bladeSpeedRelWind.
		constexpr double meanBladeSpeed = 100.0;
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(0, 0, 20), meanBladeSpeed, travelX, /*bladePitch=*/0.1);
		REQUIRE(result.has_value());
		CHECK(result->bladeSpeedRelAirflow == Approx(meanBladeSpeed));
	}
}

TEST_CASE("calculateBladeElementAirflow returns correct angle of attack")
{
	SECTION("Blade pitch equals angle of attack when there is no inflow")
	{
		// With no vertical velocity, the inflow angle atan(0 / speed) = 0,
		// so angle of attack equals blade pitch directly.
		constexpr double bladePitch = 0.2; // radians
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(0, 0, 0), /*meanBladeSpeedRelHeli=*/100.0, travelX, bladePitch);
		REQUIRE(result.has_value());
		CHECK(result->angleOfAttack == Approx(bladePitch));
	}

	SECTION("Descending increases angle of attack above blade pitch")
	{
		// Helicopter moving downward (+Z): apparent wind comes from below, increasing AoA.
		constexpr double bladePitch = 0.2;
		constexpr double verticalSpeed = 10.0;
		constexpr double bladeSpeed = 100.0;
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(0, 0, verticalSpeed), bladeSpeed, travelX, bladePitch);
		REQUIRE(result.has_value());
		CHECK(result->angleOfAttack == Approx(bladePitch + std::atan(verticalSpeed / bladeSpeed)));
		CHECK(result->angleOfAttack > bladePitch);
	}

	SECTION("Climbing decreases angle of attack below blade pitch")
	{
		// Helicopter moving upward (-Z): apparent wind comes from above, decreasing AoA.
		constexpr double bladePitch = 0.2;
		constexpr double verticalSpeed = -10.0;
		constexpr double bladeSpeed = 100.0;
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(0, 0, verticalSpeed), bladeSpeed, travelX, bladePitch);
		REQUIRE(result.has_value());
		CHECK(result->angleOfAttack == Approx(bladePitch + std::atan(verticalSpeed / bladeSpeed)));
		CHECK(result->angleOfAttack < bladePitch);
	}

	SECTION("Inflow angle is based on airflow component in the blade cross-section plane, not influenced by perpendicular blade radial component")
	{
		// Helicopter has +X velocity; blade travels in +Y. The +X component of helicopter
		// velocity doesn't contribute to blade speed in the +Y direction, so the inflow
		// angle atan(Vz / bladeSpeed) uses the unmodified rotational speed.
		constexpr double bladePitch = 0.0;
		constexpr double verticalSpeed = 10.0;
		constexpr double bladeSpeed = 100.0;
		auto result = MainRotorComponent::calculateBladeElementAirflow(
			-Vector3(50, 0, verticalSpeed), bladeSpeed, travelY, bladePitch);
		REQUIRE(result.has_value());
		CHECK(result->angleOfAttack == Approx(bladePitch + std::atan(verticalSpeed / bladeSpeed)));
	}
}
