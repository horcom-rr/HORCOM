// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "doctest.h"
#include "horcom/chart/histogram.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

using namespace horcom;

namespace {

// a hand built sky, every position chosen so its sign and house are
// obvious, houses of thirty degrees from five degrees Aries
Chart synthetic() {
  Chart c;
  c.ok = true;
  const auto put = [&c](int slot, double deg) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  };
  put(body::kSun, 10.0);    // Aries, fire, cardinal, house 1
  put(body::kMoon, 40.0);   // Taurus, earth, fixed, house 2
  put(body::kMars, 100.0);  // Cancer, water, cardinal
  put(body::kAscendant, 5.0);
  put(body::kMc, 275.0);
  c.houses.ok = true;
  for (int i = 1; i <= 13; ++i) {
    c.houses.cusp[static_cast<std::size_t>(i)] = norm_rad((5.0 + (i - 1) * 30.0) * kDegToRad);
  }
  return c;
}

}  // namespace

TEST_CASE("the histogram scores signs and houses with his weights") {
  const Chart c = synthetic();
  ChartSettings s;
  HistogramOptions opt;
  opt.points = histogram_points({});  // his old fixed values
  opt.double_first_house = false;
  opt.double_ruler = false;
  const Histogram h = chart_histogram(c, s, opt);
  // sun 6 fire, mars 3 water, moon 6 earth, AC 6 fire, MC 6 earth
  CHECK(h.element_sign[1] == 12);
  CHECK(h.element_sign[2] == 12);
  CHECK(h.element_sign[3] == 0);
  CHECK(h.element_sign[4] == 3);
  // cardinal sun mars MC, fixed moon, AC cardinal
  CHECK(h.quality_sign[1] == 6 + 3 + 6 + 6);
  CHECK(h.quality_sign[2] == 6);
  CHECK(h.quality_sign[3] == 0);
  // houses, sun house 1, moon house 2, mars house 4. The axes stand
  // exactly on their cusps and his open windows never score them
  CHECK(h.houses_counted);
  CHECK(h.element_house[1] == 6);
  CHECK(h.element_house[2] == 6);
  CHECK(h.element_house[4] == 3);
}

TEST_CASE("first house planets and the birth ruler double") {
  const Chart c = synthetic();
  ChartSettings s;
  HistogramOptions opt;
  opt.points = histogram_points({});
  opt.double_first_house = true;
  //RR GebHerr doppelt, Widder-AC macht Mars zum Geburtsherrscher
  opt.double_ruler = true;
  const Histogram h = chart_histogram(c, s, opt);
  // the sun sits in house one and doubles, mars doubles as ruler
  CHECK(h.element_sign[1] == 12 + 6);
  CHECK(h.element_sign[4] == 6);
}

TEST_CASE("the equal systems keep the house columns silent") {
  const Chart c = synthetic();
  ChartSettings s;
  s.houses = HouseSystem::kAcMcOnly;
  HistogramOptions opt;
  opt.points = histogram_points({});
  const Histogram h = chart_histogram(c, s, opt);
  CHECK_FALSE(h.houses_counted);
  CHECK(h.element_house[1] == 0);
}

TEST_CASE("a stored weight row replaces the fixed values") {
  std::array<int, 16> pn{};
  pn[1] = 9;
  pn[15] = 4;
  const auto points = histogram_points(pn);
  CHECK(points[body::kSun] == 9);
  CHECK(points[body::kMoon] == 0);
  CHECK(points[body::kChiron] == 4);
  CHECK(points[body::kXena] == 4);
}
