/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Event.h>
#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltEngine/CameraInputSystem.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/System/System.h>

class ViewportInputSystem : public skybolt::sim::SystemT<ViewportInputSystem>, public skybolt::EventListener
{
public:
	ViewportInputSystem(const skybolt::InputPlatformPtr& inputPlatform, const skybolt::CameraInputSystemPtr& cameraInputSystem, skybolt::NonNullPtr<skybolt::EngineRoot> engineRoot);
	~ViewportInputSystem() override;

	void setViewportHeight(int heightPixels);

	void onEvent(const skybolt::Event& event) override;

private:
	skybolt::InputPlatformPtr mInputPlatform;
	skybolt::CameraInputSystemPtr mCameraInputSystem;
	skybolt::NonNullPtr<skybolt::EngineRoot> mEngineRoot;
};