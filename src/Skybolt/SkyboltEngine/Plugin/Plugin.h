/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <SkyboltCommon/Registry.h>
#include <SkyboltCommon/SkyboltCommonFwd.h>
#include <SkyboltCommon/TypedItemContainer.h>

#include <boost/dll/import.hpp>
#include <set>

namespace skybolt {

struct PluginConfig
{
	EngineRoot* engineRoot; //!< Never null. Lifetime is guaranteed to exceed plugin lifetime.

	//! Never null. Allows plugins use the logger as the main application module, which plugins otherwise
	//! might not have access to since the logger is a singleton stored as a static variable inside the application module.
	LoggerPtr logger;
};

class BOOST_SYMBOL_VISIBLE Plugin
{
public:
	Plugin(const PluginConfig& config);
	virtual ~Plugin() = default;

	//! Name of a symbol with `CreatePluginFunctionT` signature.
	static std::string factorySymbolName() { return "createEnginePlugin"; }
	template <class PluginT, class PluginConfigT>
	using CreatePluginFunctionT = std::shared_ptr<PluginT>(const PluginConfigT&);

	//! Name of a symbol with `GetCategoriesFunction` signature.
	//! Category names describe what kind of plugin this is.
	//! Category names are defined in the `skybolt::plugin_category` namespace.
	static std::string categoriesSymbolName() { return "getCategories"; }
	using Categories = std::set<std::string>;
	using GetCategoriesFunction = Categories();
};

} // namespace skybolt