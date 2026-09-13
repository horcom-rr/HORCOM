// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "wheel_widget.hpp"

#include <QPaintEvent>
#include <QPainter>
#include <algorithm>

#include "painter.hpp"

namespace horcom {

WheelWidget::WheelWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(480, 360);
}

void WheelWidget::set_display_list(DisplayList dl) {
  dl_ = std::move(dl);
  update();
}

void WheelWidget::paintEvent(QPaintEvent* /*event*/) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::TextAntialiasing, true);
  // the dark desk of the theme
  p.fillRect(rect(), QColor(0x0A, 0x0F, 0x1E));
  if (dl_.items.empty()) {
    return;
  }
  // map the virtual canvas into the widget, aspect preserved, with a
  // margin for the paper card
  const double margin = 14.0;
  const double s = std::min((width() - 2.0 * margin) / dl_.width, (height() - 2.0 * margin) / dl_.height);
  const double ox = (width() - dl_.width * s) / 2.0;
  const double oy = (height() - dl_.height * s) / 2.0;

  // the chart lies like paper on the desk, a soft shadow and a warm
  // white sheet
  const QRectF paper(ox, oy, dl_.width * s, dl_.height * s);
  p.setPen(Qt::NoPen);
  for (int i = 3; i >= 1; --i) {
    p.setBrush(QColor(0, 0, 0, 22));
    p.drawRoundedRect(paper.adjusted(-i, -i + 2.0, i, i + 2.0), 10, 10);
  }
  p.setBrush(QColor(0xFC, 0xFA, 0xF4));
  p.setPen(QPen(QColor(0x23, 0x2D, 0x4A), 1.0));
  p.drawRoundedRect(paper, 8, 8);

  p.translate(ox, oy);
  p.scale(s, s);

  paint_display_list(p, dl_);
}

}  // namespace horcom
