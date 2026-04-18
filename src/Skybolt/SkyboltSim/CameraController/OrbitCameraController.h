/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "CameraController.h"
#include "CameraModifierStack.h"
#include "Dollyable.h"
#include "Pitchable.h"
#include "Yawable.h"
#include "EntityTargeter.h"
#include "SkyboltSim/Serialization/Serialization.h"

#include <optional>

namespace skybolt {
namespace sim {

class OrbitCameraController : public CameraController, public CameraModifierStack, public ExplicitSerialization, public Pitchable, public EntityTargeter, public Yawable, public Dollyable
{
public:
	struct Params
	{
		Params(double _minDist, double _maxDist, double _zoomRate = 0.5f) :
			minDist(_minDist), maxDist(_maxDist), zoomRate(_zoomRate) {}

		double minDist;
		double maxDist;
		double zoomRate;
		Vector3 orientationLagTimeConstant = Vector3(0);
	};

	OrbitCameraController(Entity* camera, World* world, const Params& params, const CameraModifierFactoryRegistryPtr& cameraModifierFactories);

	double lagTimeConstant = 0;
	bool lockOrientationToTarget = true;
	Vector3 targetPositionOffset = {};

public:
	// CameraController interface
	void setActive(bool active) override;
	void updatePostDynamicsSubstep(SecondsD simTime, SecondsD dtSubstep) override;
	void updateTimeStep(const UpdateTimeStepArgs& args) override;
	void setInput(const Input& input) override { mInput = input; }

public: // ExplicitSerialization interface
	nlohmann::json toJson(refl::TypeRegistry& typeRegistry) const;
	void fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j);

private:
	void resetFiltering();

	Params mParams;
	std::optional<Quaternion> mSmoothedTargetOrientation;
	EntityId mPrevTargetId = nullEntityId();
	Input mInput = Input::zero();

	static const float msYawRate;
	static const float msPitchRate;
	static const float msZoomRate;
};

SKYBOLT_REFLECT_EXTERN(OrbitCameraController)

} // namespace sim
} // namespace skybolt