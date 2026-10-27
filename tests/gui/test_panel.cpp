// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QCheckBox>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
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
#include "horcom/data/place_file.hpp"
#include "horcom/time/local_time.hpp"
#include "print_pages.hpp"
#include "probe.hpp"
#include "theme.hpp"
#include "zodiac_cells.hpp"


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

TEST_CASE("the text sizes step on his ladder, a pixel down and two up") {
  // the tester found one step down far too small and one step up too
  // timid, the old tenths cut the pixels short
  CHECK(theme::next_text_scale(theme::kTextScaleNormal, true) == 115);
  CHECK(theme::next_text_scale(theme::kTextScaleNormal, false) == 90);
  CHECK(theme::scaled_px(14, 90) == 13);
  CHECK(theme::scaled_px(13, 90) == 12);
  CHECK(theme::scaled_px(14, 115) == 16);
  CHECK(theme::scaled_px(11, 115) == 13);
  // the ends of the ladder hold and a stored value between steps moves on
  CHECK(theme::next_text_scale(theme::kTextScaleMax, true) == theme::kTextScaleMax);
  CHECK(theme::next_text_scale(theme::kTextScaleMin, false) == theme::kTextScaleMin);
  CHECK(theme::next_text_scale(140, true) == 150);
  CHECK(theme::next_text_scale(140, false) == 130);
  // the column heads of every table stand bold and a step larger than the
  // cells, bold alone had looked like the bold columns below them. The row
  // heads keep the size of the cells
  const QString qss = theme::stylesheet(theme::kTextScaleNormal, false);
  const auto rule_of = [&qss](const QString& selector) {
    const qsizetype at = qss.indexOf(selector + " {");
    return at < 0 ? QString() : qss.mid(at, qss.indexOf('}', at) - at);
  };
  const QString head = rule_of("QHeaderView::section");
  const QString cell_size = QString("font-size: %1px;").arg(theme::kTableCellPx);
  REQUIRE_FALSE(head.isEmpty());
  CHECK(head.contains(QString("font-size: %1px;").arg(theme::kTableHeadPx)));
  CHECK(head.contains("font-weight: bold;"));
  CHECK(rule_of("QTableWidget").contains(cell_size));
  CHECK(rule_of("QHeaderView::section:vertical").contains(cell_size));
}

