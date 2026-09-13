// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <vector>

#include "horcom/chart/chart.hpp"

// Symbolic and primary directions, ported from the original asymb,
// ar_sys and aprim world. A direction turns arcs into years of life,
// every degree a body must travel to reach an aspect of another radix
// point becomes one year under the classic key, and the found arcs
// list as ages. The symbolic variants measure the arc along the
// ecliptic, the equator or the mundane house space, the primary
// directions of the E.C. Kühr school measure it in oblique ascension
// under each significator's own pole.
namespace horcom {

/// Which frame the arcs are measured in.
enum class DirectionMethod {
  /// ecliptic longitudes, his ekliptikale symbolische Direktion
  kSymbolicEcliptic,
  /// right ascensions, the AR system
  kSymbolicEquatorial,
  /// mundane house space positions
  kSymbolicMundane,
  /// oblique ascension under the own pole, the Kühr primary
  kPrimary,
};

/// What stands beside the bodies as targets on slots 15 to 18.
enum class DirectionExtras {
  kNone,
  /// the intermediate cusps of houses two, three, five and six
  kCusps,
  /// the four cardinal points, zero Aries to zero Capricorn
  kCardinal,
};

/// One found direction, an arc that became an age.
struct DirectionHit {
  /// the moving significator, a body slot, 13 and 14 the angles, 15 to
  /// 18 the extras when they are switched on
  int directed = 0;
  /// the radix point it reaches
  int target = 0;
  /// which multiple of the base angle, zero the conjunction
  int multiple = 0;
  /// the arc in degrees
  double arc_deg = 0.0;
  /// the age the key makes of the arc
  double years = 0.0;
  /// true for the converse direction, his K against D
  bool converse = false;
};

/// The inputs of the direction run, his a18eing panel.
struct DirectionRange {
  /// the age window in years, hits outside stay silent
  double from_years = 0.0;
  double to_years = 90.0;
  /// the aspect grid in degrees, his w4d, thirty by default
  double base_angle_deg = 30.0;
  /// years per degree of arc, the classic key one
  double key = 1.0;
  /// aspect points of bodies with latitude keep a faded latitude in
  /// the primary frame, his mit Breite switch
  bool with_latitude = true;
  DirectionExtras extras = DirectionExtras::kNone;
};

/// Runs one direction method over a chart and lists every arc inside
/// the age window, both ways, direct and converse.
///
/// @param chart   the radix
/// @param method  the measuring frame
/// @param range   window, grid, key and extras
/// @param lat_deg latitude for the mundane and primary frames
/// @return the hits in scan order
[[nodiscard]] std::vector<DirectionHit> direction_hits(const Chart& chart, DirectionMethod method, const DirectionRange& range, double lat_deg);

}  // namespace horcom
