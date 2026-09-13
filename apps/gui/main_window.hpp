// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QMainWindow>
#include <filesystem>
#include <optional>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/chart/harmonics.hpp"
#include "horcom/chart/transit_search.hpp"
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

  /// Opens the comparison view over the given partner record, the
  /// capture hook's path into the Vergleich toggle.
  void show_compare(const AafRecord& partner);

  /// Opens the composite over the given partner record.
  void show_composite(const AafRecord& partner);

  /// Opens the directed axes for an event moment, the capture hook's
  /// path into the Direktionen toggle.
  void show_directions(double jd_event_ut, bool converse);

  /// Switches the mundane view on, the horm 2 mode.
  void show_mundane();

  /// Switches the heliocentric mode on, the original hrg.
  void show_helio();

  /// Opens the harmonic double wheel, the capture hook's path.
  void show_harmonic(int n);

  /// Opens the 90 degree circle over a partner, the capture hook's path.
  void show_dial(const AafRecord& partner);

  /// Writes the current wheel as a PDF page, also the capture hook's
  /// path into the print world.
  ///
  /// @param path the target file
  /// @return true when the page was written
  bool export_pdf_to(const QString& path);

 private slots:
  void recompute();
  void open_records();
  void open_place();
  void pick_zone();
  void edit_record();
  void open_statistics();
  void open_aspektarium();
  void solar_chart();
  void lunar_chart();
  void septar_chart();
  void degree_list();
  void fixed_star_table();
  void arabic_table();
  void house_table();
  void great_year();
  void rise_set();
  void eclipse_table();
  void rhythm_table();
  void dynamogram_view();
  void midpoint_tree();
  void correction();
  void chain_files();
  void orb_settings();
  void converters();
  void planetar_chart();
  void personar_chart();
  void progression_chart();
  void day_chart();
  void transit_list();
  void ingress_table();
  void combin_chart();
  void save_aaf();
  void export_svg();
  void print_chart();
  void export_pdf();
  void about();

 private:
  void build_ui();
  void fill_tables(const Chart& chart, const AspectResult& aspects);
  [[nodiscard]] ChartInput current_input() const;
  [[nodiscard]] ChartSettings current_settings() const;
  void apply_record(const AafRecord& r);
  void apply_moment(double jd_ut, const QString& label);
  void run_solar(int year);
  void refresh_record_label();
  [[nodiscard]] SearchContext make_context() const;
  [[nodiscard]] std::optional<AafRecord> choose_record(const QString& title);
  [[nodiscard]] ChartInput record_input(const AafRecord& r) const;
  bool set_partner(const AafRecord& r);
  QString record_label_;
  AafRecord record_;

  VsopTables vsop_;
  Ephemerides eph_;
  std::filesystem::path data_dir_;
  Konsta konsta_;
  AspectSettings aspect_settings_;
  /// the user defined fixed point in radians, negative when off
  double fixpunkt_ = -1.0;
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
  QCheckBox* helio_ = nullptr;
  QCheckBox* transit_on_ = nullptr;
  QDateEdit* tdate_ = nullptr;
  QTimeEdit* ttime_ = nullptr;
  QAction* clock_action_ = nullptr;
  QTimer* clock_timer_ = nullptr;
  QAction* compare_action_ = nullptr;
  QAction* dial_action_ = nullptr;
  QAction* harmonic_action_ = nullptr;
  int harm_n_ = 0;
  bool harm_new_mc_ = false;
  QAction* multi_action_ = nullptr;
  MultiMode multi_mode_ = MultiMode::kMulti1;
  MultiReference multi_ref_;
  double multi_event_jd_ = 0.0;
  bool multi_new_mc_ = false;
  QAction* composite_action_ = nullptr;
  QAction* directions_action_ = nullptr;
  QAction* mundane_action_ = nullptr;
  double dir_jd_ = 0.0;
  bool dir_converse_ = false;
  double dir_vary_ = 0.0;
  std::optional<Chart> partner_chart_;
  ChartInput partner_input_;
  QString partner_name_;
  QTableWidget* bodies_ = nullptr;
  QTableWidget* cusps_ = nullptr;
  QLabel* aspects_label_ = nullptr;
  std::optional<Chart> last_chart_;
  std::optional<AspectResult> last_aspects_;
};

}  // namespace horcom
