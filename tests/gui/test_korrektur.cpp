// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QAbstractButton>
#include <QLabel>
#include <QRegularExpression>

#include <cmath>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/directions.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "probe.hpp"
#include "robert_text.hpp"

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
  r.zone = kUtZoneText;
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 19;
  return r;
}

// the answers of input_grmise_zod for a longitude in whole arc seconds
struct ZodiacAnswer {
  int sign = 0;
  QStringList dms;
  double rad = 0.0;
};

ZodiacAnswer zodiac_answer(double rad) {
  const int total = static_cast<int>(std::lround(norm_deg(rad * kRadToDeg) * kArcsecPerDeg));
  const int per_sign = static_cast<int>(kDegPerSign * kArcsecPerDeg);
  ZodiacAnswer a;
  a.sign = total / per_sign;
  const int in_sign = total - a.sign * per_sign;
  a.dms = {QString::number(in_sign / 3600), QString::number((in_sign / 60) % 60), QString::number(in_sign % 60)};
  a.rad = total / kArcsecPerDeg * kDegToRad;
  return a;
}

double off_arcsec(double a, double b) {
  return std::abs(fold_rad(a - b)) * kRadToDeg * kArcsecPerDeg;
}

// every label of a box joined, his alert lines
QString labels(QDialog* d) {
  QStringList out;
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    out << l->text();
  }
  return out.join(QChar(0x0A));
}

const DialogDriver::Step kClose = [](QDialog* d) { d->reject(); };

// the panel clock holds whole seconds, half a second of sidereal time
// turns the MC by some seven and a half arc seconds
constexpr double kPanelMcArcsec = 10.0;
constexpr double kPanelHalfSecondHours = 0.5 / 3600.0;

}  // namespace

TEST_CASE("KORREKTUR mit MC walks the dialogs of korr and lands on the degree") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const ZodiacAnswer mc = zodiac_answer(MainWindowProbe::chart(*w).houses.angles.mc + 2.0 * kDegToRad);
  DialogDriver drive;
  drive.then(DialogDriver::click("MC"))
      .then(DialogDriver::pick_row(mc.sign))
      .then(DialogDriver::fill(mc.dms, "OK"))
      .then(kClose);  // his hausa after the angle modes
  MainWindowProbe::correction(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(off_arcsec(MainWindowProbe::chart(*w).houses.angles.mc, mc.rad) < kPanelMcArcsec);
}

TEST_CASE("KORREKTUR mit STERNZEIT reads the local sidereal time") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const double lst = MainWindowProbe::chart(*w).armc_deg / kDegPerHour + 0.5;
  const int seconds = static_cast<int>(std::lround(lst * 3600.0));
  DialogDriver drive;
  drive.then(DialogDriver::click("STERNZEIT"))
      .then(DialogDriver::fill({QString::number(seconds / 3600), QString::number((seconds / 60) % 60),
                                QString::number(seconds % 60)},
                               "OK"))
      .then(kClose);
  MainWindowProbe::correction(*w);
  CHECK(drive.unexpected() == 0);
  CHECK(std::abs(MainWindowProbe::chart(*w).armc_deg / kDegPerHour - seconds / 3600.0) < kPanelHalfSecondHours);
}

TEST_CASE("KORREKTUR mit MOND stays next to the birth and shows the coordinates like a91") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const double before = MainWindowProbe::panel_jd(*w);
  const ZodiacAnswer moon = zodiac_answer(MainWindowProbe::chart(*w).b[body::kMoon].el + 2.0 * kDegToRad);
  QString table;
  DialogDriver drive;
  drive.then(DialogDriver::click("MOND"))
      .then(DialogDriver::pick_row(moon.sign))
      .then(DialogDriver::fill(moon.dms, "OK"))
      .then([&table](QDialog* d) {
        table = d->windowTitle();
        d->reject();
      });
  MainWindowProbe::correction(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(table == "PLANETEN-KOORDINATEN");
  CHECK(off_arcsec(MainWindowProbe::chart(*w).b[body::kMoon].el, moon.rad) < 1.0);
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - before) < 0.5);
}

