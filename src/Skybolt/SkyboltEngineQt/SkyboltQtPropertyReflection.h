/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltWidgets/Property/QtPropertyReflection.h>
#include <SkyboltCommon/Units.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/Spatial/LatLon.h>

Q_DECLARE_METATYPE(skybolt::sim::LatLon)
Q_DECLARE_METATYPE(skybolt::sim::CameraControllerSelector::ControllersMap)
Q_DECLARE_METATYPE(skybolt::sim::CameraModifierPtr)

skybolt::ReflValueTranslatorMap createSkyboltReflValueTranslators(skybolt::refl::TypeRegistry& typeRegistry);