TEST_CASE("the coordinate dock lists the tester's rows with both lunar forms") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::preset_extras(*w, true, false, true);
  MainWindowProbe::include(*w, body::kFortune, true);
  MainWindowProbe::apply(*w, sample(10, 5, 1970, 7, 0, "00hE00:00"));
  QTableWidget* t = MainWindowProbe::bodies(*w);
  QStringList heads;
  for (int r = 0; r < t->rowCount(); ++r) {
    heads << t->verticalHeaderItem(r)->text();
  }
  INFO(heads.join(",").toStdString());
  // SO to PL, the node twice, the real extras, the Black Moon twice, the
  // Glückspunkt, the invented points and the axes last, no DS, the order
  // SO-PL, DR wahr, DR mittel, CH, QU, XE, AG wahr, AG mittel, GL, AC, MC
  REQUIRE(heads.size() > 14);
  CHECK(heads.mid(0, 10) == QStringList{"SO", "MO", "ME", "VE", "MA", "JU", "SA", "UR", "NE", "PL"});
  CHECK(heads[10] == "DR wahr");
  CHECK(heads[11] == "DR mittel");
  CHECK(heads.indexOf("CH") == 12);
  CHECK(heads.indexOf("QU") == 13);
  CHECK(heads.indexOf("XE") == 14);
  CHECK(heads.indexOf("AG wahr") > heads.indexOf("XE"));
  CHECK(heads.indexOf("AG mittel") == heads.indexOf("AG wahr") + 1);
  REQUIRE(heads.contains("GL"));
  CHECK(heads.indexOf("GL") == heads.indexOf("AG mittel") + 1);
  CHECK(heads.indexOf("AC") == heads.indexOf("GL") + 1);
  CHECK(heads.indexOf("MC") == heads.indexOf("AC") + 1);
  CHECK_FALSE(heads.contains("DS"));
  // each form of the node stands with its own value, wahr the osculating
  // node and mittel the mean one
  const Chart& chart = MainWindowProbe::chart(*w);
  const std::unique_ptr<QTableWidgetItem> true_node(zodiac_item(chart.lunar.true_node));
  const std::unique_ptr<QTableWidgetItem> mean_node(zodiac_item(chart.lunar.mean_node));
  CHECK(t->item(10, 0)->text() == true_node->text());
  CHECK(t->item(11, 0)->text() == mean_node->text());
  CHECK(t->item(10, 0)->text() != t->item(11, 0)->text());
  // the true node framed and the true Black Moon on its dark square, the
  // dress the wheel gives them, the mean forms plain
  CHECK(icon_stamp(t->verticalHeaderItem(10)->icon()) == "framed");
  CHECK(icon_stamp(t->verticalHeaderItem(11)->icon()) == "plain");
  REQUIRE(heads.contains("AG wahr"));
  REQUIRE(heads.contains("AG mittel"));
  CHECK(icon_stamp(t->verticalHeaderItem(heads.indexOf("AG wahr"))->icon()) == "inverted");
  CHECK(icon_stamp(t->verticalHeaderItem(heads.indexOf("AG mittel"))->icon()) == "plain");
  CHECK(icon_stamp(t->verticalHeaderItem(0)->icon()) == "plain");
  // the motion right after the longitude as the tester reads a planet, the
  // R of a retrograde body, the arc minutes per day and the sign of their
  // change, then latitude and declination, the distance in percent of the
  // mean beside the AU
  QStringList cols;
  for (int c = 0; c < t->columnCount(); ++c) {
    cols << t->horizontalHeaderItem(c)->text();
  }
  CHECK(cols.mid(0, 8) == QStringList{"Länge", "R", "Vel.'", "A", "Breite", "Deklin.", "Entf.%", "Entf.AE"});
  CHECK(t->item(0, 6)->text().endsWith('%'));
  CHECK(t->item(0, 7)->text().toDouble() == doctest::Approx(1.0).epsilon(0.02));
  // the R stands where the velocity is negative and nowhere else, a slow
  // retrograde body may print -0.0
  for (int r = 0; r < t->rowCount(); ++r) {
    if (t->item(r, 2) != nullptr && t->item(r, 1) != nullptr) {
      CHECK((t->item(r, 1)->text() == "R") == t->item(r, 2)->text().startsWith('-'));
    }
  }
  // the A column carries a sign for the Sun and the Moon now, the mean
  // forms and the Glückspunkt stay blank like a motion that never changes
  const int a = 3;
  for (const char* body : {"SO", "MO", "DR wahr", "CH"}) {
    const int row = heads.indexOf(body);
    REQUIRE(t->item(row, a) != nullptr);
    CHECK((t->item(row, a)->text() == "+" || t->item(row, a)->text() == QString::fromUtf8("−")));
  }
  for (const char* body : {"DR mittel", "AG mittel", "GL"}) {
    REQUIRE(heads.contains(body));
    CHECK(t->item(heads.indexOf(body), a) == nullptr);
  }
  // the cusps 1 to 6 with their opposites beside them
  QTableWidget* c = MainWindowProbe::cusps(*w);
  CHECK(c->rowCount() == 6);
  CHECK(c->columnCount() == 2);
  CHECK(c->item(0, 0)->text().startsWith(" 1  "));
  CHECK(c->item(0, 1)->text().startsWith(" 7  "));
  CHECK(c->item(5, 1)->text().startsWith("12  "));
  // the aspects and midpoints in a dock of their own
  QDockWidget* aspects = MainWindowProbe::aspects_dock(*w);
  REQUIRE(aspects != nullptr);
  CHECK(aspects->windowTitle() == "Aspekte / Halbsummen");
  CHECK(MainWindowProbe::summary(*w).contains("ASPEKTE"));
}

TEST_CASE("the mundane frame lists the one projected form of each lunar point") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::preset_extras(*w, false, false, true);
  MainWindowProbe::set_mundane_frame(*w, true);
  MainWindowProbe::apply(*w, sample(10, 5, 1970, 7, 0, "00hE00:00"));
  const auto rows = [&w]() {
    QTableWidget* t = MainWindowProbe::bodies(*w);
    QStringList heads;
    for (int r = 0; r < t->rowCount(); ++r) {
      heads << t->verticalHeaderItem(r)->text();
    }
    return heads;
  };
  // the lunar points of the chart stay ecliptic, only the form the wheel
  // projected stands, named like the M and W of bes11
  QStringList heads = rows();
  INFO(heads.join(",").toStdString());
  CHECK(heads.contains("DR wahr"));
  CHECK_FALSE(heads.contains("DR mittel"));
  const int dr = heads.indexOf("DR wahr");
  QTableWidget* t = MainWindowProbe::bodies(*w);
  CHECK(icon_stamp(t->verticalHeaderItem(dr)->icon()) == "framed");
  const std::unique_ptr<QTableWidgetItem> projected(zodiac_item(MainWindowProbe::chart(*w).b[body::kNodeAsc].el));
  CHECK(t->item(dr, 0)->text() == projected->text());
  // the true node carries its sign in the A column, a mean node row stays
  // blank
  constexpr int a = 3;
  REQUIRE(t->item(dr, a) != nullptr);
  MainWindowProbe::true_node(*w, false);
  heads = rows();
  REQUIRE(heads.contains("DR mittel"));
  CHECK_FALSE(heads.contains("DR wahr"));
  CHECK(MainWindowProbe::bodies(*w)->item(heads.indexOf("DR mittel"), a) == nullptr);
  CHECK(icon_stamp(MainWindowProbe::bodies(*w)->verticalHeaderItem(heads.indexOf("DR mittel"))->icon()) == "plain");
  MainWindowProbe::set_mundane_frame(*w, false);
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
  // the input, the coordinates, the cusps and the aspects with midpoints
  CHECK(dressed == 4);
  w->hide();
}

