/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttachmentPointsComponent.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/Spatial/Positionable.h"
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

namespace skybolt {
namespace sim {

void addAttachmentPoint(Entity& entity, const std::string& name, const AttachmentPointPtr& point)
{
	assert(point);

	auto points = entity.getFirstComponent<AttachmentPointsComponent>();
	if (!points)
	{
		points = std::make_shared<AttachmentPointsComponent>();
		entity.addComponent(points);
	}
	points->attachmentPoints[name] = point;
}

AttachmentPointPtr findAttachmentPoint(const Entity& entity, const std::string& name)
{
	auto points = entity.getFirstComponent<AttachmentPointsComponent>();
	if (points)
	{
		return findOptional(points->attachmentPoints, name).value_or(nullptr);
	}
	return nullptr;
}

AttachmentPointPtr findAttachmentPointRequired(const Entity& entity, const std::string& name)
{
	auto result = findAttachmentPoint(entity, name);
	if (result)
	{
		return result;
	}
	throw std::runtime_error("Unable to find attachment point: '" + name + "'");
}

Vector3 calcAttachmentPointPosition(const Entity& entity, const AttachmentPoint& point)
{
	auto position = getPosition(entity);
	auto orientation = getOrientation(entity);
	if (position && orientation)
	{
		return *position + *orientation * point.positionRelBody;
	}
	else
	{
		return math::dvec3Zero();
	}
}

Quaternion calcAttachmentPointOrientation(const Entity& entity, const AttachmentPoint& point)
{
	auto orientation = getOrientation(entity);
	if (orientation)
	{
		return *orientation * point.orientationRelBody;
	}
	else
	{
		return math::dquatIdentity();
	}
}

Vector3 calcAttachmentPointPosition(const Positionable& positionable, const AttachmentPoint& point)
{
	return positionable.getPosition() + positionable.getOrientation() * point.positionRelBody;
}

Quaternion calcAttachmentPointOrientation(const Positionable& positionable, const AttachmentPoint& point)
{
	return positionable.getOrientation() * point.orientationRelBody;
}

} // namespace sim
} // namespace skybolt