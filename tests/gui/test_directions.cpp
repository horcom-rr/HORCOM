// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QAbstractButton>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <cmath>

#include "dialog_driver.hpp"
#include "direction_list_dialog.hpp"
#include "doctest.h"
#include "probe.hpp"
#include "wheel_widget.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a synthetic birth east of Greenwich, no real person
AafRecord birth() {
  AafRecord r;
  r.surname = "TESTFALL";
  r.day = 13;
  r.month = 10;
  r.year = 1992;
  r.hour = 3;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 19;
  return r;
}

struct Table {
  QStringList rows;
  QStringList labels;
};

// reads the direction table and closes it
DialogDriver::Step read_table(Table& out) {
  return [&out](QDialog* d) {
    if (auto* list = qobject_cast<DirectionListDialog*>(d)) {
      for (int r = 0; r < list->row_count(); ++r) {
        out.rows << list->row_texts(r).join("|");
      }
      for (const QLabel* l : list->findChildren<QLabel*>()) {
        out.labels << l->text();
      }
    }
    d->reject();
  };
}

}  // namespace

TEST_CASE("SYMB. DIREKTION: EKLIPT. asks his boxes and lists dated arcs") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  Table t;
  DialogDriver drive;
  drive.then(DialogDriver::click("DATUM"))                // FORMAT der AUSGABE ?
      .then(DialogDriver::click("90° => TEILER"))         // GRUND-ASPEKT WÄHLEN!
      .then(DialogDriver::click("1JAHR/GRAD"))            // SCHLÜSSEL-ZAHL
      .then(DialogDriver::click("ALLE"))                  // PLANETEN AUSWÄHLEN ?
      .then(DialogDriver::fill({"0"}, "OK"))              // BEGINN der ZÄHLUNG
      .then(DialogDriver::fill({"60"}, "OK"))             // ENDE der ZÄHLUNG
      .then(DialogDriver::click("SORTIEREN"))  // LISTE SORTIEREN ?
      .then(read_table(t));
  MainWindowProbe::symbolic_direction(*w, false);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  REQUIRE(!t.rows.isEmpty());
  for (const QString& row : t.rows) {
    const QStringList f = row.split('|');
    REQUIRE(f.size() == 5);
    // a date between birth and the sixty first year
    const int year = f[0].right(4).toInt();
    CHECK(year >= 1992);
    CHECK(year <= 1992 + 62);
    CHECK((f[4] == "D" || f[4] == "K"));
  }
  CHECK(t.labels.join(" ").contains("Eklipt."));
}

TEST_CASE("PRIMÄR-DIREKTION lists life years and months with his latitude note") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  Table t;
  DialogDriver drive;
  drive.then(DialogDriver::click("LEBENSJAHR/MONAT"))
      .then(DialogDriver::click("90° => TEILER"))
      .then(DialogDriver::click("NAIBOD"))
      .then(DialogDriver::click("ALLE"))
      .then(DialogDriver::fill({"20"}, "OK"))
      .then(DialogDriver::fill({"40"}, "OK"))
      .then(DialogDriver::click("SORTIEREN"))  // LISTE SORTIEREN ?
      .then(read_table(t));
  MainWindowProbe::primary_direction(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  REQUIRE(!t.rows.isEmpty());
  int last_lj = 0;
  for (const QString& row : t.rows) {
    // LJ between twenty and forty one, his lje carries one year more
    const int lj = row.split('|')[0].trimmed().split(' ')[0].toInt();
    CHECK(lj >= 20);
    CHECK(lj <= 41);
    // dirend asked LISTE SORTIEREN ? before the list, ZEITLICH orders it
    CHECK(lj >= last_lj);
    last_lj = lj;
  }
  CHECK(t.labels.join(" ").contains("Breite"));
}

TEST_CASE("SEKUNDÄR-DIREKTION runs his Ausgabe-Modus and MOND questions") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const ChartSettings s = MainWindowProbe::settings(*w);
  Table t;
  DialogDriver drive;
  drive.then(DialogDriver::click("TABELLE"))
      .then(DialogDriver::click("DATUM"))
      .then(DialogDriver::click("90° => TEILER"))
      .then(DialogDriver::click("ALLE"))
      .then(DialogDriver::fill({"0"}, "OK"))
      .then(DialogDriver::fill({"30"}, "OK"))
      .then(DialogDriver::click("NEIN"));  // MOND BERÜCKSICHTIGEN ?
  if (s.true_apogee && s.extra_bodies && s.nk[1] > 0) {
    drive.then(DialogDriver::click("NEIN"));
  }
  if (s.topocentric_parallax) {
    drive.then(DialogDriver::click("JA"));
  }
  drive.then(DialogDriver::click("SORTIEREN")).then(read_table(t));  // LISTE SORTIEREN ?
  MainWindowProbe::secondary_direction(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  REQUIRE(!t.rows.isEmpty());
  for (const QString& row : t.rows) {
    // no progressed moon without MOND BERÜCKSICHTIGEN
    CHECK(!row.split('|')[1].startsWith("MO"));
  }
  CHECK(t.labels.join(" ").contains("SEKUNDÄR"));
}

