/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"

#include <osg/Camera>
#include <osg/Group>

namespace skybolt {
namespace vis {

enum class RenderOperationOrder
{
	PrepareScene,
	PrecomputeAtmosphere,
	EnvironmentMap,
	WaterMaterial,
	ShadowMap,
	Clouds,
	MainPass,
	FinalComposite,
	Hud
};

} // namespace vis
} // namespace skybolt
