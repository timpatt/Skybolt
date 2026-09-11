/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Logging.h"

#include <iostream>
#include <mutex>

#ifdef USE_BOOST_LOG
#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>

namespace bl = boost::log;
#endif

namespace skybolt {

#ifdef USE_BOOST_LOG

using BoostLoggerImpl = boost::log::sources::severity_logger_mt<bl::trivial::severity_level>;

namespace {

bl::trivial::severity_level toBoostSeverity(LogLevel level)
{
	switch (level)
	{
		case LogLevel::trace:
			return bl::trivial::trace;
		case LogLevel::debug:
			return bl::trivial::debug;
		case LogLevel::info:
			return bl::trivial::info;
		case LogLevel::warning:
			return bl::trivial::warning;
		case LogLevel::error:
			return bl::trivial::error;
		case LogLevel::fatal:
		default:
			return bl::trivial::fatal;
	}
}

void addConsoleSink()
{
	using console_sink = bl::sinks::synchronous_sink<bl::sinks::text_ostream_backend>;
	boost::shared_ptr<console_sink> consoleSink = boost::make_shared<console_sink>();
	consoleSink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter()));

	// Ideally we would add a sink to our logger instance instaed of to the singleton, but the boost log interface
	// doesn't seem to allow that.
	bl::core::get()->add_sink(consoleSink);
}

} // namespace

//! Logger implementation that forwards messages to boost::log.
class BoostLogger : public Logger
{
public:
	BoostLogger()
	{
		addConsoleSink();
	}

	void log(LogLevel level, const std::string& message) override
	{
		BOOST_LOG_SEV(mLogger, toBoostSeverity(level)) << message;
	}

private:
	BoostLoggerImpl mLogger;
};

#else // !USE_BOOST_LOG

namespace {

const char* toString(LogLevel level)
{
	switch (level)
	{
		case LogLevel::trace:
			return "trace";
		case LogLevel::debug:
			return "debug";
		case LogLevel::info:
			return "info";
		case LogLevel::warning:
			return "warning";
		case LogLevel::error:
			return "error";
		case LogLevel::fatal:
		default:
			return "fatal";
	}
}

} // namespace

//! Fallback Logger implementation that writes messages to std::cout, used when boost::log is not available.
class ConsoleLogger : public Logger
{
public:
	void log(LogLevel level, const std::string& message) override
	{
		std::lock_guard<std::mutex> lock(mMutex);
		std::cout << "[" << toString(level) << "] " << message << std::endl;
	}

private:
	std::mutex mMutex;
};

#endif

std::shared_ptr<Logger> createDefaultLogger()
{
#ifdef USE_BOOST_LOG
	return std::make_shared<BoostLogger>();
#else
	return std::make_shared<ConsoleLogger>();
#endif
}

namespace {

std::shared_ptr<Logger>& loggerInstance()
{
	static std::shared_ptr<Logger> logger = createDefaultLogger();
	return logger;
}

} // namespace

std::shared_ptr<Logger> getLogger()
{
	return loggerInstance();
}

void setLogger(std::shared_ptr<Logger> logger)
{
	loggerInstance() = std::move(logger);
}

} // namespace skybolt