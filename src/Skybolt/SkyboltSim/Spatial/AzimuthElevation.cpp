#include "AzimuthElevation.h"

#include <glm/glm.hpp>

namespace skybolt::sim {
	
using AzimuthElevation = glm::dvec2;
	
AzimuthElevation directionToAzimuthElevation(const Vector3& dir)
{
	double horizontalLength = glm::length(glm::dvec2(dir));
	return AzimuthElevation(
		std::atan2(dir.y, dir.x),
		std::atan2(-dir.z, horizontalLength)
		);
}

Vector3 azimuthAndElevationToDirection(const AzimuthElevation& azimuthElevation)
{
	double cosElevation = std::cos(azimuthElevation.y);
	return Vector3(
		std::cos(azimuthElevation.x) * cosElevation,
		std::sin(azimuthElevation.x) * cosElevation,
		-std::sin(azimuthElevation.y)
	);
}

} // namespace skybolt::sim