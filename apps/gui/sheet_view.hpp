// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <functional>
#include <utility>

#include "wheel_widget.hpp"

namespace horcom {

/// A plain sheet in a window of its own that waits for his WEITER key,
/// the wart over the graph screens of the Rhythmenlehre and the
/// DYNAMOGRAMM. Space, Return and Enter hand over to the step, ESC
/// closes the window.
class SheetView final : public QDialog {
 public:
  /// @param parent the owner window
  /// @param step   runs on the WEITER key and returns true while the
  ///               window stays, empty closes it on the first key
  explicit SheetView(QWidget* parent, std::function<bool()> step = {}) : QDialog(parent), step_(std::move(step)) {
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    canvas_ = new WheelWidget(this);
    v->addWidget(canvas_);
  }

  /// @return the canvas the sheet is drawn on
  [[nodiscard]] WheelWidget* canvas() const { return canvas_; }

  /// @param step the new WEITER action, true keeps the window open
  void set_step(std::function<bool()> step) { step_ = std::move(step); }

 protected:
  void keyPressEvent(QKeyEvent* e) override {
    if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
      if (!step_ || !step_()) {
        accept();
      }
      return;
    }
    QDialog::keyPressEvent(e);
  }

 private:
  WheelWidget* canvas_ = nullptr;
  std::function<bool()> step_;
};

}  // namespace horcom
