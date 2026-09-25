// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <utility>
#include <vector>

#include "horcom/chart/transit_search.hpp"

// The Dynamogramm after Krafft and Goerner, ported from the original
// huber module. Progressed positions walk one day per year of life.
// Wherever a running point enters the orb of a radix point's aspect,
// or of another running point, an arc of intensity spans the days
// between orb entry and exit, a cosine half wave or a bell curve,
// signed by the valuation the tradition gives the pair and the angle,
// weighted by the running point's amplitude. All arcs sum into two
// curves over the life axis, the ground mood and the existential line
// of the angles.
namespace horcom {

/// The knobs of one Dynamogramm run, his question chain.
struct DynamogramOptions {
  /// the first year of life on the visible axis
  double from_age = 0.0;
  /// true keeps the fast moon among the running points
  bool with_moon = false;
  /// true also runs the regressive direction and adds its arcs
  bool regressive = false;
  /// true shapes the arcs as bell curves, cosine half waves otherwise
  bool gauss = false;
  /// true opens the half square and half sextile families
  bool classic_minors = false;
  /// true opens the quincunx pair alone
  bool quincunx = false;
};

/// One arc as his EINZEL - BÖGEN screen shows it.
struct DynamogramArc {
  int pl = 0;               ///< the running point
  int rd = 0;               ///< the radix or running partner
  int na = 1;               ///< the divisor
  int ma = 1;               ///< the multiple
  int i1 = 0;               ///< first curve sample
  int i2 = 0;               ///< last curve sample
  double tb = 0.0;          ///< the arc centre in sample days
  bool radix = true;        ///< against a radix factor, else among the running
  bool regressive = false;  ///< from the regressive run
  std::vector<double> bog;  ///< his bog(i1..i2), the unscaled arc
};

/// The two summed curves. One hundred twenty samples span one year,
/// the visible window of the original starts at index three thousand,
/// the first year of life asked for.
struct Dynamogram {
  std::vector<double> existential;  // the arcs of the angles
  std::vector<double> mood;         // everything else
  double from_age = 0.0;
  /// the arcs that touch the visible window, in the order he drew them
  std::vector<DynamogramArc> arcs;
};

/// the first sample of the visible window, his na& = 25 * 120
inline constexpr int kDynamogramWindowStart = 3000;
/// the last sample of the visible window, his ne& = na& + 598
inline constexpr int kDynamogramWindowEnd = 3598;
/// samples per year of life, one progressed day
inline constexpr int kDynamogramPerYear = 120;
/// the end of the single arc screen, five years past the window start,
/// his 3600 of einzel_bogen_anz
inline constexpr int kDynamogramArcWindowEnd = kDynamogramWindowStart + 5 * kDynamogramPerYear;

/// The pairs of running points whose mutual arcs the Dynamogramm adds,
/// ported from asp_analy_mund.
///
/// @return first and second slot, Sun and Mercury to Pluto against
///         Mercury to Pluto, AC and MC, never the Moon
[[nodiscard]] std::vector<std::pair<int, int>> dynamogram_mutual_pairs();

/// The MITTEL line of hubausg, the mean of both curves over the fifty
/// years. His integer mittel% took the samples outside the window without
/// the amplitude factor and kept the value of the previous run, the port
/// scales every sample and starts afresh.
///
/// @param d     the curves
/// @param scale his amplitude factor f, 0.3
/// @return the sum of existential plus mood over the samples 0 to 6000
///         divided by 6000 like his mittel% / 6000, the time mean over
///         fifty years of one hundred twenty samples
[[nodiscard]] double dynamogram_mean(const Dynamogram& d, double scale);

/// Runs the Dynamogramm over a radix.
///
/// @param radix the birth chart
/// @param opt   the run's switches
/// @param ctx   observer and settings for the progressed positions
/// @return the summed curves
[[nodiscard]] Dynamogram dynamogram(const Chart& radix, const DynamogramOptions& opt, const SearchContext& ctx);

}  // namespace horcom
