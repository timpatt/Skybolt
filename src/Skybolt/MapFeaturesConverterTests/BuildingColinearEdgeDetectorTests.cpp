/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch_test_macros.hpp>

#include <MapFeaturesConverter/BuildingColinearEdgeDetector.h>

using namespace skybolt::mapfeatures;

namespace {

Building makeBuilding(std::initializer_list<std::pair<double, double>> lonLatPoints)
{
	Building building;
	for (const auto& [lon, lat] : lonLatPoints)
	{
		building.points.emplace_back(lat, lon, 0.0);
	}
	building.height = 10.0f;
	return building;
}

} // namespace

TEST_CASE("findBuildingsWithColinearEdges detects same-facing overlapping edges", "[MapFeaturesConverter]")
{
	/*
	    -------
	    |  2  |
		-------
		| 1, 2|
		-------
	*/
	Building first = makeBuilding({{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}});
	Building second = makeBuilding({{0.0, 0.0}, {1.0, 0.0}, {1.0, 2.0}, {0.0, 2.0}});

	std::vector<Building*> buildings = {&first, &second};
	const auto pairs = findBuildingsWithColinearEdges(buildings);

	REQUIRE(pairs.size() == 1);
	CHECK(pairs.front().first == 0);
	CHECK(pairs.front().second == 1);
}

TEST_CASE("findBuildingsWithColinearEdges ignores opposite-facing shared walls", "[MapFeaturesConverter]")
{
	/*
	    -------
	    |  2  |
		-------
		|  1  |
		-------
	*/
	Building first = makeBuilding({{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}});
	Building second = makeBuilding({{0.0, 1.0}, {1.0, 1.0}, {1.0, 2.0}, {0.0, 2.0}});

	std::vector<Building*> buildings = {&first, &second};
	const auto pairs = findBuildingsWithColinearEdges(buildings);

	CHECK(pairs.empty());
}

TEST_CASE("findBuildingsWithColinearEdges ignores intersecting non-colinear faces", "[MapFeaturesConverter]")
{
	/*
	       -------
	       |  2  |
		-------  |
		|  -------
		|  1  |
		-------
	*/
	Building first = makeBuilding({{0.0, 0.0}, {2.0, 0.0}, {2.0, 2.0}, {0.0, 2.0}});
	Building second = makeBuilding({{1, 1}, {3, 1}, {3, 3}, {1, 3}});

	std::vector<Building*> buildings = {&first, &second};
	const auto pairs = findBuildingsWithColinearEdges(buildings);

	CHECK(pairs.empty());
}

TEST_CASE("findLargerBuildingByArea returns the larger footprint", "[MapFeaturesConverter]")
{
	Building small = makeBuilding({{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}});
	Building large = makeBuilding({{0.0, 0.0}, {3.0, 0.0}, {3.0, 2.0}, {0.0, 2.0}});

	CHECK(findLargerBuildingByArea(small, large) == &large);
	CHECK(findLargerBuildingByArea(large, small) == &large);
}
