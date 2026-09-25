// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include <cmath>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/time/delta_t.hpp"
#include "probe.hpp"
#include "robert_input.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// the lines of a result box, then close it
DialogDriver::Step read_box(QString& title, QStringList& lines) {
  return [&title, &lines](QDialog* d) {
    title = d->windowTitle();
    if (auto* box = qobject_cast<QMessageBox*>(d)) {
      lines = box->text().split(QChar(0x0A));
    }
    d->accept();
  };
}

// his ut_etd$ for a moment given in whole seconds of the day
QString ut_etd(int day, int month, int year, long long seconds) {
  return QString::asprintf("%2d.%2d.%5d     %2lldh %2lldm %2llds", day, month, year, seconds / 3600, (seconds / 60) % 60,
                           seconds % 60);
}

// the number behind the label of a result line
double number_after(const QString& line, const QString& label) {
  return line.mid(line.indexOf(label) + label.size()).trimmed().split(QChar(0xB0)).first().toDouble();
}

}  // namespace

TEST_CASE("ET aus UT adds delta T like et_ut") {
  auto w = MainWindowProbe::make();
  QString title;
  QStringList lines;
  DialogDriver drive;
  drive.then(DialogDriver::fill({"", "1", "1", "2000", "12", "0", "0"}, "OK")).then(read_box(title, lines));
  MainWindowProbe::et_from_ut(*w);
  CHECK(drive.pending() == 0);
  CHECK(title == "Ergebnis");
  REQUIRE(lines.size() == 5);
  // delta T was 63.8 seconds at the start of 2000
  const double dt_s = delta_t_minutes(kJdJ2000) * 60.0;
  CHECK(dt_s == doctest::Approx(63.8).epsilon(0.02));
  const long long dt = std::llround(dt_s);
  CHECK(lines[0] == "WELT-ZEIT        UT = " + ut_etd(1, 1, 2000, 12 * 3600));
  CHECK(lines[1] == "EPHEMERIDENZEIT  ET = " + ut_etd(1, 1, 2000, 12 * 3600 + dt));
  CHECK(lines[2] == QString::asprintf("ET - UT =  0d  0h %2lldm %2llds ", dt / 60, dt % 60));
  CHECK(lines[4] == "WERTE sind EXTRAPOLIERT ! ( s. Erl. 3 )");
}

TEST_CASE("UT aus ET splits a delta T of more than a day into days and hours") {
  auto w = MainWindowProbe::make();
  QString title;
  QStringList lines;
  DialogDriver drive;
  // 4001 before Christ, delta T runs past a day there
  drive.then(DialogDriver::fill({"V", "1", "1", "4001", "12", "0", "0"}, "OK")).then(read_box(title, lines));
  MainWindowProbe::ut_from_et(*w);
  CHECK(drive.pending() == 0);
  REQUIRE(lines.size() == 5);
  const double jde = julian_day({1, 1, -4000, 12.0, 0.0});
  const double dt_min = delta_t_minutes(jde);
  REQUIRE(dt_min > kMinutesPerDay);
  const long long total = std::llround(dt_min) * 60;
  // his etm_ut printed TRUNC(dt * 24), the hours of the whole difference
  CHECK(lines[2] == QString::asprintf("ET - UT = %2lldd %2lldh %2lldm ", total / 86400, (total / 3600) % 24, (total / 60) % 60));
  CHECK(lines[4] == "WERTE sind EXTRAPOLIERT ! ( s. Erl. 3 )");
}

TEST_CASE("the delta T remark stays empty where his mes5 kept the last one") {
  CHECK(MainWindowProbe::et_ut_note(1300).isEmpty());
  CHECK(MainWindowProbe::et_ut_note(1995).isEmpty());
  CHECK(MainWindowProbe::et_ut_note(1800) == "GENÄHERTE WERTE ! ( s. Erl. 3 )");
  CHECK(MainWindowProbe::et_ut_note(500) == "WERTE sind EXTRAPOLIERT ! ( s. Erl. 3 )");
}

