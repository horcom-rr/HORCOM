// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QCheckBox>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QRadioButton>
#include <QScrollArea>
#include <QSettings>
#include <QTableWidget>
#include <QTimeEdit>
#include <QTimer>
#include <QToolButton>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/konsta.hpp"
#include "horcom/time/local_time.hpp"
#include "print_pages.hpp"
#include "probe.hpp"
#include "theme.hpp"


using namespace horcom;

namespace {

// a synthetic record, no real person
AafRecord sample(int day, int month, int year, int hour, int minute, const char* zone, const char* dst = "") {
  AafRecord r;
  r.surname = "TESTFALL";
  r.day = day;
  r.month = month;
  r.year = year;
  r.hour = hour;
  r.minute = minute;
  r.zone = zone;
  r.dst = dst;
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 35;
  return r;
}

constexpr double kSecondDays = 1.0 / 86400.0;

}  // namespace

TEST_CASE("a half hour zone and the summer code reach the panel moment") {
  auto w = MainWindowProbe::make();
  // the old atof read 05hE30:00 as five hours
  const AafRecord india = sample(15, 8, 1947, 12, 0, "05hE30:00");
  MainWindowProbe::apply(*w, india);
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - aaf_moment_jd_ut(india)) < kSecondDays);
  CHECK(MainWindowProbe::zone(*w)->value() == doctest::Approx(5.5));
  // the summer code checks the box and enters the moment
  const AafRecord summer = sample(1, 7, 1980, 14, 30, "01hE00:00", "1");
  MainWindowProbe::apply(*w, summer);
  CHECK(MainWindowProbe::sommerzeit(*w)->isChecked());
  CHECK(MainWindowProbe::dst(*w) == doctest::Approx(1.0));
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - aaf_moment_jd_ut(summer)) < kSecondDays);
}

TEST_CASE("the summer box does not leak into the next record") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(1, 7, 1980, 14, 30, "01hE00:00", "1"));
  REQUIRE(MainWindowProbe::sommerzeit(*w)->isChecked());
  // a DAT record carries UT without any summer code
  const AafRecord ut = sample(3, 3, 1975, 9, 0, "00hE00:00");
  MainWindowProbe::apply(*w, ut);
  CHECK_FALSE(MainWindowProbe::sommerzeit(*w)->isChecked());
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - aaf_moment_jd_ut(ut)) < kSecondDays);
  // and the saved record carries the summer shift it was entered with
  MainWindowProbe::apply(*w, sample(1, 7, 1980, 14, 30, "01hE00:00", "1"));
  const AafRecord back = MainWindowProbe::record(*w);
  CHECK(aaf_dst_hours(back.dst) == doctest::Approx(1.0));
  CHECK(std::abs(aaf_moment_jd_ut(back) - MainWindowProbe::panel_jd(*w)) < kSecondDays);
}

TEST_CASE("a Julian record keeps its calendar through the panel and the file") {
  auto w = MainWindowProbe::make();
  AafRecord r = sample(1, 1, 1900, 12, 0, "00hE00:00");
  r.calendar = Calendar::kJulian;
  MainWindowProbe::apply(*w, r);
  CHECK(MainWindowProbe::calendar(*w) == Calendar::kJulian);
  CHECK(MainWindowProbe::julian(*w)->isChecked());
  // the Julian first of January 1900 is the Gregorian thirteenth
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - julian_day({13, 1, 1900, 12, 0.0})) < kSecondDays);
  // the saved record and its DAT form keep the Julian date, no drift
  const AafRecord back = MainWindowProbe::record(*w);
  CHECK(back.calendar == Calendar::kJulian);
  CHECK(back.day == 1);
  CHECK(back.month == 1);
  const ChartRecord dat = MainWindowProbe::dat(*w, back);
  CHECK(dat.day == 1);
  CHECK(dat.month == 1);
  CHECK(dat.year == 1900);
  CHECK(dat.calendar() == Calendar::kJulian);
  // the Julian leap day of 1700 is a real date of that calendar
  MainWindowProbe::date(*w)->setText("29.02.1700");
  CHECK(MainWindowProbe::day_valid(*w));
  MainWindowProbe::julian(*w)->setChecked(false);
  CHECK_FALSE(MainWindowProbe::day_valid(*w));
}

