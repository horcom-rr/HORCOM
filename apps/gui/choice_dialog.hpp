// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <QStringList>

namespace horcom {

/// The stacked button question box of the original alerte and alertbox
/// dialogs. One question per box, every answer a full width button in
/// its own row, the fallback answer last, exactly like the dialog
/// system Robert Rettig built for the file and parameter flows.
class ChoiceDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param title         the window title, his ue$ line
  /// @param info          centered lines above the buttons, may be empty
  /// @param buttons       the answers top to bottom, his ze$ rows
  /// @param default_index which answer carries the return key
  ChoiceDialog(const QString& title, const QStringList& info, const QStringList& buttons,
               int default_index, QWidget* parent = nullptr);

  /// @return the chosen answer, -1 after ESC or the close box
  [[nodiscard]] int choice() const { return choice_; }

  /// Runs the box modally.
  ///
  /// @return the zero based answer index, -1 when dismissed
  static int ask(QWidget* parent, const QString& title, const QStringList& info,
                 const QStringList& buttons, int default_index = 0);

 private:
  int choice_ = -1;
};

}  // namespace horcom
