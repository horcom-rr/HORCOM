// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/harmonics.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

using namespace horcom;

namespace {

// a synthetic chart, the transforms are pure so no ephemeris is needed
Chart synthetic() {
  Chart c;
  c.ok = true;
  c.ekls0 = 23.44 * kDegToRad;
  const std::initializer_list<std::pair<int, double>> bodies = {
      {body::kSun, 15.0},      {body::kMoon, 95.0},     {body::kMars, 200.0},
      {body::kNodeAsc, 40.0},  {body::kNodeDesc, 220.0}, {body::kTranspluto, 123.0},
      {body::kChiron, 300.0},  {body::kFortune, 91.0},   {body::kAscendant, 10.0},
      {body::kMc, 280.0}};
  for (const auto& [slot, deg] : bodies) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  }
  c.houses.ok = true;
  const double cusps[12] = {10.0, 40.0, 70.0, 100.0, 130.0, 160.0, 190.0, 220.0, 250.0, 280.0, 310.0, 340.0};
  for (int k = 1; k <= 12; ++k) {
    c.houses.cusp[static_cast<std::size_t>(k)] = cusps[k - 1] * kDegToRad;
  }
  c.houses.cusp[13] = c.houses.cusp[1];
  c.houses.angles.ac = c.houses.cusp[1];
  c.houses.angles.mc = c.houses.cusp[10];
  return c;
}

double deg(double rad) {
  return norm_rad(rad) * kRadToDeg;
}

}  // namespace

TEST_CASE("the harmonic multiplies longitudes and keeps his quirks") {
  const Chart base = synthetic();
  // an even order separates the node quirk, four times the south node
  // itself would land on the north node
  const Chart h = harmonic_chart(base, 4.0, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK(deg(h.b[body::kSun].el) == doctest::Approx(60.0));
  CHECK(deg(h.b[body::kMars].el) == doctest::Approx(800.0 - 720.0));
  // the south node follows the transformed north node, not its own turn
  CHECK(deg(h.b[body::kNodeAsc].el) == doctest::Approx(160.0));
  CHECK(deg(h.b[body::kNodeDesc].el) == doctest::Approx(340.0));
  // Transpluto goes dark like the CASE list leaves it
  CHECK_FALSE(h.b[body::kTranspluto].present);
  CHECK(deg(h.b[body::kChiron].el) == doctest::Approx(1200.0 - 1080.0));
  // every cusp times the order in the standard mode
  CHECK(deg(h.houses.cusp[1]) == doctest::Approx(40.0));
  CHECK(deg(h.houses.cusp[2]) == doctest::Approx(160.0));
  CHECK(deg(h.b[body::kAscendant].el) == doctest::Approx(40.0));
  CHECK(deg(h.b[body::kMc].el) == doctest::Approx(deg(h.houses.cusp[10])));
  // the a901_m rebuild stays commented out in harm21, the point of
  // fortune multiplies like every other extra
  CHECK(deg(h.b[body::kFortune].el) == doctest::Approx(4.0));
}

TEST_CASE("the recomputed harmonic houses answer to the new MC") {
  const Chart base = synthetic();
  const Chart h = harmonic_chart(base, 5.0, HarmonicHouses::kFromNewMc, HouseSystem::kPlacidus, 48.0);
  REQUIRE(h.houses.ok);
  // eckp1 from the derived armc reproduces the multiplied MC
  CHECK(deg(h.houses.angles.mc) == doctest::Approx(norm_deg(5.0 * 280.0)).epsilon(1e-6));
}

TEST_CASE("the 90 degree circle transforms like a12f") {
  const Chart base = synthetic();
  Chart d = dial_chart(base, 4.0);
  CHECK(deg(d.b[body::kSun].el) == doctest::Approx(60.0));
  CHECK(deg(d.b[body::kMars].el) == doctest::Approx(800.0 - 720.0));
  // the aspect scan sees the ascendant at four times its longitude
  CHECK(deg(d.b[body::kAscendant].el) == doctest::Approx(40.0));
  // axes ride along, intermediates clear
  CHECK(deg(d.houses.cusp[1]) == doctest::Approx(40.0));
  CHECK(deg(d.houses.cusp[10]) == doctest::Approx(norm_deg(4.0 * 280.0)));
  CHECK(d.houses.cusp[2] == 0.0);
  // the rotation source stays the radix ascendant
  CHECK(d.houses.angles.ac == base.houses.angles.ac);
  // the divide back folds the angles into the first quarter
  dial_display(d, 4.0);
  CHECK(deg(d.b[body::kAscendant].el) == doctest::Approx(10.0));
  CHECK(deg(d.b[body::kMc].el) == doctest::Approx(norm_deg(4.0 * 280.0) / 4.0));
}

TEST_CASE("the multi directions run every mode over the radix") {
  const Chart base = synthetic();
  const MultiReference sun{MultiReference::Kind::kBody, body::kSun, 1, 1};
  const double lja = 2.0;
  // each body advances by age times its degree within the sign
  Chart m1 = multi_chart(base, MultiMode::kMulti1, lja, sun, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK(deg(m1.b[body::kMars].el) == doctest::Approx(240.0));
  CHECK(deg(m1.b[body::kNodeAsc].el) == doctest::Approx(60.0));
  CHECK(deg(m1.b[body::kNodeDesc].el) == doctest::Approx(240.0));
  CHECK_FALSE(m1.b[body::kTranspluto].present);
  CHECK(deg(m1.houses.cusp[1]) == doctest::Approx(30.0));
  CHECK(m1.houses.cusp[2] == 0.0);
  // the whole longitude drives the second mode
  const Chart m2 = multi_chart(base, MultiMode::kMulti2, lja, sun, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK(deg(m2.b[body::kMars].el) == doctest::Approx(240.0));
  CHECK(deg(m2.b[body::kSun].el) == doctest::Approx(45.0));
  // the third mode runs from a reference point
  const Chart m3 = multi_chart(base, MultiMode::kMulti3, lja, sun, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK(deg(m3.b[body::kMars].el) == doctest::Approx(55.0));
  // the zero point modes anchor every body on its rulership sign
  const Chart me = multi_chart(base, MultiMode::kZeroEast, lja, sun, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK(deg(me.b[body::kSun].el) == doctest::Approx(150.0));
  CHECK(deg(me.b[body::kMoon].el) == doctest::Approx(100.0));
  CHECK(deg(me.b[body::kMars].el) == doctest::Approx(40.0));
  CHECK(deg(me.houses.cusp[1]) == doctest::Approx(20.0));
  CHECK(deg(me.houses.cusp[10]) == doctest::Approx(290.0));
  const Chart mw = multi_chart(base, MultiMode::kZeroWest, lja, sun, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK(deg(mw.b[body::kMars].el) == doctest::Approx(250.0));
  // the arc mode scales the distance to the reference
  const Chart ma = multi_chart(base, MultiMode::kArc, lja, sun, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK(deg(ma.b[body::kMars].el) == doctest::Approx(25.0));
  // the recomputed houses answer to the directed midheaven
  const Chart mh = multi_chart(base, MultiMode::kMulti1, lja, sun, HarmonicHouses::kFromNewMc, HouseSystem::kPlacidus, 48.0);
  REQUIRE(mh.houses.ok);
  CHECK(deg(mh.houses.angles.mc) == doctest::Approx(300.0).epsilon(1e-6));
}
