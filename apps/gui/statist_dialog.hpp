// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <optional>

#include "horcom/data/statist.hpp"

class QComboBox;
class QLabel;
class QTableWidget;

namespace horcom {

/// The statistics browser over his STATIST7 datasets. Loads a .STA with
/// its .PAR and .STH, lists the charts, shows the sign distribution of
/// a chosen object over the whole set, and hands one record back to the
/// shell.
class StatistDialog : public QDialog {
  Q_OBJECT

 public:
  explicit StatistDialog(QWidget* parent = nullptr);

  /// Loads a dataset, used by the open flow and the capture hook.
  ///
  /// @param sta the <NAME>.STA path
  /// @return true when the set loaded
  bool load(const QString& sta);

  /// @return the picked record after accept
  [[nodiscard]] const std::optional<StatRecord>& chosen() const { return chosen_; }

 private:
  void refresh_distribution();
  void accept_row(int row);

  StatSet set_;
  std::optional<StatRecord> chosen_;
  QComboBox* object_ = nullptr;
  QTableWidget* table_ = nullptr;
  QLabel* distribution_ = nullptr;
  QLabel* count_ = nullptr;
};

}  // namespace horcom
