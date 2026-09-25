// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <string>
#include <vector>

#include "horcom/render/wheel.hpp"

// The output screen of list_ausg, one page of his statistics list with
// the header, the info boxes of the condition and the footer, on the
// 640 by 480 canvas of his screens.
namespace horcom {

/// The rows one page holds, his zl& = 24.
inline constexpr int kStatRowsPerPage = 24;

/// One row of the page, already reduced to what list_ausg prints.
struct StatSheetRow {
  /// the body glyph of plan_ds at x 80, 0 for none
  int slot = 0;
  /// his S of a mirror point or A of an arabic part in place of the glyph
  std::string tag;
  /// false prints " -------", no value or an extra out of range
  bool has_value = true;
  /// the position in radians, the separation of an aspect
  double value = 0.0;
  /// an aspect prints its separation with grmise at x 90
  bool aspect = false;
  /// the name, or so$ with its chain under several conditions
  std::string label;
  /// the complete UND chain in inverse video
  bool inverse = false;
  /// kltext, the moment line, and in the KLEIN list the second line
  std::string moment;
  std::string moment2;
  /// the AC or MC of the GROß list at x 556, absent in the KLEIN list
  bool has_angle = false;
  double angle = 0.0;
};

/// The frame of one page.
struct StatSheetText {
  /// KLEIN-SCHRIFT, his slist!
  bool small = false;
  /// several conditions, his odu$ > ""
  bool multi = false;
  /// the column heads at (396, 29), empty in the helio GROß list
  std::string heads;
  /// " Datei : " with the name at (396, 17)
  std::string file;
  /// the page number at (610, 24)
  int page = 1;
  /// inf_box1, the object and the window of a single condition
  std::string object_line;
  std::string window_line;
  /// the aspect box sits at x 4 and names its values ISTWERT / GRAD
  bool aspect_box = false;
  /// inf_box2, the distribution bars of a single condition
  bool bars = false;
  /// the bars count houses, numbers instead of sign glyphs
  bool by_house = false;
  /// sum%(1..12) of the tested values
  std::array<int, 13> sums{};
  /// the searched sign or house framed, 0 for none
  int framed = 0;
  /// the object tag at (4, 16)
  std::string object_tag;
  /// Total = laf& and Partial = zdm&, OHNE EINSCHRÄNKUNG shows no Partial
  int total = 0;
  bool partial = true;
  int partial_count = 0;
  /// inf_box21, the vertical count box at x 20 or 100, empty for none
  int counts_x = 0;
  std::array<std::string, 3> counts{};
  /// Mehrere Bedingungen ! at (8, 28)
  bool several = false;
  /// inf_box3, the vertical list of the conditions
  std::vector<std::string> conditions;
  /// inf_box4, the horizontal box the right mouse button lays over the
  /// rows, with the conditions below it under several conditions
  bool info = false;
  /// its count lines, three and under UND two more
  std::vector<std::string> info_counts;
  /// the footer lines at y 433 and 445, the second empty below 146 rows
  std::string footer;
  std::string footer2;
  /// his fixed words, the caller translates them
  std::string label_signs = "Zeichen";
  std::string label_houses = "H\xC3\xA4user";
  std::string label_total = "Total =";
  std::string label_partial = "Partial=";
  std::string label_several = "Mehrere Bedingungen !";
  std::string label_aspect = "ISTWERT / GRAD";
};

/// Draws one page like list_ausg with list_ausg_ueb, inf_box1 to 4 and
/// kltext.
///
/// @param rows the rows of the page, at most kStatRowsPerPage
/// @param text the frame
/// @return the page on the 640 by 480 canvas
[[nodiscard]] DisplayList build_stat_page(const std::vector<StatSheetRow>& rows, const StatSheetText& text);

}  // namespace horcom
