// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <filesystem>
#include <optional>

#include "horcom/data/aaf.hpp"

class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

namespace horcom {

/// The AAF- Eingabe- und Anzeige- Box of the original aaf_box, the
/// richer entry form for the AAF exchange format. It edits the full
/// AAF record, the standard data of groups AAF-A and AAF-B and the
/// extra data of group AAF-C (comment, source, quality, catchword and
/// the rest), so the whole information a fetched AAF record carries can
/// be seen and changed, not just the fields the plain HORCOM box holds.
/// Entering the Horoskopart, Land, Sommerzeit and Zone fields opens his
/// pick lists, entering the Ortsname his EREIGNIS-ORT menu. OK =
/// Speichern writes the record into an AAF file and rebuilds the DAT
/// twin like CASE 168 of aaf_box.
class AafMaskDialog : public QDialog {
  Q_OBJECT

 public:
  /// The states of his aafein! and aafaend! flags.
  enum class Mode {
    kEntry,  ///< aafein!, a new record, OK = Speichern
    kEdit,   ///< aafaend!, a changed record, OK = Speichern
    kShow,   ///< neither, Zurück zum HORCOM-Format and Datensatz ÄNDERN
  };

  /// Opens the box in edit mode.
  ///
  /// @param record   the record shown and edited, all AAF fields
  /// @param data_dir the data folder, F1 opens the AAF help from its
  ///                 kommen folder, the place menu reads ort.ext there,
  ///                 the pick lists read laender.int and zonnamen.int
  /// @param parent   the owner
  AafMaskDialog(AafRecord record, std::filesystem::path data_dir, QWidget* parent = nullptr);

  /// @param record   the record shown and edited, all AAF fields
  /// @param mode     entry, edit or show
  /// @param data_dir the data folder like above
  /// @param parent   the owner
  AafMaskDialog(AafRecord record, Mode mode, std::filesystem::path data_dir, QWidget* parent = nullptr);

  /// Presets the file of the save, his aaffile$ of the working Daten-Datei.
  ///
  /// @param aaf the AAF file the save picker opens with, may not exist
  void set_aaf_file(std::filesystem::path aaf);

  /// @return the record with every edited field applied
  [[nodiscard]] AafRecord record() const;

  /// @return the AAF file OK = Speichern wrote, nothing when the box
  ///         closed without saving
  [[nodiscard]] const std::optional<std::filesystem::path>& saved_file() const { return saved_; }

 protected:
  void keyPressEvent(QKeyEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;

 private:
  void set_mode(Mode mode);
  void place_menu();
  void clear_place();
  void kind_menu();
  void country_menu();
  void summer_menu();
  void zone_menu();
  void jd_priority();
  void jd_taken();
  void year_check();
  void show_calendar_note();
  void letter_check(QLineEdit* field, char first, char second);
  void save();
  [[nodiscard]] bool store(const std::filesystem::path& aaf);

  AafRecord base_;
  Mode mode_ = Mode::kEdit;
  std::filesystem::path data_dir_;
  std::filesystem::path kommen_;
  std::filesystem::path aaf_file_;
  std::optional<std::filesystem::path> saved_;
  // his vorz_neu!, a preferred place typed by hand is stored on the way
  // into the zone field
  bool preferred_new_ = false;
  // his jua!, the JULDATUM PRIORITÄT question was answered with JA
  bool jd_open_ = false;
  // the calendar suffix already checked, his cg! and cj!
  bool gregorian_checked_ = false;
  bool julian_checked_ = false;
  QLineEdit* surname_ = nullptr;
  QLineEdit* given_ = nullptr;
  QLineEdit* sex_ = nullptr;
  QLineEdit* day_ = nullptr;
  QLineEdit* month_ = nullptr;
  QLineEdit* year_ = nullptr;
  QLabel* calendar_note_ = nullptr;
  QLineEdit* hour_ = nullptr;
  QLineEdit* minute_ = nullptr;
  QLineEdit* second_ = nullptr;
  QLineEdit* place_ = nullptr;
  QLineEdit* country_ = nullptr;
  QLineEdit* lat_ns_ = nullptr;
  QLineEdit* lat_deg_ = nullptr;
  QLineEdit* lat_min_ = nullptr;
  QLineEdit* lat_sec_ = nullptr;
  QLineEdit* lon_ew_ = nullptr;
  QLineEdit* lon_deg_ = nullptr;
  QLineEdit* lon_min_ = nullptr;
  QLineEdit* lon_sec_ = nullptr;
  QLineEdit* jd_ = nullptr;
  double jd_seed_ = 0.0;
  QLineEdit* zone_ = nullptr;
  QLineEdit* dst_ = nullptr;
  QPlainTextEdit* com_ = nullptr;
  QLineEdit* via_ = nullptr;
  QLineEdit* src_ = nullptr;
  QLineEdit* gzq_ = nullptr;
  QLineEdit* znam_ = nullptr;
  QLineEdit* cword_ = nullptr;
  QLineEdit* attrb_ = nullptr;
  QPushButton* save_ = nullptr;
  QPushButton* back_ = nullptr;
  QPushButton* change_ = nullptr;
};

}  // namespace horcom
