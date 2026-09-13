// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/corrections.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

//RR apparente Pos.
void apparent_position(BodyPosition& p, ApparentMode mode, double sun_el, double sun_eb) {
  const double el = p.el;
  const double eb = p.eb;
  // light time in days per AU times the daily motion
  const double e = 0.0057755 * p.tb * p.dr;
  double de = 0.0;
  double db = 0.0;
  switch (mode) {
    case ApparentMode::kLightTime:
      de = -e;
      db = 0.0;
      break;
    case ApparentMode::kLightTimeAberration:
      de = -e - 0.00009936509 * std::cos(el - sun_el) / std::cos(eb);
      db = sun_eb * std::sin(el - sun_el) * std::sin(eb);
      break;
    case ApparentMode::kTrue:
      de = 0.0;
      db = 0.0;
      break;
  }
  p.el = norm_rad(el + de);
  p.eb = eb + db;
}

void to_equatorial(BodyPosition& p, double ekls) {
  const Equatorial eq = ecliptic_to_equatorial(p.el, p.eb, ekls);
  p.ar = eq.ra;
  p.de = eq.dec;
}

//RR Parallaxe
//RR aus Montenbruck S.25  Ohne Ber. d.Meereshöhe
void parallax(BodyPosition& p, double lat_deg, double armc_deg, double ekls) {
  const double r = p.dr;
  const double ar = p.ar;
  const double de = p.de;
  const double g = kDegToRad * lat_deg;
  const double s = kDegToRad * armc_deg;
  //RR Erdradius in AE
  const double ro = 0.000042634515;
  double z = r * std::cos(de) * std::sin(ar) - ro * std::cos(g) * std::sin(s);
  double n = r * std::cos(de) * std::cos(ar) - ro * std::cos(g) * std::cos(s);
  const double arn = atn(z, n);
  z = std::sin(arn) * (r * std::sin(de) - ro * std::sin(g));
  n = r * std::cos(de) * std::sin(ar) - ro * std::cos(g) * std::sin(s);
  // the original keeps the raw arctangent here, no quadrant correction
  const double den = std::atan(z / n);
  p.ar = arn;
  p.de = den;
  const Ecliptic ec = equatorial_to_ecliptic(arn, den, ekls);
  p.el = ec.lon;
  p.eb = ec.lat;
}

}  // namespace horcom
