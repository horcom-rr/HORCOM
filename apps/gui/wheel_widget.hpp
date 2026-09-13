// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QWidget>

#include "horcom/render/wheel.hpp"

namespace horcom {

/// Paints the wheel display list, one more backend beside the SVG
/// writer. The mouse wheel zooms around the cursor, a left drag pans
/// the zoomed sheet and a double click resets the view.
class WheelWidget : public QWidget {
  Q_OBJECT

 public:
  explicit WheelWidget(QWidget* parent = nullptr);

  /// Replaces the drawing and repaints.
  void set_display_list(DisplayList dl);

  /// @return the current drawing, the print and PDF paths read it
  [[nodiscard]] const DisplayList& display_list() const { return dl_; }

 protected:
  void paintEvent(QPaintEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

 private:
  DisplayList dl_;
  double zoom_ = 1.0;
  QPointF pan_;
  QPointF drag_start_;
  QPointF pan_start_;
  bool dragging_ = false;
};

}  // namespace horcom
