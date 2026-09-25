// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/great_year.hpp"

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"
#include "horcom/ephem/precession.hpp"

namespace horcom {

// ported from HORCOM grossj1
GreatYearPoint great_year_point(double jd, double tja, double ekls, double jd_ref, int zal_deg) {
  const double la1 = kDegToRad * zal_deg;
  const Equatorial eq0 = ecliptic_to_equatorial(la1, 0.0, ekls);
  double ar = 0.0;
  double de = 0.0;
  precess_newcomb(jd, tja, jd_ref, eq0.ra, eq0.dec, ar, de);
  const Ecliptic ec = equatorial_to_ecliptic(ar, de, ekls);
  // his di = (la1 - la) * up compared an unfolded 360 degree start with a
  // normalised longitude, the FISCHE age reported 359.7 instead of -0.3
  // and warned on every date. The fold keeps his tests for the other ages
  const double di = fold_rad(la1 - ec.lon) * kRadToDeg;
  GreatYearPoint p;
  p.di_deg = di;
  p.point_deg = norm_deg(zal_deg + di);
  // his IF di < -30 OR la1 > la, NICHT MEHR im ZEITALTER
  p.outside = di < -kDegPerSign || di > 0.0;
  return p;
}

}  // namespace horcom