TEST_CASE("the right docks stand still while the charts change in the input panel") {
  auto w = MainWindowProbe::make();
  w->resize(1600, 900);
  w->show();
  QApplication::processEvents();
  QApplication::processEvents();
  QTableWidget* table = MainWindowProbe::bodies(*w);
  auto* bodies = qobject_cast<QDockWidget*>(table->parentWidget());
  REQUIRE(bodies != nullptr);
  auto* cusps = qobject_cast<QDockWidget*>(MainWindowProbe::cusps(*w)->parentWidget());
  REQUIRE(cusps != nullptr);
  const int dock_width = bodies->width();
  const int cusp_height = cusps->height();
  std::vector<int> widths;
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
    // the six cusp rows keep their dock at one height
    CHECK(cusps->height() == cusp_height);
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

TEST_CASE("each click on a switch is a step of its own, however quick") {
  auto w = MainWindowProbe::make();
  w->show();
  MainWindowProbe::apply(*w, person("TESTFALL", 1990));
  settle_step();
  const auto box = [&w](const char* text) -> QCheckBox* {
    for (QCheckBox* b : w->findChildren<QCheckBox*>()) {
      if (b->text().startsWith(QString::fromUtf8(text))) {
        return b;
      }
    }
    return nullptr;
  };
  QCheckBox* parallax = box("Parallaxe");
  QCheckBox* apogee = box("Schwarzer");
  QCheckBox* extras = box("Zusatz");
  REQUIRE(parallax != nullptr);
  REQUIRE(apogee != nullptr);
  REQUIRE(extras != nullptr);
  const bool p0 = parallax->isChecked();
  const bool a0 = apogee->isChecked();
  const bool e0 = extras->isChecked();
  // well inside the settle window of a typed field
  const auto quick = []() {
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < 200) {
      QApplication::processEvents(QEventLoop::AllEvents, 50);
    }
  };
  const std::size_t before = MainWindowProbe::history_steps(*w);
  parallax->click();
  quick();
  apogee->click();
  quick();
  extras->click();
  CHECK(MainWindowProbe::history_steps(*w) == before + 3);
  // each Zurück takes back the last switch alone
  QToolButton* back = history_button(*w, "Zur");
  back->click();
  CHECK(extras->isChecked() == e0);
  CHECK(apogee->isChecked() != a0);
  CHECK(parallax->isChecked() != p0);
  back->click();
  CHECK(apogee->isChecked() == a0);
  CHECK(parallax->isChecked() != p0);
  back->click();
  CHECK(parallax->isChecked() == p0);
  history_button(*w, "Vor")->click();
  CHECK(parallax->isChecked() != p0);
  CHECK(apogee->isChecked() == a0);
  w->hide();
}

TEST_CASE("a body taken in through the Planeten-Auswahl is a step of Zurück and Vor") {
  using horcom::test::DialogDriver;
  auto w = MainWindowProbe::make();
  w->show();
  MainWindowProbe::apply(*w, person("TESTFALL", 1990));
  settle_step();
  REQUIRE_FALSE(MainWindowProbe::chart(*w).b[body::kCeres].present);
  // ticks Zeigen of Ceres and takes the choice
  {
    DialogDriver drive;
    drive.then([](QDialog* d) {
      auto* table = d->findChild<QTableWidget*>();
      REQUIRE(table != nullptr);
      for (int row = 0; row < table->rowCount(); ++row) {
        const QTableWidgetItem* name = table->item(row, 0);
        if (name != nullptr && name->data(Qt::UserRole).toInt() == body::kCeres) {
          table->cellWidget(row, 1)->findChild<QCheckBox*>()->click();
        }
      }
      d->accept();
    });
    w->open_planet_selection();
    CHECK(drive.unexpected() == 0);
  }
  REQUIRE(MainWindowProbe::chart(*w).b[body::kCeres].present);
  settle_step();
  // the step back leaves Ceres out again, the step forward brings it back
  history_button(*w, "Zur")->click();
  CHECK_FALSE(MainWindowProbe::chart(*w).b[body::kCeres].present);
  history_button(*w, "Vor")->click();
  CHECK(MainWindowProbe::chart(*w).b[body::kCeres].present);
  w->hide();
}

