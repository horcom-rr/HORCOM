// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QCheckBox>
#include <QColorDialog>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QMenuBar>
#include <QListWidget>
#include <QLabel>
#include <QMessageBox>
#include <QTableWidget>

#include "choice_dialog.hpp"
#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/chart/great_year.hpp"
#include "horcom/ephem/eclipses.hpp"
#include "probe.hpp"
#include "theme.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

QStringList first_column(QDialog* d) {
  QStringList out;
  const QTableWidget* table = d->findChild<QTableWidget*>();
  for (int r = 0; table != nullptr && r < table->rowCount(); ++r) {
    out << table->item(r, 0)->text();
  }
  return out;
}

// one column block of the FINSTERNISSE screen, date, marker, position
// and kind joined per row
QStringList finst_block(const QTableWidget* table, int col) {
  QStringList out;
  for (int r = 0; r < table->rowCount(); ++r) {
    QStringList cells;
    for (int c = col; c < col + 4; ++c) {
      const QTableWidgetItem* it = table->item(r, c);
      cells << (it != nullptr ? it->text() : QString());
    }
    out << cells.join("|");
  }
  return out;
}

// the new moon of 1999 August 11, a synthetic birth at that moment, no
// real person
AafRecord eclipse_birth() {
  AafRecord r;
  r.surname = "TESTFALL";
  r.day = 11;
  r.month = 8;
  r.year = 1999;
  r.hour = 11;
  r.minute = 8;
  r.zone = kUtZoneText;
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 19;
  return r;
}

// datum3$ of a lunation, the row prefix of his screen
QString lunation_day(double k) {
  const CalendarDate d = calendar_date(std::floor(lunation_at(k).jd_ut * kMinutesPerDay + 0.5) / kMinutesPerDay);
  return QString::asprintf("%2d.%2d.%d", d.day, d.month, d.year);
}

}  // namespace

TEST_CASE("the house table names the Vehlow axes as houses and adds AC and MC") {
  auto w = MainWindowProbe::make();
  QStringList vehlow;
  QStringList placidus;
  DialogDriver drive;
  drive.then([&vehlow](QDialog* d) {
    vehlow = first_column(d);
    d->reject();
  });
  // his haw& = 7
  MainWindowProbe::houses(*w, 7);
  MainWindowProbe::house_table(*w);
  REQUIRE(vehlow.size() == 15);
  CHECK(vehlow[0] == "H1");
  CHECK(vehlow[3] == "H4");
  CHECK(vehlow[6] == "H7");
  CHECK(vehlow[9] == "H10");
  CHECK(vehlow[12] == "VERTEX");
  CHECK(vehlow[13] == "AC");
  CHECK(vehlow[14] == "MC");
  drive.then([&placidus](QDialog* d) {
    placidus = first_column(d);
    d->reject();
  });
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::house_table(*w);
  REQUIRE(placidus.size() == 13);
  CHECK(placidus[0] == "AC");
  CHECK(placidus[3] == "IC");
  CHECK(placidus[9] == "MC");
}

TEST_CASE("FINSTERNISSE lists the eclipses at their greatest moment and pages like finst") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, eclipse_birth());
  QStringList left;
  QStringList right;
  QStringList labels;
  QString forward_first;
  QString back_first;
  bool eclipse_inverse = false;
  DialogDriver drive;
  drive.then(DialogDriver::click(" NEIN "))                     // ASPEKTE mit GÜLTIGEM DATENSATZ UNTERSUCHEN ?
      .then(DialogDriver::fill({"", "1", "8", "1999"}, "OK")) // SUCH-DATUM EINGEBEN !
      .then([&](QDialog* d) {
        auto* table = d->findChild<QTableWidget*>();
        REQUIRE(table != nullptr);
        left = finst_block(table, 0);
        right = finst_block(table, 4);
        for (const QLabel* l : d->findChildren<QLabel*>()) {
          labels << l->text();
        }
        for (int r = 0; r < table->rowCount(); ++r) {
          if (table->item(r, 3) != nullptr && table->item(r, 3)->text().contains("ZT")) {
            eclipse_inverse = table->item(r, 0)->background().color() == QColor(0, 0, 0);
          }
        }
        // his asc& = 32, k1 = k1 + INT(ABS(zf& - zf& / 5))
        QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(d, &space);
        forward_first = table->item(0, 0)->text();
        // his asc& = 82, k1 = k1 - INT(2 * ABS(zf& - zf& / 5))
        QKeyEvent r(QEvent::KeyPress, Qt::Key_R, Qt::NoModifier, "r");
        QApplication::sendEvent(d, &r);
        back_first = table->item(0, 0)->text();
        d->reject();
      });
  MainWindowProbe::eclipse_table(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  REQUIRE(left.size() == 27);
  REQUIRE(right.size() == 27);
  // k1 of 1 August 1999 is -6, the first new moon is k1 - 1, the first
  // full moon k1 - 1.5
  const double k1 = lunation_number({1, 8, 1999, 0.0, 0.0});
  CHECK(k1 == -6.0);
  CHECK(left[0].startsWith(lunation_day(k1 - 1.0)));
  CHECK(right[0].startsWith(lunation_day(k1 - 1.5)));
  // the sun marker rides on rows 1, 8, 15 and 22 of the lunation rows
  CHECK(left[0].section('|', 1, 1) == "SO");
  CHECK(right[0].section('|', 1, 1) == "MO");
  // the total eclipse of 1999 August 11, conjunction at 11:08:5x UT and the
  // greatest eclipse at 11:03 UT, central, total, north of the axis
  const int nm = static_cast<int>(left.indexOf(QRegularExpression(R"(^11\. 8\.1999    11h   [89]m.*)")));
  REQUIRE(nm >= 0);
  CHECK(left[nm + 1].startsWith("11. 8.1999    11h   3m"));
  CHECK(left[nm + 1].endsWith("| ZT TOT N"));
  CHECK(eclipse_inverse);
  // the partial umbral eclipse of 1999 July 28 peaked at 11:34 UT
  const int fm = static_cast<int>(right.indexOf(QRegularExpression(R"(^28\. 7\.1999    11h  2\dm.*)")));
  REQUIRE(fm >= 0);
  CHECK(right[fm + 1].startsWith("28. 7.1999    11h  34m"));
  CHECK(right[fm + 1].endsWith("| KERNSCH"));
  CHECK(labels.join(QChar(0x0A)).contains("EPHEM:App.1,Ohne Parallaxe"));
  // without aspects every row counts, zf = 27 moves 21 lunations on and
  // then 43 back
  CHECK(forward_first.startsWith(lunation_day(k1 - 1.0 + 21.0)));
  CHECK(back_first.startsWith(lunation_day(k1 - 1.0 + 21.0 - 43.0)));
}

