// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/composite.hpp"

#include <algorithm>
#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

// ported from halbsmin
double midpoint_near(double p1, double p2) {
  p1 = norm_rad(p1);
  p2 = norm_rad(p2);
  const double p31 = (p1 + p2) / 2.0;
  const double p32 = p31 + kPi;
  const double a1 = std::min(norm_rad(p1 - p31), norm_rad(p2 - p31));
  const double a2 = std::min(norm_rad(p1 - p32), norm_rad(p2 - p32));
  // his last step added pi whenever the raw sum passed a full circle
  // and the result lay below pi, so a pair around zero Aries like 100
  // and 300 degrees got 200 instead of the near 20. The port keeps the
  // near side his own a1 and a2 test chose
  return norm_rad(a1 < a2 ? p31 : p32);
}

namespace {

// the axis flip of a13, the midpoint moves to the side of its anchor
double flip_to(double value, double anchor) {
  double w1 = value;
  double w2 = anchor;
  vergl1(w1, w2);
  if (std::abs(w1 - w2) > kHalfPi) {
    return norm_rad(value + kPi);
  }
  return value;
}

}  // namespace

// ported from a13 with a13comp_1 and a13comp_2
Chart composite_chart(const Chart& a, const ChartInput& ia, const Chart& b, const ChartInput& ib,
                      CompositeHouses mode, double residence_lat_deg, const ChartSettings& s) {
  Chart c{};
  c.ok = true;
  c.jd_ut = (a.jd_ut + b.jd_ut) / 2.0;
  c.hs = (a.hs + b.hs) / 2.0;
  const double ekls = (a.smo.ekls + b.smo.ekls) / 2.0;

  // the bodies, near side midpoints, the south node rides the north. The
  // loop runs from aa& like a13, a fixed point set in both charts joins
  double node_mid = 0.0;
  for (int t = body::kFixpunkt; t < body::kSlotCount; ++t) {
    if (t == body::kNodeDesc || t == body::kAscendant || t == body::kMc) {
      continue;
    }
    const BodyState& ba = a.b[static_cast<std::size_t>(t)];
    const BodyState& bb = b.b[static_cast<std::size_t>(t)];
    if (!ba.present || !ba.valid || !bb.present || !bb.valid) {
      continue;
    }
    BodyState& out = c.b[static_cast<std::size_t>(t)];
    out.present = true;
    out.valid = true;
    out.el = midpoint_near(ba.el, bb.el);
    if (t == body::kNodeAsc) {
      node_mid = out.el;
    }
  }
  if (c.b[body::kNodeAsc].present) {
    BodyState& ds = c.b[body::kNodeDesc];
    ds.present = true;
    ds.valid = true;
    ds.el = norm_rad(node_mid + kPi);
  }

  // the houses per mode, the equal systems take the schematic halves
  // IF haw& = 6 OR haw& = 7 : CLR comp_hand!,comp_mstz!
  if (s.houses == HouseSystem::kEqualAsc || s.houses == HouseSystem::kEqualVehlow) {
    mode = CompositeHouses::kSchematic;
  }
  switch (mode) {
    case CompositeHouses::kMeanSidereal: {
      //RR Eigene Methode
      // his hs = (hsi + hsa) / 2 plus the mean longitude is the linear mean
      // of the two local sidereal times. Two local times more than twelve
      // hours apart gave the far half of the day, the ARMC stood opposite
      // the MC midpoint. The port takes the near mean of the local ARMCs
      // like halbsmin does for degrees
      const double gl = (ia.lon_deg_east + ib.lon_deg_east) / 2.0;
      const double gg = (ia.lat_deg + ib.lat_deg) / 2.0;
      const double armc_a = kDegPerHour * norm_hours(a.hs + ia.lon_deg_east / kDegPerHour);
      const double armc_b = kDegPerHour * norm_hours(b.hs + ib.lon_deg_east / kDegPerHour);
      const double armc = norm_deg(kRadToDeg * midpoint_near(armc_a * kDegToRad, armc_b * kDegToRad));
      c.hs = norm_hours((armc - gl) / kDegPerHour);
      c.armc_deg = armc;
      c.houses = compute_houses(s.houses, armc * kDegToRad, gg, ekls);
      break;
    }
    case CompositeHouses::kRobertHand: {
      //RR Nach R.HAND
      const double p3 = midpoint_near(a.b[body::kMc].el, b.b[body::kMc].el);
      const double z = std::sin(p3) * std::cos(ekls);
      const double n = std::cos(p3);
      double armcb = atn(z, n);
      double w1 = armcb;
      double w2 = p3;
      vergl1(w1, w2);
      if (std::abs(w1 - w2) > kHalfPi) {
        armcb = norm_rad(armcb + kPi);
      }
      c.armc_deg = armcb * kRadToDeg;
      c.houses = compute_houses(s.houses, armcb, residence_lat_deg, ekls);
      break;
    }
    case CompositeHouses::kSchematic: {
      auto pair_cusp = [&](int t) {
        const double p3 = midpoint_near(a.houses.cusp[static_cast<std::size_t>(t)],
                                        b.houses.cusp[static_cast<std::size_t>(t)]);
        c.houses.cusp[static_cast<std::size_t>(t)] = p3;
        if (t <= 6) {
          c.houses.cusp[static_cast<std::size_t>(t + 6)] = norm_rad(p3 + kPi);
        } else {
          c.houses.cusp[static_cast<std::size_t>(t - 6)] = norm_rad(p3 + kPi);
        }
      };
      for (int t = 1; t <= 12; ++t) {
        pair_cusp(t);
      }
      pair_cusp(10);
      pair_cusp(1);
      // the AC follows the MC in zodiacal order
      {
        double w1 = c.houses.cusp[10];
        double w2 = c.houses.cusp[1];
        vergl1(w1, w2);
        if (w2 < w1 && w2 > 0.0) {
          c.houses.cusp[1] = norm_rad(c.houses.cusp[1] + kPi);
          c.houses.cusp[7] = norm_rad(c.houses.cusp[1] + kPi);
        }
      }
      // the chain of a13comp_2, every next cusp stays ahead of its
      // neighbour, the opposite cusp follows
      for (int t : {1, 2, 4, 5, 7, 8, 10, 11}) {
        double w1 = c.houses.cusp[static_cast<std::size_t>(t)];
        double w2 = c.houses.cusp[static_cast<std::size_t>(t + 1)];
        vergl1(w1, w2);
        if (w2 < w1 && w2 > 0.0) {
          const double lifted = norm_rad(c.houses.cusp[static_cast<std::size_t>(t + 1)] + kPi);
          c.houses.cusp[static_cast<std::size_t>(t + 1)] = lifted;
          if (t <= 6) {
            c.houses.cusp[static_cast<std::size_t>(t + 7)] = norm_rad(lifted + kPi);
          } else {
            c.houses.cusp[static_cast<std::size_t>(t - 5)] = norm_rad(lifted + kPi);
          }
        }
      }
      c.houses.name = a.houses.name;
      c.houses.ok = true;
      break;
    }
  }
  c.houses.angles.ac = c.houses.cusp[1];
  c.houses.angles.mc = c.houses.cusp[10];
  c.houses.angles.dc = c.houses.cusp[7];
  c.houses.angles.ic = c.houses.cusp[4];
  c.houses.cusp[13] = c.houses.cusp[1];

  // the AC and MC body slots, midpoints flipped to their cusps
  BodyState& ac = c.b[body::kAscendant];
  ac.present = true;
  ac.valid = true;
  ac.el = flip_to(midpoint_near(a.b[body::kAscendant].el, b.b[body::kAscendant].el), c.houses.cusp[1]);
  BodyState& mc = c.b[body::kMc];
  mc.present = true;
  mc.valid = true;
  mc.el = flip_to(midpoint_near(a.b[body::kMc].el, b.b[body::kMc].el), c.houses.cusp[10]);
  return c;
}

