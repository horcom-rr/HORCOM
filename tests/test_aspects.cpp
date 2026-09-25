// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/aspects.hpp"
#include "horcom/chart/bodies.hpp"
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

TEST_CASE("the extras in high fixed slots take part in the scans") {
  // his profile extras, Black Moon, Chiron, Quaoar and Xena, sit in the
  // fixed slots 19, 20, 35 and 40 of the port
  ChartSettings s;
  s.extra_bodies = true;
  s.nk[1] = body::kApogee;
  s.nk[2] = body::kChiron;
  s.nk[17] = body::kQuaoar;
  s.nk[22] = body::kXena;
  CHECK(s.body_count() == body::kXena);
  const Chart c = synthetic({{1, 10.0}, {2, 200.0}, {body::kApogee, 50.0}, {body::kChiron, 100.5},
                             {body::kQuaoar, 130.5}, {body::kXena, 11.0}});
  const AspectResult r = scan_aspects(c, s, {});
  CHECK(cell(r, 1, body::kQuaoar) == doctest::Approx(kTwoPi / 3.0));
  CHECK(cell(r, 1, body::kXena) == doctest::Approx(kTwoPi));
  // the midpoint of Sun and Xena reaches Chiron on the 90 degree level
  // only when the gaps between the chosen extras are stepped over
  const MidpointResult m = scan_midpoints(c, s, {});
  bool xena = false;
  for (const auto& h : m.hits) {
    xena = xena || h.u == body::kXena || h.w == body::kXena || h.t == body::kXena;
  }
  CHECK(xena);
}

TEST_CASE("an absent body never forms a midpoint at zero Aries") {
  // the Sun and the Moon at 170 and 190 put their midpoint at zero Aries
  // opposite, an empty slot must not stand there as a factor
  ChartSettings s;
  s.extra_bodies = true;
  s.nk[2] = body::kChiron;
  const Chart c = synthetic({{1, 170.0}, {2, 190.0}, {body::kChiron, 45.0}});
  const MidpointResult m = scan_midpoints(c, s, {});
  for (const auto& h : m.hits) {
    CHECK(c.b[static_cast<std::size_t>(h.t)].present);
    CHECK(c.b[static_cast<std::size_t>(h.u)].present);
    CHECK(c.b[static_cast<std::size_t>(h.w)].present);
  }
}

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

