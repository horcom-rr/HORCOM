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

// the old fixed values punkte_pla keeps as a comment, a handy row with
// distinct weights per class
constexpr std::array<int, 16> kFixedRow = {0, 6, 6, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 6, 6, 0};

}  // namespace

TEST_CASE("the histogram scores signs and houses with his weights") {
  const Chart c = synthetic();
  ChartSettings s;
  HistogramOptions opt;
  opt.points = histogram_points(kFixedRow);
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
  opt.points = histogram_points(kFixedRow);
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
  opt.points = histogram_points(kFixedRow);
  const Histogram h = chart_histogram(c, s, opt);
  CHECK_FALSE(h.houses_counted);
  CHECK(h.element_house[1] == 0);
}

TEST_CASE("a stored weight row weighs its slots and every extra body") {
  std::array<int, 16> pn{};
  pn[1] = 9;
  pn[15] = 4;
  const auto points = histogram_points(pn);
  CHECK(points[body::kSun] == 9);
  CHECK(points[body::kMoon] == 0);
  CHECK(points[body::kChiron] == 4);
  CHECK(points[body::kXena] == 4);
}

TEST_CASE("a row without weights for SO to MC counts each point once") {
  // kon_dhol sets pn 1 to 14 to one, the extra weight stays as stored
  std::array<int, 16> pn{};
  pn[15] = 3;
  const auto points = histogram_points(pn);
  CHECK(points[body::kSun] == 1);
  CHECK(points[body::kMc] == 1);
  CHECK(points[body::kChiron] == 3);
  const auto empty = histogram_points({});
  CHECK(empty[body::kPluto] == 1);
  CHECK(empty[body::kChiron] == 0);
}

TEST_CASE("the house quality column honours both doubling switches") {
  // kard_fix_gemh doubled the first house and the ruler without asking
  // haus1_dop and gebherr_dop, the other three passes asked. Switched
  // off the original gave 12 for the Sun and 6 for Mars, 18 in all
  const Chart c = synthetic();
  ChartSettings s;
  HistogramOptions opt;
  opt.points = histogram_points(kFixedRow);
  opt.double_first_house = false;
  opt.double_ruler = false;
  const Histogram h = chart_histogram(c, s, opt);
  // Sun in house 1 and Mars in house 4 are cardinal houses, Moon in 2 fixed
  CHECK(h.quality_house[1] == 6 + 3);
  CHECK(h.quality_house[2] == 6);
}

TEST_CASE("a body at exactly zero Aries or on a sign boundary keeps its sign") {
  // his windows between kk edges let both fall out of every sign
  Chart c = synthetic();
  c.b[body::kSun].el = 0.0;
  c.b[body::kMoon].el = 30.0 * kDegToRad;
  HistogramOptions opt;
  opt.points = histogram_points(kFixedRow);
  const Histogram h = chart_histogram(c, {}, opt);
  Chart d = synthetic();
  d.b[body::kSun].el = 0.5 * kDegToRad;
  d.b[body::kMoon].el = 30.5 * kDegToRad;
  const Histogram g = chart_histogram(d, {}, opt);
  for (int i = 1; i <= 4; ++i) {
    CAPTURE(i);
    CHECK(h.element_sign[static_cast<std::size_t>(i)] == g.element_sign[static_cast<std::size_t>(i)]);
  }
  for (int i = 1; i <= 3; ++i) {
    CAPTURE(i);
    CHECK(h.quality_sign[static_cast<std::size_t>(i)] == g.quality_sign[static_cast<std::size_t>(i)]);
  }
}
