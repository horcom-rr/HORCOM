// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDate>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QPixmap>
#include <QTime>
#include <filesystem>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include "print_pages.hpp"
#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/chart/dynamogram.hpp"
#include "horcom/chart/harmonics.hpp"
#include "horcom/chart/rhythm.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/konsta.hpp"
#include "horcom/render/wheel.hpp"
#include "horcom/time/local_time.hpp"
#include "direction_list_dialog.hpp"

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
class QScrollArea;
class QLineEdit;
class QMenu;
class QContextMenuEvent;
class QTableWidget;
class QTimeEdit;

namespace horcom {

class Banner;
class WheelWidget;
struct DirectedAxes;

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

  /// Presets the house system for a scripted capture, the --houses flag.
  ///
  /// @param haw his hausw number, 1 Placidus to 9 none
  void preset_houses(int haw);

  /// Opens the comparison view over the given partner record, the
  /// capture hook's path into the Vergleich toggle.
  void show_compare(const AafRecord& partner);

  /// Opens the composite over the given partner record.
  void show_composite(const AafRecord& partner);

  /// Opens the directed axes for an event moment, the capture hook's
  /// path into the Direktionen toggle.
  void show_directions(double jd_event_ut, bool converse);

  /// Switches the mundane reference frame on, the horm 2 mode.
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

  /// Promotes the active derived chart into a fresh radix slot, the
  /// capture hook's path into ERGEBNIS als RADIX. Bypasses the operator
  /// warning so a scripted shot does not stall on a modal
  void promote_result_to_radix_scripted(int slot = -1);

