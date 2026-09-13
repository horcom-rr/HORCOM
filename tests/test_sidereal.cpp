// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/time/sidereal.hpp"

using namespace horcom;

// The original uses the Newcomb sidereal time series. For 1987-04-10 it
// differs from the modern IAU value in Meeus by 0.066 s, so each test pins
// the exact transcription value and adds a half second sanity band against
// the Meeus example.

TEST_CASE("GMST at 0h UT for the Meeus example 1987-04-10") {
  const double h0 = gmst0_hours(2446895.5);
  CHECK(h0 == doctest::Approx(13.179528156063).epsilon(1e-12));
  const double meeus = 13.0 + 10.0 / 60.0 + 46.3668 / 3600.0;
  CHECK(std::abs(h0 - meeus) * 3600.0 < 0.5);
  // the convenience overload rewinds to midnight by itself
  CHECK(gmst0_hours(CalendarDate{10, 4, 1987, 19, 21.0}) == doctest::Approx(h0));
}

TEST_CASE("mean sidereal time at a moment, Meeus example 1987-04-10 19:21 UT") {
  const double h0 = gmst0_hours(2446895.5);
  const double hs = sidereal_at_hours(h0, 19.0 + 21.0 / 60.0);
  CHECK(hs == doctest::Approx(8.582506675863).epsilon(1e-12));
  const double meeus = 8.0 + 34.0 / 60.0 + 57.0896 / 3600.0;
  CHECK(std::abs(hs - meeus) * 3600.0 < 0.5);
}

TEST_CASE("apparent correction reproduces the equation of the equinoxes") {
  // Meeus example, dpsi = -3.788 arcsec and true obliquity 23.44357 deg
  // give -0.2317 s of time
  const double dpsi = -3.788 * kArcsecToRad;
  const double ekls = 23.44357 * kDegToRad;
  const double h0 = 13.0;
  const double corrected = apparent_sidereal_hours(h0, dpsi, ekls);
  CHECK((corrected - h0) * 3600.0 == doctest::Approx(-0.2317).epsilon(2e-3));
}
