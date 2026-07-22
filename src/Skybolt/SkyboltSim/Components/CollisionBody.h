/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/Spatial/Positionable.h"
#include <vector>
#include <map>
#include <set>

namespace skybolt {
namespace sim {

class CollisionBody
{
public:
	virtual ~CollisionBody() = default;

	virtual void setCollisionsEnabled(bool enabled) = 0;

	virtual void setCollisionGroupMask(int mask) = 0;
	virtual int getCollisionGroupMask() const = 0;

	virtual void setCollisionFilterMask(int mask) = 0;
	virtual int getCollisionFilterMask() const = 0;
};

} // namespace sim
} // namespace skybolt