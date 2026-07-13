/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BulletSystem.h"
#include "BulletCollisionObject.h"
#include "BulletWorld.h"
#include "BulletDynamicBodyComponent.h"
#include "BulletTypeConversion.h"
#include "KinematicBody.h"

#include <SkyboltSim/Entity.h>

namespace skybolt::sim {

const BulletCollisionObject* getWrappedCollisionObject(const btCollisionObject& object)
{
	return static_cast<const BulletCollisionObject*>(object.getUserPointer());
}

void setPointerToWrappedCollisionObject(btCollisionObject& object, BulletCollisionObject* bulletCollisionObject)
{
	object.setUserPointer(bulletCollisionObject);
}

sim::EntityId getEntity(const btCollisionObject& object)
{
	return static_cast<const BulletCollisionObject*>(object.getUserPointer())->getOwnerEntityId();
}

const btCollisionObject* getBtCollisionObject(const Entity& entity)
{
	if (auto bulletCollisionObject = entity.getFirstComponent<BulletCollisionObject>())
	{
		return bulletCollisionObject->getBtCollisionObject();
	}
	return nullptr;
}

BulletSystem::BulletSystem(BulletWorld* world) :
	mWorld(world)
{
	assert(mWorld);
}

void BulletSystem::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	mDt += dt;
}

std::optional<RayIntersectionResult> BulletSystem::intersectRay(const Vector3 &start, const Vector3 &end, int collisionFilterMask, const Entity* entityToIgnore) const
{
	const btCollisionObject* objectToIgnore = entityToIgnore ? getBtCollisionObject(*entityToIgnore) : nullptr;

	return mWorld->intersectRay(start, end, collisionFilterMask, objectToIgnore);
}

void BulletSystem::performSubStep()
{
	mWorld->getDynamicsWorld()->stepSimulation(mDt, 0, mDt);
	processCollisionEvents();
	mDt = 0;
};

void BulletSystem::processCollisionEvents()
{
	const auto& dynamicsWorld = mWorld->getDynamicsWorld();
    int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i=0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* objectA = contactManifold->getBody0();
		const btCollisionObject* objectB = contactManifold->getBody1();

		int numContacts = contactManifold->getNumContacts();
		for (int j=0;j<numContacts;j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance()<0.f)
			{
				CollisionEvent event;
				event.entityA = objectA->getUserPointer() ? getEntity(*objectA) : nullEntityId();
				event.entityB = objectB->getUserPointer() ? getEntity(*objectB) : nullEntityId();
				event.bodyCategoryA = objectA->getCollisionFlags();
				event.bodyCategoryB = objectB->getCollisionFlags();
				event.position = toGlmDvec3(pt.getPositionWorldOnB());
				event.normalB = toGlmDvec3(pt.m_normalWorldOnB);
				if (event.entityA != nullEntityId() || event.entityB != nullEntityId())
				{
					mEventEmitter->emitEvent(event);
				}
			}
		}
	}
}

} // namespace skybolt::sim