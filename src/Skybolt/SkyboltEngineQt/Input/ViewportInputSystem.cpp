/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportInputSystem.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Input/InputPlatform.h>

using namespace skybolt;

ViewportInputSystem::ViewportInputSystem(const skybolt::InputPlatformPtr& inputPlatform, const skybolt::CameraInputSystemPtr& cameraInputSystem, NonNullPtr<EngineRoot> engineRoot) :
	mInputPlatform(inputPlatform),
	mCameraInputSystem(cameraInputSystem),
	mEngineRoot(engineRoot)
{
	assert(mInputPlatform);
	assert(mCameraInputSystem);
	assert(mEngineRoot);

	// Start with input disabled. Input will only be enabled on mouse down event in the viewport.
	mCameraInputSystem->setMouseEnabled(false);
	mCameraInputSystem->setKeyboardEnabled(false);

	mInputPlatform->getEventEmitter()->addEventListener<MouseEvent>(this);
}

ViewportInputSystem::~ViewportInputSystem()
{
	mInputPlatform->getEventEmitter()->removeEventListener(this);
}

void ViewportInputSystem::onEvent(const Event& event)
{
	if (const auto& mouseEvent = dynamic_cast<const MouseEvent*>(&event))
	{
		// Disable input when mouse is released.
		if (mouseEvent->type == MouseEvent::Type::Pressed)
		{
			mCameraInputSystem->setMouseEnabled(true);
			mCameraInputSystem->setKeyboardEnabled(true);
		}
		else if (mouseEvent->type == MouseEvent::Type::Released)
		{
			mCameraInputSystem->setMouseEnabled(false);
			mCameraInputSystem->setKeyboardEnabled(false);
		}
	}
}

void ViewportInputSystem::setViewportHeight(int heightPixels)
{
	configure(*mCameraInputSystem, heightPixels, mEngineRoot->engineSettings);
}