TEST_CASE("the Planeten-Auswahl reads and sets DR and DS through the Mondknoten switch") {
  using horcom::test::DialogDriver;
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, person("TESTFALL", 1990));
  QCheckBox* nodes = nullptr;
  for (QCheckBox* box : w->findChildren<QCheckBox*>()) {
    if (box->text() == "Mondknoten") {
      nodes = box;
    }
  }
  REQUIRE(nodes != nullptr);
  nodes->setChecked(false);
  // reads the Zeigen of both node rows, then ticks the ones asked for
  const auto visit = [&w](std::array<bool, 2>& seen, std::array<int, 2> tick) {
    DialogDriver drive;
    drive.then([&seen, tick](QDialog* d) {
      auto* table = d->findChild<QTableWidget*>();
      REQUIRE(table != nullptr);
      for (int row = 0; row < table->rowCount(); ++row) {
        const QTableWidgetItem* name = table->item(row, 0);
        const int slot = name != nullptr ? name->data(Qt::UserRole).toInt() : -1;
        if (slot != body::kNodeAsc && slot != body::kNodeDesc) {
          continue;
        }
        auto* box = table->cellWidget(row, 1)->findChild<QCheckBox*>();
        const std::size_t i = slot == body::kNodeAsc ? 0 : 1;
        seen[i] = box->isChecked();
        if (tick[i] >= 0) {
          box->setChecked(tick[i] == 1);
        }
      }
      d->accept();
    });
    w->open_planet_selection();
    CHECK(drive.unexpected() == 0);
  };
  std::array<bool, 2> seen{};
  // the first port showed both ticked while the wheel left them out
  visit(seen, {1, -1});
  CHECK_FALSE(seen[0]);
  CHECK_FALSE(seen[1]);
  // the tick of DR turns the switch on, DS stays out as it stood unticked
  CHECK(nodes->isChecked());
  visit(seen, {0, 0});
  CHECK(seen[0]);
  CHECK_FALSE(seen[1]);
  // both rows off turn the switch off, the switch alone brings both back
  CHECK_FALSE(nodes->isChecked());
  nodes->setChecked(true);
  visit(seen, {-1, -1});
  CHECK(seen[0]);
  CHECK(seen[1]);
}

TEST_CASE("Ctrl+Z and Ctrl+Y in a panel field reach Zurück and Vor") {
  auto w = MainWindowProbe::make();
  // a field that claims the key in its ShortcutOverride keeps it from the
  // window, the date field undid its own text as a new step and the clock
  // swallowed the key
  const auto claims = [](QWidget* field, QKeySequence::StandardKey key) {
    const QKeyCombination combo = QKeySequence(key)[0];
    QKeyEvent over(QEvent::ShortcutOverride, combo.key(), combo.keyboardModifiers());
    over.ignore();
    QApplication::sendEvent(field, &over);
    return over.isAccepted();
  };
  for (QWidget* field : std::initializer_list<QWidget*>{MainWindowProbe::date(*w), MainWindowProbe::time(*w),
                                                        MainWindowProbe::zone(*w)}) {
    INFO(field->metaObject()->className());
    CHECK_FALSE(claims(field, QKeySequence::Undo));
    CHECK_FALSE(claims(field, QKeySequence::Redo));
    // the other keys of a field stay its own
    CHECK(claims(field, QKeySequence::SelectAll));
  }
}

