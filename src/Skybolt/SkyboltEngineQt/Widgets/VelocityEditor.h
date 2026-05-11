/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <QWidget>

class QStackedWidget;

class VelocityEditor : public QWidget
{
	Q_OBJECT
public:
	VelocityEditor(QWidget* parent = nullptr);

	//! Set the geocentric position used for NED and Speed-Bearing-Inclination conversions
	void setGeocentricPosition(const skybolt::sim::Vector3& position);

	void setVelocity(const skybolt::sim::Vector3& velocity);

	skybolt::sim::Vector3 getVelocity() const;

	Q_SIGNAL void valueChanged(const skybolt::sim::Vector3& velocity);

private:
	void addEditor(class VelocityEditorImpl*);
	class VelocityEditorImpl* getCurrentEditor() const;

private:
	QStackedWidget* mStackedWidget;
};
