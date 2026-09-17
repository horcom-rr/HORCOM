// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <filesystem>

#include "horcom/data/aaf.hpp"

class QLineEdit;
class QPlainTextEdit;

namespace horcom {

/// The AAF- Eingabe- und Anzeige- Box of the original aaf_box, the
/// richer entry form for the AAF exchange format. It edits the full
/// AAF record, the standard data of groups AAF-A and AAF-B and the
/// extra data of group AAF-C (comment, source, quality, catchword and
/// the rest), so the whole information a fetched AAF record carries can
/// be seen and changed, not just the fields the plain HORCOM box holds.
class AafMaskDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param record   the record shown and edited, all AAF fields
  /// @param kommen   the commentary folder, F1 opens the AAF help there
  /// @param parent   the owner
  AafMaskDialog(AafRecord record, std::filesystem::path kommen, QWidget* parent = nullptr);

  /// @return the record with every edited field applied
  [[nodiscard]] AafRecord record() const;

 protected:
  void keyPressEvent(QKeyEvent* event) override;

 private:
  AafRecord base_;
  std::filesystem::path kommen_;
  QLineEdit* surname_ = nullptr;
  QLineEdit* given_ = nullptr;
  QLineEdit* sex_ = nullptr;
  QLineEdit* bc_ = nullptr;
  QLineEdit* day_ = nullptr;
  QLineEdit* month_ = nullptr;
  QLineEdit* year_ = nullptr;
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
  QLineEdit* zone_ = nullptr;
  QLineEdit* dst_ = nullptr;
  QPlainTextEdit* com_ = nullptr;
  QLineEdit* via_ = nullptr;
  QLineEdit* src_ = nullptr;
  QLineEdit* gzq_ = nullptr;
  QLineEdit* znam_ = nullptr;
  QLineEdit* cword_ = nullptr;
  QLineEdit* attrb_ = nullptr;
};

}  // namespace horcom
