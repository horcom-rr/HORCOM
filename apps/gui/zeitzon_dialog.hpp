// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <filesystem>

class QCheckBox;
class QLineEdit;
class QListWidget;

namespace horcom {

/// The zone box behind MEZ und SONSTIGE ZONEN-ZEITEN of his NEU-EINGABE,
/// AUSWAHL der ZEIT - ZONE. The difference field reads like his, zone
/// time plus the value gives UT, so MEZ is minus one. The EUROPA and
/// WELT lists offer a nearby place whose zone fills the field, the two
/// summer boxes add one or two hours on top.
class ZeitzonDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param data_dir the data folder with places, zeitbest and zonnamen.int
  /// @param parent   the owning widget
  explicit ZeitzonDialog(std::filesystem::path data_dir, QWidget* parent = nullptr);

  /// @return his ZZD in hours, zone time plus ZZD gives UT, summer time
  ///         included
  [[nodiscard]] double zzd() const;

  /// @return the summer time the boxes chose, 0, 1 for DSZ, 2 for DDSZ
  [[nodiscard]] int summer() const;

 private:
  void set_base(double zzd, bool clear_summer);
  void show_field();

  std::filesystem::path data_dir_;
  double base_ = -1.0;
  QLineEdit* field_ = nullptr;
  QCheckBox* dsz_ = nullptr;
  QCheckBox* ddsz_ = nullptr;
  QListWidget* europa_ = nullptr;
  QListWidget* welt_ = nullptr;
};

/// ZEITBESTIMMUNGEN LESEN, the FILESELECT over his ZEITBEST texts and
/// the TEXT-DATEI LESEN box that shows the chosen one.
///
/// @param parent   the owning widget
/// @param data_dir the data folder, the texts live in its zeitbest folder
void read_time_determinations(QWidget* parent, const std::filesystem::path& data_dir);

}  // namespace horcom
