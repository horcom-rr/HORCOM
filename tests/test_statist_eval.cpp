// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/statist_eval.hpp"

using namespace horcom;

namespace {

// a synthetic dataset of two invented charts, no real people
StatSet sample() {
  StatSet set;
  StatRecord a;
  a.name = "ANNA MUSTER";
  a.ac = 10.0 * kDegToRad;
  a.mc = 280.0 * kDegToRad;
  a.h2 = 40.0 * kDegToRad;
  a.h3 = 70.0 * kDegToRad;
  a.h5 = 130.0 * kDegToRad;
  a.h6 = 160.0 * kDegToRad;
  a.el[body::kSun] = 15.0 * kDegToRad;
  a.el[body::kMoon] = 95.0 * kDegToRad;
  a.el[body::kVenus] = 358.0 * kDegToRad;
  a.el[body::kMars] = 105.0 * kDegToRad;
  a.el[body::kJupiter] = 195.0 * kDegToRad;
  StatRecord b;
  b.name = "BERND BEISPIEL";
  b.ac = 190.0 * kDegToRad;
  b.mc = 100.0 * kDegToRad;
  b.h2 = 220.0 * kDegToRad;
  b.h3 = 250.0 * kDegToRad;
  b.h5 = 310.0 * kDegToRad;
  b.h6 = 340.0 * kDegToRad;
  b.el[body::kSun] = 125.5 * kDegToRad;
  b.el[body::kMoon] = 245.0 * kDegToRad;
  b.el[body::kVenus] = 20.0 * kDegToRad;
  b.el[body::kMars] = 305.4 * kDegToRad;
  set.records.push_back(a);
  set.records.push_back(b);
  return set;
}

StatEvalResult run(const StatSet& set, const StatQuery& q, std::vector<double>& mask) {
  const AspectSettings a;
  return evaluate_statistics(set, q, a, mask);
}

}  // namespace

TEST_CASE("haus_def mirrors the six stored cusps into twelve") {
  const StatSet set = sample();
  const auto f = stat_houses(set.records[0]);
  CHECK(f[1] == doctest::Approx(10.0 * kDegToRad));
  CHECK(f[4] == doctest::Approx(100.0 * kDegToRad));
  CHECK(f[7] == doctest::Approx(190.0 * kDegToRad));
  CHECK(f[10] == doctest::Approx(280.0 * kDegToRad));
  CHECK(f[12] == doctest::Approx(340.0 * kDegToRad));
  CHECK(f[13] == f[1]);
}

TEST_CASE("ze_pl hands every sign its ruler, the old ones on demand") {
  CHECK(sign_ruler(15.0 * kDegToRad, false) == body::kMars);
  CHECK(sign_ruler(45.0 * kDegToRad, false) == body::kVenus);
  CHECK(sign_ruler(75.0 * kDegToRad, false) == body::kMercury);
  CHECK(sign_ruler(105.0 * kDegToRad, false) == body::kMoon);
  CHECK(sign_ruler(135.0 * kDegToRad, false) == body::kSun);
  CHECK(sign_ruler(165.0 * kDegToRad, false) == body::kMercury);
  CHECK(sign_ruler(195.0 * kDegToRad, false) == body::kVenus);
  CHECK(sign_ruler(225.0 * kDegToRad, false) == body::kPluto);
  CHECK(sign_ruler(225.0 * kDegToRad, true) == body::kMars);
  CHECK(sign_ruler(255.0 * kDegToRad, false) == body::kJupiter);
  CHECK(sign_ruler(285.0 * kDegToRad, false) == body::kSaturn);
  CHECK(sign_ruler(315.0 * kDegToRad, false) == body::kUranus);
  CHECK(sign_ruler(315.0 * kDegToRad, true) == body::kSaturn);
  CHECK(sign_ruler(345.0 * kDegToRad, false) == body::kNeptune);
  CHECK(sign_ruler(345.0 * kDegToRad, true) == body::kJupiter);
  CHECK(sign_ruler(0.0, false) == 0);
}

