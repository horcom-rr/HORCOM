// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QColor>
#include <QImage>
#include <QRectF>
#include <QString>

#include "horcom/render/svg.hpp"
#include "horcom/render/wheel.hpp"

class QPainter;

namespace horcom {

/// A display list colour for Qt.
///
/// @param c     the colour as 0xRRGGBB
/// @param alpha the opacity, 255 opaque
/// @return the Qt colour
[[nodiscard]] QColor to_qcolor(Rgb c, int alpha = 255);

/// A Qt colour for the display list, the opacity dropped.
///
/// @param c the Qt colour
/// @return the colour as 0xRRGGBB
[[nodiscard]] inline Rgb to_rgb(const QColor& c) {
  return static_cast<Rgb>((c.red() << 16) | (c.green() << 8) | c.blue());
}

/// Paints a display list onto an already transformed painter, the one
/// code path behind the screen widget, the printer and the PDF export
/// that the original resolved through its moda branches.
///
/// @param p  painter whose coordinate system is the virtual canvas
/// @param dl the list to draw
void paint_display_list(QPainter& p, const DisplayList& dl);

/// The box a text item covers on the canvas, the same geometry the
/// painter sets the text in.
///
/// @param item  a text or glyph primitive
/// @param scale device pixels per canvas unit
/// @return the box in canvas units, the full text height tall, empty for
///         other kinds
[[nodiscard]] QRectF text_box(const Primitive& item, double scale);

/// Fits a display list into a target rectangle, aspect preserved and
/// centred, used by the print and PDF paths over a white page.
///
/// @param p      painter in device coordinates
/// @param dl     the list to draw
/// @param target the page area to fill
void paint_fitted(QPainter& p, const DisplayList& dl, const QRectF& target);

/// A resolver that embeds his sprite drawings into SVG exports as data
/// URIs, tinted like the screen, so the export shows the same symbols.
///
/// @return the hook for to_svg, empty results fall back to the font
[[nodiscard]] GlyphImageResolver svg_sprite_resolver();

/// One of his symbbmp sprites tinted in a colour, for the tables that
/// paint glyphs like the original coordinate screen did.
///
/// @param glyph the unicode glyph or text tag a display list would use
/// @param color the tint, black keeps the sprite as drawn
/// @return the sprite image, null when no sprite exists for the glyph
[[nodiscard]] QImage glyph_sprite(const QString& glyph, Rgb color);

/// One of his sprites averaged down to the device pixels it lands on,
/// for the table cells that draw it one to one as sharp as the wheel.
///
/// @param glyph the unicode glyph or text tag a display list would use
/// @param color the tint
/// @param px    the device pixels of the cell
/// @return the sprite image, null when no sprite exists for the glyph
[[nodiscard]] QImage glyph_sprite_fitted(const QString& glyph, Rgb color, QSize px);

}  // namespace horcom
