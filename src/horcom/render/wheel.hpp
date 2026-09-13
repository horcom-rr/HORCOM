// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string>
#include <vector>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"

// The chart wheel as a backend neutral display list. The geometry is the
// original's, a virtual canvas of 640 by 480 with the wheel centre at
// 430, 224 and the scale km 0.95, the ring radii 90, 128, 152, 165 and
// 182, the polar mapping w = nb(lambda + pi - fza) with the ascendant on
// the left, and his glyph de clumping from plentz. Only the flood fill of
// the sign band becomes explicit annular sectors, a change the analysis
// already demanded for any modern renderer.
namespace horcom {

/// Colours as packed 0xRRGGBB like the original RGB() calls.
using Rgb = unsigned;

/// One drawing primitive on the virtual 640 by 480 canvas.
struct Primitive {
  enum class Kind { kCircle, kLine, kSector, kGlyph, kText, kDot };
  enum class Style { kSolid, kDashed, kDotted };
  Kind kind = Kind::kLine;
  double x1 = 0.0;   // centre for circles, sectors, glyphs and dots
  double y1 = 0.0;
  double x2 = 0.0;   // line end
  double y2 = 0.0;
  double r1 = 0.0;   // radius, inner radius for sectors
  double r2 = 0.0;   // outer radius for sectors
  double a1 = 0.0;   // sector start angle on the canvas, radians
  double a2 = 0.0;   // sector end angle
  double size = 0.0; // text height for glyphs and text
  Rgb color = 0x000000;
  Rgb fill = 0xFFFFFF;
  Style style = Style::kSolid;
  double width = 1.0;
  std::string text;  // glyph character or label
};

/// A ready to draw wheel.
struct DisplayList {
  double width = 640.0;
  double height = 480.0;
  std::vector<Primitive> items;
};

/// Options of the wheel builder.
struct WheelOptions {
  /// draw the aspect chords of a scan result
  bool aspect_lines = true;
  /// draw the blue dashed node axis like the original Mondknotenlinie
  bool node_axis = true;
  /// print the degree within sign under each glyph, the original pziff
  bool degree_numbers = true;
};

/// Builds the display list of one chart wheel.
///
/// @param chart   the computed chart
/// @param s       chart settings, decides which slots appear
/// @param aspects the aspect scan whose hits become the chords, may be a
///                default constructed result when aspect_lines is off
/// @param opt     drawing options
/// @return primitives on the virtual canvas, in paint order
[[nodiscard]] DisplayList build_wheel(const Chart& chart, const ChartSettings& s, const AspectResult& aspects, const WheelOptions& opt = {});

}  // namespace horcom
