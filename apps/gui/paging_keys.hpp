// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QEvent>
#include <QKeyEvent>
#include <QObject>
#include <functional>
#include <utility>

namespace horcom {

/// The wart keys of his paging screens, Space, plus and PgDn go on,
/// minus, R and PgUp go back.
class PagingKeys final : public QObject {
 public:
  /// @param next the step forward
  /// @param back the step back
  PagingKeys(std::function<void()> next, std::function<void()> back)
      : next_(std::move(next)), back_(std::move(back)) {}

 protected:
  bool eventFilter(QObject* watched, QEvent* e) override {
    if (e->type() != QEvent::KeyPress) {
      return QObject::eventFilter(watched, e);
    }
    switch (static_cast<QKeyEvent*>(e)->key()) {
      case Qt::Key_Space:
      case Qt::Key_Plus:
      case Qt::Key_PageDown:
        next_();
        return true;
      case Qt::Key_Minus:
      case Qt::Key_R:
      case Qt::Key_PageUp:
        back_();
        return true;
      default:
        return QObject::eventFilter(watched, e);
    }
  }

 private:
  std::function<void()> next_;
  std::function<void()> back_;
};

}  // namespace horcom
