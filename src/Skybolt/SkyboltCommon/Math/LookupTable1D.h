/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltCommon/Range.h"
#include <algorithm>
#include <nlohmann/json.hpp>
#include <optional>
#include <variant>
#include <vector>

namespace skybolt {
namespace math {

struct LookupTable1D
{
	// xData and yData must be the same length.
	std::vector<double> xData;
	std::vector<double> yData;
};

struct InterpolationPoint
{
	IntRangeClosedOpen bounds;
	double weight; //!< In range [0 to 1]
};

//! Returns null if the input vector is empty, otherwise returns a valid result.
std::optional<InterpolationPoint> findInterpolationPoint(const std::vector<double> &xData, double x, bool extrapolate);

//! Returns null if the input vectors is empty, otherwise returns a valid result.
//! xData and yData must be the same length.
std::optional<double> interpolateTableLinear(const std::vector<double> &xData, const std::vector<double> &yData, double x, bool extrapolate);

//! Returns null if the input vectors is empty, otherwise returns a valid result.
std::optional<double> interpolateTableLinear(const LookupTable1D& table, double x, bool extrapolate);

math::LookupTable1D readLookupTable1D(const nlohmann::json& json);

using ScalarOrCurve = std::variant<double, math::LookupTable1D>;
ScalarOrCurve readOptionalScalarOrCurve(const nlohmann::json& json, const std::string& key, double defaultScalar);

} // namespace math
} // namespace skybolt
