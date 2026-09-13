// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/aspects.hpp"
#include "horcom/core/constants.hpp"

using namespace horcom;

namespace {

// a synthetic chart, positions in degrees on chosen slots
Chart synthetic(std::initializer_list<std::pair<int, double>> positions) {
  Chart c;
  c.ok = true;
  for (const auto& [slot, deg] : positions) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  }
  return c;
}

double cell(const AspectResult& r, int t, int w) {
  return r.asp[static_cast<std::size_t>(t)][static_cast<std::size_t>(w)];
}

}  // namespace

TEST_CASE("a trine lands in the divisor three family") {
  const Chart c = synthetic({{1, 10.0}, {2, 130.5}});
  const AspectResult r = scan_aspects(c, {}, {});
  // base orb pn / 30 is four degrees for the trine
  CHECK(r.zh[3] == 1);
  CHECK(cell(r, 1, 2) == doctest::Approx(kTwoPi / 3.0));
  CHECK(r.az[1] == 1);
  CHECK(r.az[2] == 1);
}

TEST_CASE("the conjunction survives the zero wrap") {
  const Chart c = synthetic({{1, 359.0}, {2, 1.5}});
  const AspectResult r = scan_aspects(c, {}, {});
  CHECK(r.zh[1] == 1);
  CHECK(cell(r, 1, 2) == doctest::Approx(kTwoPi));
}

TEST_CASE("the smaller divisor wins and blocks the pair") {
  // 180 degrees is the opposition, divisor two, and must never count
  // again as twice ninety under divisor four
  const Chart c = synthetic({{1, 20.0}, {5, 200.0}});
  const AspectResult r = scan_aspects(c, {}, {});
  CHECK(r.zh[2] == 1);
  CHECK(r.zh[4] == 0);
  CHECK(cell(r, 1, 5) == doctest::Approx(kPi));
}

TEST_CASE("the lunar nodes never aspect each other") {
  const Chart c = synthetic({{11, 100.0}, {12, 280.0}});
  const AspectResult r = scan_aspects(c, {}, {});
  CHECK(r.hits.empty());
}

TEST_CASE("a body with weight zero is silent") {
  const Chart c = synthetic({{1, 10.0}, {2, 130.0}});
  AspectSettings a;
  a.weight[2] = 0;
  const AspectResult r = scan_aspects(c, {}, a);
  CHECK(r.hits.empty());
}

TEST_CASE("the orb factor scales the window") {
  const Chart c = synthetic({{1, 5.0}, {2, 130.0}});
  AspectSettings a;
  const AspectResult wide = scan_aspects(c, {}, a);
  CHECK(wide.zh[3] == 0);
  a.orb = 1.5;
  const AspectResult wider = scan_aspects(c, {}, a);
  CHECK(wider.zh[3] == 1);
}

TEST_CASE("equal probability mode keeps only the classical multiples") {
  AspectSettings a;
  a.equal_probability = true;
  a.preset_equal_orbs();
  // 150 degrees is five twelfths, an allowed multiple of divisor twelve
  const Chart quincunx = synthetic({{1, 10.0}, {2, 160.0}});
  const AspectResult r1 = scan_aspects(quincunx, {}, a);
  CHECK(r1.zh[12] == 1);
  // 165 degrees would be eleven twenty fourths, no divisor of the table
  // owns it, under the tight equal orbs nothing may fire
  const Chart odd = synthetic({{1, 10.0}, {2, 175.05}});
  const AspectResult r2 = scan_aspects(odd, {}, a);
  for (int n = 1; n <= 12; ++n) {
    CAPTURE(n);
    CHECK(r2.zh[static_cast<std::size_t>(n)] == 0);
  }
}

TEST_CASE("the Schiemenz Triga counter sees a chained conjunction") {
  const Chart c = synthetic({{1, 10.0}, {2, 12.0}, {3, 14.0}});
  const AspectResult r = scan_aspects(c, {}, {});
  CHECK(r.zh[1] == 3);
  CHECK(r.triga >= 1);
}

TEST_CASE("the Schiemenz counter sees a grand trine") {
  const Chart c = synthetic({{1, 10.0}, {5, 130.0}, {6, 250.0}});
  const AspectResult r = scan_aspects(c, {}, {});
  CHECK(r.zh[3] == 3);
  CHECK(r.grand_trines == 1);
}

TEST_CASE("a body at exactly zero Aries never aspects, like the original") {
  const Chart c = synthetic({{1, 0.0}, {2, 120.0}});
  const AspectResult r = scan_aspects(c, {}, {});
  CHECK(r.hits.empty());
}

TEST_CASE("midpoints count direct, square and semi square contacts") {
  // Venus sits on the Sun Moon midpoint, Mars squares it
  const Chart c = synthetic({{1, 10.0}, {2, 50.0}, {4, 30.0}, {5, 120.0}});
  const MidpointResult r = scan_midpoints(c, {}, {});
  bool direct_found = false;
  bool square_found = false;
  for (const MidpointHit& h : r.hits) {
    if (h.t == 4 && h.u == 1 && h.w == 2 && h.nh == 1) {
      direct_found = true;
    }
    if (h.t == 5 && h.u == 1 && h.w == 2 && h.nh == 2) {
      square_found = true;
    }
  }
  CHECK(direct_found);
  CHECK(square_found);
  CHECK(r.direct >= 1);
  CHECK(r.square >= 1);
}

TEST_CASE("the antipode of a midpoint counts as direct too") {
  // Venus opposite the Sun Moon midpoint
  const Chart c = synthetic({{1, 10.0}, {2, 50.0}, {4, 210.0}});
  const MidpointResult r = scan_midpoints(c, {}, {});
  bool found = false;
  for (const MidpointHit& h : r.hits) {
    if (h.t == 4 && h.u == 1 && h.w == 2 && h.nh == 1) {
      found = true;
    }
  }
  CHECK(found);
}

TEST_CASE("the duplicate cube spans the passes") {
  // a contact that the direct pass already booked must not return in the
  // semi square family through its ninety degree overlap
  const Chart c = synthetic({{1, 10.0}, {2, 50.0}, {5, 120.0}});
  const MidpointResult r = scan_midpoints(c, {}, {});
  int count = 0;
  for (const MidpointHit& h : r.hits) {
    if (h.t == 5 && h.u == 1 && h.w == 2) {
      ++count;
    }
  }
  CHECK(count == 1);
}

TEST_CASE("the comparison scan lists the running sky over the radix") {
  const Chart radix = synthetic({{body::kVenus, 231.95}});
  // the running sun close by, a square mars, and a quincunx jupiter
  // that the comparison never scans
  const Chart running = synthetic({{body::kSun, 232.10}, {body::kMars, 322.35}, {body::kJupiter, 21.95}});
  const std::vector<CrossAspectHit> hits = scan_aspects_between(radix, running, {}, true);
  REQUIRE(hits.size() == 2);
  CHECK(hits[0].t == body::kVenus);
  CHECK(hits[0].w == body::kSun);
  CHECK(hits[0].n == 1);
  CHECK(hits[0].sep_deg == doctest::Approx(0.15).epsilon(0.01));
  CHECK(hits[1].w == body::kMars);
  CHECK(hits[1].n == 4);
  CHECK(hits[1].sep_deg == doctest::Approx(90.4).epsilon(0.001));
}
