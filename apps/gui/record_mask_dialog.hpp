// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>

#include "horcom/data/aaf.hpp"

class QLineEdit;
class QLabel;

namespace horcom {

/// The EINGABE- und ANZEIGE-BOX of the original eing_box. Name and
/// place on the first row, the coordinates as degree, minute and second
/// boxes with their hemisphere letter, the date as TT MM JJJJ with the
/// 'V' flag for years before Christ, the clock as WZ = GMT = UT and the
/// BEMERKUNG line above the OK button.
class RecordMaskDialog : public QDialog {
  Q_OBJECT

 public:
  /// What the box is doing.
  enum class Mode {
    kShow,   ///< ANZEIGE while fetching, OK takes the record over
    kEntry,  ///< NEU-EINGABE, ABBRUCH ends the entry loop
  };

  /// @param record the record shown and edited
  /// @param title  the window title, RADIX NR.n while fetching
  RecordMaskDialog(AafRecord record, const QString& title, Mode mode, QWidget* parent = nullptr);

  /// @return the record with every edited field applied
  [[nodiscard]] AafRecord record() const;

 private:
  AafRecord base_;
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
