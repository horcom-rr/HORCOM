// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/ephem/eclipses.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/calendar.hpp"
#include "horcom/time/delta_t.hpp"

namespace horcom {

namespace {

// the shared lunation arguments, the original finst_0
struct LunationArgs {
  double jde = 0.0;  // mean syzygy in dynamical time
  double t21 = 0.0;
  double m1 = 0.0;  // the sun's anomaly
  double m2 = 0.0;  // the moon's anomaly
  double fm = 0.0;  // the argument of latitude
  double o2 = 0.0;  // the node
  double e = 0.0;
  double e2 = 0.0;
};

LunationArgs lunation_args(double k) {
  LunationArgs a;
  const double t11 = k / 1236.85;
  a.t21 = t11 * t11;
  const double t31 = a.t21 * t11;
  const double t41 = t31 * t11;
  a.jde = 2451550.09765 + 29.530588853 * k + 0.0001337 * a.t21 - 1.5e-07 * t31 + 7.3e-10 * t41;
  a.m1 = norm_rad(kDegToRad * (2.5534 + 29.10535669 * k - 0.0000218 * a.t21 - 1.1e-07 * t31));
  // his finst_0 carried 0.1017438, Meeus prints 0.0107438 for the T
  // squared term of the moon's anomaly, the typo moved old lunations by
  // an hour and more
  a.m2 = norm_rad(kDegToRad * (201.5643 + 385.81693528 * k + 0.0107438 * a.t21 + 0.00001239 * t31 - 5.8e-08 * t41));
  a.fm = norm_rad(kDegToRad * (160.7108 + 390.67050274 * k - 0.0016341 * a.t21 - 2.27e-06 * t31 + 1.1e-08 * t41));
  a.o2 = norm_rad(kDegToRad * (124.7746 - 1.5637558 * k + 0.0020691 * a.t21 + 2.15e-06 * t31));
  a.e = 1.0 - 0.002516 * t11 - 7.4e-06 * a.t21;
  a.e2 = a.e * a.e;
  return a;
}

// the exact syzygy moment in dynamical time, the original neu_voll
double syzygy_td(double k, bool full) {
  const LunationArgs a = lunation_args(k);
  const double sm1 = std::sin(a.m1);
  const double sm2 = std::sin(a.m2);
  const double sm12 = std::sin(a.m2 + a.m1);
  const double sm12_ = std::sin(a.m2 - a.m1);
  const double s2m2 = std::sin(2.0 * a.m2);
  const double s2m1 = std::sin(2.0 * a.m1);
  const double a1 = std::sin(kDegToRad * (299.77 + 0.107408 * k - 0.009173 * a.t21));
  const double a2 = std::sin(kDegToRad * (251.88 + 0.016321 * k));
  const double a3 = std::sin(kDegToRad * (251.83 + 26.651886 * k));
  const double a4 = std::sin(kDegToRad * (349.42 + 36.412478 * k));
  const double a5 = std::sin(kDegToRad * (84.66 + 18.206239 * k));
  const double a6 = std::sin(kDegToRad * (141.74 + 53.303771 * k));
  const double a7 = std::sin(kDegToRad * (207.14 + 2.453732 * k));
  const double a8 = std::sin(kDegToRad * (154.84 + 7.30686 * k));
  const double a9 = std::sin(kDegToRad * (34.52 + 27.261239 * k));
  const double a10 = std::sin(kDegToRad * (207.19 + 0.121824 * k));
  double dlg = -0.00111 * std::sin(a.m2 - 2.0 * a.fm) - 0.00057 * std::sin(a.m2 + 2.0 * a.fm);
  dlg += 0.00056 * a.e * std::sin(2.0 * a.m2 + a.m1) - 0.00042 * std::sin(3.0 * a.m2) +
         0.00042 * a.e * std::sin(a.m1 + 2.0 * a.fm) + 0.00038 * a.e * std::sin(a.m1 - 2.0 * a.fm);
  dlg += -0.00024 * a.e * std::sin(2.0 * a.m2 - a.m1) - 0.00017 * std::sin(a.o2) - 0.00007 * std::sin(a.m2 + 2.0 * a.m1);
  dlg += 0.00004 * std::sin(2.0 * a.m2 - 2.0 * a.fm) + 0.00004 * std::sin(3.0 * a.m1);
  dlg += 0.000325 * a1 + 0.000165 * a2 + 0.000164 * a3 + 0.000126 * a4 + 0.00011 * a5 + 0.000062 * a6 + 0.00006 * a7;
  dlg += 0.000056 * a8 + 0.000047 * a9 + 0.000042 * a10;
  double dl = 0.0;
  if (!full) {
    //RR Neumond
    dl = -0.4072 * sm2 + 0.17241 * a.e * sm1 + 0.01608 * s2m2 + 0.01039 * std::sin(2.0 * a.fm) +
         0.00739 * a.e * sm12_ - 0.00514 * a.e * sm12 + 0.00208 * a.e2 * s2m1;
  } else {
    dl = -0.40614 * sm2 + 0.17302 * a.e * sm1 + 0.01614 * s2m2 + 0.01043 * std::sin(2.0 * a.fm) +
         0.00734 * a.e * sm12_ - 0.00515 * a.e * sm12 + 0.00209 * a.e2 * s2m1;
  }
  return a.jde + dl + dlg;
}

// gamma and u of the eclipse rules, the original finst_1
void eclipse_quantities(double k, double& g, double& u) {
  const LunationArgs a = lunation_args(k);
  const double f1 = a.fm - kDegToRad * 0.02665 * std::sin(a.o2);
  const double sm1 = std::sin(a.m1);
  const double sm2 = std::sin(a.m2);
  const double sm12 = std::sin(a.m2 + a.m1);
  const double sm12_ = std::sin(a.m2 - a.m1);
  const double s2m2 = std::sin(2.0 * a.m2);
  const double s2m1 = std::sin(2.0 * a.m1);
  const double cm1 = std::cos(a.m1);
  const double cm2 = std::cos(a.m2);
  const double cm12 = std::cos(a.m2 + a.m1);
  const double cm12_ = std::cos(a.m2 - a.m1);
  const double c2m2 = std::cos(2.0 * a.m2);
  const double c2m1 = std::cos(2.0 * a.m1);
  const double p = 0.207 * a.e * sm1 + 0.0024 * a.e * s2m1 - 0.0392 * sm2 + 0.0116 * s2m2 -
                   0.0073 * a.e * sm12 + 0.0067 * a.e * sm12_ + 0.0118 * std::sin(2.0 * f1);
  const double q = 5.2207 - 0.0048 * a.e * cm1 + 0.002 * a.e * c2m1 - 0.3299 * cm2 -
                   0.006 * a.e * cm12 + 0.0041 * a.e * cm12_;
  const double w = std::abs(std::cos(f1));
  g = (p * std::cos(f1) + q * std::sin(f1)) * (1.0 - 0.0048 * w);
  u = 0.0059 + 0.0046 * a.e * cm1 - 0.0182 * cm2 + 0.0004 * c2m2 - 0.0005 * cm12;
}

// the moment of greatest eclipse in dynamical time, the dj series of
// finst_1, the solar coefficients for a whole k
double eclipse_maximum_td(double k) {
  const LunationArgs a = lunation_args(k);
  const double f1 = a.fm - kDegToRad * 0.02665 * std::sin(a.o2);
  const double a1 = kDegToRad * (299.77 + 0.107408 * k - 0.009173 * a.t21);
  const double sm1 = std::sin(a.m1);
  const double sm2 = std::sin(a.m2);
  const double sm12 = std::sin(a.m2 + a.m1);
  const double sm12_ = std::sin(a.m2 - a.m1);
  const double s2m2 = std::sin(2.0 * a.m2);
  const double s2m1 = std::sin(2.0 * a.m1);
  //RR Korrektur in Tagen für Sonnenfinsternis
  double dj = k == std::floor(k) ? -0.4075 * sm2 + 0.1721 * a.e * sm1 : -0.4065 * sm2 + 0.1727 * a.e * sm1;
  dj += 0.0161 * s2m2 - 0.0097 * std::sin(2.0 * f1);
  dj += 0.0073 * a.e * sm12_ - 0.005 * a.e * sm12 - 0.0023 * std::sin(a.m2 - 2.0 * f1);
  dj += 0.0021 * a.e * s2m1 + 0.0012 * std::sin(a.m2 + 2.0 * f1) + 0.0006 * a.e * std::sin(2.0 * a.m2 + a.m1);
  dj += -0.0004 * std::sin(3.0 * a.m2) - 0.0003 * a.e * std::sin(a.m1 + 2.0 * f1) + 0.0003 * std::sin(a1) -
        0.0002 * a.e * std::sin(a.m1 - 2.0 * f1);
  dj += -0.0002 * a.e * std::sin(2.0 * a.m2 - a.m1) - 0.0002 * std::sin(a.o2);
  return a.jde + dj;
}

double to_ut(double jd_td) {
  return jd_td - delta_t_minutes(jd_td) / kMinutesPerDay;
}

// his screen letters for one syzygy
std::string classify(double k, bool full, bool& eclipse) {
  double g = 0.0;
  double u = 0.0;
  eclipse_quantities(k, g, u);
  eclipse = false;
  if (!full) {
    const double g1 = std::abs(g);
    //RR FINSTERNIS
    if (g1 >= 1.5433 + u) {
      return {};
    }
    const std::string ns = g > 0 ? " N" : " S";
    if (g > -0.9972 && g < 0.9972) {
      std::string art;
      if (u < 0.0) {
        art = " TOT";
      } else if (u > 0.0 && u < 0.0047) {
        art = u < 0.00464 * std::sqrt(1.0 - g * g) ? " TOT" : " RF";
      } else if (u > 0.0047) {
        art = " RF";
      }
      eclipse = true;
      return "ZT" + art + ns;
    }
    if (g1 > 0.9972 && g1 < 1.5433 + u) {
      std::string art;
      if (g1 > 0.9972 && g1 < 1.026) {
        art = " RF";
        if (g1 < 0.9972 + std::abs(u)) {
          art = " TOT";
        }
      }
      eclipse = true;
      return "EX" + art + ns;
    }
    return {};
  }
  //RR HALBSCHATTEN / KERNSCHATTEN
  const double hs = (1.5573 + u - std::abs(g)) / 0.545;
  const double ks = (1.0128 - u - std::abs(g)) / 0.545;
  if (ks > 0.0) {
    eclipse = true;
    return "KERNSCH";
  }
  if (hs > 0.0) {
    eclipse = true;
    return "HALBSCH";
  }
  return {};
}

}  // namespace

double lunation_number(const CalendarDate& d) {
  return std::floor((d.year + (d.month - 1) / 12.0 + d.day / 365.25 - 2000.0) * 12.3685);
}

// ported from HORCOM neu_voll with finst_1
Lunation lunation_at(double k) {
  Lunation row;
  row.k = k;
  row.full = k != std::floor(k);
  row.jd_ut = to_ut(syzygy_td(k, row.full));
  row.kind = classify(k, row.full, row.eclipse);
  if (row.eclipse) {
    row.max_ut = to_ut(eclipse_maximum_td(k));
  }
  return row;
}

// ported from HORCOM finst, k = k1 - 2 and k1 - 2.5 before the first INC
std::vector<Lunation> lunations(double jd_start_ut, int count, bool full_moons) {
  std::vector<Lunation> out;
  double k = lunation_number(calendar_date(jd_start_ut)) - 2.0;
  if (full_moons) {
    k -= 0.5;
  }
  for (int i = 0; i < count; ++i) {
    k += 1.0;
    out.push_back(lunation_at(k));
  }
  return out;
}

}  // namespace horcom
