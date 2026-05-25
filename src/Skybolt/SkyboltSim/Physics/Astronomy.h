/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/Spatial/AzimuthElevation.h"
#include "SkyboltSim/Spatial/LatLon.h"

namespace skybolt {
namespace sim {

double calcJulianDate(int year, int month, int day, double hourUtc);
void julianDateToYmd(double julianDate, int& year, int& month, int& day);
void julianDateToHms(double julianDate, int& hour, int& minute, double& second);

double calcEarthAxialTilt(double julianDate);

LatLon convertEclipticToEquatorial(double julianDate, const LatLon& ecliptic);

LatLon convertEquatorialToEcliptic(double julianDate, const LatLon& equatorial);

//! @returns hour angle of the vernal equinox (a.k.a Greenwich mean sidereal time) in radians
double calcHourAngleOfVernalEquinox(double julianDate);

//! @returns angle in radians
double calcHourAngle(double julianDate, const LatLon& equatorial, const LatLon& observer);

//! @returns local hour in range [0, 24)
double calcLocalHour(double julianDate, const LatLon& observer);

AzimuthElevation convertEquatorialToHorizontal(double julianDate, const LatLon& equatorial, const LatLon& observer);

LatLon convertHorizontalToEquatorial(double julianDate, const AzimuthElevation& azimuthElevation, const LatLon& observer);

LatLon calcSunEclipticPosition(double julianDate);

LatLon calcMoonEclipticPosition(double julianDate);

//! @returns phase in range [0,1] where 0 and 1 are new moon, 0.5 is full moon
double calcMoonPhase(double julianDate);

} // namespace skybolt
} // namespace sim