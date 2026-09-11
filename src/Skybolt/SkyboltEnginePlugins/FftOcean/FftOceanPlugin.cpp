/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FftOceanPlugin.h"

#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltEngine/EngineRoot.h>

#ifdef BUILD_WITH_OSG
#include "FftOceanHeightfieldTextureGenerator.h"
#include <SkyboltVisOsg/VisRoot.h>
#include <SkyboltVisOsg/Renderable/Water/SimpleOceanHeightfieldTextureGenerator.h>
#endif

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>

namespace skybolt {

#ifdef BUILD_WITH_OSG
static sim::ComponentPtr loadFftOcean(sim::Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json)
{
	return std::make_shared<vis::FftOceanHeightfieldTextureGenerator>(vis::FftOceanHeightfieldTextureGeneratorConfig{
		.textureSizePixels = 512
		});
}


static ComponentFactoryPtr getOceanComponentFactory()
{
	static auto factory = std::make_shared<ComponentFactoryFunctionAdapter>(loadFftOcean);
	factory->tags = { "fft" };
	return factory;
}
#endif

FftOceanPlugin::FftOceanPlugin(const PluginConfig& config) :
	Plugin(config)
{
#ifdef BUILD_WITH_OSG
	// If using the default ocean, or no factory, then use this plugin instead.
	// Otherwise anothe plugin is already being used and it should take precedent, so do nothing.
	ComponentFactoryPtr& oceanFactory = (*config.engineRoot->componentFactoryRegistry)["ocean"];
	
	if (!oceanFactory || oceanFactory->hasTag("sim") ||oceanFactory->hasTag("vis"))
	{
		oceanFactory = getOceanComponentFactory();
	}
#endif
}

FftOceanPlugin::~FftOceanPlugin() = default;

namespace plugins {

	std::shared_ptr<Plugin> createFftOceanPlugin(const PluginConfig& config)
	{
		return std::make_shared<FftOceanPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createFftOceanPlugin,
		createEnginePlugin
	)
}

} // namespace skybolt