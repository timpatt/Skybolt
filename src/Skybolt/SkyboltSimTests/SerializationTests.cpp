/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <SkyboltSim/Serialization/Serialization.h>
#include <catch2/catch.hpp>

#include <assert.h>
#include <optional>

using namespace skybolt;
using namespace skybolt::sim;

struct TestNestedObject
{
public:
	TestNestedObject() {}
	TestNestedObject(int intProperty) : intProperty(intProperty) {}

	int intProperty;
};

struct TestObject
{
	int intProperty;
	TestNestedObject nestedObjectProperty;
	std::optional<int> optionalIntProperty1;
	std::optional<int> optionalIntProperty2;
};

SKYBOLT_REFLECT(TestNestedObject) {
	registry.type<TestNestedObject>("TestNestedObject")
		.property("intProperty", &TestNestedObject::intProperty);
}

SKYBOLT_REFLECT(TestObject) {
	registry.type<TestObject>("TestObject")
		.property("intProperty", &TestObject::intProperty)
		.property("nestedObjectProperty", &TestObject::nestedObjectProperty)
		.property("optionalIntProperty1", &TestObject::optionalIntProperty1)
		.property("optionalIntProperty2", &TestObject::optionalIntProperty2);
}

TEST_CASE("Read and write to JSON")
{
	refl::TypeRegistry registry;

	// Write
	TestObject originalObject;
	originalObject.intProperty = 2;
	originalObject.nestedObjectProperty.intProperty = 3;
	// Leave originalObject.optionalIntProperty1 unset
	originalObject.optionalIntProperty2 = 123;
	nlohmann::json json = writeReflectedObject(registry, refl::makeRefInstance(registry, &originalObject));

	// Read
	TestObject readObject;
	auto instance = refl::makeRefInstance(registry, &readObject);
	readReflectedObject(registry, instance, json);

	CHECK(readObject.intProperty == originalObject.intProperty);
	CHECK(readObject.nestedObjectProperty.intProperty == originalObject.nestedObjectProperty.intProperty);
	CHECK(!readObject.optionalIntProperty1);
	REQUIRE(readObject.optionalIntProperty2);
	CHECK(readObject.optionalIntProperty2 == 123);
}

struct TestBaseObject
{
public:
	virtual ~TestBaseObject() = default;
};

struct TestDerivedObject : public TestBaseObject
{
public:
	TestDerivedObject() {}
	TestDerivedObject(float floatProperty) : floatProperty(floatProperty) {}
	~TestDerivedObject() override = default;

	float floatProperty;
};

SKYBOLT_REFLECT(TestDerivedObject) {
	registry.type<TestDerivedObject>("TestDerivedObject")
		.superType<TestBaseObject>()
		.property("floatProperty", &TestDerivedObject::floatProperty);
}

TEST_CASE("Read and write polymorphic type to JSON")
{
	refl::TypeRegistry registry;

	// Write
	TestDerivedObject derivedObject;
	derivedObject.floatProperty = 123;

	TestBaseObject& baseObject = derivedObject;
	nlohmann::json json = writeReflectedObject(registry, refl::makeRefInstance(registry, &baseObject)); // test writing the base object reference

	// Read
	TestDerivedObject readObject;
	auto instance = refl::makeRefInstance(registry, &readObject);
	readReflectedObject(registry, instance, json);

	CHECK(readObject.floatProperty == derivedObject.floatProperty);
}

struct TestObjectWithSerializationMethods : public ExplicitSerialization
{
public:
	nlohmann::json toJson(refl::TypeRegistry& typeRegistry) const
	{
		return data;
	};

	void fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j)
	{
		data = j.get<int>();
	}

	int data = 0;
};

SKYBOLT_REFLECT(TestObjectWithSerializationMethods) {
	registry.type<TestObjectWithSerializationMethods>("TestObjectWithSerializationMethods")
		.superType<ExplicitSerialization>();
}

