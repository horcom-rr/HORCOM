// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <QStringList>
#include <filesystem>
#include <optional>

#include "horcom/data/aaf.hpp"

class QCheckBox;
class QLineEdit;
class QLabel;
class QPushButton;

namespace horcom {

/// The EINGABE- und ANZEIGE-BOX of the original eing_box. Name and
/// place on the first row, the coordinates as degree, minute and second
/// boxes with their hemisphere letter, the date as TT MM JJJJ with the
/// 'V' flag for years before Christ, the clock and the BEMERKUNG line
/// above the OK button. The NEU-EINGABE mask reads the clock as UHRZEIT
/// and adds his zone box, the ORTSZEIT and NOCH JULIANISCH switches and
/// the ZEITBESTIMMUNGEN texts. Tabbing into ORT opens his place menu,
/// tabbing out of the coordinates asks ORT ABSPEICHERN ? for a new place.
class RecordMaskDialog : public QDialog {
  Q_OBJECT

 public:
  /// What the box is doing.
  enum class Mode {
    kShow,   ///< ANZEIGE while fetching, OK takes the record over
    kEntry,  ///< NEU-EINGABE, ABBRUCH ends the entry loop
  };

  /// @param record   the record shown and edited
  /// @param title    the window title, RADIX NR.n while fetching
  /// @param mode     ANZEIGE or NEU-EINGABE
  /// @param data_dir the data folder. The fetch mask offers the AAF box
  ///                 with its help from the kommen folder, the entry mask
  ///                 reads its zone lists and texts, empty hides both
  /// @param parent   the owner
  RecordMaskDialog(AafRecord record, const QString& title, Mode mode,
                   std::filesystem::path data_dir = {}, QWidget* parent = nullptr);

  /// @return the record with every edited field applied, in NEU-EINGABE
  ///         the zone of the zone box and the Julian calendar included
  [[nodiscard]] AafRecord record() const;

  /// @return true when ORTSZEIT (HISTORISCHE HOROSKOPE) or NOCH JULIANISCH
  ///         was ticked, the clock is then local time of the longitude
  [[nodiscard]] bool local_time() const;

  /// Ticks ORTSZEIT again when an entry comes back for editing.
  ///
  /// @param on the state of the switch
  void preset_local_time(bool on);

  /// Locks the fields behind the last editable one like his nne& of
  /// eingabe, 10 ends with the coordinates, 14 with the date and 17 with
  /// the clock. The AAF box leaves with them, it edits the whole record.
  ///
  /// @param last his field number of the last editable field
  void limit_fields(int last);

  /// Binds the ANZEIGE box to the working Daten-Datei like his datru$.
  /// An edit then raises his ABSPEICHERN question on OK, and with an AAF
  /// twin beside the file first his warning that changes belong there.
  ///
  /// @param file_label the file name the question shows
  /// @param aaf_file   the AAF twin of the file, may not exist, the AAF
  ///                   box saves there by default
  void bind_file(const QString& file_label, std::filesystem::path aaf_file);

  /// @return true when the edit question was answered with ABSPEICHERN
  [[nodiscard]] bool save_requested() const { return save_requested_; }

  /// @return the AAF file the AAF box saved into, nothing when it did not
  [[nodiscard]] const std::optional<std::filesystem::path>& aaf_saved() const { return aaf_saved_; }

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

 private:
  void fill_fields(const AafRecord& r);
  [[nodiscard]] QStringList field_texts() const;
  [[nodiscard]] bool edited() const;
  void finish();
  void open_aaf_box();
  void place_menu();
  void place_leave();
  void take_place(const QString& name, double lon, double lat);
  [[nodiscard]] QStringList place_fields() const;
  void zone_box(bool on);
  void show_zzd();

  AafRecord base_;
  Mode mode_;
  std::filesystem::path data_dir_;
  // his datru$ and aaffile$ of the ANZEIGE box, and the flags of his
  // ABSPEICHERN answer
  QString file_label_;
  std::filesystem::path aaf_file_;
  bool bound_ = false;
  bool save_requested_ = false;
  std::optional<std::filesystem::path> aaf_saved_;
  // his i$(1..18), the fields as the box opened them for his afg! test
  QStringList opened_;
  // his zeitzon and somz of the zone box, zone time plus zzd gives UT
  double zzd_ = 0.0;
  int summer_ = 0;
  QCheckBox* zones_ = nullptr;
  QLabel* zzd_label_ = nullptr;
  QLabel* clock_label_ = nullptr;
  QPushButton* aaf_ = nullptr;
  QCheckBox* local_ = nullptr;
  QCheckBox* julian_ = nullptr;
  // his neu!, vorz! and holda! of the ORT menu, and i$(2..10) the place
  // fields as the menu last saw them for his afo test
  bool neu_ = false;
  bool vorz_ = false;
  bool holda_ = false;
  QStringList place_seen_;
  QString joined_name_;
  QLineEdit* name_ = nullptr;
  QLineEdit* place_ = nullptr;
  QLineEdit* lon_ew_ = nullptr;
  QLineEdit* lon_deg_ = nullptr;
  QLineEdit* lon_min_ = nullptr;
  QLineEdit* lon_sec_ = nullptr;
  QLineEdit* lat_ns_ = nullptr;
  QLineEdit* lat_deg_ = nullptr;
  QLineEdit* lat_min_ = nullptr;
  QLineEdit* lat_sec_ = nullptr;
  QLineEdit* bc_ = nullptr;
  QLineEdit* day_ = nullptr;
  QLineEdit* month_ = nullptr;
  QLineEdit* year_ = nullptr;
  QLineEdit* hour_ = nullptr;
  QLineEdit* minute_ = nullptr;
  QLineEdit* second_ = nullptr;
  QLineEdit* comment_ = nullptr;
};

}  // namespace horcom
