// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/chart/chart.hpp"
#include "horcom/chart/transit_search.hpp"

// KORREKTUR, the birth time rectification of the DIVERSES menu. Ported
// from the original HORCOM procedures korr, korr0, korr1, korr10, korr2,
// korr21 and korh. The angle modes solve in sidereal time space first and
// turn the found sidereal angle into a clock of the radix date, the two
// lights search their longitude near the birth like plant.
namespace horcom {

/// What the birth time is corrected with, his menu of korr.
enum class CorrectionTarget {
  kSiderealTime,  // local sidereal time in hours, STERNZEIT
  kMc,
  kAc,
  kCusp,          // an intermediate house cusp, ZWISCHEN-HÄUSERN
  kSun,
  kMoon,
};

/// One correction request.
struct CorrectionRequest {
  CorrectionTarget target = CorrectionTarget::kMc;
  /// the wanted value, hours of local sidereal time for kSiderealTime,
  /// ecliptic longitude in radians otherwise
  double value = 0.0;
  /// the house for kCusp, never 1, 4, 7 or 10
  int cusp = 2;
};

/// The corrected moment.
struct CorrectionResult {
  bool ok = false;
  double jd_ut = 0.0;
  /// the damped walk did not settle, his Spanne zu groß
  bool span_too_large = false;
};

/// The damped walk of korr1 stops within this distance of the wanted
/// angle, in radians.
inline constexpr double kCorrectionTolerance = 5.0e-7;

/// The clock of the radix date whose local sidereal angle equals armcb,
/// ported from korr21.
///
/// @param radix         the chart whose date and midnight sidereal time serve
/// @param armcb         local sidereal angle in radians
/// @param lon_deg_east  geographic longitude of the birth place
/// @return Julian day UT on the radix date
[[nodiscard]] double moment_for_armc(const Chart& radix, double armcb, double lon_deg_east);

/// The birth time that brings the chosen factor onto the wanted value.
///
/// @param radix  the uncorrected chart
/// @param q      what to correct with and the wanted value
/// @param ctx    the radix place and the settings of the search
/// @return the corrected moment, ok false when nothing converged
/// @note the cusp mode works on Placidus cusps whatever system the chart
///       uses, his plre switched to Placidus for the whole correction
[[nodiscard]] CorrectionResult correct_birth_time(const Chart& radix, const CorrectionRequest& q,
                                                  const SearchContext& ctx);

}  // namespace horcom
