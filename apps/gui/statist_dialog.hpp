// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <optional>
#include <vector>

#include "horcom/chart/aspects.hpp"
#include "horcom/data/statist.hpp"
#include "horcom/data/statist_eval.hpp"

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace horcom {

/// The statistics browser over his STATIST7 datasets. Loads a .STA with
/// its .PAR and .STH, lists the charts, shows the sign distribution of
/// a chosen object, runs the stat_ausw search conditions with UND
/// chaining over the survivors, and hands one record back to the shell.
class StatistDialog : public QDialog {
  Q_OBJECT

 public:
  explicit StatistDialog(const AspectSettings& aspects = {}, QWidget* parent = nullptr);

  /// Loads a dataset, used by the open flow and the capture hook.
  ///
  /// @param sta the <NAME>.STA path
  /// @return true when the set loaded
  bool load(const QString& sta);

  /// Takes an already built dataset, the capture hook's path.
  void load_set(StatSet set);

  /// @return the picked record after accept
  [[nodiscard]] const std::optional<StatRecord>& chosen() const { return chosen_; }

 private:
  void refresh_distribution();
  void accept_row(int row);
  void fill_operand(QComboBox* combo) const;
  [[nodiscard]] StatOperand operand_from(const QComboBox* combo) const;
  void update_eval_fields();
  void apply_condition();
  void reset_conditions();
  void count_file();

  StatSet set_;
  std::optional<StatRecord> chosen_;
  AspectSettings aspects_;
  std::vector<double> mask_;
  int conditions_ = 0;
  QComboBox* object_ = nullptr;
  QTableWidget* table_ = nullptr;
  QLabel* distribution_ = nullptr;
  QLabel* count_ = nullptr;
  // the condition builder of stat2
  QComboBox* eval_object_ = nullptr;
  QComboBox* op_a_ = nullptr;
  QComboBox* op_b_ = nullptr;
  QComboBox* op_c_ = nullptr;
  QComboBox* mirror_ = nullptr;
  QLineEdit* name_ = nullptr;
  QComboBox* window_ = nullptr;
  QDoubleSpinBox* degree_ = nullptr;
  QDoubleSpinBox* orb_ = nullptr;
  QComboBox* sign_ = nullptr;
  QSpinBox* house_ = nullptr;
  QSpinBox* house_pct_ = nullptr;
  QComboBox* near_ = nullptr;
  QSpinBox* asp_low_ = nullptr;
  QSpinBox* asp_high_ = nullptr;
  QDoubleSpinBox* asp_orb_ = nullptr;
  QCheckBox* und_ = nullptr;
  QPushButton* apply_ = nullptr;
  QLabel* eval_count_ = nullptr;
  QLabel* eval_dist_ = nullptr;
};

}  // namespace horcom
