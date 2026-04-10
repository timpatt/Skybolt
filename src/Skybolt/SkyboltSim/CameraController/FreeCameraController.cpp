/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FreeCameraController.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/CameraComponent.h"
#include "SkyboltSim/Spatial/Geocentric.h"

#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt::sim {

SKYBOLT_REFLECT(FreeCameraController) {
	registry.type<FreeCameraController>("FreeCameraController")
		.superType<CameraController>()
		.superType<Pitchable>()
		.superType<Yawable>()
		.superType<Zoomable>()
		.property("minFovY", &FreeCameraController::minFovY, {{PropertyMetadataNames::units, Units::Radians}})
		.property("maxFovY", &FreeCameraController::maxFovY, { {PropertyMetadataNames::units, Units::Radians}});
}


FreeCameraController::FreeCameraController(Entity* camera) :
	CameraController(camera)
{
}

void FreeCameraController::update(SecondsD dt)
{
	mYaw += mInput.yawRate * dt;
	mPitch += mInput.tiltRate * dt;

	if (mInput.zoomRate != 0)
	{
		setZoom(getZoom() + mInput.zoomRate * dt);
	}
	mCameraComponent->getState().fovY = std::clamp(mCameraComponent->getState().fovY, float(minFovY), float(maxFovY));
	
	double speed = mInput.modifier1Pressed ? 10000.0 : (mInput.modifier2Pressed ? 100.0 : 1000.0);
	Vector3 vel = Vector3(mInput.forwardSpeed, mInput.rightSpeed, 0.0f) * speed;

	sim::Matrix3 ltpOrientation = geocentricToLtpOrientation(mNodeComponent->getPosition());
	sim::Quaternion ltpOrientationQuat(ltpOrientation);

	Quaternion orientation = ltpOrientationQuat * glm::angleAxis(mYaw, Vector3(0, 0, 1)) * glm::angleAxis(mPitch, Vector3(0, 1, 0));
	mNodeComponent->setOrientation(orientation);
	Vector3 position = mNodeComponent->getPosition() + orientation * vel * (double)dt;
	mNodeComponent->setPosition(position);
}

double FreeCameraController::getZoom() const
{
	// Minimum FOV when zoom is 1, maximum FOV when zoom is 0
	double zoom = (mCameraComponent->getState().fovY - maxFovY) / (minFovY - maxFovY);
	return std::clamp(zoom, 0.0, 1.0);
}

void FreeCameraController::setZoom(double zoom)
{
	zoom = skybolt::math::clamp(zoom, 0.0, 1.0);
	mCameraComponent->getState().fovY = skybolt::math::lerp(maxFovY, minFovY, zoom);
}

} // namespace skybolt::sim