TEST_CASE("FINSTERNISSE lists the aspects of the lights with the active chart like suchas") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, eclipse_birth());
  QStringList left;
  QStringList labels;
  QStringList titles;
  DialogDriver drive;
  drive.then(DialogDriver::click("JA"))
      .then([&titles](QDialog* d) {
        titles << d->windowTitle();
        DialogDriver::click("KONJUNKTION")(d);
      })
      .then([&titles](QDialog* d) {
        titles << d->windowTitle();
        DialogDriver::click("1.0")(d);
      })
      .then(DialogDriver::fill({"", "1", "8", "1999"}, "OK"))
      .then([&](QDialog* d) {
        auto* table = d->findChild<QTableWidget*>();
        REQUIRE(table != nullptr);
        left = finst_block(table, 0);
        for (const QLabel* l : d->findChildren<QLabel*>()) {
          labels << l->text();
        }
        d->reject();
      });
  MainWindowProbe::eclipse_table(*w);
  CHECK(drive.pending() == 0);
  CHECK(titles == QStringList{"GRUND-ASPEKT WÄHLEN!", "ORBIS-FAKTOR ?"});
  REQUIRE(left.size() == 27);
  // the radix stands at the new moon itself, the lunation row and the
  // greatest eclipse both meet the radix sun and moon, e$ = LEFT$(d$,13)
  // keeps the conjunction rows short
  const int nm = static_cast<int>(left.indexOf(QRegularExpression(R"(^11\. 8\.1999    11h   [89]m.*)")));
  REQUIRE(nm >= 0);
  CHECK(left[nm + 1].startsWith("=> SO-SO   0°|"));
  CHECK(left[nm + 2].startsWith("=> SO-MO   0°|"));
  int eclipse = -1;
  for (int r = nm + 1; r < left.size(); ++r) {
    if (left[r].endsWith("| ZT TOT N")) {
      eclipse = r;
      break;
    }
  }
  REQUIRE(eclipse > nm);
  CHECK(left[eclipse + 1].startsWith("=> SO-SO   0°|"));
  // a conjunction scan never names another harmonic
  for (const QString& row : left) {
    if (row.startsWith("=>")) {
      CHECK(row.contains("   0°"));
    }
  }
  CHECK(labels.join(QChar(0x0A)).contains("Mit ASPEKTEN SO - bzw. MO - mit RADIX  TESTFALL"));
}

namespace {

// the labels of a box, his alert lines
QString box_lines(QDialog* d) {
  QStringList out;
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    out << l->text();
  }
  return out.join(QChar(0x0A));
}

// the text of a message box, then close it
DialogDriver::Step read_message(QString& title, QString& text) {
  return [&title, &text](QDialog* d) {
    title = d->windowTitle();
    if (auto* box = qobject_cast<QMessageBox*>(d)) {
      text = box->text();
    }
    d->accept();
  };
}

}  // namespace

