// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/ephemerides.hpp"
#include "horcom/chart/houses.hpp"
#include "horcom/chart/settings.hpp"
#include "horcom/ephem/moon.hpp"
#include "horcom/ephem/sunmoon.hpp"
#include "horcom/ephem/vsop.hpp"

// The chart pipeline, the C++ form of the original chain jseckp, a60 and
// a9 with a90. Houses run in Universal Time, the bodies run in Ephemeris
// Time, exactly like the original. One faithful quirk matters, the
// original's sidt leaves the 0h UT nutation state behind in the globals,
// so angles and house cusps see the true obliquity of MIDNIGHT while the
// bodies see the one of the epoch. The pipeline reproduces that.
namespace horcom {

/// Input of one chart, the clock already normalised to UT.
struct ChartInput {
  CalendarDate date_ut;
  double lon_deg_east = 0.0;  // the original gl, east positive
  double lat_deg = 0.0;       // the original gg, north positive
};

/// One body slot of the result, mirroring the original element arrays.
struct BodyState {
  bool present = false;  // slot is active in this chart
  bool valid = false;    // false outside an ephemeris span, jdplanetex
  double el = 0.0;       // geocentric apparent ecliptic longitude
  double eb = 0.0;       // latitude
  double ar = 0.0;       // right ascension
  double de = 0.0;       // declination
  double dr = 0.0;       // geocentric distance, AU
  double tb = 0.0;       // daily motion, negative when retrograde
  double ttb = 0.0;      // change of the daily motion
  double hel = 0.0;      // heliocentric longitude
  double heb = 0.0;      // heliocentric latitude
  double r = 0.0;        // heliocentric radius, AU
};

/// A computed chart.
struct Chart {
  bool ok = false;
  double jd_ut = 0.0;
  double delt_minutes = 0.0;  // delta T used
  double jd_et = 0.0;
  double h0 = 0.0;            // sidereal time at 0h UT, hours
  double hs = 0.0;            // sidereal time of the moment, hours
  double armc_deg = 0.0;      // the original armc in degrees
  TimeArguments ta;           // time arguments of the ET epoch
  SunMoonState smo;           // somo state of the ET epoch
  double ekls0 = 0.0;         // true obliquity at 0h UT, the house epoch
  Houses houses;
  MoonPosition moon;
  LunarPoints lunar;
  std::array<BodyState, body::kSlotCount> b{};
};

/// Computes a geocentric chart.
///
/// @param in    date in UT, place with east positive longitude
/// @param s     calculation settings
/// @param vsop  loaded VSOP term tables
/// @param eph   ephemeris directory cache
/// @return the chart, ok false when the house guard refuses the latitude
/// @note the heliocentric mode of the original is not ported yet
[[nodiscard]] Chart compute_chart(const ChartInput& in, const ChartSettings& s, const VsopTables& vsop, const Ephemerides& eph);

}  // namespace horcom
