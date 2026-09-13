// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/ephem/sunmoon.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

SunMoonState somo(const TimeArguments& t, const CalendarDate& date, bool heliocentric) {
  SunMoonState s;
  s.heliocentric = heliocentric;
  const double t1 = t.t1;
  const double t11 = t.t11;
  const double t21 = t.t21;
  const double t31 = t.t31;
  const double t41 = t.t41;
  // mean obliquity, then deps is added at the end exactly like the original
  s.ekls = 0.40931974745 - 0.0002271109689 * t1 - 2.86234e-08 * t.t2 + 8.779e-09 * t.t3;

  //RR SO
  // the split sums are kept exactly as the original writes them
  s.sun_mel = norm_rad(kDegToRad * (280.466449 + 36000.0 * t11 + 0.7698231 * t11 + 0.00030368 * t21 + 2.1e-08 * t31));
  s.sun_e = 0.01670862 - 0.000042037 * t11 - 1.236e-07 * t21 + 4.0e-11 * t31;
  s.sun_a = 1.000001018;
  s.sun_p = kDegToRad * (282.937348 + 1.7195269 * t11 + 0.00045962 * t21 + 4.99e-07 * t31);
  s.sun_man = norm_rad(s.sun_mel - s.sun_p);

  if (!heliocentric) {
    //RR MO
    s.moon_mel = norm_rad(kDegToRad * (218.3164591 + 481267.0 * t11 + 0.88134236 * t11 - 0.0013268 * t21 + (t31 / 538841.0) - (t41 / 65194000.0)));
    s.moon_man = norm_rad(kDegToRad * (134.9634114 + 477198.0 * t11 + 0.8676313 * t11 + 0.008997 * t21 + (t31 / 69699.0) - (t41 / 14712000.0)));
    s.moon_o = norm_rad(kDegToRad * (125.044555 - 1934.1361849 * t11 + 0.0020762 * t21 + (t31 / 467410.0) - (t41 / 60616000.0)));
    s.moon_dm = norm_rad(kDegToRad * (297.8502042 + 445267.0 * t11 + 0.1115168 * t11 - 0.00163 * t21 + (t31 / 545868.0) - (t41 / 113065000.0)));
    s.moon_fm = norm_rad(kDegToRad * (93.2720993 + 483202.0 * t11 + 0.0175273 * t11 - 0.0034029 * t21 - (t31 / 3526000.0) + (t41 / 863310000.0)));
    s.moon_i = 0.087943 * (1.0 + 0.0280505 * std::cos(2.0 * (s.moon_mel - s.moon_o)));
    s.moon_a = 0.002567555;

    const double a = date.year + (date.month - 1 + date.day / 30.0) / 12.0;
    s.sun_node = norm_rad(kDegToRad * (73.6667 + (a - 1850.0) * 0.01396));
    s.sun_node_opp = norm_rad(s.sun_node + kPi);

    const double o = s.moon_o;
    const double d = s.moon_dm;
    const double ms = s.sun_man;
    const double m = s.moon_man;
    const double f = s.moon_fm;
    const double o2 = 2.0 * o;
    const double d2 = 2.0 * d;
    const double f2 = 2.0 * f;
    const double pp = kArcsecToRad / 10000.0;
    double dpsi = -(171996.0 + 174.2 * t11) * std::sin(o) - (13187.0 + 1.6 * t11) * std::sin(-d2 + f2 + o2) - (2274.0 + 0.2 * t11) * std::sin(f2 + o2);
    dpsi += (2062.0 + 0.2 * t11) * std::sin(o2) + (1426.0 - 3.4 * t11) * std::sin(ms) + (712.0 + 0.1 * t11) * std::sin(m);
    dpsi += (-517.0 + 1.2 * t11) * std::sin(-d2 + ms + f2 + o2) - (386.0 + 0.4 * t11) * std::sin(f2 + o);
    dpsi += -301.0 * std::sin(m + f2 + o2) + (217.0 - 0.5 * t11) * std::sin(-d2 - ms + f2 + o2) - 158.0 * std::sin(-d2 + m);
    dpsi += (129.0 + 0.1 * t11) * std::sin(-d2 + f2 + o) + 123.0 * std::sin(-m + f2 + o2);
    s.dpsi = pp * dpsi;
    double deps = (92025.0 + 8.9 * t11) * std::cos(o) + (5736.0 - 3.1 * t11) * std::cos(-d2 + f2 + o2) + (977.0 - 0.5 * t11) * std::cos(f2 + o2);
    deps += (-895.0 + 0.5 * t11) * std::cos(o2) + (224.0 - 0.6 * t11) * std::cos(-d2 + ms + f2 + o2) + 200.0 * std::cos(f2 + o);
    deps += (129.0 - 0.1 * t11) * std::cos(m + f2 + o2);
    s.deps = pp * deps;

    s.mean_node = norm_rad(s.moon_o + s.dpsi);
  } else {
    s.dpsi = 0.0;
    s.deps = 0.0;
  }
  s.ekls += s.deps;
  return s;
}

double equation_of_time(const SunMoonState& s) {
  const double m = s.sun_man;
  const double l = s.sun_mel;
  const double e = s.sun_e;
  const double y = std::tan(s.ekls / 2.0) * std::tan(s.ekls / 2.0);
  return y * std::sin(2.0 * l) - 2.0 * e * std::sin(m) + 4.0 * e * y * std::sin(m) * std::cos(2.0 * l) - 0.5 * y * y * std::sin(4.0 * l) - 1.25 * e * e * std::sin(2.0 * m);
}

}  // namespace horcom