TEST_CASE("GROSSES JAHR keeps the CHAUVIN reference and shows the age point like grossj1") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, eclipse_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  // his jdgross=2370832
  //RR CHAUVIN f.AQU.
  k.jdgross = 2370832.0;
  k.zal_grossj = 330;
  QString alert;
  QString title;
  QString text;
  DialogDriver drive;
  drive.then([&alert](QDialog* d) {
         alert = box_lines(d);
         DialogDriver::click(" Beibehalten ")(d);
       })
      .then(read_message(title, text));
  MainWindowProbe::great_year(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(MainWindowProbe::great_year_on(*w));
  CHECK(alert.contains("Bezugsdatum :  6. 1.1779   "));
  // his box said 30° for every age
  CHECK(alert.contains("Zeitalters-Punkt : 330° Wassermann"));
  // the kept setting leaves his d$ empty
  CHECK(title.startsWith(" | RADIX | Datum : 11. 8.1999"));
  const QStringList lines = text.split(QChar(0x0A));
  REQUIRE(lines.size() == 5);
  CHECK(lines[0] == "Ekliptikale Bezugs-Länge = 330° Entspr. Wassermann - Zeitalter");
  CHECK(lines[1] == "Bezugs-Zeitpunkt =  6. 1.1779   ");
  // 220.6 years of Newcomb precession, about 3.08 degrees
  const Chart& c = MainWindowProbe::chart(*w);
  const GreatYearPoint p = great_year_point(c.jd_ut, c.ta.tropical_year_days, c.smo.ekls, 2370832.0, 330);
  CHECK(p.di_deg == doctest::Approx(-3.08).epsilon(0.01));
  CHECK(lines[2] == QString::asprintf("L\u00e4ngen - Differenz zur Bezugs - L\u00e4nge = %8.4f\u00b0", std::abs(p.di_deg)));
  CHECK(lines[3].startsWith("Der 'Zeitalter - Punkt' f\u00fcr das Datum  11. 8.1999"));
  CHECK(lines[4].endsWith(QString::asprintf(" =  %8.4f\u00b0", p.point_deg)));
  // the yellow box of his main screen repeats the lines with the record
  QString banner;
  drive.then([&banner](QDialog* d) {
    if (const auto* box = d->findChild<QLabel*>("greatYearBox")) {
      banner = box->text();
    }
    d->reject();
  });
  MainWindowProbe::vorgaben_overview(*w);
  CHECK(banner.startsWith(lines[0]));
  CHECK(banner.contains(" Datensatz : TESTFALL"));
}

TEST_CASE("GROSSES JAHR changes the age start and the reference and can switch the display off") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, eclipse_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  k.jdgross = 2370832.0;
  k.zal_grossj = 330;
  QStringList titles;
  QString warning_title;
  QString warning;
  QString title;
  QString text;
  DialogDriver drive;
  drive.then(DialogDriver::click(" \u00c4ndern "))
      .then([&titles](QDialog* d) {
        titles << d->windowTitle();
        DialogDriver::click("FISCHE")(d);
      })
      .then([&titles](QDialog* d) {
        titles << d->windowTitle() + "|" + box_lines(d);
        DialogDriver::fill({"", "21", "3", "2000"}, "OK")(d);
      })
      // the record lies before the new reference, the age has not begun
      .then(read_message(warning_title, warning))
      .then(read_message(title, text));
  MainWindowProbe::great_year(*w);
  CHECK(drive.pending() == 0);
  REQUIRE(titles.size() == 2);
  CHECK(titles[0] == "Zeitalter - Start w\u00e4hlen !");
  CHECK(titles[1].startsWith("DATUM-ZEIT-EINGABE|Bezugsdatum Eingeben entsprechend 360\u00b0 = FISCHE"));
  CHECK(k.zal_grossj == 360);
  CHECK(k.jdgross == doctest::Approx(julian_day({21, 3, 2000, 12.0, 0.0})));
  CHECK(warning == "NICHT MEHR im FISCHE - ZEITALTER !");
  CHECK(title.startsWith("F\u00fcr DATENSATZ  :  TESTFALL"));
  CHECK(text.startsWith("Ekliptikale Bezugs-L\u00e4nge = 360\u00b0 Entspr. FISCHE - Zeitalter"));
  // Anzeige abschalten ends the display, the box leaves the overview
  drive.then(DialogDriver::click("Anzeige abschalten"));
  MainWindowProbe::great_year(*w);
  CHECK_FALSE(MainWindowProbe::great_year_on(*w));
  bool box = true;
  drive.then([&box](QDialog* d) {
    box = d->findChild<QLabel*>("greatYearBox") != nullptr;
    d->reject();
  });
  MainWindowProbe::vorgaben_overview(*w);
  CHECK_FALSE(box);
}

namespace {

// his AUFGANG mask, place and date, the clock stays locked
DialogDriver::Step rise_mask(const QString& day, const QString& month, const QString& year, bool& clock_locked) {
  return [day, month, year, &clock_locked](QDialog* d) {
    const QList<QLineEdit*> edits = d->findChildren<QLineEdit*>();
    clock_locked = edits.size() >= 18 && !edits[14]->isEnabled() && !edits[17]->isEnabled() && edits[13]->isEnabled();
    DialogDriver::fill({"AUFGANG.....", "TESTORT", "E", "11", "19", "0", "N", "48", "10", "0", "", day, month, year},
                       "OK")(d);
  };
}

// the three block columns of one screen row
QStringList screen_row(const QTableWidget* table, int row) {
  QStringList out;
  for (int c = 0; c < 3; ++c) {
    const QTableWidgetItem* it = table->item(row, c);
    out << (it != nullptr ? it->text() : QString());
  }
  return out;
}

// minutes of day from his clock text, 3h  13m or 3h 13m 20s
int clock_minutes(const QString& text) {
  const QRegularExpressionMatch m = QRegularExpression(R"((\d+)h\s+(\d+)m)").match(text);
  return m.hasMatch() ? m.captured(1).toInt() * 60 + m.captured(2).toInt() : -1;
}

}  // namespace

