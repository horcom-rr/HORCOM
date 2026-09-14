// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <vector>

#include "horcom/data/kommen.hpp"

class QLineEdit;
class QListWidget;
class QTextBrowser;

namespace horcom {

/// The TEXT-DATEI LESEN box over Robert Rettig's commentary texts,
/// shipped as markdown editions in the data folder and rendered as
/// such. The picker also reads an original KOMMEN7P folder kept
/// somewhere else, those show through the ported lese_text rules.
class KommenDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param dir the local kommen folder, data/kommen by convention
  explicit KommenDialog(const std::filesystem::path& dir, QWidget* parent = nullptr);

 private:
  void reload(const std::filesystem::path& dir);
  void show_entry(int row);
  void search();

  std::vector<KommenEntry> entries_;
  QListWidget* list_ = nullptr;
  QTextBrowser* text_ = nullptr;
  QLineEdit* find_ = nullptr;
};

}  // namespace horcom
