// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <vector>

#include "horcom/chart/transit_search.hpp"

class QComboBox;
class QDoubleSpinBox;
class QDateEdit;
class QLabel;
class QTableWidget;

namespace horcom {

/// The transit event list, the modern face of the a180 sweep. A window,
/// a base angle, and the found moments as a table, one of them can be
/// picked to jump the shell into its transit view.
class TransitListDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param radix the birth chart whose positions form the targets
  /// @param ctx   observer and settings for the running sky
  /// @param parent the owning widget
  TransitListDialog(Chart radix, SearchContext ctx, QWidget* parent = nullptr);

  /// Presets the window, used by the capture hook.
  void preset(const QDate& from, const QDate& to);

  /// Runs the sweep and fills the table.
  void run_scan();

  /// @return the picked moment, zero when none was chosen
  [[nodiscard]] double chosen_jd() const { return chosen_jd_; }

 private:
  void accept_row(int row);

  Chart radix_;
  SearchContext ctx_;
  std::vector<TransitEvent> events_;
  double chosen_jd_ = 0.0;
  QDateEdit* from_ = nullptr;
  QDateEdit* to_ = nullptr;
  QComboBox* angle_ = nullptr;
  QDoubleSpinBox* ev_lon_ = nullptr;
  QDoubleSpinBox* ev_lat_ = nullptr;
  QTableWidget* table_ = nullptr;
  QLabel* count_ = nullptr;
};

}  // namespace horcom
