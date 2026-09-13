// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "wheel_widget.hpp"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

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
  // the view the mouse built, pan then zoom about the origin
  p.translate(pan_);
  p.scale(zoom_, zoom_);
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

void WheelWidget::wheelEvent(QWheelEvent* event) {
  const double factor = std::pow(1.15, event->angleDelta().y() / 120.0);
  const double next = std::clamp(zoom_ * factor, 1.0, 12.0);
  const double f = next / zoom_;
  // the point under the cursor stays put while the sheet grows
  pan_ = event->position() - (event->position() - pan_) * f;
  zoom_ = next;
  if (zoom_ == 1.0) {
    pan_ = QPointF();
  }
  update();
  event->accept();
}

void WheelWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton && zoom_ > 1.0) {
    dragging_ = true;
    drag_start_ = event->position();
    pan_start_ = pan_;
    setCursor(Qt::ClosedHandCursor);
  }
  QWidget::mousePressEvent(event);
}

void WheelWidget::mouseMoveEvent(QMouseEvent* event) {
  if (dragging_) {
    pan_ = pan_start_ + (event->position() - drag_start_);
    update();
  }
  QWidget::mouseMoveEvent(event);
}

void WheelWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (dragging_) {
    dragging_ = false;
    unsetCursor();
  }
  QWidget::mouseReleaseEvent(event);
}

void WheelWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  // back to the whole sheet
  zoom_ = 1.0;
  pan_ = QPointF();
  update();
  QWidget::mouseDoubleClickEvent(event);
}

}  // namespace horcom
