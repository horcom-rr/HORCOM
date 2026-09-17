// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/time/sidereal.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"

namespace horcom {

double gmst0_hours(double jd0) {
  const double t1 = (jd0 - kJdEpoch1900) / kDaysPerCentury;
  const double t2 = t1 * t1;
  // the cubic term runs on t = t1 - 1, centuries from J2000, exactly as
  // the original sidt does
  const double t = t1 - 1.0;
  return norm_hours(6.6460656 + 2400.051262 * t1 + 0.00002581 * t2 - 1.72222e-09 * t * t * t);
}

double gmst0_hours(const CalendarDate& d, Calendar cal) {
  CalendarDate midnight = d;
  midnight.hour = 0.0;
  midnight.minute = 0.0;
  return gmst0_hours(julian_day(midnight, cal));
}

double apparent_sidereal_hours(double h0_mean, double dpsi, double ekls) {
  // kRadToRaHours converts the equation of the equinoxes from radians to
  // hours of right ascension, twelve over pi as one literal in the original
  return norm_hours(h0_mean + dpsi * kRadToRaHours * std::cos(ekls));
}

double sidereal_at_hours(double h0, double hours_since_midnight) {
  return norm_hours(h0 + hours_since_midnight * kSolarToSiderealRate);
}

}  // namespace horcom
