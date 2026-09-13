// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <vector>

#include "horcom/data/aaf.hpp"
#include "horcom/data/countries.hpp"

class QComboBox;
class QLineEdit;

namespace horcom {

/// The record mask, the personal side of the original NEU-EINGABE. The
/// moment and the coordinates stay in the main panel, this mask carries
/// the person, the place name, the country abbreviation and the summer
/// time code of the AAF field.
class RecordDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param record    the current record whose personal fields preset
  /// @param countries the German abbreviation table for the Land box
  /// @param parent    the owning widget
  RecordDialog(AafRecord record, const std::vector<GermanCountry>& countries, QWidget* parent = nullptr);

  /// @return the record with the mask fields merged in
  [[nodiscard]] AafRecord record() const;

 private:
  AafRecord base_;
  QLineEdit* surname_ = nullptr;
  QLineEdit* given_ = nullptr;
  QComboBox* sex_ = nullptr;
  QLineEdit* place_ = nullptr;
  QComboBox* country_ = nullptr;
  QComboBox* dst_ = nullptr;
  QLineEdit* comment_ = nullptr;
};

}  // namespace horcom