TEST_CASE("an edit taken back within its settle window leaves no step behind") {
  auto w = MainWindowProbe::make();
  w->show();
  MainWindowProbe::apply(*w, person("TESTFALL", 1990));
  settle_step();
  const std::size_t before = MainWindowProbe::history_steps(*w);
  // a spin step up and down again, the chart ends where it was
  QTimeEdit* time = MainWindowProbe::time(*w);
  const QTime was = time->time();
  time->setTime(was.addSecs(60));
  time->setTime(was);
  settle_step();
  CHECK(MainWindowProbe::history_steps(*w) == before);
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

TEST_CASE("the empty start draws nothing and greys the chart entries until a date stands") {
  auto w = MainWindowProbe::make();
  w->start_idle();
  const auto action = [&w](const char* text) -> QAction* {
    for (QAction* a : w->findChildren<QAction*>()) {
      if (a->text() == QString::fromUtf8(text)) {
        return a;
      }
    }
    return nullptr;
  };
  // his start with ze = 0, no wheel, no table, no record in the banner
  CHECK(MainWindowProbe::wheel(*w).items.empty());
  CHECK(MainWindowProbe::bodies(*w)->rowCount() == 0);
  CHECK(MainWindowProbe::banner_record(*w).isEmpty());
  CHECK_FALSE(MainWindowProbe::day_valid(*w));
  for (const char* grey : {"TRANSITE…", "HOROSKOP - GRAPHIK", "PLANETEN-KOORDINATEN", "SOLAR…", "LUNAR…",
                           "SEPTAR…", "KORREKTUR…", "GROßES ( = PLATONISCHES ) JAHR…", "DRUCKEN…", "HISTOGRAMME…"}) {
    INFO(grey);
    REQUIRE(action(grey) != nullptr);
    CHECK_FALSE(action(grey)->isEnabled());
  }
  for (const char* open : {"DATEN-DATEI EIN-AUSGABE…", "NEU-EINGABE von DATENSÄTZEN…", "VORGABEN HOROSKOP ÄNDERN…",
                           "STATISTIK G/H…", "UHR G/H…", "DATUM aus JD…", "ERLÄUTERUNG 5…", "DESKTOP ( QUIT HORCOM )"}) {
    INFO(open);
    REQUIRE(action(open) != nullptr);
    CHECK(action(open)->isEnabled());
  }
  // a switch of the panel leaves the screen empty
  MainWindowProbe::houses(*w, 2);
  CHECK(MainWindowProbe::wheel(*w).items.empty());
  // a whole date brings the chart and the entries back
  MainWindowProbe::date(*w)->setText("10.05.1970");
  emit MainWindowProbe::date(*w)->editingFinished();
  CHECK_FALSE(MainWindowProbe::wheel(*w).items.empty());
  CHECK(MainWindowProbe::bodies(*w)->rowCount() > 10);
  CHECK(action("TRANSITE…")->isEnabled());
  CHECK(action("SOLAR…")->isEnabled());
  CHECK(action("KORREKTUR…")->isEnabled());
  // no step of Zurück leads back into the empty start
  CHECK(MainWindowProbe::history_steps(*w) == 0);
  // a fetched record ends it the same way
  w->start_idle();
  REQUIRE(MainWindowProbe::wheel(*w).items.empty());
  MainWindowProbe::apply(*w, sample(10, 5, 1970, 7, 0, "00hE00:00"));
  CHECK_FALSE(MainWindowProbe::wheel(*w).items.empty());
  CHECK(action("HOROSKOP - GRAPHIK")->isEnabled());
}

TEST_CASE("a birth typed key by key on the empty start waits for its whole year") {
  auto w = MainWindowProbe::make();
  w->start_idle();
  QLineEdit* date = MainWindowProbe::date(*w);
  // a pause after each part longer than the settle window of the field
  const auto type_and_pause = [date](const QString& keys) {
    for (const QChar ch : keys) {
      QKeyEvent key(QEvent::KeyPress, 0, Qt::NoModifier, QString(ch));
      QApplication::sendEvent(date, &key);
    }
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < 700) {
      QApplication::processEvents(QEventLoop::AllEvents, 50);
    }
  };
  // 10.05.1 and 10.05.19 are real dates of the years 1 and 19
  for (const char* part : {"10.05.1", "9", "7"}) {
    type_and_pause(QString::fromUtf8(part));
    INFO(date->text().toStdString());
    CHECK(MainWindowProbe::wheel(*w).items.empty());
  }
  type_and_pause("0");
  CHECK_FALSE(MainWindowProbe::wheel(*w).items.empty());
  const double y1900 = julian_day({1, 1, 1900, 0.0, 0.0});
  CHECK(MainWindowProbe::chart(*w).jd_ut > y1900);
  // the first chart is the start of the history, no half typed year before it
  settle_step();
  CHECK(MainWindowProbe::history_steps(*w) == 0);
  // Enter takes a short year on purpose
  date->setText("10.05.197");
  emit date->editingFinished();
  CHECK(MainWindowProbe::chart(*w).jd_ut < julian_day({1, 1, 1000, 0.0, 0.0}));
}

TEST_CASE("a table dock may stand beside the DATEN panel left of the wheel") {
  auto w = MainWindowProbe::make();
  // the tester could only stack the tables above the panel
  CHECK(w->dockOptions().testFlag(QMainWindow::AllowNestedDocks));
  QDockWidget* panel = nullptr;
  QDockWidget* table = nullptr;
  for (QDockWidget* d : w->findChildren<QDockWidget*>()) {
    if (d->windowTitle() == "DATEN") {
      panel = d;
    } else if (d->windowTitle() == "Planeten-Koordinaten") {
      table = d;
    }
  }
  REQUIRE(panel != nullptr);
  REQUIRE(table != nullptr);
  w->splitDockWidget(panel, table, Qt::Horizontal);
  w->resize(1600, 900);
  w->show();
  QApplication::processEvents();
  CHECK(w->dockWidgetArea(table) == Qt::LeftDockWidgetArea);
  // side by side, the table right of the panel and both as tall
  CHECK(table->x() >= panel->x() + panel->width() - 1);
  CHECK(table->y() == panel->y());
  w->hide();
}

