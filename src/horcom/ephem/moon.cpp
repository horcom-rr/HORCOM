// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/ephem/moon.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

struct MoonMainTerm {
  int d;
  int m;
  int mo;
  int fm;
  double l;
  double r;
};

struct MoonLatTerm {
  int d;
  int m;
  int mo;
  int fm;
  double b;
};

#include "horcom/ephem/moon_tables.inc"

}  // namespace

//RR MEEUS NEU
MoonPosition moon_position(const TimeArguments& t, const SunMoonState& s) {
  const double t11 = t.t11;
  const double t21 = t.t21;
  const double t31 = t.t31;
  const double t41 = t.t41;
  MoonPosition out;
  out.mel = norm_rad(kDegToRad * (218.3164591 + 481267.88134236 * t11 - 0.0013268 * t21 + 1.855835e-6 * t31 - 1.53388e-8 * t41));
  const double a1 = norm_rad(kDegToRad * (119.75 + 131.849 * t11));
  const double a2 = norm_rad(kDegToRad * (53.09 + 479264.29 * t11));
  const double a3 = norm_rad(kDegToRad * (313.45 + 481266.484 * t11));
  const double e = 1.0 - 0.002516 * t11 - 0.0000074 * t21;
  const double e2 = e * e;

  //RR Ermittlung der Geschw.:
  //RR Ableitungen:
  const double pup = kDegPerCenturyToRad;
  const double a1p = norm_rad(pup * 131.849);
  const double a2p = norm_rad(pup * 479264.29);
  const double a3p = norm_rad(pup * 481266.484);
  const double mel1p = norm_rad(pup * (36000.7698231 + 0.00030368 * t11 + 2.1e-08 * t21));
  const double p1p = pup * (1.7195269 + 0.00045962 * t11 + 4.99e-07 * t21);
  const double man1p = norm_rad(mel1p - p1p);
  const double mel2p = norm_rad(pup * (481267.88134236 - 0.0013268 * t11 + (t21 / 538841.0) - (t31 / 65194000.0)));
  const double man2p = norm_rad(pup * (477198.8676313 + 0.008997 * t11 + (t21 / 69699.0) - (t31 / 14712000.0)));
  const double dmp = norm_rad(pup * (445267.1115168 - 0.00163 * t11 + (t21 / 545868.0) - (t31 / 113065000.0)));
  const double fmp = norm_rad(pup * (483202.0175273 - 0.0034029 * t11 - (t21 / 3526000.0) + (t31 / 863310000.0)));

  double lae = 0.0;
  double rdv = 0.0;
  double laep = 0.0;
  double rdvp = 0.0;
  for (const MoonMainTerm& term : kMoonMain) {
    double l = term.l;
    double r = term.r;
    if (term.m == 1 || term.m == -1) {
      l *= e;
      r *= e;
    }
    if (term.m == 2 || term.m == -2) {
      l *= e2;
      r *= e2;
    }
    const double arg = term.d * s.moon_dm + term.m * s.sun_man + term.mo * s.moon_man + term.fm * s.moon_fm;
    const double sarg = std::sin(arg);
    const double carg = std::cos(arg);
    lae += l * sarg;
    rdv += r * carg;
    const double argp = term.d * dmp + term.m * man1p + term.mo * man2p + term.fm * fmp;
    laep += l * argp * carg;
    rdvp += -r * argp * sarg;
  }
  double bre = 0.0;
  double brep = 0.0;
  for (const MoonLatTerm& term : kMoonLat) {
    double b = term.b;
    if (term.m == 1 || term.m == -1) {
      b *= e;
    }
    if (term.m == 2 || term.m == -2) {
      b *= e2;
    }
    const double arg = term.d * s.moon_dm + term.m * s.sun_man + term.mo * s.moon_man + term.fm * s.moon_fm;
    const double sarg = std::sin(arg);
    const double carg = std::cos(arg);
    bre += b * sarg;
    const double argp = term.d * dmp + term.m * man1p + term.mo * man2p + term.fm * fmp;
    brep += b * argp * carg;
  }

  const double mel2 = out.mel;
  out.el = norm_rad(s.dpsi + mel2 + kDegToRad * (lae + 3958.0 * std::sin(a1) + 1962.0 * std::sin(mel2 - s.moon_fm) + 318.0 * std::sin(a2)) * 1e-6);
  out.eb = s.deps + kDegToRad * (bre - 2235.0 * std::sin(mel2) + 382.0 * std::sin(a3) + 175.0 * std::sin(a1 - s.moon_fm) + 175.0 * std::sin(a1 + s.moon_fm) + 127.0 * std::sin(mel2 - s.moon_man) - 115.0 * std::sin(mel2 + s.moon_man)) * 1e-6;
  out.r = (385000.56 + rdv / 1000.0) / kKmPerAu;
  out.parallax = std::asin(0.0000426345151 / out.r);  //RR Parall

  const double el = out.el;
  const double eb = out.eb;
  const double r2 = out.r;
  const double sel = std::sin(el);
  const double cel = std::cos(el);
  const double seb = std::sin(eb);
  const double ceb = std::cos(eb);
  out.x[0] = r2 * ceb * cel;
  out.x[1] = r2 * ceb * sel;
  out.x[2] = r2 * seb;

  //RR Ableitungen:
  const double elp = norm_rad(mel2p + kDegToRad * (laep + 3958.0 * a1p * std::cos(a1) + 1962.0 * (mel2p - fmp) * std::cos(mel2 - s.moon_fm) + 318.0 * a2p * std::cos(a2)) * 1e-6);
  const double ebp = kDegToRad * (brep - 2235.0 * mel2p * std::cos(mel2) + 382.0 * a3p * std::cos(a3) + 175.0 * (a1p - fmp) * std::cos(a1 - s.moon_fm) + 175.0 * (a1p + fmp) * std::cos(a1 + s.moon_fm) + 127.0 * (mel2p - man2p) * std::cos(mel2 - s.moon_man) - 115.0 * (mel2p + man2p) * std::cos(mel2 + s.moon_man)) * 1e-6;
  const double rp = (rdvp / 1000.0) / kKmPerAu;
  out.v[0] = rp * ceb * cel - r2 * seb * ebp * cel - r2 * ceb * sel * elp;
  out.v[1] = rp * ceb * sel - r2 * seb * ebp * sel + r2 * ceb * cel * elp;
  out.v[2] = rp * seb + r2 * ceb * ebp;
  out.elp = elp;  //RR Bereich ca.712 bis 858 '/Tag
  return out;
}

