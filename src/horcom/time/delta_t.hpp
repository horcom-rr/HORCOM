// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

// Delta T, the difference between Ephemeris Time and Universal Time.
// Ported from the original HORCOM procedures utet, utet1 and etut, table
// values from the initialisation procedure DIM. All planet computation in
// HORCOM runs in ET, houses run in UT.
namespace horcom {

/// original constant in utet1 and etut, Robert Rettig's rounded 1/1440
inline constexpr double kDeltaTDaysPerMinute = 0.0006944444444;

/// The original utet.
///
/// @param jd epoch in UT
/// @return delta T in MINUTES like the original delt, table interpolation
///         1620 to 2008, polynomial extrapolation outside
[[nodiscard]] double delta_t_minutes(double jd);

/// The original utet1, UT in ET.
///
/// @param jd_ut epoch in UT
/// @return the same epoch in Ephemeris Time
[[nodiscard]] double ut_to_et(double jd_ut);

/// The original etut applied with the delt of the forward step, matching
/// the pipeline of a90 where one delt value brackets the computation.
///
/// @param jd_et        epoch in Ephemeris Time
/// @param delt_minutes the delta T used on the way in
/// @return the epoch back in UT
[[nodiscard]] double et_to_ut(double jd_et, double delt_minutes);

}  // namespace horcom
