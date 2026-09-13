// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>

#include "horcom/chart/progressions.hpp"
#include "horcom/chart/symbolic.hpp"
#include "horcom/chart/transit_search.hpp"

class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QTableWidget;

namespace horcom {

/// The direction evaluation of the original a18 tables in one dialog.
/// The symbolic and primary methods list arcs as ages, the secondary
/// and the two arc directions list dated events on the life axis.
class DirectionListDialog : public QDialog {
  Q_OBJECT

 public:
  DirectionListDialog(Chart radix, SearchContext ctx, QWidget* parent = nullptr);

  /// Runs the chosen method, also the capture hook's path.
  void run_scan();

 private:
  void update_fields();

  Chart radix_;
  SearchContext ctx_;
  QComboBox* method_ = nullptr;
  QDoubleSpinBox* from_years_ = nullptr;
  QDoubleSpinBox* to_years_ = nullptr;
  QDateEdit* from_date_ = nullptr;
  QDateEdit* to_date_ = nullptr;
  QComboBox* angle_ = nullptr;
  QDoubleSpinBox* key_ = nullptr;
  QComboBox* extras_ = nullptr;
  QTableWidget* table_ = nullptr;
  QLabel* count_ = nullptr;
};

}  // namespace horcom