TEST_CASE("a derived chart lands in UT whatever the summer box said") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(1, 7, 1980, 14, 30, "01hE00:00", "1"));
  const double solar = julian_day({2, 7, 2026, 7, 15.0});
  MainWindowProbe::moment(*w, solar, false);
  // the summer hour once stayed on and shifted every derived chart
  CHECK_FALSE(MainWindowProbe::sommerzeit(*w)->isChecked());
  CHECK(MainWindowProbe::zone(*w)->value() == doctest::Approx(0.0));
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - solar) < kSecondDays);
}

TEST_CASE("a corrected radix keeps its own clock, zone and summer time") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(1, 7, 1980, 14, 30, "01hE00:00", "1"));
  const double corrected = MainWindowProbe::panel_jd(*w) + 7.0 / 1440.0;
  MainWindowProbe::moment(*w, corrected, true);
  CHECK(MainWindowProbe::sommerzeit(*w)->isChecked());
  CHECK(MainWindowProbe::zone(*w)->value() == doctest::Approx(1.0));
  CHECK(MainWindowProbe::time(*w)->time() == QTime(14, 37, 0));
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - corrected) < kSecondDays);
}

TEST_CASE("a historic local clock follows the longitude like zuo") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(3, 11, 1792, 12, 0, "00hE00:00"));
  MainWindowProbe::local_time(*w, true);
  // before 1810 the clock counts as true local time
  CHECK(MainWindowProbe::clock(*w) == ClockKind::kTrueLocal);
  const double jd_local = julian_day({3, 11, 1792, 12, 0.0});
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) -
                 ut_from_local_clock(jd_local, MainWindowProbe::lon(*w), ClockKind::kTrueLocal)) < kSecondDays);
  MainWindowProbe::local_time(*w, false);
  CHECK(MainWindowProbe::clock(*w) == ClockKind::kZone);
}

TEST_CASE("the julian date of a record outranks disagreeing clock fields") {
  auto w = MainWindowProbe::make();
  AafRecord r = sample(1, 1, 2000, 0, 0, "00hE00:00");
  r.jd = 2451545.25;
  MainWindowProbe::apply(*w, r);
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - 2451545.25) < kSecondDays);
}

TEST_CASE("a hidden body loses its aspects and a hidden node its axis") {
  auto w = MainWindowProbe::make();
  const auto involves = [](const AspectResult& a, int slot) {
    for (const auto& h : a.hits) {
      if (h.t == slot || h.w == slot) {
        return true;
      }
    }
    return false;
  };
  REQUIRE(involves(MainWindowProbe::aspects(*w), body::kMars));
  //RR NUR AUSGEWÄHLTE Planeten und DEREN ASPEKTE im HOROSKOP ANZEIGEN
  MainWindowProbe::hide(*w, body::kMars);
  CHECK_FALSE(involves(MainWindowProbe::aspects(*w), body::kMars));
  // the node line stands only while both nodes are chosen
  const auto dashed_blue = [](const DisplayList& dl) {
    int n = 0;
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kLine && p.style == Primitive::Style::kDashed && p.color == 0x0000FF) {
        ++n;
      }
    }
    return n;
  };
  const int with_nodes = dashed_blue(MainWindowProbe::wheel(*w));
  MainWindowProbe::hide(*w, body::kNodeDesc);
  CHECK(dashed_blue(MainWindowProbe::wheel(*w)) < with_nodes);
}

namespace {

std::string file_text(const std::filesystem::path& p) {
  std::ifstream in(p, std::ios::binary);
  std::ostringstream text;
  text << in.rdbuf();
  return text.str();
}

}  // namespace

