/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "CameraControllerSelector.h"
#include "EntityTargeter.h"

#include <assert.h>

namespace skybolt {
namespace sim {

SKYBOLT_REFLECT(CameraControllerSelector) {
	registry.type<CameraControllerSelector>("CameraControllerSelector")
		.property("selectedController", &CameraControllerSelector::getSelectedControllerName, &CameraControllerSelector::selectController)
		.property("controllers", &CameraControllerSelector::getControllers, &CameraControllerSelector::setControllers);
}

CameraControllerSelector::CameraControllerSelector(const ControllersMap& controllers) :
	mControllers(controllers)
{
	assert(!mControllers.empty());
	selectController(mControllers.begin()->first);
}

void CameraControllerSelector::selectController(const std::string& name)
{
	if (name != mSelectedName)
	{
		CameraController* selectedController = getSelectedController();
		if (selectedController)
		{
			selectedController->setActive(false);
		}
		mSelectedName = name;

		selectedController = getSelectedController();
		if (selectedController)
		{
			selectedController->setActive(true);
		}

		controllerSelected(name);
	}
}

CameraController* CameraControllerSelector::getSelectedController() const
{
	if (mSelectedName.empty())
	{
		return nullptr;
	}

	auto i = mControllers.find(mSelectedName);
	return (i != mControllers.end()) ? i->second.get() : nullptr;
}

void CameraControllerSelector::setControllers(const ControllersMap& controllers)
{
	mControllers = controllers;
}

void CameraControllerSelector::addController(const std::string& name, const CameraControllerPtr& controller)
{
	mControllers[name] = controller;
	if (auto targeter = dynamic_cast<EntityTargeter*>(controller.get()); targeter)
	{
		targeter->setTargetId(getTargetId());
	}
}

void CameraControllerSelector::setTargetId(const EntityId& targetId)
{
	for (const auto& item : mControllers)
	{
		if (auto targeter = dynamic_cast<EntityTargeter*>(item.second.get()); targeter)
		{
			targeter->setTargetId(targetId);
		}
	}
}

EntityId CameraControllerSelector::getTargetId() const
{
	if (CameraController* selectedController = getSelectedController(); selectedController)
	{
		if (auto targeter = dynamic_cast<EntityTargeter*>(selectedController); targeter)
		{
			return targeter->getTargetId();
		}
	}
	return sim::nullEntityId();
}

} // namespace sim
} // namespace skybolt