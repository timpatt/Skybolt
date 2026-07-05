/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/Components/ControlInputsComponent.h"
#include "SkyboltSim/Components/Rotor.h"
#include "SkyboltSim/Components/Trimmable.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include <SkyboltCommon/Math/LookupTable1D.h>
#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltCommon/NullablePtr.h>
#include <vector>

namespace skybolt {
namespace sim {

using ScalarOrCurve = std::variant<double, math::LookupTable1D>;

struct MainRotorParams
{
	double maxRpm;

	double minCollectivePitch; //!< Pitch angle of the rotor blades, in radians, when cyclic is neutral and collective is at minimum
	double maxCollectivePitch; //!< Pitch angle of the rotor blades, in radians, when cyclic is neutral and collective is at maximum

	double maxTppPitch; //!< The maximum pitch angle of the tip-plane-path.
	double maxTppRoll; //!< The maximum roll angle of the tip-plane-path.

	double zeroLiftAngleOfAttack;
	double bladeStallAngleOfAttack;
	double bladeLiftSlopePerRadian;
	double stallLiftCoefficient;
	double planAreaOfAllBlades; //!< bladeSurfaceArea * bladeCount
	double diskRadius;

	//! The downward velocity of air through the rotor disk. If the scalar is used, the induced velocity is constant.
	//! If the curve is used, the induced velocity is a function of the aircraft airspeed.
	ScalarOrCurve inducedVelocity;
};

typedef std::shared_ptr<MainRotorParams> MainRotorParamsPtr;

struct MainRotorComponentConfig
{
	MainRotorParamsPtr params;
	NonNullPtr<Node> node;
	NonNullPtr<Motion> motion;
	NullablePtr<DynamicBodyComponent> body;
	Vector3 positionRelBody;
	Quaternion orientationRelBody;

	//! First component is positive right stick deflection, second component is positive backward stick deflection. Range is [-1, 1].
	ControlInputVec2Ptr cyclicInput;
	ControlInputVec2Ptr cyclicTrimInput; //!< May be null

	ControlInputFloatPtr collectiveInput; //!< range [0, 1]
};

class MainRotorComponent : public Component, public Rotor, public Trimmable
{
public:
	MainRotorComponent(const MainRotorComponentConfig& config);

	void setNormalizedRpm(double rpm) { mDriverRpm = rpm; }

	double getCollectivePitchAngle() const {return mCollectivePitch;}

	void setRotationAngle(double angle) {mRotationAngle = angle;}
	double getRotationAngle() const override {return mRotationAngle;}

	const Vector3& getHubPositionRelBody() const override {return mPositionRelBody;}
	const Quaternion& getTppOrientationRelBody() const override {return mTppOrientationRelBody;}

	//! Calculates the induced velocity, which is the downward velocity of the air through the rotor disk.
	//! @param airspeed is the airspeed of the helicopter.
	double calculateInducedVelocity(double airspeed) const;

	//! Calculates the induced velocity, which is the downward velocity of the air through the rotor disk.
	//! Uses the current airspeed of the helicopter.
	double calculateInducedVelocity() const;

	struct BladeAirflow
	{
		double bladeSpeedRelAirflow;
		double angleOfAttack;
	};

	//! @returns std::nullopt if there is no forward airflow through the blade element
	static std::optional<MainRotorComponent::BladeAirflow> calculateBladeElementAirflow(const Vector3& airflowVelInTppFrame, double meanBladeSpeedRelHeli, const Vector3& bladeTravelDirectionInTppFrame, double bladePitch);

	Vector3 calculateHubForceInBodyAxes(double inducedVel, const Quaternion& tppOrientationRelBody) const;

public: // Component interface
	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(MainRotorComponent), typeid(Rotor), typeid(Trimmable)};
	}

public: // Trimmable interface
	Vector3 calcRotationalTrimMomentInBodyAxes(const Controls& controls) const override;

public: // SimUpdatable interface
	void advanceSimTime(SecondsD newTime, SecondsD dt) override;

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(UpdateStage::PreDynamicsSubStep, updatePreDynamicsSubstep)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updatePreDynamicsSubstep();

private:
	//! Sets the orientation of the tip-plane-path (TPP) based on the cyclic control inputs.
	//! The TPP orientation is used for orientating the rotor disk for visualization purposes, but does not affect the physics of the rotor disk in our model.
	Quaternion calcTppOrientationFromControls(const glm::dvec2& cyclicInput) const;

	double lookupInducedVelocityCurve(double airspeed) const;

private:
	MainRotorParamsPtr mParams;
	NonNullPtr<Node> mNode;
	NonNullPtr<Motion> mMotion;
	NullablePtr<DynamicBodyComponent> mBody;
	ControlInputVec2Ptr mCyclicInput; //!< range is [-1, 1]. Positive backward and right.
	ControlInputVec2Ptr mCyclicTrimInput; //!< range is [-1, 1]. Positive backward and right. May be null.
	ControlInputFloatPtr mCollectiveInput; //!< range [0, 1]

	const Vector3 mPositionRelBody;
	const Quaternion mOrientationRelBody;
	double mDriverRpm;

	double mRpm;
	double mRotationAngle;
	double mCollectivePitch;
	Quaternion mTppOrientationRelBody;
	SecondsD mDt = 0;
};

} // namespace sim
} // namespace skybolt