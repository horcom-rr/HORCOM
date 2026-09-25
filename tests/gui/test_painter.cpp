// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QImage>
#include <QPainter>
#include <cmath>
#include <functional>

#include "doctest.h"
#include "painter.hpp"

using namespace horcom;

namespace {

// a fractional sheet scale like a wheel fitted into a window
constexpr double kScale = 1.37;
constexpr double kGlyphSize = 15.0;

// the extent of the pixels a test accepts, empty when none
QRect extent(const QImage& img, const std::function<bool(QRgb)>& take) {
  QRect r;
  for (int y = 0; y < img.height(); ++y) {
    for (int x = 0; x < img.width(); ++x) {
      if (take(img.pixel(x, y))) {
        r = r.united(QRect(x, y, 1, 1));
      }
    }
  }
  return r;
}

}  // namespace

TEST_CASE("his inverted sprite stands on a square of whole device pixels") {
  DisplayList dl;
  dl.width = 60.0;
  dl.height = 60.0;
  constexpr double kCentre = 30.3;
  dl.items.push_back(inverted_patch(kCentre, kCentre, kGlyphSize));
  Primitive g;
  g.kind = Primitive::Kind::kGlyph;
  g.x1 = kCentre;
  g.y1 = kCentre;
  g.size = kGlyphSize;
  g.text = body_glyph(body::kPluto);
  g.color = kInvertedInk;
  dl.items.push_back(g);
  REQUIRE_FALSE(glyph_sprite(QString::fromUtf8(body_glyph(body::kPluto)), kInvertedInk).isNull());

  QImage img(static_cast<int>(60 * kScale), static_cast<int>(60 * kScale), QImage::Format_ARGB32);
  img.fill(Qt::white);
  {
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.scale(kScale, kScale);
    paint_display_list(p, dl);
  }
  // the square is his SRCINVERT cell, the sprite box itself
  const QRect square = extent(img, [](QRgb c) { return qGray(c) < 250; });
  REQUIRE_FALSE(square.isEmpty());
  const double side = kGlyphSize * kSpriteBox * kScale;
  CHECK(std::abs(square.width() - side) <= 1.0);
  CHECK(square.width() == square.height());
  // whole device pixels, no grey rim around the ground
  for (int x = square.left(); x <= square.right(); ++x) {
    CHECK(qGray(img.pixel(x, square.top())) < 128);
    CHECK(qGray(img.pixel(x, square.bottom())) < 128);
  }
  // the white drawing stands centred in its square, both on one grid
  const QRect ink = extent(img.copy(square), [](QRgb c) { return qGray(c) > 128; }).translated(square.topLeft());
  REQUIRE_FALSE(ink.isEmpty());
  CHECK(ink.width() < square.width());
  CHECK(std::abs(ink.center().x() - square.center().x()) <= 2);
  CHECK(std::abs(ink.center().y() - square.center().y()) <= 2);
}
