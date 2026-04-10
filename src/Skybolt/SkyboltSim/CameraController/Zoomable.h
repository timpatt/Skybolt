/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltReflect/Reflection.h>

namespace skybolt {
namespace sim {

class Zoomable
{
public:
	virtual ~Zoomable() = default;
	virtual double getZoom() const = 0;
	virtual void setZoom(double zoom) = 0;
};

class DefaultZoomable : public Zoomable
{
public:
	~DefaultZoomable() override = default;
	double getZoom() const override { return mZoom; }
	void setZoom(double zoom) override { mZoom = zoom; }

protected:
	double mZoom = 0;
};

SKYBOLT_REFLECT(Zoomable) {
	registry.type<Zoomable>("Zoomable")
		.property("zoom", &Zoomable::getZoom, &Zoomable::setZoom);
}

} // namespace sim
} // namespace skybolt