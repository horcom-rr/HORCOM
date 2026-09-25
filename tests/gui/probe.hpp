// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QTimeEdit>
#include <filesystem>
#include <functional>
#include <memory>

#include "horcom/data/aaf.hpp"
#include <QLabel>

#include "horcom/data/chart_file.hpp"
#include "main_window.hpp"
#include "banner.hpp"
#include "wheel_widget.hpp"

namespace horcom {

// the tests reach the private panel state of the shell through this
struct MainWindowProbe {
  static std::unique_ptr<MainWindow> make() {
    VsopTables vsop = VsopTables::load(HORCOM_TEST_DATA_DIR "/planets.ndx", HORCOM_TEST_DATA_DIR "/planets.dat");
    Ephemerides eph{HORCOM_TEST_DATA_DIR "/eph"};
    auto w = std::make_unique<MainWindow>(std::move(vsop), std::move(eph), HORCOM_TEST_DATA_DIR);
    // the shipped konsta.int stays untouched, kon_dsp writes to scratch
    w->konsta_file_ = std::filesystem::temp_directory_path() / "horcom_gui_tests_konsta.int";
    // the suites drive the outputs without the HARDCOPY box whatever the
    // data folder holds, the print tests switch the DRUCKER-OPTION on
    w->konsta_.prenbl = 0;
    return w;
  }
  static double panel_jd(const MainWindow& w) {
    return julian_day(w.current_input().date_ut, w.current_settings().calendar);
  }
  static void apply(MainWindow& w, const AafRecord& r) { w.apply_record(r); }
  static void full_reset(MainWindow& w) { w.full_reset(); }
  static void wart(MainWindow& w, int item, bool more_follows) { w.wart(item, more_follows); }
  static void on_restart(MainWindow& w, std::function<void()> f) { w.restart_ = std::move(f); }
  static AafRecord record(const MainWindow& w) { return w.panel_record(); }
  static ChartRecord dat(const MainWindow& w, const AafRecord& r) { return w.dat_from_record(r); }
  static void moment(MainWindow& w, double jd, bool keep_clock) { w.apply_moment(jd, "TEST", false, keep_clock); }
  static CalendarDate day(const MainWindow& w) { return w.panel_day(); }
  static bool day_valid(const MainWindow& w) { return w.panel_day_valid(); }
  static QLineEdit* date(MainWindow& w) { return w.date_; }
  static QTimeEdit* time(MainWindow& w) { return w.time_; }
  static QDoubleSpinBox* zone(MainWindow& w) { return w.zone_; }
  static QCheckBox* sommerzeit(MainWindow& w) { return w.sommerzeit_; }
  static QCheckBox* julian(MainWindow& w) { return w.julian_; }
  static double dst(const MainWindow& w) { return w.dst_hours_; }
  static Calendar calendar(const MainWindow& w) { return w.panel_calendar_; }
  static ClockKind clock(const MainWindow& w) { return w.clock_kind_; }
  static void local_time(MainWindow& w, bool on) { w.set_local_time(on, false); }
  static double lon(const MainWindow& w) { return w.lon_->value(); }
  static const Chart& chart(const MainWindow& w) { return *w.last_chart_; }
  static void correction(MainWindow& w) { w.correction(); }
  static void prima(MainWindow& w) { w.primary_axes_session(); }
  static bool axes_shown(const MainWindow& w) { return w.directions_action_->isChecked(); }
  static void transite(MainWindow& w) { w.transite(); }
  static bool transit_on(const MainWindow& w) { return w.transit_on_->isChecked(); }
  static QDate transit_date(const MainWindow& w) { return w.tdate_->date(); }
  static bool arc_on(const MainWindow& w) { return w.arc_action_->isChecked(); }
  static double arc_prog_jd(const MainWindow& w) { return w.arc_prog_jd_; }
  static void day_chart(MainWindow& w) { w.day_chart(); }
  static void progression(MainWindow& w) { w.progression_chart(); }
  static void lunar(MainWindow& w) { w.lunar_chart(); }
  static void return_list(MainWindow& w, bool lunar) { w.return_list(lunar); }
  static void horoskop_graphik(MainWindow& w) { w.horoskop_graphik(); }
  static void printer_option_entry(MainWindow& w) { w.printer_option_entry(); }
  static void double_capture(MainWindow& w) { w.double_capture(&w); }
  static std::size_t double_count(const MainWindow& w) { return w.double_buffer_.size(); }
  static bool double_active(const MainWindow& w) { return w.double_active_; }
  static void last_picture(MainWindow& w) { w.last_picture(); }
  static void helio(MainWindow& w, bool on) { w.helio_->setChecked(on); }
  static void solar(MainWindow& w) { w.solar_chart(); }
  static void planetar(MainWindow& w) { w.planetar_chart(); }
  static QString banner_record(const MainWindow& w) { return w.banner_->record(); }
  static ClassicSheetText sheet(const MainWindow& w) { return w.classic_sheet_text(); }
  static ChartInput radix_input(const MainWindow& w) { return w.radix_input(); }
  static SearchContext context(const MainWindow& w) { return w.make_context(); }
  static void house_table(MainWindow& w) { w.house_table(); }
  static void eclipse_table(MainWindow& w) { w.eclipse_table(); }
  static void rise_set(MainWindow& w) { w.rise_set(); }
  static void time_wander(MainWindow& w) { w.time_wander(); }
  static void place_wander(MainWindow& w) { w.place_wander(); }
  static void uhr(MainWindow& w) { w.uhr(); }
  static void dynamogram_view(MainWindow& w) { w.dynamogram_view(); }
  static void rhythm(MainWindow& w) { w.rhythm(); }
  static void coordinate_table(MainWindow& w, bool extras) { w.coordinate_table(extras); }
  static void degree_list(MainWindow& w) { w.degree_list(); }
  static void arabic_table(MainWindow& w) { w.arabic_table(); }
  static void ingress_table(MainWindow& w) { w.ingress_table(); }
  static void fixed_star_table(MainWindow& w) { w.fixed_star_table(); }
  static void vorgaben_ephemeride(MainWindow& w) { w.vorgaben_ephemeride(); }
  static double& fixpunkt(MainWindow& w) { return w.fixpunkt_; }
  static void activate_solar(MainWindow& w, int i) {
    w.active_is_solar_ = true;
    w.active_solar_ = i;
  }
  static void degree_date_list(MainWindow& w) { w.degree_date_list(); }
  static void rhythm_define_degree(MainWindow& w) { w.rhythm_define_degree(); }
  static void rhythm_delete_degrees(MainWindow& w) { w.rhythm_delete_degrees(); }
  static void set_data_dir(MainWindow& w, const std::filesystem::path& dir) { w.data_dir_ = dir; }
  static void septar(MainWindow& w) { w.septar_chart(); }
  static void secondary_direction_run(MainWindow& w) { w.secondary_direction(); }
  static std::pair<double, double> step_dates(double from, double to, bool forward) {
    MainWindow::A18Answers a;
    a.jd_from = from;
    a.jd_to = to;
    MainWindow::step_linear_window(a, forward, Calendar::kAuto);
    return {a.jd_from, a.jd_to};
  }
  static std::pair<double, double> step_years(double from, double to, bool forward) {
    MainWindow::A18Answers a;
    a.from_years = from;
    a.to_years = to;
    MainWindow::step_linear_window(a, forward, Calendar::kAuto);
    return {a.from_years, a.to_years};
  }
  static bool uhr_on(const MainWindow& w) { return w.uhr_on_; }
  static int uhr_slot(const MainWindow& w) { return w.uhr_slot_; }
  static QString clock_strip(const MainWindow& w) { return w.clock_strip_ != nullptr ? w.clock_strip_->text() : QString(); }
  static QString help_stem(const MainWindow& w) { return w.help_stem_; }
  static void set_help_stem(MainWindow& w, const QString& stem) { w.help_stem_ = stem; }
  static bool scrout(const MainWindow& w) { return w.scrout_; }
  static void erste_hilfe(MainWindow& w) { w.erste_hilfe(); }
  static void anmerkungen(MainWindow& w) { w.anmerkungen(); }
  static void toggle_helio(MainWindow& w) { w.toggle_helio(); }
  static void toggle_printer_option(MainWindow& w) { w.toggle_printer_option(); }
  static void store_solar(MainWindow& w, const QString& label) { w.store_solar(label); }
  static void result_as_radix(MainWindow& w) { w.result_as_radix(); }
  static void desktop_quit(MainWindow& w) { w.desktop_quit(); }
  static bool quit_confirmed(const MainWindow& w) { return w.quit_confirmed_; }
  static void background_colors(MainWindow& w) { w.background_colors(); }
  static void et_from_ut(MainWindow& w) { w.et_from_ut(); }
  static void ut_from_et(MainWindow& w) { w.ut_from_et(); }
  static void date_from_jd(MainWindow& w) { w.date_from_jd(); }
  static void arde_from_eleb(MainWindow& w) { w.arde_from_eleb(); }
  static void eleb_from_arde(MainWindow& w) { w.eleb_from_arde(); }
  static void local_time_convert(MainWindow& w, bool from_ut) { w.local_time_convert(from_ut); }
  static QString et_ut_note(int year) { return MainWindow::et_ut_note(year); }
  static void great_year(MainWindow& w) { w.great_year(); }
  static bool great_year_on(const MainWindow& w) { return w.great_year_on_; }
  static void vorgaben_overview(MainWindow& w) { w.vorgaben_overview(); }
  static void houses(MainWindow& w, int haw) { w.preset_houses(haw); }
  static void hide(MainWindow& w, int slot) {
    w.emphasis_[static_cast<std::size_t>(slot)] = -1;
    w.recompute();
  }
  static const AspectResult& aspects(const MainWindow& w) { return *w.last_aspects_; }
  static const DisplayList& wheel(const MainWindow& w) { return w.wheel_->display_list(); }
  static Konsta& konsta(MainWindow& w) { return w.konsta_; }
  static DisplayList a4(const MainWindow& w) { return w.a4_export_list(); }
  static DisplayList classic_export(const MainWindow& w) { return w.classic_export_list(); }
  static Histogram histogram(const MainWindow& w) { return w.sheet_histogram(*w.last_chart_, w.current_settings()); }
  static void histogram_view(MainWindow& w) { w.histogram_view(); }
  static ChartSettings settings(const MainWindow& w) { return w.current_settings(); }
  static void transit_list(MainWindow& w) { w.transit_list(); }
  static void mundane_aspects(MainWindow& w) { w.mundane_aspects(); }
  static void vorgaben_horoskop(MainWindow& w) { w.vorgaben_horoskop(); }
  static void midpoint_tree(MainWindow& w) { w.midpoint_tree(); }
  static void open_aspektarium(MainWindow& w) { w.open_aspektarium(); }
  static void put_slot(MainWindow& w, int i, const AafRecord& r) { w.set_slot(i, r, false); }
  static void double_wheel_session(MainWindow& w) { w.double_wheel_session(); }
  static void multi_session(MainWindow& w) { w.multi_session(); }
  static void composite_session(MainWindow& w) { w.composite_session(); }
  static bool composite_on(const MainWindow& w) { return w.composite_action_->isChecked(); }
  static std::optional<std::string> residence(const MainWindow& w) {
    return w.comp_residence_ ? std::optional<std::string>(w.comp_residence_->name) : std::nullopt;
  }
  static bool body_column_hidden(const MainWindow& w, int col) { return w.bodies_->isColumnHidden(col); }
  static void recall_double(MainWindow& w, int kind) { w.recall_double(kind); }
  static void reset_views(MainWindow& w) { w.reset_views(); }
  static void combin_chart(MainWindow& w) { w.combin_chart(); }
  static void statistics_hub(MainWindow& w) { w.statistics_hub(); }
  static QString count_line(const MainWindow& w, const std::array<int, 4>& n, bool quarter) {
    return w.midpoint_count_line(n, quarter);
  }
  static QString slot_text(const MainWindow& w, int i) { return w.slot_actions_[static_cast<std::size_t>(i)]->text(); }
  static QAction* slot_action(MainWindow& w, int i) { return w.slot_actions_[static_cast<std::size_t>(i)]; }
  static void session_click(MainWindow& w, int i) { w.activate_slot({false, i}); }
  static std::optional<AafRecord> solar_slot(const MainWindow& w, int i) { return w.solar_slots_[static_cast<std::size_t>(i)]; }
  static bool dial_on(const MainWindow& w) { return w.dial_action_->isChecked(); }
  static bool multi_on(const MainWindow& w) { return w.multi_action_->isChecked(); }
  static bool harmonic_on(const MainWindow& w) { return w.harmonic_action_->isChecked(); }
  static double harmonic_order(const MainWindow& w) { return w.harm_n_; }
  static void preset_extras(MainWindow& w, bool real, bool hamburg, bool apogee) {
    w.preset_extras(real, hamburg, apogee);
  }
  static bool full_sheet(const MainWindow& w) { return w.full_sheet_; }
  static int active_slot(const MainWindow& w) { return w.active_slot_; }
  static bool& alt_rulers(MainWindow& w) { return w.alt_rulers_; }
  static QString summary(const MainWindow& w) { return w.aspects_label_->text(); }
  static QWidget* wheel_widget(MainWindow& w) { return w.wheel_; }
  static std::array<int, body::kSlotCount>& emphasis(MainWindow& w) { return w.emphasis_; }
  static WheelOptions wheel_options(const MainWindow& w) {
    return w.radix_wheel_options(*w.last_chart_, w.current_settings());
  }
  static void vorgaben_direktionen(MainWindow& w) { w.vorgaben_direktionen(); }
  static void secondary_direction(MainWindow& w) { w.secondary_direction(); }
  static void arc_direction(MainWindow& w) { w.arc_direction(); }
  static void symbolic_direction(MainWindow& w, bool equatorial) { w.symbolic_direction(equatorial); }
  static void primary_direction(MainWindow& w) { w.primary_direction(); }
  static bool mundane_frame(const MainWindow& w) { return w.mundane_frame_; }
  static void new_entry(MainWindow& w) { w.new_records_entry(); }
  static void delete_from_file(MainWindow& w) { w.delete_from_file(); }
  static void preview(MainWindow& w, const AafRecord& r) { w.preview_record(r); }
  static void clear_slots(MainWindow& w) { w.clear_slots(); }
  static void aaf_convert(MainWindow& w) { w.aaf_convert(); }
  static void vorgaben_ein_ausgabe(MainWindow& w) { w.vorgaben_ein_ausgabe(); }
  static std::filesystem::path konsta_file(const MainWindow& w) { return w.konsta_file_; }
  static AspectSettings& aspect_settings(MainWindow& w) { return w.aspect_settings_; }
  static int& outer_color(MainWindow& w) { return w.outer_color_; }
  static void persist_konsta(MainWindow& w) { w.persist_konsta(); }
  static void remember_double(MainWindow& w, int kind, const AafRecord& partner) { w.remember_double(kind, partner); }
  static QAction* double_action(MainWindow& w, int kind) { return w.double_actions_[static_cast<std::size_t>(kind)]; }
  static bool compare_on(const MainWindow& w) { return w.compare_action_->isChecked(); }
  static QString partner_name(const MainWindow& w) { return w.partner_name_; }
  static void save(MainWindow& w) { w.save_record(); }
  static void bind(MainWindow& w, const QString& path) { w.bind_data_file(path); }
  static QString data_file(const MainWindow& w) { return w.data_file_; }
  static std::optional<AafRecord> slot(const MainWindow& w, int i) { return w.slots_[static_cast<std::size_t>(i)]; }
  static void pick_zone(MainWindow& w) { w.pick_zone(); }
  static int claim_slot(MainWindow& w, int zmsp) { return w.claim_radix_slot(zmsp); }
  static void activate_slot(MainWindow& w, int i, const AafRecord& r) { w.set_slot(i, r, true); }
  static void clock_in_slot(MainWindow& w, int slot) {
    w.uhr_on_ = true;
    w.uhr_slot_ = slot;
  }
  static void data_file_io(MainWindow& w) { w.data_file_io(); }
  static void chain_files(MainWindow& w) { w.chain_files(); }
  static void clock_tick(MainWindow& w) { w.clock_tick(); }
  static void clock_gate_open(MainWindow& w) { w.clock_record_clock_.invalidate(); }
  static QAction* clock_action(MainWindow& w) { return w.clock_action_; }
  static void clear_clock(MainWindow& w) { w.clock_off(); }
  static std::size_t history_steps(const MainWindow& w) { return w.back_.size() + (w.pending_ ? 1U : 0U); }
  static void personar(MainWindow& w) { w.personar_chart(); }
  static int wart_item(const MainWindow& w) { return w.wart_item_; }
  static int active_solar(const MainWindow& w) { return w.active_is_solar_ ? w.active_solar_ : -1; }
  static DisplayList dynamo_arc_screen(const MainWindow& w, const Dynamogram& d, std::size_t first, std::size_t k, int lja) {
    return w.dynamo_arc_screen(d, first, k, lja);
  }
  // one GRAPHIK phase screen of the Rhythmenlehre over a given chart
  static DisplayList rhythm_screen(const MainWindow& w, const Chart& c, const RhythmOptions& o, int phase) {
    MainWindow::RhythmRun run;
    run.opt = o;
    run.clock.base_jd = c.jd_ut;
    run.direction = "LINKS ";
    run.chart = c;
    AspectSettings a = w.aspect_settings_;
    a.divisors = o.sextile ? 6 : 4;
    const AspectResult scan = scan_aspects(c, w.current_settings(), a);
    return w.rhythm_phase_screen(run, rhythm_triggers(c, scan, a, o), phase);
  }
};

}  // namespace horcom
