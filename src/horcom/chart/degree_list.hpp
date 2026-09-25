// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string>
#include <vector>

#include "horcom/chart/chart.hpp"

// The GRAD-LISTE of the original grli. Every point of the chart with
// its degree, on demand the intermediate cusps, then the midpoint of
// every pair, in the order his first pass printed them.
namespace horcom {

/// One entry of the list, his q$ with the degree it stands on.
struct GradEntry {
  /// the ecliptic degree, 0 to 360
  double deg = 0.0;
  /// his q$, the degree text and the tag or the pair
  std::string text;
};

/// Builds the list in the order of his unsorted pass.
///
/// @param chart the computed chart
/// @param s     its settings, the extras and the hrg mode
/// @param cusps the ZWISCHEN - HÄUSER answer, the cusps H2 to H12
///              without the MC join the list and the pairs
/// @return the entries, bodies first, then cusps, then midpoints
/// @note Without extra bodies the four cardinal points stand as midpoint
///       partners but never in the list. His 0 Aries sat at kk and fell
///       under the pl > kk guard, the port pairs it like the other three.
///       A pair takes the precision of its first point, two decimals for
///       SO to MC, AG, GL, the cusps and the extras his packed layout put
///       on the indices 33 to 45, one decimal for the cardinal points and
///       the other extras.
[[nodiscard]] std::vector<GradEntry> grad_list(const Chart& chart, const ChartSettings& s, bool cusps);

/// The height unit of the distribution panel, his h = INT(180 / bb&).
///
/// @param s the settings, the extras widen the list and the hrg mode
///          doubles the unit
/// @return pixels per entry and degree
[[nodiscard]] int grad_unit(const ChartSettings& s);

}  // namespace horcom