TEST_CASE("ALLES ZURÜCKSETZEN returns the view and his profile and starts anew") {
  using horcom::test::DialogDriver;
  auto w = MainWindowProbe::make();
  const std::filesystem::path file = MainWindowProbe::konsta_file(*w);
  bool restarted = false;
  MainWindowProbe::on_restart(*w, [&restarted]() { restarted = true; });
  QSettings().setValue(theme::kTextScaleKey, 140);
  QSettings().setValue(theme::kDarkKey, true);
  QSettings().setValue("language", "en");
  Konsta changed = robert_profile();
  changed.orb = 3.0;
  REQUIRE(save_konsta(file, changed));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ABBRUCH"));
    MainWindowProbe::full_reset(*w);
    CHECK(drive.pending() == 0);
  }
  // ABBRUCH leaves every choice as it was
  CHECK_FALSE(restarted);
  CHECK(QSettings().value(theme::kTextScaleKey).toInt() == 140);
  CHECK(load_konsta(file)->orb == doctest::Approx(3.0));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ZURÜCKSETZEN und NEU STARTEN"));
    MainWindowProbe::full_reset(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(restarted);
  CHECK_FALSE(QSettings().contains(theme::kTextScaleKey));
  CHECK_FALSE(QSettings().contains(theme::kDarkKey));
  // the language is a choice of its own and stays
  CHECK(QSettings().value("language").toString() == "en");
  // his profile stands in the file byte for byte
  const std::filesystem::path fresh = std::filesystem::temp_directory_path() / "horcom_gui_tests_profile.int";
  REQUIRE(save_konsta(fresh, robert_profile()));
  CHECK(file_text(file) == file_text(fresh));
  QSettings().remove("language");
  std::filesystem::remove(fresh);
}

TEST_CASE("the docks keep their dressed title bar while they float") {
  auto w = MainWindowProbe::make();
  w->show();
  QApplication::processEvents();
  int dressed = 0;
  for (QDockWidget* d : w->findChildren<QDockWidget*>()) {
    QWidget* bar = d->titleBarWidget();
    REQUIRE(bar != nullptr);
    // the system title would stand in while floating or dragging
    CHECK(bar->objectName() == "dockTitleBar");
    ++dressed;
    if (d->features().testFlag(QDockWidget::DockWidgetFloatable)) {
      d->setFloating(true);
      QApplication::processEvents();
      CHECK(d->isFloating());
      CHECK(d->titleBarWidget() == bar);
      d->setFloating(false);
      QApplication::processEvents();
    }
    // the close button of a closable dock hides it like the X of Qt
    if (d->features().testFlag(QDockWidget::DockWidgetClosable)) {
      const QList<QToolButton*> buttons = bar->findChildren<QToolButton*>();
      REQUIRE(buttons.size() == 2);
      buttons.back()->click();
      QApplication::processEvents();
      CHECK_FALSE(d->isVisible());
      d->show();
    }
  }
  CHECK(dressed == 3);
  w->hide();
}

TEST_CASE("the right docks stand still while the charts change in the input panel") {
  auto w = MainWindowProbe::make();
  w->resize(1600, 900);
  w->show();
  QApplication::processEvents();
  QApplication::processEvents();
  QDockWidget* bodies = nullptr;
  for (QDockWidget* d : w->findChildren<QDockWidget*>()) {
    if (d->widget() != nullptr && qobject_cast<QTableWidget*>(d->widget()) != nullptr) {
      bodies = d;
    }
  }
  REQUIRE(bodies != nullptr);
  auto* table = qobject_cast<QTableWidget*>(bodies->widget());
  auto* summary = w->findChild<QScrollArea*>();
  REQUIRE(summary != nullptr);
  const int dock_width = bodies->width();
  std::vector<int> widths;
  int summary_height = summary->height();
  // invented charts over three centuries, their values differ in width
  for (const int year : {1805, 1950, 1992, 2031, 1877}) {
    AafRecord r;
    r.surname = "TESTFALL";
    r.day = 1 + year % 27;
    r.month = 1 + year % 12;
    r.year = year;
    r.hour = year % 24;
    r.zone = "00hE00:00";
    r.lat_deg = 48;
    r.lon_deg = 11;
    MainWindowProbe::apply(*w, r);
    QApplication::processEvents();
    QApplication::processEvents();
    // the dock keeps the width it was given
    CHECK(bodies->width() == dock_width);
    widths.resize(static_cast<std::size_t>(table->columnCount()), 0);
    for (int c = 0; c + 1 < table->columnCount(); ++c) {
      // a column never narrows again in the session
      CHECK(table->columnWidth(c) >= widths[static_cast<std::size_t>(c)]);
      widths[static_cast<std::size_t>(c)] = table->columnWidth(c);
    }
    // the summary box only grows, the cusps above it keep their place
    CHECK(summary->height() >= summary_height);
    summary_height = summary->height();
  }
  w->hide();
}

