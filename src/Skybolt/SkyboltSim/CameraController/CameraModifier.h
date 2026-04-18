/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Chrono.h"
#include "SkyboltSim/Components/CameraComponent.h"

namespace skybolt::sim {

class CameraModifier
{
public:
	~CameraModifier() = default;
	
	virtual const char* getTypeName() const = 0;

	struct State
	{
		Vector3 position;
		Quaternion orientation;
		CameraState cameraState;
	};

	virtual void update(State& state, SecondsD time, SecondsD dt) = 0;
	virtual void reset() {}
};

} // namespace skybolt::sim