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

const SecondsD maxWallDt = 0.2;

SimUpdater::SimUpdater(skybolt::NonNullPtr<skybolt::EngineRoot> engineRoot) :
	mEngineRoot(engineRoot),
	mAverageWallDt(std::make_unique<UniformAveragedBuffer>(16))
{
	// TODO: unhack modifying the sim stepper here. It's a bit messy.
	if (auto simStepper = dynamic_cast<sim::SimStepper*>(mEngineRoot->scenario->timeSource.get()); simStepper)
	{
		simStepper->setMaxDynamicsSubsteps(std::nullopt);
	}
}

SimUpdater::~SimUpdater() = default;

void SimUpdater::update(SecondsD wallDt)
{
	if (auto simStepper = dynamic_cast<sim::SimStepper*>(mEngineRoot->scenario->timeSource.get()); simStepper)
	{
		bool isLive = mEngineRoot->scenario->timelineMode.get() == TimelineMode::Live;
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
	// Calculate simulation delta time
	double simDt;
	TimeSource& timeSource = *mEngineRoot->scenario->timeSource;
	if (timeSource.getState() == TimeSource::StatePlaying)
	{
		mAverageWallDt->addValue(wallDt);
		double averageWallDt = mAverageWallDt->getResult();

		simDt = std::min(averageWallDt * mRequestedTimeRate, mMaxSimDt);
		mActualTimeRate = simDt / averageWallDt;
	}
	else
	{
		simDt = 0;
	}

	// Simulate by dt.
	// Note: we still need to simulate even if dt is 0, because some systems/components
	// need to still be updated even when the simulation is paused, e.g. in an editor application.
	timeSource.advanceTime(simDt);

	// Advance wallclock time
	for (const SystemPtr& system : *mEngineRoot->systemRegistry)
	{
		system->advanceWallTime(mWallTime, wallDt);
	}

	mWallTime += wallDt;
}
