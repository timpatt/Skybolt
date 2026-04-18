/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "CameraModifier.h"
#include <SkyboltCommon/Registry.h>
#include "SkyboltSim/Chrono.h"
#include "SkyboltSim/SkyboltSimFwd.h"

#include <functional>
#include <nlohmann/json.hpp>
#include <vector>

namespace skybolt::sim {

using CameraModifierFactory = std::function<std::unique_ptr<CameraModifier>(const nlohmann::json& json)>;
using CameraModifierFactoryRegistry = RegistryT<std::string, CameraModifierFactory>;
using CameraModifierFactoryRegistryPtr = std::shared_ptr<CameraModifierFactoryRegistry>;

struct CameraModifierStack
{
	std::vector<CameraModifierPtr> modifiers;

	CameraModifierStack(CameraModifierFactoryRegistryPtr factories) : mFactories(std::move(factories)) {}

	nlohmann::json toJson(refl::TypeRegistry& typeRegistry) const;
	void fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j);

private:
	CameraModifierFactoryRegistryPtr mFactories;
};

void applyUpdate(const CameraModifierStack& stack, CameraModifier::State& state, SecondsD time, SecondsD dt);
void resetState(const CameraModifierStack& stack);


std::vector<CameraModifierPtr> readCameraModifiersFromJson(const CameraModifierFactoryRegistry& factories, const nlohmann::json& json);

SKYBOLT_REFLECT_EXTERN(CameraModifierStack)

} // namespace skybolt::sim