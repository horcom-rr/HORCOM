// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QComboBox>
#include <QKeyEvent>
#include <QLabel>
#include <cmath>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "probe.hpp"
#include "transit_list_dialog.hpp"
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

// the boxes after the dates that depend on the settings, his black
// moon question and the event place of ereig_ort
void settings_steps(DialogDriver& drive, const ChartSettings& s) {
  if (s.true_apogee && !s.heliocentric && s.extra_bodies && s.nk[1] > 0) {
    drive.then(DialogDriver::click("NEIN"));
  }
  if (!s.heliocentric && s.topocentric_parallax) {
    drive.then(DialogDriver::click("JA"));
  }
}

void press(QDialog* d, int key) {
  QKeyEvent e(QEvent::KeyPress, key, Qt::NoModifier);
  QApplication::sendEvent(d, &e);
}

// every label of a box joined, his alert lines
QString labels(QDialog* d) {
  QStringList out;
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    out << l->text();
  }
  return out.join(QChar(0x0A));
}

// the outer glyphs of a transit ring on a wheel list, the running sky
// rides beyond the sign band at the a20 scale
QStringList outer_glyphs(const DisplayList& dl) {
  // the columns and the grid of the walk screen stand left of the rule
  // bes1 draws at 220, the ring lies right of it
  constexpr double kColumnRule = 220.0;
  QStringList out;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kGlyph && p.x1 > kColumnRule &&
        std::hypot(p.x1 - kWheelCenterX, p.y1 - kWheelCenterY) > kTransitWheelScale * 190.0) {
      out << QString::fromStdString(p.text);
    }
  }
  return out;
}

// reads the result table and closes it
DialogDriver::Step read_rows(QStringList& rows, QStringList& heading) {
  return [&rows, &heading](QDialog* d) {
    if (auto* list = qobject_cast<TransitListDialog*>(d)) {
      for (int r = 0; r < list->row_count(); ++r) {
        rows << list->row_texts(r).join("|");
      }
      for (const QLabel* l : list->findChildren<QLabel*>()) {
        heading << l->text();
      }
    }
    d->reject();
  };
}

}  // namespace

TEST_CASE("TRANSIT-LISTE walks his a18eing boxes and lists the sun over radix venus") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  QStringList rows;
  QStringList heading;
  DialogDriver drive;
  drive.then(DialogDriver::click("30° => TEILER"))            // GRUND-ASPEKT WÄHLEN!
      .then(DialogDriver::click("ALLE"))                       // LAUFENDE FAKTOREN
      .then(DialogDriver::fill({"", "1", "11", "1992"}, "OK"))  // BEGINN - DATUM
      .then(DialogDriver::fill({"", "30", "11", "1992"}, "OK")) // ENDE-DATUM
      .then(DialogDriver::click("NEIN"));                      // MOND BERÜCKSICHTIGEN ?
  settings_steps(drive, MainWindowProbe::settings(*w));
  drive.then(DialogDriver::click("SORTIEREN")).then(read_rows(rows, heading));  // LISTE SORTIEREN ?
  MainWindowProbe::transit_list(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  REQUIRE(!rows.isEmpty());
  bool sun_venus = false;
  for (const QString& row : rows) {
    const QStringList f = row.split('|');
    REQUIRE(f.size() == 5);
    // no moon without MOND BERÜCKSICHTIGEN
    CHECK(!f[2].startsWith("MO"));
    if (f[2] == "SO" && f[3].trimmed() == "0" && f[4] == "VE") {
      sun_venus = f[0].endsWith(".11.1992");
    }
  }
  CHECK(sun_venus);
  CHECK(heading.join(" ").contains("TRANSITE"));
  // with the parallax the event place of ereig_ort stands under the table
  if (MainWindowProbe::settings(*w).topocentric_parallax) {
    CHECK(heading.join(" ").contains("Ereignis-Ort"));
  }
}

TEST_CASE("MUNDAN-ASPEKTE draws his linear graph without a record") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  QStringList texts;
  DialogDriver drive;
  drive.then(DialogDriver::click("LINEAR-GRAPHIK"))       // Ausgabe-Modus ? of mund
      .then(DialogDriver::click("90° => TEILER"))         // GRUND-ASPEKT WÄHLEN!
      .then(DialogDriver::click("ALLE"))                  // LAUFENDE FAKTOREN
      .then(DialogDriver::fill({"", "1", "3", "2023"}, "OK"))  // BEGINN - DATUM
      .then(DialogDriver::click("1 MONAT"))               // ZEIT-INTERVALL ?
      .then(DialogDriver::click("'TREFFER - LINIEN' MARKIEREN ?"))
      .then(DialogDriver::click("MIT ZEICHEN"))
      .then(DialogDriver::click(" NEIN "))                // MOND BERÜCKSICHTIGEN ?
      .then(DialogDriver::click("FEIN"))                  // FEINE oder DICKERE Linien
      .then([&texts](QDialog* d) {
        if (const auto* c = d->findChild<WheelWidget*>()) {
          for (const Primitive& p : c->display_list().items) {
            if (p.kind == Primitive::Kind::kText) {
              texts << QString::fromStdString(p.text);
            }
          }
        }
        d->reject();
      });
  MainWindowProbe::mundane_aspects(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(texts.contains(" Ekliptikale Mundan-Aspekte "));
  CHECK(texts.contains("GRUNDWINKEL"));
  CHECK(texts.contains("Jahr"));
  // a18kopf writes no name line under mund!
  for (const QString& t : texts) {
    CHECK_FALSE(t.startsWith("Name :"));
  }
}

