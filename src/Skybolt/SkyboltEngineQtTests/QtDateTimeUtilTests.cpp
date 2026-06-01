/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>

#include <SkyboltEngineQt/Util/QtDateTimeUtil.h>

#include <QDateTime>
#include <QTimeZone>

TEST_CASE("julianDateToQDateTime returns correct date and time")
{
	// Use a well-known value: JD 2451545.0 = J2000.0 = 2000-01-01 12:00:00 UTC
	const double j2000 = 2451545.0;
	QDateTime dt = julianDateToQDateTime(j2000);

	REQUIRE(dt.isValid());
	CHECK(dt.date().year() == 2000);
	CHECK(dt.date().month() == 1);
	CHECK(dt.date().day() == 1);
	CHECK(dt.time().hour() == 12);
	CHECK(dt.time().minute() == 0);
	CHECK(dt.time().second() == 0);
	CHECK(dt.timeZone() == QTimeZone::utc());
}

TEST_CASE("qdateTimeToJulianDate returns correct Julian date for known date")
{
	// 2018-03-02 18:00:00 UTC = JD 2458180.25
	QDateTime dt(QDate(2018, 3, 2), QTime(18, 0, 0), QTimeZone::utc());
	double jd = qdateTimeToJulianDate(dt);

	CHECK(jd == Approx(2458180.25).margin(1e-5));
}

TEST_CASE("julianDateToQDateTime and qdateTimeToJulianDate are inverse operations")
{
	const double originalJd = 2458180.25;
	QDateTime dt = julianDateToQDateTime(originalJd);
	double roundTripJd = qdateTimeToJulianDate(dt);

	CHECK(roundTripJd == Approx(originalJd).margin(1e-5));
}
