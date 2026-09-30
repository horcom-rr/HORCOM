// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <QStringList>
#include <vector>

class QKeyEvent;

namespace horcom {

/// The stacked button question box of the original alerte and alertbox
/// dialogs. One question per box, every answer a full width button in
/// its own row, the fallback answer last, exactly like the dialog
/// system Robert Rettig built for the file and parameter flows.
class ChoiceDialog : public QDialog {
  Q_OBJECT

 public:
  /// Builds the box, shown by the caller or through one of the ask
  /// functions.
  ///
  /// @param title         the window title, his ue$ line
  /// @param info          centered lines above the buttons, may be empty
  /// @param buttons       the answers top to bottom, his ze$ rows
  /// @param default_index which answer carries the return key
  /// @param parent        the window the box belongs to
  ChoiceDialog(const QString& title, const QStringList& info, const QStringList& buttons,
               int default_index, QWidget* parent = nullptr);

  /// @return the chosen answer, -1 after ESC or the close box
  [[nodiscard]] int choice() const { return choice_; }

  /// Runs the box modally.
  ///
  /// @param parent        the window the box belongs to
  /// @param title         the window title
  /// @param info          centered lines above the buttons
  /// @param buttons       the answers top to bottom
  /// @param default_index which answer carries the return key
  /// @return the zero based answer index, -1 when dismissed
  static int ask(QWidget* parent, const QString& title, const QStringList& info,
                 const QStringList& buttons, int default_index = 0);

  /// Runs the box with rows that stand but cannot be chosen, his alerte
  /// rows handed in as numbers.
  ///
  /// @param parent        the window the box belongs to
  /// @param title         the window title
  /// @param info          centered lines above the buttons
  /// @param buttons       the answers top to bottom
  /// @param default_index which answer carries the return key
  /// @param disabled      the zero based rows shown grey
  /// @return the zero based answer index, -1 when dismissed
  static int ask_with_disabled(QWidget* parent, const QString& title, const QStringList& info,
                               const QStringList& buttons, int default_index, const std::vector<int>& disabled);

  /// The answer of a box whose R or PgUp stepped back, his zurueck!.
  static constexpr int kBack = -2;

  /// Runs a box of a wizard chain, R and PgUp step back like his alertbox
  /// with zurueck!.
  ///
  /// @param parent        the window the box belongs to
  /// @param title         the window title
  /// @param info          centered lines above the buttons
  /// @param buttons       the answers top to bottom
  /// @param default_index which answer carries the return key
  /// @param disabled      the zero based rows shown grey
  /// @return the zero based answer, -1 after ESC, kBack after R or PgUp
  static int ask_step(QWidget* parent, const QString& title, const QStringList& info, const QStringList& buttons,
                      int default_index, const std::vector<int>& disabled = {});

 protected:
  void keyPressEvent(QKeyEvent* e) override;

 private:
  /// Greys the given rows and runs the box modally.
  ///
  /// @param disabled the zero based rows shown grey
  /// @return the zero based answer, -1 when dismissed
  int run(const std::vector<int>& disabled);

  int choice_ = -1;
  bool back_ = false;
};

}  // namespace horcom