TEST_CASE("SONNEN-BOGEN lists the arc directions and draws its wheel") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  Table t;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("SONNE"))
        .then(DialogDriver::click("TABELLE"))
        .then(DialogDriver::click("DATUM"))
        .then(DialogDriver::click("90° => TEILER"))
        .then(DialogDriver::click("ALLE"))
        .then(DialogDriver::fill({"0"}, "OK"))
        .then(DialogDriver::fill({"30"}, "OK"))
        .then(DialogDriver::click("SORTIEREN"))  // LISTE SORTIEREN ?
      .then(read_table(t));
    MainWindowProbe::arc_direction(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(!t.rows.isEmpty());
  CHECK(t.labels.join(" ").contains("SONNEN-BOGEN"));

  // the HOROSKOP-GRAPHIK branch of a20_horg asks the running factors and
  // puts the directed ring on the wheel, ESC at ZEIT-EINHEIT leaves it
  // standing at the SUCH-DATUM
  DialogDriver wheel;
  wheel.then(DialogDriver::click("SONNE"))
      .then(DialogDriver::click("HOROSKOP-GRAPHIK"))
      .then(DialogDriver::click("ALLE"))  // LAUFENDE FAKTOREN AUSWÄHLEN ?
      .then(DialogDriver::fill({"", "1", "1", "2022"}, "OK"))
      .then([](QDialog* d) { d->reject(); });  // ZEIT-EINHEIT ?
  MainWindowProbe::arc_direction(*w);
  CHECK(wheel.pending() == 0);
  CHECK(wheel.unexpected() == 0);
  CHECK(MainWindowProbe::arc_on(*w));
  bool label = false;
  bool prog = false;
  for (const Primitive& p : MainWindowProbe::wheel(*w).items) {
    if (p.kind == Primitive::Kind::kText) {
      // a$ + "=>" + q$ and the PROG: date of zeitwi
      label = label || p.text == "SOBDIR.=>RADIX";
      prog = prog || p.text.rfind("PROG: 1. 1.2022", 0) == 0;
    }
  }
  CHECK(label);
  CHECK(prog);
}

TEST_CASE("SEKUNDÄR HOROSKOP-GRAPHIK draws the progressed ring of a20_horg and walks symbolic years") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const Chart radix = MainWindowProbe::chart(*w);
  const double tja = radix.ta.tropical_year_days;
  QStringList grey;
  QStringList first;
  DialogDriver drive;
  drive.then(DialogDriver::click("HOROSKOP-GRAPHIK"))              // Ausgabe-Modus ?
      .then(DialogDriver::click("ALLE"))                          // LAUFENDE FAKTOREN AUSWÄHLEN ?
      .then(DialogDriver::fill({"", "13", "10", "2022"}, "OK"))   // SUCH-DATUM eingeben !
      .then([&grey](QDialog* d) {                                 // RECHEN-MODUS ?
        for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
          if (!b->isEnabled()) {
            grey << b->text();
          }
        }
        DialogDriver::click("STRENG PROPORTIONALE")(d);
      })
      .then(DialogDriver::click("SYMBOLISCHE 'JAHRE'"))  // ZEIT-EINHEIT ?
      .then(DialogDriver::fill({"1"}, "OK"))
      .then(DialogDriver::click("VOR"))  // RICHTUNG ?
      .then([](QDialog* d) {
        // his eingalp sheet with the p! legend
        QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(d, &space);
      })
      .then([&first](QDialog* d) {
        QApplication::processEvents();
        if (const auto* c = d->findChild<WheelWidget*>()) {
          for (const Primitive& p : c->display_list().items) {
            if (p.kind == Primitive::Kind::kText) {
              first << QString::fromStdString(p.text);
            }
          }
        }
        QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QApplication::sendEvent(d, &esc);
      })
      .then(DialogDriver::click(" NEIN = ENDE "));
  MainWindowProbe::secondary_direction(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  // prog_mode leaves the first two rows blank and grey
  CHECK(grey == QStringList{"   ", "   "});
  CHECK(first.contains("SECDIR.=>RADIX"));
  // the SUCH-DATUM of the thirtieth birthday one symbolic year on
  bool prog = false;
  for (const QString& t : first) {
    prog = prog || t.startsWith("PROG:13.10.2023");
  }
  CHECK(prog);
  CHECK(MainWindowProbe::arc_on(*w));
  // jd = jd1 + da with da = (jd2 - jd1) / tja, the step adds one day
  const double jd2 = julian_day({13, 10, 2022, 3.0, 0.0});
  const double expected = radix.jd_ut + (jd2 - radix.jd_ut) / tja + 1.0;
  CHECK(std::abs(MainWindowProbe::arc_prog_jd(*w) - expected) < 1.0e-8);
}

