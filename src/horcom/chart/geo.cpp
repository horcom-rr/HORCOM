// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/geo.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

//RR NR.,LÄNGE,BREITE,LÄNGE SO,BREITE SO
// the original plko12. The Earth terms carry no cos(b1) factor and the
// third component uses r1 * sin(b1), both kept faithfully, the solar
// latitude is arcseconds small.
double plko12(double l, double b, double l1, double b1, double r, double r1, double& dr) {
  const double x1 = r * std::cos(b) * std::cos(l) - r1 * std::cos(l1);
  const double x2 = r * std::cos(b) * std::sin(l) - r1 * std::sin(l1);
  const double x3 = r * std::sin(b) - r1 * std::sin(b1);
  dr = std::sqrt(x1 * x1 + x2 * x2 + x3 * x3);
  return atn(x2, x1 + kEps);
}

}  // namespace

GeoResult helio_to_geo(const HelioState& body, const HelioState& earth, double dpsi, double deps) {
  GeoResult out;
  double dr = 0.0;
  const double el = norm_rad(plko12(body.l, body.b, earth.l, earth.b, body.r, earth.r, dr));
  out.dr = dr;
  out.el = norm_rad(el + dpsi);
  out.eb = deps + std::asin((body.r / (kEps + dr)) * std::sin(body.b));

  // daily motion from a symmetric step of a tenth of the rates
  double drf = 0.0;
  double w1 = atn(0.0, 1.0);
  {
    const double l = body.l + body.lt / 10.0;
    const double l1 = earth.l + earth.lt / 10.0;
    const double b = body.b + body.bt / 10.0;
    const double b1 = earth.b + earth.bt / 10.0;
    const double r = body.r + body.rt / 10.0;
    const double r1 = earth.r + earth.rt / 10.0;
    w1 = plko12(l, b, l1, b1, r, r1, drf);
  }
  double w2 = el;
  verv(w1, w2);
  const double tb1 = 10.0 * (w1 - w2);
  double w3 = el;
  double w4 = atn(0.0, 1.0);
  {
    const double l = body.l - body.lt / 10.0;
    const double l1 = earth.l - earth.lt / 10.0;
    const double b = body.b - body.bt / 10.0;
    const double b1 = earth.b - earth.bt / 10.0;
    const double r = body.r - body.rt / 10.0;
    const double r1 = earth.r - earth.rt / 10.0;
    w4 = plko12(l, b, l1, b1, r, r1, drf);
  }
  verv(w3, w4);
  const double tb2 = 10.0 * (w3 - w4);
  out.tb = (tb1 + tb2) / 2.0;
  out.ttb = tb1 - tb2;
  return out;
}

}  // namespace horcom