namespace {

// the history settles a step after a pause of the panel
void settle_step() {
  QElapsedTimer t;
  t.start();
  while (t.elapsed() < 1000) {
    QApplication::processEvents(QEventLoop::AllEvents, 100);
  }
}

// the Zurück or Vor button of the panel, found by its caption
QToolButton* history_button(MainWindow& w, const QString& part) {
  for (QToolButton* b : w.findChildren<QToolButton*>()) {
    if (b->defaultAction() != nullptr && b->defaultAction()->text().contains(part)) {
      return b;
    }
  }
  return nullptr;
}

// an invented person, no real one
AafRecord person(const char* name, int year) {
  AafRecord r;
  r.surname = name;
  r.day = 3;
  r.month = 4;
  r.year = year;
  r.hour = 9;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lon_deg = 11;
  return r;
}

}  // namespace

TEST_CASE("Zurück and Vor bring the Wahrer and Mittlerer radios back with the node") {
  auto w = MainWindowProbe::make();
  w->show();
  MainWindowProbe::apply(*w, person("TESTFALL", 1990));
  settle_step();
  QRadioButton* wahr = nullptr;
  QRadioButton* mittel = nullptr;
  for (QRadioButton* r : w->findChildren<QRadioButton*>()) {
    // the first pair of the panel is the Mondknoten row
    if (r->text().contains("Wahrer") && wahr == nullptr) {
      wahr = r;
    } else if (r->text().contains("Mittlerer") && mittel == nullptr) {
      mittel = r;
    }
  }
  REQUIRE(wahr != nullptr);
  REQUIRE(mittel != nullptr);
  const bool was_true = wahr->isChecked();
  (was_true ? mittel : wahr)->click();
  settle_step();
  REQUIRE(wahr->isChecked() != was_true);
  history_button(*w, "Zur")->click();
  QApplication::processEvents();
  // the visible radios follow the node switch the step restored
  CHECK(wahr->isChecked() == was_true);
  CHECK(mittel->isChecked() != was_true);
  history_button(*w, "Vor")->click();
  QApplication::processEvents();
  CHECK(wahr->isChecked() != was_true);
  w->hide();
}

TEST_CASE("Zurück across a slot switch brings the slot back and never overwrites the other one") {
  auto w = MainWindowProbe::make();
  w->show();
  MainWindowProbe::put_slot(*w, 0, person("ERSTE", 1950));
  MainWindowProbe::put_slot(*w, 1, person("ZWEITE", 1980));
  MainWindowProbe::session_click(*w, 0);
  settle_step();
  MainWindowProbe::session_click(*w, 1);
  settle_step();
  REQUIRE(MainWindowProbe::active_slot(*w) == 1);
  REQUIRE(MainWindowProbe::record(*w).surname == "ZWEITE");
  history_button(*w, "Zur")->click();
  QApplication::processEvents();
  // the step belonged to SATZ1, SATZ1 leads again with its person
  CHECK(MainWindowProbe::active_slot(*w) == 0);
  CHECK(MainWindowProbe::record(*w).surname == "ERSTE");
  // an edit now lands in SATZ1, SATZ2 keeps its own person
  w->findChild<QTimeEdit*>()->setTime(QTime(17, 45));
  settle_step();
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  REQUIRE(MainWindowProbe::slot(*w, 1).has_value());
  CHECK(MainWindowProbe::slot(*w, 0)->surname == "ERSTE");
  CHECK(MainWindowProbe::slot(*w, 0)->hour == 17);
  CHECK(MainWindowProbe::slot(*w, 1)->surname == "ZWEITE");
  CHECK(MainWindowProbe::slot(*w, 1)->hour == 9);
  w->hide();
}

