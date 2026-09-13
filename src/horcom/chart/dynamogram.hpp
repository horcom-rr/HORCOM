// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

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

/// The two summed curves. One hundred twenty samples span one year,
/// the visible window of the original starts at index three thousand,
/// the first year of life asked for.
struct Dynamogram {
  std::vector<double> existential;  // the arcs of the angles
  std::vector<double> mood;         // everything else
  double from_age = 0.0;
};

/// Runs the Dynamogramm over a radix.
///
/// @param radix the birth chart
/// @param opt   the run's switches
/// @param ctx   observer and settings for the progressed positions
/// @return the summed curves
[[nodiscard]] Dynamogram dynamogram(const Chart& radix, const DynamogramOptions& opt, const SearchContext& ctx);

}  // namespace horcom
