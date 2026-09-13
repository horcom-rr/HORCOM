// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <array>

#include "horcom/chart/transit_search.hpp"

class QComboBox;
class QDateEdit;
class QTableWidget;

namespace horcom {

/// The sign entry table of ingre1, a body and a year's twelve ingresses,
/// one of them can be picked to jump the shell into its transit view.
class IngressDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param ctx observer and settings for the running sky
  /// @param parent the owning widget
  explicit IngressDialog(SearchContext ctx, QWidget* parent = nullptr);

  /// Runs the search and fills the table.
  void run_scan();

  /// @return the picked moment, zero when none was chosen
  [[nodiscard]] double chosen_jd() const { return chosen_jd_; }

 private:
  void accept_row(int row);

  SearchContext ctx_;
  std::array<LongitudeCrossing, 12> table_data_{};
  double chosen_jd_ = 0.0;
  QComboBox* body_ = nullptr;
  QDateEdit* when_ = nullptr;
  QTableWidget* table_ = nullptr;
};

}  // namespace horcom
