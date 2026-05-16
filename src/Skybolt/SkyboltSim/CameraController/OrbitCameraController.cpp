/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "OrbitCameraController.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/World.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Spatial/Geocentric.h"
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Math/FirstOrderLag.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

namespace skybolt::sim {

const float OrbitCameraController::msYawRate = 1.0f;
const float OrbitCameraController::msPitchRate = 1.0f;
const float OrbitCameraController::msZoomRate = 1.0f;

SKYBOLT_REFLECT(OrbitCameraController) {
	registry.type<OrbitCameraController>("OrbitCameraController")
		.superType<CameraController>()
		.superType<CameraModifierStack>()
		.superType<ExplicitSerialization>()
		.superType<Pitchable>()
		.superType<EntityTargeter>()
		.superType<Yawable>()
		.superType<Dollyable>()
		.property("lagTimeConstant", &OrbitCameraController::lagTimeConstant)
		.property("lockOrientationToTarget", &OrbitCameraController::lockOrientationToTarget)
		.property("targetPositionOffset", &OrbitCameraController::targetPositionOffset);
}

OrbitCameraController::OrbitCameraController(sim::Entity* camera, sim::World* world, const Params& params, const CameraModifierFactoryRegistryPtr& cameraModifierFactories) :
	CameraController(camera),
	CameraModifierStack(cameraModifierFactories),
	EntityTargeter(world),
	mParams(params)
{
	setDollyFactor(0.5f);
}

void OrbitCameraController::resetFiltering()
{
	mSmoothedTargetOrientation.reset();
	resetState(*this);
}

void OrbitCameraController::setActive(bool active)
{
	CameraController::setActive(active);
	resetFiltering();
}

static Quaternion safeSlerp(const Quaternion& a, const Quaternion& b, double t)
{
	Quaternion sSafe = (glm::dot(a, b) < 0) ? -a : a;
	return glm::slerp(sSafe, b, t);
}

void OrbitCameraController::updatePostDynamicsSubstep(SecondsD simTime, SecondsD dtSubstep)
{
	if (Entity* entity = getTarget(); entity)
	{
		Quaternion orientation = getOrientation(*entity).value_or(math::dquatIdentity());
		if (mSmoothedTargetOrientation)
		{
			orientation = safeSlerp(*mSmoothedTargetOrientation, orientation, calcFirstOrderLagInterpolationFactor(dtSubstep, lagTimeConstant));
		}
		mSmoothedTargetOrientation = orientation;
	}
}

void OrbitCameraController::updateTimeStep(const UpdateTimeStepArgs& args)
{
	// Reset filtering when target changes, to avoid a sudden jump in camera orientation
	EntityId targetId = getTargetId();
	if (targetId != mPrevTargetId)
	{
		resetFiltering();
		mPrevTargetId = targetId;
	}

	// Update rotation and zoom based on input
	mYaw += msYawRate * mInput.yawRate * args.wallTimeStep;
	mPitch += msPitchRate * mInput.tiltRate * args.wallTimeStep;
	mDollyFactor += msZoomRate * mInput.zoomRate * args.wallTimeStep;
	mDollyFactor = math::clamp(mDollyFactor, 0.0, 1.0);
    
    double maxPitch = math::halfPiD();
    mPitch = math::clamp(mPitch, -maxPitch, maxPitch);

	// Get current target state
	sim::Entity* target = getTarget();
	if (!target)
	{
		return;
	}
	auto targetPosition = getPosition(*target);
	if (!targetPosition)
	{
		return;
	}

	// Calculate orientation
	Quaternion targetOrientation;
	if (lockOrientationToTarget)
	{
		targetOrientation = mSmoothedTargetOrientation.value_or(getOrientation(*target).value_or(math::dquatIdentity()));
	}
	else
	{
		targetOrientation = sim::latLonToGeocentricLtpOrientation(sim::geocentricToLatLon(*targetPosition));
	}

	CameraState& cameraState = mCameraComponent->getState();
	cameraState.nearClipDistance = 0.5;

	Quaternion orbitOrientation = targetOrientation * glm::angleAxis(mYaw, Vector3(0, 0, 1)) * glm::angleAxis(mPitch, Vector3(0, 1, 0));
	double dist = mParams.maxDist + mDollyFactor * (mParams.minDist - mParams.maxDist);
	Vector3 orbitOffset = orbitOrientation * (Vector3(-dist, 0, 0) + targetPositionOffset);

	// Apply camera modifiers.
	// Note that modifers are applied after calculating orbitOffset, so that modifiers can add effects like shake without affecting the underlying orbiting behavior.
	{
		CameraModifier::State state = {
			.position = *targetPosition,
			.orientation = orbitOrientation,
			.cameraState = mCameraComponent->getState()
		};
		applyUpdate(*this, state, args.newSimTime, args.simTimeStep);
		targetPosition = state.position;
		orbitOrientation = state.orientation;
		cameraState = state.cameraState;
	}

	// Apply state to camera
	mNodeComponent->setOrientation(orbitOrientation);
	mNodeComponent->setPosition(*targetPosition + orbitOffset);
}

nlohmann::json OrbitCameraController::toJson(refl::TypeRegistry& typeRegistry) const
{
	// TODO: unduplicate toJson and fromJson methods with FreeCameraController
	nlohmann::json json = writeReflectedObjectProperties(typeRegistry, refl::makeRefInstance(typeRegistry, const_cast<OrbitCameraController*>(this)));
	if (nlohmann::json modifierJson = CameraModifierStack::toJson(typeRegistry); !modifierJson.is_null())
	{
		json["cameraModifiers"] = modifierJson;
	}
	return json;
}

void OrbitCameraController::fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j)
{
	refl::Instance instance = refl::makeRefInstance(typeRegistry, this);
	readReflectedObjectProperties(typeRegistry, instance, j);
	ifChildExists(j, "cameraModifiers", [&] (const nlohmann::json& modifiersJson) {
		CameraModifierStack::fromJson(typeRegistry, modifiersJson);
	});
}

} // namespace skybolt::sim