LunarPoints lunar_points(const MoonPosition& m, const SunMoonState& s, const TimeArguments& t) {
  LunarPoints out;
  const double r4 = m.r;
  const double vv = m.v[0] * m.v[0] + m.v[1] * m.v[1] + m.v[2] * m.v[2];
  const double c1 = m.x[1] * m.v[2] - m.x[2] * m.v[1];
  const double c2 = m.x[2] * m.v[0] - m.x[0] * m.v[2];
  const double c3 = m.x[0] * m.v[1] - m.x[1] * m.v[0];
  const double c = std::sqrt(c1 * c1 + c2 * c2 + c3 * c3);  //RR Flächengeschw.
  const double cc = c * c;
  const double el11 = atn(c1, -c2);  //RR wahrer Knoten
  const double i4 = std::asin(std::sqrt(c1 * c1 + c2 * c2) / c);  //RR MONT. S.78 oben
  const double u4 = atn(m.x[2], std::sin(i4) * (m.x[0] * std::cos(el11) + m.x[1] * std::sin(el11)));  //RR S.78 mitte
  out.true_apogee_lat = std::asin(std::sin(u4) * std::sin(i4));  //RR Breite des Knotens
  const double g = 0.0002959122083;  //RR Grav.Konst
  const double mm = 3.0404332e-06;   //RR Masse Erde +Mond
  const double a4 = 1.0 / ((2.0 / r4) - (vv / g / mm));  //RR Grosse Halbachse
  const double p = cc / g / mm;  //RR Bahnpar.
  const double e4 = std::sqrt(1.0 - (p / a4));  //RR Exzentr.
  const double ea = atn(m.x[0] * m.v[0] + m.x[1] * m.v[1] + m.x[2] * m.v[2],
                        (1.0 - (r4 / a4)) * std::sqrt(a4 * g * mm));  //RR Exzentr.Anomalie
  const double wa = 2.0 * atn(std::sqrt(1.0 + e4) * std::tan(ea / 2.0), std::sqrt(1.0 - e4));  //RR Wahre Anom.
  const double el24 = norm_rad(kPi + u4 - wa + el11);  //RR Apogäum

  out.true_node = norm_rad(el11 + s.dpsi);
  out.true_apogee = norm_rad(el24 + s.dpsi);
  out.mean_node = norm_rad(s.moon_o + s.dpsi);
  out.mean_node_speed = -0.00092422029;
  out.mean_apogee = norm_rad(s.dpsi + kDegToRad * (180.0 + 83.3532430 + 4069.0137111 * t.t11 - 0.0103238 * t.t21 - t.t31 / 80053.0 - t.t41 / 18999000.0));
  out.mean_apogee_speed = kDegPerCenturyToRad * (4069.0137111 - 0.0103238 * t.t11 - t.t21 / 80053.0 - t.t31 / 18999000.0);
  const double u = norm_rad(out.mean_apogee - s.moon_o);
  const double i2 = 0.087943 * (1.0 + 0.0280505 * std::cos(2.0 * (s.moon_mel - s.moon_o)));
  out.mean_apogee_lat = std::asin(std::sin(u) * std::sin(i2));
  return out;
}

}  // namespace horcom
