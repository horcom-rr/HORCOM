// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <string>
#include <vector>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/chart/harmonics.hpp"
#include "horcom/render/wheel.hpp"

// The left half of his two chart sheets, the DOPPELKREIS of a12 and the
// MULTI and HARMONICS sheets of multi11. Two narrow coordinate columns
// like bes11 and bes111 in their dppel and mult dress, the comparison
// grid of a12asp under them and the halbsm boxes of the MULTI sheet.
namespace horcom {

/// The dress of one coordinate column.
struct PairColumnOptions {
  /// his header, Ekl.Länge: or Länge: with the mode tag A1, A2 or W
  std::string header;
  /// vf!, the rows take the degree sign and the minute mark
  bool compact = false;
  /// par = 1, SO to MA carry the P of the parallax
  bool parallax = false;
  /// moknw! and apogw!, the W or M behind the node and the Black Moon
  bool true_node = false;
  bool true_apogee = false;
  bool heliocentric = false;
  /// klpl!, the extras join the rows
  bool extras = false;
  /// dop = 4, only AC and MC among the cusps
  bool dial = false;
  /// MULTI NULL OST, WEST and ARC print only AC and MC
  bool angles_only = false;
  /// a18st, a running column lists only the chosen factors and its rows
  /// close up over the ones left out
  bool present_only = false;
  /// Häuserspitzen or Häusersp.
  std::string houses_header;
  /// the house system name, printed in brackets
  std::string house_name;
};

/// Writes one of his sheet texts, which stand on their bottom line, the
/// display list centres them.
///
/// @param dl     the sheet
/// @param x      the left end
/// @param bottom his y, the bottom line of the text
/// @param s      the text
/// @param size   the text height
void add_sheet_text(DisplayList& dl, double x, double bottom, const std::string& s, double size);

/// Writes the bodies like bes11 under dppel! or mult!.
///
/// @param dl  the sheet
/// @param c   the chart of the column
/// @param opt its dress
/// @param xt  his xt&
/// @param yt  his yt&, the bottom line of the header
/// @return yt& after the last row, his y1&
double add_pair_bodies(DisplayList& dl, const Chart& c, const PairColumnOptions& opt, double xt, double yt);

/// Writes the house cusps like bes111 under dppel! or mult!.
///
/// @param dl  the sheet
/// @param c   the chart of the column
/// @param opt its dress
/// @param xt  his xt&
/// @param yt  his yt&, the bottom of the body rows
/// @return yt& after the last row
double add_pair_houses(DisplayList& dl, const Chart& c, const PairColumnOptions& opt, double xt, double yt);

/// The column letters and the dress of one a12asp grid.
struct CrossGridOptions {
  /// d1$ and d2$, " I" and " A", " R" and " M" and so on
  std::string left_tag = " I";
  std::string right_tag = " A";
  /// the tags of the second pair of columns, empty for MULTI where the
  /// second grid writes its own
  std::string left_tag2 = " I";
  std::string right_tag2 = " A";
  /// mult!, four rows per column, the grid ends with the hits
  bool multi = false;
  /// drgrph!, the running body leads the cell
  bool running_first = false;
  /// hard&, the colour of the second chart's glyph, 1 red, 3 blue
  int outer_color = 2;
  /// the mark of a grid too long for the page, the caller translates it
  std::string more_label = " MEHR ";
};

/// Draws the comparison grid of a12asp from the given hits.
///
/// @param dl   the sheet
/// @param hits the scan, each pair drawn once
/// @param opt  the dress
/// @param xt   his xt&, 2 or 112
/// @param yt   his yt&, the top of the grid
void add_cross_grid(DisplayList& dl, const std::vector<CrossAspectHit>& hits, const CrossGridOptions& opt, double xt,
                    double yt);

/// Draws the halbsm boxes of the MULTI sheet, bottom up from y 454, the
/// sign axes red on cyan and the radix cusps navy on yellow at x 6, the
/// directed angles at x 170.
///
/// @param dl    the sheet
/// @param lines the scan of multi_midpoints
/// @param white weiss!, every box black on white
void add_multi_midpoints(DisplayList& dl, const std::vector<MultiMidpoint>& lines, bool white);

/// His grze gz7$ and gz$ forms, "SO 12 AR 34" and "SO 12°AR 34'",
/// minutes rounded with the carry into the degree.
///
/// @param slot    the body, its tag leads
/// @param lon     ecliptic longitude in radians
/// @param compact true for the gz$ form
/// @return the text
[[nodiscard]] std::string pair_row_text(int slot, double lon, bool compact);

}  // namespace horcom
