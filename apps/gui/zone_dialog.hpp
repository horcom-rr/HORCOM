// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <filesystem>
#include <vector>

#include "horcom/data/zone_names.hpp"

class QLineEdit;
class QTableWidget;

namespace horcom {

/// The zone catalogue picker, the modern face of the original dialog
/// "Zeit-Zonen (P.D. Via B.MAHL)" of zeitzon_nam_horc.
class ZoneDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param catalogue the zonnamen.int file
  /// @param parent the owning widget
  explicit ZoneDialog(const std::filesystem::path& catalogue, QWidget* parent = nullptr);

  /// @return true when the catalogue file was found and read
  [[nodiscard]] bool loaded() const { return !entries_.empty(); }

  /// @return the accepted zone, valid after exec returns accepted
  [[nodiscard]] const ZoneEntry& chosen() const { return chosen_; }

 private:
  void refresh();
  void accept_row(int row);

  std::vector<ZoneEntry> entries_;
  ZoneEntry chosen_;
  QLineEdit* filter_ = nullptr;
  QTableWidget* table_ = nullptr;
};

}  // namespace horcom
