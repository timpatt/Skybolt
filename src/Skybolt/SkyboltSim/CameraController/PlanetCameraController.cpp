/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "PlanetCameraController.h"
#include "SkyboltSim/Entity.h"
#include "SkyboltSim/World.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Components/PlanetComponent.h"
#include "SkyboltSim/Spatial/Geocentric.h"
#include "SkyboltSim/Spatial/GreatCircle.h"
#include <SkyboltCommon/Math/MathUtility.h>

#include <assert.h>

namespace skybolt::sim {

const float PlanetCameraController::msYawRate = 1.f;
const float PlanetCameraController::msPitchRate = 1.f;
const float PlanetCameraController::msZoomRate = 0.2f;
constexpr float pitchControlSensitivity = 2.f;

SKYBOLT_REFLECT(PlanetCameraController) {
	registry.type<PlanetCameraController>("PlanetCameraController")
		.superType<CameraController>()
		.superType<LatLonSettable>()
		.superType<Pitchable>()
		.superType<EntityTargeter>()
		.superType<Dollyable>();
}

PlanetCameraController::PlanetCameraController(sim::Entity* camera, World* world, const Params& params) :
	CameraController(camera),
	EntityTargeter(world),
	mParams(params)
{
	setPitch(skybolt::math::halfPiF());
}

void PlanetCameraController::updateTimeStep(const UpdateTimeStepArgs& args)
{
	if (Entity* target = getTarget(); target)
	{
		auto position = getPosition(*target);
		auto orientation = getOrientation(*target);
		auto planet = target->getFirstComponent<PlanetComponent>();
		if (!position || !orientation || !planet)
		{
			return;
		}

		float yawDelta = msYawRate * mInput.yawRate * args.wallTimeStep;
		float pitchDelta = msPitchRate * mInput.tiltRate * args.wallTimeStep;
		float zoomDelta = (mInput.zoomRate + mInput.forwardSpeed) * args.wallTimeStep * msZoomRate;
		mDollyFactor = skybolt::math::clamp(mDollyFactor + zoomDelta, 0.0, 1.0);
		float maxDistance = mParams.maxDistOnRadius * planet->radius;

		// Zoom control
		float distFromSurface;
		if (maxDistance > (float)planet->radius)
		{
			float exponent = log(maxDistance - (float)planet->radius);
			distFromSurface = exp(exponent * (1 - mDollyFactor));
		}
		else
		{
			distFromSurface = 0;
		}

		// Orientation control
		if (mInput.modifier1Pressed)
		{
			mPitch -= (double)pitchDelta * pitchControlSensitivity;
			mPitch = skybolt::math::clamp<float>((float)mPitch, 0, skybolt::math::halfPiF());
		}
		else
		{
			const float fovY = mCameraComponent->getState().fovY;
			float fovHeightAtPlanetSurfaceInMeters = 2.0f * std::tan(fovY * 0.5f) * distFromSurface;
			float planetSurfaceVisibleVerticalArcInRadians = fovHeightAtPlanetSurfaceInMeters / planet->radius;

			mLatLon.lon -= planetSurfaceVisibleVerticalArcInRadians * yawDelta / std::max(0.1, std::cos(mLatLon.lat));
			mLatLon.lat -= planetSurfaceVisibleVerticalArcInRadians * pitchDelta;
			mLatLon.lat = skybolt::math::clamp<double>(mLatLon.lat, -skybolt::math::halfPiD(), skybolt::math::halfPiD());
		}

		Quaternion orientationRelPlanet = *orientation * latLonToGeocentricLtpOrientation(mLatLon) * glm::angleAxis(skybolt::math::halfPiD(), Vector3(0, -1, 0));

		mNodeComponent->setOrientation(orientationRelPlanet * glm::angleAxis(skybolt::math::halfPiD() - mPitch, Vector3(0, 1, 0)));

		Vector3 surfacePosition = *position + orientationRelPlanet * Vector3(-planet->radius, 0, 0);

		// Derive camera position
		mNodeComponent->setPosition(surfacePosition + mNodeComponent->getOrientation() * Vector3(-distFromSurface, 0, 0));
	}
}

} // namespace skybolt::sim