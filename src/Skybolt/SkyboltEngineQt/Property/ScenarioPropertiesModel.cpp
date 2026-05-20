/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ScenarioPropertiesModel.h"
#include "Util/QtDateTimeUtil.h"

#include <SkyboltWidgets/Property/QtPropertyMetadata.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <QDateTime>

using namespace skybolt;

ScenarioPropertiesModel::ScenarioPropertiesModel(Scenario* scenario) :
	mScenario(scenario)
{
	assert(mScenario);

	{
		mStartDateTime = createQtProperty("startTime", QDateTime());
		mProperties[PropertiesModel::getDefaultSectionName()].push_back(mStartDateTime);

		connect(mStartDateTime->value().get(), &QtValue::valueChanged, [this]() {
			QDateTime dateTime = mStartDateTime->value()->value().toDateTime();
			mScenario->startJulianDate = qdateTimeToJulianDate(dateTime);
		});
	}
	{
		mDuration = createQtProperty("duration", 0.0);
		mProperties[PropertiesModel::getDefaultSectionName()].push_back(mDuration);

		connect(mDuration->value().get(), &QtValue::valueChanged, [this]() {
			sim::TimeRange range = mScenario->timeSource->getRange();
			range.end = mDuration->value()->value().toDouble();
			mScenario->timeSource->setRange(range);
		});
	}
	{
		mTimelineMode = createQtProperty("timelineMode", 0);
		mTimelineMode->setProperty(QtPropertyMetadataKeys::optionNames, QStringList({"Live", "Free"}));
		mProperties[PropertiesModel::getDefaultSectionName()].push_back(mTimelineMode);

		connect(mTimelineMode->value().get(), &QtValue::valueChanged, [this]() {
			mScenario->timelineMode.set(skybolt::TimelineMode(mTimelineMode->value()->value().toInt()));
		});
	}

	update();
}

void ScenarioPropertiesModel::update()
{
	mStartDateTime->value()->setValue(julianDateToQDateTime(mScenario->startJulianDate));
	mDuration->value()->setValue(mScenario->timeSource->getRange().end);
	mTimelineMode->value()->setValue(int(mScenario->timelineMode.get()));
}