TEST_CASE("AUFGANG walks one body day by day like auf_unt") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, eclipse_birth());
  bool clock_locked = false;
  QStringList first;
  QStringList second;
  QStringList paged;
  QStringList back;
  QStringList labels;
  QString title;
  DialogDriver drive;
  drive.then(DialogDriver::click("EINZELNER Planet"))
      .then(DialogDriver::click("SCHEINBAR"))
      .then(rise_mask("21", "6", "2000", clock_locked))
      .then(DialogDriver::pick_row(0))  // EIN OBJEKT AUSWÄHLEN !, SONNE
      .then([&](QDialog* d) {
        title = d->windowTitle();
        auto* table = d->findChild<QTableWidget*>();
        REQUIRE(table != nullptr);
        first = screen_row(table, 0);
        second = screen_row(table, 6);
        labels = QStringList();
        for (const QLabel* l : d->findChildren<QLabel*>()) {
          labels << l->text();
        }
        QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(d, &space);
        paged = screen_row(table, 0);
        QKeyEvent r(QEvent::KeyPress, Qt::Key_R, Qt::NoModifier, "r");
        QApplication::sendEvent(d, &r);
        back = screen_row(table, 0);
        d->reject();
      });
  MainWindowProbe::rise_set(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(clock_locked);
  CHECK(title.startsWith("* AUFGANG"));
  REQUIRE(first.size() == 3);
  // the almanac for eleven degrees east and forty eight north, sunrise
  // near 3:13, noon near 11:15 and sunset near 19:17 UT
  CHECK(first[0].startsWith("21. 6.2000 | "));
  CHECK(std::abs(clock_minutes(first[0]) - (3 * 60 + 13)) <= 2);
  CHECK(std::abs(clock_minutes(first[1]) - (11 * 60 + 15)) <= 2);
  CHECK(std::abs(clock_minutes(first[2]) - (19 * 60 + 17)) <= 2);
  // SCHEINBAR prints whole minutes like ze1$
  CHECK_FALSE(first[0].contains("s"));
  CHECK(second[0].startsWith("22. 6.2000 | "));
  CHECK(paged[0].startsWith("26. 6.2000 | "));
  CHECK(back[0].startsWith("21. 6.2000 | "));
  const QString all = labels.join(QChar(0x0A));
  CHECK(all.contains(" Scheinbare Werte  | Zeiten in UT ( GMT )"));
  CHECK(all.contains("TESTORT | Geog. Länge : 11° 19.0' E | Geog. Breite: 48° 10.0' N"));
}

TEST_CASE("AUFGANG lists all real bodies at one date five to a screen") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, eclipse_birth());
  bool clock_locked = false;
  QString sun_len;
  QString sun_clock;
  QString moon_len;
  QString moon_clock;
  QString jupiter_len;
  DialogDriver drive;
  drive.then(DialogDriver::click("Für ALLE"))
      .then(DialogDriver::click("WAHR"))
      .then(rise_mask("21", "6", "2000", clock_locked))
      .then([&](QDialog* d) {
        auto* table = d->findChild<QTableWidget*>();
        REQUIRE(table != nullptr);
        sun_clock = screen_row(table, 0)[0];
        sun_len = screen_row(table, 2)[0];
        moon_clock = screen_row(table, 6)[0];
        moon_len = screen_row(table, 8)[0];
        QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(d, &space);
        jupiter_len = screen_row(table, 2)[0];
        d->reject();
      });
  MainWindowProbe::rise_set(*w);
  CHECK(drive.pending() == 0);
  CHECK(sun_len.startsWith("Länge   = SO"));
  CHECK(moon_len.startsWith("Länge   = MO"));
  // WAHR prints seconds like ze$, the moon keeps whole minutes
  CHECK(QRegularExpression(R"(\d+h\s+\d+m\s+\d+s)").match(sun_clock).hasMatch());
  CHECK_FALSE(moon_clock.contains("s"));
  CHECK(jupiter_len.startsWith("Länge   = JU"));
}

TEST_CASE("AUFGANG checks the latitude against the obliquity of the entered date like juld") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, eclipse_birth());
  QString refused;
  DialogDriver drive;
  // 66 degrees 32 minutes north, inside the limit of 1999 and beyond the
  // one of the year 1600 whose obliquity stood some 0.05 degrees larger
  drive.then(DialogDriver::click("EINZELNER Planet"))
      .then(DialogDriver::click("SCHEINBAR"))
      .then(DialogDriver::fill({"AUFGANG.....", "TESTORT", "E", "11", "19", "0", "N", "66", "32", "0", "", "21", "6", "1600"},
                               "OK"))
      .then([&refused](QDialog* d) {
        if (auto* box = qobject_cast<QMessageBox*>(d)) {
          refused = box->text();
        }
        d->accept();
      });
  MainWindowProbe::rise_set(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  // the date gives some 66.51, the radix of 1999 gave 66.56 and let the
  // latitude pass
  CHECK(refused.startsWith("Geog. Breite zu groß ! Nur bis  +- 66.5"));
}

