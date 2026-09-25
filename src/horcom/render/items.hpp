// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <cmath>
#include <string>
#include <utility>

#include "horcom/render/wheel.hpp"

// The two primitives his list and graph screens are drawn from, a line
// and a text on the virtual 640 by 480 canvas.
namespace horcom {

/// A straight line like his @line.
///
/// @param x1    the start on the canvas
/// @param y1    the start on the canvas
/// @param x2    the end on the canvas
/// @param y2    the end on the canvas
/// @param color the pen colour
/// @param width the pen width, his DEFLINE width
/// @return the primitive
[[nodiscard]] inline Primitive line_item(double x1, double y1, double x2, double y2, Rgb color = kInkColor,
                                         double width = 1.0) {
  Primitive p;
  p.kind = Primitive::Kind::kLine;
  p.x1 = x1;
  p.y1 = y1;
  p.x2 = x2;
  p.y2 = y2;
  p.color = color;
  p.width = width;
  return p;
}

/// A text like his @text, growing rightward from x or centred on it.
///
/// @param x       the left edge, or the centre when centred
/// @param y       the middle of the text height
/// @param size    the text height
/// @param text    UTF-8 text
/// @param color   the ink
/// @param centred true centres the text on x like his textzent
/// @return the primitive
[[nodiscard]] inline Primitive text_item(double x, double y, double size, std::string text, Rgb color = kInkColor,
                                         bool centred = false) {
  Primitive p;
  p.kind = Primitive::Kind::kText;
  p.x1 = x;
  p.y1 = y;
  p.size = size;
  p.text = std::move(text);
  p.color = color;
  p.align_left = !centred;
  return p;
}

/// A text of his list and graph screens, his @text and @textc with the
/// fixed pitch FONT WIDTH of textg, standing on its bottom line like his
/// TA_BOTTOM alignment.
///
/// @param x      the left edge
/// @param bottom his y, the bottom of the text cell
/// @param size   his te_gr, the text height
/// @param text   UTF-8 text
/// @param color  the ink
/// @return the primitive
[[nodiscard]] inline Primitive screen_text(double x, double bottom, double size, std::string text,
                                           Rgb color = kInkColor) {
  Primitive p = text_item(x, bottom - 0.5 * size, size, std::move(text), color);
  p.pitch = font_pitch(size);
  return p;
}

/// His textzent and textzentc, a screen text centred on the 640 wide
/// screen, the blanks around it trimmed like his TRIM$.
///
/// @param bottom his y, the bottom of the text cell
/// @param size   his te_gr, the text height
/// @param text   UTF-8 text
/// @param color  the ink
/// @return the primitive
[[nodiscard]] inline Primitive screen_text_centred(double bottom, double size, const std::string& text,
                                                    Rgb color = kInkColor) {
  const auto first = text.find_first_not_of(' ');
  const auto last = text.find_last_not_of(' ');
  std::string trimmed = first == std::string::npos ? std::string() : text.substr(first, last - first + 1);
  Primitive p = text_item(kCanvasWidth / 2.0, bottom - 0.5 * size, size, std::move(trimmed), color, true);
  p.pitch = font_pitch(size);
  return p;
}

/// His texts, the escapement 900 font reading upward from x, y with the
/// cell to the left of x like his TA_BOTTOM alignment.
///
/// @param x     his x, the bottom edge of the turned text cell
/// @param y     where the text starts, it runs upward
/// @param size  his te_gr, the text height
/// @param text  UTF-8 text
/// @param color the ink
/// @return the primitive
[[nodiscard]] inline Primitive screen_text_vertical(double x, double y, double size, std::string text,
                                                    Rgb color = kInkColor) {
  Primitive p = text_item(x - 0.5 * size, y, size, std::move(text), color);
  p.vertical = true;
  p.pitch = font_pitch(size);
  return p;
}

/// His textzentrl, a centred screen text in a box with his shadow lines
/// on the top and the right, the box centred on 632 like his.
///
/// @param dl     the list the box and the text join
/// @param bottom his y, the bottom of the text cell
/// @param size   his te_gr, the text height
/// @param text   UTF-8 text, the blanks kept like his LEN(tex$)
/// @param color  the ink of the text
inline void add_screen_text_boxed(DisplayList& dl, double bottom, double size, const std::string& text,
                                  Rgb color = kInkColor) {
  if (text.empty()) {
    return;
  }
  //RR l& = LEN(tex$) * te_w&, xe& = (632 - l&) / 2
  const double pitch = font_pitch(size);
  std::size_t chars = 0;
  for (const unsigned char c : text) {
    chars += (c & 0xC0) != 0x80 ? 1 : 0;
  }
  const double l = static_cast<double>(chars) * pitch;
  const double xe = std::floor((632.0 - l) / 2.0);
  const double top = bottom - size;
  //RR @boxn(xe& - 2,yte& - te_gr& - 1,xe& + l& + 2,yte& + 2)
  dl.items.push_back(line_item(xe - 2.0, top - 1.0, xe + l + 2.0, top - 1.0));
  dl.items.push_back(line_item(xe + l + 2.0, top - 1.0, xe + l + 2.0, bottom + 2.0));
  dl.items.push_back(line_item(xe + l + 2.0, bottom + 2.0, xe - 2.0, bottom + 2.0));
  dl.items.push_back(line_item(xe - 2.0, bottom + 2.0, xe - 2.0, top - 1.0));
  // the shadow on the top and the right
  dl.items.push_back(line_item(xe - 1.0, top - 2.0, xe + l + 3.0, top - 2.0));
  dl.items.push_back(line_item(xe + l + 3.0, top - 2.0, xe + l + 3.0, bottom + 1.0));
  dl.items.push_back(line_item(xe - 1.0, top - 2.0, xe - 2.0, top - 1.0));
  //RR @textc(xe&,yte& + 1,te_gr&,l$)
  dl.items.push_back(screen_text(xe, bottom + 1.0, size, text, color));
}

}  // namespace horcom
