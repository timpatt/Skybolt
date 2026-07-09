/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FuselageComponent.h"
#include "SkyboltSim/Components/DynamicBodyComponent.h"
#include "SkyboltSim/Components/Node.h"
#include "SkyboltSim/Components/Motion.h"
#include "SkyboltSim/Physics/Atmosphere.h"
#include "SkyboltSim/Spatial/Geocentric.h"
#include "SkyboltSim/Spatial/GreatCircle.h"
#include "SkyboltSim/Spatial/Orientation.h"

#include <algorithm>

namespace skybolt {
namespace sim {

FuselageComponent::FuselageComponent(const FuselageComponentConfig& config) :
	mParams(config.params),
	mNode(config.node),
	mMotion(config.motion),
	mBody(config.body),
	mStickInput(config.stickInput),
	mStickTrimInput(config.stickTrimInput),
	mRudderInput(config.rudderInput)
{
	assert(mNode);
	assert(mMotion);
	assert(mBody);
}

void FuselageComponent::advanceSimTime(SecondsD newTime, SecondsD dt)
{
	mDt += dt;
}

static Vector3 calculateLiftDirection(const Vector3& velocity, const Vector3& bodyUp) {
    // Prevent division by zero if velocity is zero or extremely small
    if (glm::dot(velocity, velocity) < 1e-6f)
	{
        return glm::normalize(bodyUp); 
    }

    // Find the vector perpendicular to both velocity and up
    Vector3 right = glm::cross(velocity, bodyUp);

    // Handle the edge case where velocity is perfectly parallel to bodyUp 
    if (glm::dot(right, right) < 1e-6f)
	{
        return glm::normalize(bodyUp);
    }

    // Get the lift direction
    return glm::normalize(glm::cross(right, velocity));
}

void FuselageComponent::updatePreDynamicsSubstep()
{
	SecondsD dt = 0;
	std::swap(mDt, dt);

	const Quaternion& orientation = mNode->getOrientation();
	const Vector3& velocityInBodyAxes = glm::inverse(orientation) * mMotion->linearVelocity;

	// Calculate angle of attack and side slip
	double angleOfAttack = std::atan2(velocityInBodyAxes.z, velocityInBodyAxes.x);
	double sideSlipAngle = std::atan2(-velocityInBodyAxes.y, velocityInBodyAxes.x);

	// Apply lift
	double airDensity = calcAirDensity(calcAltitude(mNode->getPosition()));
	Vector3 lift = calcLiftForceInWorldAxes({
		.velocityInBodyAxes = velocityInBodyAxes,
		.angleOfAttack = angleOfAttack,
		.airDensity = airDensity
	});
	mBody->applyCentralForce(lift);

	// Apply drag
	Vector3 drag = calcTotalDragForceInWorldAxes(velocityInBodyAxes, airDensity, glm::length(lift));
	mBody->applyCentralForce(drag);

	// Resolve stick input
	glm::dvec2 stickInput = mStickInput ? glm::dvec2(mStickInput->value) : glm::dvec2(0,0);
	if (mStickTrimInput)
	{
		stickInput = glm::clamp(stickInput + glm::dvec2(mStickTrimInput->value), glm::dvec2(-1), glm::dvec2(1));
	}

	// Apply moments
	const Vector3 moment = calcMomentInBodyAxes(CalcMomentArgs{
		.controls = Controls{
			.stickInput = stickInput,
			.rudderInput = mRudderInput ? mRudderInput->value : 0
		},
		.airDensity = airDensity,
		.angleOfAttack = angleOfAttack,
		.sideSlipAngle = sideSlipAngle,
		.angularVelocityInBodyAxes = glm::inverse(orientation) * mMotion->angularVelocity
	});
	mBody->applyTorque(orientation * moment);

	// Cache some values so they can be queried by the API
	mAngleOfAttackFromLastTimestep = angleOfAttack;
	mSideSlipAngleFromLastTimestep = sideSlipAngle;
}

double FuselageComponent::calcLiftCoefficent(double alpha) const
{
	double alphaDelta = alpha - mParams.zeroLiftAlpha;
	if (std::abs(alphaDelta) > mParams.stallAlpha)
	{
		return mParams.stallLift;
	}
	return mParams.liftSlope * alphaDelta;
}

Vector3 FuselageComponent::calcTrimRotationalMomentInBodyAxes(const Controls& controls) const
{
	const Vector3& velocityInBodyAxes = glm::inverse(mNode->getOrientation()) * mMotion->linearVelocity;

	return calcMomentInBodyAxes(CalcMomentArgs{
		.controls = controls,
		.airDensity = calcAirDensity(calcAltitude(mNode->getPosition())),
		.angleOfAttack = std::atan2(velocityInBodyAxes.z, velocityInBodyAxes.x),
		.sideSlipAngle = std::atan2(-velocityInBodyAxes.y, velocityInBodyAxes.x),
		.angularVelocityInBodyAxes = {} // In trim condition the angular velocity is zero
	});
}

Vector3 FuselageComponent::calcTrimNetForceInWorldAxes(const Controls& controls) const
{
	Vector3 upDir = calcLtpUpDirection(mNode->getPosition()).value_or(math::dvec3Zero());
	Vector3 horizontalVelocityInWorldAxes = mMotion->linearVelocity - upDir * glm::dot(upDir, mMotion->linearVelocity); // Trim for level flight, so ignore vertical component of velocity when calculating net force

	Vector3 velocityInBodyAxes = glm::inverse(mNode->getOrientation()) * horizontalVelocityInWorldAxes;

	double airDensity = calcAirDensity(calcAltitude(mNode->getPosition()));
	double angleOfAttack = std::atan2(velocityInBodyAxes.z, velocityInBodyAxes.x);

	Vector3 lift = calcLiftForceInWorldAxes({
		.velocityInBodyAxes = velocityInBodyAxes,
		.angleOfAttack = angleOfAttack,
		.airDensity = airDensity});
	Vector3 drag = calcTotalDragForceInWorldAxes(velocityInBodyAxes, airDensity, glm::length(lift));
	return lift + drag;
}

constexpr double airDensityAtSeaLevel = 1.225; //!< kg/m^3

static double fakeWeathercockCurve(double angleToWind)
{
	// The weathercock factor is a measure of how much the fuselage will tend to align with the relative wind.
	// In reality this is a complex function of the fuselage shape, but here we use a simple fudge as a fallback for when a lookup table is not provided. It's not physically realistic, but better than nothing.
	return std::sin(angleToWind);
}

static double lookupWeathercockFactor(const ScalarOrCurve& v, double angleToWind)
{
	if (std::holds_alternative<double>(v))
	{
		return std::get<double>(v) * fakeWeathercockCurve(angleToWind);
	}
	const math::LookupTable1D& table = std::get<math::LookupTable1D>(v);
	return math::interpolateTableLinear(table, angleToWind, /* extrapolate */ false).value_or(0.0);
}

Vector3 FuselageComponent::calcMomentInBodyAxes(const CalcMomentArgs& args) const
{
	const double speed = glm::length(mMotion->linearVelocity);

	double speedScaleFactor = std::min(speed, mParams.aerodynamicDerivativeReferenceSpeed) / mParams.aerodynamicDerivativeReferenceSpeed;
	speedScaleFactor *= speedScaleFactor; // Scale by square of speed ratio, since aerodynamic forces scale with the square of speed

	double rollAngle = math::eulerFromQuat(toLtpNed(GeocentricOrientation(mNode->getOrientation()), geocentricToLatLon(mNode->getPosition())).orientation).x;

	Vector3 accel;

	// Roll
	accel.x = lookupWeathercockFactor(mParams.rollAccelDueToSideSlipAngle, args.sideSlipAngle) * speedScaleFactor
		+ mParams.rollAccelDueToRollRate * args.angularVelocityInBodyAxes.x
		+ mParams.rollAccelDueToYawRate * args.angularVelocityInBodyAxes.z;

	// Pitch
	accel.y = mParams.pitchBaseAccel * speedScaleFactor + lookupWeathercockFactor(mParams.pitchAccelDueToAngleOfAttack, args.angleOfAttack) * speedScaleFactor
			 + mParams.pitchAccelDueToPitchRate * args.angularVelocityInBodyAxes.y;

	// Yaw
	accel.z = lookupWeathercockFactor(mParams.yawAccelDueToSideSlipAngle, args.sideSlipAngle) * speedScaleFactor + mParams.yawAccelDueToRollRate * args.angularVelocityInBodyAxes.x + mParams.yawAccelDueToYawRate * args.angularVelocityInBodyAxes.z;

	// Control inputs
	accel.x += mParams.rollAccelDueToAileron * args.controls.stickInput.x * speedScaleFactor;
	accel.y += mParams.pitchAccelDueToElevator * args.controls.stickInput.y * speedScaleFactor;
	accel.z += mParams.yawAccelDueToRudder * args.controls.rudderInput * speedScaleFactor;

	Vector3 moment = accel * mBody->getMomentOfInertia();

	return moment * (double)args.airDensity / airDensityAtSeaLevel;
}

Vector3 FuselageComponent::calcLiftForceInWorldAxes(const CalcLiftForceArgs& args) const
{
	// Calculate lift
	const double airDensity = calcAirDensity(calcAltitude(mNode->getPosition()));
	double liftCoeff = calcLiftCoefficent(args.angleOfAttack);

	Vector3 liftDirection = calculateLiftDirection(mNode->getOrientation() * args.velocityInBodyAxes, mNode->getOrientation() * Vector3(0, 0, -1));
	double liftMagnitude = liftCoeff * mParams.liftArea * 0.5 * airDensity * glm::dot(args.velocityInBodyAxes, args.velocityInBodyAxes);

	return liftDirection * liftMagnitude;
}

Vector3 FuselageComponent::calcTotalDragForceInWorldAxes(const Vector3& velocityInBodyAxes, double airDensity, double liftForce) const
{
	double speed = glm::length(velocityInBodyAxes);
	if (speed > 0.0f)
	{
		Vector3 velocityWorld = mNode->getOrientation() * velocityInBodyAxes;
		Vector3 dragDirection = -velocityWorld / speed;
		return dragDirection * (calcParasiteDragScalar(velocityInBodyAxes, airDensity) + calcInducedDragScalar(velocityInBodyAxes, airDensity, liftForce));
	}
	return math::dvec3Zero();
}

double FuselageComponent::calcParasiteDragScalar(const Vector3 &velocityInBodyAxes, double density) const
{
	// Drag = 0.5 * airDensity * v^2 * Cd * A.
	// Here we've combined Cd * A into dragConst
	double CDrag = velocityInBodyAxes.x*velocityInBodyAxes.x * mParams.dragConstant.x
			     + velocityInBodyAxes.y*velocityInBodyAxes.y * mParams.dragConstant.y
				 + velocityInBodyAxes.z*velocityInBodyAxes.z * mParams.dragConstant.z;
	return CDrag * 0.5 * density;
}

double FuselageComponent::calcInducedDragScalar(const Vector3 &velocityInBodyAxes, double density, double liftForce) const
{
	if (mParams.effectiveWingSpan <= 0)
	{
		return 0;
	}

	double q = 0.5 * density * dot(velocityInBodyAxes, velocityInBodyAxes);
	double numerator  =liftForce * liftForce;
	double denominator = q * math::piD() * mParams.effectiveWingSpan * mParams.effectiveWingSpan * mParams.wingOswaldEfficiencyFactor;
	return numerator / denominator;
}

double FuselageComponent::calcAltitude(const sim::Vector3& position)
{
	return glm::length(position) - earthRadius();
}

double FuselageComponent::calcAirDensity(double altitude)
{
	static Atmosphere atmosphere = createEarthAtmosphere();
	return atmosphere.getDensity(altitude);
}

} // namespace sim
} // namespace skybolt