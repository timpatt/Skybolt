/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <source_location>
#include <string_view>

namespace skybolt {

using TypeId = std::string_view;

class TypeIdentifiable
{
public:
	virtual ~TypeIdentifiable() = default;

	template <class TypeIdentifiableT>
	bool is() const
	{
		return getTypeId() == TypeIdentifiableT::staticTypeId();
	}

	template <class TypeIdentifiableT>
	const TypeIdentifiableT* as() const
	{
		return is<TypeIdentifiableT>() ? static_cast<const TypeIdentifiableT*>(this) : nullptr;
	}

	virtual TypeId getTypeId() const = 0;
};

// TODO FIXME: source_location will produce mis-matched TypeIds across different compilers (i.e. MSVC, GCC, Clang).
// This isn't a problem for the current use case as we require all code to be compiled with the same compiler,
// but we should eventually replace this with C++26 reflection once it's available.
#define SKYBOLT_TYPE_IDENTIFIABLE \
public: \
	static constexpr TypeId staticTypeId() { return std::source_location::current().function_name(); } \
	TypeId getTypeId() const override { return staticTypeId(); }

} // namespace skybolt