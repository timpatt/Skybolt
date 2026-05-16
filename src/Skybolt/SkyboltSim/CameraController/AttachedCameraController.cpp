/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "AttachedCameraController.h"
#include "SkyboltSim/Components/AttachmentPointsComponent.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/World.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/Node.h"
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

namespace skybolt::sim {

const float AttachedCameraController::msYawRate = 1.0f;
const float AttachedCameraController::msPitchRate = 1.0f;
const float AttachedCameraController::msZoomRate = 1.0f;

SKYBOLT_REFLECT(AttachedCameraController) {
	registry.type<AttachedCameraController>("AttachedCameraController")
		.superType<CameraController>()
		.superType<Pitchable>()
		.superType<EntityTargeter>()
		.superType<Yawable>()
		.property("minFovY", &AttachedCameraController::minFovY, { {PropertyMetadataNames::units, Units::Radians} })
		.property("maxFovY", &AttachedCameraController::maxFovY, { {PropertyMetadataNames::units, Units::Radians} });
}

AttachedCameraController::AttachedCameraController(Entity* camera, World* world, const Params& params) :
	CameraController(camera),
	EntityTargeter(world),
	mParams(params)
{
}

void AttachedCameraController::updateTimeStep(const UpdateTimeStepArgs& args)
{
	mYaw += msYawRate * mInput.yawRate * args.wallTimeStep;
	mPitch += msPitchRate * mInput.tiltRate * args.wallTimeStep;

	if (mInput.zoomRate != 0)
	{
		setZoom(getZoom() + mInput.zoomRate * args.wallTimeStep);
	}
	mCameraComponent->getState().fovY = std::clamp(mCameraComponent->getState().fovY, float(minFovY), float(maxFovY));
    
    double maxPitch = math::halfPiD();
    mPitch = math::clamp(mPitch, -maxPitch, maxPitch);

	CameraState& state = mCameraComponent->getState();
	state.nearClipDistance = 0.5;

	if (Entity* target = getTarget(); target)
	{
		if (const AttachmentPointPtr& attachmentPoint = findAttachmentPoint(*target))
		{
			mNodeComponent->setPosition(calcAttachmentPointPosition(*target, *attachmentPoint));
			mNodeComponent->setOrientation(calcAttachmentPointOrientation(*target, *attachmentPoint) * glm::angleAxis(mYaw, Vector3(0, 0, 1)) * glm::angleAxis(mPitch, Vector3(0, 1, 0)));
		}
	}
}

AttachmentPointPtr AttachedCameraController::findAttachmentPoint(const Entity& entity) const
{
	auto points = entity.getFirstComponent<AttachmentPointsComponent>();
	if (points)
	{
		auto point = findOptional(points->attachmentPoints, mParams.attachmentPointName);
		if (point)
		{
			return *point;;
		}
	}
	return nullptr;
}

double AttachedCameraController::getZoom() const
{
	double fovRange = maxFovY - minFovY;
	if (maxFovY == 0)
	{
		return 0.0; // Avoid division by zero
	}

	// Minimum FOV when zoom is 1, maximum FOV when zoom is 0
	return (mCameraComponent->getState().fovY - maxFovY) / -fovRange;
}

void AttachedCameraController::setZoom(double zoom)
{
	zoom = skybolt::math::clamp(zoom, 0.0, 1.0);
	mCameraComponent->getState().fovY = skybolt::math::lerp(maxFovY, minFovY, zoom);
}

} // namespace skybolt::sim