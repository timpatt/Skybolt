/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

class Trimmable
{
public:
	virtual ~Trimmable() = default;

	struct Controls
	{
		glm::dvec2 stickInput; //!< range [-1, 1]. Positive backward and right.
		double rudderInput; //!< range [-1, 1]
	};

	//! Calculates the moment required to achieve zero angular velocity, given the control inputs.
	//! Used by a trim solver to find the control inputs that achieve a trimmed state.
	virtual Vector3 calcRotationalTrimMomentInBodyAxes(const Controls& controls) const = 0;
};

} // namespace sim
} // namespace skybolt