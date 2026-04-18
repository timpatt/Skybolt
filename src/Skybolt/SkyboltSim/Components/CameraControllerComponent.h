/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/CameraController/CameraControllerSelector.h"
#include "SkyboltSim/Serialization/Serialization.h"

#include <map>

namespace skybolt {
namespace sim {

class CameraControllerComponent : public CameraControllerSelector, public Component, public ExplicitSerialization
{
public:
	CameraControllerComponent(const ControllersMap& controllers);

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::PostDynamicsSubStep, postDynamicsSubStep)
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::Attachments, updateAttachments)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void setSimTime(SecondsD simTime) override { mSimTime = simTime; }
	void advanceSimTime(SecondsD newTime, SecondsD dt) override;
	void advanceWallTime(SecondsD newTime, SecondsD dt) override;
	void postDynamicsSubStep();
	void updateAttachments();

	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(CameraControllerComponent), typeid(CameraControllerSelector)};
	}

public: // ExplicitSerialization interface
	nlohmann::json toJson(refl::TypeRegistry& typeRegistry) const override;
	void fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j) override;

private:
	SecondsD mSimTime = 0;
	SecondsD mSimTimeSinceLastDynamicsStep = 0;
	SecondsD mSimTimeSinceLastAttachmentsUpdate = 0;
	SecondsD mWallDt = 0;
};

SKYBOLT_REFLECT_EXTERN(CameraControllerComponent)

} // namespace sim
} // namespace skybolt