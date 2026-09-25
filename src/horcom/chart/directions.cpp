// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
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
  // his dst = 0.0027379092
  //RR 1/tja
  const double dst = 1.0 / tja;
  const double djd = dst * (jd_event_ut - radix.jd_ut) / tja;
  double brm = out.arm_deg + (converse ? -djd : djd) * kDegPerCircle;
  // his ADD brm,dif * 360 / tja turned the axes by 0.9856 degrees for
  // every degree of sidereal time asked, while the take over into the
  // radix moved the birth by the sidereal rate. The variation is a turn
  // of the sidereal time itself, degree for degree, so the preview shows
  // the axes the taken over radix will carry
  brm += vary_deg;
  out.arc_deg = brm - out.arm_deg;
  out.armc_deg = norm_deg(brm);
  out.houses = compute_houses(sys, out.armc_deg * kDegToRad, lat_deg, radix.smo.ekls);
  return out;
}

}  // namespace horcom
