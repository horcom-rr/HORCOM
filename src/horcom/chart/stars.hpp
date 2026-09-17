// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string_view>
#include <vector>

#include "horcom/chart/chart.hpp"

// The fixed stars of the original stella screen, sixty two entries he
// collected himself, each with the planetary quality the tradition
// assigns and the distance in light years. Positions move linearly
// from their catalogue epoch, the star clusters and the galactic
// points precess properly, and annual aberration rides on the chart's
// sun before the ecliptic longitude forms.
namespace horcom {

/// One star against the chart.
struct StarRow {
  std::string_view name;
  /// the planetary quality letters of his table
  std::string_view quality;
  /// the astronomical designation
  std::string_view astro;
  /// distance in light years, zero where his table says NN
  int lightyears = 0;
  /// ecliptic longitude and latitude of date, radians
  double la = 0.0;
  double br = 0.0;
  /// right ascension and declination of date, radians
  double ar = 0.0;
  double de = 0.0;
  /// aspects to the chart, one letter per hit like his columns, K
  /// conjunction, O opposition, Q square, T trine, with the body tag
  std::vector<std::pair<int, char>> aspects;
};

/// Computes the star table over a chart, the original stella with the
/// aspect matching of stelk.
///
/// @param chart the computed chart, its sun feeds the aberration
/// @param orb   the orb factor, the windows span two degrees times it
///              for the conjunction and shrink by his divisors
/// @return the sixty two stars in his screen order
[[nodiscard]] std::vector<StarRow> fixed_stars(const Chart& chart, double orb);

/// The aspect scan of HORCOM stelk over one ecliptic longitude,
/// conjunction, opposition, square and trine against every present
/// body except the descending node, the sensitive points use it too.
///
/// @param chart the computed chart
/// @param la    the point's ecliptic longitude, radians
/// @param orb   the orb factor, windows like fixed_stars
/// @return per hit the body slot and his letter K O Q T
[[nodiscard]] std::vector<std::pair<int, char>> point_aspects(const Chart& chart, double la, double orb);

}  // namespace horcom
