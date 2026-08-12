/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BuildingColinearEdgeDetector.h"

#include <SkyboltCommon/Logging/Logging.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltSim/Spatial/GreatCircle.h>

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace skybolt {
namespace mapfeatures {
namespace {

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using Point2 = bg::model::d2::point_xy<double>;
using Segment2 = bg::model::segment<Point2>;
using Box2 = bg::model::box<Point2>;

constexpr double overlapParallelEpsilonMeters = 0.001; //!< Segments must overlap by at least this much (in meters) to be considered overlapping.
constexpr double overlapPerpendicularEpsilonMeters = 0.05; //!< Two colinear edges are considered coincident (the same line) if the perpendicular distance between them is less than this, in meters.
constexpr double boundsExpansionEpsilonMeters = 0.1; //!< Expand bounding boxes by this much (in meters) to avoid missing edges that are very close together.
constexpr double minEdgeLengthMeters = 0.001; //!< Edges shorter than this (in meters) are ignored.
constexpr double colinearityAngleEpsilonRadians = 0.1; //!< Two edges are considered colinear if the angle between their direction vectors is less than this, in radians. Easier to tune than a raw cross-product tolerance.

const double overlapParallelEpsilon = overlapParallelEpsilonMeters / sim::earthRadius(); //!< In radians of latitude/longitude.
const double overlapPerpendicularEpsilon = overlapPerpendicularEpsilonMeters / sim::earthRadius(); //!< In radians of latitude/longitude.
const double boundsExpansionEpsilon = boundsExpansionEpsilonMeters / sim::earthRadius(); //!< In radians of latitude/longitude.
const double minEdgeLengthSquared = std::pow(minEdgeLengthMeters / sim::earthRadius(), 2.0);
const double colinearityMinDot = std::cos(colinearityAngleEpsilonRadians); //!< Minimum dot product of two unit direction vectors for them to be considered colinear.
const double colinearitySin = std::sin(colinearityAngleEpsilonRadians); //!< sin(angleEpsilon), used to scale cross-product based colinearity checks. For small angles this is approximately equal to colinearityAngleEpsilonRadians.

struct BuildingEdge
{
	size_t buildingIndex;
	size_t edgeIndex;
	Point2 a;
	Point2 b;
	Point2 direction;
	Box2 bounds;
};

Point2 toPoint(const sim::LatLonAlt& point)
{
	return Point2(point.lon, point.lat);
}

Point2 subtract(const Point2& a, const Point2& b)
{
	return Point2(a.x() - b.x(), a.y() - b.y());
}

double dot(const Point2& a, const Point2& b)
{
	return a.x() * b.x() + a.y() * b.y();
}

double cross(const Point2& a, const Point2& b)
{
	return a.x() * b.y() - a.y() * b.x();
}

double lengthSquared(const Point2& v)
{
	return dot(v, v);
}

Box2 makeExpandedBox(const Point2& a, const Point2& b)
{
	Box2 box;
	bg::assign_inverse(box);
	bg::expand(box, a);
	bg::expand(box, b);
	bg::set<bg::min_corner, 0>(box, bg::get<bg::min_corner, 0>(box) - boundsExpansionEpsilon);
	bg::set<bg::min_corner, 1>(box, bg::get<bg::min_corner, 1>(box) - boundsExpansionEpsilon);
	bg::set<bg::max_corner, 0>(box, bg::get<bg::max_corner, 0>(box) + boundsExpansionEpsilon);
	bg::set<bg::max_corner, 1>(box, bg::get<bg::max_corner, 1>(box) + boundsExpansionEpsilon);
	return box;
}

double polygonAreaMagnitude(const Building& building)
{
	double area = 0.0;
	const auto& points = building.points;
	if (points.size() < 3)
	{
		return 0.0;
	}

	for (size_t i = 0; i < points.size(); ++i)
	{
		const auto& a = points[i];
		const auto& b = points[(i + 1) % points.size()];
		area += a.lon * b.lat - b.lon * a.lat;
	}
	return std::fabs(area) * 0.5;
}

bool haveSameFacingColinearOverlap(const BuildingEdge& lhs, const BuildingEdge& rhs)
{
	const Point2 lhsVector = subtract(lhs.b, lhs.a);
	const Point2 rhsVector = subtract(rhs.b, rhs.a);
	const double lhsLengthSq = lengthSquared(lhsVector);
	const double rhsLengthSq = lengthSquared(rhsVector);
	if (lhsLengthSq <= minEdgeLengthSquared || rhsLengthSq <= minEdgeLengthSquared)
	{
		return false;
	}

	if (dot(lhs.direction, rhs.direction) <= colinearityMinDot)
	{
		return false;
	}

	if (std::fabs(cross(lhsVector, rhsVector)) > colinearitySin * std::sqrt(lhsLengthSq * rhsLengthSq))
	{
		return false;
	}

	const double axisLength = std::sqrt(lhsLengthSq);
	const Point2 axis(lhsVector.x() / axisLength, lhsVector.y() / axisLength);

	// Perpendicular distance from rhs.a to the line through lhs. This is a true
	// distance (not scaled by edge length), so compare against a distance-based tolerance.
	const Point2 offset = subtract(rhs.a, lhs.a);
	const double perpendicularDistance = std::fabs(cross(lhsVector, offset)) / axisLength;
	if (perpendicularDistance > overlapPerpendicularEpsilon)
	{
		return false;
	}

	const double lhsMin = 0.0;
	const double lhsMax = axisLength;
	const double rhsStart = dot(subtract(rhs.a, lhs.a), axis);
	const double rhsEnd = dot(subtract(rhs.b, lhs.a), axis);
	const double rhsMin = std::min(rhsStart, rhsEnd);
	const double rhsMax = std::max(rhsStart, rhsEnd);
	const double overlap = std::min(lhsMax, rhsMax) - std::max(lhsMin, rhsMin);
	return overlap > overlapParallelEpsilon;
}

std::vector<BuildingEdge> collectEdges(const std::vector<Building*>& buildings)
{
	std::vector<BuildingEdge> edges;
	for (size_t buildingIndex = 0; buildingIndex < buildings.size(); ++buildingIndex)
	{
		const Building& building = *buildings[buildingIndex];
		const auto& points = building.points;
		if (points.size() < 2)
		{
			continue;
		}

		for (size_t edgeIndex = 0; edgeIndex < points.size(); ++edgeIndex)
		{
			const Point2 a = toPoint(points[edgeIndex]);
			const Point2 b = toPoint(points[(edgeIndex + 1) % points.size()]);
			const Point2 vector = subtract(b, a);
			const double lenSq = lengthSquared(vector);
			if (lenSq <= minEdgeLengthSquared)
			{
				continue;
			}

			const double len = std::sqrt(lenSq);
			edges.push_back({
				buildingIndex,
				edgeIndex,
				a,
				b,
				Point2(vector.x() / len, vector.y() / len),
				makeExpandedBox(a, b)
			});
		}
	}
	return edges;
}

} // namespace

std::vector<BuildingPair> findBuildingsWithColinearEdges(const std::vector<Building*>& buildings)
{
	std::vector<BuildingPair> result;
	const std::vector<BuildingEdge> edges = collectEdges(buildings);
	using RTreeValue = std::pair<Box2, size_t>;
	bgi::rtree<RTreeValue, bgi::quadratic<16>> rtree;

	for (size_t i = 0; i < edges.size(); ++i)
	{
		rtree.insert({edges[i].bounds, i});
	}

	std::set<std::pair<size_t, size_t>> uniquePairs;
	for (size_t i = 0; i < edges.size(); ++i)
	{
		std::vector<RTreeValue> candidates;
		rtree.query(bgi::intersects(edges[i].bounds), std::back_inserter(candidates));
		for (const auto& candidate : candidates)
		{
			const size_t j = candidate.second;
			if (j <= i)
			{
				continue;
			}

			const BuildingEdge& lhs = edges[i];
			const BuildingEdge& rhs = edges[j];
			if (lhs.buildingIndex == rhs.buildingIndex)
			{
				continue;
			}

			if (!haveSameFacingColinearOverlap(lhs, rhs))
			{
				continue;
			}

			auto pair = std::minmax(lhs.buildingIndex, rhs.buildingIndex);
			if (uniquePairs.insert(pair).second)
			{
				result.push_back({pair.first, pair.second});
			}
		}
	}

	return result;
}

const Building* findLargerBuildingByArea(const Building& first, const Building& second)
{
	return polygonAreaMagnitude(first) >= polygonAreaMagnitude(second) ? &first : &second;
}

void removeSmallerBuildingsWithColinearEdges(std::vector<FeaturePtr>& features)
{
	std::vector<Building*> buildings;
	std::vector<size_t> featureIndices;
	for (size_t i = 0; i < features.size(); ++i)
	{
		if (auto building = std::dynamic_pointer_cast<Building>(features[i]))
		{
			buildings.push_back(building.get());
			featureIndices.push_back(i);
		}
	}

	const std::vector<BuildingPair> pairs = findBuildingsWithColinearEdges(buildings);
	std::set<size_t> buildingsToRemove;
	for (const BuildingPair& pair : pairs)
	{
		const Building* larger = findLargerBuildingByArea(*buildings[pair.first], *buildings[pair.second]);
		const size_t buildingIndexToRemove = (larger == buildings[pair.first]) ? pair.second : pair.first;
		buildingsToRemove.insert(featureIndices[buildingIndexToRemove]);
	}
	if (buildingsToRemove.empty())
	{
		return;
	}

	std::vector<FeaturePtr> filtered;
	filtered.reserve(features.size() - buildingsToRemove.size());
	for (size_t i = 0; i < features.size(); ++i)
	{
		if (!buildingsToRemove.contains(i))
		{
			filtered.push_back(features[i]);
		}
	}
	features.swap(filtered);

	SKYBOLT_LOG(info) << "Removed " << buildingsToRemove.size() << " buildings with colinear edges";
}

} // namespace mapfeatures
} // namespace skybolt
