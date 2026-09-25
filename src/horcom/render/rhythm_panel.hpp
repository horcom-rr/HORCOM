// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string>
#include <vector>

#include "horcom/chart/rhythm.hpp"
#include "horcom/render/wheel.hpp"

// The left strip of the Rhythmenlehre graph, the GRAPHIK ( HOROSKOP )
// mode of the original a17. One screen per phase, a time axis up the
// middle of the strip, the direct and ruler triggers on its left, the
// aspect and mirror partners on its right, both columns pushed apart
// so no label covers another.
namespace horcom {

/// The time axis of a phase, the start at the bottom, the end at the
/// top, his c3& = 416 - fm& * 336 * (l - start) / vp.
inline constexpr double kRhythmAxisTop = 80.0;
inline constexpr double kRhythmAxisBottom = 416.0;

/// One label of a phase column, a1791 on the left and a1792 on the right.
struct RhythmPanelEntry {
  /// the triggered body, 15 to 18 the cardinal points
  int slot = 0;
  /// the body that pulled it in, right column only
  int source = 0;
  /// the aspect family of an aspect row, his al1&, zero for a mirror
  int family = 0;
  /// a mirror partner, his S
  bool mirror = false;
  /// drawn inverted, a ruler of the phase his c1& marks, unless a direct
  /// trigger of the same body overwrote its letter
  bool inverse = false;
  /// the exact height on the time axis
  double y = 0.0;
  /// the date or the LJ/MO text, his lja$ and ljb$
  std::string label;
};

/// The texts and columns of one phase screen.
struct RhythmPanel {
  /// his a170_1tit lines, the title and the period line
  std::string title;
  std::string period;
  /// the LJ/MO span of the phase and its unit, empty with dates
  std::string span;
  std::string span_unit;
  /// the red NEGATIVE Periode lines, empty for a forward walk
  std::string negative1;
  std::string negative2;
  /// the WEITER line under the strip
  std::string footer;
  /// the dates carry his wider offsets, ausgd!
  bool dated = true;
  std::vector<RhythmPanelEntry> left;
  std::vector<RhythmPanelEntry> right;
};

/// The height of an age on the time axis of its phase.
///
/// @param age    the trigger age in years
/// @param start  the age at the start of the phase
/// @param length the length of the phase in years, negative into the past
/// @return the canvas y, 416 at the start and 80 at the end
[[nodiscard]] double rhythm_axis_y(double age, double start, double length);

/// Places the labels of a column like a17911, sorted by height and
/// pushed upward from the bottom until each stands sixteen pixels above
/// the one below.
///
/// @param ys the exact heights
/// @return the placed height of each entry in input order
[[nodiscard]] std::vector<double> rhythm_label_rows(const std::vector<double>& ys);

/// The glyph a slot wears in the strip, the body glyph, AC and MC as
/// text and the cardinal points as their signs.
///
/// @param slot a body slot
/// @return UTF-8 text
[[nodiscard]] std::string rhythm_glyph(int slot);

/// Draws the strip left of the wheel, a170 with a1791, a1792 and
/// a170_1tit.
///
/// @param dl the sheet, the classic 640 wide canvas
/// @param p  the texts and columns of the phase
void add_rhythm_panel(DisplayList& dl, const RhythmPanel& p);

}  // namespace horcom
