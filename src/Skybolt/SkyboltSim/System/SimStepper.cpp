/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimStepper.h"
#include "SkyboltSim/System/System.h"
#include <assert.h>

namespace skybolt {
namespace sim {

SimStepper::SimStepper(const SystemRegistryPtr& systems, const TimeRange& range) :
	TimeSource(range),
	mSystems(systems)
{
	assert(mSystems);
}

SimStepper::~SimStepper() = default;

void SimStepper::setTime(SecondsD t)
{
	if (mTime != t)
	{
		TimeSource::setTime(t);
		for (const SystemPtr& system : *mSystems)
		{
			system->setSimTime(t);
		}
	}
}

void SimStepper::advanceTime(SecondsD dt)
{
	auto systems = *mSystems; // Take copy in case a system adds/removes another system during step

	updateSystem(systems, UpdateStage::Input);
	updateSystem(systems, UpdateStage::BeginStateUpdate);

	if (dt > 0)
	{
		if (mDynamicsEnabled)
		{
			advanceTimeByDynamicsSubSteps(systems, dt);
		}
		else
		{
			advaniceTimeByNonDynamicsStep(systems, dt);
		}
	}

	updateSystem(systems, UpdateStage::EndStateUpdate);
	updateSystem(systems, UpdateStage::Attachments);
	updateSystem(systems, UpdateStage::Output);
}

void SimStepper::advanceTimeByDynamicsSubSteps(const std::vector<SystemPtr>& systems, SecondsD dt)
{
	assert(mDynamicsEnabled);

	// Calculate required number of substeps
	double newStepTimer = mStepTimer + dt;

	int requiredSteps = int(newStepTimer / mDynamicsStepSize);
	if (mMaxDynamicsSubsteps && requiredSteps > *mMaxDynamicsSubsteps)
	{
		requiredSteps = *mMaxDynamicsSubsteps;
		double dt = (double)*mMaxDynamicsSubsteps * mDynamicsStepSize;
		newStepTimer = mStepTimer + dt;
	}

	mStepTimer = newStepTimer - requiredSteps * mDynamicsStepSize;

	// Perform substeps
	for (int i = 0; i < requiredSteps; i++)
	{
		TimeSource::advanceTime(mDynamicsStepSize);

		updateSystem(systems, UpdateStage::PreDynamicsSubStep);

		for (const SystemPtr& system : *mSystems)
		{
			system->advanceSimTime(mTime, mDynamicsStepSize);
		}
		updateSystem(systems, UpdateStage::DynamicsSubStep);
		updateSystem(systems, UpdateStage::PostDynamicsSubStep);
	}
}

void SimStepper::advaniceTimeByNonDynamicsStep(const std::vector<SystemPtr>& systems, SecondsD dt)
{
	assert(!mDynamicsEnabled);

	// Dynamics is disabled, so just advance time by dt without taking substeps
	TimeSource::advanceTime(dt);

	for (const SystemPtr& system : *mSystems)
	{
		system->advanceSimTime(mTime, mDynamicsStepSize);
	}
}

void SimStepper::updateSystem(const std::vector<SystemPtr>& systems, UpdateStage stage)
{
	for (const SystemPtr& system : systems)
	{
		system->update(stage);
	}
}

} // namespace sim
} // namespace skybolt