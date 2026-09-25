// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "painter.hpp"

#include <QBuffer>
#include <QFontMetricsF>
#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QString>
#include <algorithm>
#include <cmath>

#include "horcom/core/constants.hpp"
#include "theme.hpp"

namespace horcom {

QColor to_qcolor(Rgb c, int alpha) {
  return QColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, alpha);
}

namespace {

// Robert Rettig's own symbol drawings, the glyph text keys his sprite
// stems from symbbmp. Unmapped texts fall back to the font.
const QHash<QString, QString>& sprite_stems() {
  static const QHash<QString, QString> map = {
      {QStringLiteral("☉"), QStringLiteral("SO")},  {QStringLiteral("☽"), QStringLiteral("MO")},
      {QStringLiteral("☿"), QStringLiteral("ME")},  {QStringLiteral("♀"), QStringLiteral("VE")},
      {QStringLiteral("♂"), QStringLiteral("MA")},  {QStringLiteral("♃"), QStringLiteral("JU")},
      {QStringLiteral("♄"), QStringLiteral("SA")},  {QStringLiteral("♅"), QStringLiteral("UR")},
      {QStringLiteral("♆"), QStringLiteral("NE")},  {QStringLiteral("♇"), QStringLiteral("PL")},
      {QStringLiteral("☊"), QStringLiteral("DR")},  {QStringLiteral("☋"), QStringLiteral("DS")},
      {QStringLiteral("⊕"), QStringLiteral("TE")},  {QStringLiteral("⚸"), QStringLiteral("AG")},
      {QStringLiteral("⚷"), QStringLiteral("CH")},  {QStringLiteral("TP"), QStringLiteral("TP")},
      {QStringLiteral("⊗"), QStringLiteral("GL")},  {QStringLiteral("⚳"), QStringLiteral("CE")},
      {QStringLiteral("⚴"), QStringLiteral("PA")},  {QStringLiteral("⚵"), QStringLiteral("JN")},
      {QStringLiteral("⚶"), QStringLiteral("VS")},  {QStringLiteral("CU"), QStringLiteral("CU")},
      {QStringLiteral("HA"), QStringLiteral("HA")}, {QStringLiteral("ZE"), QStringLiteral("ZE")},
      {QStringLiteral("KR"), QStringLiteral("KR")}, {QStringLiteral("AP"), QStringLiteral("AP")},
      {QStringLiteral("AD"), QStringLiteral("AD")}, {QStringLiteral("VU"), QStringLiteral("VU")},
      {QStringLiteral("PO"), QStringLiteral("PO")}, {QStringLiteral("QU"), QStringLiteral("QU")},
      {QStringLiteral("☄"), QStringLiteral("HL")},  {QStringLiteral("PH"), QStringLiteral("PH")},
      {QStringLiteral("DA"), QStringLiteral("DA")}, {QStringLiteral("NS"), QStringLiteral("NS")},
      {QStringLiteral("XE"), QStringLiteral("XE")}, {QStringLiteral("AC"), QStringLiteral("AC")},
      {QStringLiteral("MC"), QStringLiteral("MC")}, {QStringLiteral("♈"), QStringLiteral("ZAR")},
      {QStringLiteral("♉"), QStringLiteral("ZTA")}, {QStringLiteral("♊"), QStringLiteral("ZGM")},
      {QStringLiteral("♋"), QStringLiteral("ZCN")}, {QStringLiteral("♌"), QStringLiteral("ZLE")},
      {QStringLiteral("♍"), QStringLiteral("ZVI")}, {QStringLiteral("♎"), QStringLiteral("ZLI")},
      {QStringLiteral("♏"), QStringLiteral("ZSC")}, {QStringLiteral("♐"), QStringLiteral("ZSG")},
      {QStringLiteral("♑"), QStringLiteral("ZCP")}, {QStringLiteral("♒"), QStringLiteral("ZAQ")},
      {QStringLiteral("♓"), QStringLiteral("ZPS")}};
  return map;
}

// his sprites are black on transparent, the wheel tints them like
// bmp_color_pl, red for emphasis, white for the inverted nodes
const QImage& sprite(const QString& stem, Rgb color) {
  static QHash<QString, QImage> cache;
  const QString key = stem + QChar(':') + QString::number(color, 16);
  auto it = cache.find(key);
  if (it != cache.end()) {
    return *it;
  }
  QImage img(QStringLiteral(":/symb/") + stem + QStringLiteral(".png"));
  if (color != 0x000000 && !img.isNull()) {
    QPainter tint(&img);
    tint.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tint.fillRect(img.rect(), to_qcolor(color));
  }
  return *cache.insert(key, std::move(img));
}

// the fitted sprites of all window sizes seen, emptied once this many
constexpr int kFittedSprites = 4096;

// his drawing at the device size it lands on, averaged down once from
// the 384 pixel sprite. The bilinear drawImage of the raster engine reads
// four source pixels whatever the reduction and thins his strokes at
// twenty fold, the area filter of QImage::scaled keeps their weight
QImage fitted_sprite(const QString& stem, Rgb color, QSize px) {
  static QHash<QString, QImage> cache;
  const QString key = stem + QChar(':') + QString::number(color, 16) + QChar(':') + QString::number(px.width()) +
                      QChar('x') + QString::number(px.height());
  const auto it = cache.constFind(key);
  if (it != cache.constEnd()) {
    return *it;
  }
  const QImage& full = sprite(stem, color);
  if (full.isNull()) {
    return {};
  }
  // premultiplied so the averaged rim keeps the tint instead of darkening
  QImage img = full.convertToFormat(QImage::Format_ARGB32_Premultiplied)
                   .scaled(px, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  if (cache.size() >= kFittedSprites) {
    cache.clear();
  }
  cache.insert(key, img);
  return img;
}

// a canvas box on whole device pixels, empty while the painter rotates
// or shears, so sprites and their grounds share one pixel grid
QRect device_box(const QPainter& p, const QRectF& r) {
  const QTransform dt = p.deviceTransform();
  if (dt.type() > QTransform::TxScale) {
    return {};
  }
  const QRectF d = dt.mapRect(r);
  const int left = static_cast<int>(std::lround(d.left()));
  const int top = static_cast<int>(std::lround(d.top()));
  const int right = static_cast<int>(std::lround(d.right()));
  const int bottom = static_cast<int>(std::lround(d.bottom()));
  return {QPoint(left, top), QSize(std::max(1, right - left), std::max(1, bottom - top))};
}

// lifts the world transform and returns what stays between the painter
// and the device, the offset of a child widget in its window and the
// ratio of a high density screen
QTransform lift_world(QPainter& p) {
  p.resetTransform();
  return p.deviceTransform();
}

// the Courier New of the old IDE days, bold so the sheet reads at wheel
// sizes
QFont sheet_face() {
  QFont f = theme::mono_font();
  f.setFixedPitch(true);
  f.setWeight(QFont::Bold);
  return f;
}

// one text set at a device scale
struct SetText {
  QFont font;
  int px = 0;
  double width = 0.0;  // device pixels
};

// Sheet text renders at a whole device pixel size so the raster face
// stays sharp like his SYSTEM_FIXED_FONT did at its native strike. His
// FONT WIDTH is the room his layouts budget a character, the face steps
// text_advance, the stretch lands on whole percents and a letter spacing
// closes the rest so every character steps exactly one cell and his
// columns hold
SetText set_text(const Primitive& item, double scale, const QFont& face) {
  SetText s;
  s.font = face;
  s.px = std::max(6, static_cast<int>(std::lround(item.size * scale)));
  s.font.setPixelSize(s.px);
  s.font.setStretch(QFont::Unstretched);
  s.font.setLetterSpacing(QFont::AbsoluteSpacing, 0.0);
  const QString text = QString::fromStdString(item.text);
  if (item.pitch > 0.0) {
    // the cell the boxes and exports of the sheet are laid out with, not
    // the advance a rounded pixel size happens to give the face
    const double cell = text_advance(item) * scale;
    s.font.setHintingPreference(QFont::PreferNoHinting);
    const double natural = QFontMetricsF(s.font).horizontalAdvance(QLatin1Char('M'));
    if (natural > 0.0) {
      s.font.setStretch(std::clamp(static_cast<int>(std::lround(100.0 * cell / natural)), 1, 4000));
      s.font.setLetterSpacing(QFont::AbsoluteSpacing, cell - QFontMetricsF(s.font).horizontalAdvance(QLatin1Char('M')));
    }
    s.width = cell * static_cast<double>(text.size());
  } else {
    s.width = QFontMetricsF(s.font).horizontalAdvance(text);
  }
  return s;
}

// the left edge of a horizontal text in device pixels
double text_left(const Primitive& item, double x, double width) {
  if (item.align_right) {
    return x - width;
  }
  if (item.align_left) {
    return x;
  }
  return x - width / 2.0;
}

Qt::PenStyle pen_style(Primitive::Style s) {
  switch (s) {
    case Primitive::Style::kDashed: return Qt::DashLine;
    case Primitive::Style::kDotted: return Qt::DotLine;
    case Primitive::Style::kDashDot: return Qt::DashDotLine;
    default: return Qt::SolidLine;
  }
}

}  // namespace

QRectF text_box(const Primitive& item, double scale) {
  if ((item.kind != Primitive::Kind::kText && item.kind != Primitive::Kind::kGlyph) || scale <= 0.0) {
    return {};
  }
  const SetText s = set_text(item, scale, sheet_face());
  const double w = s.width / scale;
  if (item.vertical) {
    return {item.x1 - item.size / 2.0, item.y1 - w, item.size, w};
  }
  return {text_left(item, item.x1, w), item.y1 - item.size / 2.0, w, item.size};
}

void paint_display_list(QPainter& p, const DisplayList& dl) {
  // the fixed font of his SYSTEM_FIXED_FONT screens for the labels,
  // the symbol face only for the glyphs
  const QFont text_font = sheet_face();
  const QFont glyph_font = p.font();
  for (const Primitive& item : dl.items) {
    switch (item.kind) {
      case Primitive::Kind::kCircle: {
        p.setPen(QPen(to_qcolor(item.color), item.width));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(item.x1, item.y1), item.r1, item.r1);
        break;
      }
      case Primitive::Kind::kLine: {
        QPen pen(to_qcolor(item.color), item.width);
        pen.setStyle(pen_style(item.style));
        p.setPen(pen);
        p.drawLine(QPointF(item.x1, item.y1), QPointF(item.x2, item.y2));
        break;
      }
      case Primitive::Kind::kSector: {
        // the wheel angle equals the Qt arc angle, both run counter
        // clockwise on screen
        const QRectF outer(item.x1 - item.r2, item.y1 - item.r2, 2.0 * item.r2, 2.0 * item.r2);
        const QRectF inner(item.x1 - item.r1, item.y1 - item.r1, 2.0 * item.r1, 2.0 * item.r1);
        const double a1 = item.a1 * kRadToDeg;
        const double sweep = (item.a2 - item.a1) * kRadToDeg;
        QPainterPath path;
        path.arcMoveTo(inner, a1);
        const QPointF start = path.currentPosition();
        path.arcMoveTo(outer, a1);
        const QPointF outer_start = path.currentPosition();
        path.moveTo(start);
        path.lineTo(outer_start);
        path.arcTo(outer, a1, sweep);
        path.arcTo(inner, a1 + sweep, -sweep);
        path.closeSubpath();
        // fill only, the sign borders and ring circles draw the edges
        p.setPen(Qt::NoPen);
        p.setBrush(to_qcolor(item.fill));
        p.drawPath(path);
        break;
      }
      case Primitive::Kind::kGlyph:
      case Primitive::Kind::kText: {
        if (item.kind == Primitive::Kind::kGlyph) {
          const QString key = QString::fromStdString(item.text);
          const auto stem = sprite_stems().find(key);
          if (stem != sprite_stems().end()) {
            const double side = item.size * kSpriteBox;
            const QRectF cell(item.x1 - side / 2.0, item.y1 - side / 2.0, side, side);
            const QRect box = device_box(p, cell);
            if (!box.isEmpty()) {
              QImage img = fitted_sprite(*stem, item.color, box.size());
              if (!img.isNull()) {
                p.save();
                const QTransform base = lift_world(p);
                img.setDevicePixelRatio(base.m11());
                p.drawImage(base.inverted().map(QPointF(box.topLeft())), img);
                p.restore();
                break;
              }
            }
            const QImage& img = sprite(*stem, item.color);
            if (!img.isNull()) {
              p.setRenderHint(QPainter::SmoothPixmapTransform, true);
              p.drawImage(cell, img);
              break;
            }
          }
        }
        p.setPen(QPen(to_qcolor(item.color)));
        // the text is set with the transform lifted, on device pixels
        const QTransform tf = p.transform();
        const double sc = std::hypot(tf.m11(), tf.m12());
        const SetText s = set_text(item, sc, item.kind == Primitive::Kind::kGlyph ? glyph_font : text_font);
        p.save();
        const QPointF dev = tf.map(QPointF(item.x1, item.y1));
        p.resetTransform();
        p.setFont(s.font);
        // the box is one text height of slack wider than the text so
        // the face never wraps or elides, the left edge alone places it
        const double slack = s.px;
        constexpr int kFlags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine;
        if (item.vertical) {
          // his escapement 900 font, the line reads upward from the point
          p.translate(dev);
          p.rotate(-90.0);
          p.drawText(QRectF(0.0, -2.0 * s.px, s.width + slack, 4.0 * s.px), kFlags, QString::fromStdString(item.text));
        } else {
          const double left = text_left(item, dev.x(), s.width);
          p.drawText(QRectF(left, dev.y() - 2.0 * s.px, s.width + slack, 4.0 * s.px), kFlags,
                     QString::fromStdString(item.text));
        }
        p.restore();
        break;
      }
      case Primitive::Kind::kDot: {
        p.setPen(Qt::NoPen);
        p.setBrush(to_qcolor(item.color));
        p.drawEllipse(QPointF(item.x1, item.y1), item.r1, item.r1);
        break;
      }
      case Primitive::Kind::kRect: {
        p.setPen(Qt::NoPen);
        p.setBrush(to_qcolor(item.fill));
        const QRectF r(item.x1 - item.r1, item.y1 - item.r2, 2.0 * item.r1, 2.0 * item.r2);
        const QRect box = device_box(p, r);
        if (box.isEmpty()) {
          p.drawRect(r);
          break;
        }
        // the sprite grounds on the pixel grid of their sprites
        p.save();
        const QTransform base = lift_world(p);
        p.drawRect(base.inverted().mapRect(QRectF(box)));
        p.restore();
        break;
      }
    }
  }
}

