#include "SystemRegistry.h"
#include "System.h"

namespace skybolt {
namespace sim {

void resetSystemsToInitialState(const SystemRegistry& registry)
{
	for (const auto& system : registry)
	{
		system->reset();
	}
}

} // namespace sim
} // namespace skybolt