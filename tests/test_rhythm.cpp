// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <initializer_list>
#include <utility>
#include <vector>

#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/rhythm.hpp"
#include "horcom/core/constants.hpp"

using namespace horcom;

namespace {

// equal houses of thirty degrees from 10 Aries, the bodies at the given
// longitudes, a synthetic sky for the walk
Chart walk_chart(std::initializer_list<std::pair<int, double>> bodies) {
  Chart c;
  c.ok = true;
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
  return c;
}

}  // namespace

TEST_CASE("the published degree 107 is known to the own degree definition") {
  // his gs& list of a17_3 skipped the half degree 214, so the BEREITS
  // VORHANDEN test answered false there while a174 marked it MO-SA
  CHECK(degree_known(214, {}));
  const Chart c = walk_chart({{body::kSun, 15.0}});
  RhythmOptions opt;
  const std::vector<DegreeDate> rows = degree_dates(c, opt, {}, false, 48.0);
  REQUIRE(rows.size() == 720);
  CHECK(rows[214].p == body::kMoon);
  CHECK(rows[214].q == body::kSaturn);
  CHECK_FALSE(rows[214].custom);
  // an own definition there yields to the published pair
  const std::vector<DegreeDate> own = degree_dates(c, opt, {{107.0, body::kMars, body::kVenus}}, false, 48.0);
  CHECK(own[214].p == body::kMoon);
  CHECK_FALSE(own[214].custom);
}

TEST_CASE("the phases of the walk count from the start house in both directions") {
  RhythmOptions opt;
  opt.begin_house = 4;
  opt.leftward = true;
  CHECK(rhythm_phase_count(opt) == 9);
  CHECK(rhythm_phase_house(opt, 1) == 4);
  CHECK(rhythm_phase_house(opt, 9) == 12);
  opt.leftward = false;
  CHECK(rhythm_phase_house(opt, 1) == 9);
  CHECK(rhythm_phase_house(opt, 9) == 1);
  opt.begin_house = 1;
  CHECK(rhythm_phase_count(opt) == 12);
  CHECK(rhythm_phase_house(opt, 1) == 12);
  CHECK(rhythm_phase_house(opt, 12) == 1);
  // the trigger walk steps the same houses
  const Chart c = walk_chart({{body::kSun, 15.0}, {body::kMars, 105.0}});
  AspectSettings as;
  as.divisors = 4;
  const AspectResult scan = scan_aspects(c, ChartSettings{}, as);
  opt.begin_house = 4;
  for (const bool left : {true, false}) {
    opt.leftward = left;
    for (const RhythmTrigger& t : rhythm_triggers(c, scan, as, opt)) {
      CHECK(t.house == rhythm_phase_house(opt, t.phase));
      CHECK(t.phase <= rhythm_phase_count(opt));
    }
  }
}

TEST_CASE("an aspect trigger carries the family of a1720") {
  // Mars squares, Jupiter trines and Venus opposes the Sun
  const Chart c = walk_chart({{body::kSun, 15.0}, {body::kMars, 105.0}, {body::kJupiter, 135.0}, {body::kVenus, 195.0}});
  AspectSettings as;
  as.divisors = 4;
  const AspectResult scan = scan_aspects(c, ChartSettings{}, as);
  RhythmOptions opt;
  bool square = false;
  bool trine = false;
  bool opposition = false;
  for (const RhythmTrigger& t : rhythm_triggers(c, scan, as, opt)) {
    if (t.kind != RhythmKind::kAspect) {
      CHECK(t.family == 0);
      continue;
    }
    // FIX(kk + pv2 / wa) on the folded angle, the main aspects only
    CHECK(t.family >= 1);
    CHECK(t.family <= 4);
    if (t.slot == body::kSun || t.source == body::kSun) {
      const int other = t.slot == body::kSun ? t.source : t.slot;
      square = square || (other == body::kMars && t.family == 4);
      trine = trine || (other == body::kJupiter && t.family == 3);
      opposition = opposition || (other == body::kVenus && t.family == 2);
    }
  }
  CHECK(square);
  CHECK(trine);
  CHECK(opposition);
  CHECK(rhythm_ruler(RhythmKind::kRuler2));
  CHECK_FALSE(rhythm_ruler(RhythmKind::kDirect));
}

TEST_CASE("the clock opens with his tropical year of the program start") {
  const RhythmClock c;
  CHECK(c.tja == kInitialTropicalYearDays);
  CHECK(kInitialTropicalYearDays == doctest::Approx(365.24219878).epsilon(1e-12));
}
