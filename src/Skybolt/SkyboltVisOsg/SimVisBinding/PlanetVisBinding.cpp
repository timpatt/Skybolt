/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlanetVisBinding.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/CloudComponent.h>
#include <SkyboltSim/Components/OceanComponent.h>
#include <SkyboltVisOsg/Renderable/Planet/Planet.h>
#include <SkyboltVisOsg/Renderable/Water/WaterMaterial.h>

namespace skybolt {

PlanetVisBinding::PlanetVisBinding(JulianDateProvider dateProvider, SimTimeProvider simTimeProvider, const sim::Entity* entity, const vis::PlanetPtr& visObject) :
	SimpleSimVisBinding(entity, visObject),
	mDateProvider(dateProvider),
	mSimTimeProvider(simTimeProvider)
{}

void PlanetVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	SimpleSimVisBinding::syncVis(converter);

	auto visPlanet = static_cast<vis::Planet*>(mVisObjects.front().object.get());
	visPlanet->setJulianDate(mDateProvider());
	visPlanet->setSimTime(mSimTimeProvider());

	if (auto cloud = mEntity->getFirstComponent<sim::CloudComponent>(); cloud)
	{
		visPlanet->setCloudCoverageFraction(cloud->layers.size() ? cloud->layers.front().coverageFraction : std::optional<float>());
	}
}

} // namespace skybolt