TEST_CASE("geb_herr names the ascendant ruler and the intercepted second") {
  // TEST02, AC 24 AR and cusp 2 at 1 GM leave Taurus whole in house 1
  auto k = birth_rulers(24.29 * kDegToRad, 61.16 * kDegToRad, false);
  CHECK(k.first == body::kMars);
  CHECK(k.second == body::kVenus);
  // TEST04, AC 19 PS with cusp 2 at 13 TA intercepts Aries
  k = birth_rulers(348.98 * kDegToRad, 43.32 * kDegToRad, false);
  CHECK(k.first == body::kNeptune);
  CHECK(k.second == body::kMars);
  // a plain chart, cusp 2 in the next sign carries no second ruler
  k = birth_rulers(15.0 * kDegToRad, 40.0 * kDegToRad, false);
  CHECK(k.first == body::kMars);
  CHECK(k.second == 0);
  k = birth_rulers(0.0, 40.0 * kDegToRad, false);
  CHECK(k.first == 0);
}

TEST_CASE("the degree window finds a body and wraps the zero point") {
  const StatSet set = sample();
  StatQuery q;
  q.object = StatObject::kBody;
  q.a.body = body::kSun;
  q.window = StatWindow::kAtDegree;
  q.degree = 125.5 * kDegToRad;
  q.orb = 1.0 * kDegToRad;
  std::vector<double> mask;
  const StatEvalResult r = run(set, q, mask);
  REQUIRE(r.matches.size() == 1);
  CHECK(r.matches[0].record == 1);
  CHECK(r.matches[0].slot == body::kSun);
  CHECK(mask[1] > 0.0);
  CHECK(mask[0] == 0.0);
  // venus at 358 degrees only reaches a window at 2 through the wrap
  StatQuery w;
  w.object = StatObject::kBody;
  w.a.body = body::kVenus;
  w.window = StatWindow::kAtDegree;
  w.degree = 2.0 * kDegToRad;
  w.orb = 5.0 * kDegToRad;
  std::vector<double> wmask;
  const StatEvalResult wr = run(set, w, wmask);
  REQUIRE(wr.matches.size() == 1);
  CHECK(wr.matches[0].record == 0);
}

TEST_CASE("the sign window and the house window read the record") {
  const StatSet set = sample();
  StatQuery q;
  q.object = StatObject::kBody;
  q.a.body = body::kMoon;
  q.window = StatWindow::kInSign;
  q.sign = 4;
  std::vector<double> mask;
  const StatEvalResult r = run(set, q, mask);
  REQUIRE(r.matches.size() == 1);
  CHECK(r.matches[0].record == 0);
  // the sun of the first chart stands in its first house
  StatQuery h;
  h.object = StatObject::kBody;
  h.a.body = body::kSun;
  h.window = StatWindow::kInHouse;
  h.house = 1;
  std::vector<double> hmask;
  const StatEvalResult hr = run(set, h, hmask);
  REQUIRE(hr.matches.size() == 1);
  CHECK(hr.matches[0].record == 0);
  // the house distribution counts where every sun stood, each chart
  // under its own cusps
  CHECK(hr.distribution[1] == 1);
  CHECK(hr.distribution[10] == 1);
}

TEST_CASE("the house orb widens a house over 0 Aries instead of inverting it") {
  // the first house runs from 340 to 10 degrees. His stat_ausw took
  // dw = (w2 - w1) * obp / 100 from the raw cusps, -33 degrees at ten
  // percent, and searched 13 to 337 degrees, the rest of the circle. The
  // house is 30 degrees wide, the orb adds 3 on either side, 337 to 13
  StatSet set;
  const double sun[3] = {350.0, 11.5, 100.0};
  for (const double s : sun) {
    StatRecord r;
    r.ac = 340.0 * kDegToRad;
    r.h2 = 10.0 * kDegToRad;
    r.h3 = 40.0 * kDegToRad;
    r.mc = 250.0 * kDegToRad;
    r.h5 = 110.0 * kDegToRad;
    r.h6 = 140.0 * kDegToRad;
    r.el[body::kSun] = s * kDegToRad;
    set.records.push_back(r);
  }
  StatQuery q;
  q.object = StatObject::kBody;
  q.a.body = body::kSun;
  q.window = StatWindow::kInHouse;
  q.house = 1;
  q.house_orb_pct = 10.0;
  std::vector<double> mask;
  const StatEvalResult r = run(set, q, mask);
  // the Sun inside the house and the one within the orb past its end
  // match, the Sun at 100 degrees, which his window took, does not
  REQUIRE(r.matches.size() == 2);
  CHECK(r.matches[0].record == 0);
  CHECK(r.matches[1].record == 1);
  CHECK(mask[2] == 0.0);
  // without an orb the Sun at 11.5 degrees stands in the second house
  q.house_orb_pct = 0.0;
  std::vector<double> plain;
  const StatEvalResult p = run(set, q, plain);
  REQUIRE(p.matches.size() == 1);
  CHECK(p.matches[0].record == 0);
}

