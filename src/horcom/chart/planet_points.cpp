// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/planet_points.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/ephem/elements.hpp"
#include "horcom/time/calendar.hpp"

namespace horcom {

namespace {

// the mean Pluto elements of plelempl without the perturbation sums,
// with the precession of node and perihelion the caller applied
Orbit pluto_mean_elements(const TimeArguments& ta) {
  const double t11 = ta.t11;
  const double t21 = ta.t21;
  Orbit o;
  o.a = 39.544674;
  const double kp = -0.178744;
  const double qp = -0.051699;
  const double hp = -0.173426;
  const double pp = 0.139782;
  //RR PRO JAHRH.
  const double nup = 2.53333;
  o.mel = norm_rad(nup * t11 + 4.166868);
  o.o = atn(pp, qp);
  double arg = norm_rad(atn(hp, kp) - o.o);
  o.e = std::sqrt(hp * hp + kp * kp);
  o.i = 2.0 * std::asin(std::sqrt(pp * pp + qp * qp));
  //RR PRÄZESSION KNOTEN u. PERIHEL
  const double c = 3.036223248 * t11 - 0.00421448533 * t21 - o.o;
  const double a = std::sin(c);
  const double pa = 0.02437095349 * t11 + 5.381431861e-06 * t21;
  const double ppp = 0.0002282018 * t11 - 1.45444e-07 * t21;
  o.i = o.i - ppp * std::cos(c);
  o.o = o.o + pa - ppp * a * 3.242061199;
  arg = arg + ppp * a / std::sin(o.i);
  o.p = norm_rad(arg + o.o);
  return o;
}

// ported from plko1001 and plko12, one mean orbit point seen from the
// Earth, the point's radius follows the ellipse at its true anomaly
double project(const Chart& chart, const Orbit& orb, double v, double l) {
  const double r = orb.a * (1.0 - orb.e * orb.e) / (1.0 + orb.e * std::cos(v));
  const BodyState& earth = chart.b[body::kSun];  // the sun slot carries the Earth
  const double x = r * std::cos(l) - earth.r * std::cos(earth.hel);
  const double y = r * std::sin(l) - earth.r * std::sin(earth.hel);
  return norm_rad(atn(y, x + kEps) + chart.smo.dpsi);
}

}  // namespace

// ported from HORCOM plko10 and plko100
PlanetPoints planet_points(const Chart& chart, int slot, const ChartSettings& s) {
  PlanetPoints out;
  if (slot < 1 || slot > 10 || !chart.ok) {
    return out;
  }
  if (slot == body::kSun) {
    if (s.heliocentric) {
      return out;
    }
    // his gom(1), the node line of the solar equator
    const CalendarDate d = calendar_date(chart.jd_ut, s.calendar);
    const double a = d.year + (d.month - 1 + d.day / 30.0) / 12.0;
    out.node = norm_rad(kDegToRad * (73.6667 + (a - 1850.0) * 0.01396));
    out.node_south = norm_rad(out.node + kPi);
    // the perigee of the apparent orbit, his pdg(1) = p(1)
    out.perihelion = norm_rad(chart.smo.sun_p);
    out.aphelion = norm_rad(out.perihelion + kPi);
    out.ok = true;
    return out;
  }
  if (slot == body::kMoon) {
    if (s.heliocentric) {
      //RR undefiniert
      // his IF i& = 2, the Earth has no node, its apsides stand opposite
      // the Sun's perigee, w = nb(PI + p(1)) above
      out.node = -1.0;
      out.node_south = -1.0;
      out.perihelion = norm_rad(chart.smo.sun_p + kPi);
      out.aphelion = norm_rad(chart.smo.sun_p);
      out.ok = true;
      return out;
    }
    out.node = norm_rad(chart.lunar.mean_node);
    out.node_south = norm_rad(out.node + kPi);
    // his pdga(2), the apogee after the AG setting, true with apogw!, pdg
    // opposite
    out.aphelion = norm_rad(s.true_apogee ? chart.lunar.true_apogee : chart.lunar.mean_apogee);
    out.perihelion = norm_rad(out.aphelion + kPi);
    out.ok = true;
    return out;
  }
  const Orbit orb = slot == body::kPluto ? pluto_mean_elements(chart.ta) : mean_elements(slot, chart.ta);
  const double arg = orb.p - orb.o;
  if (s.heliocentric) {
    // directions instead of space points, his o and p columns
    out.node = norm_rad(orb.o);
    out.node_south = norm_rad(orb.o + kPi);
    out.perihelion = norm_rad(orb.p);
    out.aphelion = norm_rad(orb.p + kPi);
    out.ok = true;
    return out;
  }
  out.node = project(chart, orb, -arg, orb.o);
  out.node_south = project(chart, orb, kPi - arg, orb.o + kPi);
  out.perihelion = project(chart, orb, 0.0, orb.o + arg);
  out.aphelion = project(chart, orb, kPi, orb.o + arg + kPi);
  out.ok = true;
  return out;
}

}  // namespace horcom
