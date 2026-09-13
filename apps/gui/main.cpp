// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QTimer>

#include "main_window.hpp"
#include "place_dialog.hpp"
#include "theme.hpp"

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

  const std::filesystem::path data = find_data_dir();
  if (data.empty()) {
    QMessageBox::critical(nullptr, "HORCOM", "Der Ordner 'data' mit planets.dat wurde nicht gefunden.");
    return 1;
  }
  horcom::VsopTables vsop;
  try {
    vsop = horcom::VsopTables::load(data / "planets.ndx", data / "planets.dat");
  } catch (const std::exception&) {
    QMessageBox::critical(nullptr, "HORCOM", "Die Planetentafeln konnten nicht geladen werden.");
    return 1;
  }
  horcom::Ephemerides eph(data / "eph");

  // --shot-place FILE captures the place dialog unshown, the same hook
  // for visual checks as --shot below
  const QStringList early_args = QApplication::arguments();
  const int shot_place = early_args.indexOf("--shot-place");
  if (shot_place >= 0 && shot_place + 1 < early_args.size()) {
    horcom::PlaceDialog dialog(data / "places");
    dialog.grab().save(early_args[shot_place + 1]);
    return 0;
  }

  horcom::MainWindow window(std::move(vsop), std::move(eph), data);

  // --shot FILE saves a capture of the window and quits, the hook for
  // visual checks without touching the desktop
  const QStringList args = QApplication::arguments();
  const int shot = args.indexOf("--shot");
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