TEST_CASE("every table dock may stack on the DATEN panel") {
  // the least height of the window with the dock below the panel, a drop
  // only lands where the column still fits the window
  const auto stacked = [](const char* title) {
    auto w = MainWindowProbe::make();
    w->resize(1920, 1000);
    w->show();
    QApplication::processEvents();
    QDockWidget* panel = nullptr;
    QDockWidget* dock = nullptr;
    for (QDockWidget* d : w->findChildren<QDockWidget*>()) {
      if (d->windowTitle() == "DATEN") {
        panel = d;
      } else if (d->windowTitle() == QString::fromUtf8(title)) {
        dock = d;
      }
    }
    REQUIRE(panel != nullptr);
    REQUIRE(dock != nullptr);
    w->splitDockWidget(panel, dock, Qt::Vertical);
    QApplication::processEvents();
    CHECK(w->dockWidgetArea(dock) == Qt::LeftDockWidgetArea);
    const int least = w->minimumSizeHint().height();
    w->hide();
    return least;
  };
  // the tester stacked the coordinates there, the fixed six rows of the
  // cusps asked for more height than his screen had
  const int coordinates = stacked("Planeten-Koordinaten");
  CHECK(stacked("Häuser-Spitzen") <= coordinates);
  CHECK(stacked("Aspekte / Halbsummen") <= coordinates);
}

TEST_CASE("HOROSKOP-HINTERGRUND turns the paper white and back to the dress") {
  auto w = MainWindowProbe::make();
  QSettings().remove(theme::kPaperKey);
  {
    test::DialogDriver drive;
    drive.then(test::DialogDriver::click("WEIß"));
    MainWindowProbe::paper_choice(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(theme::paper_override() == QColor(Qt::white));
  CHECK(QSettings().value(theme::kPaperKey).toString() == "#ffffff");
  // the box frames the paper in force, PAPIER brings the dress back
  QString framed;
  {
    test::DialogDriver drive;
    drive.then([&framed](QDialog* d) {
      for (const QPushButton* b : d->findChildren<QPushButton*>()) {
        if (b->isDefault()) {
          framed = b->text();
        }
      }
      test::DialogDriver::click("PAPIER")(d);
    });
    MainWindowProbe::paper_choice(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(framed == "WEIß");
  CHECK_FALSE(theme::paper_override().isValid());
  CHECK_FALSE(QSettings().contains(theme::kPaperKey));
}

TEST_CASE("the Weltzeit row shows the UT the Uhrzeit stands for") {
  auto w = MainWindowProbe::make();
  // the panel names both clocks like his box named UHRZEIT and WZ = GMT = UT
  QStringList labels;
  for (const QLabel* l : w->findChildren<QLabel*>()) {
    labels << l->text();
  }
  CHECK(labels.contains("Uhrzeit"));
  CHECK(labels.contains("Weltzeit (UT)"));
  CHECK(MainWindowProbe::local_time_box(*w)->text() == "Wahre Ortszeit (LT)");
  // an afternoon five hours west of Greenwich
  MainWindowProbe::apply(*w, sample(29, 5, 1917, 15, 0, "05hW00:00"));
  CHECK(MainWindowProbe::time(*w)->time() == QTime(15, 0));
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(20, 0));
  CHECK(MainWindowProbe::ut_day(*w).isEmpty());
  // a late evening reaches UT on the next day, the row names that day
  MainWindowProbe::apply(*w, sample(29, 5, 1917, 22, 30, "05hW00:00"));
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(3, 30));
  CHECK(MainWindowProbe::ut_day(*w) == "30.05.1917");
}

TEST_CASE("the place and its coordinates follow the Uhrzeit and the UT closes the time rows") {
  auto w = MainWindowProbe::make();
  QFormLayout* form = nullptr;
  for (QFormLayout* f : w->findChildren<QFormLayout*>()) {
    int row = -1;
    QFormLayout::ItemRole role = QFormLayout::FieldRole;
    f->getWidgetPosition(MainWindowProbe::date(*w), &row, &role);
    if (row >= 0) {
      form = f;
    }
  }
  REQUIRE(form != nullptr);
  // each row by its caption, a row of switches by its first switch
  QStringList rows;
  for (int r = 0; r < form->rowCount(); ++r) {
    if (const QLayoutItem* label = form->itemAt(r, QFormLayout::LabelRole); label != nullptr && label->widget() != nullptr) {
      rows << static_cast<const QLabel*>(label->widget())->text();
    } else if (const QLayoutItem* span = form->itemAt(r, QFormLayout::SpanningRole);
               span != nullptr && span->widget() != nullptr) {
      const auto* box = qobject_cast<const QCheckBox*>(span->widget());
      if (box == nullptr) {
        box = span->widget()->findChild<QCheckBox*>();
      }
      rows << (box != nullptr ? box->text() : QString());
    }
  }
  const qsizetype from = rows.indexOf("Datum");
  REQUIRE(from >= 0);
  // ORT, LÄNGE and BREITE stand together like in his EINGABE box, the UT
  // follows everything that makes it
  CHECK(rows.mid(from, 10) == QStringList{"Datum", "Uhrzeit", "Ortsname", "Länge (Ost +)", "Breite (Nord +)", "Zeit-Zone",
                                          "Sommerzeit", "Wahre Ortszeit (LT)", "Weltzeit (UT)", "Häuser"});
  // the moment of the transits names its clock the same way
  CHECK(rows.mid(rows.indexOf("Transit-Datum"), 2) == QStringList{"Transit-Datum", "Weltzeit (UT)"});
  // the keys walk the fields in the order of the rows
  QList<QWidget*> stops{MainWindowProbe::date(*w)};
  QWidget* at = stops.front();
  while (stops.size() < 24) {
    at = at->nextInFocusChain();
    if (at == stops.front()) {
      break;
    }
    if ((at->focusPolicy() & Qt::TabFocus) == 0 || !at->isVisibleTo(w.get()) || stops.back()->isAncestorOf(at)) {
      continue;
    }
    stops << at;
  }
  const auto stop = [&stops](QWidget* x) { return stops.indexOf(x); };
  CHECK(stop(MainWindowProbe::time(*w)) == 1);
  CHECK(stop(MainWindowProbe::place_field(*w)) == 2);
  CHECK(stop(MainWindowProbe::place_field(*w)) < stop(MainWindowProbe::zone(*w)));
  CHECK(stop(MainWindowProbe::zone(*w)) < stop(MainWindowProbe::sommerzeit(*w)));
  CHECK(stop(MainWindowProbe::sommerzeit(*w)) < stop(MainWindowProbe::local_time_box(*w)));
  CHECK(stop(MainWindowProbe::local_time_box(*w)) < stop(MainWindowProbe::julian(*w)));
  CHECK(stop(MainWindowProbe::julian(*w)) + 1 == stop(MainWindowProbe::ut_time(*w)));
}

TEST_CASE("a place with a zone keeps the UT of a fetched record") {
  const auto dir = std::filesystem::temp_directory_path() / "horcom_panel_place_zone";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir / "places");
  // the picker files carry the step from zone time to UT in the name tail
  REQUIRE(write_place_file(dir / "places" / "ZONE.INT", {{-73.9, 40.6, "Neustadt / US    +5"}}));
  auto w = MainWindowProbe::make();
  // a DAT record holds the UT and no zone
  MainWindowProbe::apply(*w, sample(29, 5, 1917, 20, 0, "00hE00:00"));
  const double before = MainWindowProbe::panel_jd(*w);
  MainWindowProbe::set_data_dir(*w, dir);
  {
    test::DialogDriver drive;
    drive.then([](QDialog* d) {
      auto* table = d->findChild<QTableWidget*>();
      for (int r = 0; table != nullptr && r < table->rowCount(); ++r) {
        if (table->item(r, 0) != nullptr && table->item(r, 0)->text().startsWith("Neustadt")) {
          emit table->cellDoubleClicked(r, 0);
          return;
        }
      }
      d->reject();
    });
    MainWindowProbe::open_place(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::zone(*w)->value() == doctest::Approx(-5.0));
  // the first port kept the clock and the UT jumped to 01:00 of the next day
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - before) < kSecondDays);
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(20, 0));
  CHECK(MainWindowProbe::time(*w)->time() == QTime(15, 0));
  std::filesystem::remove_all(dir);
}

