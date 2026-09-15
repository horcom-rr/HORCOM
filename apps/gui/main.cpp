// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTimeEdit>
#include <QCalendarWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QTranslator>

#include "aspektarium_dialog.hpp"
#include "kommen_dialog.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "main_window.hpp"
#include "place_dialog.hpp"
#include "record_dialog.hpp"
#include "statist_dialog.hpp"
#include "theme.hpp"
#include "transit_list_dialog.hpp"
#include "zone_dialog.hpp"

namespace {

// the data directory travels next to the executable or above it during
// development builds
std::filesystem::path find_data_dir() {
  const QString app = QCoreApplication::applicationDirPath();
  for (const QString& candidate : {app + "/data", app + "/../data", app + "/../../data", QDir::currentPath() + "/data"}) {
    if (QDir(candidate).exists("planets.dat")) {
      return std::filesystem::path(candidate.toStdWString());
    }
  }
  return {};
}

}  // namespace

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  QApplication::setApplicationName("horcom");
  QApplication::setOrganizationName("horcom");
  // the remembered text scale of the Ansicht menu
  app.setStyleSheet(horcom::theme::stylesheet(
      QSettings().value(horcom::theme::kTextScaleKey, horcom::theme::kTextScaleNormal).toInt()));
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
  const bool german = lang.isEmpty() ? QLocale::system().language() == QLocale::German : lang == "de";
  QTranslator translator;
  if (!german && translator.load(":/i18n/horcom_en.qm")) {
    QApplication::installTranslator(&translator);
  }
  QTranslator qt_translator;
  if (german && qt_translator.load(QLocale(QLocale::German), "qtbase", "_",
                                   QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
    QApplication::installTranslator(&qt_translator);
  }

  const std::filesystem::path data = find_data_dir();
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
    std::vector<horcom::GermanCountry> countries;
    if (const auto c = horcom::load_german_countries(data / "laender.int")) {
      countries = *c;
    }
    horcom::AafRecord sample;
    sample.surname = "Muster";
    sample.given = "Max";
    sample.place = "Eichenau";
    sample.country = "D";
    horcom::RecordDialog dialog(sample, countries);
    dialog.grab().save(args[shot_record + 1]);
    return 0;
  }
  const int shot_statist = args.indexOf("--shot-statist");
  if (shot_statist >= 0 && shot_statist + 2 < args.size()) {
    horcom::StatistDialog dialog;
    bool ok = false;
    if (args[shot_statist + 2] == "-") {
      // a synthetic capture set, invented charts, no real people
      horcom::StatSet set;
      const char* names[3] = {"ANNA MUSTER", "BERND BEISPIEL", "CARLA DEMO"};
      for (int i = 0; i < 3; ++i) {
        horcom::StatRecord r;
        r.name = names[i];
        r.place = "EICHENAU";
        r.day = 1 + 10 * i;
        r.month = 1 + i;
        r.year = 1960 + 10 * i;
        r.ac = (10.0 + 120.0 * i) * horcom::kDegToRad;
        r.mc = (280.0 + 120.0 * i) * horcom::kDegToRad;
        r.h2 = (40.0 + 120.0 * i) * horcom::kDegToRad;
        r.h3 = (70.0 + 120.0 * i) * horcom::kDegToRad;
        r.h5 = (130.0 + 120.0 * i) * horcom::kDegToRad;
        r.h6 = (160.0 + 120.0 * i) * horcom::kDegToRad;
        for (int slot = 1; slot <= 10; ++slot) {
          r.el[static_cast<std::size_t>(slot)] = horcom::norm_rad((15.0 + 37.0 * slot + 55.0 * i) * horcom::kDegToRad);
        }
        set.records.push_back(r);
      }
      dialog.load_set(set);
      ok = true;
    } else {
      ok = dialog.load(args[shot_statist + 2]);
    }
    if (ok) {
      dialog.grab().save(args[shot_statist + 1]);
    }
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
    horcom::AspektariumDialog dialog(chart, cs, {}, "13.10.1992");
    dialog.grab().save(args[shot_aspektarium + 1]);
    return 0;
  }
  const int shot_kommen = args.indexOf("--shot-kommen");
  if (shot_kommen >= 0 && shot_kommen + 1 < args.size()) {
    horcom::KommenDialog dialog(data / "kommen");
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
    horcom::TransitListDialog dialog(radix, sctx);
    dialog.preset(QDate(2026, 9, 1), QDate(2026, 10, 1));
    dialog.run_scan();
    dialog.grab().save(args[shot_list + 1]);
    return 0;
  }

  horcom::MainWindow window(std::move(vsop), std::move(eph), data);

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
        window.preset_chart(rec, true, true);
      }
    }
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
  // --shot-pdf FILE writes the current wheel as a PDF page and quits,
  // the headless check of the print path
  const int shot_pdf = args.indexOf("--shot-pdf");
  if (shot_pdf >= 0 && shot_pdf + 1 < args.size()) {
    window.export_pdf_to(args[shot_pdf + 1]);
    return 0;
  }
  if (shot >= 0) {
    window.showMinimized();
  } else {
    window.show();
  }
  if (shot >= 0 && shot + 1 < args.size()) {
    const QString target = args[shot + 1];
    QTimer::singleShot(1200, &window, [&window, target]() {
      window.grab().save(target);
      QApplication::quit();
    });
  }
  return QApplication::exec();
}