QImage glyph_sprite(const QString& glyph, Rgb color) {
  const auto stem = sprite_stems().find(glyph);
  if (stem != sprite_stems().end()) {
    return sprite(*stem, color);
  }
  // a plain body tag like SO or MO is already a sprite stem
  if (glyph.size() <= 3 && glyph.toUpper() == glyph) {
    return sprite(glyph, color);
  }
  return {};
}

QImage glyph_sprite_fitted(const QString& glyph, Rgb color, QSize px) {
  if (px.isEmpty()) {
    return {};
  }
  const auto stem = sprite_stems().find(glyph);
  if (stem != sprite_stems().end()) {
    return fitted_sprite(*stem, color, px);
  }
  if (glyph.size() <= 3 && glyph.toUpper() == glyph) {
    return fitted_sprite(glyph, color, px);
  }
  return {};
}

GlyphImageResolver svg_sprite_resolver() {
  return [](const std::string& glyph, Rgb color) -> std::string {
    const auto stem = sprite_stems().find(QString::fromStdString(glyph));
    if (stem == sprite_stems().end()) {
      return {};
    }
    static QHash<QString, std::string> cache;
    const QString key = *stem + QChar(':') + QString::number(color, 16);
    const auto it = cache.constFind(key);
    if (it != cache.constEnd()) {
      return *it;
    }
    const QImage& img = sprite(*stem, color);
    if (img.isNull()) {
      return {};
    }
    QByteArray png;
    QBuffer buf(&png);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return *cache.insert(key, "data:image/png;base64," + png.toBase64().toStdString());
  };
}

void paint_fitted(QPainter& p, const DisplayList& dl, const QRectF& target) {
  if (dl.items.empty()) {
    return;
  }
  const double s = std::min(target.width() / dl.width, target.height() / dl.height);
  const double ox = target.x() + (target.width() - dl.width * s) / 2.0;
  const double oy = target.y() + (target.height() - dl.height * s) / 2.0;
  p.save();
  p.translate(ox, oy);
  p.scale(s, s);
  // the warm paper of the sheet, the glyph cutouts blend into it
  p.fillRect(QRectF(0, 0, dl.width, dl.height), to_qcolor(kPaperColor));
  // his page ended the drawing at its edge like his window did
  p.setClipRect(QRectF(0, 0, dl.width, dl.height), Qt::IntersectClip);
  paint_display_list(p, dl);
  p.restore();
}

}  // namespace horcom
