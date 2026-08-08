/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "System.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include <SkyboltCommon/Exception.h>
#include <vector>

namespace skybolt {
namespace sim {

typedef std::vector<SystemPtr> SystemRegistry;
typedef std::shared_ptr<SystemRegistry> SystemRegistryPtr;

//! @returns system of type T, or null if none found
template <typename T>
std::shared_ptr<T> findSystem(const SystemRegistry& registry)
{
	for (const SystemPtr& system : registry)
	{
		if (system->is<T>())
		{
			return std::static_pointer_cast<T>(system);
		}
	}
	return nullptr;
}

template <typename T>
std::shared_ptr<T> findRequiredSystem(const SystemRegistry& registry)
{
	auto system = sim::findSystem<T>(registry);
	if (!system)
	{
		throw Exception("Could not find system: " + std::string(typeid(T).name()));
	}
	return system;
}

void resetSystemsToInitialState(const SystemRegistry& registry);


} // namespace sim
} // namespace skybolt