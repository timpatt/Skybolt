/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltCommon/Math/LookupTable1D.h>
#include <catch2/catch.hpp>

using namespace skybolt;
using namespace math;

TEST_CASE("findInterpolationPoint")
{
	std::vector<double> xData = { 4, 5, 7 };

	SECTION("Null returned when interpolating empty vector")
	{
		std::optional<InterpolationPoint> point = findInterpolationPoint({}, 2, /* extrapolate */ true);
		CHECK(!point.has_value());
	}

	SECTION("Extrapolate below lower bound")
	{
		std::optional<InterpolationPoint> point = findInterpolationPoint(xData, 2, /* extrapolate */ true);
		REQUIRE(point.has_value());
		CHECK(point->bounds.first == 0);
		CHECK(point->bounds.last == 1);
		CHECK(point->weight == -2);
	}

	SECTION("Extrapolate above upper bound")
	{
		std::optional<InterpolationPoint> point = findInterpolationPoint(xData, 8, /* extrapolate */ true);
		REQUIRE(point.has_value());
		CHECK(point->bounds.first == 1);
		CHECK(point->bounds.last == 2);
		CHECK(point->weight == 1.5);
	}

	SECTION("Clamp below lower bound")
	{
		std::optional<InterpolationPoint> point = findInterpolationPoint(xData, 2, /* extrapolate */ false);
		REQUIRE(point.has_value());
		CHECK(point->bounds.first == 0);
		CHECK(point->bounds.last == 1);
		CHECK(point->weight == 0);
	}

	SECTION("Clamp above upper bound")
	{
		std::optional<InterpolationPoint> point = findInterpolationPoint(xData, 8, /* extrapolate */ false);
		REQUIRE(point.has_value());
		CHECK(point->bounds.first == 1);
		CHECK(point->bounds.last == 2);
		CHECK(point->weight == 1);
	}

	SECTION("Interpolate")
	{
		std::optional<InterpolationPoint> point = findInterpolationPoint(xData, 5.5, /* extrapolate */ false);
		REQUIRE(point.has_value());
		CHECK(point->bounds.first == 1);
		CHECK(point->bounds.last == 2);
		CHECK(point->weight == 0.25);
	}
}

TEST_CASE("interpolateTableLinear")
{
	std::vector<double> xData = { 1.0, 2.0, 4.0 };
	std::vector<double> yData = { 10.0, 20.0, 40.0 };
	LookupTable1D table{ xData, yData };

	SECTION("Null returned when vectors or table are empty")
	{
		CHECK(!interpolateTableLinear({}, {}, 2.0, /* extrapolate */ true).has_value());
		CHECK(!interpolateTableLinear(LookupTable1D{}, 2.0, /* extrapolate */ true).has_value());
	}

	SECTION("Exact match on data points")
	{
		// Exact match at the lower bound
		auto resultLower = interpolateTableLinear(table, 1.0, /* extrapolate */ false);
		REQUIRE(resultLower.has_value());
		CHECK(*resultLower == 10.0);

		// Exact match in the middle
		auto resultMiddle = interpolateTableLinear(table, 2.0, /* extrapolate */ false);
		REQUIRE(resultMiddle.has_value());
		CHECK(*resultMiddle == 20.0);

		// Exact match at the upper bound
		auto resultUpper = interpolateTableLinear(table, 4.0, /* extrapolate */ false);
		REQUIRE(resultUpper.has_value());
		CHECK(*resultUpper == 40.0);
	}

	SECTION("Interpolate between data points")
	{
		auto result = interpolateTableLinear(table, 1.5, /* extrapolate */ false);
		REQUIRE(result.has_value());
		CHECK(*result == 15.0);
	}

	SECTION("Clamp below lower bound (extrapolate = false)")
	{
		auto result = interpolateTableLinear(table, 0.5, /* extrapolate */ false);
		REQUIRE(result.has_value());
		CHECK(*result == 10.0);
	}

	SECTION("Clamp above upper bound (extrapolate = false)")
	{
		// Should clamp to the last y-value (40.0)
		auto result = interpolateTableLinear(table, 5.0, /* extrapolate */ false);
		REQUIRE(result.has_value());
		CHECK(*result == 40.0);
	}

	SECTION("Extrapolate below lower bound (extrapolate = true)")
	{
		auto result = interpolateTableLinear(table, 0.5, /* extrapolate */ true);
		REQUIRE(result.has_value());
		CHECK(*result == 5.0);
	}

	SECTION("Extrapolate above upper bound (extrapolate = true)")
	{
		auto result = interpolateTableLinear(table, 5.0, /* extrapolate */ true);
		REQUIRE(result.has_value());
		CHECK(*result == 50.0);
	}

	SECTION("Vector overload matches LookupTable1D overload")
	{
		double x = 5;
		bool extrapolate = false;
		
		auto resVector = interpolateTableLinear(xData, yData, x, extrapolate);
		auto resTable = interpolateTableLinear(table, x, extrapolate);
		
		REQUIRE(resVector.has_value());
		REQUIRE(resTable.has_value());
		CHECK(*resVector == *resTable);
	}
}