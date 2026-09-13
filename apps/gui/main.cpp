// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QTimer>
#include <QTranslator>

#include "main_window.hpp"
#include "place_dialog.hpp"
#include "theme.hpp"
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
  app.setStyleSheet(horcom::theme::kStyleSheet);
  QApplication::setApplicationName("horcom");
  QApplication::setOrganizationName("horcom");
  const QStringList args = QApplication::arguments();

  // German is the native language of the program, every other locale
  // reads the English translation, --lang de|en overrides for checks
  QString lang;
  const int lang_arg = args.indexOf("--lang");
  if (lang_arg >= 0 && lang_arg + 1 < args.size()) {
    lang = args[lang_arg + 1];
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

  horcom::MainWindow window(std::move(vsop), std::move(eph), data);

  // --shot FILE saves a capture of the window and quits, the hook for
  // visual checks without touching the desktop, --shot-transit FILE does
  // the same with the transit view switched on
  int shot = args.indexOf("--shot");
  const int shot_transit = args.indexOf("--shot-transit");
  if (shot_transit >= 0) {
    shot = shot_transit;
    window.show_transits(QDate::currentDate(), QTime(12, 0));
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
