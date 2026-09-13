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

/// The virtual canvas and wheel geometry of the original, shared with the
/// tests and every backend.
inline constexpr double kCanvasWidth = 640.0;
inline constexpr double kCanvasHeight = 480.0;
//RR Horoskop-Mitte
inline constexpr double kWheelCenterX = 430.0;
inline constexpr double kWheelCenterY = 224.0;
inline constexpr double kWheelScale = 0.95;  // the original km
inline constexpr double kGlyphRingRadius = 128.0;

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

/// the a20 transit screen shrinks the wheel to make room for the outer
/// ring, the original km there
inline constexpr double kTransitWheelScale = 0.85;

/// Options of the wheel builder.
struct WheelOptions {
  /// draw the aspect chords of a scan result
  bool aspect_lines = true;
  /// the centre label of the transit wheel, TRANSIT=> plus the date
  std::string transit_label;
  /// which divisors draw chords, the stand in for the original's per
  /// aspect aspli flags until the KONSTA colours are wired through. The
  /// default shows the classical set, conjunction to sextile and the
  /// quincunx family
  std::array<bool, 17> chord_divisor = {false, true, true,  true,  true,  true, true, false, false,
                                        false, false, false, true, false, false, false, false};
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

/// Builds the transit double wheel of the original a20 screen. The radix
/// sits inside with its glyphs, houses and aspect chords, the running sky
/// rides outside with glyphs at 212 and tick markers on the sign ring,
/// everything at the smaller a20 scale.
///
/// @param radix   the birth chart, rules the houses and the rotation
/// @param transit the chart of the transit moment, same settings
/// @param s       chart settings, decides which slots appear in both rings
/// @param radix_aspects the radix scan whose hits draw as chords
/// @param opt     drawing options, transit_label prints in the centre
/// @return primitives on the virtual canvas, in paint order
[[nodiscard]] DisplayList build_transit_wheel(const Chart& radix, const Chart& transit, const ChartSettings& s, const AspectResult& radix_aspects, const WheelOptions& opt = {});

}  // namespace horcom
