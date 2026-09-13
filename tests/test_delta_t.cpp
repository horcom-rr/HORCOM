// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "doctest.h"
#include "horcom/time/delta_t.hpp"

using namespace horcom;

namespace {

// jd for the original's year scale j = 1900 + (jd - 2415020) / 365.25
double jd_for_j(double j) {
  return 2415020.0 + (j - 1900.0) * 365.25;
}

}  // namespace

TEST_CASE("table nodes reproduce exactly") {
  // j = 2000 sits on the 65 s node, t1 is exactly 1 there
  CHECK(delta_t_minutes(2451545.0) == doctest::Approx(65.0 / 60.0));
  // the 1620 edge runs through the zero element of the GFA array and
  // still yields the first table value
  CHECK(delta_t_minutes(jd_for_j(1620.0)) == doctest::Approx(124.0 / 60.0));
  CHECK(delta_t_minutes(jd_for_j(1997.0)) == doctest::Approx(63.0 / 60.0));
}

TEST_CASE("interpolation between nodes") {
  // 1990 lies between the 1960 and 1991 nodes
  const double expected_seconds = 33.15 + 30.0 * (57.2 - 33.15) / 31.0;
  CHECK(delta_t_minutes(jd_for_j(1990.0)) == doctest::Approx(expected_seconds / 60.0).epsilon(1e-9));
}

TEST_CASE("polynomial branches") {
  //RR 2008 a.d. ........
  const double jd2010 = jd_for_j(2010.0);
  const double t1 = (jd2010 - 2415020.0) / 36525.0;
  CHECK(delta_t_minutes(jd2010) == doctest::Approx(1.2053 * t1 + 0.4992 * t1 * t1 - 0.7506263));
  //RR 948 a.d.  ..... 1600 a.d.
  const double jd1200 = jd_for_j(1200.0);
  const double t2 = (jd1200 - 2415020.0) / 36525.0;
  CHECK(delta_t_minutes(jd1200) == doctest::Approx(0.425 + 0.85 * t2 + 0.425 * t2 * t2));
  //RR 390 v.chr.  .... 948 a.d.
  const double jd900 = jd_for_j(900.0);
  CHECK(delta_t_minutes(jd900) == doctest::Approx(28.74 + 6.81 * -10.0 + 0.7383 * 100.0));
}

TEST_CASE("UT to ET round trip via the a90 pattern") {
  const double jd_ut = 2451545.0;
  const double delt = delta_t_minutes(jd_ut);
  const double jd_et = ut_to_et(jd_ut);
  CHECK(jd_et == doctest::Approx(jd_ut + delt * kDeltaTDaysPerMinute));
  CHECK(et_to_ut(jd_et, delt) == doctest::Approx(jd_ut));
}