TEST_CASE("the prima session sums two variations into the radix") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const double before = MainWindowProbe::panel_jd(*w);
  const Chart radix = MainWindowProbe::chart(*w);
  int date_fields = 0;
  QString sums;
  DialogDriver drive;
  drive.then(DialogDriver::click("JA"))  // Ereignis-ORT = Geburts-ORT ?
      .then([&date_fields](QDialog* d) {  // DATUM EINGEBEN !
        date_fields = static_cast<int>(d->findChildren<QLineEdit*>().size());
        DialogDriver::fill({"", "1", "1", "2020"}, "OK")(d);
      })
      .then(DialogDriver::click("RADIX"))   // Mit RADIX - Planeten ?
      .then(DialogDriver::click("DIREKT"))  // RICHTUNG der ACHSEN
      .then(DialogDriver::click("STZ VARIIEREN"))
      .then(DialogDriver::fill({"2"}, "OK"))
      .then(DialogDriver::click(" SUMMIEREN "))
      .then([&sums](QDialog* d) {
        sums = labels(d);
        DialogDriver::click("WEITERE VARIATION")(d);
      })
      .then(DialogDriver::click("STZ VARIIEREN"))
      .then(DialogDriver::fill({"1"}, "OK"))
      .then(DialogDriver::click(" SUMMIEREN "))
      .then(DialogDriver::click("WEITERE VARIATION"))
      .then(DialogDriver::click("VARIATION BEENDEN"))
      .then(DialogDriver::click("SUMME der VARIATIONEN"))
      .then(DialogDriver::click("TESTS BEENDEN"));
  MainWindowProbe::prima(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK_FALSE(MainWindowProbe::axes_shown(*w));
  // three degrees of sidereal time are almost twelve clock minutes, 11.97,
  // his dst = 1/tja gave 11.83 and turned the radix ARMC by 2.965 degrees
  // while his preview had shown 2.957
  const double expected = 3.0 / (kDegPerCircle * kSolarToSiderealRate);
  CHECK(std::abs((MainWindowProbe::panel_jd(*w) - before) - expected) * kSecondsPerDay < 1.0);
  // his a37dat with the blank zeitv$ carries the clock row, V, date and time
  CHECK(date_fields == 7);
  // the total turn is the directed arc with the two degrees, the event at
  // the radix clock. His brm + dif - arm counted the variation once more
  // after 2 * 360 / tja, 1.971 degrees too far
  const double turn = direct_axes(radix, MainWindowProbe::lon(*w), MainWindowProbe::radix_input(*w).lat_deg,
                                  julian_day({1, 1, 2020, 3.0, 0.0}), false, 2.0, HouseSystem::kPlacidus)
                          .arc_deg;
  const QRegularExpressionMatch m = QRegularExpression(R"(GESAMT-DREHUNG[^:]*:\s*(-?[0-9.]+))").match(sums);
  REQUIRE(m.hasMatch());
  CHECK(std::abs(m.captured(1).toDouble() - turn) < 0.0015);
}

TEST_CASE("the mean variation goes back to the sums box on TESTS BEENDEN like primend") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const double before = MainWindowProbe::panel_jd(*w);
  QStringList after_mean;
  DialogDriver drive;
  drive.then(DialogDriver::click("JA"))
      .then(DialogDriver::fill({"", "1", "1", "2020"}, "OK"))
      .then(DialogDriver::click("RADIX"))
      .then(DialogDriver::click("DIREKT"))
      .then(DialogDriver::click("STZ VARIIEREN"))
      .then(DialogDriver::fill({"2"}, "OK"))
      .then(DialogDriver::click(" SUMMIEREN "))
      .then(DialogDriver::click("WEITERE VARIATION"))
      .then(DialogDriver::click("STZ VARIIEREN"))
      .then(DialogDriver::fill({"1"}, "OK"))
      .then(DialogDriver::click(" SUMMIEREN "))
      .then(DialogDriver::click("WEITERE VARIATION"))
      .then(DialogDriver::click("VARIATION BEENDEN"))
      .then(DialogDriver::click("MITTLERE VARIATION"))
      .then([&after_mean](QDialog* d) {
        for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
          after_mean << b->text();
        }
        DialogDriver::click(" TESTS BEENDEN ")(d);
      })
      .then(DialogDriver::click("TESTS BEENDEN"));  // the sums box once more
  MainWindowProbe::prima(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  // his m$ joins the two answers after the mean
  CHECK(after_mean.contains("KORREKTUR beenden"));
  // the mean of three degrees over two variations, taken once
  const double expected = 1.5 / (kDegPerCircle * kSolarToSiderealRate);
  CHECK(std::abs((MainWindowProbe::panel_jd(*w) - before) - expected) * kSecondsPerDay < 1.0);
}

TEST_CASE("KORREKTUR runs on Placidus and geocentric like plre and hands the system back") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const ZodiacAnswer mc = zodiac_answer(MainWindowProbe::chart(*w).houses.angles.mc + 2.0 * kDegToRad);
  // KOCH-GOH, his haw& = 3
  MainWindowProbe::houses(*w, 3);
  MainWindowProbe::helio(*w, true);
  QString notice;
  QString table;
  DialogDriver drive;
  drive.then([&notice](QDialog* d) {
         notice = d->windowTitle() + QChar(0x0A) + labels(d);
         DialogDriver::click("WEITER")(d);
       })
      .then(DialogDriver::click("MC"))
      .then(DialogDriver::pick_row(mc.sign))
      .then(DialogDriver::fill(mc.dms, "OK"))
      .then([&table](QDialog* d) {
        table = d->windowTitle();
        d->reject();
      });
  MainWindowProbe::correction(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(notice.startsWith("!! ACHTUNG !!"));
  CHECK(notice.contains("PLACIDUS - HÄUSER ERFORDERLICH !"));
  // his korr21 ends in hausa while haw& = 1
  CHECK(table.contains("Placidus"));
  // korrend brings haw_merk& back, hrg! stays cleared like his CLR hrg!
  CHECK(MainWindowProbe::settings(*w).houses == HouseSystem::kKochGoh);
  CHECK_FALSE(MainWindowProbe::settings(*w).heliocentric);
  CHECK(off_arcsec(MainWindowProbe::chart(*w).houses.angles.mc, mc.rad) < kPanelMcArcsec);
}

