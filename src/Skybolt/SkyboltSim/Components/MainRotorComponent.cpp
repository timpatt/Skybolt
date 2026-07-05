/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "MainRotorComponent.h"
#include "SkyboltSim/Components/DynamicBodyComponent.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/Motion.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/Units.h>

using namespace std;

namespace skybolt {
namespace sim {

MainRotorComponent::MainRotorComponent(const MainRotorComponentConfig& config) :
	mParams(config.params),
	mNode(config.node),
	mMotion(config.motion),
	mBody(config.body),
	mDriverRpm(0.0f),
	mRpm(0.0f),
	mRotationAngle(0.0f),
	mCollectivePitch(config.params->minCollectivePitch),
	mPositionRelBody(config.positionRelBody),
	mOrientationRelBody(config.orientationRelBody),
	mTppOrientationRelBody(config.orientationRelBody),
	mCyclicInput(config.cyclicInput),
	mCyclicTrimInput(config.cyclicTrimInput),
	mCollectiveInput(config.collectiveInput)
{
	assert(mCyclicInput);
	assert(mCollectiveInput);
}

void MainRotorComponent::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	mDt += dt;
}

void MainRotorComponent::updatePreDynamicsSubstep()
{
	SecondsD dt = 0;
	std::swap(mDt, dt);
	
	// Calculate collective pitch.
	// Collective pitch is typically connected to collective through direct linkages, so assume there's no lag.
	mCollectivePitch = std::lerp(mParams->minCollectivePitch, mParams->maxCollectivePitch, mCollectiveInput->value);

	// Calc rotor RPM
	assert(mDriverRpm >= 0.0f && mDriverRpm <= 1.0f);
	mRpm = mDriverRpm * mParams->maxRpm;

	// Spin rotor
	mRotationAngle += mRpm * skybolt::rpmToRadPerSec * dt;
	mRotationAngle = fmod(mRotationAngle, skybolt::math::twoPiF());

	// Calculate induced velocity
	Quaternion bodyOrientation = mNode->getOrientation();
	const Vector3 velocityLocal = glm::inverse(bodyOrientation) * mMotion->linearVelocity;
	const double airspeed = glm::length(velocityLocal);
	double inducedVel = calculateInducedVelocity(airspeed); // induced velocity curve lookup

	// Calculate TPP orientation from cyclic
	glm::dvec2 trim = mCyclicTrimInput ? glm::dvec2(mCyclicTrimInput->value) : glm::dvec2(0.0, 0.0);
	glm::dvec2 trimmedCyclic = glm::clamp(glm::dvec2(mCyclicInput->value) + trim, glm::dvec2(-1), glm::dvec2(1));
	mTppOrientationRelBody = calcTppOrientationFromControls(trimmedCyclic);

	if (mBody)
	{
		// Calc lift
		Vector3 force = calculateHubForceInBodyAxes(inducedVel, mTppOrientationRelBody);
		assert(force < 1e10); // make sure it hasn't blown up

		mBody->applyForce(bodyOrientation * force, bodyOrientation * mPositionRelBody);
	}
}

Quaternion MainRotorComponent::calcTppOrientationFromControls(const glm::dvec2& cyclicInput) const
{
	double tppPitch = cyclicInput.y * mParams->maxTppPitch;
	double tppRoll = cyclicInput.x * mParams->maxTppRoll;
	return mOrientationRelBody * glm::angleAxis(tppPitch, Vector3(0, 1, 0)) * glm::angleAxis(tppRoll, Vector3(1, 0, 0));
}

double MainRotorComponent::lookupInducedVelocityCurve(double airspeed) const
{
	if (std::holds_alternative<double>(mParams->inducedVelocity))
	{
		return std::get<double>(mParams->inducedVelocity);
	}
	else if (std::holds_alternative<math::LookupTable1D>(mParams->inducedVelocity))
	{
		return math::interpolateTableLinear(std::get<math::LookupTable1D>(mParams->inducedVelocity), airspeed, /* extrapolate */ false).value_or(0);
	}
	assert(false && "MainRotorComponent: inducedVelocity must be either a double or a LookupTable1D");
	return 0;
}

double MainRotorComponent::calculateInducedVelocity(double airspeed) const
{
	double inducedVel = lookupInducedVelocityCurve(airspeed);
	inducedVel *= mDriverRpm;
	return inducedVel;
}

double MainRotorComponent::calculateInducedVelocity() const
{
	return calculateInducedVelocity(glm::length(mMotion->linearVelocity));
}

static const int elementCount = 4; //!< This is the number of integration elements to use around the rotor disk, not the number of rotor blades.

std::optional<MainRotorComponent::BladeAirflow> MainRotorComponent::calculateBladeElementAirflow(const Vector3& airflowVelInTppFrame, double meanBladeSpeedRelHeli, const Vector3& bladeTravelDirectionInTppFrame, double bladePitch)
{
	Vector3 bladeVelRelAirflowInTppFrame = meanBladeSpeedRelHeli * bladeTravelDirectionInTppFrame - airflowVelInTppFrame;

	double bladeSpeedRelAirflow = glm::dot(Vector3(bladeVelRelAirflowInTppFrame.x, bladeVelRelAirflowInTppFrame.y, 0.0), bladeTravelDirectionInTppFrame);
	if (bladeSpeedRelAirflow <= 0.0f)
		return std::nullopt;

	return BladeAirflow{
		.bladeSpeedRelAirflow = bladeSpeedRelAirflow,
		.angleOfAttack = bladePitch + (double)atan(bladeVelRelAirflowInTppFrame.z / bladeSpeedRelAirflow)
	};
}

//! Distance along the blade from the center of rotation to the point where the lift is calculated, normalized by the rotor radius. Used to calculate the average lift across the rotor disk.
//! The RMS is used because the lift is proportional to the square of the velocity, which is proportional to the square of the distance from the center of rotation. The RMS of a uniform distribution from 0 to 1 is sqrt(1/3).
const double rootMeanSquaredBladeStation = std::sqrt(1.0 / 3.0);

Vector3 MainRotorComponent::calculateHubForceInBodyAxes(double inducedVelocity, const Quaternion& tppOrientationRelBody) const
{
	// Calculate relative velocity experienced by rotor disk plane perpendicular to the Tip Path Plane (TPP).
	Vector3 airflowVelInTppFrame = glm::inverse(mNode->getOrientation() * tppOrientationRelBody) * -mMotion->linearVelocity;
	airflowVelInTppFrame += inducedVelocity * Vector3(0, 0, 1); // Induced airflow velocity is downward
	
	double bladeStation = mParams->diskRadius * rootMeanSquaredBladeStation;
	double meanBladeSpeed = mRpm * skybolt::rpmToRadPerSec * bladeStation;

	// Accumulators for net forces and moments acting at the hub origin in the Hub Frame
	double lift = 0;

	for (int i = 0; i < elementCount; i++)
	{
		// Azimuth angle is 0 at the helicpter's nose (+X) and clockwise to the right (+Y)
		double elementAzimuth = (double)i * skybolt::math::twoPiD() / (double)elementCount;

		Vector3 bladeVelocityDir(-std::sin(elementAzimuth), std::cos(elementAzimuth), 0.0);
	
		if (std::optional<MainRotorComponent::BladeAirflow> airflow = calculateBladeElementAirflow(airflowVelInTppFrame, meanBladeSpeed, bladeVelocityDir, mCollectivePitch); airflow)
		{
			constexpr double airDensity = 1.225; // kg/m^3 at sea level TODO: use actual air density from atmosphere model

			// Calculate aerodynamic coefficients
			double liftCoefficient;
			if (std::abs(airflow->angleOfAttack) < mParams->bladeStallAngleOfAttack)
			{
				liftCoefficient = mParams->bladeLiftSlopePerRadian * (airflow->angleOfAttack - mParams->zeroLiftAngleOfAttack);
			}
			else
			{
				// Stalled
				liftCoefficient = mParams->stallLiftCoefficient * glm::sign(airflow->angleOfAttack);
			}

			// Calculate lift
			double elementLiftMagnitude = 0.5 * airDensity * airflow->bladeSpeedRelAirflow * airflow->bladeSpeedRelAirflow * mParams->planAreaOfAllBlades * liftCoefficient;
			lift += elementLiftMagnitude;
		}
	}
	lift /= (float)elementCount;

	return tppOrientationRelBody * Vector3(0, 0, -lift);
}

Vector3 MainRotorComponent::calcRotationalTrimMomentInBodyAxes(const Controls& controls) const
{
	if (!mBody)
	{
		return math::dvec3Zero();
	}

	Vector3 force = calculateHubForceInBodyAxes(calculateInducedVelocity(), calcTppOrientationFromControls(controls.stickInput));
	return glm::cross(mPositionRelBody - mBody->getCenterOfMass(), force);
}

} // namespace sim
} // namespace skybolt