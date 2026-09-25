// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>

#include "horcom/render/wheel.hpp"

namespace horcom {

class WheelWidget;

/// The window of the ASPEKTARIUM, his aspar sheet drawn by
/// build_aspektarium. The right mouse asks for single planets like his
/// einzel_plan_wahl, the owner redraws the sheet then.
class AspektariumDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param sheet  the drawn sheet
  /// @param parent the owner window
  explicit AspektariumDialog(DisplayList sheet, QWidget* parent = nullptr);

  /// Replaces the drawing.
  ///
  /// @param sheet the new sheet
  void set_sheet(DisplayList sheet);

 signals:
  /// The right mouse button went down on the sheet.
  void right_clicked();

 private:
  WheelWidget* canvas_ = nullptr;
};

}  // namespace horcom
