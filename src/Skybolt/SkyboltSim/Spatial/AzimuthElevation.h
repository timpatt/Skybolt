#pragma once

#include "SkyboltSim/SimMath.h"

namespace skybolt::sim {
	
using AzimuthElevation = glm::dvec2;
	
AzimuthElevation directionToAzimuthElevation(const Vector3& dir);

Vector3 azimuthAndElevationToDirection(const AzimuthElevation& azimuthElevation);

} // namespace skybolt::sim