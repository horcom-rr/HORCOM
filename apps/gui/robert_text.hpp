// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QString>

#include "horcom/time/calendar.hpp"

// The text forms of his screens, ported from the original HORCOM
// procedures date_form, grmise, homise and grze. The rounding carries
// run through every unit, his whole minutes and hundredth seconds could
// read 60 without one.
namespace horcom {

/// His datum3$ of date_form with vchr.
///
/// @param d the date, astronomical year
/// @return day and month right aligned, the year historically counted
///         with three blanks or vC behind it
[[nodiscard]] QString datum3_text(const CalendarDate& d);

/// His datum$ of date_form, the short form of his result tables.
///
/// @param d the date, astronomical year
/// @return day, month and the last two digits of the year, each right
///         aligned in two places, a v behind years before Christ
[[nodiscard]] QString datum_text(const CalendarDate& d);

/// His grmise$ of grmise(gd,0).
///
/// @param deg the angle in degrees, the sign is dropped like his ABS
/// @return whole degrees, minutes and rounded seconds, "%3d°%2d'%2d\""
[[nodiscard]] QString grmise_text(double deg);

/// His grmi$ of grmise.
///
/// @param deg      the angle in degrees, the sign is dropped
/// @param decimals 0 for whole minutes, 1 for tenths
/// @return whole degrees and minutes
[[nodiscard]] QString grmi_text(double deg, int decimals);

/// His homise$ of homise(w,d), an angle read as a clock.
///
/// @param deg      the angle in degrees, fifteen to the hour
/// @param decimals decimals of the seconds
/// @return hours, minutes and seconds, "%2dh %2dm %2ds"
[[nodiscard]] QString homise_text(double deg, int decimals);

/// The zodiac form of a longitude, degree, sign tag, minute and second.
///
/// @param rad the longitude in radians
/// @return "%2d TAG %02d'%02d\"", the rounded second carries into the
///         next sign
[[nodiscard]] QString zodiac(double rad);

/// @return today's date of the system clock at midnight, the date his
///         input boxes open with
[[nodiscard]] CalendarDate today_date();

}  // namespace horcom
