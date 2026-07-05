/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/LookupTable1D.h>
#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include "SkyboltSim/Components/ControlInputsComponent.h"
#include <SkyboltSim/Components/Trimmable.h>

namespace skybolt {
namespace sim {

using ScalarOrCurve = std::variant<double, math::LookupTable1D>;

struct FuselageParams
{
	// Lift
	double liftSlope; // Cl per radian
	double zeroLiftAlpha;
	double stallAlpha;
	double stallLift; // Cl in stalled region
	double liftArea; // m^2

	Vector3 dragConstant; // Cd * area in each direction

	// Wing
	double effectiveWingSpan;
	double wingOswaldEfficiencyFactor;

	// Moments
	ScalarOrCurve rollAccelDueToSideSlipAngle;
	double rollAccelDueToRollRate;
	double rollAccelDueToYawRate;
	double rollAccelDueToAileron;

	double pitchBaseAccel;
	ScalarOrCurve pitchAccelDueToAngleOfAttack;
	double pitchAccelDueToPitchRate;
	double pitchAccelDueToElevator;

	ScalarOrCurve yawAccelDueToSideSlipAngle;
	double yawAccelDueToRollRate;
	double yawAccelDueToYawRate;
	double yawAccelDueToRudder;

	double aerodynamicDerivativeReferenceSpeed;
};

struct FuselageComponentConfig
{
	FuselageParams params;
	Node* node;
	Motion* motion;
	DynamicBodyComponent* body;
	ControlInputVec2Ptr stickInput; //!< Optional. Range is [-1, 1]. Positive backward and right.
	ControlInputVec2Ptr stickTrimInput; //!< Optional. Range is [-1, 1]. Positive backward and right.
	ControlInputFloatPtr rudderInput; //!< Optional. Range [-1, 1]
};

class FuselageComponent : public Component, public Trimmable
{
public:
	FuselageComponent(const FuselageComponentConfig& config);

	double getAngleOfAttack() const { return mAngleOfAttackFromLastTimestep; }
	double getSideSlipAngle() const { return mSideSlipAngleFromLastTimestep; }

	double calcLiftCoefficent(double alpha) const;

	struct CalcMomentArgs
	{
		const Controls& controls;
		double airDensity;
		double angleOfAttack;
		double sideSlipAngle;
		Vector3 angularVelocityInBodyAxes;
	};

	Vector3 calcMomentInBodyAxes(const CalcMomentArgs& args) const;

	double calcParasiteDragScalar(const Vector3 &velocityLocal, double density) const;
	double calcInducedDragScalar(const Vector3 &velocityLocal, double density, double liftForce) const;

public: // Component interface
	std::vector<std::type_index> getExposedTypes() const override
	{
		return {typeid(FuselageComponent), typeid(Trimmable)};
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
	const FuselageParams mParams;
	Node* mNode;
	Motion* mMotion;
	DynamicBodyComponent* mBody;
	ControlInputVec2Ptr mStickInput; //!< May be null
	ControlInputVec2Ptr mStickTrimInput; //!< May be null
	ControlInputFloatPtr mRudderInput; //!< May be null

	double mAngleOfAttackFromLastTimestep = 0;
	double mSideSlipAngleFromLastTimestep = 0;
	SecondsD mDt = 0;
};

} // namespace sim
} // namespace skybolt