// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string>
#include <vector>

#include "horcom/chart/chart.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/render/wheel.hpp"

// The LINEAR-GRAPHIK, the life diagram in the manner of R. Ebertin.
// Every radix factor becomes a horizontal line at its position folded
// by the chosen base angle, the running bodies trace curves over a
// time axis, and every crossing of a curve with a line is a direction
// or transit becoming exact. Ported from HORCOM a18_lin, a180trpr_lin,
// a18_line_col, skalh, skalh1, skalh_gitter, skalv, mark_jahr and the
// lin! branch of a18kopf.
namespace horcom {

/// The band of the 640 by 460 sheet, 360 pixels high between these
/// rows and framed between these columns. The reading lines of the
/// screen share them.
inline constexpr double kLinearBandTop = 60.0;
inline constexpr double kLinearBandBottom = 420.0;
inline constexpr double kLinearFrameLeft = 18.0;
inline constexpr double kLinearFrameRight = 622.0;

/// Which movement the curves follow.
enum class LinearKind {
  /// the running sky in real time
  kTransits,
  /// the secondary progression, one day per tropical year
  kSecondary,
  /// the sun arc, every radix point shifted by the progressed sun
  kSunArc,
  /// the moon arc, his TERTIÄR 2, many hits
  kMoonArc,
  /// the running bodies among themselves, the MUNDAN-ASPEKTE of mund,
  /// no radix lines
  kMundane,
};

/// One exact hit on the graph, his a180aus marks.
struct LinearHit {
  /// the moment in real time, dates for transits, the life moment for
  /// the directions
  double jd_ut = 0.0;
  /// the running body and the radix factor it meets, for a mundane hit
  /// the second and the first running body
  int running = 0;
  int radix = 0;
  /// an intermediate cusp 2, 3, 5 or 6 when the factor is a house
  int cusp = 0;
  /// the angle of the hit, k times the base angle
  double angle_deg = 0.0;
  /// the mark of a mundane hit, the first body at the moment in radians
  double lon = 0.0;
};

/// The choices of his linear boxes.
struct LinearOptions {
  LinearKind kind = LinearKind::kSecondary;
  //RR GRUNDWINKEL, jeder Winkel der sich durch ganzzahlige Teilung von
  //RR 360 Grad ergibt, solange 15 Grad nicht unterschritten wird
  double base_angle_deg = kDefaultBaseAngleDeg;
  /// the date window of the transits, jdbeg and jdend, 31, 125 or 488
  /// days from the first of a month
  double jd_from_ut = 0.0;
  double jd_to_ut = 0.0;
  /// the life window of the directions in years, lja and lje
  double from_years = 0.0;
  double to_years = 0.0;
  /// Nach UNTEN POSITIV ( R.EBERTIN ), his lin_inv
  bool downward = false;
  /// his zeichen!, the sign boundaries and the sign beside every label
  bool signs = true;
  /// his zwhd!, the lines of H2 H3 H5 H6 with their opposite cusps
  bool with_houses = false;
  /// his linie!, a vertical line from every hit down to the axis
  bool hit_lines = true;
  /// his gitter!, the fixed grid of vertical lines
  bool grid = false;
  /// his bildsdick&, 1 for FEIN, 2 for DICKER
  double line_width = 1.0;
  /// his pl1&, the first running body, and the chosen ones when not empty
  int first_slot = 1;
  std::vector<int> chosen;
  /// his mas!, the moon runs along
  bool moon = false;
  /// the lines of his a18kopf in his places, title line, name line and
  /// the start date of a date window, empty ones stay off the sheet
  std::string title;
  std::string record;
  std::string name;
  std::string place;
  std::string start;
  /// the exact hits of the window
  std::vector<LinearHit> hits;
  /// reports the drawn fraction of the curves, false stops the drawing
  SweepProgress progress;
};

/// His nbl%, the pixels of one day on a date axis or of one year on a
/// life axis.
///
/// @param opt the window
/// @return 16, 4 or 1 per day, 96 down to 3 per year
[[nodiscard]] int linear_pixels_per_step(const LinearOptions& opt);

/// Builds the linear graph as a display list on the 640 by 460 sheet.
///
/// @param radix the birth chart the horizontal lines come from
/// @param opt   base angle, window, movement and display switches
/// @param ctx   observer and settings for the running positions
/// @return the drawing, ready for every display list backend
[[nodiscard]] DisplayList build_linear_graph(const Chart& radix, const LinearOptions& opt, const SearchContext& ctx);

/// The folded positions of the sign boundaries, his zeichen lines. His
/// loop multiplied the folded boundary of the first sign by the sign
/// number, so below 360 degrees only one line fell inside the band.
///
/// @param base_angle_deg the base angle
/// @return the distinct folded degrees inside the band, 0 left out
[[nodiscard]] std::vector<double> linear_sign_lines(double base_angle_deg);

}  // namespace horcom
