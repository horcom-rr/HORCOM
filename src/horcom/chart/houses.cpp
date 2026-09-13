// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/houses.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the original iteration threshold of plac1
constexpr double kPlacidusTolerance = 1.0e-06;

// cusp working state shared by the system routines like the original f()
struct Work {
  std::array<double, 14>& f;
  double armcb;
  double gg;    // latitude degrees
  double ekls;
};

// the original regio0, orders the first six cusps and mirrors the rest
void regio0(std::array<double, 14>& f) {
  for (int i = 1; i <= 5; ++i) {
    const auto fi = static_cast<std::size_t>(i);
    double w1 = f[fi];
    double w2 = f[fi + 1];
    vergl1(w1, w2);
    if (w1 > w2 && w1 - w2 < kPi && i != 3) {
      f[fi + 1] = norm_rad(f[fi + 1] + kPi);
    }
    if (w2 > w1 + kPi) {
      f[fi + 1] = norm_rad(f[fi + 1] + kPi);
    }
  }
  for (int g = 1; g <= 6; ++g) {
    if (g == 4) {
      ++g;
    }
    const auto gi = static_cast<std::size_t>(g);
    f[gi + 6] = norm_rad(f[gi] + kPi);
  }
}

// the original plac1, the fixed point iteration on the semi arc. x serves
// cusps 11 and 12, y serves cusps 2 and 3.
void plac1(const Work& w, double w1, double n1, double& x, double& y) {
  x = norm_rad(w1 + w.armcb);
  y = x;
  double x1 = 0.0;
  double y1 = 0.0;
  do {
    x1 = x;
    y1 = y;
    x = norm_rad(w.armcb + std::acos(-std::sin(x) * std::tan(w.ekls) * std::tan(kDegToRad * w.gg + kEps)) / n1);
    y = norm_rad(w.armcb + kPi - std::acos(std::sin(y) * std::tan(w.ekls) * std::tan(kDegToRad * w.gg + kEps)) / (n1 + kEps));
  } while (std::abs(x1 - x) >= kPlacidusTolerance || std::abs(y1 - y) >= kPlacidusTolerance);
}

// the original plac2, right ascension of a cusp to ecliptic longitude
double plac2(double ar, double ekls) {
  return atn(std::sin(ar), std::cos(ekls) * std::cos(ar));
}

void placidus(Work& w) {
  double x = 0.0;
  double y = 0.0;
  plac1(w, kPi / 6.0, 3.0, x, y);
  w.f[11] = plac2(norm_rad(x), w.ekls);
  plac1(w, kPi / 3.0, 1.5, x, y);
  w.f[12] = plac2(norm_rad(x), w.ekls);
  plac1(w, 2.0 * kPi / 3.0, 1.5, x, y);
  w.f[2] = plac2(norm_rad(y), w.ekls);
  plac1(w, 5.0 * kPi / 6.0, 3.0, x, y);
  w.f[3] = plac2(norm_rad(y), w.ekls);
  w.f[5] = norm_rad(w.f[11] + kPi);
  w.f[6] = norm_rad(w.f[12] + kPi);
  w.f[8] = norm_rad(w.f[2] + kPi);
  w.f[9] = norm_rad(w.f[3] + kPi);
  w.f[13] = w.f[1];
  regio0(w.f);
}

// the original koch1
double koch1(const Work& w, double h) {
  const double a = std::asin(std::sin(w.armcb) * std::tan(w.gg * kDegToRad) * std::tan(w.ekls));
  const double b = 2.0 * h / kPi - 1.0;
  const double c = norm_rad(w.armcb + h + a * b);
  const double z = std::sin(c);
  const double n = std::cos(c) * std::cos(w.ekls) - std::tan(w.gg * kDegToRad) * std::sin(w.ekls);
  return atn(z, n);
}

void koch(Work& w) {
  // the rounded angle literals are the original's own
  w.f[11] = koch1(w, 0.5235988);
  w.f[5] = norm_rad(w.f[11] + kPi);
  w.f[12] = koch1(w, 1.0471976);
  w.f[6] = norm_rad(w.f[12] + kPi);
  w.f[2] = koch1(w, 2.0943951);
  w.f[8] = norm_rad(w.f[2] + kPi);
  w.f[3] = koch1(w, 2.6179939);
  w.f[9] = norm_rad(w.f[3] + kPi);
  regio0(w.f);
}

// the original regio1
double regio1(const Work& w, double h) {
  const double hh = norm_rad(w.armcb + h);
  const double r = atn(std::sin(h) * std::sin(kDegToRad * w.gg + kEps), std::cos(kDegToRad * w.gg + kEps) * std::cos(hh));
  const double k = norm_rad(r + w.ekls);
  return atn(std::cos(r) * std::sin(hh), std::cos(hh) * std::cos(k));
}

void regiomontanus(Work& w) {
  w.f[11] = regio1(w, kPi / 6.0);
  w.f[5] = norm_rad(w.f[11] + kPi);
  w.f[12] = regio1(w, kPi / 3.0);
  w.f[6] = norm_rad(w.f[12] + kPi);
  w.f[2] = regio1(w, 2.0 * kPi / 3.0);
  w.f[8] = norm_rad(w.f[2] + kPi);
  w.f[3] = regio1(w, 5.0 * kPi / 6.0);
  w.f[9] = norm_rad(w.f[3] + kPi);
  regio0(w.f);
}

