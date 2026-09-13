// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/ephem/kepler.hpp"

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {
// the original's break out threshold
constexpr double kIterationTolerance = 1.0e-07;
}  // namespace

OrbitPosition kepler(const Orbit& orbit) {
  const double m = orbit.man;
  const double e = orbit.e;
  //RR START ODELL..
  double ea = norm_rad(m + e * std::sin(m) / std::sqrt(1.0 - 2.0 * e * std::cos(m) + e * e));
  double ea0 = 0.0;
  do {
    ea0 = ea;
    //RR MEEUS
    ea = norm_rad(ea + (m + e * std::sin(ea) - ea) / (1.0 - e * std::cos(ea)));
  } while (std::abs(ea - ea0) > kIterationTolerance);

  OrbitPosition out;
  //RR WAHRE ANOMALIE
  out.v = norm_rad(2.0 * std::atan(std::sqrt((1.0 + e) / (1.0 - e)) * std::tan(0.5 * ea)));
  out.r = orbit.a * (1.0 - e * std::cos(ea));
  //RR EXZENTRISCHE ANOMALIE
  out.ean = ea;
  out.u = norm_rad(orbit.p + out.v);
  const double z = std::cos(orbit.i) * std::sin(out.u);
  const double n = std::cos(orbit.p + out.v);
  out.hel = norm_rad(atn(z, n + kEps) + orbit.o);
  if (out.u + orbit.o - out.hel > 0.17) {
    out.u = norm_rad(out.u - kPi);
  }
  if (out.u + orbit.o - out.hel > 0.17) {
    out.u = norm_rad(out.u - kPi);
  }
  out.heb = std::asin(std::sin(out.u) * std::sin(orbit.i));
  return out;
}

}  // namespace horcom
