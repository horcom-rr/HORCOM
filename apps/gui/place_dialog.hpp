// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <filesystem>
#include <set>
#include <vector>

#include "horcom/data/countries.hpp"
#include "horcom/data/place_file.hpp"

class QComboBox;
class QPushButton;
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
  /// @param nima_table the landnima.int file naming the file prefixes
  /// @param parent the owning widget
  PlaceDialog(std::filesystem::path places_dir, const std::filesystem::path& nima_table,
              QWidget* parent = nullptr);

  /// @return the accepted place, valid after exec returns accepted
  [[nodiscard]] const PlaceRecord& chosen() const { return chosen_; }

  /// @return the accepted place name without the trailing zone letters
  [[nodiscard]] QString chosen_name() const;

  /// Binds the chooser to one file like his FILESELECT before the
  /// ausw_datei list, the file combo and the browse button go quiet.
  ///
  /// @param file the place file
  void lock_file(const std::filesystem::path& file);

  /// Turns the chooser into his LÖSCHEN list, places are marked by
  /// click until WAHL - ENDE.
  ///
  /// @param max_pick how many places may be marked, his ten
  void set_delete_mode(int max_pick);

  /// @return the marked places as indices in file order, valid after an
  ///         accepted delete mode run
  [[nodiscard]] const std::vector<std::size_t>& marked() const { return marked_; }

 private:
  void scan_directory();
  void load_current_file();
  void browse();
  void refresh();
  void accept_row(int row);
  void accept_marks();

  std::filesystem::path dir_;
  std::vector<NimaCountry> nima_;
  std::vector<PlaceRecord> records_;
  // the file position of every sorted record, the LÖSCHEN pass works on
  // the file order
  std::vector<std::size_t> file_index_;
  std::vector<std::size_t> marked_;
  // his nd&() marks of the LÖSCHEN list as indices into records_, they
  // outlive a new filter
  std::set<std::size_t> marks_;
  // the table is being rebuilt, its selection signals are no clicks
  bool refreshing_ = false;
  int max_pick_ = 0;
  PlaceRecord chosen_;
  QComboBox* files_ = nullptr;
  QPushButton* browse_ = nullptr;
  QLineEdit* filter_ = nullptr;
  QTableWidget* table_ = nullptr;
  QLabel* count_ = nullptr;
};

}  // namespace horcom