TEST_CASE("MUNDAN-ASPEKTE lists the new moon of March 2023") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  QStringList rows;
  QStringList heading;
  DialogDriver drive;
  drive.then(DialogDriver::click("TABELLE"))  // Ausgabe-Modus ? of mund
      .then(DialogDriver::click("KONJUNKTION"))
      .then(DialogDriver::click("ALLE"))
      .then(DialogDriver::fill({"", "20", "3", "2023"}, "OK"))
      .then(DialogDriver::fill({"", "22", "3", "2023"}, "OK"))
      .then(DialogDriver::click("JA"));  // MOND BERÜCKSICHTIGEN ?
  settings_steps(drive, MainWindowProbe::settings(*w));
  drive.then(DialogDriver::click("SORTIEREN")).then(read_rows(rows, heading));  // LISTE SORTIEREN ?
  MainWindowProbe::mundane_aspects(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  bool new_moon = false;
  for (const QString& row : rows) {
    const QStringList f = row.split('|');
    REQUIRE(f.size() == 6);
    if (f[0] == "21.03.2023" && f[2] == "SO" && f[4] == "MO") {
      new_moon = true;
      // the sun leads the pair, so the clock shows minutes
      CHECK(f[1].contains(':'));
      CHECK(f[5].contains("AR"));
    }
  }
  CHECK(new_moon);
  CHECK(heading.join(" ").contains("Mundan"));
}

TEST_CASE("the BEZUGS-SYSTEM of VORGABEN HOROSKOP switches the mundane frame") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  REQUIRE_FALSE(MainWindowProbe::mundane_frame(*w));
  DialogDriver drive;
  drive.then(DialogDriver::click("Bezugs-System"))  // GEWÜNSCHTES THEMA ANKLICKEN !
      .then(DialogDriver::click("MUNDAN"))          // BEZUGS-SYSTEM ?
      .then(DialogDriver::click("EXIT"));           // BILDSCHIRME SPEICHERN ?
  MainWindowProbe::vorgaben_horoskop(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(MainWindowProbe::mundane_frame(*w));
  CHECK(MainWindowProbe::konsta(*w).horm == 2);
}

