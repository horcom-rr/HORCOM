// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QWidget>

#include "horcom/render/wheel.hpp"

namespace horcom {

/// Paints the wheel display list, one more backend beside the SVG writer.
class WheelWidget : public QWidget {
  Q_OBJECT

 public:
  explicit WheelWidget(QWidget* parent = nullptr);

  /// Replaces the drawing and repaints.
  void set_display_list(DisplayList dl);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  DisplayList dl_;
};

}  // namespace horcom
