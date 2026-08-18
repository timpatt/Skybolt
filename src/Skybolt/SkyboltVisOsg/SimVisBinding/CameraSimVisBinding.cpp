/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "CameraSimVisBinding.h"
#include <SkyboltVisOsg/OsgMathHelpers.h>
#include <SkyboltSim/Spatial/GeocentricToNedConverter.h>
#include <SkyboltVisOsg/Camera.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/CameraComponent.h>

namespace skybolt {

CameraSimVisBinding::CameraSimVisBinding(const sim::Entity* simCamera, const vis::CameraPtr& visCamera) :
	mEntity(simCamera),
	mCamera(visCamera),
	mCameraComponent(simCamera->getFirstComponent<sim::CameraComponent>().get())
{
	assert(mEntity);
	assert(mCamera);
	assert(mCameraComponent);
}

void CameraSimVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	glm::dquat q = converter.convert(*sim::getOrientation(*mEntity));
	mCamera->setOrientation(math::toOsgQuat(q));

	glm::dvec3 p = converter.convertPosition(*sim::getPosition(*mEntity));
	mCamera->setPosition(math::toOsgVec3d(p));
	
	const sim::CameraState& state = mCameraComponent->getState();
	
	mCamera->setFovY(state.fovY);
	mCamera->setNearClipDistance(state.nearClipDistance);
	mCamera->setFarClipDistance(state.farClipDistance);
}

vis::CameraPtr getVisCamera(const sim::Entity& camera)
{
	return static_cast<const CameraSimVisBinding&>(*camera.getFirstComponent<SimVisBindingsComponent>()->bindings.front()).getCamera();
}

} // namespace skybolt