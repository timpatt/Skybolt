/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <sstream>
#include <string>

namespace skybolt {

//! Severity levels used for logging. Names match boost::log::trivial::severity_level
//! so that the SKYBOLT_LOG macro can be used the same way regardless of backend.
enum class LogLevel
{
	trace,
	debug,
	info,
	warning,
	error,
	fatal
};

class Logger
{
public:
	virtual ~Logger() = default;

	virtual void log(LogLevel level, const std::string& message) = 0;
};

//! Creates a Logger implementation backed by boost::log (if USE_BOOST_LOG is defined), or a simple fallback logger otherwise.
std::shared_ptr<Logger> createDefaultLogger();

//! Returns the currently active global logger, creating a default one and logs warning if not already set.
std::shared_ptr<Logger> getLogger();

//! Sets the global logger instance used by the SKYBOLT_LOG macro.
void setLogger(std::shared_ptr<Logger> logger);

//! Helper class that accumulates a message via operator<< and forwards it to the active
//! logger when the object goes out of scope. Used by the SKYBOLT_LOG macro.
class LogStreamer
{
public:
	explicit LogStreamer(LogLevel level) : mLevel(level) {}

	~LogStreamer()
	{
		getLogger()->log(mLevel, mStream.str());
	}

	template <typename T>
	LogStreamer& operator<<(const T& value)
	{
		mStream << value;
		return *this;
	}

private:
	LogLevel mLevel;
	std::ostringstream mStream;
};

#define SKYBOLT_LOG(level) skybolt::LogStreamer(skybolt::LogLevel::level)

} // namespace skybolt