TEST_CASE("a typed Uhrzeit leads and the UT follows a new zone") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(1, 3, 1990, 12, 0, "00hE00:00"));
  MainWindowProbe::time(*w)->setTime(QTime(15, 0));
  MainWindowProbe::zone(*w)->setValue(-5.0);
  CHECK(MainWindowProbe::time(*w)->time() == QTime(15, 0));
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(20, 0));
  // the summer hour rides on the zone, the clock stays as typed
  MainWindowProbe::sommerzeit(*w)->setChecked(true);
  CHECK(MainWindowProbe::time(*w)->time() == QTime(15, 0));
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(19, 0));
}

TEST_CASE("a fetched or typed UT leads and the Uhrzeit follows a new zone") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(29, 5, 1917, 20, 0, "00hE00:00"));
  MainWindowProbe::zone(*w)->setValue(-5.0);
  CHECK(MainWindowProbe::time(*w)->time() == QTime(15, 0));
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(20, 0));
  // a typed UT moves the Uhrzeit through the zone in force
  MainWindowProbe::ut_time(*w)->setTime(QTime(21, 30));
  CHECK(MainWindowProbe::time(*w)->time() == QTime(16, 30));
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - julian_day({29, 5, 1917, 21.0, 30.0}, Calendar::kAuto)) < kSecondDays);
  // and keeps leading when the zone changes again
  MainWindowProbe::zone(*w)->setValue(10.0);
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(21, 30));
  CHECK(MainWindowProbe::time(*w)->time() == QTime(7, 30));
  CHECK(MainWindowProbe::date(*w)->text() == "30.05.1917");
  CHECK(MainWindowProbe::ut_day(*w) == "29.05.1917");
}

TEST_CASE("Zurück takes a typed UT back with its Uhrzeit") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(1, 3, 1990, 12, 0, "01hE00:00"));
  MainWindowProbe::flush_history(*w);
  MainWindowProbe::ut_time(*w)->setTime(QTime(14, 15));
  CHECK(MainWindowProbe::time(*w)->time() == QTime(15, 15));
  MainWindowProbe::history_back(*w);
  CHECK(MainWindowProbe::time(*w)->time() == QTime(12, 0));
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(11, 0));
}

