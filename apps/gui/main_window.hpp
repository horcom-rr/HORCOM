// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QMainWindow>
#include <filesystem>
#include <optional>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/konsta.hpp"

class QAction;
class QCheckBox;
class QComboBox;
class QDate;
class QDateEdit;
class QTime;
class QTimer;
class QDoubleSpinBox;
class QLabel;
class QTableWidget;
class QTimeEdit;

namespace horcom {

class Banner;
class WheelWidget;

/// The main window, the wheel always visible, his data on docks beside
/// it, the workflow of the original in a resizable shell.
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(VsopTables vsop, Ephemerides eph, std::filesystem::path data_dir, QWidget* parent = nullptr);

  /// Switches the transit view on for the given UT moment, used by the
  /// capture hook and by workflows that open straight into transits.
  void show_transits(const QDate& date, const QTime& time);

  /// Jumps to the solar return of the given year, the capture hook's
  /// path into the Horoskop menu.
  void show_solar(int year);

  /// Switches the running clock chart on, the original UHR.
  void show_clock();

 private slots:
  void recompute();
  void open_records();
  void open_place();
  void pick_zone();
  void solar_chart();
  void lunar_chart();
  void save_aaf();
  void export_svg();
  void about();

 private:
  void build_ui();
  void fill_tables(const Chart& chart, const AspectResult& aspects);
  [[nodiscard]] ChartInput current_input() const;
  [[nodiscard]] ChartSettings current_settings() const;
  void apply_record(const AafRecord& r);
  void apply_moment(double jd_ut, const QString& label);
  void run_solar(int year);
  QString record_label_;

  VsopTables vsop_;
  Ephemerides eph_;
  std::filesystem::path data_dir_;
  Konsta konsta_;
  AspectSettings aspect_settings_;
  WheelWidget* wheel_ = nullptr;
  Banner* banner_ = nullptr;
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
  QCheckBox* transit_on_ = nullptr;
  QDateEdit* tdate_ = nullptr;
  QTimeEdit* ttime_ = nullptr;
  QAction* clock_action_ = nullptr;
  QTimer* clock_timer_ = nullptr;
  QTableWidget* bodies_ = nullptr;
  QTableWidget* cusps_ = nullptr;
  QLabel* aspects_label_ = nullptr;
  std::optional<Chart> last_chart_;
  std::optional<AspectResult> last_aspects_;
};

}  // namespace horcom
