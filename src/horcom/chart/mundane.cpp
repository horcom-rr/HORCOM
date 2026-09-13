// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/mundane.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"

namespace horcom {

namespace {

// ported from md11, east of the meridian axis or west
bool md_east(double ar, double arm, double aric) {
  double w1 = arm;
  double w3 = ar;
  double w2 = aric - kEps;
  vergl2(w1, w2, w3);
  return w3 > w1 && w3 < w2;
}

// ported from md, the semi arc proportion per quadrant
void md(bool east, double g, double arm, double aric, double ar, double de, double& aoe, double& doe) {
  aoe = 0.0;
  doe = 0.0;
  const double ad = std::asin(std::tan(g) * std::tan(de));
  const double sad = kPi / 2.0 + ad;
  const double san = kPi / 2.0 - ad;
  if (east) {
    //RR linke Hälfte
    double w1 = arm;
    double w2 = ar;
    vergl1(w1, w2);
    if (w2 - w1 > sad && w2 < w1 + kPi) {
      //RR 1. Quadrant
      const double dist = w1 + kPi - w2;
      aoe = ar - dist * ad / (san + kEps);
    }
    if (w2 - w1 < sad && w2 > w1) {
      //RR 4. Quadrant
      const double dist = w2 - w1;
      aoe = ar - dist * ad / (sad + kEps);
    }
  } else {
    //RR rechte Hälfte
    double w1 = aric;
    double w2 = ar;
    vergl1(w1, w2);
    if (w2 - w1 < san && w2 > w1) {
      //RR 2. Quadrant
      doe = ar + (w2 - w1) * ad / san;
    }
    if (w2 - w1 > san && w2 < w1 + kPi) {
      //RR 3. Quadrant
      doe = ar + (w1 + kPi - w2) * ad / sad;
    }
  }
}

}  // namespace

// ported from mundan with mundh1
double mundane_longitude(double la_rad, double br_rad, double ekls, double armcb, double lat_deg) {
  const double g = lat_deg * kDegToRad;
  const double arm = armcb;
  const double aric = norm_rad(arm + kPi);
  const Equatorial eq = ecliptic_to_equatorial(la_rad, br_rad, ekls);
  double aoe = 0.0;
  double doe = 0.0;
  const bool east = md_east(eq.ra, arm, aric);
  md(east, g, arm, aric, eq.ra, eq.dec, aoe, doe);
  if (east) {
    double w2 = aoe + 3.0 * kPi / 2.0;
    double w1 = arm;
    vergl1(w1, w2);
    return norm_rad(w2 - w1);
  }
  double w2 = doe + 3.0 * kPi / 2.0;
  double w1 = aric;
  vergl1(w1, w2);
  return norm_rad(kPi + w2 - w1);
}

// ported from mundhorp and mundhorh
void to_mundane(Chart& c, double lat_deg) {
  const double armcb = c.armc_deg * kDegToRad;
  for (int t = 0; t <= 40; ++t) {
    // mundhorp leaves the axis and cusp slots alone
    if (t >= 13 && t <= 18) {
      continue;
    }
    BodyState& b = c.b[static_cast<std::size_t>(t)];
    if (!b.present || !b.valid) {
      continue;
    }
    //RR Mundanwert
    b.el = mundane_longitude(norm_rad(b.el), b.eb + kEps, c.smo.ekls, armcb, lat_deg);
  }
  for (int t = 1; t <= 12; ++t) {
    c.houses.cusp[static_cast<std::size_t>(t)] = norm_rad(kEps + (t - 1) * kPi / 6.0);
  }
  c.houses.cusp[13] = c.houses.cusp[1];
  c.houses.angles.ac = c.houses.cusp[1];
  c.houses.angles.dc = c.houses.cusp[7];
  c.houses.angles.mc = c.houses.cusp[10];
  c.houses.angles.ic = c.houses.cusp[4];
  if (c.b[body::kAscendant].present) {
    c.b[body::kAscendant].el = c.houses.cusp[1];
  }
  if (c.b[body::kMc].present) {
    c.b[body::kMc].el = c.houses.cusp[10];
  }
}

}  // namespace horcom
