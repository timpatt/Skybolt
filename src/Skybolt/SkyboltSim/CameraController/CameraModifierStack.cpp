/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CameraModifierStack.h"
#include "CameraModifier.h"
#include "SkyboltSim/Serialization/Serialization.h"
#include <SkyboltCommon/Logging/Logging.h>

namespace skybolt::sim {

SKYBOLT_REFLECT(CameraModifierStack) {
	registry.type<CameraModifierStack>("CameraModifierStack")
		.property("modifiers", &CameraModifierStack::modifiers);
}

nlohmann::json CameraModifierStack::toJson(refl::TypeRegistry& typeRegistry) const
{
	nlohmann::json json;
	for (const auto& modifier : modifiers)
	{
		nlohmann::json modifierJson = writeReflectedObject(typeRegistry, refl::makeRefInstance(typeRegistry, modifier.get()));
		modifierJson["type"] = modifier->getTypeName();
		json.push_back(modifierJson);
	}
	return json;
}

void CameraModifierStack::fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& modifiersJson)
{
	modifiers = readCameraModifiersFromJson(*mFactories, modifiersJson);
}

void applyUpdate(const CameraModifierStack& stack, CameraModifier::State& state, SecondsD time, SecondsD dt)
{
	for (const auto& modifier : stack.modifiers)
	{
		modifier->update(state, time, dt);
	}
}

void resetState(const CameraModifierStack& stack)
{
	for (const auto& modifier : stack.modifiers)
	{
		modifier->reset();
	}
}

std::vector<CameraModifierPtr> readCameraModifiersFromJson(const CameraModifierFactoryRegistry& factories, const nlohmann::json& json)
{
	if (json.is_null())
	{
		return {};
	}

	std::vector<CameraModifierPtr> modifiers;
	for (auto item : json)
	{
		const std::string& type = item.at("type");
		auto factoryIt = factories.find(type);
		if (factoryIt != factories.end())
		{
			auto modifier = factoryIt->second(item);
			if (modifier)
			{
				modifiers.push_back(std::move(modifier));
			}
		}
		else
		{
			SKYBOLT_LOG(error) << "Unknown camera modifier type: " << type;
		}
	}
	return modifiers;
}

} // namespace skybolt::sim