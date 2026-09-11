/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Plugin.h"
#include <SkyboltCommon/Logging/Logging.h>

namespace skybolt {

Plugin::Plugin(const PluginConfig& config)
{
	// Allow plugins to use the same logger as the main application module, which plugins otherwise
	// might not have access to since the logger is stored as a static variable inside the application module.
	setLogger(config.logger);
}

} // namespace skybolt