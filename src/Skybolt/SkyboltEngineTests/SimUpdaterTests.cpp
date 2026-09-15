/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TestHelpers.h"
#include <catch2/catch.hpp>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltEngine/UpdateLoop/SimUpdater.h>
#include <SkyboltSim/System/SystemRegistry.h>
#include <SkyboltSim/System/TimeSource.h>


using namespace skybolt;

TEST_CASE("SimUpdater: update advances time", "[SimUpdater]") 
{
    auto timeSource = std::make_shared<sim::TimeSource>(sim::TimeRange{0, 100});
	timeSource->setState(sim::TimeSource::StatePlaying);

    skybolt::Scenario scenario(timeSource);
    auto systemRegistry = std::make_shared<sim::SystemRegistry>();

    SimUpdater updater(&scenario, systemRegistry);

    SECTION("Update with small wall clock time advances sim time by same amount") 
    {
        updater.update(0.1);
        CHECK(scenario.timeSource->getTime() == Approx(0.1));
    }

    SECTION("Update with delta time does not advance sim time") 
    {
        updater.update(0.0);
        CHECK(scenario.timeSource->getTime() == 0.0);
    }

    SECTION("Update with extremely large delta time advances sim time by MaxSimDt") 
    {
		updater.setMaxSimTimeStep(10.0);
        updater.update(100);
        CHECK(scenario.timeSource->getTime() == Approx(10.0));
        CHECK(updater.getActualTimeRate() == Approx(0.1));
    }

    SECTION("Sim time advances at requested rate") 
    {
        updater.setRequestedTimeRate(2.0);
        updater.update(0.1);
        CHECK(scenario.timeSource->getTime() == Approx(0.2));
        CHECK(updater.getActualTimeRate() == Approx(2.0));
    }

    SECTION("Sim time does not update when TimeSource is stopped") 
    {
		timeSource->setState(sim::TimeSource::StateStopped);
        updater.update(0.1);
        CHECK(scenario.timeSource->getTime() == 0.0);
    }

    SECTION("Sim time does not update until enough wallclock time has accumulated to satisfy MinSimDt") 
    {
        updater.setMinSimTimeStep(0.1);
        updater.update(0.05);
        CHECK(scenario.timeSource->getTime() == 0.0);

        updater.update(0.05);
        CHECK(scenario.timeSource->getTime() == Approx(0.1));
        CHECK(updater.getActualTimeRate() == Approx(1.0));
    }
}