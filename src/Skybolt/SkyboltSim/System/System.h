/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/TypeIdentifiable.h>
#include "SkyboltSim/SimUpdatable.h"

namespace skybolt {
namespace sim {

class System : public SimUpdatable, public TypeIdentifiable
{
public:
	System() = default;
	~System() override = default;

	//! Sets the system to its initial state.
	//! This method is used to reset the system when restarting the simulation.
	//! This method is required because systems are persisted across simulation restarts, so their state is not automatically reset when the simulation is restarted.
	virtual void reset() {};
};

template <class DerivedT> // template argument provided to ensure each derived class has its own staticTypeId() implementation
class SystemT : public System
{
	SKYBOLT_TYPE_IDENTIFIABLE
};

} // namespace sim
} // namespace skybolt