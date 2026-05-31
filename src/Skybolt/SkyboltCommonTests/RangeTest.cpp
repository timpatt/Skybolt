#include <catch2/catch.hpp>
#include <SkyboltCommon/Range.h>

using namespace skybolt;

TEST_CASE("RangeInclusive<int> basic properties")
{
    IntRangeInclusive r(3, 5);
    REQUIRE(!r.isEmpty());
    REQUIRE(r.size() == 3);
    REQUIRE(r.contains(3));
    REQUIRE(r.contains(4));
    REQUIRE(r.contains(5));
    REQUIRE(!r.contains(2));
    REQUIRE(!r.contains(6));
}

TEST_CASE("RangeInclusive<int> empty and equality")
{
    IntRangeInclusive r1(5, 3);
    REQUIRE(r1.isEmpty());
    REQUIRE(r1.size() == 0);
    IntRangeInclusive r2(3, 5);
    IntRangeInclusive r3(3, 5);
    REQUIRE(r2 == r3);
    REQUIRE(!(r1 == r2));
}

TEST_CASE("RangeInclusive<double> properties")
{
    DoubleRangeInclusive r(1.0, 2.0);
    REQUIRE(!r.isEmpty());
    REQUIRE(r.size() == Approx(1.0));
    REQUIRE(r.contains(1.0));
    REQUIRE(r.contains(2.0));
    REQUIRE(!r.contains(2.1));
}

TEST_CASE("RangeClosedOpen<int> basic properties")
{
    IntRangeClosedOpen r(3, 6);
    REQUIRE(!r.isEmpty());
    REQUIRE(r.size() == 3);
    REQUIRE(r.contains(3));
    REQUIRE(r.contains(5));
    REQUIRE(!r.contains(6));
}

TEST_CASE("RangeClosedOpen<int> empty and equality")
{
    IntRangeClosedOpen r1(5, 3);
    REQUIRE(r1.isEmpty());
    REQUIRE(r1.size() == -2);
    IntRangeClosedOpen r2(3, 6);
    IntRangeClosedOpen r3(3, 6);
    REQUIRE(r2 == r3);
    REQUIRE(!(r1 == r2));
}