TEST_CASE("VORGABEN DIREKTIONEN walks his topics from the picked one on") {
  auto w = MainWindowProbe::make();
  Konsta& k = MainWindowProbe::konsta(*w);
  k.zwhd = false;
  k.kard = false;
  k.bres = 1;
  k.brep = 1;
  k.halbs_dir = 0;
  DialogDriver drive;
  drive.then(DialogDriver::click("ZWISCHENHÄUSERN"))  // GEWÜNSCHTES THEMA ANKLICKEN !
      .then(DialogDriver::click("MIT"))                 // DIREKTIONEN MIT/OHNE ZWISCHENHÄUSER ?
      .then(DialogDriver::click("MIT"))                 // MIT KARDINAL-PUNKTEN ?
      .then(DialogDriver::click("OHNE"))                // SIGNIFIKAT. MIT/OHNE Breite ?
      // PROMISSOREN mit Breite stays silent without the significator
      .then(DialogDriver::click("ALLE ANZEIGEN"))       // HALBSUMMEN in TABELLEN ANZEIGEN ?
      .then(DialogDriver::click("EXIT"));               // ORDINATEN-RICHTUNG
  MainWindowProbe::vorgaben_direktionen(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(k.zwhd);
  CHECK(k.kard);
  CHECK(k.bres == 0);
  CHECK(k.brep == 0);
  CHECK(k.halbs_dir == 1);
}

TEST_CASE("TRANSITE HOROSKOP-GRAPHIK asks the running factors and walks the ring like zeitwi") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  QStringList factor_titles;
  QStringList first;
  QString direction;
  DialogDriver drive;
  drive.then(DialogDriver::click("HOROSKOP-GRAPHIK"))  // Ausgabe-Modus ?
      .then([&factor_titles](QDialog* d) {             // a18eing_plw
        factor_titles << d->windowTitle();
        DialogDriver::click("Ab JUPITER AUFWÄRTS")(d);
      })
      .then(DialogDriver::click("JA"))                                        // EREIGNIS-Ort = GEBURTS-Ort ?
      .then(DialogDriver::fill({"", "1", "1", "2020", "12", "0", "0"}, "OK"))  // DATUM-ZEIT-EINGABE
      .then(DialogDriver::click("TAGE"))                                      // ZEIT-EINHEIT ?
      .then(DialogDriver::fill({"10"}, "OK"))
      .then(DialogDriver::click("VOR"))                                       // RICHTUNG ?
      .then([](QDialog* d) { press(d, Qt::Key_Space); })                      // his eingalp sheet
      .then([&](QDialog* d) {
        // the first step runs as the screen opens
        QApplication::processEvents();
        if (const auto* c = d->findChild<WheelWidget*>()) {
          first = outer_glyphs(c->display_list());
        }
        for (const QLabel* l : d->findChildren<QLabel*>()) {
          if (l->text().startsWith("Vorwärts") || l->text().startsWith("Rückwärts")) {
            direction = l->text();
          }
        }
        press(d, Qt::Key_Escape);
      })
      .then(DialogDriver::click(" NEIN = ENDE "));  // Weiter Mit NEUER ZEITEINHEIT ?
  MainWindowProbe::transite(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(factor_titles == QStringList{" LAUFENDE FAKTOREN AUSWÄHLEN ? "});
  // pl1& = 6, the running ring starts at Jupiter
  CHECK_FALSE(first.contains("☉"));
  CHECK_FALSE(first.contains("♂"));
  CHECK(first.contains("♃"));
  CHECK(direction == "Vorwärts !");
  // one step of ten days, the view keeps the moment of the walk
  CHECK(MainWindowProbe::transit_on(*w));
  CHECK(MainWindowProbe::transit_date(*w) == QDate(2020, 1, 11));
  // after the walk the panel ring carries every running factor again
  CHECK(outer_glyphs(MainWindowProbe::wheel(*w)).contains("☉"));
}

TEST_CASE("the event place of the a18 tables carries the parallax lines of ereig_ort") {
  auto w = MainWindowProbe::make();
  w->preset_chart(birth(), true, MainWindowProbe::settings(*w).true_node);
  const ChartSettings s = MainWindowProbe::settings(*w);
  REQUIRE(s.topocentric_parallax);
  REQUIRE_FALSE(s.heliocentric);
  QString transit_box;
  QString mundane_box;
  const auto capture = [](QString& into) {
    return [&into](QDialog* d) {
      into = labels(d);
      DialogDriver::click("JA")(d);
    };
  };
  const bool black_moon = s.true_apogee && s.extra_bodies && s.nk[1] > 0;
  {
    QStringList rows;
    QStringList heading;
    DialogDriver drive;
    drive.then(DialogDriver::click("30° => TEILER"))
        .then(DialogDriver::click("ALLE"))
        .then(DialogDriver::fill({"", "1", "11", "1992"}, "OK"))
        .then(DialogDriver::fill({"", "30", "11", "1992"}, "OK"))
        .then(DialogDriver::click("NEIN"));  // MOND BERÜCKSICHTIGEN ?
    if (black_moon) {
      drive.then(DialogDriver::click("NEIN"));
    }
    drive.then(capture(transit_box)).then(DialogDriver::click("SORTIEREN")).then(read_rows(rows, heading));
    MainWindowProbe::transit_list(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  {
    QStringList rows;
    QStringList heading;
    DialogDriver drive;
    drive.then(DialogDriver::click("TABELLE"))
        .then(DialogDriver::click("KONJUNKTION"))
        .then(DialogDriver::click("ALLE"))
        .then(DialogDriver::fill({"", "20", "3", "2023"}, "OK"))
        .then(DialogDriver::fill({"", "22", "3", "2023"}, "OK"))
        .then(DialogDriver::click("JA"));  // MOND BERÜCKSICHTIGEN ?
    if (black_moon) {
      drive.then(DialogDriver::click("NEIN"));
    }
    drive.then(capture(mundane_box)).then(DialogDriver::click("SORTIEREN")).then(read_rows(rows, heading));
    MainWindowProbe::mundane_aspects(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(transit_box.contains("PARALLAXE evtl. AUSSCHALTEN,da AC und MC NICHT DEM GEBURTSORT ENTSPRECHEN !"));
  CHECK(transit_box.contains("Oder den GEBURTSORT wählen !"));
  CHECK(transit_box.contains("EREIGNIS-Ort = GEBURTS-Ort ?"));
  CHECK(mundane_box.contains("EREIGNISORT ? ( Wegen PARALLAXE )"));
  CHECK_FALSE(mundane_box.contains("PARALLAXE evtl."));
}
