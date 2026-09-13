// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/time/calendar.hpp"

// Sidereal time. Ported from the original HORCOM procedure sidt. The
// original temporarily rewinds the clock to 0h UT of the calendar day,
// recomputes jd and the time arguments there,
// evaluates GMST at 0h and then advances to the moment with the solar to
// sidereal rate. Note that sidt leaves the nutation state of 0h behind in
// the globals, the chart pipeline recomputes it right afterwards.
namespace horcom {

// h0, Greenwich MEAN sidereal time at 0h UT in hours. jd0 must be the
// Julian date of 0h UT of the day, obtained via julian_day with hour and
// minute zero and the SAME calendar override as the chart.
[[nodiscard]] double gmst0_hours(double jd0);

// convenience overload building jd0 from the calendar date
[[nodiscard]] double gmst0_hours(const CalendarDate& d, Calendar cal = Calendar::kAuto);

//RR wahre Sternzeit
// the stzw& = 1 correction from mean to apparent sidereal time. dpsi is the
// nutation in longitude in radians and ekls the true obliquity in radians,
// both evaluated at 0h UT like the original does inside sidt.
[[nodiscard]] double apparent_sidereal_hours(double h0_mean, double dpsi, double ekls);

// hs, sidereal time at the moment. hours_since_midnight is ho + mi / 60 in UT.
[[nodiscard]] double sidereal_at_hours(double h0, double hours_since_midnight);

}  // namespace horcom
