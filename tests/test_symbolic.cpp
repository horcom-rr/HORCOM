// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/rhythm.hpp"
#include "horcom/chart/symbolic.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

using namespace horcom;

namespace {

// a synthetic radix, the direction arcs are pure geometry
Chart synthetic() {
  Chart c;
  c.ok = true;
  c.armc_deg = 100.0;
  c.smo.ekls = 23.44 * kDegToRad;
  const std::initializer_list<std::pair<int, double>> bodies = {
      {body::kSun, 15.0}, {body::kMoon, 95.0}, {body::kMars, 200.0}};
  for (const auto& [slot, deg] : bodies) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
    b.eb = 1.0 * kDegToRad;
  }
  c.houses.ok = true;
  const double cusps[12] = {10.0, 40.0, 70.0, 100.0, 130.0, 160.0, 190.0, 220.0, 250.0, 280.0, 310.0, 340.0};
  for (int k = 1; k <= 12; ++k) {
    c.houses.cusp[static_cast<std::size_t>(k)] = cusps[k - 1] * kDegToRad;
  }
  return c;
}

const DirectionHit* find_hit(const std::vector<DirectionHit>& hits, int t, int u, int w) {
  for (const DirectionHit& h : hits) {
    if (h.directed == t && h.target == u && h.multiple == w) {
      return &h;
    }
  }
  return nullptr;
}

}  // namespace

TEST_CASE("the ecliptic symbolic direction turns arcs into ages") {
  const Chart c = synthetic();
  DirectionRange range;
  range.from_years = 0.0;
  range.to_years = 180.0;
  range.base_angle_deg = 90.0;
  const std::vector<DirectionHit> hits = direction_hits(c, DirectionMethod::kSymbolicEcliptic, range, 48.0);
  // the ten degree arc between the moon and the sun's square lists
  // from the sun's side, the walk reaches that pair first and the
  // mirror from the moon's side stays silent
  const DirectionHit* ten = find_hit(hits, body::kSun, body::kMoon, 3);
  REQUIRE(ten != nullptr);
  CHECK(ten->arc_deg == doctest::Approx(10.0));
  CHECK(ten->years == doctest::Approx(10.0));
  CHECK(find_hit(hits, body::kMoon, body::kSun, 1) == nullptr);
  // one arc between two points lists once, the mirror stays silent
  const bool mars_sun = find_hit(hits, body::kMars, body::kSun, 0) != nullptr;
  const bool sun_mars = find_hit(hits, body::kSun, body::kMars, 0) != nullptr;
  CHECK(mars_sun != sun_mars);
  // the half key halves the ages
  DirectionRange half = range;
  half.key = 0.5;
  const std::vector<DirectionHit> slow = direction_hits(c, DirectionMethod::kSymbolicEcliptic, half, 48.0);
  const DirectionHit* ten_half = find_hit(slow, body::kSun, body::kMoon, 3);
  REQUIRE(ten_half != nullptr);
  CHECK(ten_half->years == doctest::Approx(5.0));
}

TEST_CASE("the equatorial, mundane and primary frames answer") {
  const Chart c = synthetic();
  DirectionRange range;
  range.to_years = 180.0;
  range.base_angle_deg = 90.0;
  for (const DirectionMethod m : {DirectionMethod::kSymbolicEquatorial, DirectionMethod::kSymbolicMundane, DirectionMethod::kPrimary}) {
    const std::vector<DirectionHit> hits = direction_hits(c, m, range, 48.0);
    CHECK(!hits.empty());
    for (const DirectionHit& h : hits) {
      CHECK(h.years > 0.0);
      CHECK(h.arc_deg > range.from_years);
      CHECK(h.arc_deg <= range.to_years);
      CHECK(h.directed != h.target);
    }
  }
  // the cusp extras join as targets
  DirectionRange cusps = range;
  cusps.extras = DirectionExtras::kCusps;
  const std::vector<DirectionHit> with = direction_hits(c, DirectionMethod::kSymbolicEcliptic, cusps, 48.0);
  bool cusp_seen = false;
  for (const DirectionHit& h : with) {
    cusp_seen = cusp_seen || (h.target >= 15 && h.target <= 18) || (h.directed >= 15 && h.directed <= 18);
  }
  CHECK(cusp_seen);
}

TEST_CASE("the rhythm walk triggers houses, rulers and chains") {
  Chart c;
  c.ok = true;
  const std::initializer_list<std::pair<int, double>> bodies = {
      {body::kSun, 15.0}, {body::kMars, 105.0}, {body::kAscendant, 10.0}, {body::kMc, 280.0}};
  for (const auto& [slot, deg] : bodies) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  }
  c.houses.ok = true;
  for (int k = 1; k <= 12; ++k) {
    c.houses.cusp[static_cast<std::size_t>(k)] = (10.0 + (k - 1) * 30.0) * kDegToRad;
  }
  c.houses.cusp[13] = c.houses.cusp[1];
  AspectSettings as;
  as.divisors = 4;
  ChartSettings cs;
  const AspectResult scan = scan_aspects(c, cs, as);
  RhythmOptions opt;
  const std::vector<RhythmTrigger> rows = rhythm_triggers(c, scan, as, opt);
  REQUIRE(!rows.empty());
  // the first phase rises in Aries, its ruler mars stands five degrees
  // into the fourth house, one sixth of a seven year phase
  bool ruler = false;
  bool direct = false;
  bool chain = false;
  for (const RhythmTrigger& t : rows) {
    if (t.phase == 1 && t.kind == RhythmKind::kRuler && t.slot == body::kMars) {
      ruler = true;
      CHECK(t.value == doctest::Approx(7.0 / 6.0).epsilon(1e-6));
    }
    if (t.phase == 1 && t.kind == RhythmKind::kDirect && t.slot == body::kSun) {
      direct = true;
      CHECK(t.value == doctest::Approx(7.0 / 6.0).epsilon(1e-6));
    }
    if (t.phase == 1 && t.kind == RhythmKind::kAspect && t.slot == body::kSun && t.source == body::kMars) {
      chain = true;
      CHECK(t.angle_deg == doctest::Approx(90.0));
    }
    // the tenth phase carries the midheaven at its cusp
    if (t.phase == 10 && t.kind == RhythmKind::kDirect && t.slot == body::kMc) {
      CHECK(t.value == doctest::Approx(63.0).epsilon(1e-4));
    }
  }
  CHECK(ruler);
  CHECK(direct);
  CHECK(chain);
  // the month unit divides every age by twelve
  RhythmOptions monthly = opt;
  monthly.months = true;
  const std::vector<RhythmTrigger> m = rhythm_triggers(c, scan, as, monthly);
  bool scaled = false;
  for (const RhythmTrigger& t : m) {
    if (t.phase == 1 && t.kind == RhythmKind::kDirect && t.slot == body::kSun) {
      scaled = t.value == doctest::Approx(7.0 / 72.0).epsilon(1e-6);
    }
  }
  CHECK(scaled);
}
