/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Expected.h>
#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltCommon/WeightAveragedBuffer.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/Chrono.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/System/SystemRegistry.h>

class SimUpdater
{
public:
	SimUpdater(skybolt::NonNullPtr<skybolt::Scenario> scenario, const skybolt::sim::SystemRegistryPtr& systemRegistry);
	~SimUpdater();

	void update(skybolt::sim::SecondsD wallDt);

	double getRequestedTimeRate() const { return mRequestedTimeRate; }
	void setRequestedTimeRate(double rate) { mRequestedTimeRate = rate; }

	double getActualTimeRate() const { return mActualTimeRate; }

	void setMinSimTimeStep(double dt) { mMinSimDt = dt; }
	void setMaxSimTimeStep(double dt) { mMaxSimDt = dt; }

	//! If true, the simulation will only advance when all systems are ready to advance.
	bool requireLockstep = false;

	const skybolt::Expected<std::monostate>& getReadyForAdvanceStatus() const { return mReadyForAdvanceStatus; }

protected:
	void advanceWallTime(skybolt::sim::SecondsD wallDt);

	const skybolt::NonNullPtr<skybolt::Scenario> mScenario;
	const skybolt::sim::SystemRegistryPtr mSystemRegistry;

	std::unique_ptr<skybolt::UniformAveragedBuffer> mAverageWallDt;
	skybolt::sim::SecondsD mMinSimDt = 0.01;
	skybolt::sim::SecondsD mMaxSimDt = 10;

	skybolt::sim::SecondsD mWallTime = 0;
	skybolt::sim::SecondsD mAccumulatedWallTime = 0;
	double mRequestedTimeRate = 1;
	double mActualTimeRate = 1;

	skybolt::Expected<std::monostate> mReadyForAdvanceStatus;
};