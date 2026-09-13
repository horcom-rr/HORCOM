// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "painter.hpp"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>

#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

QColor rgb(Rgb c, int alpha = 255) {
  return QColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, alpha);
}

Qt::PenStyle pen_style(Primitive::Style s) {
  switch (s) {
    case Primitive::Style::kDashed: return Qt::DashLine;
    case Primitive::Style::kDotted: return Qt::DotLine;
    default: return Qt::SolidLine;
  }
}

}  // namespace

void paint_display_list(QPainter& p, const DisplayList& dl) {
  QFont font = p.font();
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
        p.setPen(QPen(rgb(item.color), 0.4));
        p.setBrush(rgb(item.fill, 110));
        p.drawPath(path);
        break;
      }
      case Primitive::Kind::kGlyph:
      case Primitive::Kind::kText: {
        p.setPen(QPen(rgb(item.color)));
        font.setPixelSize(static_cast<int>(item.size));
        p.setFont(font);
        // long labels like the transit line need a wider box, the
        // centring keeps them in place
        const double half = std::max(40.0, 0.5 * static_cast<double>(item.text.size()) * item.size);
        if (item.align_left) {
          const QRectF box(item.x1, item.y1 - 20.0, 2.0 * half, 40.0);
          p.drawText(box, Qt::AlignLeft | Qt::AlignVCenter, QString::fromStdString(item.text));
        } else {
          const QRectF box(item.x1 - half, item.y1 - 20.0, 2.0 * half, 40.0);
          p.drawText(box, Qt::AlignCenter, QString::fromStdString(item.text));
        }
        break;
      }
      case Primitive::Kind::kDot: {
        p.setPen(Qt::NoPen);
        p.setBrush(rgb(item.color));
        p.drawEllipse(QPointF(item.x1, item.y1), item.r1, item.r1);
        break;
      }
    }
  }
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
  paint_display_list(p, dl);
  p.restore();
}

}  // namespace horcom
