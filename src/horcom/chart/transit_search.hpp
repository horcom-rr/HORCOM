// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/chart/chart.hpp"

// The exact hit search of the original, plant with its evaluation kernel
// plant1. Given a start moment it walks backward in time with per body
// step sizes, brackets the crossing with his branch reconcilers, then
// converges with his damped secant and one linear interpolation. The
// original states the remaining time error is mostly under five seconds.
namespace horcom {

/// One longitude evaluation, the plant1 view of a body.
struct BodyLongitude {
  bool valid = false;
  double el = 0.0;  // apparent ecliptic longitude, normalised radians
  double tb = 0.0;  // daily motion, negative when retrograde
};

/// The fixed observer of a search, the original globals around plant.
struct SearchContext {
  ChartInput base;        // place of the observer, date part is ignored
  ChartSettings settings; // parallax and calendar rule the evaluations
  const VsopTables* vsop = nullptr;
  const Ephemerides* eph = nullptr;
};

/// A found crossing.
struct LongitudeCrossing {
  bool ok = false;
  double jd_ut = 0.0;
  bool retrograde = false;  // the body ran backward through the point
};

/// Evaluates one body like plant1.
///
/// @param jd_ut the moment in UT
/// @param slot  body slot, 1 Sun through the extra bodies
/// @param ctx   observer and settings
/// @return longitude and daily motion, valid false outside an ephemeris
[[nodiscard]] BodyLongitude body_longitude(double jd_ut, int slot, const SearchContext& ctx);

/// Searches backward from a start moment for the body reaching the
/// target longitude, the first passage of the original plant.
///
/// @param jd_start_ut search starts here and walks into the past
/// @param slot        body slot
/// @param target_rad  the longitude to reach, radians
/// @param ctx         observer and settings
/// @return the crossing, ok false when an ephemeris ends or no crossing
///         converges within the step budget
[[nodiscard]] LongitudeCrossing find_longitude_backward(double jd_start_ut, int slot, double target_rad, const SearchContext& ctx);

}  // namespace horcom
