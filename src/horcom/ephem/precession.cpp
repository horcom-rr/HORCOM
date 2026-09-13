// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/ephem/precession.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

void apply(std::array<double, 3>& x, const double a[3][3]) {
  const double x1 = a[0][0] * x[0] + a[0][1] * x[1] + a[0][2] * x[2];
  const double x2 = a[1][0] * x[0] + a[1][1] * x[1] + a[1][2] * x[2];
  const double x3 = a[2][0] * x[0] + a[2][1] * x[1] + a[2][2] * x[2];
  x = {x1, x2, x3};
}

}  // namespace

//RR nach MONT. 2. S 18  Elemente nach MEEUS
void precess_equatorial(std::array<double, 3>& x, double jdn, double jdaeq) {
  const double t0 = (jdaeq - kJdJ2000) / kDaysPerCentury;
  const double t = (jdn - jdaeq) / kDaysPerCentury;
  const double t2 = t * t;
  const double t3 = t2 * t;
  const double t02 = t0 * t0;
  const double f1 = kArcsecToRad;
  const double zeta = f1 * ((2306.2181 + 1.39656 * t0 - 0.000139 * t02) * t + (+0.30188 - 0.000344 * t0) * t2 + 0.017998 * t3);
  const double z = f1 * ((2306.2181 + 1.39656 * t0 - 0.000139 * t02) * t + (1.09468 + 0.000066 * t0) * t2 + 0.018203 * t3);
  const double teta = f1 * ((2004.3109 - 0.85330 * t0 - 0.000217 * t02) * t - (0.42665 + 0.000217 * t0) * t2 - 0.041833 * t3);
  const double a[3][3] = {
      {-std::sin(z) * std::sin(zeta) + std::cos(z) * std::cos(teta) * std::cos(zeta),
       -std::sin(z) * std::cos(zeta) - std::cos(z) * std::cos(teta) * std::sin(zeta),
       -std::cos(z) * std::sin(teta)},
      {std::cos(z) * std::sin(zeta) + std::sin(z) * std::cos(teta) * std::cos(zeta),
       std::cos(z) * std::cos(zeta) - std::sin(z) * std::cos(teta) * std::sin(zeta),
       -std::sin(z) * std::sin(teta)},
      {std::sin(teta) * std::cos(zeta), -std::sin(teta) * std::sin(zeta), std::cos(teta)}};
  apply(x, a);
}

//RR nach MONT. 2. S 18  Elemente nach MEEUS
void precess_ecliptic(std::array<double, 3>& x, double jdn, double jdaeq) {
  const double t0 = (jdaeq - kJdJ2000) / kDaysPerCentury;
  const double t = (jdn - jdaeq) / kDaysPerCentury;
  const double t2 = t * t;
  const double t3 = t2 * t;
  const double t02 = t0 * t0;
  const double f1 = kArcsecToRad;
  const double pik = f1 * ((47.0029 - 0.06603 * t0 + 0.000598 * t02) * t + (-0.03302 + 0.000598 * t0) * t2 + 0.000060 * t3);
  const double pig = f1 * (3600.0 * 174.876384 + 3289.4789 * t0 + 0.60622 * t02 - (869.8089 + 0.50491 * t0) * t + 0.03536 * t2);
  const double pp = f1 * ((5029.0966 + 2.22226 * t0 - 0.000042 * t02) * t + (1.11113 - 0.000042 * t0) * t2 - 0.000006 * t3);
  const double lag = pig + pp;
  const double a[3][3] = {
      {std::cos(lag) * std::cos(pig) + std::sin(lag) * std::cos(pik) * std::sin(pig),
       std::cos(lag) * std::sin(pig) - std::sin(lag) * std::cos(pik) * std::cos(pig),
       -std::sin(lag) * std::sin(pik)},
      {std::sin(lag) * std::cos(pig) - std::cos(lag) * std::cos(pik) * std::sin(pig),
       std::sin(lag) * std::sin(pig) + std::cos(lag) * std::cos(pik) * std::cos(pig),
       std::cos(lag) * std::sin(pik)},
      {std::sin(pik) * std::sin(pig), -std::sin(pik) * std::cos(pig), std::cos(pik)}};
  apply(x, a);
}

void precess_newcomb(double jd, double tja, double jda, double ar0, double de0, double& ar, double& de) {
  //RR Epoche 1900.0
  const double jdbez = kJdBessel1900;
  const double ta0 = (jda - jdbez) / (100.0 * tja);
  const double ta4 = (jd - jda) / (100.0 * tja);
  const double ta2 = ta4 * ta4;
  const double ta3 = ta4 * ta2;
  const double puu = kArcsecToRad;
  const double zf = puu * ((2304.25 + 1.396 * ta0) * ta4 + 0.302 * ta2 + 0.018 * ta3);
  const double zt = zf + puu * (0.791 * ta2 + 0.001 * ta3);
  const double te = puu * ((2004.682 - 0.853 * ta0) * ta4 - 0.426 * ta2 - 0.042 * ta3);
  const double a = std::cos(de0) * std::sin(ar0 + zf);
  const double b = std::cos(te) * std::cos(de0) * std::cos(ar0 + zf) - std::sin(te) * std::sin(de0);
  const double c = std::sin(te) * std::cos(de0) * std::cos(ar0 + zf) + std::cos(te) * std::sin(de0);
  de = std::asin(c);
  const double ar1 = atn(a, b);
  ar = zt + ar1;
}

}  // namespace horcom