TEST_CASE("ERGEBNIS als RADIX warns like erg_rad and claims the next slot") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, eclipse_birth());
  MainWindowProbe::store_solar(*w, "SOLAR 2000");
  QString title;
  QString lines;
  DialogDriver drive;
  // ABBRUCH is his default, nothing moves
  drive.then([&title, &lines](QDialog* d) {
    title = d->windowTitle();
    lines = box_lines(d);
    DialogDriver::click("ABBRUCH")(d);
  });
  MainWindowProbe::result_as_radix(*w);
  CHECK(title == "ENTSCHEIDUNG !");
  CHECK(lines.contains("Nur für GEÜBTE !"));
  CHECK(lines.contains("Das CHAOS DROHT !"));
  CHECK_FALSE(MainWindowProbe::slot(*w, 1).has_value());
  QString mask_title;
  drive.then(DialogDriver::click("WEITER")).then([&mask_title](QDialog* d) {
    mask_title = d->windowTitle();
    DialogDriver::click("OK")(d);
  });
  MainWindowProbe::result_as_radix(*w);
  CHECK(drive.pending() == 0);
  //RR @direkt(0,-1,1,18) with sol$ = s$ + " ALS " + rd$
  CHECK(mask_title == "EINGABE- und ANZEIGE-BOX | SOLAR 2000 ALS RADIX NR.2");
  REQUIRE(MainWindowProbe::slot(*w, 1).has_value());
  // the name stays, the slot carries the label
  CHECK(MainWindowProbe::slot(*w, 1)->surname == "TESTFALL");
  CHECK(MainWindowProbe::slot_text(*w, 1) == "SATZ2: SOLAR 2000 ALS RADIX  TESTFALL");
  CHECK(MainWindowProbe::active_slot(*w) == 1);
}

TEST_CASE("ERGEBNIS als RADIX with all five slots filled asks for the number of the result") {
  auto w = MainWindowProbe::make();
  for (int i = 0; i < 5; ++i) {
    AafRecord r = eclipse_birth();
    r.surname = "FALL" + std::to_string(i + 1);
    MainWindowProbe::put_slot(*w, i, r);
  }
  MainWindowProbe::apply(*w, *MainWindowProbe::slot(*w, 0));
  MainWindowProbe::store_solar(*w, "SOLAR 2000");
  QString overwrite;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("WEITER"))
        .then([&overwrite](QDialog* d) {
          overwrite = box_lines(d);
          DialogDriver::click("Irrtum")(d);
        });
    MainWindowProbe::result_as_radix(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR "NR. "+STR$(zmsp)+" :  "+TRIM$(na$(od,zmsp)), zmsp = ze of the result
  CHECK(overwrite.contains("DIESER DATENSATZ ÜBERSCHREIBT"));
  CHECK(overwrite.contains("NR. 1 :  FALL1"));
  // his Irrtum wrote the slot anyway, the port leaves everything
  CHECK(MainWindowProbe::slot(*w, 0)->surname == "FALL1");
  CHECK(MainWindowProbe::slot_text(*w, 0) == "SATZ1: RADIX  FALL1");
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("WEITER")).then(DialogDriver::click("Weiter")).then(DialogDriver::click("OK"));
    MainWindowProbe::result_as_radix(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::slot_text(*w, 0) == "SATZ1: SOLAR 2000 ALS RADIX  FALL1");
  // dats_l cleared the SOLAR entry of that number, the others stay
  CHECK(MainWindowProbe::slot(*w, 4)->surname == "FALL5");
  CHECK_FALSE(MainWindowProbe::solar_slot(*w, 0).has_value());
}

TEST_CASE("DESKTOP ( QUIT HORCOM ) asks his quit question") {
  auto w = MainWindowProbe::make();
  QString title;
  QString text;
  DialogDriver drive;
  drive.then([&title, &text](QDialog* d) {
    title = d->windowTitle();
    if (auto* box = qobject_cast<QMessageBox*>(d)) {
      text = box->text();
    }
    DialogDriver::click("No")(d);
  });
  MainWindowProbe::desktop_quit(*w);
  CHECK(title == "ABBRUCH?");
  CHECK(text == "PROGRAMM   HORCOM   BEENDEN ?");
  CHECK_FALSE(MainWindowProbe::quit_confirmed(*w));
  drive.then(DialogDriver::click("Yes"));
  MainWindowProbe::desktop_quit(*w);
  CHECK(drive.pending() == 0);
  CHECK(MainWindowProbe::quit_confirmed(*w));
}

TEST_CASE("every close asks his quit question like MENU(1) = 4") {
  auto w = MainWindowProbe::make();
  QString text;
  DialogDriver drive;
  drive.then([&text](QDialog* d) {
    if (auto* box = qobject_cast<QMessageBox*>(d)) {
      text = box->text();
    }
    DialogDriver::click("No")(d);
  });
  // a close from the program itself, the way a separate BEENDEN or the
  // quit key of the system closes the window, asks as well
  CHECK_FALSE(w->close());
  CHECK(text == "PROGRAMM   HORCOM   BEENDEN ?");
  CHECK_FALSE(MainWindowProbe::quit_confirmed(*w));
  drive.then(DialogDriver::click("Yes"));
  CHECK(w->close());
  CHECK(drive.pending() == 0);
  CHECK(MainWindowProbe::quit_confirmed(*w));
}

