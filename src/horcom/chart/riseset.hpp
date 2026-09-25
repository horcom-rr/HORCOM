// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/chart/transit_search.hpp"

// Rising, meridian passage and setting, ported from the original
// auf_unt. Three daily positions bracket the day, the standard hour
// angle opens the first guesses and the interpolation with the
// altitude correction iterates onto the exact clock, the sun with its
// refraction depth, the moon with its own parallax, recomputed as the
// iteration moves.
namespace horcom {

/// One of the three moments with the body as his auf_unt3 block shows it.
struct RiseSetMoment {
  /// the iteration closed, his block stays empty otherwise
  bool ok = false;
  double jd_ut = 0.0;
  /// Greenwich sidereal time of the moment in degrees, his tee
  double gst_deg = 0.0;
  /// the body at the moment, geocentric without parallax, radians
  double el = 0.0;
  double eb = 0.0;
  double ar = 0.0;
  double de = 0.0;
};

/// The three moments of one body over one day.
struct RiseSet {
  /// the positions of the day came out
  bool ok = false;
  /// the body never crosses the horizon that day, his AUßER BEREICH
  bool circumpolar = false;
  RiseSetMoment rise;
  RiseSetMoment transit;
  RiseSetMoment set;
};

/// Computes rising, transit and setting for the day.
///
/// @param jd_day_ut any moment of the wanted day, truncated to zero
///                  hours Universal Time
/// @param slot      the body, sun and moon through the extras, the
///                  angles and hypothetical points have no horizon
/// @param true_position true takes the true positions and drops the
///                  refraction depth like his WAHRE POSITION answer, the
///                  moon keeps its own standard altitude either way
/// @param ctx       observer and settings, the place decides
/// @return the three moments in UT
[[nodiscard]] RiseSet rise_transit_set(double jd_day_ut, int slot, bool true_position, const SearchContext& ctx);

}  // namespace horcom
