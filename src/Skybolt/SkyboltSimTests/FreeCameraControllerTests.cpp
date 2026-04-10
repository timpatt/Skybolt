/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TestHelpers.h"
#include <catch2/catch.hpp>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/CameraController/FreeCameraController.h>
#include <SkyboltSim/Entity.h>

using namespace skybolt;
using namespace skybolt::sim;

TEST_CASE("FreeCameraController FOV and zoom level are synchronized")
{
	sim::Entity entity(sim::EntityId(1,2));

	auto camera = std::make_shared<sim::CameraComponent>();
	entity.addComponent(camera);

	auto node = std::make_shared<sim::Node>();
	entity.addComponent(node);

	FreeCameraController controller(&entity);

	// Test FOV updates when zoom level changes
	CHECK(controller.minFovY > 0);
	CHECK(controller.maxFovY > controller.minFovY);

	controller.setZoom(0);
	CHECK(camera->getFovY() == Approx(controller.maxFovY));

	controller.setZoom(1);
	CHECK(camera->getFovY() == Approx(controller.minFovY));

	// Test zoom level changes when FOV changes
	camera->setFovY(controller.minFovY);
	CHECK(controller.getZoom() == Approx(1));

	camera->setFovY(controller.maxFovY);
	CHECK(controller.getZoom() == Approx(0));

	// Test zoom level increases with positive input zoom rate
	controller.setZoom(0);

	CameraController::Input input{};
	input.zoomRate = 0.2;
	controller.setInput(input);
	controller.update(/* dt */ 1);

	CHECK(controller.getZoom() == Approx(0.2));
}
