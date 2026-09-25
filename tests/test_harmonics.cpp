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

TEST_CASE("halbsm finds directed midpoints on 0 Aries, the radix cusps and the directed angles") {
  Chart radix = synthetic();
  Chart multi;
  multi.ok = true;
  const auto put = [&multi](int slot, double deg) {
    BodyState& b = multi.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  };
  // Sun and Moon meet at 0 Aries, Mercury and Venus on the second radix
  // cusp at 40, Mars and Jupiter on the directed MC at 150
  put(body::kSun, 350.0);
  put(body::kMoon, 10.0);
  put(body::kMercury, 30.0);
  put(body::kVenus, 50.0);
  put(body::kMars, 140.0);
  put(body::kJupiter, 160.0);
  multi.houses.ok = true;
  multi.houses.cusp[1] = 60.0 * kDegToRad;
  multi.houses.cusp[10] = 150.0 * kDegToRad;
  const std::vector<MultiMidpoint> lines = multi_midpoints(radix, multi, 1.0);
  const auto has = [&](int u, int w, MultiMidpoint::Target target, int first) {
    for (const MultiMidpoint& m : lines) {
      if (m.u == u && m.w == w && m.target == target && m.first == first) {
        return true;
      }
    }
    return false;
  };
  // his loop from aa& = 1 never tested 0 degrees, the axis AR/LI stood
  // only at its 180 degree end
  CHECK(has(body::kSun, body::kMoon, MultiMidpoint::Target::kSignAxis, 1));
  CHECK(has(body::kMercury, body::kVenus, MultiMidpoint::Target::kRadixCusp, 2));
  CHECK(has(body::kMars, body::kJupiter, MultiMidpoint::Target::kMultiAngle, 4));
  // a fifth of a degree times the orb factor, 0.3 degrees away misses
  put(body::kMoon, 10.6);
  bool sun_moon = false;
  for (const MultiMidpoint& m : multi_midpoints(radix, multi, 1.0)) {
    sun_moon = sun_moon || (m.u == body::kSun && m.w == body::kMoon);
  }
  CHECK_FALSE(sun_moon);
}

TEST_CASE("the MULTI world directs the fixed point and leaves the Hamburg factors dark") {
  Chart base = synthetic();
  const auto put = [&base](int slot, double deg) {
    BodyState& b = base.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  };
  put(body::kFixpunkt, 100.0);
  put(body::kCupido, 50.0);
  put(body::kPoseidon, 250.0);
  const MultiReference sun{MultiReference::Kind::kBody, body::kSun, 1, 1};
  // multi11 to multiarc1 run CASE aa& TO 11, the fixed point directs,
  // 100 plus twice its ten degrees within Cancer
  const Chart m1 = multi_chart(base, MultiMode::kMulti1, 2.0, sun, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  REQUIRE(m1.b[body::kFixpunkt].present);
  CHECK(deg(m1.b[body::kFixpunkt].el) == doctest::Approx(120.0));
  // their CASE lists skip n9 to n16, the Hamburg factors fall to DEFAULT
  CHECK_FALSE(m1.b[body::kCupido].present);
  CHECK_FALSE(m1.b[body::kPoseidon].present);
  // the zero point modes know no anchor for the fixed point
  const Chart me = multi_chart(base, MultiMode::kZeroEast, 2.0, sun, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK_FALSE(me.b[body::kFixpunkt].present);
  // harm21 runs CASE 1 TO 11, the fixed point stays empty there
  const Chart h = harmonic_chart(base, 3.0, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK_FALSE(h.b[body::kFixpunkt].present);
  CHECK_FALSE(h.b[body::kCupido].present);
  CHECK(h.b[body::kChiron].present);
}

TEST_CASE("the new MC of mc_armcb reads the tenth cusp") {
  // the equal systems keep the true midheaven apart from the tenth cusp,
  // fz(1,ze,10) is the cusp. The port read angles.mc before
  Chart base = synthetic();
  base.houses.angles.mc = 305.0 * kDegToRad;
  const MultiReference sun{MultiReference::Kind::kBody, body::kSun, 1, 1};
  const Chart mh = multi_chart(base, MultiMode::kMulti1, 2.0, sun, HarmonicHouses::kFromNewMc, HouseSystem::kPlacidus, 48.0);
  REQUIRE(mh.houses.ok);
  // cusp ten at 280 plus twice its ten degrees within Capricorn, the true
  // MC would have given 305 plus twice five
  CHECK(deg(mh.houses.angles.mc) == doctest::Approx(300.0).epsilon(1e-6));
  const Chart hh = harmonic_chart(base, 5.0, HarmonicHouses::kFromNewMc, HouseSystem::kPlacidus, 48.0);
  REQUIRE(hh.houses.ok);
  CHECK(deg(hh.houses.angles.mc) == doctest::Approx(norm_deg(5.0 * 280.0)).epsilon(1e-6));
}

TEST_CASE("the south node reference of mc_armcb1 directs from the node") {
  // his BEZUGS-FAKTOR box offers MONDKNOTEN S, mc_armcb1 has no CASE for
  // it and ran MULTI 3 from e = 0, Mars at 200 went to 0 + 2 * 20 = 40.
  // The port reads the south node at 220
  const Chart base = synthetic();
  const MultiReference ds{MultiReference::Kind::kBody, body::kNodeDesc, 1, 1};
  const Chart m3 = multi_chart(base, MultiMode::kMulti3, 2.0, ds, HarmonicHouses::kLikeBodies, HouseSystem::kPlacidus, 48.0);
  CHECK(deg(m3.b[body::kMars].el) == doctest::Approx(260.0));
  CHECK(deg(m3.b[body::kMars].el) != doctest::Approx(40.0));
}

TEST_CASE("a901_m rebuilds the Glueckspunkt with the day rule of the radix") {
  // a day birth, the Sun at 280 stands above the horizon of the AC at 10
  Chart base = synthetic();
  base.b[body::kSun].el = 280.0 * kDegToRad;
  const MultiReference sun{MultiReference::Kind::kBody, body::kSun, 1, 1};
  const Chart m1 = multi_chart(base, MultiMode::kMulti1, 2.0, sun, HarmonicHouses::kFromNewMc, HouseSystem::kPlacidus, 48.0);
  REQUIRE(m1.houses.ok);
  const double ac = m1.houses.cusp[1];
  const double so = m1.b[body::kSun].el;
  const double mo = m1.b[body::kMoon].el;
  // multi11 had zeroed pl(13) before ta_na read it, every chart took the
  // night formula AC - MO + SO. The port takes the day formula of the birth
  CHECK(m1.b[body::kFortune].el == doctest::Approx(norm_rad(ac + mo - so)));
  CHECK(m1.b[body::kFortune].el != doctest::Approx(norm_rad(ac - mo + so)));
  // the zero point modes never call a901_m, the point keeps its anchor run
  const Chart me = multi_chart(base, MultiMode::kZeroEast, 2.0, sun, HarmonicHouses::kFromNewMc, HouseSystem::kPlacidus, 48.0);
  // 0 + 2 * 1 degree within Cancer
  CHECK(deg(me.b[body::kFortune].el) == doctest::Approx(2.0));
}