TEST_CASE("Use explicit to/from json methods if an object provides them")
{
	refl::TypeRegistry registry;

	// Write
	TestObjectWithSerializationMethods writeObject;
	writeObject.data = 123;
	nlohmann::json json = writeReflectedObject(registry, refl::makeRefInstance(registry, &writeObject));

	// Read
	TestObjectWithSerializationMethods readObject;
	auto instance = refl::makeRefInstance(registry, &readObject);
	readReflectedObject(registry, instance, json);
	CHECK(readObject.data == 123);
}

struct TestObjectWithVectorProperty
{
	std::vector<int> intVectorProperty;
	std::vector<TestNestedObject> objectVectorProperty;
};

SKYBOLT_REFLECT(TestObjectWithVectorProperty) {
	registry.type<TestObjectWithVectorProperty>("TestObjectWithVectorProperty")
		.property("intVectorProperty", &TestObjectWithVectorProperty::intVectorProperty)
		.property("objectVectorProperty", &TestObjectWithVectorProperty::objectVectorProperty);
}

TEST_CASE("Read and write vector of primitives to JSON")
{
	refl::TypeRegistry registry;

	// Write
	TestObjectWithVectorProperty originalObject;
	originalObject.intVectorProperty = { 10, 20, 30 };
	nlohmann::json json = writeReflectedObject(registry, refl::makeRefInstance(registry, &originalObject));

	// Read
	TestObjectWithVectorProperty readObject;
	auto instance = refl::makeRefInstance(registry, &readObject);
	readReflectedObject(registry, instance, json);

	REQUIRE(readObject.intVectorProperty.size() == 3);
	CHECK(readObject.intVectorProperty[0] == 10);
	CHECK(readObject.intVectorProperty[1] == 20);
	CHECK(readObject.intVectorProperty[2] == 30);
}

TEST_CASE("Read and write empty vector to JSON")
{
	refl::TypeRegistry registry;

	// Write
	TestObjectWithVectorProperty originalObject;
	// Leave intVectorProperty empty
	nlohmann::json json = writeReflectedObject(registry, refl::makeRefInstance(registry, &originalObject));

	// Read
	TestObjectWithVectorProperty readObject;
	readObject.intVectorProperty = { 99 }; // pre-populate to ensure it gets cleared
	auto instance = refl::makeRefInstance(registry, &readObject);
	readReflectedObject(registry, instance, json);

	CHECK(readObject.intVectorProperty.empty());
}

TEST_CASE("Read and write vector of objects to JSON")
{
	refl::TypeRegistry registry;

	// Write
	TestObjectWithVectorProperty originalObject;
	originalObject.objectVectorProperty = { TestNestedObject(5), TestNestedObject(7) };
	nlohmann::json json = writeReflectedObject(registry, refl::makeRefInstance(registry, &originalObject));

	// Read
	TestObjectWithVectorProperty readObject;
	auto instance = refl::makeRefInstance(registry, &readObject);
	readReflectedObject(registry, instance, json);

	REQUIRE(readObject.objectVectorProperty.size() == 2);
	CHECK(readObject.objectVectorProperty[0].intProperty == 5);
	CHECK(readObject.objectVectorProperty[1].intProperty == 7);
}

TEST_CASE("Read and write present optional to JSON")
{
	refl::TypeRegistry registry;

	// Write
	std::optional<int> originalValue = 42;
	nlohmann::json json = writeReflectedObject(registry, refl::makeRefInstance(registry, &originalValue));

	// Read
	std::optional<int> readValue;
	auto instance = refl::makeRefInstance(registry, &readValue);
	readReflectedObject(registry, instance, json);

	REQUIRE(readValue.has_value());
	CHECK(*readValue == 42);
}

TEST_CASE("Read and write empty optional to JSON")
{
	refl::TypeRegistry registry;

	// Write
	std::optional<int> originalValue; // unset
	nlohmann::json json = writeReflectedObject(registry, refl::makeRefInstance(registry, &originalValue));

	// Read into a pre-populated optional to ensure it gets cleared
	std::optional<int> readValue = 99;
	auto instance = refl::makeRefInstance(registry, &readValue);
	readReflectedObject(registry, instance, json);

	CHECK(!readValue.has_value());
}