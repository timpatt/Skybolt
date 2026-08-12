/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/Renderable/Planet/Features/PlanetFeaturesSource.h>

#include <vector>

namespace skybolt {
namespace mapfeatures {

struct BuildingPair
{
	size_t first;
	size_t second;
};

std::vector<BuildingPair> findBuildingsWithColinearEdges(const std::vector<Building*>& buildings);
const Building* findLargerBuildingByArea(const Building& first, const Building& second);
void removeSmallerBuildingsWithColinearEdges(std::vector<FeaturePtr>& features);

} // namespace mapfeatures
} // namespace skybolt
