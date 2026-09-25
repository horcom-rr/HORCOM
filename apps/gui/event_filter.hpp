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

/// An event filter that hands every event to a function, the shape of
/// his key and mouse loops around an output window.
class LambdaFilter : public QObject {
 public:
  /// @param handle returns true when it consumed the event
  /// @param parent the owner, may stay empty for a filter on the stack
  explicit LambdaFilter(std::function<bool(QEvent*)> handle, QObject* parent = nullptr)
      : QObject(parent), handle_(std::move(handle)) {}

  /// Builds a filter that sees key presses only.
  ///
  /// @param key returns true when it consumed the key
  /// @return the filter, installed by the caller
  [[nodiscard]] static std::function<bool(QEvent*)> keys(std::function<bool(int)> key) {
    return [key = std::move(key)](QEvent* e) {
      return e->type() == QEvent::KeyPress && key(static_cast<QKeyEvent*>(e)->key());
    };
  }

 protected:
  bool eventFilter(QObject*, QEvent* e) override { return handle_(e); }

 private:
  std::function<bool(QEvent*)> handle_;
};

}  // namespace horcom