TEST_CASE("DATUM aus JD reads the calendar date and the obliquity of J2000") {
  auto w = MainWindowProbe::make();
  QStringList info;
  int asked = 0;
  DialogDriver drive;
  drive.then([&asked](QDialog* d) {
         ++asked;
         DialogDriver::fill({"2451545"}, "OK")(d);
       })
      .then([&info](QDialog* d) {
        for (const QLabel* l : d->findChildren<QLabel*>()) {
          info << l->text();
        }
        DialogDriver::click("Weiter")(d);
      })
      .then([&asked](QDialog* d) {
        ++asked;
        d->reject();
      });
  MainWindowProbe::date_from_jd(*w);
  CHECK(drive.pending() == 0);
  CHECK(asked == 2);
  const QString all = info.join(QChar(0x0A));
  CHECK(all.contains(" Julianisches Jahrhundert ab 31.12.1899 12h =  1.0000000000"));
  CHECK(all.contains("Datum :  1. 1.2000    "));
  CHECK(all.contains("| Welt-Zeit : 12h  0m  0s"));
  CHECK(all.contains("( JD = 0 entspricht : 12 h  am  1.1.4713 v.Chr. )"));
  // the tester's Auréas count starts at 31.12.1899 12h, the epoch of the
  // centuries above, so 1.1.2000 12h reads his 36525. The label said
  // 1.1.1900 before, half a day off its own count
  CHECK(all.contains(QString::fromUtf8(" Auréas-Zählung ( Tage seit 31.12.1899 12h ) = 36525.0000")));
  // Meeus gives the mean obliquity of J2000 as 23 26 21.448, the
  // nutation takes 5.6 arcseconds off that day
  double mean = 0.0;
  double apparent = 0.0;
  for (const QString& line : info.join(QChar(0x0A)).split(QChar(0x0A))) {
    if (line.contains("Mittlere Ekliptik-Schiefe")) {
      mean = number_after(line, "=");
    }
    if (line.contains("Apparente Ekliptik-Schiefe")) {
      apparent = number_after(line, "=");
    }
  }
  CHECK(mean == doctest::Approx(23.4393).epsilon(0.00001));
  CHECK(std::abs(apparent - 23.43774) < 0.0003);
}

TEST_CASE("AR-DE aus EL-EB puts the solstice point at six hours and the obliquity") {
  auto w = MainWindowProbe::make();
  QString title;
  QStringList lines;
  DialogDriver drive;
  drive.then(DialogDriver::fill({"", "1", "1", "2000"}, "OK"))
      .then(DialogDriver::fill({"", "", "", "90", "N", "", "", "", "0"}, "OK"))
      .then(read_box(title, lines));
  MainWindowProbe::arde_from_eleb(*w);
  CHECK(drive.pending() == 0);
  CHECK(title == "Ergebnis :");
  REQUIRE(lines.size() == 5);
  CHECK(lines[0] == "Ekliptikale Länge =  90.0000°");
  CHECK(lines[2] == "Entsprechen beim Datum  1. 1.2000    :");
  CHECK(lines[3] == " Rektaszension  AR  =  90.0000°  =  6h  0m  0.00s");
  CHECK(lines[4].startsWith(" Deklination    DE  = 23.43"));
  CHECK(lines[4].endsWith(" N"));
}

TEST_CASE("EL-EB aus AR-DE puts the equator point of six hours below the ecliptic") {
  auto w = MainWindowProbe::make();
  QString title;
  QStringList lines;
  DialogDriver drive;
  drive.then(DialogDriver::fill({"", "1", "1", "2000"}, "OK"))
      .then(DialogDriver::fill({"", "", "", "90", "N", "", "", "", "0"}, "OK"))
      .then(read_box(title, lines));
  MainWindowProbe::eleb_from_arde(*w);
  CHECK(drive.pending() == 0);
  REQUIRE(lines.size() == 5);
  CHECK(lines[0] == "Rektaszension AR =  90.0000°");
  CHECK(lines[3].endsWith("=  90.0000°"));
  CHECK(lines[4].startsWith(" Eklipt.Breite    EB  = 23.43"));
  CHECK(lines[4].endsWith(" S"));
}

