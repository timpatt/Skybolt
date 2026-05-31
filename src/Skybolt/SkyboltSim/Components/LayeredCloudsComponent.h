#pragma once

#include <SkyboltCommon/Units.h>
#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>

namespace skybolt::sim {

struct CloudLayer
{
	double bottomAltitude;
	double topAltitude;
	double density; //!< In range [0, inf)
	double coverageFraction; //!< Fraction of sky covered by this cloud layer, in range [0, 1].
	std::string type;
};

SKYBOLT_REFLECT(CloudLayer)
{
	registry.type<CloudLayer>("CloudLayer")
		.property("bottomAltitude", &CloudLayer::bottomAltitude)
		.property("topAltitude", &CloudLayer::topAltitude)
		.property("density", &CloudLayer::density)
		.property("type", &CloudLayer::type);
};

struct LayeredCloudsComponent : public skybolt::sim::Component
{
public:
	std::vector<CloudLayer> layers;
};


SKYBOLT_REFLECT(LayeredCloudsComponent)
{
	registry.type<LayeredCloudsComponent>("LayeredCloudsComponent")
		.superType<Component>()
		.property("layers", &LayeredCloudsComponent::layers);
};

} // namespace skybolt::sim