TEST_CASE("Wahre Ortszeit on a UT record keeps the UT, a typed clock reckons it") {
  auto w = MainWindowProbe::make();
  // a synthetic historic UT record, before 1810 the clock is true local time
  MainWindowProbe::apply(*w, sample(28, 8, 1749, 11, 25, "00hE00:00"));
  const double before = MainWindowProbe::panel_jd(*w);
  {
    test::DialogDriver drive;
    //RR WAHRE Ortszeit wird in MITTLERE Ortszeit umgerechnet !
    drive.then(test::DialogDriver::click("OK"));
    MainWindowProbe::local_time_box(*w)->setChecked(true);
    CHECK(drive.pending() == 0);
  }
  CHECK(MainWindowProbe::clock(*w) == ClockKind::kTrueLocal);
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - before) < kSecondDays);
  // the clock now reads the true solar time of the place, about 46 minutes later
  CHECK(MainWindowProbe::time(*w)->time() > QTime(12, 5));
  // a clock typed under the box gives its UT
  MainWindowProbe::time(*w)->setTime(QTime(12, 0));
  const double local = julian_day({28, 8, 1749, 12.0, 0.0}, Calendar::kAuto);
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) -
                 ut_from_local_clock(local, MainWindowProbe::lon(*w), ClockKind::kTrueLocal)) < kSecondDays);
}

TEST_CASE("a typed UT at midnight under a local clock stays on its day") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(1, 3, 1900, 12, 0, "00hE00:00"));
  MainWindowProbe::local_time(*w, true);
  REQUIRE(MainWindowProbe::clock(*w) == ClockKind::kMeanLocal);
  // 11 35 7 east is 46 minutes 20.47 seconds of clock, the whole seconds
  // of the clock read back half a second before midnight
  MainWindowProbe::lon_box(*w)->setValue(11.0 + 35.0 / 60.0 + 7.0 / 3600.0);
  MainWindowProbe::ut_time(*w)->setTime(QTime(0, 0));
  // the clamp had shown 23:59:59 of the day before and the next key
  // landed a day early
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(0, 0));
  CHECK(MainWindowProbe::ut_day(*w).isEmpty());
  CHECK(MainWindowProbe::time(*w)->time() == QTime(0, 46, 20));
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - julian_day({1, 3, 1900, 0.0, 0.0}, Calendar::kAuto)) < kSecondDays);
}

TEST_CASE("a typed UT under the true local clock reads back to the second") {
  auto w = MainWindowProbe::make();
  // before 1810 the local clock is true solar time
  MainWindowProbe::apply(*w, sample(28, 8, 1749, 11, 25, "00hE00:00"));
  MainWindowProbe::local_time(*w, true);
  REQUIRE(MainWindowProbe::clock(*w) == ClockKind::kTrueLocal);
  // the inverse read the equation of time at the mean clock and the field
  // jumped a second under the typing now and then
  for (int i = 0; i < 40; ++i) {
    const QTime typed(9 + i % 7, (i * 7) % 60, (i * 13 + 1) % 60);
    MainWindowProbe::ut_time(*w)->setTime(typed);
    INFO(typed.toString().toStdString());
    CHECK(MainWindowProbe::ut_time(*w)->time() == typed);
  }
}

TEST_CASE("NOCH JULIANISCH reads the typed clock, the UT follows a new place") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(1, 6, 1700, 12, 0, "00hE00:00"));
  {
    test::DialogDriver drive;
    //RR WAHRE Ortszeit wird in MITTLERE Ortszeit umgerechnet !
    drive.then(test::DialogDriver::click("OK"));
    MainWindowProbe::julian(*w)->setChecked(true);
    CHECK(drive.pending() == 0);
  }
  const QTime clock = MainWindowProbe::time(*w)->time();
  const double before = MainWindowProbe::panel_jd(*w);
  // a place a degree further east keeps the clock and moves the UT by
  // four minutes, the box spoke for the typed clock
  MainWindowProbe::lon_box(*w)->setValue(MainWindowProbe::lon(*w) + 1.0);
  CHECK(MainWindowProbe::time(*w)->time() == clock);
  CHECK((before - MainWindowProbe::panel_jd(*w)) * kSecondsPerDay == doctest::Approx(240.0).epsilon(0.01));
}

TEST_CASE("the empty start shows no UT of the chart before") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample(29, 5, 1917, 22, 30, "05hW00:00"));
  REQUIRE_FALSE(MainWindowProbe::ut_day(*w).isEmpty());
  w->start_idle();
  CHECK(MainWindowProbe::ut_time(*w)->time() == QTime(0, 0));
  CHECK(MainWindowProbe::ut_day(*w).isEmpty());
  CHECK_FALSE(MainWindowProbe::ut_time(*w)->isEnabled());
}
