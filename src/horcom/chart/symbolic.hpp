// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <vector>

#include "horcom/chart/chart.hpp"
#include "horcom/chart/transit_search.hpp"

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

/// One found direction, an arc that became an age.
struct DirectionHit {
  /// the moving significator, a body slot, 13 and 14 the angles, 15 to
  /// 18 the cardinal points 0 AR to 0 CP
  int directed = 0;
  /// an intermediate cusp 2, 3, 5 or 6 as the significator, directed 0
  int directed_cusp = 0;
  /// the radix point it reaches
  int target = 0;
  /// the second factor when the target is a midpoint
  int target2 = 0;
  /// an intermediate cusp as the promissor of the primary, target 0
  int target_cusp = 0;
  /// which multiple of the base angle, zero the conjunction
  int multiple = 0;
  /// the arc in degrees
  double arc_deg = 0.0;
  /// the age the key makes of the arc
  double years = 0.0;
  /// true for the converse direction, his K against D
  bool converse = false;
};

/// The inputs of the direction run, his a18eing answers and the
/// switches of VORGABEN DIREKTIONEN.
struct DirectionRange {
  /// the age window in years, hits outside stay silent
  double from_years = 0.0;
  double to_years = 90.0;
  /// the aspect grid in degrees, his w4d, thirty by default
  double base_angle_deg = kDefaultBaseAngleDeg;
  /// the combined GRUND-ASPEKT filter over the base angle
  AspectGrid grid = AspectGrid::kPlain;
  /// years per degree of arc, the classic key one, NAIBOD 1.0146
  double key = 1.0;
  /// PROMISSOREN mit Breite, the faded latitude of the aspect points
  /// in the primary frame, his brep
  bool with_latitude = true;
  /// SIGNIFIKAT. MIT Breite, his bres
  bool significator_latitude = true;
  /// DIREKTIONEN MIT ZWISCHENHÄUSERN, the cusps 2, 3, 5 and 6 direct
  bool house_targets = false;
  /// MIT KARDINAL-PUNKTEN, the ecliptic methods reach 0 AR to 0 CP
  bool cardinal_targets = false;
  /// his pl1, the first body of the walk, 5 from Mars, 6 from Jupiter
  int first_slot = 0;
  /// NUR DIESE DARSTELLEN, the chosen significators, empty runs all
  std::vector<int> chosen;
  /// his halbs_dir for the ecliptic frame, 1 every midpoint, 2 only
  /// those the directed body is no factor of
  int midpoints = 0;
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
