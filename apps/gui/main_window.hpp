// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QMainWindow>
#include <optional>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"

class QCheckBox;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QTableWidget;
class QTimeEdit;

namespace horcom {

class WheelWidget;

/// The main window, the wheel always visible, his data on docks beside
/// it, the workflow of the original in a resizable shell.
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(VsopTables vsop, Ephemerides eph, QWidget* parent = nullptr);

 private slots:
  void recompute();
  void open_records();
  void save_aaf();
  void export_svg();
  void about();

 private:
  void build_ui();
  void fill_tables(const Chart& chart, const AspectResult& aspects);
  [[nodiscard]] ChartInput current_input() const;
  [[nodiscard]] ChartSettings current_settings() const;
  void apply_record(const AafRecord& r);

  VsopTables vsop_;
  Ephemerides eph_;
  WheelWidget* wheel_ = nullptr;
  QDateEdit* date_ = nullptr;
  QTimeEdit* time_ = nullptr;
  QDoubleSpinBox* zone_ = nullptr;
  QDoubleSpinBox* lon_ = nullptr;
  QDoubleSpinBox* lat_ = nullptr;
  QComboBox* houses_ = nullptr;
  QCheckBox* parallax_ = nullptr;
  QCheckBox* extras_ = nullptr;
  QCheckBox* true_node_ = nullptr;
  QCheckBox* true_apogee_ = nullptr;
  QLabel* header_ = nullptr;
  QTableWidget* bodies_ = nullptr;
  QTableWidget* cusps_ = nullptr;
  QLabel* aspects_label_ = nullptr;
  std::optional<Chart> last_chart_;
  std::optional<AspectResult> last_aspects_;
};

}  // namespace horcom
