/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TimeSource.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt::sim {

TimeSource::TimeSource(const TimeRange& range) :
	mRange(range),
	mState(StateStopped)
{
	mTime = mRange.start;
}

void TimeSource::setTime(sim::SecondsD time)
{
	SecondsD newTime = math::clamp(time, mRange.start, mRange.end);
	if (mTime != newTime)
	{
		mTime = newTime;
		timeChanged(mTime);

		if (mTime == mRange.end)
		{
			setState(StateStopped);
		}
	}
}

void TimeSource::setRange(const TimeRange& range)
{
	if (mRange != range)
	{
		mRange = range;
		mTime = math::clamp(mTime, range.start, range.end);
		rangeChanged(range);

		if (mTime == mRange.end)
		{
			setState(StateStopped);
		}
	}
}

void TimeSource::setState(const State& state)
{
	if (mState != state)
	{
		mState = state;
		stateChanged(mState);
	}
}

void TimeSource::advanceTime(SecondsD dt)
{
	SecondsD time = mTime + dt;
	setTime(time);
}

void TimeSource::advanceToTime(sim::SecondsD time)
{
	SecondsD dt = time - mTime;
	if (dt > 0)
	{
		advanceTime(dt);
	}
}

} // namespace skybolt::sim