 protected:
  void closeEvent(QCloseEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

 private:
  // the GUI tests reach the panel state through this probe
  friend struct MainWindowProbe;

 private slots:
  void recompute();
  void data_file_io();
  void new_records_entry();
  void vorgaben_ein_ausgabe();
  void open_place();
  void save_place();
  void pick_zone();
  void edit_record();
  /// STATISTIK G/H, his stat hub.
  void statistics_hub();
  void open_aspektarium();
  /// SOLAR, the return of the Sun in a calendar year at the event place,
  /// the SOLAR case of a16.
  void solar_chart();
  /// LUNAR, the return of the Moon by date or by number at the event
  /// place, the LUNAR case of a16.
  void lunar_chart();
  /// SEPTAR, the Solar that opens the Septar of a year of life, the
  /// SEPTAR case of a16.
  void septar_chart();
  /// TERRAR, the heliocentric SOLAR of a16.
  void terrar_chart();
  void degree_list();
  void fixed_star_table();
  void arabic_table();
  /// HÄUSER-TABELLE, his hausa with the hsa0 header.
  void house_table();
  /// GROßES ( = PLATONISCHES ) JAHR, his grossj with grossj1.
  void great_year();
  /// UHR, his running clock.
  void uhr();
  /// DESKTOP ( QUIT HORCOM ), his quit question and the end.
  void desktop_quit();
  /// His asp_halbs_zaehler box.
  ///
  /// @param statistik true offers the STATISTIK buttons
  /// @param whole     the fourth STATISTIK button, AUSWERTUNG des ZÄHLERS
  ///                  for the whole list or file
  /// @return 1 to 4 the answer, 0 when the session switched it off, -1 ESC
  [[nodiscard]] int counter_switch(bool statistik, const QString& whole = {});
  /// His halbs_zaehl_gr, the popup of the midpoint counts of one chart
  /// once they pass fifty, with the switch-off for the session.
  ///
  /// @param over    the window the popup opens over
  /// @param counts  direct, square, semi square and quarter square
  /// @param quarter the HALBSUMMEN-GRAPHIK adds the quarter squares
  [[nodiscard]] QString midpoint_count_line(const std::array<int, 4>& counts, bool quarter) const;
  /// His einzel_plan_wahl1 box before a chart output.
  ///
  /// @param preset the default row, 3 is NORMALE AUSGABE
  /// @return false after ABBRUCH
  [[nodiscard]] bool single_planet_choice(int preset, bool midpoints = false);
  /// His eingalp sheet of the walk keys.
  ///
  /// @param wait_ms     the wait between two steps
  /// @param seconds     the zeitwim legend of hours, minutes and seconds
  ///                    instead of days, hours and minutes
  /// @param progressive his p! legend of the symbolic years, months and
  ///                    days of the SEKUNDÄR and SONNEN-BOGEN rings
  void wander_help_sheet(int wait_ms, bool seconds = false, bool progressive = false);
  /// The Erste Hilfe list of F1 and the right mouse button.
  void erste_hilfe();
  /// The ERLÄUTERUNG of the menu the running output came from.
  void context_help();
  /// ÄNDERUNGEN / HINWEISE / KURZANL., his choice of the three texts.
  void anmerkungen();
  /// F2, the chart in between the outputs, his zeige_horoskop.
  ///
  /// @param item his muuu& number of the output the key came from
  void chart_between(int item);
  /// F3, the calculator of the system.
  void start_calculator();
  /// F6 and HELIOZENTRISCHE VERSION EIN/AUS with his message.
  void toggle_helio();
  /// F7, the window in front as a picture on the clipboard and in a file.
  void save_output_picture();
  /// F8, DRUCKER-OPTION EIN-AUS.
  void toggle_printer_option();
  /// @return true when the user answered his quit question with yes
  [[nodiscard]] bool confirm_quit();
  /// HINTERGRUND-FARBEN, the colours of the dialogs and the passive screen.
  void background_colors();
  /// His a2113_1, the next free RADIX slot or, all five filled, the
  /// number that gives way after his question.
  ///
  /// @param zmsp the slot index that gives way, negative for the
  ///        number of the active chart
  /// @return the slot index, -1 after Irrtum
  [[nodiscard]] int claim_radix_slot(int zmsp = -1);
  /// The five result lines of grossj1 for the active chart.
  ///
  /// @param outside receives the NICHT MEHR im ZEITALTER flag
  /// @return mes1$ through mes5$
  [[nodiscard]] QStringList great_year_lines(bool& outside) const;
  /// AUFGANG / UNTERGANG, his auf_unt screens.
  void rise_set();
  /// FINSTERNISSE, his finst screen of lunations and eclipses.
  void eclipse_table();
  /// PLANETEN-KOORDINATEN and ZUSATZ-PLANETEN-KOORDINATEN, his a91 and
  /// a10 screens with the ZEIT VARIIEREN walk of a9zw.
  ///
  /// @param extras true for the ZUSATZ table of every extra body
  void coordinate_table(bool extras);
  /// MÜNCHNER RHYTHMENLEHRE, his a17 chain from the AUSWERTE-MODUS on.
  void rhythm();
  /// GRAD-DATUM-LISTE, the a17 chain straight into the degree list.
  void degree_date_list();
  /// DYNAMOGRAMM, his huber with its question chain, the single arcs and
  /// the intervals of four years.
  void dynamogram_view();
  void linear_graph();
  void midpoint_tree();
  void planet_selection();
  void histogram_view();
  /// KORREKTUR, his korr, the birth time rectification.
  void correction();
  /// ZEIT-WANDERN, his zeitw with zeitwi.
  void time_wander();
  /// ORT-WANDERN, his ortwandern with ortwi.
  void place_wander();
  void chain_files();
  void aaf_convert();
  void import_aaf_file();
  void export_dat_file();
  /// VORGABEN HOROSKOP ÄNDERN, the avh wizard over his eleven topics.
  void vorgaben_horoskop();
  /// DOPPEL-KREIS / 90-GRAD-KREIS, his a12 with the MODUS box.
  void double_wheel_session();
  /// MULTIPLE DIREKTIONEN / HARMONICS, his multiple session.
  void multi_session();
  /// COMPOSIT, his a13 with the MODUS box, the two SATZ clicks and the
  /// residence of ROBERT HAND.
  void composite_session();
  // the converters of EPHEMERIDE and DIVERSES, one menu entry each
  void et_from_ut();
  void ut_from_et();
  void date_from_jd();
  void arde_from_eleb();
  void eleb_from_arde();
  /// LT aus UT or UT aus LT.
  ///
  /// @param from_ut true converts a Greenwich time into mean local time
  void local_time_convert(bool from_ut);
  /// PLANETARE, the return of a planet to its radix place, the PLANETARE
  /// case of a16.
  void planetar_chart();
  /// PERSONARE, the Sun over the radix place of a planet in the first
  /// year of life, the PERSONARE case of a16.
  void personar_chart();
  /// PROGRESSIONS-HOROSKOP, one day of the sky per year of life, his proho.
  void progression_chart();
  /// TAGES-HOROSKOP, the birth's true solar time on a chosen day, his taho.
  void day_chart();
  void transite();
  void transit_list();
  void mundane_aspects();
  void vorgaben_direktionen();
  void secondary_direction();
  /// the LINEAR-GRAPHIK with its kind preset, 1 secondary, 2 sun arc,
  /// 3 moon arc, 0 transits, -1 the plain entry
  void linear_graph_of(int preset_kind);
  void arc_direction();
  void symbolic_direction(bool equatorial);
  void primary_direction();
  void ingress_table();
  /// COMBIN, his a14 with two to five SATZ clicks.
  void combin_chart();
  void result_as_radix();
  /// @param index a RADIX slot
  /// @return its sol$, RADIX or the ALS RADIX label of a promoted result
  [[nodiscard]] QString radix_label(int index) const;
  void save_record();
  /// Writes the panel record into the working file like a22dat.
  ///
  /// @param from_entry true on the NEU-EINGABE path, a duplicate beside
  ///                   an AAF twin then only points to the AAF file
  /// @return true when the file took the record
  bool store_record(bool from_entry);
  void export_svg();
  void print_chart();
  void export_pdf();
  void about();

 private:
  // helpers of the entries above, kept out of the slot section so moc
  // registers no metatypes for them
  /// Names the ERLÄUTERUNG of every menu entry for F1 in the outputs.
  ///
  /// @param menus the top menus with the stem of their text
  void tag_help_stems(const std::vector<std::pair<QMenu*, QString>>& menus);
  /// @return the modal window in front, the main window when none is up
  [[nodiscard]] QWidget* front_window();
  [[nodiscard]] std::optional<QString> ask_file_stem();
  /// @return the moment the converter boxes open with
  [[nodiscard]] CalendarDate convert_start() const;
  /// @return the date of hi2 with the true obliquity of that day
  [[nodiscard]] std::optional<std::pair<CalendarDate, double>> convert_date();
  /// @param year the year of the converted moment
  /// @return the delta T remark of et_ut_erl
  [[nodiscard]] static QString et_ut_note(int year);
  /// The combin of two to five records, the mean moment and place into
  /// the panel as the chart of COMBIN-ORT.
  ///
  /// @param parts the records in the order of the clicks
  /// @param sets  their slot numbers for the SÄTZE: line
  void combin_of(const std::vector<AafRecord>& parts, const std::vector<int>& sets);
  /// @return the SOLAR result or the COMBIN on the panel with its sol$
  [[nodiscard]] std::optional<std::pair<AafRecord, QString>> promotable_result() const;
  /// @return true when a SOLAR result or a COMBIN drives the panel
  [[nodiscard]] bool can_promote_result() const;
  /// A RADIX or SOLAR slot, the target of his SATZ clicks.
  struct SlotChoice {
    bool solar = false;
    int index = -1;
  };
  /// Asks for one filled slot like the menu click his flows wait for.
  ///
  /// @param title  the box title
  /// @param info   the lines above the slot buttons
  /// @param marked the slots picked already, his * before their sol$
  /// @return the slot, nothing after ABBRUCH
  [[nodiscard]] std::optional<SlotChoice> pick_slot(const QString& title, const QStringList& info,
                                                    const std::vector<SlotChoice>& marked = {}) const;
  /// Makes a slot the active chart, the SATZ click inside the paired
  /// sessions, where his direkt went on into a12, a13 or a14.
  void activate_slot(const SlotChoice& c);
  /// The menu click on a SATZ row, his a4 or a5 with direkt and eingabe,
  /// the slot becomes the chart and its EINGABE- und ANZEIGE-BOX opens.
  ///
  /// @param c the clicked slot
  void open_slot(const SlotChoice& c);
  /// His a12 sheet around the given double wheel, both coordinate
  /// columns, the comparison grid and the corners of both charts.
  [[nodiscard]] DisplayList a12_sheet(const Chart& inner, const Chart& outer, const ChartSettings& s, bool dial,
                                      DisplayList wheel) const;
  /// Shows a finished sheet of the original as it is, no centring.
  void show_full_sheet(DisplayList dl);
  /// His wart, the screen stays until a key or a mouse button.
  ///
  /// @param over the waiting screen, it carries the bar that names the key
  /// @return the key, zero for a mouse button
  /// @note The menu greys while the screen waits.
  int wait_key(QWidget* over);
  /// His haus_ber box, true for NEU RECHNEN, nothing after ABBRUCH.
  [[nodiscard]] std::optional<bool> ask_house_mode();
  /// The BEZUGS-FAKTOR of MULTI 3 and MULTI-ARC, nothing after ESC.
  [[nodiscard]] std::optional<MultiReference> ask_multi_reference(const QString& mul);
  /// His mul$ of a MULTI mode.
  ///
  /// @param mode the mode
  /// @return MULTI 1 to MULTI-ARC
  [[nodiscard]] static QString multi_name(MultiMode mode);
  /// His zeitwim, the birth time walks while the MULTI sheet follows,
  /// false after ABBRUCH.
  bool multi_time_variation(const QString& mul);
  /// The full sheet of multi11 or harm21 around the given double wheel.
  [[nodiscard]] DisplayList multi_sheet(const Chart& radix, const Chart& multi, const ChartSettings& s, bool harmonic,
                                        const QString& mul, double lja, double event_jd, DisplayList wheel) const;
  /// The running sums of his counter windows, asp% with aspz% and the
  /// halbsz% of the midpoints, charts is his halbszaus% and aspzaus%.
  struct WanderCounts {
    long charts = 0;
    std::array<long, 17> asp_now{};
    std::array<long, 17> asp_sum{};
    long troika_now = 0;
    long troika_sum = 0;
    long grand_trine_now = 0;
    long grand_trine_sum = 0;
    std::array<long, 4> mid_now{};
    std::array<long, 4> mid_sum{};
  };
  /// The nearest place of WCAPITAL.INT with its distance.
  struct NearestCapital {
    double lon = 0.0;
    double lat = 0.0;
    QString name;
    double east_km = 0.0;   // the walked place east of it, negative west
    double north_km = 0.0;  // the walked place north of it, negative south
  };
  /// His auswert_ortelist.
  ///
  /// @param lon east longitude in degrees
  /// @param lat north latitude in degrees
  /// @return the nearest capital, nothing without the file
  [[nodiscard]] std::optional<NearestCapital> nearest_capital(double lon, double lat) const;
  /// His ortgalp sheet of the walk keys.
  ///
  /// @param wait_ms the wait between two steps
  void place_help_sheet(int wait_ms);
  /// The classic sheet of a record at a moment and place.
  ///
  /// @param r  the record whose name and place the sheet prints
  /// @param in the moment and place
  /// @param s  the settings
  /// @return the display list, nothing when the chart failed
  [[nodiscard]] std::optional<DisplayList> sheet_wheel(const AafRecord& r, const ChartInput& in, const ChartSettings& s,
                                                     const std::string& centre = {}) const;
  /// @return the running moment of the clock in UT
  [[nodiscard]] static CalendarDate clock_now();
  /// @return the clock place at the moment of now, his uhr_par
  [[nodiscard]] ChartInput clock_input() const;
  /// His uhr_kon_l, the clock forgets place and record.
  void clock_off();
  /// His gouhr$ with gluhr and gguhr, the place of the clock from a
  /// record, the clock itself reads UT.
  ///
  /// @param r the record whose place the clock takes
  void set_clock_place(const AafRecord& r);
  /// The record of the clock slot at a moment, his a2113 under uhr.
  ///
  /// @param d the moment in UT
  /// @return the clock place with that date and whole seconds
  [[nodiscard]] AafRecord clock_record(const CalendarDate& d) const;
  /// The men2 caption of UHR, grey while a clock record ticks in its slot.
  void clock_menu_update();
  /// His acmcl strip of UHR, AC, MC and STZ.
  void clock_strip_update();
  /// The one second tick of the strip and the clock record.
  void clock_tick();
  /// His single arc screen, the arcs of a pass up to k with the captions
  /// of k.
  ///
  /// @param d     the run
  /// @param first the first arc of the pass
  /// @param k     the last arc shown
  /// @param lja   the first year of life of the window
  /// @return the screen, empty when the pass holds no arc up to k
  [[nodiscard]] DisplayList dynamo_arc_screen(const Dynamogram& d, std::size_t first, std::size_t k, int lja) const;
  /// One EINZEL - BÖGEN screen, the arcs of one pass drawn one after the
  /// other, two beeps and his wart at the end.
  ///
  /// @param d     the run
  /// @param first the first arc of the pass
  /// @param end   one past the last arc of the pass
  /// @param lja   the first year of life of the window
  void dynamo_arc_pass(const Dynamogram& d, std::size_t first, std::size_t end, int lja);
  /// His hubausg graph.
  ///
  /// @param d     the run
  /// @param lja   the first year of life of the window
  /// @param radix the birth chart of the run
  /// @return the screen
  [[nodiscard]] DisplayList dynamo_graph(const Dynamogram& d, int lja, const Chart& radix) const;
  /// The rows of his two yellow counter windows.
  ///
  /// @param c         the sums
  /// @param midpoints receives the HALBSUMMEN - ZÄHLER rows
  /// @param aspects   receives the ASPEKTE - ZÄHLER rows
  /// @param records   STATISTIK counts DATENSÄTZE, ZEIT-WANDERN VORGÄNGE
  void counter_texts(const WanderCounts& c, QString& midpoints, QString& aspects, bool records = false) const;
  /// Adds one chart to the counter sums, asp% and aspz%, the Schiemenz
  /// counters and the midpoints of halbs1.
  ///
  /// @param c       the sums
  /// @param chart   the chart
  /// @param aspects its scan
  /// @param s       its settings
  void count_chart(WanderCounts& c, const Chart& chart, const AspectResult& aspects, const ChartSettings& s) const;
  // the STATISTIK flows of main_window_statist.cpp and its _cond and _list
  // parts, the session types in main_window_statist.hpp
  struct StatSession;
  struct StatPick;
  /// The VORGABEN STATISTIK row of stat, NEUE DATENSÄTZE UPDATEN and the
  /// size of the list.
  void statistics_defaults();
  /// His stat1, a dataset of a .DAT computed into STATIST7.
  void create_statistics();
  /// His stat3, a dataset deleted with its twins.
  void delete_statistics();
  /// His stat2, the evaluation of a dataset.
  void run_statistics();
  /// His ausw_pl_hs with pl_h, one factor of a condition.
  ///
  /// @param ss      the session, its dataset names the extras
  /// @param groups  object 1 offers SO / MO / AC and ALLE PLANETEN
  /// @param notes   the lines above the list
  /// @param out     receives the pick
  /// @param running the condition his numw printed over the HAUS NR.
  ///                digits, empty for none
  /// @return false after ESC
  bool stat_pick(const StatSession& ss, bool groups, const QStringList& notes, StatPick& out,
                 const QString& running = {});
  /// obj_wahl, such_wo and the inputs of stat_ausw, then the run.
  ///
  /// @param ss the session
  /// @return false after ABBRUCH
  bool stat_condition(StatSession& ss);
  /// od_un, the join of the next condition.
  ///
  /// @param ss the session
  /// @return 1 to 3 the join, 4 the list, -1 after ABBRUCH
  int stat_join(StatSession& ss);
  /// list_ausg, the pages of the list.
  ///
  /// @param ss the session
  /// @return 0 the end, 1 a new evaluation of the file, 2 a further
  ///         condition
  int stat_list(StatSession& ss);
  /// kotab_sta, the box of a clicked row.
  ///
  /// @param ss     the session
  /// @param record the record index into the dataset
  /// @return false after ABBRUCH
  bool stat_row(StatSession& ss, int record);
  /// The chart of a dataset record with the parameters of its .PAR, his
  /// a11_1 after stat2parl, shown until a key.
  ///
  /// @param ss       the session
  /// @param record   the record index into the dataset
  /// @param counters show the yellow counter windows with the sums so far
  void stat_view(const StatSession& ss, int record, bool counters);
  /// Adds one dataset chart to the counters of the session, the counting
  /// of a11_1 under zaehl_asp_halbs& 2 and 4.
  ///
  /// @param ss     the session
  /// @param record the record index into the dataset
  void stat_count(StatSession& ss, int record) const;
  /// NEUE DATENSÄTZE UPDATEN of a22dat, the saved record into the
  /// dataset of its file.
  ///
  /// @param dat   the working file
  /// @param entry the saved record
  void stat_update_record(const std::filesystem::path& dat, const ChartRecord& entry);
  /// His halbs_ruecksetz question.
  ///
  /// @param counts the sums, zeroed on JA
  void reset_counters_question(WanderCounts& counts);
  /// @return his horgt$, the frame of the positions
  [[nodiscard]] QString horgt_text() const;
  /// @return his gena2$, the ephemeris mode of the tables
  [[nodiscard]] QString gena2_text() const;
  /// His gena4$, the ephemeris mode without the heading.
  ///
  /// @param appa his appa& from 1 to 3, zero reads the profile
  /// @return the mode and the parallax, "App.1,Mit Parallaxe" and the like
  [[nodiscard]] QString gena4_text(int appa = 0) const;
  /// The NEU DEFINIEREN branch of arabt, own points until NEIN.
  ///
  /// @param dir the folder of the ARABTEI files
  /// @return 0 back to the formula question, 1 on to the table, -1 ESC
  [[nodiscard]] int arabic_define(const std::filesystem::path& dir);
  /// The rows of ko_ta for one coordinate table.
  ///
  /// @param table  the table, twelve columns or ten for ZUSATZ
  /// @param chart  the chart of the table
  /// @param s      its settings
  /// @param extras true for the ZUSATZ rows
  void fill_coordinate_table(QTableWidget* table, const Chart& chart, const ChartSettings& s, bool extras) const;
  /// The answers of one Rhythmenlehre run.
  struct RhythmRun {
    /// his dbr&, 1 the graph, 2 the table, 3 the degree list
    int mode = 1;
    /// the walk options the answers set
    RhythmOptions opt;
    /// where the ages of the walk stand in time, his a174init
    RhythmClock clock;
    /// ausgd!, dates instead of LJ and MO
    bool dated = true;
    /// his richt$, RECHTS or LINKS
    QString direction;
    /// the chart of the walk, the Sonderpunkt on slot zero
    Chart chart;
  };
  /// The a17 chain for one AUSWERTE-MODUS.
  ///
  /// @param preset his dbr& to start with, zero asks the AUSWERTE-MODUS
  void rhythm_run(int preset);
  /// @return the a17sol lines his screen showed above every box
  [[nodiscard]] QStringList rhythm_sol_lines() const;
  /// @return the label of the chart on screen, his sol$(od,ze)
  [[nodiscard]] QString rhythm_chart_label() const;
  /// His a17eing11, ZEIT-EINHEIT MARKIEREN!
  ///
  /// @param septar a Septar opens with MONAT
  /// @param check  ask PARAMETER RICHTIG ? when the unit changed
  /// @return false after ESC
  [[nodiscard]] bool rhythm_unit_question(bool septar, bool check);
  /// His a17eing12, Periode PRO HAUS ?
  ///
  /// @param check ask PARAMETER RICHTIG ? when the period changed
  /// @return false after ABBRUCH
  [[nodiscard]] bool rhythm_period_question(bool check);
  /// @return the walk options of the answers of the session
  [[nodiscard]] RhythmOptions rhythm_options() const;
  /// His a174init, where the ages of the walk stand in time.
  ///
  /// @param opt the walk options, the Septar offset reads them
  /// @return the clock of the chart on screen
  [[nodiscard]] RhythmClock rhythm_clock(const RhythmOptions& opt) const;
  /// His a17sonderpkt, the SONDERPUNKT box.
  ///
  /// @param septar the Septar variant without the date definition
  /// @param opt    the walk options, the special point lands in them
  /// @param clock  the clock of the walk for the date definition
  /// @return false after ABBRUCH
  [[nodiscard]] bool rhythm_special_question(bool septar, RhythmOptions& opt, const RhythmClock& clock);
  /// His a17eing CASE 4, a self defined degree for the list.
  void rhythm_define_degree();
  /// His a17eing CASE 5, all self defined degrees gone.
  void rhythm_delete_degrees();
  /// The GRAPHIK ( HOROSKOP ) screens, one per phase.
  void rhythm_graph(const RhythmRun& run);
  /// The AUSLÖSUNGS-TABELLE.
  void rhythm_table(const RhythmRun& run);
  /// The GRAD-DATUM-LISTE.
  void rhythm_degree_list(const RhythmRun& run);
  /// The phase screen of the graph.
  ///
  /// @param run  the answers
  /// @param rows the triggers of the whole walk
  /// @param phase the walk step, one to twelve
  /// @return the sheet with the wheel and the strip
  [[nodiscard]] DisplayList rhythm_phase_screen(const RhythmRun& run, const std::vector<RhythmTrigger>& rows,
                                                int phase) const;
  /// @return the a18kopf lines of the table and the list
  [[nodiscard]] QStringList rhythm_heading(const RhythmRun& run) const;
  /// One remembered step of the Eingabe panel, the Zurück and Vor
  /// buttons walk these.
  struct PanelState {
    QString given;
    QString surname;
    QString place;
    /// the date field as typed, read in the panel calendar
    QString date;
    QTime time;
    double zone = 0.0;
    /// the summer time shift in hours, his somz, DSZ one and DDSZ two
    double dst = 0.0;
    /// the calendar rule of the date field, his jul$
    Calendar calendar = Calendar::kAuto;
    /// which clock the time field shows, zone time or a local time
    ClockKind clock = ClockKind::kZone;
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
    /// the SATZ slot the step belonged to, -1 without one, so Zurück and
    /// Vor bring the slot back with its person
    int slot = -1;
    /// the slot was a SOLAR one
    bool solar = false;

    bool operator==(const PanelState& o) const {
      return given == o.given && surname == o.surname && place == o.place && date == o.date &&
             time == o.time && zone == o.zone && dst == o.dst && calendar == o.calendar && clock == o.clock &&
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
  /// Writes the panel into the active slot, a settled step is the live
  /// data of the slot like his eingabe wrote into the SATZ arrays.
  void store_active_slot();
  void history_back();
  void history_forward();
  void update_history_actions();
  /// The place of an event, the answer of his ort_wahl.
  struct EventPlace {
    double lon = 0.0;
    double lat = 0.0;
    std::string name;
  };
  /// Asks EREIGNIS-Ort = GEBURTS-Ort ? like ort_wahl, nothing on ABBRUCH.
  ///
  /// @param notes the lines his screen showed above the box, the parallax
  ///              hints of ereig_ort, empty for the plain ort_wahl
  /// @return the event place, nothing on ABBRUCH
  [[nodiscard]] std::optional<EventPlace> ask_event_place(const QStringList& notes = {});
  /// @return the place of the radix, the JA answer of his ort_wahl box
  [[nodiscard]] EventPlace birth_place() const;
  /// The Ort row of the SOLAR, LUNAR and PLANETARE dialogs, the event
  /// place with a button that asks his ort_wahl box again.
  ///
  /// @param dialog  the dialog the row lives in
  /// @param place   the event place the box rewrites, it outlives the dialog
  /// @param changed runs after the box answered with a place
  /// @return the row for the layout of the dialog
  [[nodiscard]] QLayout* event_place_row(QWidget* dialog, EventPlace* place, std::function<void()> changed = {});
  /// Moves the panel place to an event place, the record keeps its own.
  void set_panel_place(const EventPlace& p);
  /// @return the birth moment and place the derived charts build on
  [[nodiscard]] ChartInput radix_input() const;
  /// @return the birth chart, the record's own, not the one on the wheel
  [[nodiscard]] Chart radix_chart() const;
  /// Which run asks the a18eing boxes, his tras!, mund!, prog!, sobg!,
  /// mob!, syb! with ars!, mars! and prim! flags.
  enum class A18Mode { kTransits, kMundane, kSecondary, kSunArc, kMoonArc, kSymbolic, kSymbolicMundane, kPrimary };
  /// The answers of his a18eing boxes.
  struct A18Answers {
    /// the run that asked, his a18kopf heading reads it
    A18Mode mode = A18Mode::kTransits;
    double base_deg = kDefaultBaseAngleDeg;
    AspectGrid grid = AspectGrid::kPlain;
    /// his stund_ast, only aspects before a body leaves its sign
    bool within_sign = false;
    /// his pl1, 5 from Mars, 6 from Jupiter
    int first_slot = 1;
    /// NUR DIESE DARSTELLEN
    std::vector<int> chosen;
    /// ROT MARKIEREN
    std::vector<int> marked;
    /// MOND BERÜCKSICHTIGEN
    bool moon = false;
    /// his delblm, the true black moon stays out
    bool drop_true_apogee = false;
    /// the date window of TRANSITE and MUNDAN-ASPEKTE
    double jd_from = 0.0;
    double jd_to = 0.0;
    /// FORMAT der AUSGABE of the directions
    DirectionFormat format = DirectionFormat::kDate;
    /// SCHLÜSSEL-ZAHL, years per degree of arc
    double key = 1.0;
    /// ZÄHLUNG der LEBENSJAHRE, his lja and lje
    double from_years = 0.0;
    double to_years = 0.0;
  };
  /// Asks the a18eing boxes of the table branch in his order.
  ///
  /// @param mode   the run that asks
  /// @param linear his lin!, the LINEAR-GRAPHIK branch with its windows,
  ///               its greyed rows and the boxes it skips
  /// @return the answers, nothing after ABBRUCH
  [[nodiscard]] std::optional<A18Answers> a18_questions(A18Mode mode, bool linear = false);
  /// His a18eing_plw, LAUFENDE FAKTOREN AUSWÄHLEN ? or PLANETEN
  /// AUSWÄHLEN ? for the arcs.
  ///
  /// @param a      the answers, the run is read from them, first_slot,
  ///               chosen, marked and moon land in them
  /// @param linear his lin!, ROT MARKIEREN stands blank and grey
  /// @return false after ABBRUCH
  [[nodiscard]] bool a18_factor_box(A18Answers& a, bool linear);
  /// The HOROSKOP-GRAPHIK branch of a18asw, his a20_horg with the zeitwi
  /// walk, for the transits, the secondary and the sun arc direction.
  ///
  /// @param mode kTransits, kSecondary or kSunArc
  void wheel_run(A18Mode mode);
  /// His zeitwi under drgrph!, the running ring steps while the radix
  /// stays, ZEIT-EINHEIT and RICHTUNG first, then the stepping screen.
  ///
  /// @param title       the window title of the stepping screen
  /// @param progressive his p!, the symbolic units of the progressed rings,
  ///                    else the TAGE, STUNDEN and MINUTEN of the transits
  /// @param tja         the tropical year of the radix, his symbolic day
  /// @param jd          the moment of the ring, the transit moment or the
  ///                    progressed one, it walks
  /// @param show        puts the ring of a moment on the main wheel
  void ring_walk(const QString& title, bool progressive, double tja, double& jd,
                 const std::function<void(double)>& show);
  /// The running factors of a wheel_run, his pl1& and the plw& of
  /// plan_wahl! and plan_col!.
  struct RunningRing {
    int first_slot = 1;
    std::vector<int> chosen;
    std::vector<int> marked;
  };
  /// Applies the running factors to the outer ring, auswahl_flag hides the
  /// bodies before pl1& and those not chosen, the marked ones turn red.
  ///
  /// @param ring the running chart, bodies lose their present flag
  /// @param opt  the wheel options, the marks land in outer_marked
  void trim_running_ring(Chart& ring, WheelOptions& opt) const;
  /// Draws the ring of wheel_run over the radix for the SEKUNDÄR and
  /// SONNEN-BOGEN runs with his zeitwi texts in the centre.
  ///
  /// @param radix   the chart of the panel
  /// @param in      its moment and place, the progressed sky stands there
  /// @param s       the settings
  /// @param aspects the radix scan
  /// @param wopt    the wheel dress of the radix
  /// @return true when the ring was drawn
  bool show_ring(const Chart& radix, const ChartInput& in, const ChartSettings& s, const AspectResult& aspects,
                 const WheelOptions& wopt);
  /// The screen of a20_horg and zeitwi under drgrph!, the running ring
  /// over the radix with the bes11 columns of both charts, the a12asp
  /// grid of the running bodies against the radix and the corners of
  /// bes1 and bes111.
  ///
  /// @param radix the standing chart, the inner wheel and the left column
  /// @param ring  the running chart, the outer ring and the right column
  /// @param s     the settings of both
  /// @param mode  kTransits, kSecondary or kSunArc, the head of the right
  ///              column and the letters of the grid
  /// @param place the Ereig.-Ort of bes111, the event place of a transit
  ///              or the radix place of the progressed rings
  /// @param wheel the transit wheel with its centre texts, in the classic
  ///              sheet coordinates
  /// @return the full sheet
  [[nodiscard]] DisplayList a20_sheet(const Chart& radix, const Chart& ring, const ChartSettings& s, A18Mode mode,
                                      const EventPlace& place, DisplayList wheel) const;
  /// the TRANSITE walk of a20_horg shows his full sheet until the transit
  /// view is left, the Transite switch of the panel keeps its own view
  bool a20_transits_ = false;
  /// His avd_lin, the orientation lines and the signs of the linear graph.
  ///
  /// @return false after EXIT or ESC
  [[nodiscard]] bool linear_orientation();
  /// The LINEAR-GRAPHIK branch of a18asw for one run, and the one of mund.
  ///
  /// @param mode the transits, the mundane aspects, the secondary or the
  ///             arc directions
  void linear_run(A18Mode mode);
  /// His CLR hrg! at the start of the direction runs, the helio switch
  /// of the panel goes off.
  void force_geocentric();
  /// His a18asw1, the next or previous window of the linear graph.
  ///
  /// @param a       the answers, their window moves
  /// @param forward NÄCHSTES or VORHERGEHENDES
  /// @param cal     the calendar of the dates
  static void step_linear_window(A18Answers& a, bool forward, Calendar cal);
  /// @return the four hsa0 header lines above his tables, record,
  ///         moment, sidereal time and place with the moon phase
  [[nodiscard]] QStringList hsa0_lines(const Chart& chart) const;
  /// The a18kopf header lines above a result table.
  ///
  /// @param title       his di$, the name of the run
  /// @param a           the answers, grid, window and key
  /// @param with_record true adds the name, place and moment lines
  /// @return the lines, the Grund-Aspekt line first
  [[nodiscard]] QStringList a18_heading(const QString& title, const A18Answers& a, bool with_record) const;
  /// His a18asw box, TABELLE, HOROSKOP-GRAPHIK or LINEAR-GRAPHIK.
  ///
  /// @param moon_arc true greys HOROSKOP-GRAPHIK like his mob! run
  /// @return 0 table, 1 wheel, 2 linear graph, -1 after ABBRUCH
  int ask_output_mode(bool moon_arc = false);
  /// Asks EREIGNIS-Ort like ereig_ort when the parallax is on.
  ///
  /// @param ctx     the context whose place follows the answer
  /// @param footer  receives the Ereignis-Ort line of a18tab, empty when
  ///                no place was asked
  /// @param mundane his mund!, the one line hint of the mundane aspects
  ///                instead of the PARALLAXE lines of the other runs
  /// @return false after ABBRUCH
  bool event_place_for(SearchContext& ctx, QString& footer, bool mundane = false);
  /// The TransitScan the a18eing answers and VORGABEN DIREKTIONEN make.
  ///
  /// @param a the answers
  /// @return the scan with window, grid, choices and extra targets
  [[nodiscard]] TransitScan a18_scan(const A18Answers& a) const;
  /// Shows a direction result table and its sort question.
  ///
  /// @param hits   the arcs of the symbolic or primary walk
  /// @param a      the answers, format, marks and window
  /// @param title  his di$ for the heading
  /// @param footer the latitude note of the primary, may stay empty
  /// @param item   his muuu& number of the direction entry
  void show_direction_list(std::vector<DirectionHit> hits, const A18Answers& a, const QString& title,
                           const QString& footer, int item);
  /// Switches the horm 2 frame, the BEZUGS-SYSTEM of VORGABEN HOROSKOP.
  void set_mundane_frame(bool on);
  /// what a page of the avh wizard answers, on, back one page, or EXIT
  enum class WizardStep { kNext, kBack, kExit };
  // the pages of avh, the orb topic with orbis_asp and orbis_pla, the
  // points of punkte_pla, the colours with hor_farb and the aspects with
  // the line style screen
  WizardStep orb_topic();
  bool orb_table_dialog();
  WizardStep orb_weight_dialog();
  WizardStep points_topic();
  WizardStep colour_topic();
  WizardStep ring_colour_dialog();
  WizardStep aspect_topic();
  WizardStep line_style_screen();
  /// Opens the transit view of a moment picked from a list.
  ///
  /// @param jd_ut the moment
  /// @param place the event place of the running sky, the birth place
  ///              when empty
  void show_moment_transits(double jd_ut, std::optional<EventPlace> place = std::nullopt);
  // the EREIGNIS-Ort of the transit view, his goe$ of a20_horg
  std::optional<EventPlace> transit_place_;
  /// KORREKTUR option 7, the PRIMÄR DIRIGIERTE ACHSEN session of prima.
  void primary_axes_session();
  /// The wheel of the directed axes with the texts of primhorg, the turn,
  /// the event moment, the direction, the directed sidereal time and the
  /// sums of the variations.
  ///
  /// @param dl the wheel of the directed axes
  /// @param d  the directed axes it was drawn from
  void show_directed_axes(DisplayList dl, const DirectedAxes& d);
  /// His plre notice, PLACIDUS - HÄUSER ERFORDERLICH for the house
  /// systems two to seven.
  void placidus_notice();
  /// Opens the frame of a KORREKTUR session like korr and prima, Placidus
  /// houses after plre, geocentric positions like his CLR hrg! and the
  /// panel history held so the whole session makes one step.
  ///
  /// @param always_placidus prima's haw& = 1 for every house system
  /// @return the closing step for a qScopeGuard, it hands back the house
  ///         system like korrend with haw_merk& and records the net change
  [[nodiscard]] std::function<void()> korrektur_frame(bool always_placidus);
  /// Holds the panel history while a walk or a session steps the panel,
  /// so neither the Zurück list nor the active slot follows every step.
  ///
  /// @return the release for a qScopeGuard, the tracking resumes and the
  ///         net change of the run becomes one step on its next recompute
  [[nodiscard]] std::function<void()> hold_history();
  /// SOLAR-LISTE and LUNAR-LISTE, up to 84 returns in three columns, his
  /// sol_lun_tabelle, the TERRAR list under hrg!.
  ///
  /// @param lunar true lists the lunars from a start date, false the
  ///              solars from a first year
  void return_list(bool lunar);
  void show_wheel(DisplayList dl);
  [[nodiscard]] ClassicSheetText classic_sheet_text() const;
  /// The record corners of the sheet for any record and chart.
  ///
  /// @param r              the record naming the sheet
  /// @param in             the moment and place drawn
  /// @param chart          the computed chart, nullptr leaves the STZ out
  /// @param s              the settings of the drawing
  /// @param shown_calendar the calendar the date line reads in
  /// @return the corner texts
  [[nodiscard]] ClassicSheetText sheet_text_for(const AafRecord& r, const ChartInput& in, const Chart* chart,
                                                const ChartSettings& s, Calendar shown_calendar) const;
  /// NUR HOROSKOP ZEIGEN of the record chooser, the chart of a record in
  /// a window of its own without taking it into a slot.
  ///
  /// @param r the record to look at
  void preview_record(const AafRecord& r);
  /// Writes the Vorgaben like kon_dsp, after every settings change and at
  /// the end of the session.
  void persist_konsta();
  [[nodiscard]] DisplayList classic_export_list() const;
  [[nodiscard]] DisplayList a4_export_list() const;
  int ask_print_format();
  /// Fills the body and cusp panels.
  ///
  /// @param chart the shown chart
  /// @param aspects its scan
  /// @param longitudes_only the composite of bes10, longitudes without
  ///        latitude, speed or distance
  void fill_tables(const Chart& chart, const AspectResult& aspects, bool longitudes_only = false);
  /// @return the HALBSUM. list of the KOMPAKT-AUSWERTUNG, empty without it
  [[nodiscard]] QString compact_midpoints(const Chart& chart) const;
  [[nodiscard]] ChartInput current_input() const;
  [[nodiscard]] ChartSettings current_settings() const;
  void apply_record(const AafRecord& r, bool claim_slot = true);
  /// Puts a found moment onto the panel. Derived charts land in UT, a
  /// corrected radix keeps the panel's own clock, zone and calendar.
  void apply_moment(double jd_ut, const QString& label, bool solar_slot = false, bool keep_clock = false);
  /// Casts the SOLAR of a year, one pass of his a16sol loop.
  ///
  /// @param year  the calendar year of the return
  /// @param place the event place, the search and the chart stand there
  void run_solar(int year, const EventPlace& place);
  void refresh_record_label();
  [[nodiscard]] SearchContext make_context() const;
  /// @return the emphasis of the wheel, the Planeten-Auswahl with the
  ///         Mondknoten switch on top, -1 hides a body
  [[nodiscard]] std::array<int, body::kSlotCount> shown_emphasis() const;
  /// @return the orb settings with every hidden body silenced, his
  ///         asp_wahl, NUR AUSGEWÄHLTE Planeten und DEREN ASPEKTE
  [[nodiscard]] AspectSettings shown_aspect_settings() const;
  /// The wheel dress of a radix.
  ///
  /// @param chart the chart the wheel shows
  /// @param s     its settings
  /// @param frame false keeps the ecliptic frame whatever horm& says, the
  ///              MULTI world forces horm& = 1
  /// @return colours, hidden bodies, the fixed point and the inverted
  ///         birth rulers of geb_herr
  [[nodiscard]] WheelOptions radix_wheel_options(const Chart& chart, const ChartSettings& s, bool frame = true) const;
  /// The radix the MULTI and HARMONICS sheets direct. His multiple and
  /// harm clear hrg!, moknw!, apogw! and kard! and set horm& = 1 for the
  /// session.
  ///
  /// @param in the panel input
  /// @param s  the panel settings
  /// @return the geocentric radix with the mean node and apogee, the
  ///         fixed point of fixpunkt_def_mult on slot zero, and the
  ///         settings it was computed with
  [[nodiscard]] std::pair<Chart, ChartSettings> multi_radix(const ChartInput& in, const ChartSettings& s) const;
  /// @return the element and quality counts with his pn weights and
  ///         the doubling switches of punkte_pla
  [[nodiscard]] Histogram sheet_histogram(const Chart& chart, const ChartSettings& s) const;
  // the working Daten-Datei and its hub, ported from a2dat and a2fdat
  void bind_data_file(const QString& path);
  void refresh_data_file_label();
  [[nodiscard]] std::optional<std::vector<AafRecord>> load_collection(const QString& path) const;
  [[nodiscard]] std::vector<std::size_t> ask_order(const std::vector<AafRecord>& records, const QString& file_label);
  void fetch_from_file();
  void delete_from_file();
  void tidy_data_file(bool minimize);
  // the RADIX slots of his EIN-AUSG. menu, up to five records loaded
  /// Writes a RADIX slot.
  ///
  /// @param index    the slot, 0 to 4
  /// @param r        the record
  /// @param activate true makes it the chart on the panel
  /// @param label    his sol$ of the slot, empty for RADIX
  void set_slot(int index, const AafRecord& r, bool activate, const QString& label = {});
  void update_slot_actions();
  [[nodiscard]] int next_slot() const;
  /// Writes the panel into the SOLAR...-DATEN row of the radix number it
  /// came from like his sol$(2,ze), a new result replaces the old one.
  ///
  /// @param label his sol$ of the result
  void store_solar(const QString& label);
  /// Names the SOLAR...-DATEN rows after their results.
  void update_solar_actions();
  // AUFRÄUMEN / RÜCKSETZEN and the plain HOROSKOP - GRAPHIK entry
  void reset_views();
  /// HOROSKOP - GRAPHIK, the plain chart and under KOMPAKT-AUSWERTUNG
  /// the count popup of a11_1.
  void horoskop_graphik();
  /// Switches every special view off except the kept ones, their own
  /// off handlers clean up behind them.
  ///
  /// @param keep the view actions that stay as they are
  void leave_views(std::initializer_list<const QAction*> keep);
  void clear_slots();
  /// ALLES ZURÜCKSETZEN of the ANSICHT menu, a rewrite addition. Returns
  /// the view settings and the VORGABEN of his profile and starts the
  /// program anew, the records, places and own files stay.
  void full_reset();
  /// Stores the pair of a double chart for its DOPPEL-DATEN row.
  ///
  /// @param kind    kDoubleComposit, kDoubleCombin or kDoubleWheel
  /// @param partner the second record, the first is the current record
  void remember_double(int kind, const AafRecord& partner);
  /// Names the DOPPEL-DATEN rows after their stored pairs.
  void update_double_actions();
  /// Brings a stored pair back, an empty row starts the chart.
  ///
  /// @param kind the row
  void recall_double(int kind);
  // the VORGABEN EPHEMERIDE ÄNDERN chain and the HÄUSERSYSTEM box
  void vorgaben_ephemeride();
  void choose_house_system();
  // the printer side of the original, main_window_print.cpp
  /// DRUCKER-OPTION EIN / AUS of the EIN-AUSG. menu, his pr_enabel.
  void printer_option_entry();
  /// @return true while a composite, combin or double chart is up
  [[nodiscard]] bool pair_view() const;
  /// @return true when start_hardc would offer the HARDCOPY for the entry
  [[nodiscard]] bool hardcopy_due(int item) const;
  /// His hardc_kompl, the HARDCOPY box after an output and its print.
  ///
  /// @param output the output window, the main window for its chart
  /// @param item   his muuu& number of the output
  void hardcopy_offer(QWidget* output, int item);
  /// His wart over the chart of the main window with the HARDCOPY after it.
  ///
  /// @param item his muuu& number of the output
  void wart(int item, bool more_follows = false);
  /// His druck_graph_ein, BILDSCHIRM or a DRUCKER-GRAPHIK.
  ///
  /// @param item his muuu& number of the output
  /// @param a4   the DIN A4 answer is offered
  /// @return one of the kOutput answers, kOutputScreen while the option is off
  [[nodiscard]] int ask_graphic_output(int item, bool a4);
  /// Prints a page as his DRUCKER-GRAPHIK.
  ///
  /// @param page     the page
  /// @param a4       the a11 page of DIN A4, else the half page
  /// @param left_div the divisor of his left margin
  /// @return true when the printer took the page
  bool print_graphic(const DisplayList& page, bool a4, double left_div);
  /// druck_graph_ein around a chart already on the main window, the
  /// print or the screen until a key with the HARDCOPY.
  ///
  /// @param item his muuu& number of the output
  /// @param a4   the DIN A4 answer is offered
  void chart_output(int item, bool a4);
  /// His druck_horm after MULTI and HARMONICS.
  ///
  /// @return false after ABBRUCH
  [[nodiscard]] bool multi_print_offer();
  /// His mehrf_1, F9 in an output keeps its picture.
  ///
  /// @param output the output window, the main window for its chart
  void double_capture(QWidget* output);
  /// His mehrf_aus_1, the print offer of the two kept pictures.
  void double_offer();
  /// His mehrf_a, the preview and the print of both pictures.
  void double_print_run();
  /// His mehrf_a_end, the double memory empties.
  void double_clear();
  /// His scget at the end of an output, the page of the output window.
  ///
  /// @param output the output window, the main window for its chart
  void remember_picture(QWidget* output);
  /// LETZTES BILD ZEIGEN / bzw.SPEICHERN, his screen routine.
  void last_picture();
  /// Shows a page or a picture until a key like his KEYGET after PUT.
  ///
  /// @param title the window title
  /// @param page  the page, empty for the picture
  /// @param shot  the picture of an output without a sheet canvas
  void show_picture(const QString& title, const DisplayList& page, const QPixmap& shot);
  // the Parameter - Einstellungen = VORGABEN overview of his main screen
  void vorgaben_overview();
  [[nodiscard]] std::optional<AafRecord> choose_record(const QString& title);
  [[nodiscard]] std::optional<AafRecord> choose_record_from_file(const QString& title);
  [[nodiscard]] ChartInput record_input(const AafRecord& r) const;
  [[nodiscard]] ChartRecord dat_from_record(const AafRecord& r) const;
  [[nodiscard]] AafRecord panel_record() const;
  /// @return the panel date in the panel calendar, astronomical year
  ///         count, day zero when the field does not hold a date
  [[nodiscard]] CalendarDate panel_day() const;
  /// @return true when the field holds a date of the panel calendar
  [[nodiscard]] bool panel_day_valid() const;
  /// Writes the date field, years before Christ with his vC.
  void set_panel_day(int day, int month, int astro_year);
  /// @return the panel moment as a Gregorian QDate for date widgets
  [[nodiscard]] QDate panel_qdate() const;
  /// @return the Julian day UT a record stands for, the JD first like
  ///         the loader rule, else its clock, zone and summer time
  [[nodiscard]] double record_jd_ut(const AafRecord& r) const;
  /// Switches the panel between zone time and a historic local time,
  /// the ORTSZEIT box of his EINGABE-BOX with the questions of zuo.
  void set_local_time(bool on, bool ask);
  /// Sets the summer time shift and mirrors it into the two boxes.
  void set_dst(double hours);
  /// Sets the calendar rule and mirrors it into the NOCH JULIANISCH box.
  void set_panel_calendar(Calendar cal);
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
  /// the ERGEBNIS als RADIX menu entry, only enabled while an active
  /// solar/derived chart holds the panel
  QAction* result_as_radix_action_ = nullptr;
  // his grossj! and zipu$, the great year display of this session
  bool great_year_on_ = false;
  // DESKTOP ( QUIT HORCOM ) asked already, the close needs no second box
  bool quit_confirmed_ = false;
  // ALLES ZURÜCKSETZEN wrote his profile, the close stores nothing over it
  bool keep_konsta_file_ = false;
  // starts the fresh program after ALLES ZURÜCKSETZEN, the tests replace it
  std::function<void()> restart_;
  // his halbszeitwaus!, the counters switched off for the whole session
  bool counters_off_session_ = false;
  // his halbszaus!, the midpoint count popup switched off for the session
  // zaehl_asp_halbs& of the STATISTIK counters, asked once per visit
  int stat_counter_mode_ = 0;
  QAction* korrektur_action_ = nullptr;
  // the a16 entries whose captions follow hrg!
  QAction* solar_action_ = nullptr;
  QAction* solar_list_action_ = nullptr;
  QAction* lunar_action_ = nullptr;
  QAction* lunar_list_action_ = nullptr;
  QAction* septar_action_ = nullptr;
  QAction* time_wander_action_ = nullptr;
  // the ERLÄUTERUNG of the menu the last entry came from, his muuu&
  QString help_stem_ = QStringLiteral("komm1");
  QString great_year_age_;
  QAction* great_year_action_ = nullptr;
  /// the slot the running clock feeds, his zeuhr, -1 when none
  int uhr_slot_ = -1;
  /// the two captured sheets of the F9 double print, his mehrf buffers
  std::vector<DisplayList> double_buffer_;
  /// the pictures of captured outputs without a sheet canvas
  std::vector<QPixmap> double_pictures_;
  /// his mehrf!, the double memory collects and HARDCOPY rests
  bool double_active_ = false;
  /// his scrn&(win&), the page of the last output and its picture when
  /// the output had no sheet canvas
  DisplayList last_page_;
  QPixmap last_shot_;
  QString last_title_;
  /// his scrout!, STATISTIK or ZEIT-WANDERN left no picture for
  /// LETZTES BILD
  bool scrout_ = false;
  /// his hardcop!, a HARDCOPY is under way
  bool hardcopy_running_ = false;
  /// the copy of a wart key that goes on to the output after the
  /// HARDCOPY, only this very event passes the HARDCOPY gate
  const QEvent* replayed_key_ = nullptr;
  /// his muuu& while the chart of the main window waits in wart, zero
  /// otherwise
  int wart_item_ = 0;
  /// his moda& answers of druck_graph_ein
  static constexpr int kOutputNone = 0;
  static constexpr int kOutputScreen = 1;
  static constexpr int kOutputA5 = 2;
  static constexpr int kOutputA4 = 3;
  /// the outer symbol colour of the double wheels, his hard&
  int outer_color_ = 2;
  /// his alt!, the old sign rulers Mars, Saturn and Jupiter for Scorpio,
  /// Aquarius and Pisces, kept for the session like the original
  bool alt_rulers_ = false;
  /// the wheel shows a plain chart, the right mouse opens einzel_plan_wahl
  bool plain_view_ = false;
  /// the capture hook switches the clock without the takeover question
  // his uhr&, gouhr$ and the rest of uhr_par, the clock of the session
  bool uhr_on_ = false;
  bool uhr_place_set_ = false;
  AafRecord uhr_place_;
  QElapsedTimer clock_record_clock_;
  QLabel* clock_strip_ = nullptr;

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
  /// sets the Wahrer and Mittlerer radios of Mondknoten and Schwarzer
  /// Mond from the hidden switches, for callers that set those blocked
  std::function<void()> sync_lunar_radios_;
  /// the TERRAR captions and the blank lunar entries of the helio mode
  std::function<void(bool)> helio_menu_;
  QComboBox* houses_ = nullptr;
  QDockWidget* body_dock_ = nullptr;
  QDockWidget* cusp_dock_ = nullptr;
  /// the next turn of the loop fits the coordinate dock, after a show or
  /// a resize of the window, never after a new fill so the panels stand
  /// still while one works in the input panel
  QTimer* dock_fit_ = nullptr;
  /// the widest width each coordinate column showed this session and the
  /// text size they were measured at, the columns never narrow again
  std::vector<int> body_widths_;
  int body_widths_px_ = 0;
  /// the tallest the summary box stood this session
  int summary_height_ = 0;
  /// Fits the coordinate dock on the next turn of the loop, near a third
  /// of the window and closed on its last whole column.
  void schedule_dock_fit();
  /// Fits the coordinate dock, its columns close flush with the edge and
  /// the rest scrolls.
  void fit_body_dock();
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
  /// Sommerzeit moves the event time by one hour, the summer offset is
  /// added to the zone in the calculation like in his ZONEN file
  QCheckBox* sommerzeit_ = nullptr;
  /// DOPPELTE SOMMERZEIT = DDSZ of his zone dialog, two hours
  QCheckBox* double_dst_ = nullptr;
  /// the summer shift the calculation uses, half an hour for the AAF h
  double dst_hours_ = 0.0;
  /// ORTSZEIT (HISTORISCHE HOROSKOPE), the clock follows the longitude
  QCheckBox* local_time_ = nullptr;
  ClockKind clock_kind_ = ClockKind::kZone;
  /// NOCH JULIANISCH ( NACH 1582 ), his jul$ of the panel record
  QCheckBox* julian_ = nullptr;
  Calendar panel_calendar_ = Calendar::kAuto;
  QCheckBox* helio_ = nullptr;
  QCheckBox* transit_on_ = nullptr;
  QDateEdit* tdate_ = nullptr;
  QTimeEdit* ttime_ = nullptr;
  QAction* clock_action_ = nullptr;
  QTimer* clock_timer_ = nullptr;
  QAction* compare_action_ = nullptr;
  QAction* dial_action_ = nullptr;
  QAction* harmonic_action_ = nullptr;
  double harm_n_ = 0.0;
  bool harm_new_mc_ = false;
  QAction* multi_action_ = nullptr;
  MultiMode multi_mode_ = MultiMode::kMulti1;
  MultiReference multi_ref_;
  double multi_event_jd_ = 0.0;
  bool multi_new_mc_ = false;
  QAction* composite_action_ = nullptr;
  QAction* directions_action_ = nullptr;
  // horm 2, the ÄQUATORIAL = MUNDAN reference frame of every chart
  bool mundane_frame_ = false;
  // the Rhythmenlehre answers of the session, his phas$ with mon$ and
  // jahre$, the eingp$ and eingm$ of the last run and the lpkt age of a
  // date defined Sonderpunkt, none of them in the settings file
  QString rhythm_phase_;
  bool rhythm_months_ = false;
  QString rhythm_unit_;
  QString eingp_;
  QString eingm_;
  double rhythm_lpkt_ = 0.0;
  // the sol$(2,ze) of a SEPTAR under way, it heads the a17sol lines before
  // the chart exists, empty otherwise
  QString rhythm_label_;
  // SONNEN- or MOND-BOGEN on the wheel, the directed radix outside, the
  // HOROSKOP-GRAPHIK branch of a19, arc_jd_ the life date of the ring
  QAction* arc_action_ = nullptr;
  double arc_jd_ = 0.0;
  bool arc_moon_ = false;
  // the SEKUNDÄR ring of a20_horg, the sky of the progressed moment
  // arc_prog_jd_ at the radix place instead of the light's arc. Both
  // rings keep the progressed moment, his jd = jd1 + da
  bool arc_secondary_ = false;
  double arc_prog_jd_ = 0.0;
  // the running factors while a wheel_run walks, empty otherwise
  std::optional<RunningRing> running_ring_;
  // the directed axes of prima on the wheel, the event place and whose
  // planets ride on them, his cpl& choice
  double dir_jd_ = 0.0;
  bool dir_converse_ = false;
  double dir_vary_ = 0.0;
  double dir_lon_ = 0.0;
  double dir_lat_ = 0.0;
  bool dir_event_planets_ = false;
  /// the variations primhorg writes under the directed axes, his dif,
  /// sum and va& of the prima session
  struct AxesSums {
    double dif = 0.0;
    double sum = 0.0;
    int va = 0;
  };
  AxesSums dir_sums_;
  std::optional<Chart> partner_chart_;
  std::optional<AafRecord> partner_record_;
  // KONSTA7P.INT of the original, beside the data where the working
  // files live
  std::filesystem::path konsta_file_;
  // the od = 0 slots of his DOPPEL-DATEN rows
  static constexpr int kDoubleComposit = 0;
  static constexpr int kDoubleCombin = 1;
  static constexpr int kDoubleWheel = 2;
  static constexpr int kDoubleKinds = 3;
  struct DoubleSlot {
    AafRecord base;
    AafRecord partner;
    /// the ROBERT HAND residence of a COMPOSIT
    std::optional<EventPlace> residence;
    /// all records of a COMBIN and their slot numbers
    std::vector<AafRecord> parts;
    std::vector<int> sets;
  };
  std::array<std::optional<DoubleSlot>, kDoubleKinds> double_slots_{};
  std::array<QAction*, kDoubleKinds> double_actions_{};
  ChartInput partner_input_;
  QString partner_name_;
  QString partner_place_;
  /// the sol$ of the outer chart, RADIX or the label of its SOLAR slot
  QString partner_label_ = QStringLiteral("RADIX");
  /// the wheel shows one of his full sheets, exports take it as it is
  bool full_sheet_ = false;
  /// the ROBERT HAND residence of the shown COMPOSIT, his a31_o after
  /// the place mask, empty lets the panel place stand in
  std::optional<EventPlace> comp_residence_;
  // the combin source memory, so the wheel can name both parents plus
  // the calculated mid moment as the tester wants, ready as the a13aus
  // corner rows. An empty name1 means no combin
  std::string combin_name1_;
  std::string combin_moment1_;
  std::string combin_name2_;
  std::string combin_moment2_;
  std::string combin_note_;
  // the names of a COMBIN of more than two
  std::vector<std::string> combin_list_;
  // his na$(0,2) of the COMBIN on the panel, LEFT$ 10 of the first two
  QString combin_na_;
  // the sol$ of the RADIX slots, empty reads RADIX, ERGEBNIS als RADIX
  // writes the label of its source with ALS RADIX
  std::array<QString, 5> radix_labels_{};
  // the Ereig: and RADIX: lines proho writes beside a progressed wheel
  std::string prog_event_note_;
  std::string prog_radix_note_;
  bool progression_pending_ = false;
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
  QScrollArea* aspects_scroll_ = nullptr;
  /// sizes the summary box to its text, eight lines at most
  void fit_summary();
  std::optional<Chart> last_chart_;
  std::optional<AspectResult> last_aspects_;
};

}  // namespace horcom