TEST_CASE("the PRIMÄR DIRIGIERTE ACHSEN row announces Placidus once") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  MainWindowProbe::houses(*w, 2);
  DialogDriver drive;
  drive.then(DialogDriver::click("WEITER"))
      .then(DialogDriver::click("* PRIMÄR DIRIGIERTE ACHSEN *"))
      .then(kClose);  // Ereignis-ORT = Geburts-ORT ?
  MainWindowProbe::correction(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  // his first plre switched haw& already, the second stayed silent
  CHECK(drive.titles().count(QStringLiteral("!! ACHTUNG !!")) == 1);
  CHECK(MainWindowProbe::settings(*w).houses == HouseSystem::kTopocentric);
  CHECK_FALSE(MainWindowProbe::axes_shown(*w));
}

TEST_CASE("the directed axes carry the texts of primhorg with the sums of the variations") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const Chart radix = MainWindowProbe::chart(*w);
  const auto texts = [&w]() {
    QStringList out;
    for (const Primitive& p : MainWindowProbe::wheel(*w).items) {
      if (p.kind == Primitive::Kind::kText) {
        out << QString::fromStdString(p.text);
      }
    }
    return out;
  };
  QStringList before;
  QStringList after;
  DialogDriver drive;
  drive.then(DialogDriver::click("JA"))                        // Ereignis-ORT = Geburts-ORT ?
      .then(DialogDriver::fill({"", "1", "1", "2020"}, "OK"))  // DATUM EINGEBEN !
      .then(DialogDriver::click("RADIX"))                      // Mit RADIX - Planeten ?
      .then(DialogDriver::click("DIREKT"))                     // RICHTUNG der ACHSEN
      .then([&](QDialog* d) {                                  // PROBEWEISE STERNZEIT VARIIEREN ?
        before = texts();
        DialogDriver::click("STZ VARIIEREN")(d);
      })
      .then(DialogDriver::fill({"2"}, "OK"))
      .then(DialogDriver::click(" SUMMIEREN "))
      .then(DialogDriver::click("WEITERE VARIATION"))
      .then([&](QDialog* d) {
        after = texts();
        DialogDriver::click("VARIATION BEENDEN")(d);
      })
      .then(DialogDriver::click("TESTS BEENDEN"));
  MainWindowProbe::prima(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  // the event moment under Ereignis:, datum3$ and ze$ of the UT clock
  CHECK(before.contains("Ereignis:"));
  CHECK(before.contains(" 1. 1.2020   "));
  CHECK(before.contains(" 3h  0m  0s"));
  // k$ over the wheel and e$ at the lower left
  CHECK(before.contains(" DIREKT   ( + )"));
  CHECK(before.contains("RADIX - Planeten"));
  // IF dif <> 0, no sums before the first variation
  for (const QString& t : before) {
    CHECK_FALSE(t.startsWith("VAR."));
  }
  const double jd_event = julian_day({1, 1, 2020, 3.0, 0.0});
  const DirectedAxes turned = direct_axes(radix, MainWindowProbe::lon(*w), MainWindowProbe::radix_input(*w).lat_deg,
                                          jd_event, false, 2.0, HouseSystem::kPlacidus);
  CHECK(after.contains(QString::asprintf("STZ-DIFF=%9.3f", turned.arc_deg) + QChar(0xB0)));
  CHECK(after.contains(QString("VAR. STZ  =  2.00") + QChar(0xB0)));
  CHECK(after.contains(QString("VAR.-SUM.STZ =  2.00") + QChar(0xB0)));
  CHECK(after.contains(QString("MITT.SUM.STZ =  2.00") + QChar(0xB0)));
  // "STZ:" + homise$ of the directed ARMC stands in place of the radix STZ
  CHECK(after.contains("STZ:" + homise_text(turned.armc_deg, 0) + " "));
  CHECK_FALSE(after.contains(QString::fromStdString(MainWindowProbe::sheet(*w).stz)));
  CHECK_FALSE(after.contains(QString::fromStdString(MainWindowProbe::sheet(*w).mode)));
}