// his glc / z& is the plain mean of the longitudes. Places on both sides
// of the date line then met on the far side of the globe, 170 east and
// 170 west at Greenwich. The circle is cut in the widest arc free of
// places, where the plain mean already stands for every group that
// leaves the date line alone
double mean_longitude(std::vector<double> lons) {
  std::sort(lons.begin(), lons.end());
  const std::size_t n = lons.size();
  double widest = lons.front() + kDegPerCircle - lons.back();
  std::size_t cut = n;
  for (std::size_t i = 0; i + 1 < n; ++i) {
    if (lons[i + 1] - lons[i] > widest) {
      widest = lons[i + 1] - lons[i];
      cut = i + 1;
    }
  }
  double sum = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    sum += lons[i] + (i < cut && cut < n ? kDegPerCircle : 0.0);
  }
  const double mean = sum / static_cast<double>(n);
  return mean > kDegPerCircle / 2.0 ? mean - kDegPerCircle : mean;
}

// ported from a14
ChartInput combin_input(const std::vector<ChartInput>& parts, Calendar cal) {
  ChartInput out;
  if (parts.empty()) {
    return out;
  }
  double jd = 0.0;
  double gg = 0.0;
  std::vector<double> gl;
  for (const ChartInput& p : parts) {
    jd += julian_day(p.date_ut, cal);
    gl.push_back(p.lon_deg_east);
    gg += p.lat_deg;
  }
  const double n = static_cast<double>(parts.size());
  out.date_ut = calendar_date(jd / n, cal);
  out.lon_deg_east = mean_longitude(gl);
  out.lat_deg = gg / n;
  return out;
}

}  // namespace horcom
