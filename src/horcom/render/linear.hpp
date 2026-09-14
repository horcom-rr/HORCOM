// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/chart/chart.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/render/wheel.hpp"

// The LINEAR-GRAPHIK, the life diagram in the manner of R. Ebertin.
// Every radix factor becomes a horizontal line at its position folded
// by the chosen base angle, the running bodies trace curves over a
// time axis, and every crossing of a curve with a line is a direction
// or transit becoming exact. Ported from HORCOM a18_lin, a180trpr_lin
// and a18_line_col.
namespace horcom {

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
};

/// The choices of his linear dialogs.
struct LinearOptions {
  LinearKind kind = LinearKind::kSecondary;
  //RR GRUNDWINKEL, jeder Winkel der sich durch ganzzahlige Teilung von
  //RR 360 Grad ergibt, solange 15 Grad nicht unterschritten wird
  double base_angle_deg = 90.0;
  /// the life window in real time
  double jd_from_ut = 0.0;
  double jd_to_ut = 0.0;
  //RR Nach UNTEN POSITIV ( R.EBERTIN )
  bool downward = false;
  /// dashed sign boundary lines, they only make sense at base 360
  bool sign_lines = false;
  /// the H2 H3 H5 H6 lines of the Zwischenhäuser choice with their
  /// opposite cusps
  bool with_houses = false;
  /// sample count across the sheet, each sample is one ephemeris call
  int samples = 520;
};

/// Builds the linear graph as a display list on the 640 by 460 sheet.
///
/// @param radix the birth chart the horizontal lines come from
/// @param opt   base angle, window, movement and display switches
/// @param ctx   observer and settings for the running positions
/// @return the drawing, ready for every display list backend
[[nodiscard]] DisplayList build_linear_graph(const Chart& radix, const LinearOptions& opt, const SearchContext& ctx);

}  // namespace horcom
