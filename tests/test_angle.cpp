// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>
#include <initializer_list>

#include "doctest.h"
#include "horcom/core/angle.hpp"

using namespace horcom;

TEST_CASE("norm functions land in [0, period)") {
  CHECK(norm_deg(370.0) == doctest::Approx(10.0));
  CHECK(norm_deg(-10.0) == doctest::Approx(350.0));
  CHECK(norm_deg(360.0) == doctest::Approx(0.0));
  CHECK(norm_deg(720.5) == doctest::Approx(0.5));
  CHECK(norm_rad(-0.5) == doctest::Approx(kTwoPi - 0.5));
  CHECK(norm_rad(kTwoPi + 0.25) == doctest::Approx(0.25));
  CHECK(norm_hours(24.5) == doctest::Approx(0.5));
  CHECK(norm_hours(-1.0) == doctest::Approx(23.0));
}

TEST_CASE("deg_in_sign keeps the sign, wz uses FIX not INT") {
  CHECK(deg_in_sign(35.5) == doctest::Approx(5.5));
  CHECK(deg_in_sign(-35.5) == doctest::Approx(-5.5));
  CHECK(deg_in_sign(29.9) == doctest::Approx(29.9));
}

TEST_CASE("atn recovers the full angle from sin and cos") {
  for (double w : {0.1, 0.7, 1.2, 2.0, 2.9, 3.5, 4.0, 4.9, 5.5, 6.1}) {
    CAPTURE(w);
    CHECK(atn(std::sin(w), std::cos(w)) == doctest::Approx(w).epsilon(1e-12));
  }
}

TEST_CASE("atn matches normalised atan2 away from the z = 0 edge") {
  const double zs[] = {-2.0, -0.3, 0.4, 1.7};
  const double ns[] = {-1.5, -0.2, 0.6, 2.2};
  for (double z : zs) {
    for (double n : ns) {
      CAPTURE(z);
      CAPTURE(n);
      CHECK(atn(z, n) == doctest::Approx(norm_rad(std::atan2(z, n))).epsilon(1e-12));
    }
  }
}

TEST_CASE("vergl2 lets an aspect window survive the 0 / 2 pi wrap") {
  // window around the conjunction of a multiple near 2 pi, w2 wrapped to 0.1
  double w1 = 6.2;
  double w2 = 0.1;
  double w3 = 6.25;
  vergl2(w1, w2, w3);
  CHECK(w2 == doctest::Approx(0.1 + kTwoPi));
  CHECK(w3 == doctest::Approx(6.25));
  CHECK((w1 < w3 && w3 < w2));
}

TEST_CASE("vergl1 leaves already comparable angles alone") {
  double w2 = 3.0;
  vergl1(3.5, w2);
  CHECK(w2 == doctest::Approx(3.0));
  double w2b = 0.2;
  vergl1(5.0, w2b);
  CHECK(w2b == doctest::Approx(0.2 + kTwoPi));
}
