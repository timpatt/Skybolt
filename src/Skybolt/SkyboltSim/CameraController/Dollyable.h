/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltReflect/Reflection.h>

namespace skybolt {
namespace sim {

class Dollyable
{
public:
	virtual ~Dollyable() = default;
	virtual double getDollyFactor() const { return mDollyFactor; }
	virtual void setDollyFactor(double v) { mDollyFactor = v; }

protected:
	double mDollyFactor = 0; //!< 0 means no dolly, 1 means fully dolled in. The exact effect of the dolly factor is determined by the camera controller.
};

SKYBOLT_REFLECT(Dollyable) {
	registry.type<Dollyable>("Dollyable")
		.property("dollyFactor", &Dollyable::getDollyFactor, &Dollyable::setDollyFactor);
}

} // namespace sim
} // namespace skybolt