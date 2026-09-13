// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/time/calendar.hpp"

// Mean elements of Sun and Moon, nutation and the true obliquity.
// Ported from the original HORCOM procedures somo and zeitgleichung.
// somo runs after every juld1 in the original and its outputs feed the
// whole chain, so this struct is the shared state of a computation epoch.
// All angles radians.
namespace horcom {

struct SunMoonState {
  // Sun, indices 1 in the original element arrays
  double sun_mel = 0.0;   // mean longitude
  double sun_man = 0.0;   // mean anomaly
  double sun_e = 0.0;     // eccentricity
  double sun_a = 0.0;     // semi major axis
  double sun_p = 0.0;     // longitude of perigee, the 282 degree value
  // Moon, indices 2
  double moon_mel = 0.0;  // mean longitude
  double moon_man = 0.0;  // mean anomaly
  double moon_o = 0.0;    // mean ascending node
  double moon_dm = 0.0;   // mean elongation D
  double moon_fm = 0.0;   // argument of latitude F
  double moon_i = 0.0;    // inclination with his periodic term
  double moon_a = 0.0;    // 0.002567555 AU
  // nutation and obliquity
  double dpsi = 0.0;
  double deps = 0.0;
  double ekls = 0.0;      // TRUE obliquity, mean value plus deps
  // mean lunar node as displayed, o(2) + dpsi
  double mean_node = 0.0;
  // node of the solar equator, the original gom(1)/goms(1), from the
  // calendar year fraction
  double sun_node = 0.0;
  double sun_node_opp = 0.0;
  bool heliocentric = false;
};

/// The original somo, the shared per epoch state.
///
/// @param t            time arguments of the epoch
/// @param date         calendar date, enters only through the solar
///                     equator node formula
/// @param heliocentric zeroes the nutation and skips the Moon like the
///                     original hrg! branch
/// @return mean elements, nutation and the true obliquity
[[nodiscard]] SunMoonState somo(const TimeArguments& t, const CalendarDate& date, bool heliocentric = false);

/// The original zeitgleichung.
///
/// @param s the somo state of the epoch
/// @return the equation of time in radians, callers convert with
///         kRadToDeg / 15 to hours
[[nodiscard]] double equation_of_time(const SunMoonState& s);

}  // namespace horcom