TEST_CASE("equal probability orbs stop at divisor twelve like avh") {
  // the orbe row ends at twelve, a sixteen asked for in this mode used to
  // read past it, his avh folds it back to twelve
  AspectSettings twelve;
  twelve.equal_probability = true;
  twelve.preset_equal_orbs();
  twelve.divisors = 12;
  AspectSettings sixteen = twelve;
  sixteen.divisors = kMaxEqualOrbDivisor + 4;
  const Chart c = synthetic({{1, 10.0}, {2, 160.0}, {3, 55.0}, {4, 100.3}, {5, 212.0}});
  const AspectResult a = scan_aspects(c, {}, twelve);
  const AspectResult b = scan_aspects(c, {}, sixteen);
  CHECK(a.hits.size() == b.hits.size());
  for (std::size_t n = 0; n < a.zh.size(); ++n) {
    CAPTURE(n);
    CHECK(a.zh[n] == b.zh[n]);
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

TEST_CASE("aspdis names every matched angle for the aspektarium") {
  // the conjunction carries two pi in the asp matrix
  CHECK(aspect_symbol(kTwoPi, 12) == 1);
  CHECK(aspect_symbol(kPi, 12) == 2);
  CHECK(aspect_symbol(120.0 * kDegToRad, 12) == 3);
  CHECK(aspect_symbol(90.0 * kDegToRad, 12) == 4);
  CHECK(aspect_symbol(72.0 * kDegToRad, 12) == 5);
  CHECK(aspect_symbol(60.0 * kDegToRad, 12) == 6);
  // the named minor aspects of his CASE ladder
  CHECK(aspect_symbol(45.0 * kDegToRad, 12) == 8);
  CHECK(aspect_symbol(30.0 * kDegToRad, 12) == 12);
  CHECK(aspect_symbol(144.0 * kDegToRad, 12) == 17);
  CHECK(aspect_symbol(150.0 * kDegToRad, 12) == 18);
  CHECK(aspect_symbol(135.0 * kDegToRad, 12) == 19);
  // multiples fold back onto their divisor through the t loop
  CHECK(aspect_symbol(2.0 * 360.0 / 7.0 * kDegToRad, 12) == 7);
  CHECK(aspect_symbol(80.0 * kDegToRad, 12) == 9);
  // the far side folds first
  CHECK(aspect_symbol(240.0 * kDegToRad, 12) == 3);
  CHECK(aspect_symbol(22.5 * kDegToRad, 16) == 16);
  // his FIX truncation named these by a smaller divisor, 5/11 as the
  // ninth, 4/13 as the tenth, 5/14 and 4/15 as the eleventh
  CHECK(aspect_symbol(5.0 * kTwoPi / 11.0, 16) == 11);
  CHECK(aspect_symbol(4.0 * kTwoPi / 13.0, 16) == 13);
  CHECK(aspect_symbol(5.0 * kTwoPi / 14.0, 16) == 14);
  CHECK(aspect_symbol(4.0 * kTwoPi / 15.0, 16) == 15);
}

TEST_CASE("the house systems without angles and nodes drop their aspects") {
  // asp1 keeps the AC and MC aspects for NUR AC und MC and drops them for
  // the systems 9 and 10, system 10 drops the nodes as well
  const Chart c = synthetic({{body::kSun, 10.0},
                             {body::kMars, 100.5},
                             {body::kNodeAsc, 190.2},
                             {body::kNodeDesc, 10.2},
                             {body::kAscendant, 10.4},
                             {body::kMc, 280.3}});
  const auto touches = [](const AspectResult& r, int lo, int hi) {
    for (int t = 0; t < body::kSlotCount; ++t) {
      for (int w = 0; w < body::kSlotCount; ++w) {
        if (cell(r, t, w) != 0.0 && ((t >= lo && t <= hi) || (w >= lo && w <= hi))) {
          return true;
        }
      }
    }
    return false;
  };
  const auto scan = [&c](HouseSystem h) {
    ChartSettings s;
    s.houses = h;
    return scan_aspects(c, s, AspectSettings{});
  };
  CHECK(touches(scan(HouseSystem::kAcMcOnly), 13, 14));
  CHECK_FALSE(touches(scan(HouseSystem::kNone), 13, 14));
  CHECK(touches(scan(HouseSystem::kNone), 11, 12));
  const AspectResult ten = scan(HouseSystem::kNoneNoNodes);
  CHECK_FALSE(touches(ten, 13, 14));
  CHECK_FALSE(touches(ten, 11, 12));
  CHECK(touches(ten, 1, 5));
}

TEST_CASE("the fixed point on slot zero joins the aspect scan") {
  // lpkt and fixpunkt_def set aa& = 0, the scan of asp1 starts on the
  // fixed point, the port once began on the Sun and left it silent
  const Chart c = synthetic({{body::kFixpunkt, 100.0}, {body::kSun, 10.0}, {body::kMoon, 220.0}});
  const AspectResult r = scan_aspects(c, {}, {});
  CHECK(cell(r, body::kFixpunkt, body::kSun) == doctest::Approx(kPi / 2.0));
  CHECK(cell(r, body::kFixpunkt, body::kMoon) == doctest::Approx(kTwoPi / 3.0));
  // without the point the Sun stays the first row
  const Chart plain = synthetic({{body::kSun, 10.0}, {body::kMoon, 220.0}});
  CHECK(cell(scan_aspects(plain, {}, {}), body::kFixpunkt, body::kSun) == 0.0);
}

TEST_CASE("the trees of aspar2 carry the cusps and the 45 degree level") {
  // Venus on the Sun Moon midpoint, Mars on its 22.5 degree family, the
  // second cusp on the midpoint as well
  Chart c = synthetic({{1, 10.0}, {2, 50.0}, {4, 30.0}, {5, 52.5}});
  c.houses.ok = true;
  for (int i = 1; i <= 12; ++i) {
    c.houses.cusp[static_cast<std::size_t>(i)] = (i - 1) * 30.0 * kDegToRad;
  }
  c.houses.cusp[2] = 30.0 * kDegToRad;
  std::array<bool, body::kSlotCount> all{};
  all.fill(true);
  const std::vector<MidpointTree> trees = midpoint_trees(c, {}, {}, all);
  const auto tree_of = [&](int slot) -> const MidpointTree* {
    for (const MidpointTree& t : trees) {
      if (t.slot == slot) {
        return &t;
      }
    }
    return nullptr;
  };
  // the bodies in slot order, then the cusps two, three, five and six
  REQUIRE(tree_of(4) != nullptr);
  REQUIRE(tree_of(15) != nullptr);
  CHECK(tree_of(15)->cusp == 2);
  CHECK(tree_of(16)->cusp == 3);
  CHECK(tree_of(17)->cusp == 5);
  CHECK(tree_of(18)->cusp == 6);
  CHECK(tree_of(15)->lon == doctest::Approx(30.0 * kDegToRad));
  const auto has = [](const MidpointTree* t, int u, int w, int nh) {
    for (const MidpointHit& h : t->hits) {
      if (h.u == u && h.w == w && h.nh == nh) {
        return true;
      }
    }
    return false;
  };
  CHECK(has(tree_of(4), 1, 2, 1));
  CHECK(has(tree_of(15), 1, 2, 1));
  // 52.5 lies 22.5 past the midpoint at 30, the fourth level V
  CHECK(has(tree_of(5), 1, 2, 8));
  // the heliocentric chart keeps no cusp trees
  ChartSettings helio;
  helio.heliocentric = true;
  for (const MidpointTree& t : midpoint_trees(c, helio, {}, all)) {
    CHECK(t.cusp == 0);
  }
  // his asp_wahl drops a hidden partner but keeps its tree
  std::array<bool, body::kSlotCount> no_moon = all;
  no_moon[2] = false;
  const std::vector<MidpointTree> thin = midpoint_trees(c, {}, {}, no_moon);
  bool moon_tree = false;
  for (const MidpointTree& t : thin) {
    moon_tree = moon_tree || t.slot == 2;
    for (const MidpointHit& h : t.hits) {
      CHECK(h.u != 2);
      CHECK(h.w != 2);
    }
  }
  CHECK(moon_tree);
}

TEST_CASE("the cusp trees of aspar2 weigh a normal hundred percent") {
  // Sun and Moon put their midpoint at 30 degrees, the cusps two, three,
  // five and six all stand on it
  Chart c = synthetic({{1, 10.0}, {2, 50.0}});
  c.houses.ok = true;
  for (int i = 1; i <= 12; ++i) {
    c.houses.cusp[static_cast<std::size_t>(i)] = (i - 1) * 30.0 * kDegToRad;
  }
  for (const int h : {2, 3, 5, 6}) {
    c.houses.cusp[static_cast<std::size_t>(h)] = 30.0 * kDegToRad;
  }
  std::array<bool, body::kSlotCount> all{};
  all.fill(true);
  // his profile weighs or&(15), the ZUSATZ-PLANETEN row, and leaves 16
  // to 18 at zero, the dialog offers no row for them
  AspectSettings his;
  his.weight[15] = 40;
  his.weight[16] = 0;
  his.weight[17] = 0;
  his.weight[18] = 0;
  const AspectSettings tree = tree_orb_settings(his);
  for (int slot = 15; slot <= 18; ++slot) {
    CHECK(tree.weight[static_cast<std::size_t>(slot)] == 100);
  }
  CHECK(tree.weight[1] == his.weight[1]);
  // the original drew H3, H5 and H6 without a single branch and gave H2
  // the extra body weight, every cusp tree now carries the SO-MO midpoint
  for (const MidpointTree& t : midpoint_trees(c, {}, his, all)) {
    if (t.cusp == 0) {
      continue;
    }
    bool so_mo = false;
    for (const MidpointHit& h : t.hits) {
      so_mo = so_mo || (h.u == 1 && h.w == 2 && h.nh == 1);
    }
    CHECK_MESSAGE(so_mo, "H", t.cusp);
  }
}

TEST_CASE("a body at exactly zero Aries and an exact conjunction both count") {
  // his pl = 0 stood for an empty slot and his w3 > 0 dropped equal
  // places, the fixed point typed as 0 Aries never aspected
  Chart c = synthetic({{1, 0.0}, {2, 120.0}, {3, 120.0}});
  const AspectResult r = scan_aspects(c, {}, {});
  CHECK(cell(r, 1, 2) == doctest::Approx(kTwoPi / 3.0));
  CHECK(cell(r, 1, 3) == doctest::Approx(kTwoPi / 3.0));
  CHECK(cell(r, 2, 3) == doctest::Approx(kTwoPi));
  CHECK(r.zh[1] == 1);
  CHECK(r.zh[3] == 2);
  // the fixed point on slot zero at 0 Aries
  Chart f = synthetic({{body::kFixpunkt, 0.0}, {1, 90.5}});
  const AspectResult rf = scan_aspects(f, {}, {});
  CHECK(cell(rf, body::kFixpunkt, 1) == doctest::Approx(kTwoPi / 4.0));
}

TEST_CASE("divisor_orb is the dd of asp1 in both orb modes") {
  AspectSettings a;
  a.orb = 1.5;
  // orb * pn / 30, the trine owns four degrees at the neutral factor
  CHECK(divisor_orb(a, 3) * kRadToDeg == doctest::Approx(1.5 * 120.0 / 30.0));
  CHECK(divisor_orb(a, 16) * kRadToDeg == doctest::Approx(1.5 * 22.5 / 30.0));
  a.equal_probability = true;
  a.preset_equal_orbs();
  // orb * orbe(n), the preset is twelve degrees over the divisor
  CHECK(divisor_orb(a, 4) * kRadToDeg == doctest::Approx(1.5 * 3.0));
}

TEST_CASE("the comparison scan keeps exact aspects and bodies at 0 Aries") {
  // his a12asp demanded w > kk, wa1 > kk and wa2 > kk, so an exact aspect
  // and every body at exactly 0 Aries dropped out. A chart laid over
  // itself, the DOPPEL-KREIS of one record, lost every conjunction
  const Chart c = synthetic({{body::kSun, 0.0}, {body::kMoon, 90.0}, {body::kMars, 200.0}});
  CrossScanOptions opt;
  opt.extras = true;
  const std::vector<CrossAspectHit> hits = scan_aspects_between(c, c, {}, opt);
  const auto has = [&](int t, int w, int n) {
    for (const CrossAspectHit& h : hits) {
      if (h.t == t && h.w == w && h.n == n) {
        return true;
      }
    }
    return false;
  };
  // the original listed none of these
  CHECK(has(body::kSun, body::kSun, 1));
  CHECK(has(body::kMoon, body::kMoon, 1));
  CHECK(has(body::kMars, body::kMars, 1));
  // the exact square of the Sun at 0 Aries to the Moon
  CHECK(has(body::kSun, body::kMoon, 4));
  for (const CrossAspectHit& h : hits) {
    if (h.t == h.w) {
      CHECK(h.sep_deg == doctest::Approx(0.0));
    }
  }
}

TEST_CASE("the fixed point joins the midpoint scans like halbs11 and halbs111") {
  // t& and u& run from aa& = 0, the fixed point sits on the Sun Moon
  // midpoint and stands in a pair with Venus on the Mars midpoint too
  const Chart c = synthetic({{body::kFixpunkt, 30.0}, {1, 10.0}, {2, 50.0}, {4, 70.0}, {5, 50.0}});
  const MidpointResult r = scan_midpoints(c, {}, {});
  bool as_point = false;
  bool as_partner = false;
  for (const MidpointHit& h : r.hits) {
    as_point = as_point || (h.t == body::kFixpunkt && h.u == 1 && h.w == 2 && h.nh == 1);
    as_partner = as_partner || (h.t == 5 && h.u == body::kFixpunkt && h.w == 4 && h.nh == 1);
  }
  CHECK(as_point);
  CHECK(as_partner);
  // the trees pair it as well
  std::array<bool, body::kSlotCount> all{};
  all.fill(true);
  bool tree_partner = false;
  for (const MidpointTree& t : midpoint_trees(c, {}, {}, all)) {
    for (const MidpointHit& h : t.hits) {
      tree_partner = tree_partner || h.u == body::kFixpunkt;
    }
  }
  CHECK(tree_partner);
}
