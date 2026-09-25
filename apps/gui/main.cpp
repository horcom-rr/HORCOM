// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTimer>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTimeEdit>
#include <QCalendarWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QTranslator>

#include "aaf_mask_dialog.hpp"
#include "aspektarium_dialog.hpp"
#include "horcom/render/aspektarium.hpp"
#include "horcom/render/stat_sheet.hpp"
#include "wheel_widget.hpp"
#include "choice_dialog.hpp"
#include "kommen_dialog.hpp"
#include "record_list_dialog.hpp"
#include "record_mask_dialog.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "main_window.hpp"
#include "place_dialog.hpp"
#include "theme.hpp"
#include "transit_list_dialog.hpp"
#include "zeitzon_dialog.hpp"
#include "zone_dialog.hpp"

namespace {

// the data directory travels next to the executable, above it during
// development builds, or in the share tree of an installed or AppImage
// layout
std::filesystem::path find_data_dir() {
  const QString app = QCoreApplication::applicationDirPath();
  for (const QString& candidate : {app + "/data", app + "/../data", app + "/../../data",
                                   app + "/../share/horcom/data", QDir::currentPath() + "/data"}) {
    if (QDir(candidate).exists("planets.dat")) {
      return std::filesystem::path(candidate.toStdWString());
    }
  }
  return {};
}

// the data folder doubles as the working folder of the original, where
// konsta.int, SPEZIAL and the own place files are written. A read only
// install keeps its shipped files and works in the per user data folder
// instead, seeded with every shipped file it does not hold yet so a
// user's edits survive updates. The planet tables and ephemerides stay
// read from the shipped folder. An AppImage counts as read only even
// when unpacked to a temporary folder
std::filesystem::path working_dir(const std::filesystem::path& shipped) {
  // the data of a source tree is what ships, a development build works
  // in its own copy so a session never rewrites the shipped konsta.int
  std::error_code tree_ec;
  const bool source_tree = std::filesystem::exists(shipped.parent_path() / "CMakeLists.txt", tree_ec) &&
                           std::filesystem::exists(shipped.parent_path() / "src" / "horcom", tree_ec);
  QTemporaryFile probe(QString::fromStdWString((shipped / "probe").wstring()));
  if (!qEnvironmentVariableIsSet("APPIMAGE") && !source_tree && probe.open()) {
    return shipped;
  }
  const std::filesystem::path own =
      std::filesystem::path(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).toStdWString()) /
      "horcom" / "data";
  std::error_code ec;
  for (auto it = std::filesystem::recursive_directory_iterator(shipped, ec);
       it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
    const std::filesystem::path rel = it->path().lexically_relative(shipped);
    const std::filesystem::path top = *rel.begin();
    if (top == "eph" || top == "planets.dat" || top == "planets.ndx") {
      if (it->is_directory()) {
        it.disable_recursion_pending();
      }
      continue;
    }
    const std::filesystem::path target = own / rel;
    if (it->is_directory()) {
      std::filesystem::create_directories(target, ec);
    } else if (!std::filesystem::exists(target, ec)) {
      std::filesystem::create_directories(target.parent_path(), ec);
      std::filesystem::copy_file(it->path(), target, ec);
    }
  }
  return std::filesystem::exists(own, ec) ? own : shipped;
}

