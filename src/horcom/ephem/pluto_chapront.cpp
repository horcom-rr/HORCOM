// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/ephem/pluto_chapront.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"
#include "horcom/ephem/precession.hpp"

namespace horcom {

namespace {

// the original plelempl, equinoctial corrections to the mean elements
Orbit plelempl(double sp, double sa, double sq, double sm, double sk, double sl, double t11) {
  //RR 1.Näherung
  const double ap0 = 39.544674;
  const double kp0 = -0.178744;
  const double qp0 = -0.051699;
  const double ep0 = 4.166868;
  const double hp0 = -0.173426;
  const double pp0 = 0.139782;
  //RR PRO JAHRH.
  const double nup = 2.53333;
  const double puu = kArcsecToRad;
  const double ap = ap0 * (1.0 + puu * sa);
  const double kp = kp0 + puu * sk;
  const double qp = qp0 + puu * sq;
  const double hp = hp0 + puu * sm;
  const double pp = pp0 + puu * sp;
  Orbit o;
  o.a = ap;
  o.o = atn(pp, qp);
  o.p = norm_rad(atn(hp, kp) - o.o);
  o.e = std::sqrt(hp * hp + kp * kp);
  o.i = 2.0 * std::asin(std::sqrt(pp * pp + qp * qp));
  o.mel = norm_rad(nup * t11 + ep0 + puu * sl);
  o.man = norm_rad(o.mel - o.p - o.o);
  return o;
}

}  // namespace

ChaprontPluto pluto_chapront(const TimeArguments& t, double ekls) {
  const double t11 = t.t11;
  const double t21 = t.t21;
  const double t31 = t.t31;
  const double pu = kDegToRad;
  const double mu = 1.2711012;

  //RR MEEUS
  const double j = pu * norm_deg(34.351484 + 3034.9056746 * t11 - 0.00008501 * t21 + 4.0e-09 * t.t3);
  const double s = pu * norm_deg(50.077471 + 1222.1137943 * t11 + 0.00021004 * t21 - 1.9e-16 * t31);
  const double u = pu * norm_deg(314.055005 + 428.4669983 * t11 - 4.86e-06 * t21 + 6.0e-09 * t31);
  const double p = norm_rad(2.53333 * t11 + 4.166868);

  const double cj1 = std::cos(j);
  const double cj2 = std::cos(2.0 * j);
  const double cs1 = std::cos(s);
  const double cp1 = std::cos(p);
  const double cp2 = std::cos(2.0 * p);
  const double cjp = std::cos(j - p);
  const double cjp2 = std::cos(j - 2.0 * p);
  const double cjp3 = std::cos(j - 3.0 * p);
  const double cjp4 = std::cos(j - 4.0 * p);
  const double cj2p = std::cos(2.0 * j - p);
  const double csp = std::cos(s - p);
  const double csp2 = std::cos(s - 2.0 * p);
  const double csp3 = std::cos(s - 3.0 * p);
  const double cup = std::cos(u - p);
  const double cup3 = std::cos(u - 3.0 * p);
  const double cu2p6 = std::cos(2.0 * u - 6.0 * p);
  const double cu3p9 = std::cos(3.0 * u - 9.0 * p);
  const double cjpp = std::cos(j + p);

  const double sj1 = std::sin(j);
  const double sj2 = std::sin(2.0 * j);
  const double ss1 = std::sin(s);
  const double sp1 = std::sin(p);
  const double sp2 = std::sin(2.0 * p);
  const double sjp = std::sin(j - p);
  const double sjp2 = std::sin(j - 2.0 * p);
  const double sjp3 = std::sin(j - 3.0 * p);
  const double sjp4 = std::sin(j - 4.0 * p);
  const double sj2p = std::sin(2.0 * j - p);
  const double ssp = std::sin(s - p);
  const double ssp2 = std::sin(s - 2.0 * p);
  const double ssp3 = std::sin(s - 3.0 * p);
  const double sup = std::sin(u - p);
  const double sup3 = std::sin(u - 3.0 * p);
  const double su2p6 = std::sin(2.0 * u - 6.0 * p);
  const double su3p9 = std::sin(3.0 * u - 9.0 * p);
  const double sjpp = std::sin(j + p);

  double sa = 1083.0 * cjp - 199.0 * cjp2 + 255.0 * csp - 99.0 * cp1 - 50.0 * csp2 + 2.0 * cjp3 + 50.0 * cj2p + 16.0 * cjp4 + 32.0 * cup - 21.0 * cup3 - 16.0 * cu2p6 + csp3;
  sa = sa - sjp + 193.0 * sjp2 - 97.0 * sp1 + 48.0 * ssp2 - 80.0 * sjp3 + 13.0 * sj2p + 35.0 * sp2 + 18.0 * sjp4 - 5.0 * sup3 + 8.0 * su2p6 - 21.0 * ssp3;
  sa = sa + 19.77 * t11 - 0.314 * t21 - 0.0019 * t31;

  double sl = 1037.0 * sjp - 628.0 * sup3 + 217.0 * sj1 - 237.0 * su2p6 + 222.0 * ssp - 66.0 * sjp2 + 48.0 * ss1 + 49.0 * sj2p + 26.0 * su3p9 + 19.0 * sp1 - 13.0 * ssp2 + 23.0 * sup + 8.0 * sj2;
  sl = sl + 34.0 * cup3 - 212.0 * cj1 - 114.0 * cu2p6 - 64.0 * cjp2 - 47.0 * cs1 - 13.0 * cj2p - 24.0 * cu3p9 - 17.0 * cp1 - 13.0 * csp2 - 13.0 * cj2;
  const double mt = mu * t11;
  sl = sl + (-15.57 - 3.153 * t11 - 0.017 * t21) * std::cos(mt);
  sl = sl + (25.83 - 0.87 * t11 + 0.005 * t21) * std::sin(mt);
  sl = sl + (15.82 - 4.864 * t11) * std::cos(2.0 * mt) + (24.18 - 0.251 * t11) * std::sin(2.0 * mt);
  sl = sl + (-14.74 - 0.082 * t11) * std::cos(3.0 * mt) + (6.08 - 0.149 * t11) * std::sin(3.0 * mt);
  sl = sl + (-4.08 + 0.08 * t11) * std::cos(4.0 * mt) + (-4.5 - 0.026 * t11) * std::sin(4.0 * mt);
  sl = sl + (1.95 + 0.021 * t11) * std::cos(5.0 * mt) + (-3.87 + 0.073 * t11) * std::sin(5.0 * mt);
  sl = sl + (2.47 - 0.046 * t11) * std::cos(6.0 * mt) + (1.48 - 0.02 * t11) * std::sin(7.0 * mt);
  sl = sl + 46.506 * t11 - 37.764 * t21 + 0.4012 * t31;

  const double sq = 14.0 * cjp + 13.0 * cjpp + 8.0 * cj1 - 37.0 * sjp - 33.0 * sjpp - 19.0 * sj1 + 1.045 * t11 - 0.006 * t21;
  const double sp = -36.0 * cjp + 33.0 * cjpp + 7.0 * cj1 - 14.0 * sjp + 12.0 * sjpp - 19.0 * sj1 + 0.44 * t11 + 0.006 * t21;
  double sk = 785.0 * cj1 + 255.0 * cp1 + 248.0 * cjp2 + 173.0 * cs1 + 48.0 * cjp - 70.0 * cjp3 - 47.0 * cp2 + 62.0 * csp2 + 37.0 * cj2 + 23.0 * cjpp;
  sk = sk + -18.0 * csp3 + cjp4 + 11.0 * csp + 21.0 * cu2p6 - 10.0 * cup3;
  sk = sk + 11.0 * sj1 - 3.0 * sp1 - 5.0 * sjp2 + 2.0 * ss1 + 96.0 * sjp + 69.0 * sjp3 - 45.0 * sp2 - ssp2 + 10.0 * sj2 + 22.0 * sjpp;
  sk = sk + 18.0 * ssp3 - 34.0 * sjp4 + 23.0 * ssp + 12.0 * su2p6 + 19.0 * sup3 - 16.256 * t11 + 0.12 * t21;
  double sm = 762.0 * sj1 + 256.0 * sp1 - 248.0 * sjp2 + 169.0 * ss1 - 100.0 * sjp + 71.0 * sjp3 - 47.0 * sp2 - 62.0 * ssp2;
  sm = sm + 11.0 * cj1 - 3.0 * cp1 + 5.0 * cjp2 + 2.0 * cs1 + 46.0 * cjp + 67.0 * cjp3 + 46.0 * cp2 + csp2 - 9.0 * cj2 - 22.0 * cjpp + 17.0 * csp3;
  sm = sm - 33.0 * cjp4 + 11.0 * csp + 12.0 * cu2p6 + 23.0 * cup3;
  sm = sm + 36.0 * sj2 + 22.0 * sjpp + 18.0 * ssp3 - sjp4 - 24.0 * ssp - 21.0 * su2p6 + 9.0 * sup3 + 1.973 * t11 + 0.14 * t21;

  ChaprontPluto out;
  out.elements = plelempl(sp, sa, sq, sm, sk, sl, t11);
  // the original pl_praez, Kepler at J2000 then Newcomb precession to date
  out.pos = kepler(out.elements);
  out.r = out.pos.r;
  const Equatorial eq = ecliptic_to_equatorial(out.pos.hel, out.pos.heb, ekls);
  double ar = 0.0;
  double de = 0.0;
  precess_newcomb(t.jd, t.tropical_year_days, kJdJ2000, eq.ra, eq.dec, ar, de);
  const Ecliptic ec = equatorial_to_ecliptic(ar, de, ekls);
  out.hel = ec.lon;
  out.heb = ec.lat;
  return out;
}

}  // namespace horcom
