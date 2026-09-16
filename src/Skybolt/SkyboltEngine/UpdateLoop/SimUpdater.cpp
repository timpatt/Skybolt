/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimUpdater.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltSim/System/SimStepper.h>

using namespace skybolt;
using namespace skybolt::sim;

SimUpdater::SimUpdater(skybolt::NonNullPtr<skybolt::Scenario> scenario, const SystemRegistryPtr& systemRegistry) :
	mScenario(scenario),
	mSystemRegistry(systemRegistry),
	mAverageWallDt(std::make_unique<UniformAveragedBuffer>(16))
{
	assert(mSystemRegistry);

	// TODO: unhack modifying the sim stepper here. It's a bit messy.
	if (auto simStepper = dynamic_cast<sim::SimStepper*>(mScenario->timeSource.get()); simStepper)
	{
		simStepper->setMaxDynamicsSubsteps(std::nullopt);
	}
}

SimUpdater::~SimUpdater() = default;

void SimUpdater::update(SecondsD wallDt)
{
	if (auto simStepper = dynamic_cast<sim::SimStepper*>(mScenario->timeSource.get()); simStepper)
	{
		bool isLive = mScenario->timelineMode.get() == TimelineMode::Live;
		simStepper->setDynamicsEnabled(isLive);
	}

	// Advance forward time
	if (wallDt > 0)
	{
		advanceWallTime(wallDt);
	}
}

void SimUpdater::advanceWallTime(SecondsD wallDt)
{
	mReadyForAdvanceStatus = std::monostate{};
	if (requireLockstep)
	{
		for (const SystemPtr& system : *mSystemRegistry)
		{
			mReadyForAdvanceStatus = system->isReadyToAdvanceSimTime();
			if (!has_value(mReadyForAdvanceStatus))
			{
				break; // Don't advance time yet, wait until all systems are ready to advance
			}
		}
	}

	if (has_value(mReadyForAdvanceStatus))
	{
		// Calculate simulation delta time
		double simDt;
		TimeSource& timeSource = *mScenario->timeSource;
		if (timeSource.getState() == TimeSource::StatePlaying)
		{
			mAverageWallDt->addValue(wallDt);
			double averageWallDt = mAverageWallDt->getResult();

			mAccumulatedWallTime += averageWallDt;
			double accumulatedSimTime = mAccumulatedWallTime * mRequestedTimeRate;
			if (accumulatedSimTime > mMaxSimDt)
			{
				simDt = mMaxSimDt;
			}
			else if (accumulatedSimTime < mMinSimDt)
			{
				return; // Don't advance time yet, wait until we have accumulated enough sim time
			}
			else
			{
				simDt = accumulatedSimTime;
			}

			mActualTimeRate = simDt / mAccumulatedWallTime;
			mAccumulatedWallTime = 0;
		}
		else
		{
			simDt = 0;
		}

		// Simulate by dt.
		// Note: we still need to simulate even if dt is 0, because some systems/components
		// need to still be updated even when the simulation is paused, e.g. in an editor application.
		timeSource.advanceTime(simDt);
	}

	// Advance wallclock time
	for (const SystemPtr& system : *mSystemRegistry)
	{
		system->advanceWallTime(mWallTime, wallDt);
	}

	mWallTime += wallDt;
}
