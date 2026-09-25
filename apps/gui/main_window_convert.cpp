// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// The converter entries of EPHEMERIDE and DIVERSES, each its own menu
// entry like his menu, the WINKEL-UMRECHNUNG of rechne1 lives with the
// input boxes.

#include <cmath>
#include <limits>

#include <QMessageBox>

#include "calendar_mark.hpp"
#include "choice_dialog.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"
#include "horcom/ephem/sunmoon.hpp"
#include "horcom/time/delta_t.hpp"
#include "main_window.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"

namespace horcom {

namespace {

// his IF ja < 948 OR ja > 2050, the coarse range of his delta T
constexpr int kFineFrom = 948;
constexpr int kFineTo = 2050;
// the clock splits of his result lines in whole seconds
constexpr long long kSecPerMinute = 60;
constexpr long long kMinPerHour = 60;
constexpr auto kSecPerHour = static_cast<long long>(kSecondsPerHour);
constexpr auto kHourPerDay = static_cast<long long>(kHoursPerDay);

bool coarse_year(int year) {
  return year < kFineFrom || year > kFineTo;
}

// ported from ut_etd$, whole minutes in the coarse range, seconds else.
// His seconds were rounded without a carry, the port rounds the moment
double rounded(double jd, bool coarse) {
  const double step = coarse ? kMinutesPerDay : static_cast<double>(kSecondsPerDay);
  return std::floor(jd * step + 0.5) / step;
}

QString ut_etd_text(double jd, Calendar cal) {
  const bool coarse = coarse_year(calendar_date(jd, cal).year);
  const CalendarDate d = calendar_date(rounded(jd, coarse), cal);
  const long long total = std::llround((d.hour * 60.0 + d.minute) * 60.0);
  // his STR$(ta,2) + "." + STR$(mo,2) + "." + STR$(ja,5) + "     " + STR$(ho,2) + "h "
  const QString head = QString::asprintf("%2d.%2d.%5d     %2lldh ", d.day, d.month, d.year, total / kSecPerHour);
  const long long minutes = (total / kSecPerMinute) % kMinPerHour;
  return coarse ? head + QString::asprintf("%2lldm ", minutes)
                : head + QString::asprintf("%2lldm %2llds", minutes, total % kSecPerMinute);
}

// ported from etm_ut. His hours were the total hours of the difference
// and his rounded seconds carried nothing, the port splits days, hours,
// minutes and seconds
QString et_minus_ut_text(double dt_days, int year) {
  const bool coarse = coarse_year(year);
  const long long unit = coarse ? kSecPerMinute : 1;
  const long long total = std::llround(std::abs(dt_days) * kSecondsPerDay / static_cast<double>(unit)) * unit;
  const long long d = total / kSecondsPerDay;
  const long long h = (total / kSecPerHour) % kHourPerDay;
  const long long m = (total / kSecPerMinute) % kMinPerHour;
  const QString sign = dt_days < 0.0 ? "-" : "";
  return coarse ? QString("ET - UT = %1").arg(sign) + QString::asprintf("%2lldd %2lldh %2lldm ", d, h, m)
                : QString("ET - UT = %1").arg(sign) +
                      QString::asprintf("%2lldd %2lldh %2lldm %2llds ", d, h, m, total % kSecPerMinute);
}

}  // namespace

// ported from et_ut_erl. His mes5$ kept the note of the last run for the
// years 948 to 1620 and for 1995, the port leaves those empty
QString MainWindow::et_ut_note(int year) {
  // his IF ja > 1995 OR ja < 948
  if (year > 1995 || year < kFineFrom) {
    return tr("WERTE sind EXTRAPOLIERT ! ( s. Erl. 3 )");
  }
  // his ELSE IF ja > 1620 && ja < 1995
  if (year > 1620 && year < 1995) {
    return tr("GENÄHERTE WERTE ! ( s. Erl. 3 )");
  }
  return {};
}

// the moment his boxes open with, the clock of the active chart in UT
CalendarDate MainWindow::convert_start() const {
  return last_chart_ ? calendar_date(last_chart_->jd_ut, current_settings().calendar) : panel_day();
}

// ported from et_ut
void MainWindow::et_from_ut() {
  // his a37dat(32,"GREENWICH-ZEIT","DATUM-ZEIT-EINGABE")
  const std::optional<CalendarDate> d = ask_date(this, tr("DATUM-ZEIT-EINGABE"), convert_start(), tr("GREENWICH-ZEIT"));
  if (!d) {
    return;
  }
  const Calendar cal = current_settings().calendar;
  const double jd = julian_day(*d, cal);
  const double dt = delta_t_minutes(jd) / kMinutesPerDay;
  // his "WELT-ZEIT        UT = ", "EPHEMERIDENZEIT  ET = "
  const QStringList lines{tr("WELT-ZEIT        UT = ") + ut_etd_text(jd, cal),
                          tr("EPHEMERIDENZEIT  ET = ") + ut_etd_text(jd + dt, cal), et_minus_ut_text(dt, d->year), "  ",
                          et_ut_note(d->year)};
  QMessageBox::information(this, tr("Ergebnis"), lines.join(QChar(0x0A)));
}

// ported from ut_et
void MainWindow::ut_from_et() {
  // his a37dat(32,"EPHEMERIDENZEIT","DATUM-ZEIT-EINGABE")
  const std::optional<CalendarDate> d = ask_date(this, tr("DATUM-ZEIT-EINGABE"), convert_start(), tr("EPHEMERIDENZEIT"));
  if (!d) {
    return;
  }
  const Calendar cal = current_settings().calendar;
  const double jde = julian_day(*d, cal);
  // his @utet, @etut, delta T taken at the ET moment
  const double dt = delta_t_minutes(jde) / kMinutesPerDay;
  const QStringList lines{tr("EPHEMERIDENZEIT  ET = ") + ut_etd_text(jde, cal),
                          tr("WELT-ZEIT        UT = ") + ut_etd_text(jde - dt, cal), et_minus_ut_text(dt, d->year), "  ",
                          et_ut_note(d->year)};
  QMessageBox::information(this, tr("Ergebnis"), lines.join(QChar(0x0A)));
}

// ported from dat_jd with dat_jd1, the box asks again on Weiter
void MainWindow::date_from_jd() {
  const Calendar cal = current_settings().calendar;
  // his jul$(1,ze), the calendar mark of the record
  const QString jul = jul_mark(panel_calendar_);
  // his dat_jd1, the current record under the input box
  QStringList current;
  if (last_chart_ && (record_.day > 0 || record_.jd > 0.0)) {
    const Chart radix = radix_chart();
    const CalendarDate rd = calendar_date(radix.jd_ut, cal);
    current << tr(" Der AKTUELLE Datensatz : %1").arg(record_label_.trimmed())
            << tr(" Mit dem KALENDER - Datum :%1 %2 %3 UT=GMT ")
                   .arg(datum3_text(rd), jul, homise_text((rd.hour + rd.minute / 60.0) * kDegPerHour, 0))
            << tr(" Hat das JULIANISCHE Datum : %1").arg(radix.jd_ut, 13, 'f', 5);
  }
  for (;;) {
    // his inputbox$(160,eaz$,"JULIANISCHES DATUM ( 'JD' WELTZEIT ) EINGEBEN !","")
    const std::optional<double> jd =
        ask_number(this, tr("ZAHLEN-Eingabe !"), tr("JULIANISCHES DATUM ( 'JD' WELTZEIT ) EINGEBEN !"), 0.0, 1.0e8,
                   std::numeric_limits<double>::quiet_NaN(), 6, current);
    if (!jd) {
      return;
    }
    const CalendarDate d = calendar_date(*jd, cal);
    // his juld1 with somo gives the time arguments, the mean obliquity and
    // the nutation of the entered moment
    const TimeArguments t = time_arguments(*jd);
    const SunMoonState s = somo(t, d);
    const double eks = t.mean_obliquity_rad;
    const QStringList lines{
        tr(" Julianisches Jahrhundert ab 31.12.1899 12h = %1").arg(t.t1, 13, 'f', 10),
        tr(" Mittlere Ekliptik-Schiefe = %1°  ( OHNE NUTATION ! )").arg(eks * kRadToDeg, 10, 'f', 4),
        tr(" Apparente Ekliptik-Schiefe = %1° ( MIT NUTATION !) ").arg((eks + s.deps) * kRadToDeg, 10, 'f', 4),
        // the tester's French ephemeris counts days from 1900 January 0.5,
        // the epoch of his t1, so 1.1.2000 12h reads 36525 like his example
        tr(" Auréas-Zählung ( Tage seit 31.12.1899 12h ) = %1").arg(*jd - kJdEpoch1900, 0, 'f', 4),
        QString(),
        tr("JD = %1 Julianische Tage Welt_Zeit entsprechen :").arg(*jd, 15, 'f', 6),
        tr("Datum : %1 %2 | Welt-Zeit : %3")
            .arg(datum3_text(d), jul, homise_text((d.hour + d.minute / 60.0) * kDegPerHour, 0)),
        "  ",
        tr("( JD = 0 entspricht : 12 h  am  1.1.4713 v.Chr. )")};
    // his be$ = " Beenden", wr$ = "Weiter"
    if (ChoiceDialog::ask(this, tr("AUSWAHL"), lines, {tr(" Beenden"), tr("Weiter")}, 0) != 1) {
      return;
    }
  }
}

// the date of hi2 and the true obliquity his juld1 and somo give it
std::optional<std::pair<CalendarDate, double>> MainWindow::convert_date() {
  // his CLR ta,mo,ja,ho,mi, @hi2(10,2)
  const std::optional<CalendarDate> d = ask_date(this, tr("DATUM-ZEIT-EINGABE"), CalendarDate{});
  if (!d) {
    return std::nullopt;
  }
  const CalendarDate day{d->day, d->month, d->year, 0.0, 0.0};
  const double jd = julian_day(day, current_settings().calendar);
  return std::make_pair(day, somo(time_arguments(jd), day).ekls);
}

// ported from arde_eleb, AR-DE aus EL-EB
void MainWindow::arde_from_eleb() {
  const auto date = convert_date();
  if (!date) {
    return;
  }
  // his "EKLIPTIKALE KOORDINATEN " + eg$ + " !"
  const auto pos = ask_sky_coordinates(this, false, tr("EKLIPTIKALE KOORDINATEN Eingeben !"));
  if (!pos) {
    return;
  }
  const auto [ll, bb] = *pos;
  const Equatorial eq = ecliptic_to_equatorial(ll * kDegToRad, bb * kDegToRad, date->second);
  const double ar = norm_rad(eq.ra) * kRadToDeg;
  const double de = eq.dec * kRadToDeg;
  const QString r = de < 0.0 ? "S" : "N";
  const QStringList lines{tr("Ekliptikale Länge = %1°").arg(ll, 8, 'f', 4),
                          tr("Ekliptikale Breite= %1°").arg(bb, 8, 'f', 4),
                          tr("Entsprechen beim Datum %1 :").arg(datum3_text(date->first)),
                          tr(" Rektaszension  AR  = %1°  = %2").arg(ar, 8, 'f', 4).arg(homise_text(ar, 2)),
                          tr(" Deklination    DE  = %1° %2 = %3 %2")
                              .arg(std::abs(de), 6, 'f', 3)
                              .arg(r)
                              .arg(grmise_text(de))};
  QMessageBox::information(this, tr("Ergebnis :"), lines.join(QChar(0x0A)));
}

// ported from eleb_arde, EL-EB aus AR-DE
void MainWindow::eleb_from_arde() {
  const auto date = convert_date();
  if (!date) {
    return;
  }
  // his "ÄQUATORIALE KOORDINATEN " + eg$ + " !"
  const auto pos = ask_sky_coordinates(this, true, tr("ÄQUATORIALE KOORDINATEN Eingeben !"));
  if (!pos) {
    return;
  }
  const auto [ll, bb] = *pos;
  const Ecliptic ec = equatorial_to_ecliptic(ll * kDegToRad, bb * kDegToRad, date->second);
  const double la = norm_rad(ec.lon);
  const double br = ec.lat * kRadToDeg;
  const QString r = br < 0.0 ? "S" : "N";
  const QStringList lines{tr("Rektaszension AR = %1°").arg(ll, 8, 'f', 4),
                          tr("Deklination   DE = %1°").arg(bb, 8, 'f', 4),
                          tr("Entsprechen beim Datum  %1 :").arg(datum3_text(date->first)),
                          tr(" Eklipt.Länge     EL  = %1  = %2°").arg(zodiac(la)).arg(la * kRadToDeg, 8, 'f', 4),
                          tr(" Eklipt.Breite    EB  = %1° %2 = %3 %2")
                              .arg(std::abs(br), 6, 'f', 3)
                              .arg(r)
                              .arg(grmise_text(br))};
  QMessageBox::information(this, tr("Ergebnis :"), lines.join(QChar(0x0A)));
}

// ported from lt_ut and ut_lt, the mean local time of the longitude
void MainWindow::local_time_convert(bool from_ut) {
  // his Eingabe der LÄNGE genügt !
  const auto where = ask_geo_coordinates(this, {tr("Eingabe der LÄNGE genügt !")});
  if (!where) {
    return;
  }
  // his a37dat(32,"GREENWICH-ZEIT" bzw. "ORTS-ZEIT","DATUM-ZEIT-EINGABE")
  const std::optional<CalendarDate> d =
      ask_date(this, tr("DATUM-ZEIT-EINGABE"), convert_start(), from_ut ? tr("GREENWICH-ZEIT") : tr("ORTS-ZEIT"));
  if (!d) {
    return;
  }
  const Calendar cal = current_settings().calendar;
  const double jd = julian_day(*d, cal);
  // his oz = gz + gl / 15
  const double shift = where->first / kDegPerCircle;
  QStringList lines;
  if (from_ut) {
    lines << tr("Greenwichzeit GMT = UT :") << tr("Datum / Zeit ( UT )    : ") + ut_etd_text(jd, cal) << QString()
          << tr("Mittlere Ortszeit = LMT :") << tr("Datum / Zeit ( LMT )   : ") + ut_etd_text(jd + shift, cal);
  } else {
    lines << tr("Mittlere Ortszeit = LMT :") << tr("Datum / Zeit ( LMT )    : ") + ut_etd_text(jd, cal) << QString()
          << tr("Greenwichzeit GMT = UT  :") << tr("Datum / Zeit ( UT )     : ") + ut_etd_text(jd - shift, cal);
  }
  QMessageBox::information(this, tr("Ergebnis :"), lines.join(QChar(0x0A)));
}

}  // namespace horcom