TEST_CASE("SYMB. DIREKTION MUNDAN asks no factor box like a18eing_plw under mars!") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  Table t;
  DialogDriver drive;
  drive.then(DialogDriver::click("MUNDAN"))         // 'MUNDAN'-GEOMETRIE ?
      .then(DialogDriver::click("DATUM"))           // FORMAT der AUSGABE ?
      .then(DialogDriver::click("90° => TEILER"))   // GRUND-ASPEKT WÄHLEN!
      .then(DialogDriver::click("1JAHR/GRAD"))      // SCHLÜSSEL-ZAHL
      .then(DialogDriver::fill({"0"}, "OK"))        // BEGINN der ZÄHLUNG
      .then(DialogDriver::fill({"60"}, "OK"))       // ENDE der ZÄHLUNG
      .then(DialogDriver::click("SORTIEREN"))  // LISTE SORTIEREN ?
      .then(read_table(t));
  MainWindowProbe::symbolic_direction(*w, true);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK_FALSE(drive.titles().contains(" PLANETEN AUSWÄHLEN ? "));
  CHECK(t.labels.join(" ").contains("'MUNDANE' SYMB."));
  CHECK(t.labels.join(" ").contains("Schlüssel:1 A/°"));
}

TEST_CASE("the linear windows step by whole months and spans like a18asw1") {
  // his step back of one month went to jdbeg - 31 and kept that month,
  // from the first of March 2001 it landed on the first of January. The
  // default Approx of a julian day this size lets half a day through, the
  // checks hold a millisecond
  constexpr double kMs = 1.0e-8;
  const double march = julian_day({1, 3, 2001, 0.0, 0.0});
  const auto back = MainWindowProbe::step_dates(march, march + 31.0, false);
  CHECK(std::abs(back.first - julian_day({1, 2, 2001, 0.0, 0.0})) < kMs);
  CHECK(std::abs(back.second - back.first - 31.0) < kMs);
  const auto on = MainWindowProbe::step_dates(march, march + 31.0, true);
  CHECK(std::abs(on.first - julian_day({1, 4, 2001, 0.0, 0.0})) < kMs);
  // four and sixteen months
  const auto four = MainWindowProbe::step_dates(march, march + 125.0, false);
  CHECK(std::abs(four.first - julian_day({1, 11, 2000, 0.0, 0.0})) < kMs);
  const auto sixteen = MainWindowProbe::step_dates(march, march + 488.0, true);
  CHECK(std::abs(sixteen.first - julian_day({1, 7, 2002, 0.0, 0.0})) < kMs);
  // ho = 0 and mi = 0, a window of the hour astrology steps to midnight,
  // the port once opened every other window at noon
  const double at_three = julian_day({1, 3, 2001, 15.0, 0.0});
  const auto midnight = MainWindowProbe::step_dates(at_three, at_three + 31.0, true);
  CHECK(std::abs(midnight.first - julian_day({1, 4, 2001, 0.0, 0.0})) < kMs);
  // lja = lje - 2 * dlj
  const auto years = MainWindowProbe::step_years(20.0, 40.0, false);
  CHECK(std::abs(years.first) < kMs);
  CHECK(std::abs(years.second - 20.0) < kMs);
  const auto next = MainWindowProbe::step_years(20.0, 40.0, true);
  CHECK(std::abs(next.first - 40.0) < kMs);
  CHECK(std::abs(next.second - 60.0) < kMs);
}

TEST_CASE("SYMB. DIREKTION wants two factors and names its key") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  Table t;
  QString warning;
  DialogDriver drive;
  drive.then(DialogDriver::click("DATUM"))
      .then(DialogDriver::click("90° => TEILER"))
      .then(DialogDriver::click("1JAHR/GRAD"))
      .then(DialogDriver::click("NUR DIESE"))  // EINZELNE PLANETEN WÄHLEN und NUR DIESE DARSTELLEN
      .then([](QDialog* d) {
        if (auto* list = d->findChild<QListWidget*>()) {
          list->item(0)->setCheckState(Qt::Checked);
        }
        DialogDriver::click("OK")(d);
      })
      .then([&warning](QDialog* d) {
        // MINDESTENS 2 FAKTOREN WÄHLEN ! and the box of a18ei2 again
        if (auto* box = qobject_cast<QMessageBox*>(d)) {
          warning = box->text();
        }
        d->accept();
      })
      .then(DialogDriver::click("ALLE"))
      .then(DialogDriver::fill({"0"}, "OK"))
      .then(DialogDriver::fill({"60"}, "OK"))
      .then(DialogDriver::click("SORTIEREN"))  // LISTE SORTIEREN ?
      .then(read_table(t));
  MainWindowProbe::symbolic_direction(*w, false);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(warning.contains("MINDESTENS 2 FAKTOREN"));
  // a18kopf names the key of the symbolic and primary runs in every
  // format, the one year key too
  CHECK(t.labels.join(" ").contains("Schlüssel:1 A/°"));
}

