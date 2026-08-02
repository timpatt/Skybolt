/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "TimeSource.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/System/SystemRegistry.h"
#include "System.h"

#include <optional>
#include <vector>

namespace skybolt {
namespace sim {

class SimStepper : public TimeSource
{
public:
	SimStepper(const SystemRegistryPtr& systems, const TimeRange& range);
	~SimStepper();

	void setTime(SecondsD t) override;

	void advanceTime(SecondsD dt) override;
	void advanceToTime(SecondsD dt) override;

	bool isDynamicsEnabled() const { return mDynamicsEnabled; }
	void setDynamicsEnabled(bool enabled) { mDynamicsEnabled = enabled; }

	SecondsD getDynamicsStepSize() const { return mDynamicsStepSize; }
	void setDynamicsStepSize(double stepSize) { mDynamicsStepSize = stepSize; }

	std::optional<int> getMaxDynamicsSubsteps() const { return mMaxDynamicsSubsteps; }
	void setMaxDynamicsSubsteps(const std::optional<int>& substeps) { mMaxDynamicsSubsteps = substeps; }

private:
	void advanceTimeByDynamicsSubSteps(const std::vector<SystemPtr>& systems, SecondsD dt);
	void advanceTimeByNonDynamicsStep(const std::vector<SystemPtr>& systems, SecondsD dt);

	void updateSystem(const std::vector<SystemPtr>& systems, UpdateStage stage);

private:
	SystemRegistryPtr mSystems;
	SecondsD mStepTimer = 0;
	bool mDynamicsEnabled = true;

	double mDynamicsStepSize = 1.0 / 60.0;
	std::optional<int> mMaxDynamicsSubsteps = 10;
};

} // namespace sim
} // namespace skybolt