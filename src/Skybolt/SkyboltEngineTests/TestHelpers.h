/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <glm/glm.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

inline void check(const glm::dvec2& a, const glm::dvec2& b, double eps)
{
	CHECK(a.x == Catch::Approx(b.x).margin(eps));
	CHECK(a.y == Catch::Approx(b.y).margin(eps));
}

inline void check(const glm::dvec3& a, const glm::dvec3& b, double eps)
{
	CHECK(a.x == Catch::Approx(b.x).margin(eps));
	CHECK(a.y == Catch::Approx(b.y).margin(eps));
	CHECK(a.z == Catch::Approx(b.z).margin(eps));
}
