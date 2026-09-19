// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDate>
#include <QMainWindow>
#include <QTime>
#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/chart/harmonics.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/konsta.hpp"
#include "horcom/render/wheel.hpp"

class QAction;
class QCheckBox;
class QComboBox;
class QDate;
class QDateEdit;
class QDockWidget;
class QTime;
class QTimer;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
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

  /// Presets the input panel and switches for a scripted capture,
  /// the --chart flag of the capture hook.
  void preset_chart(const AafRecord& r, bool parallax, bool true_node);

  /// Switches the extra body rows for a scripted capture, the
  /// --extras flag of the capture hook.
  void preset_extras(bool real, bool hamburg, bool apogee);

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
  bool export_pdf_to(const QString& path, bool big = false);

  /// Writes the classic sheet as SVG with his sprites embedded, the
  /// file half of the SVG export dialog.
  ///
  /// @param path the target file
  /// @return true when the document was written
  bool export_svg_to(const QString& path);

  /// Opens the Planeten-Auswahl dialog, the capture hook's path into
  /// the extra body picker.
  void open_planet_selection() { planet_selection(); }

 private slots:
  void recompute();
  void data_file_io();
  void new_records_entry();
  void open_place();
  void save_place();
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
  void degree_date_list();
  void dynamogram_view();
  void linear_graph();
  void midpoint_tree();
  void planet_selection();
  void histogram_view();
  void correction();
  void time_wander();
  void place_wander();
  void chain_files();
  void create_statistics();
  void aaf_to_dat();
  void orb_settings();
  void converters();
  void planetar_chart();
  void personar_chart();
  void progression_chart();
  void day_chart();
  void transit_list();
  void ingress_table();
  void combin_chart();
  void save_record();
  void export_svg();
  void print_chart();
  void export_pdf();
  void about();

 private:
  /// One remembered step of the Eingabe panel, the Zurück and Vor
  /// buttons walk these.
  struct PanelState {
    QString given;
    QString surname;
    QString place;
    QDate date;
    QTime time;
    double zone = 0.0;
    /// the Sommerzeit shift, one hour east added to the base zone
    bool sommerzeit = false;
    double lon = 0.0;
    double lat = 0.0;
    int houses = 0;
    bool parallax = false;
    bool extras = false;
    bool hamburg = false;
    bool apogee = false;
    /// the Mondknoten row of the panel, DR and DS drop off when off
    bool node_show = true;
    bool true_node = false;
    bool true_apogee = false;
    bool helio = false;
    bool transit_on = false;
    QDate tdate;
    QTime ttime;
    AafRecord record;

    bool operator==(const PanelState& o) const {
      return given == o.given && surname == o.surname && place == o.place && date == o.date &&
             time == o.time && zone == o.zone && sommerzeit == o.sommerzeit &&
             lon == o.lon && lat == o.lat && houses == o.houses && parallax == o.parallax &&
             extras == o.extras && hamburg == o.hamburg && apogee == o.apogee &&
             node_show == o.node_show && true_node == o.true_node &&
             true_apogee == o.true_apogee && helio == o.helio && transit_on == o.transit_on &&
             tdate == o.tdate && ttime == o.ttime && record.surname == o.record.surname &&
             record.given == o.record.given && record.place == o.record.place &&
             record.comment == o.record.comment;
    }
  };

  void build_ui();
  void claim_wheel();
  [[nodiscard]] PanelState panel_state() const;
  void restore_state(const PanelState& s);
  void track_history();
  void flush_history();
  void history_back();
  void history_forward();
  void update_history_actions();
  void wander_dialog(bool place);
  void return_list(bool lunar);
  void show_wheel(DisplayList dl);
  [[nodiscard]] ClassicSheetText classic_sheet_text() const;
  [[nodiscard]] DisplayList classic_export_list() const;
  [[nodiscard]] DisplayList a4_export_list() const;
  int ask_print_format();
  void fill_tables(const Chart& chart, const AspectResult& aspects);
  [[nodiscard]] ChartInput current_input() const;
  [[nodiscard]] ChartSettings current_settings() const;
  void apply_record(const AafRecord& r, bool claim_slot = true);
  void apply_moment(double jd_ut, const QString& label, bool solar_slot = false);
  void run_solar(int year);
  void refresh_record_label();
  [[nodiscard]] SearchContext make_context() const;
  // the working Daten-Datei and its hub, ported from a2dat and a2fdat
  void bind_data_file(const QString& path);
  void refresh_data_file_label();
  [[nodiscard]] std::optional<std::vector<AafRecord>> load_collection(const QString& path) const;
  [[nodiscard]] std::vector<std::size_t> ask_order(const std::vector<AafRecord>& records, const QString& file_label);
  void fetch_from_file();
  void delete_from_file();
  void tidy_data_file(bool minimize);
  // the RADIX slots of his EIN-AUSG. menu, up to five records loaded
  void set_slot(int index, const AafRecord& r, bool activate);
  void update_slot_actions();
  [[nodiscard]] int next_slot() const;
  // the SOLAR...-DATEN slots, derived charts written back like his ^ items
  void store_solar(const QString& label);
  void update_solar_actions();
  // AUFRÄUMEN / RÜCKSETZEN and the plain HOROSKOP - GRAPHIK entry
  void reset_views();
  void clear_slots();
  // the VORGABEN EPHEMERIDE ÄNDERN chain and the HÄUSERSYSTEM box
  void vorgaben_ephemeride();
  void choose_house_system();
  // the F9 double print of mehrf_aus, two captured sheets on one page
  void double_print();
  // the Parameter - Einstellungen = VORGABEN overview of his main screen
  void vorgaben_overview();
  [[nodiscard]] std::optional<AafRecord> choose_record(const QString& title);
  [[nodiscard]] std::optional<AafRecord> choose_record_from_file(const QString& title);
  [[nodiscard]] ChartInput record_input(const AafRecord& r) const;
  [[nodiscard]] ChartRecord dat_from_record(const AafRecord& r) const;
  [[nodiscard]] AafRecord panel_record() const;
  /// @return the panel date, years before Christ ride the astronomical count
  [[nodiscard]] QDate panel_date() const;
  void set_panel_date(const QDate& d);
  bool set_partner(const AafRecord& r);
  QString record_label_;
  AafRecord record_;
  /// the bound working collection of DATEN-DATEI EIN-AUSGABE
  QString data_file_;
  int data_count_ = 0;
  QLabel* data_file_label_ = nullptr;
  /// the RADIX SATZ1 to SATZ5 slots, the active one drives the panel
  std::array<std::optional<AafRecord>, 5> slots_{};
  int active_slot_ = -1;
  std::array<QAction*, 5> slot_actions_{};
  /// the SOLAR...-DATEN slots, snapshots of the derived charts
  std::array<std::optional<AafRecord>, 5> solar_slots_{};
  std::array<QString, 5> solar_labels_{};
  std::array<QAction*, 5> solar_actions_{};
  /// which slot family holds the panel, false radix, true solar
  bool active_is_solar_ = false;
  int active_solar_ = -1;
  /// the slot the running clock feeds, his zeuhr, -1 when none
  int uhr_slot_ = -1;
  /// the two captured sheets of the F9 double print, his mehrf buffers
  std::vector<DisplayList> double_buffer_;
  /// the own ring colours of hor_farb, zero keeps the built in shade
  std::array<Rgb, 5> ring_colors_{};
  /// the outer symbol colour of the double wheels, his hard&
  int outer_color_ = 2;
  /// the capture hook switches the clock without the takeover question
  bool clock_scripted_ = false;

  VsopTables vsop_;
  Ephemerides eph_;
  std::filesystem::path data_dir_;
  Konsta konsta_;
  AspectSettings aspect_settings_;
  /// the user defined fixed point in radians, negative when off
  double fixpunkt_ = -1.0;
  /// the planet selection of the wheel, 0 normal, 1 red, -1 hidden
  std::array<int, body::kSlotCount> emphasis_{};
  /// which of the extra bodies flow into the calculation, driven by the
  /// Planeten-Auswahl dialog. The main planets and the angles ignore this
  /// switch, the Zusatz-Planeten and Andere Elemente rows carry it
  std::array<bool, body::kSlotCount> included_{};
  /// mark the birth ruler red like his inverse highlight
  bool ruler_red_ = false;
  /// which divisors draw chords, edited in the Planeten-Auswahl
  std::array<bool, 17> chords_{};
  bool chords_set_ = false;
  WheelWidget* wheel_ = nullptr;
  Banner* banner_ = nullptr;
  QLineEdit* given_ = nullptr;
  QLineEdit* surname_ = nullptr;
  /// the place-name field, the same textual Ortsname the record and
  /// the sheet carry, edits here land in record_.place
  QLineEdit* place_field_ = nullptr;
  /// the date as text like his TT MM JJJJ row, vC marks years BC
  QLineEdit* date_ = nullptr;
  /// the settle timer of the free text date field, a fresh date triggers
  /// a recompute a short pause after typing stops, so the user does not
  /// need to leave the field for the chart to catch up
  QTimer* date_timer_ = nullptr;
  QTimeEdit* time_ = nullptr;
  QDoubleSpinBox* zone_ = nullptr;
  /// the hidden double coordinate, the source of truth every calc and
  /// file path already reads through lon_/lat_->value()
  QDoubleSpinBox* lon_ = nullptr;
  QDoubleSpinBox* lat_ = nullptr;
  /// pulls the DMS boxes back in sync with the hidden double when a
  /// caller sets lon_/lat_ under a QSignalBlocker
  std::function<void()> refresh_lon_dms_;
  std::function<void()> refresh_lat_dms_;
  void sync_coord_boxes() {
    if (refresh_lon_dms_) refresh_lon_dms_();
    if (refresh_lat_dms_) refresh_lat_dms_();
  }
  /// refreshes the wirksame Zeit-Zone hint beside the Sommerzeit checkbox,
  /// callers that setChecked or setValue under a QSignalBlocker call it
  std::function<void()> refresh_sommer_effect_;
  QComboBox* houses_ = nullptr;
  QDockWidget* body_dock_ = nullptr;
  QDockWidget* cusp_dock_ = nullptr;
  bool dock_sized_ = false;
  QCheckBox* parallax_ = nullptr;
  /// the real extra bodies CH QU XE of his own final profile
  QCheckBox* extras_ = nullptr;
  /// the hypothetical factors of the Hamburg school, off the panel now,
  /// picked through the Planeten-Auswahl dialog like the tester's mockup
  QCheckBox* hamburg_ = nullptr;
  /// shows the Apogäum AG, the true Black Moon, on the wheel. The row
  /// pairs it with the wahr and mittel radio, the same Schwarzer Mond
  /// choice his family drives from the panel
  QCheckBox* apogee_show_ = nullptr;
  /// draws DR and DS at all, keeps the two nodes in the panel row where
  /// the wahr and mittel radio picks the formula
  QCheckBox* node_show_ = nullptr;
  QCheckBox* true_node_ = nullptr;
  QCheckBox* true_apogee_ = nullptr;
  /// Sommerzeit hebt die Ereigniszeit um eine Stunde, der Zone wird der
  /// Sommer-Zuschlag beim Rechnen aufaddiert wie in seiner ZONEN-Datei
  QCheckBox* sommerzeit_ = nullptr;
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
  /// the Summenspeicher of the primary directed axes rectification
  double vary_sum_ = 0.0;
  int vary_count_ = 0;
  std::optional<Chart> partner_chart_;
  ChartInput partner_input_;
  QString partner_name_;
  QString partner_place_;
  // the combin source memory, so the wheel can name both parents plus
  // the calculated mid moment as the tester wants. Empty means no combin
  QString combin_kind_;
  std::vector<std::string> combin_pair_lines_;
  // the panel history, every settled change one step, capped in depth
  std::vector<PanelState> back_;
  std::vector<PanelState> forward_;
  std::optional<PanelState> pending_;
  PanelState current_state_;
  bool state_init_ = false;
  bool restoring_ = false;
  QTimer* history_timer_ = nullptr;
  QAction* back_action_ = nullptr;
  QAction* forward_action_ = nullptr;
  QTableWidget* bodies_ = nullptr;
  QTableWidget* cusps_ = nullptr;
  QLabel* aspects_label_ = nullptr;
  std::optional<Chart> last_chart_;
  std::optional<AspectResult> last_aspects_;
};

}  // namespace horcom
