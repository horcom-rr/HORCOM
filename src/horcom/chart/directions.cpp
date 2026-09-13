// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/directions.hpp"

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

// ported from prima and primhorg
DirectedAxes direct_axes(const Chart& radix, double lon_deg_east, double lat_deg,
                         double jd_event_ut, bool converse, double vary_deg, HouseSystem sys) {
  DirectedAxes out;
  const double tja = radix.ta.tropical_year_days;
  out.arm_deg = kDegPerHour * norm_hours(radix.hs + lon_deg_east / kDegPerHour);
  //RR dst = 0.0027379092  // 1/tja
  const double dst = 1.0 / tja;
  const double djd = dst * (jd_event_ut - radix.jd_ut) / tja;
  double brm = out.arm_deg + (converse ? -djd : djd) * 360.0;
  // the sidereal time variation turns at the same rate
  brm += vary_deg * 360.0 / tja;
  out.arc_deg = brm - out.arm_deg;
  out.armc_deg = norm_deg(brm);
  out.houses = compute_houses(sys, out.armc_deg * kDegToRad, lat_deg, radix.smo.ekls);
  return out;
}

}  // namespace horcom
