// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>
#include <initializer_list>

#include "doctest.h"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/ephem/elements.hpp"
#include "horcom/ephem/kepler.hpp"

using namespace horcom;

TEST_CASE("the solver satisfies the Kepler equation") {
  for (double e : {0.0167, 0.2056, 0.3, 0.8662}) {
    for (double man : {0.3, 1.9, 3.6, 5.9}) {
      CAPTURE(e);
      CAPTURE(man);
      Orbit o;
      o.a = 10.0;
      o.e = e;
      o.man = man;
      const OrbitPosition p = kepler(o);
      CHECK(norm_rad(p.ean - e * std::sin(p.ean)) == doctest::Approx(man).epsilon(1e-6));
      CHECK(p.r == doctest::Approx(o.a * (1.0 - e * std::cos(p.ean))));
    }
  }
}

TEST_CASE("a flat orbit reduces to plane geometry") {
  Orbit o;
  o.a = 1.0;
  o.e = 0.1;
  o.i = 0.0;
  o.o = 0.5;
  o.p = 0.7;
  o.man = 2.0;
  const OrbitPosition p = kepler(o);
  CHECK(p.heb == doctest::Approx(0.0));
  CHECK(p.hel == doctest::Approx(norm_rad(o.o + o.p + p.v)).epsilon(1e-9));
}

TEST_CASE("latitude follows the argument of latitude") {
  Orbit o;
  o.a = 5.2;
  o.e = 0.0485;
  o.i = 0.3;
  o.o = 1.7;
  o.p = 0.2;
  o.man = 1.1;
  const OrbitPosition p = kepler(o);
  CHECK(p.heb == doctest::Approx(std::asin(std::sin(p.u) * std::sin(o.i))));
}

TEST_CASE("mean elements of Jupiter around J2000 look like Jupiter") {
  const TimeArguments t = time_arguments(2451545.0);
  const Orbit o = mean_elements(6, t);
  CHECK(o.a == doctest::Approx(5.2026).epsilon(1e-4));
  CHECK(o.e == doctest::Approx(0.04849485).epsilon(1e-6));
  CHECK(o.i * kRadToDeg == doctest::Approx(1.30327).epsilon(1e-5));
  // the solved radius must lie between perihelion and aphelion
  Orbit run = o;
  run.p = norm_rad(o.p - o.o);
  const OrbitPosition p = kepler(run);
  CHECK(p.r > o.a * (1.0 - o.e));
  CHECK(p.r < o.a * (1.0 + o.e));
}

TEST_CASE("a circular Hamburg orbit keeps its radius") {
  const TimeArguments t = time_arguments(2451545.0);
  const Orbit o = uranian_elements(9, t);
  const OrbitPosition p = kepler(o);
  CHECK(p.r == doctest::Approx(o.a));
  CHECK(p.hel == doctest::Approx(norm_rad(o.man)).epsilon(1e-9));
}
