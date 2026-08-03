/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "LookupTable1D.h"
#include "MathUtility.h"

namespace skybolt {
namespace math {

std::optional<InterpolationPoint> findInterpolationPoint(const std::vector<double> &xData, double x, bool extrapolate)
{
	int size = (int)xData.size();
	if (size == 0)
	{
		return std::nullopt;
	}
	else if (size == 1)
	{
		InterpolationPoint point;
		point.bounds.first = 0;
		point.bounds.last = 0;
		point.weight = 0;
		return point;
	}

	// Find left bound
	int i = 0;                                                                  
	if (x >= xData[size - 2]) // Make sure we're not past the right bound
	{
		i = size - 2;
	}
	else
	{
		while (x > xData[i + 1])
		{
			i++;
		}
	}
	double xL = xData[i];
	double xR = xData[i + 1];

	InterpolationPoint point;
	point.bounds.first = i;
	point.bounds.last = i + 1;
	point.weight = (x - xL) / (xR - xL);
	
	if (!extrapolate)
	{
		point.weight = math::clamp(point.weight, 0.0, 1.0);
	}

	return point;
}

std::optional<double> interpolateTableLinear(const std::vector<double> &xData, const std::vector<double> &yData, double x, bool extrapolate)
{
	std::optional<InterpolationPoint> point = findInterpolationPoint(xData, x, extrapolate);
	if (!point)
	{
		return std::nullopt;
	}
	return math::lerp(yData.at(point->bounds.first), yData.at(point->bounds.last), point->weight);
}

std::optional<double> interpolateTableLinear(const LookupTable1D& table, double x, bool extrapolate)
{
	return interpolateTableLinear(table.xData, table.yData, x, extrapolate);
}

math::LookupTable1D readLookupTable1D(const nlohmann::json& json)
{
	if (!json.is_array())
	{
		throw std::runtime_error("Expected array for lookup table.");
	}
	std::vector<double> xData;
	std::vector<double> yData;
	for (const auto& pointJson : json)
	{
		if (!pointJson.is_array() || pointJson.size() != 2)
		{
			throw std::runtime_error("Each point in lookup table must be an array of two numbers.");
		}
		xData.push_back(pointJson[0].get<double>());
		yData.push_back(pointJson[1].get<double>());
	}
	return math::LookupTable1D({xData, yData});
}

ScalarOrCurve readOptionalScalarOrCurve(const nlohmann::json& json, const std::string& key, double defaultScalar)
{
	if (json.contains(key))
	{
		const auto& value = json.at(key);
		if (value.is_number())
		{
			return value.get<double>();
		}
		else if (value.is_array())
		{
			return readLookupTable1D(value);
		}
		else
		{
			throw std::runtime_error("Invalid type for " + key + ". Expected number or array.");
		}
	}
	return defaultScalar;
}

} // namespace math
} // namespace skybolt