TEST_CASE("only a record without the Sun reads as heliocentric") {
  // his stat2 flagged AC, MC and house 2 at zero, a file of house system
  // 9 or 10 without angles switched the program to HELIO. The helio
  // writer of stat1 empties the Sun as well
  StatRecord none;
  none.el[body::kSun] = 1.0;
  CHECK_FALSE(none.heliocentric());
  StatRecord helio;
  helio.el[body::kMoon] = 2.0;
  CHECK(helio.heliocentric());
  StatSet set;
  set.records.push_back(none);
  CHECK_FALSE(stat_heliocentric(set));
  set.records.push_back(helio);
  CHECK(stat_heliocentric(set));
  // the Sun of a heliocentric file never takes part in SO / MO / AC
  StatQuery q;
  q.object = StatObject::kLights;
  q.window = StatWindow::kAnywhere;
  std::vector<double> mask;
  const StatEvalResult r = run(set, q, mask);
  bool sun_of_helio = false;
  for (const StatMatch& m : r.matches) {
    sun_of_helio = sun_of_helio || (m.record == 1 && m.slot == body::kSun);
  }
  CHECK_FALSE(sun_of_helio);
}

TEST_CASE("the ruler of the first house obeys the ascendant sign") {
  const StatSet set = sample();
  StatQuery q;
  q.object = StatObject::kHouseRuler;
  q.a.house = 1;
  q.window = StatWindow::kInSign;
  q.sign = 4;
  std::vector<double> mask;
  // the first chart rises in Aries, its mars stands in Cancer, the
  // second rises in Libra and venus stands in Aries
  const StatEvalResult r = run(set, q, mask);
  REQUIRE(r.matches.size() == 1);
  CHECK(r.matches[0].record == 0);
  CHECK(r.matches[0].slot == body::kMars);
}

TEST_CASE("the aspect windows fold the separation and honour the filter") {
  const StatSet set = sample();
  StatQuery q;
  q.object = StatObject::kAspect;
  q.a.body = body::kSun;
  q.b.body = body::kMars;
  q.asp_low = 1;
  q.asp_high = 12;
  std::vector<double> mask;
  // the first chart carries an exact square, the second a tight
  // opposition, the table orbs take both
  const StatEvalResult r = run(set, q, mask);
  CHECK(r.matches.size() >= 2);
  bool first = false;
  bool second = false;
  for (const StatMatch& m : r.matches) {
    first = first || m.record == 0;
    second = second || m.record == 1;
  }
  CHECK(first);
  CHECK(second);
  // the single aspect mode with equal orbs skips the doubled multiple,
  // the opposition no longer rides the square's divisor
  StatQuery s = q;
  s.asp_low = 4;
  s.asp_high = 4;
  s.asp_orb = 1.0 * kDegToRad;
  AspectSettings ea;
  ea.equal_probability = true;
  ea.preset_equal_orbs();
  std::vector<double> smask;
  const StatEvalResult sr = evaluate_statistics(set, s, ea, smask);
  REQUIRE(sr.matches.size() == 1);
  CHECK(sr.matches[0].record == 0);
}

TEST_CASE("the divisor inputs keep the limits of stat_ausw") {
  // a single aspect reaches twelve, a range stops at eight, beyond that
  // the UND flags of one record ran past their twelve slots
  const StatSet set = sample();
  StatQuery wide;
  wide.object = StatObject::kAspect;
  wide.a.body = body::kSun;
  wide.b.body = body::kMars;
  wide.combine_and = true;
  wide.asp_low = 1;
  wide.asp_high = 16;
  StatQuery eight = wide;
  eight.asp_high = kStatMaxRangeDivisor;
  std::vector<double> m1(set.records.size(), 1.0);
  std::vector<double> m2(set.records.size(), 1.0);
  const StatEvalResult r1 = run(set, wide, m1);
  const StatEvalResult r2 = run(set, eight, m2);
  CHECK(r1.matches.size() == r2.matches.size());
  CHECK(m1 == m2);
  StatQuery single = wide;
  single.asp_low = 16;
  single.asp_high = 16;
  std::vector<double> m3(set.records.size(), 1.0);
  (void)run(set, single, m3);
  CHECK(kStatMaxSingleDivisor == 12);
}