// Courier New carries every fixed width sheet and table. Where it is
// missing its metric twin Liberation Mono stands in, from the system or
// from a fonts folder shipped beside the data, so the columns keep the
// widths they have on Windows
void substitute_courier(const std::filesystem::path& shipped) {
  if (QFontDatabase::hasFamily(QStringLiteral("Courier New"))) {
    return;
  }
  const QDir fonts(QString::fromStdWString((shipped.parent_path() / "fonts").wstring()));
  for (const QString& file : fonts.entryList({"*.ttf", "*.otf"}, QDir::Files)) {
    QFontDatabase::addApplicationFont(fonts.filePath(file));
  }
  QFont::insertSubstitutions(QStringLiteral("Courier New"),
                             {QStringLiteral("Liberation Mono"), QStringLiteral("Cousine"),
                              QStringLiteral("Nimbus Mono PS"), QStringLiteral("DejaVu Sans Mono")});
}

}  // namespace

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  QApplication::setApplicationName("horcom");
  QApplication::setOrganizationName("horcom");
  const std::filesystem::path data = find_data_dir();
  substitute_courier(data);
  // the remembered text scale and theme of the Ansicht menu, black on
  // white unless the night theme was chosen
  horcom::theme::apply(QSettings().value(horcom::theme::kTextScaleKey, horcom::theme::kTextScaleNormal).toInt(),
                       horcom::theme::dark_theme());
  const QStringList args = QApplication::arguments();

  // German is the native language of the program, every other locale
  // reads the English translation. The Sprache choice of the Ansicht
  // menu persists in the settings, --lang de|en overrides for checks
  QString lang;
  const int lang_arg = args.indexOf("--lang");
  if (lang_arg >= 0 && lang_arg + 1 < args.size()) {
    lang = args[lang_arg + 1];
  }
  if (lang.isEmpty()) {
    lang = QSettings().value("language").toString();
  }
  const bool german = !horcom::english_for(lang);
  app.setProperty(horcom::kEnglishEditionProperty, !german);
  QTranslator translator;
  if (!german && translator.load(":/i18n/horcom_en.qm")) {
    QApplication::installTranslator(&translator);
  }
  QTranslator qt_translator;
  if (german && qt_translator.load(QLocale(QLocale::German), "qtbase", "_",
                                   QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
    QApplication::installTranslator(&qt_translator);
  }

  // --dump-qss FILE writes the resolved stylesheet, the theme check of
  // the shot hook family
  const int dump_qss = args.indexOf("--dump-qss");
  if (dump_qss >= 0 && dump_qss + 1 < args.size()) {
    QFile f(args[dump_qss + 1]);
    if (f.open(QIODevice::WriteOnly)) {
      f.write(app.styleSheet().toUtf8());
    }
  }

  if (data.empty()) {
    QMessageBox::critical(nullptr, "HORCOM",
                          QCoreApplication::translate("main", "Der Ordner 'data' mit planets.dat wurde nicht gefunden."));
    return 1;
  }
  horcom::VsopTables vsop;
  try {
    vsop = horcom::VsopTables::load(data / "planets.ndx", data / "planets.dat");
  } catch (const std::exception&) {
    QMessageBox::critical(nullptr, "HORCOM",
                          QCoreApplication::translate("main", "Die Planetentafeln konnten nicht geladen werden."));
    return 1;
  }
  horcom::Ephemerides eph(data / "eph");

  // --shot-place and --shot-zone capture the dialogs unshown, the same
  // hook for visual checks as --shot below
  // --shot-spin renders the spin and combo controls for theme checks
  const int shot_spin = args.indexOf("--shot-spin");
  if (shot_spin >= 0 && shot_spin + 1 < args.size()) {
    QDialog dialog;
    auto* form = new QFormLayout(&dialog);
    auto* d = new QDoubleSpinBox(&dialog);
    d->setValue(48.1742);
    d->setDecimals(4);
    auto* i = new QSpinBox(&dialog);
    i->setValue(7);
    auto* t = new QTimeEdit(QTime(12, 30), &dialog);
    auto* c = new QComboBox(&dialog);
    c->addItems({"PLACIDUS", "KOCH"});
    auto* off = new QDateEdit(QDate(2026, 9, 14), &dialog);
    off->setCalendarPopup(true);
    off->setEnabled(false);
    auto* cal = new QCalendarWidget(&dialog);
    form->addRow("Breite", d);
    form->addRow("Haus", i);
    form->addRow("Zeit", t);
    form->addRow("System", c);
    form->addRow("Aus", off);
    form->addRow(cal);
    dialog.resize(360, 480);
    dialog.grab().save(args[shot_spin + 1]);
    return 0;
  }
  const int shot_place = args.indexOf("--shot-place");
  if (shot_place >= 0 && shot_place + 1 < args.size()) {
    horcom::PlaceDialog dialog(data / "places", data / "landnima.int");
    dialog.grab().save(args[shot_place + 1]);
    return 0;
  }
  const int shot_zone = args.indexOf("--shot-zone");
  if (shot_zone >= 0 && shot_zone + 1 < args.size()) {
    horcom::ZoneDialog dialog(data / "zonnamen.int");
    dialog.grab().save(args[shot_zone + 1]);
    return 0;
  }
  const int shot_record = args.indexOf("--shot-record");
  if (shot_record >= 0 && shot_record + 1 < args.size()) {
    horcom::AafRecord sample;
    sample.surname = "Muster";
    sample.given = "Max";
    sample.place = "Eichenau";
    sample.country = "D";
    horcom::AafMaskDialog dialog(sample, horcom::AafMaskDialog::Mode::kEdit, data);
    dialog.grab().save(args[shot_record + 1]);
    return 0;
  }
  const int shot_statist = args.indexOf("--shot-statist");
  if (shot_statist >= 0 && shot_statist + 1 < args.size()) {
    // his list_ausg page of a synthetic set, invented charts, no real people
    std::vector<horcom::StatSheetRow> rows;
    const char* names[3] = {"ANNA MUSTER", "BERND BEISPIEL", "CARLA DEMO"};
    for (int i = 0; i < 3; ++i) {
      horcom::StatSheetRow r;
      r.slot = 1;
      r.value = (15.0 + 55.0 * i) * horcom::kDegToRad;
      r.label = names[i];
      r.moment = QString::asprintf("%2d.%2d.%5d  %2d h %2d'", 1 + 10 * i, 1 + i, 1960 + 10 * i, 12, 30).toStdString();
      r.has_angle = true;
      r.angle = (10.0 + 120.0 * i) * horcom::kDegToRad;
      rows.push_back(r);
    }
    horcom::StatSheetText text;
    text.heads = "Datum     Zeit(UT)    AC";
    text.file = " Datei : DEMO.STA ";
    text.object_line = "Länge SO";
    text.window_line = "In ZEICHEN";
    text.bars = true;
    text.sums = {3, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1};
    text.object_tag = " SO ";
    text.total = 3;
    text.partial_count = 3;
    text.footer = "* Blättern: Leertaste | Zurück mit 'R'|Weitere Beding: 'W'|ENDE: Mit 'ESC' *";
    horcom::WheelWidget w;
    w.set_plain_list(horcom::build_stat_page(rows, text));
    w.resize(1280, 960);
    w.grab().save(args[shot_statist + 1]);
    return 0;
  }
  const int shot_aspektarium = args.indexOf("--shot-aspektarium");
  if (shot_aspektarium >= 0 && shot_aspektarium + 1 < args.size()) {
    horcom::ChartInput in;
    in.date_ut = {13, 10, 1992, 3, 0.0};
    in.lon_deg_east = 11.3244;
    in.lat_deg = 48.1742;
    const horcom::ChartSettings cs;
    const horcom::Chart chart = horcom::compute_chart(in, cs, vsop, eph);
    const horcom::AspectResult scan = horcom::scan_aspects(chart, cs, {});
    horcom::AspektariumInput sheet;
    sheet.chart = &chart;
    sheet.settings = cs;
    sheet.aspects = &scan;
    sheet.weights.fill(100);
    horcom::AspektariumText text;
    text.title = "Geozentrisches Aspektarium  | RADIX";
    text.date = "13.10.1992";
    horcom::AspektariumDialog dialog(horcom::build_aspektarium(sheet, text));
    dialog.grab().save(args[shot_aspektarium + 1]);
    return 0;
  }
  // the dialog family of the DATEN-DATEI EIN-AUSGABE rework, captured
  // for the side by side check against his reference screenshots
  const int shot_hub = args.indexOf("--shot-hub");
  if (shot_hub >= 0 && shot_hub + 1 < args.size()) {
    horcom::ChoiceDialog dialog(
        "DATEI : MUSIKER.DAT", {},
        {QStringLiteral("Datensätze HOLEN ?"), QStringLiteral("AKTUELLEN Datensatz EINTRAGEN ?"),
         QStringLiteral("Datensätze LÖSCHEN ?"), QStringLiteral("Datei TRIMMEN ?"),
         QStringLiteral("Datei MINIMIEREN ?"), QStringLiteral("ABBRUCH")},
        0);
    dialog.grab().save(args[shot_hub + 1]);
    return 0;
  }
  const int shot_sort = args.indexOf("--shot-sort");
  if (shot_sort >= 0 && shot_sort + 1 < args.size()) {
    horcom::ChoiceDialog dialog(
        "SORTIER-MODUS ?  DATEI : MUSIKER.DAT", {},
        {QStringLiteral("ALPHABETISCH: 1., 2. und 3. NAME"), QStringLiteral("ALPHABETISCH: 2. und 3. NAME"),
         QStringLiteral("ALPHABETISCH: nur 3. NAME"), QStringLiteral("GEBURTSTAG"), QStringLiteral("DATUM"),
         QStringLiteral("ABBRUCH")},
        0);
    dialog.grab().save(args[shot_sort + 1]);
    return 0;
  }
  const int shot_chooser = args.indexOf("--shot-chooser");
  if (shot_chooser >= 0 && shot_chooser + 1 < args.size()) {
    // invented sample records, no real people
    std::vector<horcom::AafRecord> sample;
    const char* names[4] = {"MUSTER ANNA MARIA", "BEISPIEL BERND", "DEMO CARLA", "PROBE DORA"};
    const char* places[4] = {"EICHENAU", "SALZBURG", "WIEN", "PARIS"};
    for (int i = 0; i < 4; ++i) {
      horcom::AafRecord r;
      r.surname = names[i];
      r.place = places[i];
      r.day = 3 + 7 * i;
      r.month = 1 + 2 * i;
      r.year = 1920 + 17 * i;
      r.hour = 4 + 5 * i;
      r.minute = 10 * i;
      r.lon_deg = 11 + i;
      r.lon_min = 19;
      r.lat_deg = 48;
      r.lat_min = 10 + i;
      sample.push_back(r);
    }
    std::vector<std::size_t> order{1, 2, 0, 3};
    horcom::RecordListDialog dialog(sample, order, "MUSIKER.DAT", 5, horcom::RecordListDialog::Mode::kFetch);
    dialog.grab().save(args[shot_chooser + 1]);
    return 0;
  }
  const int shot_mask = args.indexOf("--shot-mask");
  if (shot_mask >= 0 && shot_mask + 1 < args.size()) {
    // an invented sample record, no real person
    horcom::AafRecord r;
    r.surname = "MUSTER MAX ADAM";
    r.place = "EICHENAU";
    r.day = 12;
    r.month = 4;
    r.year = 1950;
    r.hour = 8;
    r.minute = 30;
    r.second = 1;
    r.lon_deg = 11;
    r.lon_min = 19;
    r.lon_sec = 0;
    r.lat_deg = 48;
    r.lat_min = 10;
    r.lat_sec = 12;
    r.comment = "BEISPIEL: 9H30 =8H30 UT";
    horcom::RecordMaskDialog dialog(r, "EINGABE- und ANZEIGE-BOX | RADIX NR.1",
                                    horcom::RecordMaskDialog::Mode::kShow);
    dialog.grab().save(args[shot_mask + 1]);
    return 0;
  }
  const int shot_entry = args.indexOf("--shot-entry");
  if (shot_entry >= 0 && shot_entry + 1 < args.size()) {
    horcom::RecordMaskDialog dialog(horcom::AafRecord{}, "NEU-EINGABE von DATENSÄTZEN  :   |  RADIX NR.1",
                                    horcom::RecordMaskDialog::Mode::kEntry, data);
    dialog.grab().save(args[shot_entry + 1]);
    return 0;
  }
  const int shot_zeitzon = args.indexOf("--shot-zeitzon");
  if (shot_zeitzon >= 0 && shot_zeitzon + 1 < args.size()) {
    horcom::ZeitzonDialog dialog(data);
    dialog.grab().save(args[shot_zeitzon + 1]);
    return 0;
  }
  const int shot_aaf = args.indexOf("--shot-aaf");
  if (shot_aaf >= 0 && shot_aaf + 1 < args.size()) {
    // an invented sample record, no real person
    horcom::AafRecord r;
    r.surname = "MUSTER";
    r.given = "MAX ADAM";
    r.sex = "m";
    r.day = 12;
    r.month = 4;
    r.year = 1950;
    r.hour = 9;
    r.minute = 30;
    r.second = 1;
    r.place = "EICHENAU";
    r.country = "D";
    r.lat_deg = 48;
    r.lat_min = 10;
    r.lat_sec = 12;
    r.lon_deg = 11;
    r.lon_min = 19;
    r.zone = "01hE00:00";
    r.comment = "BEISPIEL: 9H30 =8H30 UT";
    r.source = "Beispiel";
    r.quality = "A";
    r.catchword = "Muster";
    horcom::AafMaskDialog dialog(r, data);
    dialog.grab().save(args[shot_aaf + 1]);
    return 0;
  }
  const int shot_kommen = args.indexOf("--shot-kommen");
  if (shot_kommen >= 0 && shot_kommen + 1 < args.size()) {
    // the capture follows the chosen language so the English edition
    // can be checked the same way as the shell
    horcom::KommenDialog dialog(data / "kommen", QString(), !german);
    dialog.grab().save(args[shot_kommen + 1]);
    return 0;
  }
  const int shot_list = args.indexOf("--shot-transit-list");
  if (shot_list >= 0 && shot_list + 1 < args.size()) {
    horcom::SearchContext sctx;
    sctx.base.date_ut = {13, 10, 1992, 3, 0.0};
    sctx.base.lon_deg_east = 11.3244;
    sctx.base.lat_deg = 48.1742;
    sctx.vsop = &vsop;
    sctx.eph = &eph;
    const horcom::Chart radix = horcom::compute_chart(sctx.base, sctx.settings, vsop, eph);
    horcom::TransitScan scan;
    scan.jd_from_ut = horcom::julian_day({1, 9, 2026, 0, 0.0});
    scan.jd_to_ut = horcom::julian_day({1, 10, 2026, 0, 0.0});
    horcom::A18Display display;
    display.heading = {" TRANSITE  |Grund-Aspekt: 30°   Ab 01.09.2026"};
    display.settings = sctx.settings;
    horcom::TransitListDialog dialog(horcom::scan_transits(radix, scan, sctx), display);
    dialog.grab().save(args[shot_list + 1]);
    return 0;
  }

  horcom::MainWindow window(std::move(vsop), std::move(eph), working_dir(data));

  // --chart "DD.MM.YYYY,HH:MM:SS,E,11,35,0,N,48,8,0,NAME" presets the
  // panel with parallax and true node on, the batch comparison hook
  const int chart_arg = args.indexOf("--chart");
  if (chart_arg >= 0 && chart_arg + 1 < args.size()) {
    const QStringList f = args[chart_arg + 1].split(',');
    if (f.size() >= 10) {
      horcom::AafRecord rec;
      const QStringList d = f[0].split('.');
      const QStringList t = f[1].split(':');
      if (d.size() == 3 && t.size() >= 2) {
        rec.day = d[0].toInt();
        rec.month = d[1].toInt();
        rec.year = d[2].toInt();
        rec.hour = t[0].toInt();
        rec.minute = t[1].toInt();
        rec.second = t.size() > 2 ? t[2].toInt() : 0;
        rec.zone = "00hE00:00";
        rec.lon_ew = f[2].isEmpty() ? 'E' : f[2].at(0).toLatin1();
        rec.lon_deg = f[3].toInt();
        rec.lon_min = f[4].toInt();
        rec.lon_sec = f[5].toInt();
        rec.lat_ns = f[6].isEmpty() ? 'N' : f[6].at(0).toLatin1();
        rec.lat_deg = f[7].toInt();
        rec.lat_min = f[8].toInt();
        rec.lat_sec = f[9].toInt();
        if (f.size() > 10) {
          rec.surname = f[10].toStdString();
        }
        if (f.size() > 11) {
          rec.place = f[11].toStdString();
        }
        window.preset_chart(rec, true, true);
      }
    }
  }

  // --extras REAL,HAMBURG,AG switches the extra body rows, each 0 or 1
  const int extras_arg = args.indexOf("--extras");
  if (extras_arg >= 0 && extras_arg + 1 < args.size()) {
    const QStringList f = args[extras_arg + 1].split(',');
    if (f.size() == 3) {
      window.preset_extras(f[0] == "1", f[1] == "1", f[2] == "1");
    }
  }

  // --houses N presets the house system, his hausw numbering 1 to 9
  const int houses_arg = args.indexOf("--houses");
  if (houses_arg >= 0 && houses_arg + 1 < args.size()) {
    window.preset_houses(args[houses_arg + 1].toInt());
  }

  // --shot FILE saves a capture of the window and quits, the hook for
  // visual checks without touching the desktop, --shot-transit FILE does
  // the same with the transit view switched on
  int shot = args.indexOf("--shot");
  const int shot_transit = args.indexOf("--shot-transit");
  if (shot_transit >= 0) {
    shot = shot_transit;
    window.show_transits(QDate::currentDate(), QTime(12, 0));
  }
  const int shot_solar = args.indexOf("--shot-solar");
  if (shot_solar >= 0) {
    shot = shot_solar;
    window.show_solar(QDate::currentDate().year());
  }
  const int shot_clock = args.indexOf("--shot-clock");
  if (shot_clock >= 0) {
    shot = shot_clock;
    window.show_clock();
  }
  const int shot_compare = args.indexOf("--shot-compare");
  if (shot_compare >= 0) {
    shot = shot_compare;
    horcom::AafRecord partner;
    partner.surname = "Partner";
    partner.day = 1;
    partner.month = 6;
    partner.year = 1990;
    partner.hour = 12;
    partner.zone = "00hE00:00";
    partner.lat_deg = 48;
    partner.lat_min = 10;
    partner.lon_deg = 11;
    partner.lon_min = 19;
    window.show_compare(partner);
  }
  const int shot_composite = args.indexOf("--shot-composite");
  if (shot_composite >= 0) {
    shot = shot_composite;
    horcom::AafRecord partner;
    partner.surname = "Partner";
    partner.day = 1;
    partner.month = 6;
    partner.year = 1990;
    partner.hour = 12;
    partner.zone = "00hE00:00";
    partner.lat_deg = 48;
    partner.lat_min = 10;
    partner.lon_deg = 11;
    partner.lon_min = 19;
    window.show_composite(partner);
  }
  const int shot_helio = args.indexOf("--shot-helio");
  if (shot_helio >= 0) {
    shot = shot_helio;
    window.show_helio();
  }
  const int shot_harmonic = args.indexOf("--shot-harmonic");
  if (shot_harmonic >= 0) {
    shot = shot_harmonic;
    window.show_harmonic(5);
  }
  const int shot_dial = args.indexOf("--shot-dial");
  if (shot_dial >= 0) {
    shot = shot_dial;
    horcom::AafRecord partner;
    partner.surname = "Partner";
    partner.day = 1;
    partner.month = 6;
    partner.year = 1990;
    partner.hour = 12;
    partner.zone = "00hE00:00";
    partner.lat_deg = 48;
    partner.lat_min = 10;
    partner.lon_deg = 11;
    partner.lon_min = 19;
    window.show_dial(partner);
  }
  const int shot_mundane = args.indexOf("--shot-mundane");
  if (shot_mundane >= 0) {
    shot = shot_mundane;
    window.show_mundane();
  }
  const int shot_directions = args.indexOf("--shot-directions");
  if (shot_directions >= 0) {
    shot = shot_directions;
    window.show_directions(horcom::julian_day({13, 10, 2026, 0, 0.0}), false);
  }
  // --shot-erg-radix FILE runs a solar off the current chart then
  // promotes the result via ERGEBNIS als RADIX, the capture hook for
  // the DIVERSES entry
  const int shot_erg = args.indexOf("--shot-erg-radix");
  if (shot_erg >= 0 && shot_erg + 1 < args.size()) {
    window.show_solar(QDate::currentDate().year());
    window.promote_result_to_radix_scripted();
    shot = shot_erg;
  }
  // --shot-planet FILE opens the Planeten-Auswahl dialog and captures it
  const int shot_planet = args.indexOf("--shot-planet");
  if (shot_planet >= 0 && shot_planet + 1 < args.size()) {
    window.showMinimized();
    const QString target = args[shot_planet + 1];
    QTimer::singleShot(400, [&window, target]() {
      QTimer::singleShot(400, [target]() {
        for (QWidget* w : QApplication::topLevelWidgets()) {
          if (auto* d = qobject_cast<QDialog*>(w); d != nullptr && d->windowTitle().contains(QStringLiteral("Planeten"))) {
            d->grab().save(target);
            QCoreApplication::exit(0);
            return;
          }
        }
        QCoreApplication::exit(0);
      });
      window.open_planet_selection();
    });
    return QApplication::exec();
  }
  // --shot-pdf FILE writes the current wheel as a PDF page and quits,
  // the headless check of the print path
  const int shot_pdf = args.indexOf("--shot-pdf");
  if (shot_pdf >= 0 && shot_pdf + 1 < args.size()) {
    window.export_pdf_to(args[shot_pdf + 1]);
    return 0;
  }
  // --shot-pdf-a4 FILE writes the big DIN A4 page with the bes_big tables
  const int shot_pdf_a4 = args.indexOf("--shot-pdf-a4");
  if (shot_pdf_a4 >= 0 && shot_pdf_a4 + 1 < args.size()) {
    window.export_pdf_to(args[shot_pdf_a4 + 1], true);
    return 0;
  }
  // --shot-svg FILE does the same for the SVG export
  const int shot_svg = args.indexOf("--shot-svg");
  if (shot_svg >= 0 && shot_svg + 1 < args.size()) {
    window.export_svg_to(args[shot_svg + 1]);
    return 0;
  }
  // --size WxH fixes the window size, mainly for captures
  const int size_arg = args.indexOf("--size");
  if (size_arg >= 0 && size_arg + 1 < args.size()) {
    const QStringList wh = args[size_arg + 1].split('x');
    if (wh.size() == 2 && wh[0].toInt() > 0 && wh[1].toInt() > 0) {
      window.resize(wh[0].toInt(), wh[1].toInt());
    }
  }
  // a shot flag without a target path opens the preset view normally,
  // so a doppelkreis or composit lands on screen for a live look
  const bool will_shoot = shot >= 0 && shot + 1 < args.size() && !args[shot + 1].startsWith("--");
  if (will_shoot) {
    window.showMinimized();
  } else {
    window.show();
  }
  if (will_shoot) {
    const QString target = args[shot + 1];
    // exit, not quit, a quit would close the window and meet the quit
    // question of the close box
    QTimer::singleShot(1200, &window, [&window, target]() {
      window.grab().save(target);
      QCoreApplication::exit(0);
    });
  }
  return QApplication::exec();
}