TEST_CASE("MOND-BOGEN greys HOROSKOP-GRAPHIK and the directions run geocentric") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  w->show_helio();
  REQUIRE(MainWindowProbe::settings(*w).heliocentric);
  QStringList grey;
  DialogDriver drive;
  drive.then(DialogDriver::click("MOND")).then([&grey](QDialog* d) {
    for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
      if (!b->isEnabled()) {
        grey << b->text();
      }
    }
    d->reject();
  });
  MainWindowProbe::arc_direction(*w);
  CHECK(drive.pending() == 0);
  CHECK(grey == QStringList{"HOROSKOP-GRAPHIK"});
  // CLR hrg! in a19, the helio switch of the panel went off
  CHECK_FALSE(MainWindowProbe::settings(*w).heliocentric);
}

TEST_CASE("the LINEAR-GRAPHIK of the SEKUNDÄR-DIREKTION walks his linear boxes") {
  auto w = MainWindowProbe::make();
  AafRecord r;
  r.surname = "TESTFALL";
  r.day = 13;
  r.month = 10;
  r.year = 1992;
  r.hour = 3;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 19;
  MainWindowProbe::apply(*w, r);
  QStringList disabled;
  QStringList spans;
  QStringList texts;
  QStringList next_texts;
  DialogDriver drive;
  // a17dat asks no FORMAT der AUSGABE for the linear graph
  drive.then(DialogDriver::click("LINEAR-GRAPHIK"))
      .then([&disabled](QDialog* d) {
        for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
          if (!b->isEnabled()) {
            disabled << b->text();
          }
        }
        DialogDriver::click("30°")(d);
      })
      .then([&disabled](QDialog* d) {
        // ROT MARKIEREN stands blank and grey in the linear factor box
        for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
          if (!b->isEnabled()) {
            disabled << b->text();
          }
        }
        DialogDriver::click("ALLE")(d);
      })
      .then(DialogDriver::fill({"20"}, "OK"))
      .then([&spans](QDialog* d) {
        for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
          spans << b->text();
        }
        DialogDriver::click("20 bis 40 JAHRE")(d);
      })
      .then(DialogDriver::click("'TREFFER - LINIEN' MARKIEREN ?"))
      .then(DialogDriver::click("MIT ZEICHEN"))
      .then(DialogDriver::click(" NEIN "))  // MOND BERÜCKSICHTIGEN ?
      .then(DialogDriver::click("FEIN"))
      .then([&texts](QDialog* d) {
        if (const auto* c = d->findChild<WheelWidget*>()) {
          for (const Primitive& p : c->display_list().items) {
            if (p.kind == Primitive::Kind::kText) {
              texts << QString::fromStdString(p.text);
            }
          }
        }
        QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(d, &space);
      })
      .then(DialogDriver::click("NÄCHSTES Intervall"))
      .then([&next_texts](QDialog* d) {
        if (const auto* c = d->findChild<WheelWidget*>()) {
          for (const Primitive& p : c->display_list().items) {
            if (p.kind == Primitive::Kind::kText) {
              next_texts << QString::fromStdString(p.text);
            }
          }
        }
        d->reject();
      });
  MainWindowProbe::secondary_direction_run(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  // the combined grids and the small bases stay grey in the linear box,
  // the blank ROT MARKIEREN row of a18eing_plw too
  CHECK(disabled.size() == 6);
  CHECK(disabled.contains("  "));
  CHECK(spans.contains("20 bis 25 JAHRE"));
  CHECK(spans.contains("20 bis 100 JAHRE"));
  CHECK(texts.contains(" SEKUNDÄR-Direktion "));
  CHECK(texts.contains("GRUNDWINKEL"));
  CHECK(texts.contains("Lebensj."));
  // twenty four pixels a year, the ages 20, 25, 30 and 35 under the axis
  CHECK(texts.contains("25"));
  CHECK(texts.contains("35"));
  // the birthday of the year of life beneath
  CHECK(texts.contains("13.10.2017"));
  CHECK(next_texts.contains("45"));
}