TEST_CASE("LT aus UT and UT aus LT shift by the longitude like lt_ut and ut_lt") {
  auto w = MainWindowProbe::make();
  QString title;
  QStringList east;
  QStringList west;
  DialogDriver drive;
  drive.then(DialogDriver::fill({"E", "15", "0", "0", "N", "48", "0", "0"}, "OK"))
      .then(DialogDriver::fill({"", "1", "1", "2000", "12", "0", "0"}, "OK"))
      .then(read_box(title, east));
  MainWindowProbe::local_time_convert(*w, true);
  CHECK(drive.pending() == 0);
  REQUIRE(east.size() == 5);
  CHECK(east[1] == "Datum / Zeit ( UT )    : " + ut_etd(1, 1, 2000, 12 * 3600));
  CHECK(east[4] == "Datum / Zeit ( LMT )   : " + ut_etd(1, 1, 2000, 13 * 3600));
  // thirty degrees west at one in the morning local time is three UT
  drive.then(DialogDriver::fill({"W", "30", "0", "0", "N", "48", "0", "0"}, "OK"))
      .then(DialogDriver::fill({"", "1", "1", "2000", "1", "0", "0"}, "OK"))
      .then(read_box(title, west));
  MainWindowProbe::local_time_convert(*w, false);
  REQUIRE(west.size() == 5);
  CHECK(west[4] == "Datum / Zeit ( UT )     : " + ut_etd(1, 1, 2000, 3 * 3600));
}

TEST_CASE("WINKEL-UMRECHNUNG keeps the sign both ways") {
  // his rechne1 took ABS on the way to degree minute second and read
  // -12 30 as -11.5 on the way back
  const std::array<QString, 4> split = angle_enter({"-12.5", "", "", ""});
  CHECK(split[1] == "-12");
  CHECK(split[2] == " 30");
  CHECK(split[3] == " 0.00");
  const std::array<QString, 4> joined = angle_enter({"", "-12", "30", "0"});
  CHECK(joined[0].trimmed() == "-12.500000");
  // a fraction of a degree below zero keeps its sign on the degree field
  CHECK(angle_enter({"-0.25", "", "", ""})[1] == " -0");
  CHECK(angle_enter({"", "-0", "15", ""})[0].trimmed() == "-0.250000");
  // the rounded hundredths carry into the minutes and degrees
  const std::array<QString, 4> carry = angle_enter({"10.999999", "", "", ""});
  CHECK(carry[1] == " 11");
  CHECK(carry[2] == "  0");
  CHECK(carry[3] == " 0.00");
  // a comma counts as the decimal point like his komma_pkt$
  CHECK(angle_enter({"1,5", "", "", ""})[2] == " 30");
}

TEST_CASE("the WINKEL-UMRECHNUNG box converts on ENTER and stays open") {
  auto w = MainWindowProbe::make();
  QStringList fields;
  DialogDriver drive;
  drive.then([&fields](QDialog* d) {
    const QList<QLineEdit*> edits = d->findChildren<QLineEdit*>();
    REQUIRE(edits.size() == 4);
    edits[0]->setText("23.4392911");
    emit edits[0]->returnPressed();
    for (const QLineEdit* e : edits) {
      fields << e->text();
    }
    CHECK(d->isVisible());
    DialogDriver::click("QUIT")(d);
  });
  angle_converter(w.get());
  CHECK(drive.pending() == 0);
  REQUIRE(fields.size() == 4);
  CHECK(fields[1] == " 23");
  CHECK(fields[2] == " 26");
  CHECK(fields[3] == "21.45");
}