TEST_CASE("UND chaining keeps only the survivors of both conditions") {
  const StatSet set = sample();
  StatQuery first;
  first.object = StatObject::kBody;
  first.a.body = body::kSun;
  first.window = StatWindow::kInSign;
  first.sign = 1;
  std::vector<double> mask;
  (void)run(set, first, mask);
  CHECK(mask[0] > 0.0);
  CHECK(mask[1] == 0.0);
  StatQuery second;
  second.object = StatObject::kBody;
  second.a.body = body::kMoon;
  second.window = StatWindow::kInSign;
  second.sign = 4;
  second.combine_and = true;
  const StatEvalResult r = run(set, second, mask);
  REQUIRE(r.matches.size() == 1);
  CHECK(r.matches[0].record == 0);
  CHECK(mask[0] > 0.0);
  // a third condition that fails clears the survivor
  StatQuery third = second;
  third.sign = 8;
  (void)run(set, third, mask);
  CHECK(mask[0] == 0.0);
}

TEST_CASE("the Aries window with an orb reaches both sides of 0 Aries") {
  // venus of the second chart at 20, of the first at 358, the sun of
  // the first at 15. His w1 = ABS(wu) skipped 0 to 5 degrees
  StatSet set = sample();
  set.records[1].el[body::kVenus] = 3.0 * kDegToRad;
  StatQuery q;
  q.object = StatObject::kBody;
  q.a.body = body::kVenus;
  q.window = StatWindow::kInSign;
  q.sign = 1;
  // the sign window takes its orb in whole degrees like his numw
  q.orb = 5.0;
  std::vector<double> mask;
  const StatEvalResult r = run(set, q, mask);
  REQUIRE(r.matches.size() == 2);
  CHECK(mask[0] > 0.0);
  CHECK(mask[1] > 0.0);
}

TEST_CASE("UND over a window around 0 Aries keeps the survivors") {
  // his two passes cleared each other's survivors, nothing ever stayed
  StatSet set = sample();
  set.records[1].el[body::kVenus] = 3.0 * kDegToRad;
  StatQuery any;
  any.object = StatObject::kBody;
  any.a.body = body::kSun;
  any.window = StatWindow::kAnywhere;
  std::vector<double> mask;
  (void)run(set, any, mask);
  REQUIRE(mask[0] > 0.0);
  REQUIRE(mask[1] > 0.0);
  StatQuery around;
  around.object = StatObject::kBody;
  around.a.body = body::kVenus;
  around.window = StatWindow::kAtDegree;
  around.degree = 0.5 * kDegToRad;
  around.orb = 5.0 * kDegToRad;
  around.combine_and = true;
  (void)run(set, around, mask);
  // venus at 358 lies in the wrapped part, venus at 3 in the first pass
  CHECK(mask[0] > 0.0);
  CHECK(mask[1] > 0.0);
  // a window that misses both still clears them
  StatQuery miss = around;
  miss.degree = 180.0 * kDegToRad;
  (void)run(set, miss, mask);
  CHECK(mask[0] == 0.0);
  CHECK(mask[1] == 0.0);
}

