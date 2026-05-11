/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Motion.h"
#include "PropertyMetadata.h"
#include "SkyboltSim/Entity.h"

namespace skybolt::sim {

SKYBOLT_REFLECT(Motion) {
	registry.type<Motion>("Motion")
		.superType<Component>()
		.property("linearVelocity", &Motion::linearVelocity, {{PropertyMetadataNames::attributeType, PropertyRepresentations::worldVelocity}})
		.property("angularVelocity", &Motion::angularVelocity);
}

} // namespace skybolt::sim