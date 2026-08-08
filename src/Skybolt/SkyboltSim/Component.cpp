/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Component.h"

namespace skybolt {
namespace sim {

std::vector<std::type_index> Component::getExposedTypes() const { return { typeid(*this) }; }


} // namespace sim

} // namespace skybolt