TEST_CASE("HINTERGRUND-FARBEN keeps his two colours in KONSTA and over the dress") {
  auto w = MainWindowProbe::make();
  const bool before = QSettings().value(theme::kOwnColorsKey, false).toBool();
  QStringList titles;
  DialogDriver drive;
  const auto pick = [&titles](QColor c) {
    return [&titles, c](QDialog* d) {
      titles << d->windowTitle();
      if (auto* cd = qobject_cast<QColorDialog*>(d)) {
        cd->setCurrentColor(c);
      }
      d->accept();
    };
  };
  drive.then(pick(QColor(128, 255, 255))).then(pick(QColor(64, 128, 128)));
  MainWindowProbe::background_colors(*w);
  CHECK(drive.pending() == 0);
  REQUIRE(titles.size() == 2);
  CHECK(titles[0].contains("MUSTER der DIALOGE"));
  CHECK(titles[1].contains("'PASSIVEN' HORCOM - BILDSCHIRM"));
  //RR his shipped KONSTA carries 16777088 and 8421440, RGB with red low
  CHECK(MainWindowProbe::konsta(*w).col_dial == 16777088);
  CHECK(MainWindowProbe::konsta(*w).col_backg == 8421440);
  CHECK(theme::desk_color(false) == QColor(64, 128, 128));
  CHECK(qApp->styleSheet().contains("QDialog { background: #80ffff; }"));
  // the text on his light cyan reads black whatever the dress, the night
  // ink of the dress would stand near white on it
  CHECK(qApp->styleSheet().contains(
      "QDialog QLabel, QDialog QCheckBox, QDialog QRadioButton, QDialog QGroupBox { color: #000000; }"));
  // his colours stand checked as the third entry of FARBEN
  const auto* own = w->findChild<QAction*>("ownColorsAction");
  REQUIRE(own != nullptr);
  CHECK(own->text() == "HINTERGRUND-FARBEN");
  CHECK(own->isChecked());
  // a dress from the Ansicht menu takes its colours back
  QSettings().setValue(theme::kOwnColorsKey, before);
  theme::set_own_colors(QColor(), QColor());
  CHECK(theme::desk_color(false) == QColor(0xE7, 0xE4, 0xD8));
  CHECK_FALSE(qApp->styleSheet().contains("QDialog { background: #80ffff; }"));
}

TEST_CASE("the ink on his dialog colour follows its lightness") {
  // his color_dial fallbacks while KONSTA holds no colour
  CHECK(theme::dialog_color(0) == QColor(100, 100, 255));
  CHECK(theme::passive_color(0) == QColor(192, 192, 192));
  CHECK(theme::ink_on(QColor(128, 255, 255)) == QColor(Qt::black));
  CHECK(theme::ink_on(QColor(192, 192, 192)) == QColor(Qt::black));
  CHECK(theme::ink_on(QColor(0, 0, 128)) == QColor(Qt::white));
  CHECK(theme::ink_on(QColor(100, 100, 255)) == QColor(Qt::white));
}

namespace {

// the entry of a top menu by its caption
QAction* menu_entry(QMainWindow& w, const QString& menu, const QString& entry, QMenu** owner = nullptr) {
  for (QAction* top : w.menuBar()->actions()) {
    if (top->text() == menu && top->menu() != nullptr) {
      for (QAction* a : top->menu()->actions()) {
        if (a->text() == entry) {
          if (owner != nullptr) {
            *owner = top->menu();
          }
          return a;
        }
      }
    }
  }
  return nullptr;
}

// the highlighted text of a reading dialog, then close it
DialogDriver::Step read_kommen(QString& current) {
  return [&current](QDialog* d) {
    if (const auto* list = d->findChild<QListWidget*>(); list != nullptr && list->currentItem() != nullptr) {
      current = list->currentItem()->text();
    }
    d->reject();
  };
}

}  // namespace

