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

/// Greenwich MEAN sidereal time at 0h UT, the original h0.
///
/// @param jd0 Julian date of 0h UT of the day, from julian_day with hour
///            and minute zero and the SAME calendar override as the chart
/// @return sidereal time in hours
[[nodiscard]] double gmst0_hours(double jd0);

/// Convenience overload that rewinds the date to midnight itself.
///
/// @param d   the calendar date, its clock time is ignored
/// @param cal calendar override of the chart
/// @return sidereal time at 0h UT in hours
[[nodiscard]] double gmst0_hours(const CalendarDate& d, Calendar cal = Calendar::kAuto);

//RR wahre Sternzeit
/// The stzw& = 1 correction from mean to apparent sidereal time.
///
/// @param h0_mean mean sidereal time in hours
/// @param dpsi    nutation in longitude, radians, evaluated at 0h UT like
///                the original does inside sidt
/// @param ekls    true obliquity, radians, same epoch
/// @return apparent sidereal time in hours
[[nodiscard]] double apparent_sidereal_hours(double h0_mean, double dpsi, double ekls);

/// Sidereal time at the moment, the original hs.
///
/// @param h0                    sidereal time at 0h UT in hours
/// @param hours_since_midnight  ho + mi / 60 in UT
/// @return sidereal time in hours
[[nodiscard]] double sidereal_at_hours(double h0, double hours_since_midnight);

}  // namespace horcom
