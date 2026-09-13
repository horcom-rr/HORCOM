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

/// The three moments of one body over one day.
struct RiseSet {
  bool ok = false;
  /// the body never crosses the horizon that day
  bool circumpolar = false;
  double jd_rise_ut = 0.0;
  double jd_transit_ut = 0.0;
  double jd_set_ut = 0.0;
};

/// Computes rising, transit and setting for the day.
///
/// @param jd_day_ut any moment of the wanted day, truncated to zero
///                  hours Universal Time
/// @param slot      the body, sun and moon through the extras, the
///                  angles and hypothetical points have no horizon
/// @param true_position true drops the refraction depth like his
///                  WAHRE POSITION answer
/// @param ctx       observer and settings, the place decides
/// @return the three clocks in UT
[[nodiscard]] RiseSet rise_transit_set(double jd_day_ut, int slot, bool true_position, const SearchContext& ctx);

}  // namespace horcom