TEST_CASE("F1 in an output opens the ERLÄUTERUNG of its menu like wart_erl") {
  auto w = MainWindowProbe::make();
  QMenu* ausw = nullptr;
  QAction* dyn = menu_entry(*w, "&AUSWERTUNG", "DYNAMOGRAMM…", &ausw);
  REQUIRE(dyn != nullptr);
  // DYNAMOGRAMM rides in his a18 world, the Direktionen text
  emit ausw->triggered(dyn);
  CHECK(MainWindowProbe::help_stem(*w) == "komm7");
  QAction* solar = menu_entry(*w, "&AUSWERTUNG", "SOLAR…");
  REQUIRE(solar != nullptr);
  emit ausw->triggered(solar);
  CHECK(MainWindowProbe::help_stem(*w) == "komm5");
  emit ausw->triggered(dyn);
  QString current;
  DialogDriver drive;
  drive
      .then([](QDialog* d) {
        QTimer::singleShot(0, d, [d]() {
          QKeyEvent f1(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
          QApplication::sendEvent(d, &f1);
        });
      })
      .then(read_kommen(current))
      .then([](QDialog* d) { d->reject(); });
  ChoiceDialog::ask(w.get(), "TEST", {"AUSGABE"}, {"OK"});
  CHECK(drive.pending() == 0);
  CHECK(current.contains("Direktionen"));
}

TEST_CASE("the entries added late to the menu tree carry the ERLÄUTERUNG of wart_erl") {
  auto w = MainWindowProbe::make();
  const auto stem_of = [&w](const QString& menu, const QString& entry) {
    QMenu* owner = nullptr;
    QAction* a = menu_entry(*w, menu, entry, &owner);
    REQUIRE(a != nullptr);
    // a stem of another menu first, the entry must replace it
    MainWindowProbe::set_help_stem(*w, "komm2");
    emit owner->triggered(a);
    return MainWindowProbe::help_stem(*w);
  };
  // his CASE 50 TO 59 of the HOROSKOPE menu
  CHECK(stem_of("H&OROSKOPE", "MULTIPLE DIREKTIONEN / HARMONICS G/H…") == "komm4");
  CHECK(stem_of("H&OROSKOPE", "COMPOSIT…") == "komm4");
  CHECK(stem_of("H&OROSKOPE", "DOPPEL-KREIS / 90-GRAD-KREIS…") == "komm4");
  // his CASE 69 TO 75 and CASE 40
  CHECK(stem_of("&AUSWERTUNG", "MUNDAN-ASPEKTE…") == "komm7");
  CHECK(stem_of("&AUSWERTUNG", "SYMB. DIREKTION: EKLIPT. G/H…") == "komm7");
  CHECK(stem_of("&EPHEMERIDE", "STATISTIK G/H…") == "kommstat");
}

TEST_CASE("the menu captions keep the Alt letters of his function keys free") {
  auto w = MainWindowProbe::make();
  QStringList titles;
  for (QAction* top : w->menuBar()->actions()) {
    titles << top->text();
    // ALT + H, D, C, R, F, Z and M reach the function keys of his legend
    const QKeySequence mnemonic = QKeySequence::mnemonic(top->text());
    for (const Qt::Key key : {Qt::Key_H, Qt::Key_D, Qt::Key_C, Qt::Key_R, Qt::Key_F, Qt::Key_Z, Qt::Key_M}) {
      INFO(top->text().toStdString());
      CHECK(mnemonic != QKeySequence(Qt::ALT | key));
    }
  }
  // his men2 captions letter for letter, the Ansicht menu added
  REQUIRE(titles.size() >= 6);
  CHECK(titles.mid(0, 6) ==
        QStringList{"&ÜBER HORCOM", "EI&N-AUSG.", "&EPHEMERIDE", "H&OROSKOPE", "&AUSWERTUNG", "D&IVERSES"});
  // DESKTOP ( QUIT HORCOM ) is the one way out and carries the quit key
  QAction* desktop = menu_entry(*w, "D&IVERSES", "DESKTOP ( QUIT HORCOM )");
  REQUIRE(desktop != nullptr);
  CHECK(desktop->shortcut() == QKeySequence(QKeySequence::Quit));
  CHECK(menu_entry(*w, "EI&N-AUSG.", "BEENDEN") == nullptr);
}

namespace {

void press(QWidget* to, int key, Qt::KeyboardModifiers mods = Qt::NoModifier) {
  QKeyEvent e(QEvent::KeyPress, key, mods);
  QApplication::sendEvent(to, &e);
}

// an output of the given menu entry whose first step presses a key, the
// chart in between follows and the second key closes it
struct BetweenRun {
  QString title;
  bool closed_by_key = false;
};

BetweenRun run_between(MainWindow& w, int item, int close_key, Qt::KeyboardModifiers close_mods,
                       const QString& answer = QString()) {
  BetweenRun run;
  DialogDriver drive;
  drive.then([](QDialog* d) {
    QTimer::singleShot(0, d, [d]() {
      press(d, Qt::Key_F2);
      d->reject();
    });
  });
  if (!answer.isEmpty()) {
    drive.then(DialogDriver::click(answer));
  }
  drive.then([&run, close_key, close_mods](QDialog* d) {
    run.title = d->windowTitle();
    QTimer::singleShot(0, d, [&run, d, close_key, close_mods]() {
      press(d, close_key, close_mods);
      run.closed_by_key = !d->isVisible();
      if (!run.closed_by_key) {
        d->reject();
      }
    });
  });
  QDialog output(&w);
  mark_output(&output, item);
  output.exec();
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  return run;
}

}  // namespace

TEST_CASE("F2 shows the chart in between and F2 or ALT + A closes it like zeige_horoskop") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, eclipse_birth());
  // the radix on screen, the ASPEKTARIUM shows it at once
  BetweenRun run = run_between(*w, menu_item::kAspektarium, Qt::Key_F2, Qt::NoModifier);
  CHECK(run.title.startsWith(" Nur zwischendurch das RADIX anzeigen : "));
  CHECK(run.closed_by_key);
  run = run_between(*w, menu_item::kAspektarium, Qt::Key_A, Qt::AltModifier);
  CHECK(run.closed_by_key);
  // a derived chart, his CASE 64 and 65 show the radix without a question
  MainWindowProbe::store_solar(*w, "SOLAR 2000");
  run = run_between(*w, menu_item::kReturns, Qt::Key_F2, Qt::NoModifier);
  CHECK(run.title.startsWith(" Nur zwischendurch das RADIX anzeigen : "));
  // the other outputs of his list ask which chart shows
  run = run_between(*w, menu_item::kAspektarium, Qt::Key_F2, Qt::NoModifier, "SOLAR 2000");
  CHECK(run.title.startsWith(" Nur zwischendurch das SOLAR 2000 anzeigen : "));
  // an output outside his list ignores the key
  DialogDriver quiet;
  quiet.then([](QDialog* d) {
    QTimer::singleShot(0, d, [d]() {
      press(d, Qt::Key_F2);
      d->reject();
    });
  });
  QDialog clock_output(w.get());
  mark_output(&clock_output, menu_item::kClock);
  clock_output.exec();
  CHECK(quiet.pending() == 0);
  CHECK(quiet.unexpected() == 0);
}

