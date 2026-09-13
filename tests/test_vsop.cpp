// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/ephem/vsop.hpp"
#include "horcom/time/calendar.hpp"

using namespace horcom;

namespace {

VsopTables& tables() {
  static VsopTables t = VsopTables::load(HORCOM_TEST_DATA_DIR "/planets.ndx", HORCOM_TEST_DATA_DIR "/planets.dat");
  return t;
}

}  // namespace

TEST_CASE("the term tables load completely") {
  // planets 1..8 in the file hold 2430 terms, the duplicate block zero of
  // the Earth stays unread like in the original
  CHECK(tables().term_count() == 2430);
}

TEST_CASE("Venus against the Meeus example for 1992-12-20 0h TD") {
  // Meeus computes with the same truncated tables, L 26.11428 degrees,
  // B -2.62070 degrees, R 0.724603 AU
  const TimeArguments t = time_arguments(2448976.5);
  const VsopTables::Result v = tables().evaluate(2, t.t11);
  CHECK(v.l * kRadToDeg == doctest::Approx(26.11428).epsilon(0).scale(0).epsilon(2e-6));
  CHECK(v.b * kRadToDeg == doctest::Approx(-2.62070).epsilon(2e-5));
  CHECK(v.r == doctest::Approx(0.724603).epsilon(2e-6));
}

TEST_CASE("Earth against the Meeus example for 1992-10-13 0h TD") {
  // the Sun example of Meeus run backwards, Earth heliocentric longitude
  // is the Sun's geometric longitude minus 180 degrees
  const TimeArguments t = time_arguments(2448908.5);
  const VsopTables::Result e = tables().evaluate(3, t.t11);
  CHECK(e.l * kRadToDeg == doctest::Approx(19.907372).epsilon(3e-6));
  CHECK(e.r == doctest::Approx(0.99760775).epsilon(1e-6));
  CHECK(std::abs(e.b * kRadToDeg) < 0.0002);
}

TEST_CASE("rates agree with a numerical difference") {
  const TimeArguments t = time_arguments(2451545.0);
  const VsopTables::Result v = tables().evaluate(4, t.t11);
  const double dt_days = 0.5;
  const TimeArguments ta = time_arguments(2451545.0 - dt_days);
  const TimeArguments tb = time_arguments(2451545.0 + dt_days);
  const VsopTables::Result va = tables().evaluate(4, ta.t11);
  const VsopTables::Result vb = tables().evaluate(4, tb.t11);
  CHECK(v.lt == doctest::Approx((vb.l - va.l) / (2.0 * dt_days)).epsilon(1e-4));
  CHECK(v.rt == doctest::Approx((vb.r - va.r) / (2.0 * dt_days)).epsilon(1e-3));
}
