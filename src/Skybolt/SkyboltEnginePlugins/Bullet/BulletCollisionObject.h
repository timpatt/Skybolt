/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/EntityId.h>

class btCollisionObject;

namespace skybolt {
namespace sim {

struct BulletCollisionObject
{
	virtual ~BulletCollisionObject() = default;

	virtual const EntityId& getOwnerEntityId() const = 0;
	virtual const btCollisionObject* getBtCollisionObject() const = 0;
};

} // namespace sim
} // namespace skybolt
