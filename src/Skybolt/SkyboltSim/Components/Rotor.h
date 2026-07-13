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

class Rotor
{
public:
	virtual ~Rotor() = default;

	virtual double getNormalizedRpm() const = 0;

	virtual void setNormalizedRpm(double rpm) = 0;

	virtual double getRotorRpm() const = 0;

	virtual double getRotationAngle() const = 0;

	virtual const Vector3& getHubPositionRelBody() const = 0;

	virtual const Quaternion& getTppOrientationRelBody() const = 0;
};

} // namespace sim
} // namespace skybolt