TEST_CASE("an edit made just before a slot switch stays in its own slot") {
  auto w = MainWindowProbe::make();
  w->show();
  MainWindowProbe::put_slot(*w, 0, person("ERSTE", 1950));
  MainWindowProbe::put_slot(*w, 1, person("ZWEITE", 1980));
  MainWindowProbe::session_click(*w, 0);
  settle_step();
  // the edit has not settled yet when the slot changes
  w->findChild<QTimeEdit*>()->setTime(QTime(21, 10));
  MainWindowProbe::session_click(*w, 1);
  settle_step();
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  CHECK(MainWindowProbe::slot(*w, 0)->hour == 21);
  CHECK(MainWindowProbe::slot(*w, 1)->hour == 9);
  w->hide();
}

TEST_CASE("AUFRÄUMEN / RÜCKSETZEN also clears the Zurück and Vor steps") {
  using horcom::test::DialogDriver;
  auto w = MainWindowProbe::make();
  w->show();
  MainWindowProbe::apply(*w, person("TESTFALL", 1970));
  settle_step();
  MainWindowProbe::apply(*w, person("TESTFALL", 1990));
  settle_step();
  REQUIRE(MainWindowProbe::history_steps(*w) > 0);
  {
    DialogDriver drive;
    drive.then([](QDialog* d) {
      // DATEN und GESPEICHERTE BILDER dieser Sitzung LÖSCHEN ?
      if (auto* box = qobject_cast<QMessageBox*>(d)) {
        box->button(QMessageBox::Ok)->click();
      }
    });
    MainWindowProbe::clear_slots(*w);
  }
  CHECK(MainWindowProbe::history_steps(*w) == 0);
  CHECK_FALSE(history_button(*w, "Zur")->isEnabled());
  CHECK_FALSE(history_button(*w, "Vor")->isEnabled());
  w->hide();
}

TEST_CASE("wart holds the chart only where a question follows and shows that it waits") {
  auto w = MainWindowProbe::make();
  w->show();
  QApplication::processEvents();
  const auto key_later = [&w](int ms) {
    QTimer::singleShot(ms, w.get(), [&w]() {
      QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
      QApplication::sendEvent(w.get(), &space);
    });
  };
  // without the DRUCKER-OPTION a wait before nothing but the HARDCOPY goes,
  // the key a few seconds on only ends a wait that should not be there
  key_later(3000);
  QElapsedTimer t;
  t.start();
  MainWindowProbe::wart(*w, menu_item::kReturns, false);
  CHECK(t.elapsed() < 1000);
  CHECK(w->menuBar()->isEnabled());
  QApplication::processEvents();
  // before his questions the chart waits, the menu greys and the bar says so
  bool menu_off = false;
  bool bar_shown = false;
  QTimer::singleShot(300, w.get(), [&]() {
    menu_off = !w->menuBar()->isEnabled();
    const auto* bar = w->findChild<QLabel*>("waitBar");
    bar_shown = bar != nullptr && bar->isVisible();
  });
  key_later(600);
  MainWindowProbe::wart(*w, menu_item::kMulti, true);
  CHECK(menu_off);
  CHECK(bar_shown);
  CHECK(w->menuBar()->isEnabled());
  CHECK(w->findChild<QLabel*>("waitBar") == nullptr);
  // the late key of the first wait finds no wait any more
  QElapsedTimer rest;
  rest.start();
  while (rest.elapsed() < 2600) {
    QApplication::processEvents(QEventLoop::AllEvents, 100);
  }
  w->hide();
}
