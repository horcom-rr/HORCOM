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

TEST_CASE("the lights and all bodies walks never write the mask") {
  const StatSet set = sample();
  StatQuery q;
  q.object = StatObject::kAllBodies;
  q.window = StatWindow::kAnywhere;
  std::vector<double> mask;
  // seven filled slots on the first chart, six on the second, the
  // empty slots fail the zero guard
  const StatEvalResult r = run(set, q, mask);
  CHECK(r.matches.size() == 13);
  for (const double v : mask) {
    CHECK(v == 0.0);
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