// the original camp1
double camp1(const Work& w, double h) {
  const double d = atn(std::sin(h) * std::cos(w.gg * kDegToRad), std::cos(h));
  const double y = std::sin(w.armcb + d);
  const double x = std::cos(w.armcb + d) * std::cos(w.ekls) - std::sin(d) * std::tan(w.gg * kDegToRad) * std::sin(w.ekls);
  return atn(y, x);
}

void campanus(Work& w) {
  w.f[11] = camp1(w, 0.5235988);
  w.f[5] = norm_rad(w.f[11] + kPi);
  w.f[12] = camp1(w, 1.0471976);
  w.f[6] = norm_rad(w.f[12] + kPi);
  w.f[2] = camp1(w, 2.0943951);
  w.f[8] = norm_rad(w.f[2] + kPi);
  w.f[3] = camp1(w, 2.6179939);
  w.f[9] = norm_rad(w.f[3] + kPi);
  regio0(w.f);
}

// the original topo1
double topo1(const Work& w, double k1, double k2) {
  const double g = atn(std::tan(w.gg * kDegToRad), 3.0 / k1);
  const double a = w.armcb - kPi / k2;
  return atn(std::cos(a), -(std::sin(w.ekls) * std::tan(g) + std::cos(w.ekls) * std::sin(a)));
}

void topocentric(Work& w) {
  w.f[11] = norm_rad(topo1(w, 1.0, 3.0));
  w.f[12] = norm_rad(topo1(w, 2.0, 6.0));
  w.f[2] = norm_rad(topo1(w, 2.0, -6.0));
  w.f[3] = norm_rad(topo1(w, 1.0, -3.0));
  w.f[5] = norm_rad(w.f[11] + kPi);
  w.f[6] = norm_rad(w.f[12] + kPi);
  w.f[8] = norm_rad(w.f[2] + kPi);
  w.f[9] = norm_rad(w.f[3] + kPi);
  w.f[13] = w.f[1];
  regio0(w.f);
}

//RR ÄQUAL EKLIPTIKAL ab AC
void equal_asc(Work& w, double ac) {
  const double p = kTwoPi / 12.0;
  for (int i = 0; i <= 11; ++i) {
    w.f[static_cast<std::size_t>(i + 1)] = norm_rad(ac + i * p);
  }
  w.f[1] = ac;
}

void equal_vehlow(Work& w, double ac) {
  const double p = kTwoPi / 12.0;
  for (int i = 0; i <= 11; ++i) {
    w.f[static_cast<std::size_t>(i + 1)] = norm_rad(ac + i * p - kPi / 12.0);
  }
}

}  // namespace

Angles chart_angles(double armcb, double lat_deg, double ekls) {
  Angles out;
  const double gg = lat_deg * kDegToRad;
  double z = std::cos(armcb);
  double n = -(std::sin(ekls) * std::tan(gg) + std::cos(ekls) * std::sin(armcb));
  out.ac = atn(z, n);
  out.dc = norm_rad(out.ac + kPi);
  z = std::sin(armcb);
  n = std::cos(armcb) * std::cos(ekls);
  out.mc = atn(z, n);
  out.ic = norm_rad(out.mc + kPi);
  z = std::cos(kPi + armcb);
  n = -(std::sin(ekls) * std::tan(kHalfPi - gg) + std::cos(ekls) * std::sin(kPi + armcb));
  out.vertex = atn(z, n);
  return out;
}

Houses compute_houses(HouseSystem system, double armcb, double lat_deg, double ekls) {
  Houses out;
  out.angles = chart_angles(armcb, lat_deg, ekls);
  out.cusp[1] = out.angles.ac;
  out.cusp[7] = out.angles.dc;
  out.cusp[10] = out.angles.mc;
  out.cusp[4] = out.angles.ic;

  // the original maxbreit guard for the semi arc systems
  if (system == HouseSystem::kPlacidus || system == HouseSystem::kKochGoh) {
    if (std::abs(lat_deg) > 90.0 - ekls * kRadToDeg) {
      //RR Geog. Breite zu groß !
      out.ok = false;
      return out;
    }
  }

  Work w{out.cusp, armcb, lat_deg, ekls};
  switch (system) {
    case HouseSystem::kPlacidus:
      placidus(w);
      out.name = "Placidus";
      break;
    case HouseSystem::kTopocentric:
      topocentric(w);
      out.name = "Topozentr.";
      break;
    case HouseSystem::kKochGoh:
      koch(w);
      out.name = "Koch-GOH";
      break;
    case HouseSystem::kRegiomontanus:
      regiomontanus(w);
      out.name = "Regiomont.";
      break;
    case HouseSystem::kCampanus:
      campanus(w);
      out.name = "Campanus";
      break;
    case HouseSystem::kEqualAsc:
      equal_asc(w, out.angles.ac);
      out.name = "Äqual-Ekl.";
      break;
    case HouseSystem::kEqualVehlow:
      equal_vehlow(w, out.angles.ac);
      out.name = "Äqual-Vehl";
      break;
    case HouseSystem::kAcMcOnly:
    case HouseSystem::kNone:
      // the original a60_l zeroes the intermediate cusps, kNone drops the
      // angles as well
      for (int i : {2, 3, 5, 6, 8, 9, 11, 12}) {
        out.cusp[static_cast<std::size_t>(i)] = 0.0;
      }
      if (system == HouseSystem::kNone) {
        for (int i : {1, 4, 7, 10}) {
          out.cusp[static_cast<std::size_t>(i)] = 0.0;
        }
      }
      out.name = "Keine";
      break;
  }
  out.ok = true;
  return out;
}

}  // namespace horcom
