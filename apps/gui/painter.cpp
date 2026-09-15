// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "painter.hpp"

#include <QBuffer>
#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QString>
#include <algorithm>

#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

QColor rgb(Rgb c, int alpha = 255) {
  return QColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, alpha);
}

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
      {QStringLiteral("XE"), QStringLiteral("XE")}, {QStringLiteral("♈"), QStringLiteral("ZAR")},
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
    tint.fillRect(img.rect(), rgb(color));
  }
  return *cache.insert(key, std::move(img));
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

void paint_display_list(QPainter& p, const DisplayList& dl) {
  // the fixed font of his SYSTEM_FIXED_FONT screens for the labels,
  // the symbol face only for the glyphs
  // the Courier New of the old IDE days, bold so the sheet reads at
  // wheel sizes, rendered on whole device pixels below
  QFont text_font(QStringLiteral("Courier New"));
  text_font.setStyleHint(QFont::Monospace);
  text_font.setFixedPitch(true);
  text_font.setWeight(QFont::Bold);
  QFont glyph_font = p.font();
  for (const Primitive& item : dl.items) {
    switch (item.kind) {
      case Primitive::Kind::kCircle: {
        p.setPen(QPen(rgb(item.color), item.width));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(item.x1, item.y1), item.r1, item.r1);
        break;
      }
      case Primitive::Kind::kLine: {
        QPen pen(rgb(item.color), item.width);
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
        p.setBrush(rgb(item.fill));
        p.drawPath(path);
        break;
      }
      case Primitive::Kind::kGlyph:
      case Primitive::Kind::kText: {
        if (item.kind == Primitive::Kind::kGlyph) {
          const QString key = QString::fromStdString(item.text);
          const auto stem = sprite_stems().find(key);
          if (stem != sprite_stems().end()) {
            const QImage& img = sprite(*stem, item.color);
            if (!img.isNull()) {
              const double side = item.size * 1.1;
              p.setRenderHint(QPainter::SmoothPixmapTransform, true);
              p.drawImage(QRectF(item.x1 - side / 2.0, item.y1 - side / 2.0, side, side), img);
              break;
            }
          }
        }
        p.setPen(QPen(rgb(item.color)));
        QFont& font = item.kind == Primitive::Kind::kGlyph ? glyph_font : text_font;
        // sheet text renders at a whole device pixel size with the
        // transform lifted, the raster face stays sharp like his
        // SYSTEM_FIXED_FONT did at its native strike
        const QTransform tf = p.transform();
        const double sc = std::hypot(tf.m11(), tf.m12());
        const int px = std::max(6, static_cast<int>(std::lround(item.size * sc)));
        font.setPixelSize(px);
        p.save();
        const QPointF dev = tf.map(QPointF(item.x1, item.y1));
        p.resetTransform();
        p.setFont(font);
        const double half =
            std::max(60.0, 0.5 * static_cast<double>(item.text.size()) * px);
        if (item.align_right) {
          const QRectF box(dev.x() - 2.0 * half, dev.y() - 2.0 * px, 2.0 * half, 4.0 * px);
          p.drawText(box, Qt::AlignRight | Qt::AlignVCenter, QString::fromStdString(item.text));
        } else if (item.align_left) {
          const QRectF box(dev.x(), dev.y() - 2.0 * px, 2.0 * half, 4.0 * px);
          p.drawText(box, Qt::AlignLeft | Qt::AlignVCenter, QString::fromStdString(item.text));
        } else {
          const QRectF box(dev.x() - half, dev.y() - 2.0 * px, 2.0 * half, 4.0 * px);
          p.drawText(box, Qt::AlignCenter, QString::fromStdString(item.text));
        }
        p.restore();
        break;
      }
      case Primitive::Kind::kDot: {
        p.setPen(Qt::NoPen);
        p.setBrush(rgb(item.color));
        p.drawEllipse(QPointF(item.x1, item.y1), item.r1, item.r1);
        break;
      }
      case Primitive::Kind::kRect: {
        p.setPen(Qt::NoPen);
        p.setBrush(rgb(item.fill));
        p.drawRect(QRectF(item.x1 - item.r1, item.y1 - item.r2, 2.0 * item.r1, 2.0 * item.r2));
        break;
      }
    }
  }
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
  p.fillRect(QRectF(0, 0, dl.width, dl.height), QColor(0xFC, 0xFA, 0xF4));
  paint_display_list(p, dl);
  p.restore();
}

}  // namespace horcom
