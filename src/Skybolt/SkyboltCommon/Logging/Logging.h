/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#ifdef USE_BOOST_LOG
	#include <boost/log/trivial.hpp>
	#define SKYBOLT_LOG(level) BOOST_LOG_TRIVIAL(level)
#else
	#include <iostream>

	class SkyboltLogLineWriter
	{
	public:
		SkyboltLogLineWriter(const char* severity) {
			std::cout << "[" << severity << "] ";
		}
		
		~SkyboltLogLineWriter() {
			std::cout << std::endl;
		}

		template<typename T>
		SkyboltLogLineWriter& operator<<(const T& value) {
			std::cout << value;
			return *this;
		}
	};	

	#define SKYBOLT_LOG(level) SkyboltLogLineWriter(#level)
#endif
