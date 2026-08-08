/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltCommon/TypeIdentifiable.h>

using namespace skybolt;

namespace {

class TestBase : public TypeIdentifiable
{
};

class TestDerivedA : public TestBase
{
	SKYBOLT_TYPE_IDENTIFIABLE
};

class TestDerivedB : public TestBase
{
	SKYBOLT_TYPE_IDENTIFIABLE
};

class TestDerivedFromA : public TestDerivedA
{
	SKYBOLT_TYPE_IDENTIFIABLE
};

} // namespace

TEST_CASE("TypeIdentifiable reports matching concrete type")
{
	TestDerivedA value;

	CHECK(value.is<TestDerivedA>());
	CHECK(!value.is<TestDerivedB>());
}

TEST_CASE("TypeIdentifiable supports querying through base reference")
{
	TestDerivedA value;
	const TestBase& base = value;

	CHECK(base.is<TestDerivedA>());
	CHECK(!base.is<TestDerivedB>());
}

TEST_CASE("TypeIdentifiable::as returns pointer for matching type")
{
	TestDerivedA value;
	const TestBase& base = value;

	CHECK(base.as<TestDerivedA>() == &value);
	CHECK(base.as<TestDerivedB>() == nullptr);
}

TEST_CASE("TypeIdentifiable matches exact concrete type only")
{
	TestDerivedFromA value;
	const TestBase& base = value;

	CHECK(base.is<TestDerivedFromA>());
	CHECK(!base.is<TestDerivedA>());
	CHECK(base.as<TestDerivedFromA>() == &value);
	CHECK(base.as<TestDerivedA>() == nullptr);
}
