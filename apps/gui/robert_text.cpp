// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "robert_text.hpp"

#include <QDate>
#include <cmath>

#include "horcom/chart/signs.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

// ported from date_form and vchr
QString datum3_text(const CalendarDate& d) {
  //RR vc$ = "   " oder " vC"
  const QString year = d.year > 0 ? QString::number(d.year) + "   " : QString::number(1 - d.year) + " vC";
  return QString::asprintf("%2d.%2d.", d.day, d.month) + year;
}

// ported from date_form, datum$ = a$ + "." + b$ + "." + c$
QString datum_text(const CalendarDate& d) {
  //RR RSET c$ = RIGHT$(STR$(ja),2), before Christ janz with v$
  const int y = d.year > 0 ? d.year : 1 - d.year;
  const QString digits = QString::number(y).right(2);
  return QString::asprintf("%2d.%2d.", d.day, d.month) + digits.rightJustified(2, ' ') + (d.year > 0 ? "" : "v");
}

// ported from grmise
QString grmise_text(double deg) {
  //RR g = ABS(gd) + kk
  const double g = std::abs(deg) + kEps;
  int a = static_cast<int>(g);
  int b = static_cast<int>(60.0 * (g - a));
  int c = static_cast<int>(std::lround(60.0 * (60.0 * (g - a) - b)));
  if (c == 60) {
    c = 0;
    ++b;
  }
  if (b == 60) {
    b = 0;
    ++a;
  }
  return QString::asprintf("%3d\xC2\xB0%2d'%2d\"", a, b, c);
}

// ported from grmise, his whole minutes carried nothing
QString grmi_text(double deg, int decimals) {
  const double g = std::abs(deg) + kEps;
  int a = static_cast<int>(g);
  const double scale = decimals > 0 ? 10.0 : 1.0;
  double m = std::round(60.0 * (g - a) * scale) / scale;
  if (m >= 60.0) {
    m = 0.0;
    ++a;
  }
  return decimals > 0 ? QString::asprintf("%3d\xC2\xB0%5.1f'", a, m)
                      : QString::asprintf("%3d\xC2\xB0%2d'", a, static_cast<int>(m));
}

// ported from homise, his seconds with decimals never carried
QString homise_text(double deg, int decimals) {
  const double scale = std::pow(10.0, decimals);
  //RR ww = up * (w + kk), c1 = INT(ww / 15)
  const long long units = std::llround((std::abs(deg) + kEps) / kDegPerHour * 3600.0 * scale);
  const long long whole = units / static_cast<long long>(scale);
  const double sec = static_cast<double>(units % (60 * static_cast<long long>(scale))) / scale;
  const long long h = whole / 3600;
  const long long m = (whole / 60) % 60;
  return decimals > 0 ? QString::asprintf("%2lldh %2lldm %*.*fs", h, m, 3 + decimals, decimals, sec)
                      : QString::asprintf("%2lldh %2lldm %2llds", h, m, whole % 60);
}

QString zodiac(double rad) {
  const ZodiacSplit z = split_zodiac(rad, true);
  return QString::asprintf("%2d %s %02d'%02d\"", z.deg, kSignTag[z.sign], z.min, z.sec);
}

CalendarDate today_date() {
  const QDate d = QDate::currentDate();
  return {d.day(), d.month(), d.year(), 0.0, 0.0};
}

}  // namespace horcom
