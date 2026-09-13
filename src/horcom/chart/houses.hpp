// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <string_view>

#include "horcom/chart/settings.hpp"

// The angles and house systems. Ported from the original HORCOM
// procedures eckp, eckp1, the dispatcher a60 with its maxbreit guard, and
// the system routines plac, topo, koch, regio, camp, aeqe, vehlow and
// their normaliser regio0. All angles radians, geographic latitude enters
// in degrees like the original global gg.
namespace horcom {

/// The four chart angles from the original eckp1.
struct Angles {
  double ac = 0.0;      // f(1), also pl(13)
  double dc = 0.0;      // f(7)
  double mc = 0.0;      // f(10), also pl(14)
  double ic = 0.0;      // f(4)
  double vertex = 0.0;  // vert(od,ze)
};

/// Cusps and metadata of one house computation.
struct Houses {
  /// f(1..12) at indices 1..12, index 13 carries the AC copy the original
  /// stores at f(13), index 0 unused
  std::array<double, 14> cusp{};
  Angles angles;
  std::string_view name;  // the original haus$
  bool ok = false;        // false when the maxbreit guard refuses
};

/// The original eckp1, ascendant, midheaven and vertex.
///
/// @param armcb   right ascension of the midheaven, radians
/// @param lat_deg geographic latitude in degrees like the original gg
/// @param ekls    true obliquity of date, in the original pipeline the
///                value left behind by sidt at 0h UT
/// @return the four angles and the vertex
[[nodiscard]] Angles chart_angles(double armcb, double lat_deg, double ekls);

/// Computes the house cusps for a system, the original a60 dispatch.
///
/// @param system  house system
/// @param armcb   right ascension of the midheaven, radians
/// @param lat_deg geographic latitude in degrees
/// @param ekls    true obliquity, same epoch convention as chart_angles
/// @return cusps with angles, ok false when the original maxbreit guard
///         refuses Placidus or Koch beyond 90 degrees minus the obliquity
[[nodiscard]] Houses compute_houses(HouseSystem system, double armcb, double lat_deg, double ekls);

}  // namespace horcom
