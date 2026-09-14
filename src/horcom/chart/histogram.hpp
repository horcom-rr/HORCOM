// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>

#include "horcom/chart/chart.hpp"

// The element and quality histograms of the chart view. Every factor
// scores its point weight into the element and the quality of its sign
// and of its house, planets standing in the first house and the birth
// ruler count double on demand. Ported from HORCOM elem1, elem2,
// kard_fix_gem and kard_fix_gemh.
namespace horcom {

/// The switches of his histogram weighting dialog.
struct HistogramOptions {
  /// point weight per body slot, his pn table fanned onto the slots
  std::array<int, body::kSlotCount> points{};
  //RR 1.Haus doppelt
  bool double_first_house = true;
  //RR GebHerr doppelt
  bool double_ruler = true;
  /// the classical rulers instead of the modern ones
  bool classic_rulers = false;
};

/// The counted columns, index 1 through 4 for fire earth air water and
/// 1 through 3 for cardinal fixed mutable.
struct Histogram {
  std::array<int, 5> element_sign{};
  std::array<int, 5> element_house{};
  std::array<int, 4> quality_sign{};
  std::array<int, 4> quality_house{};
  /// false when the house columns stayed uncounted, his haw guard
  bool houses_counted = false;
};

/// Expands the stored weight row onto the body slots. Entries 1 to 14
/// weigh their slot, entry 15 weighs every extra body, all zero falls
/// back to his old fixed values.
///
/// @param pn the konsta row, index 1 through 15
/// @return one weight per body slot
[[nodiscard]] std::array<int, body::kSlotCount> histogram_points(const std::array<int, 16>& pn);

/// Counts the four histograms over a computed chart.
///
/// @param chart the chart with houses where the house columns count
/// @param s     the settings, heliocentric changes the skips
/// @param opt   weights and doubling switches
/// @return the counted columns
[[nodiscard]] Histogram chart_histogram(const Chart& chart, const ChartSettings& s, const HistogramOptions& opt);

}  // namespace horcom
