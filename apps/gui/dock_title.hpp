// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QToolButton>

namespace horcom {

/// The title bar of a dock as a widget of its own. A dock without one
/// takes the native title of the system while it floats or is dragged,
/// the grey bar of the platform instead of the yellow box of the dress.
/// Qt keeps a widget title bar in every state, the drags and double
/// clicks it leaves unhandled still move and float the dock.
class DockTitleBar final : public QWidget {
 public:
  /// Builds the bar from the title and the features of the dock, set
  /// both before.
  ///
  /// @param dock the dock that wears the bar
  explicit DockTitleBar(QDockWidget* dock) : QWidget(dock) {
    setObjectName(QStringLiteral("dockTitleBar"));
    // the style sheet paints the box of a plain widget only with this
    setAttribute(Qt::WA_StyledBackground, true);
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(10, 4, 4, 4);
    row->setSpacing(2);
    auto* title = new QLabel(dock->windowTitle(), this);
    title->setObjectName(QStringLiteral("dockTitle"));
    row->addWidget(title, 1);
    const auto button = [this, row](QStyle::StandardPixmap icon) {
      auto* b = new QToolButton(this);
      b->setObjectName(QStringLiteral("dockTitleButton"));
      b->setIcon(style()->standardIcon(icon, nullptr, this));
      b->setAutoRaise(true);
      b->setFocusPolicy(Qt::NoFocus);
      row->addWidget(b);
      return b;
    };
    if (dock->features().testFlag(QDockWidget::DockWidgetFloatable)) {
      QToolButton* lift = button(QStyle::SP_TitleBarNormalButton);
      connect(lift, &QToolButton::clicked, dock, [dock]() { dock->setFloating(!dock->isFloating()); });
    }
    if (dock->features().testFlag(QDockWidget::DockWidgetClosable)) {
      QToolButton* shut = button(QStyle::SP_TitleBarCloseButton);
      connect(shut, &QToolButton::clicked, dock, &QDockWidget::close);
    }
    connect(dock, &QDockWidget::windowTitleChanged, title, &QLabel::setText);
  }
};

/// Gives a dock its title bar widget.
///
/// @param dock the dock, its title and features already set
inline void dress_dock_title(QDockWidget* dock) {
  dock->setTitleBarWidget(new DockTitleBar(dock));
}

}  // namespace horcom
