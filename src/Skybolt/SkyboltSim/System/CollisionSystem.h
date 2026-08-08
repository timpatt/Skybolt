/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Event.h>
#include <SkyboltCommon/SkyboltCommonFwd.h>
#include <SkyboltSim/EntityId.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/System/System.h>

#include <optional>

namespace skybolt::sim {

using CollisionGroupMask = int;

struct CollisionEvent : public EventT<CollisionEvent>
{
	EntityId entityA; //!< First object involved in the collision or nullEntityId() if not an entity
	EntityId entityB; //!< Second object involved in the collision or nullEntityId() if not an entity
	Vector3 position; //!< Position of impact point
	Vector3 normalB; //!< Direction of the second object's normal force from the collision
};

struct RayIntersectionResult
{
	Vector3 position;
	Vector3 normal;
	double distance;
	EntityId entity;
};

class CollisionSystem : public SystemT<CollisionSystem>
{
public:
	~CollisionSystem() override = default;
	EventEmitterPtr getEventEmitter() const { return mEventEmitter; }

	virtual std::optional<RayIntersectionResult> intersectRay(const Vector3 &position, const Vector3 &direction, double length, CollisionGroupMask collisionFilterMask, const Entity* entityToIgnore = nullptr) const
	{
		Vector3 end = position + length * direction;
		return intersectRay(position, end, collisionFilterMask, entityToIgnore);
	}

	virtual std::optional<RayIntersectionResult> intersectRay(const Vector3 &start, const Vector3 &end, CollisionGroupMask collisionFilterMask, const Entity* entityToIgnore = nullptr) const { return std::nullopt; };

protected:
	EventEmitterPtr mEventEmitter = std::make_shared<EventEmitter>();
};

} // namespace skybolt::sim