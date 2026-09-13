// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <filesystem>
#include <vector>

#include "horcom/data/place_file.hpp"

class QComboBox;
class QLabel;
class QLineEdit;
class QTableWidget;

namespace horcom {

/// The place search, the modern face of the original file select over
/// SPEZ_ORT with its incremental SUCHEN scan. Lists the place files of a
/// directory, filters by substring and hands back one record.
class PlaceDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param places_dir directory scanned for 36 byte place files
  /// @param parent the owning widget
  explicit PlaceDialog(std::filesystem::path places_dir, QWidget* parent = nullptr);

  /// @return the accepted place, valid after exec returns accepted
  [[nodiscard]] const PlaceRecord& chosen() const { return chosen_; }

 private:
  void scan_directory();
  void load_current_file();
  void browse();
  void refresh();
  void accept_row(int row);

  std::filesystem::path dir_;
  std::vector<PlaceRecord> records_;
  PlaceRecord chosen_;
  QComboBox* files_ = nullptr;
  QLineEdit* filter_ = nullptr;
  QTableWidget* table_ = nullptr;
  QLabel* count_ = nullptr;
};

}  // namespace horcom