TEST_CASE("names, midpoints, mirrors and the arabic part all answer") {
  const StatSet set = sample();
  StatQuery n;
  n.object = StatObject::kName;
  n.name = "BEISPIEL";
  std::vector<double> mask;
  const StatEvalResult nr = run(set, n, mask);
  REQUIRE(nr.matches.size() == 1);
  CHECK(nr.matches[0].record == 1);

  StatQuery m;
  m.object = StatObject::kMidpoint;
  m.a.body = body::kSun;
  m.b.body = body::kMoon;
  m.window = StatWindow::kAtDegree;
  m.degree = 55.0 * kDegToRad;
  m.orb = 1.0 * kDegToRad;
  std::vector<double> mmask;
  const StatEvalResult mr = run(set, m, mmask);
  REQUIRE(mr.matches.size() == 1);
  CHECK(mr.matches[0].record == 0);

  StatQuery g;
  g.object = StatObject::kMirror;
  g.a.body = body::kSun;
  g.mirror = MirrorAxis::kAriesLibra;
  g.window = StatWindow::kAtDegree;
  g.degree = 345.0 * kDegToRad;
  g.orb = 1.0 * kDegToRad;
  std::vector<double> gmask;
  const StatEvalResult gr = run(set, g, gmask);
  REQUIRE(gr.matches.size() == 1);
  CHECK(gr.matches[0].record == 0);

  // the first chart is a night chart, the part of fortune formula
  // flips to AC plus sun minus moon, ten plus fifteen minus ninety
  // five wrapped to two hundred ninety degrees
  StatQuery p;
  p.object = StatObject::kArabicPart;
  p.a.body = body::kAscendant;
  p.b.body = body::kSun;
  p.c.body = body::kMoon;
  p.window = StatWindow::kAtDegree;
  p.degree = 290.0 * kDegToRad;
  p.orb = 1.0 * kDegToRad;
  std::vector<double> pmask;
  const StatEvalResult pr = run(set, p, pmask);
  REQUIRE(pr.matches.size() == 1);
  CHECK(pr.matches[0].record == 0);
}

TEST_CASE("UND keeps a record for an aspect condition only when it holds") {
  // his bed_erf_asp cleared only after an earlier hit of the same multiple,
  // both records below kept their mask for a conjunction neither has
  const StatSet set = sample();
  StatQuery q;
  q.object = StatObject::kAspect;
  q.a.body = body::kSun;
  q.b.body = body::kMars;
  q.combine_and = true;
  q.asp_low = 1;
  q.asp_high = 1;
  std::vector<double> mask(set.records.size(), 1.0);
  (void)run(set, q, mask);
  CHECK(mask[0] == 0.0);
  CHECK(mask[1] == 0.0);
  // square and opposition hold, both records stay
  q.asp_high = 4;
  std::vector<double> kept(set.records.size(), 1.0);
  (void)run(set, q, kept);
  CHECK(kept[0] > 0.0);
  CHECK(kept[1] > 0.0);
}

TEST_CASE("SO MO AC under UND keeps a record when any light matches") {
  // the first chart has the Sun and the AC in Aries but the Moon in
  // Cancer, his walk cleared it on the Moon's miss
  const StatSet set = sample();
  StatQuery q;
  q.object = StatObject::kLights;
  q.window = StatWindow::kInSign;
  q.sign = 1;
  q.combine_and = true;
  std::vector<double> mask(set.records.size(), 1.0);
  (void)run(set, q, mask);
  CHECK(mask[0] > 0.0);
  CHECK(mask[1] == 0.0);
}

TEST_CASE("LAGE Bei PLANET reaches over 0 Aries") {
  // the first chart's Sun at 15 stands 17 degrees from Venus at 358, his
  // unnormalised window ran from 338 to 378 and never matched
  const StatSet set = sample();
  StatQuery q;
  q.object = StatObject::kBody;
  q.a.body = body::kSun;
  q.window = StatWindow::kNearBody;
  q.near_body.body = body::kVenus;
  q.orb = 20.0 * kDegToRad;
  std::vector<double> mask;
  const StatEvalResult r = run(set, q, mask);
  REQUIRE(r.matches.size() == 1);
  CHECK(r.matches[0].record == 0);
}

TEST_CASE("the lights and all bodies walks answer as an OR over the group") {
  const StatSet set = sample();
  StatQuery q;
  q.object = StatObject::kAllBodies;
  q.window = StatWindow::kAnywhere;
  std::vector<double> mask;
  // seven filled slots on the first chart, six on the second, the
  // empty slots fail the zero guard, a hit keeps the record's mask
  const StatEvalResult r = run(set, q, mask);
  CHECK(r.matches.size() == 13);
  for (const double v : mask) {
    CHECK(v > 0.0);
  }
  // the sign distribution counted every tested longitude
  CHECK(r.distribution[0] == 13);
  StatQuery l;
  l.object = StatObject::kLights;
  l.window = StatWindow::kAnywhere;
  std::vector<double> lmask;
  const StatEvalResult lr = run(set, l, lmask);
  CHECK(lr.matches.size() == 6);
}