TEST_CASE("the right button opens the Erste Hilfe on the main screen only") {
  auto w = MainWindowProbe::make();
  // the wheel keeps the button for einzel_plan_wahl
  CHECK(MainWindowProbe::wheel_widget(*w)->contextMenuPolicy() == Qt::PreventContextMenu);
  const auto right_click = [](QWidget* at) {
    QContextMenuEvent e(QContextMenuEvent::Mouse, QPoint(4, 4), at->mapToGlobal(QPoint(4, 4)));
    QApplication::sendEvent(at, &e);
  };
  // the coordinate table of the result docks stands for his output screen
  QTableWidget* table = nullptr;
  for (QTableWidget* t : w->findChildren<QTableWidget*>()) {
    if (t->columnCount() > 1) {
      table = t;
    }
  }
  REQUIRE(table != nullptr);
  {
    DialogDriver quiet;
    right_click(table->viewport());
    QApplication::processEvents();
    CHECK(quiet.unexpected() == 0);
  }
  // a switch of the input panel belongs to the main screen
  QCheckBox* panel_box = nullptr;
  for (QCheckBox* b : w->findChildren<QCheckBox*>()) {
    if (b->text() == "Sommerzeit") {
      panel_box = b;
    }
  }
  REQUIRE(panel_box != nullptr);
  QString title;
  DialogDriver drive;
  drive.then([&title](QDialog* d) {
    title = d->windowTitle();
    d->reject();
  });
  right_click(panel_box);
  CHECK(drive.pending() == 0);
  CHECK(title == "Erste Hilfe und Einführung");
}

TEST_CASE("the Erste Hilfe list leads to the short manual and back like erste_hilfe") {
  auto w = MainWindowProbe::make();
  QString title;
  QString first;
  int rows = 0;
  QString manual;
  int shown = 0;
  DialogDriver drive;
  drive
      .then([&](QDialog* d) {
        ++shown;
        title = d->windowTitle();
        const auto* list = d->findChild<QListWidget*>();
        REQUIRE(list != nullptr);
        rows = list->count();
        first = list->item(0)->text();
        DialogDriver::click("KURZ-ANLEITUNG lesen")(d);
      })
      .then(read_kommen(manual))
      .then([&shown](QDialog* d) {
        ++shown;
        DialogDriver::click("Zurück zum HAUPT - MENÜ")(d);
      });
  MainWindowProbe::erste_hilfe(*w);
  CHECK(drive.pending() == 0);
  CHECK(title == "Erste Hilfe und Einführung");
  CHECK(first.contains("Die grundlegende BEDIENUNGSWEISE von HORCOM :"));
  CHECK(rows == 53);
  CHECK(manual.contains("Kurzanleitung"));
  CHECK(shown == 2);
}

TEST_CASE("ÄNDERUNGEN / HINWEISE / KURZANL. asks which text like aendlist") {
  auto w = MainWindowProbe::make();
  QString current;
  QString lines;
  DialogDriver drive;
  drive
      .then([&lines](QDialog* d) {
        lines = box_lines(d);
        DialogDriver::click("ÄNDERUNGEN seit 1996")(d);
      })
      .then(read_kommen(current));
  MainWindowProbe::anmerkungen(*w);
  CHECK(drive.pending() == 0);
  CHECK(lines.contains("ANMERKUNGEN zu HORCOM"));
  CHECK(current.contains("Änderungsliste"));
}

TEST_CASE("the function key switches announce themselves like geohelio and druck_enbl_alt") {
  auto w = MainWindowProbe::make();
  QString title;
  QString text;
  DialogDriver drive;
  const int before = MainWindowProbe::konsta(*w).prenbl;
  drive.then(read_message(title, text));
  MainWindowProbe::toggle_printer_option(*w);
  CHECK(MainWindowProbe::konsta(*w).prenbl == (before != 0 ? 0 : 1));
  CHECK(text == (before != 0 ? "DRUCKER-OPTION AUS !" : "DRUCKER-OPTION EIN !"));
  drive.then(read_message(title, text));
  MainWindowProbe::toggle_printer_option(*w);
  CHECK(MainWindowProbe::konsta(*w).prenbl == before);
  const bool helio = MainWindowProbe::settings(*w).heliocentric;
  drive.then(read_message(title, text));
  MainWindowProbe::toggle_helio(*w);
  CHECK(MainWindowProbe::settings(*w).heliocentric != helio);
  CHECK(text == (helio ? "Heliozentrisch 'AUS' !" : "Heliozentrisch 'EIN' !"));
  drive.then(read_message(title, text));
  MainWindowProbe::toggle_helio(*w);
  CHECK(MainWindowProbe::settings(*w).heliocentric == helio);
  CHECK(drive.pending() == 0);
}

TEST_CASE("ÜBER HORCOM carries his function key legend in grey") {
  auto w = MainWindowProbe::make();
  QAction* intro = menu_entry(*w, "&ÜBER HORCOM", "EINFÜHRUNG = ERLÄUTERUNG 1");
  REQUIRE(intro != nullptr);
  CHECK(intro->isEnabled());
  QAction* f2 = menu_entry(*w, "&ÜBER HORCOM", "F2 ( oder ALT + A ) = HOROSKOP ANSEHEN ( In sonstigen Ausgaben )");
  REQUIRE(f2 != nullptr);
  CHECK_FALSE(f2->isEnabled());
  QAction* f9 = menu_entry(*w, "&ÜBER HORCOM", "F9 ( oder ALT + M ) = DOPPEL-AUSDRUCK AKTIVIEREN");
  REQUIRE(f9 != nullptr);
  CHECK_FALSE(f9->isEnabled());
}
