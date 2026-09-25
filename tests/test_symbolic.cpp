// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <algorithm>
#include <cmath>

#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

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

// the hit points into the list, a temporary list would leave it dangling
const DirectionHit* find_hit(std::vector<DirectionHit>&& hits, int t, int u, int w) = delete;

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
  // the intermediate cusps direct in their own pass
  DirectionRange cusps = range;
  cusps.house_targets = true;
  const std::vector<DirectionHit> with = direction_hits(c, DirectionMethod::kSymbolicEcliptic, cusps, 48.0);
  bool cusp_seen = false;
  for (const DirectionHit& h : with) {
    cusp_seen = cusp_seen || h.directed_cusp > 0;
    // the symbolic frames never take a cusp as the target
    CHECK(h.target_cusp == 0);
  }
  CHECK(cusp_seen);
  // the primary takes them on both sides
  bool promissor_cusp = false;
  for (const DirectionHit& h : direction_hits(c, DirectionMethod::kPrimary, cusps, 48.0)) {
    promissor_cusp = promissor_cusp || h.target_cusp > 0;
  }
  CHECK(promissor_cusp);
}

TEST_CASE("the Kühr primary drops the cardinal points and reads Placidus cusps") {
  Chart c = synthetic();
  c.ekls0 = 23.44 * kDegToRad;
  DirectionRange range;
  range.to_years = 180.0;
  range.base_angle_deg = 90.0;
  const auto extra_hit = [](const std::vector<DirectionHit>& hits) {
    for (const DirectionHit& h : hits) {
      if ((h.target >= 15 && h.target <= 18) || (h.directed >= 15 && h.directed <= 18)) {
        return true;
      }
    }
    return false;
  };
  // kard! = 0 in aprim, the symbolic ecliptic direction keeps them
  DirectionRange cardinal = range;
  cardinal.cardinal_targets = true;
  CHECK(extra_hit(direction_hits(c, DirectionMethod::kSymbolicEcliptic, cardinal, 48.0)));
  CHECK_FALSE(extra_hit(direction_hits(c, DirectionMethod::kPrimary, cardinal, 48.0)));
  // whatever cusps the chart carries, aprim and ar_sys direct Placidus
  // ones, the ecliptic direction follows the chart
  DirectionRange cusps = range;
  cusps.house_targets = true;
  Chart other = c;
  for (int k = 1; k <= 12; ++k) {
    other.houses.cusp[static_cast<std::size_t>(k)] += 3.0 * kDegToRad;
  }
  const auto ages = [](const std::vector<DirectionHit>& hits) {
    std::vector<double> out;
    for (const DirectionHit& h : hits) {
      out.push_back(h.years);
    }
    return out;
  };
  for (const DirectionMethod m : {DirectionMethod::kPrimary, DirectionMethod::kSymbolicEquatorial}) {
    CHECK(ages(direction_hits(c, m, cusps, 48.0)) == ages(direction_hits(other, m, cusps, 48.0)));
  }
  CHECK(ages(direction_hits(c, DirectionMethod::kSymbolicEcliptic, cusps, 48.0)) !=
        ages(direction_hits(other, DirectionMethod::kSymbolicEcliptic, cusps, 48.0)));
}

TEST_CASE("the direction window holds the years under every key") {
  const Chart c = synthetic();
  DirectionRange range;
  range.base_angle_deg = 90.0;
  //RR NAIBOD, the ten degree arc becomes ten point one five years
  range.key = 1.0146;
  range.to_years = 10.0;
  // his window held the arc of ten degrees and listed it at 10.15 years
  const auto short_window = direction_hits(c, DirectionMethod::kSymbolicEcliptic, range, 48.0);
  CHECK(find_hit(short_window, body::kSun, body::kMoon, 3) == nullptr);
  range.to_years = 10.2;
  const auto hits = direction_hits(c, DirectionMethod::kSymbolicEcliptic, range, 48.0);
  const DirectionHit* h = find_hit(hits, body::kSun, body::kMoon, 3);
  REQUIRE(h != nullptr);
  CHECK(h->years == doctest::Approx(10.146));
}

TEST_CASE("grid, first body and chosen significators steer the walk") {
  const Chart c = synthetic();
  DirectionRange range;
  range.to_years = 180.0;
  range.base_angle_deg = 30.0;
  range.grid = AspectGrid::k60And90;
  for (const DirectionHit& h : direction_hits(c, DirectionMethod::kSymbolicEcliptic, range, 48.0)) {
    const int deg = h.multiple * 30;
    CHECK((deg % 60 == 0 || deg % 90 == 0));
  }
  DirectionRange from_moon = range;
  from_moon.grid = AspectGrid::kPlain;
  from_moon.first_slot = body::kMoon;
  for (const DirectionHit& h : direction_hits(c, DirectionMethod::kSymbolicEcliptic, from_moon, 48.0)) {
    CHECK(h.directed != body::kSun);
    CHECK(h.target != body::kSun);
  }
  DirectionRange mars_only = range;
  mars_only.grid = AspectGrid::kPlain;
  mars_only.chosen = {body::kMars};
  const std::vector<DirectionHit> hits = direction_hits(c, DirectionMethod::kSymbolicEcliptic, mars_only, 48.0);
  REQUIRE(!hits.empty());
  for (const DirectionHit& h : hits) {
    CHECK(h.directed == body::kMars);
  }
}

TEST_CASE("the ecliptic direction reaches the midpoints of halbs_dir") {
  const Chart c = synthetic();
  DirectionRange range;
  range.to_years = 180.0;
  range.base_angle_deg = 90.0;
  range.midpoints = 1;
  // Mars at 200 reaches the sun moon midpoint at 55 after 145 degrees
  bool mars_mid = false;
  for (const DirectionHit& h : direction_hits(c, DirectionMethod::kSymbolicEcliptic, range, 48.0)) {
    if (h.directed == body::kMars && h.target == body::kSun && h.target2 == body::kMoon && h.multiple == 0) {
      mars_mid = true;
      CHECK(h.arc_deg == doctest::Approx(145.0));
    }
  }
  CHECK(mars_mid);
  // SO/MO and SO/MA share the factor sun, both list where his drk! kept one
  int with_sun = 0;
  for (const DirectionHit& h : direction_hits(c, DirectionMethod::kSymbolicEcliptic, range, 48.0)) {
    if (h.directed == body::kMars && h.target == body::kSun && h.target2 > 0 && h.multiple == 0) {
      ++with_sun;
    }
  }
  CHECK(with_sun >= 2);
  // NUR DIE MIT 3 UNTERSCHIEDLICHEN Faktoren drops the sun's own midpoints
  range.midpoints = 2;
  for (const DirectionHit& h : direction_hits(c, DirectionMethod::kSymbolicEcliptic, range, 48.0)) {
    if (h.target2 > 0) {
      CHECK(h.directed != h.target);
      CHECK(h.directed != h.target2);
    }
  }
  // the equatorial frame knows no midpoints
  range.midpoints = 1;
  for (const DirectionHit& h : direction_hits(c, DirectionMethod::kSymbolicEquatorial, range, 48.0)) {
    CHECK(h.target2 == 0);
  }
}

TEST_CASE("the primary honours SIGNIFIKAT. OHNE Breite") {
  Chart c = synthetic();
  c.ekls0 = 23.44 * kDegToRad;
  DirectionRange range;
  range.to_years = 180.0;
  range.base_angle_deg = 90.0;
  const std::vector<DirectionHit> with = direction_hits(c, DirectionMethod::kPrimary, range, 48.0);
  range.significator_latitude = false;
  const std::vector<DirectionHit> without = direction_hits(c, DirectionMethod::kPrimary, range, 48.0);
  REQUIRE(!with.empty());
  REQUIRE(!without.empty());
  // every body carries one degree of latitude, the arcs move
  bool moved = false;
  for (const DirectionHit& a : with) {
    for (const DirectionHit& b : without) {
      if (a.directed == b.directed && a.target == b.target && a.multiple == b.multiple && a.converse == b.converse) {
        moved = moved || std::abs(a.arc_deg - b.arc_deg) > 1e-6;
      }
    }
  }
  CHECK(moved);
}

namespace {

// right ascension and declination of an ecliptic point by the textbook
// formulas of Meeus, chapter 13, degrees in and out
double meeus_ra(double lam_deg, double beta_deg, double eps_deg) {
  const double l = lam_deg * kDegToRad;
  const double b = beta_deg * kDegToRad;
  const double e = eps_deg * kDegToRad;
  return norm_rad(std::atan2(std::sin(l) * std::cos(e) - std::tan(b) * std::sin(e), std::cos(l))) * kRadToDeg;
}

double meeus_dec(double lam_deg, double beta_deg, double eps_deg) {
  const double l = lam_deg * kDegToRad;
  const double b = beta_deg * kDegToRad;
  const double e = eps_deg * kDegToRad;
  return std::asin(std::sin(b) * std::cos(e) + std::cos(b) * std::sin(e) * std::sin(l)) * kRadToDeg;
}

}  // namespace

TEST_CASE("the AR system measures the arc in right ascension like Meeus 13.3") {
  // Sun at 15 and Moon at 95 degrees, both one degree north, obliquity
  // 23.44. By hand the right ascensions are 13.4225 and 95.4886 degrees,
  // the symbolic arc between them 82.0661 degrees
  const Chart c = synthetic();
  DirectionRange range;
  range.to_years = 180.0;
  range.base_angle_deg = 90.0;
  const auto hits = direction_hits(c, DirectionMethod::kSymbolicEquatorial, range, 48.0);
  const DirectionHit* h = find_hit(hits, body::kSun, body::kMoon, 0);
  REQUIRE(h != nullptr);
  const double expected = meeus_ra(95.0, 1.0, 23.44) - meeus_ra(15.0, 1.0, 23.44);
  CHECK(std::abs(h->arc_deg - expected) < 1.0e-6);
  CHECK(std::abs(h->arc_deg - 82.0661) < 1.0e-4);
  CHECK(std::abs(h->years - expected) < 1.0e-6);
}

TEST_CASE("the Kühr primary directs under the significator's Placidus pole") {
  // The Moon at 95 degrees without latitude stands 4.5529 degrees west of
  // the meridian at ARMC 100 and latitude 48. The textbook semi arc
  // method gives its declination 23.3455, ascensional difference 28.6430,
  // the proportional part 4.5529 * 28.6430 / 118.6430 = 1.0992 degrees,
  // the pole atan(sin 1.0992 / tan 23.3455) = 2.5448 and the oblique
  // descension 95.4471 + 1.0992 = 96.5463. The Sun at 15 degrees under
  // that pole descends at 13.8115 + 0.2636 = 14.0751, so the converse arc
  // of the conjunction is 82.4712 degrees
  Chart c = synthetic();
  c.ekls0 = 23.44 * kDegToRad;
  DirectionRange range;
  range.to_years = 180.0;
  range.base_angle_deg = 90.0;
  range.significator_latitude = false;
  range.with_latitude = false;
  const auto hits = direction_hits(c, DirectionMethod::kPrimary, range, 48.0);
  const DirectionHit* h = find_hit(hits, body::kMoon, body::kSun, 0);
  REQUIRE(h != nullptr);
  const double phi = 48.0 * kDegToRad;
  const double ra_moon = meeus_ra(95.0, 0.0, 23.44) * kDegToRad;
  const double dec_moon = meeus_dec(95.0, 0.0, 23.44) * kDegToRad;
  const double asc_diff = std::asin(std::tan(phi) * std::tan(dec_moon));
  const double meridian_distance = 100.0 * kDegToRad - ra_moon;
  const double part = meridian_distance * asc_diff / (kHalfPi + asc_diff);
  const double pole = std::atan(std::sin(part) / std::tan(dec_moon));
  const double descension_moon = ra_moon + part;
  const double dec_sun = meeus_dec(15.0, 0.0, 23.44) * kDegToRad;
  const double descension_sun = meeus_ra(15.0, 0.0, 23.44) * kDegToRad + std::asin(std::tan(pole) * std::tan(dec_sun));
  const double expected = std::abs(descension_sun - descension_moon) * kRadToDeg;
  CHECK(std::abs(h->arc_deg - expected) < 1.0e-6);
  CHECK(std::abs(h->arc_deg - 82.4712) < 1.0e-4);
  CHECK(h->converse);
}

TEST_CASE("the ecliptic midpoints follow the chosen first factor like asy112") {
  // his asy112 asks auswahl_flag(u&) for the first factor of a midpoint,
  // the directed Sun reaches no MO/MA midpoint when only Sun and Mars are
  // chosen, directed Mars still reaches SO/MO
  const Chart c = synthetic();
  DirectionRange range;
  range.to_years = 180.0;
  range.base_angle_deg = 90.0;
  range.midpoints = 1;
  range.chosen = {body::kSun, body::kMars};
  bool mars_sun_moon = false;
  for (const DirectionHit& h : direction_hits(c, DirectionMethod::kSymbolicEcliptic, range, 48.0)) {
    if (h.target2 > 0) {
      CHECK(h.target != body::kMoon);
    }
    mars_sun_moon = mars_sun_moon || (h.directed == body::kMars && h.target == body::kSun && h.target2 == body::kMoon);
  }
  CHECK(mars_sun_moon);
}

TEST_CASE("the rightward walk names the ruler intercepted across Aries") {
  // house one runs from 350 degrees over 0 Aries to 65, Aries and Taurus
  // lie inside it, his P3 row names the ruler of Taurus
  Chart c;
  c.ok = true;
  for (const auto& [slot, deg] : std::initializer_list<std::pair<int, double>>{
           {body::kSun, 200.0}, {body::kVenus, 100.0}, {body::kAscendant, 350.0}, {body::kMc, 260.0}}) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  }
  c.houses.ok = true;
  const double cusps[12] = {350.0, 65.0, 90.0, 120.0, 150.0, 180.0, 170.0 + 30.0, 245.0, 260.0, 290.0, 310.0, 330.0};
  for (int k = 1; k <= 12; ++k) {
    c.houses.cusp[static_cast<std::size_t>(k)] = cusps[k - 1] * kDegToRad;
  }
  c.houses.cusp[13] = c.houses.cusp[1];
  AspectSettings as;
  as.divisors = 4;
  const AspectResult scan = scan_aspects(c, ChartSettings{}, as);
  RhythmOptions opt;
  opt.leftward = false;
  bool taurus = false;
  for (const RhythmTrigger& t : rhythm_triggers(c, scan, as, opt)) {
    if (t.kind == RhythmKind::kRuler3 && t.slot == body::kVenus) {
      taurus = true;
    }
  }
  CHECK(taurus);
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

TEST_CASE("the degree date list walks every half degree onto its age") {
  Chart c;
  c.ok = true;
  c.houses.ok = true;
  // equal houses of thirty degrees from five degrees Aries, his w1 > 0
  // test skips a house whose cusp stands exactly on zero
  for (int i = 1; i <= 13; ++i) {
    c.houses.cusp[static_cast<std::size_t>(i)] = norm_rad((5.0 + (i - 1) * 30.0) * kDegToRad);
  }
  RhythmOptions opt;
  opt.phase_years = 7.0;
  const auto rows = degree_dates(c, opt, {}, false, 48.0);
  REQUIRE(rows.size() == 720);
  // twenty degrees lies mid house one, age three and a half
  CHECK(rows[40].degree == doctest::Approx(20.0));
  CHECK(rows[40].house == 1);
  CHECK(rows[40].value == doctest::Approx(3.5).epsilon(0.01));
  // the published group destiny degree 4.5 carries its pair
  CHECK(rows[9].p == 5);
  CHECK(rows[9].q == 9);
  // rightward the same degree ages from the other end
  opt.leftward = false;
  const auto back = degree_dates(c, opt, {}, false, 48.0);
  CHECK(back[40].value == doctest::Approx(12.0 * 7.0 - 3.5).epsilon(0.01));
  // a cusp on zero Aries leaves its house empty like his guard
  Chart zero = c;
  for (int i = 1; i <= 13; ++i) {
    zero.houses.cusp[static_cast<std::size_t>(i)] = norm_rad((i - 1) * 30.0 * kDegToRad);
  }
  CHECK(degree_dates(zero, opt, {}, false, 48.0)[30].house == 0);
}

TEST_CASE("self defined degrees round trip and mirror") {
  const auto dir = std::filesystem::temp_directory_path() / "grade.int";
  std::vector<CustomDegree> own = {{13.0, 5, 7}};
  REQUIRE(write_degrees(dir, own));
  const auto in = read_degrees(dir);
  REQUIRE(in.size() == 1);
  CHECK(in[0].degree == doctest::Approx(13.0));
  CHECK(in[0].p == 5);
  Chart c;
  c.ok = true;
  c.houses.ok = true;
  for (int i = 1; i <= 13; ++i) {
    c.houses.cusp[static_cast<std::size_t>(i)] = norm_rad((i - 1) * 30.0 * kDegToRad);
  }
  const auto rows = degree_dates(c, {}, in, false, 48.0);
  CHECK(rows[26].custom);
  CHECK(rows[26].p == 5);
  //RR der Spiegelpunkt an 0 Widder-Waage
  CHECK(rows[720 - 26].custom);
  CHECK(rows[720 - 26].mirror);
  // his gs& test knows the published and the own degrees alike
  CHECK(degree_known(26, in));
  CHECK(degree_known(35, {}));
  CHECK_FALSE(degree_known(27, in));
  // his SELECT m& names the published pair first, an own mirror landing
  // on 17.5 degrees leaves Uranus and Mars standing
  const auto shadowed = degree_dates(c, {}, {{342.5, 5, 7}}, false, 48.0);
  CHECK(shadowed[35].p == 8);
  CHECK(shadowed[35].q == 5);
  CHECK_FALSE(shadowed[35].custom);
  std::filesystem::remove(dir);
}

TEST_CASE("the degree file reads his quoted WRITE rows") {
  // WRITE #30,gr$,p1$,p2$ quoted every field, the rewrite once read none
  const auto file = std::filesystem::temp_directory_path() / "grade_quoted.int";
  {
    std::ofstream out(file, std::ios::binary);
    out << "\"125.0\",\" 1\",\"10\"\r\n\" 12.5\",\" 5\",\" 7\"\r\n";
  }
  const auto rows = read_degrees(file);
  REQUIRE(rows.size() == 2);
  CHECK(rows[0].degree == doctest::Approx(125.0));
  CHECK(rows[0].p == 1);
  CHECK(rows[0].q == 10);
  CHECK(rows[1].degree == doctest::Approx(12.5));
  // the writer answers in the same form
  REQUIRE(write_degrees(file, rows));
  std::ifstream in(file, std::ios::binary);
  std::string first;
  std::getline(in, first);
  CHECK(first == "\"125.0\",\" 1\",\"10\"\r");
  in.close();
  std::filesystem::remove(file);
}

TEST_CASE("the Sonderpunkt rides the walk and the age inverse lands back") {
  Chart c;
  c.ok = true;
  c.houses.ok = true;
  // houses off the zero point, his open windows never trigger at 0 Aries
  for (int i = 1; i <= 13; ++i) {
    c.houses.cusp[static_cast<std::size_t>(i)] = norm_rad((5.0 + (i - 1) * 30.0) * kDegToRad);
  }
  RhythmOptions opt;
  opt.phase_years = 7.0;
  //RR SONDERPUNKT als EKLIPTIK-GRAD
  opt.special = 20.0 * kDegToRad;
  const auto rows = rhythm_triggers(c, {}, {}, opt);
  bool found = false;
  for (const RhythmTrigger& t : rows) {
    if (t.slot == 0 && t.kind == RhythmKind::kDirect) {
      found = true;
      CHECK(t.value == doctest::Approx(3.5).epsilon(0.01));
    }
  }
  CHECK(found);
  // the date defined point inverts the walk, mid house one is age 3.5
  CHECK(degree_at_age(c, opt, 3.5) == doctest::Approx(20.0 * kDegToRad).epsilon(0.001));
  CHECK(degree_at_age(c, opt, 10.5) == doctest::Approx(50.0 * kDegToRad).epsilon(0.001));
  opt.leftward = false;
  CHECK(degree_at_age(c, opt, 3.5) == doctest::Approx(norm_rad(350.0 * kDegToRad)).epsilon(0.001));
  // the month unit reads the phase length in months, 42 months are six
  // phases of seven months and reach the start of the seventh house
  RhythmOptions monthly = opt;
  monthly.leftward = true;
  monthly.months = true;
  CHECK(degree_at_age(c, monthly, 3.5) == doctest::Approx(c.houses.cusp[7]).epsilon(0.001));
  // no degree before birth or beyond the twelve phases
  CHECK(degree_at_age(c, monthly, -1.0) < 0.0);
  RhythmOptions yearly = opt;
  yearly.leftward = true;
  CHECK(degree_at_age(c, yearly, 12.0 * 7.0 + 1.0) < 0.0);
}

namespace {

// equal houses from five degrees Aries, bodies at chosen degrees
Chart rhythm_chart(std::initializer_list<std::pair<int, double>> bodies) {
  Chart c;
  c.ok = true;
  c.houses.ok = true;
  for (int i = 1; i <= 13; ++i) {
    c.houses.cusp[static_cast<std::size_t>(i)] = norm_rad((5.0 + (i - 1) * 30.0) * kDegToRad);
  }
  for (const auto& [slot, deg] : bodies) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  }
  return c;
}

bool has_trigger(const std::vector<RhythmTrigger>& rows, int slot, RhythmKind kind, int source = -1) {
  return std::any_of(rows.begin(), rows.end(), [&](const RhythmTrigger& t) {
    return t.slot == slot && t.kind == kind && (source < 0 || t.source == source);
  });
}

}  // namespace

TEST_CASE("the rhythm walk triggers AC, MC and the south node directly") {
  // his table ran FOR pl& = aa& TO np& with np& = 12 without extras and
  // so never named AC and MC, the graph ran to 18 and did. The walk knows
  // one list for both
  const Chart c = rhythm_chart({{body::kSun, 20.0}, {body::kNodeAsc, 100.0}, {body::kNodeDesc, 280.0}});
  const std::vector<RhythmTrigger> rows = rhythm_triggers(c, {}, {}, {});
  CHECK(has_trigger(rows, body::kAscendant, RhythmKind::kDirect));
  CHECK(has_trigger(rows, body::kMc, RhythmKind::kDirect));
  //RR FOR pl& = aa& TO npm&, the south node stands in its house as well
  CHECK(has_trigger(rows, body::kNodeDesc, RhythmKind::kDirect));
  // AC opens the first house of a leftward walk at age zero
  for (const RhythmTrigger& t : rows) {
    if (t.slot == body::kAscendant) {
      CHECK(t.phase == 1);
      CHECK(t.body_house == 1);
      CHECK(t.value == doctest::Approx(0.0).epsilon(1e-6));
    }
  }
}

TEST_CASE("the cardinal points trigger only with kard") {
  const Chart c = rhythm_chart({{body::kSun, 20.0}});
  RhythmOptions opt;
  const auto without = rhythm_triggers(c, {}, {}, opt);
  CHECK_FALSE(has_trigger(without, body::kAriesPoint, RhythmKind::kDirect));
  opt.cardinals = true;
  const auto with = rhythm_triggers(c, {}, {}, opt);
  // 0 Aries lies 25 degrees into house twelve, 0 Cancer in house three
  bool aries = false;
  for (const RhythmTrigger& t : with) {
    if (t.slot == body::kAriesPoint) {
      aries = true;
      CHECK(t.house == 12);
      CHECK(t.value == doctest::Approx(11.0 * 7.0 + 7.0 * 25.0 / 30.0).epsilon(1e-6));
    }
  }
  CHECK(aries);
  CHECK(has_trigger(with, body::kAriesPoint + 1, RhythmKind::kDirect));
}

TEST_CASE("the Sonderpunkt chains its aspects and the mundane walk drops the mirrors") {
  // the point at 110 squares the Sun at 20, lpkt sets aa& = 0 so asp1
  // scans it like a body
  Chart c = rhythm_chart({{body::kSun, 20.0}, {body::kMars, 340.0}});
  c.b[body::kFixpunkt].present = true;
  c.b[body::kFixpunkt].valid = true;
  c.b[body::kFixpunkt].el = 110.0 * kDegToRad;
  AspectSettings as;
  as.divisors = 4;
  const AspectResult scan = scan_aspects(c, ChartSettings{}, as);
  RhythmOptions opt;
  opt.special = 110.0 * kDegToRad;
  const auto rows = rhythm_triggers(c, scan, as, opt);
  CHECK(has_trigger(rows, body::kSun, RhythmKind::kAspect, body::kFixpunkt));
  CHECK(has_trigger(rows, body::kFixpunkt, RhythmKind::kAspect, body::kSun));
  // Sun at 20 and Mars at 340 mirror across 0 Aries
  CHECK(has_trigger(rows, body::kMars, RhythmKind::kMirror, body::kSun));
  //RR a173 runs only with horm& = 1
  opt.mundane = true;
  const auto mundane = rhythm_triggers(c, scan, as, opt);
  CHECK_FALSE(std::any_of(mundane.begin(), mundane.end(), [](const RhythmTrigger& t) { return t.kind == RhythmKind::kMirror; }));
}

TEST_CASE("the Septar offset counts the unit") {
  RhythmOptions years;
  years.phase_years = 1.0;
  // the third Septar of one year per house begins at 24, the original
  // (sen - 1) * vp gave 2
  CHECK(septar_offset(3, years) == doctest::Approx(24.0));
  RhythmOptions months;
  months.phase_years = 7.0;
  months.months = true;
  // seven months per house make a Septar of seven years
  CHECK(septar_offset(3, months) == doctest::Approx(14.0));
  CHECK(septar_offset(1, months) == doctest::Approx(0.0));
}

TEST_CASE("the rhythm clock dates an age and turns a date back into it") {
  // a Solar of the age 30, his lpk = (lpkt - sn) * fm& / vp took the
  // age once more and landed 30 years early
  RhythmClock solar;
  solar.base_jd = 2460000.5;
  solar.sn = 30.0;
  solar.tja = 365.2422;
  const double jd = rhythm_jd(solar, 10.5);
  // the default relative Approx of a julian day of this size lets a whole
  // month through, the checks hold a millisecond
  CHECK(std::abs(jd - (2460000.5 + 10.5 * 365.2422)) < 1.0e-8);
  CHECK(std::abs(rhythm_years(solar, jd) - 10.5) < 1.0e-9);
  CHECK(std::abs(rhythm_years(solar, jd) - solar.sn - 10.5) > 1.0);
}

TEST_CASE("the LJ and MO columns read a signed age with a carry") {
  // minus two years three months, his a175 printed -3 -3.0
  RhythmAge a = rhythm_age(-2.25, 0.0, false);
  CHECK(a.negative);
  CHECK(a.years == 2);
  CHECK(a.months == doctest::Approx(3.0));
  // a178 carried a rounded twelfth month toward zero, -2.99 read -1/ 0
  a = rhythm_age(-2.99, 0.0, true);
  CHECK(a.negative);
  CHECK(a.years == 3);
  CHECK(a.months == doctest::Approx(0.0));
  // his a175 never carried, 2.999 read 2 12.0
  a = rhythm_age(2.999, 0.0, false);
  CHECK_FALSE(a.negative);
  CHECK(a.years == 3);
  CHECK(a.months == doctest::Approx(0.0));
  // the Solar age joins before the split, 35 minus 2.25 is 32 and nine
  // months, his INT(sn + lj) with FRAC(lj) read 32 -3.0
  a = rhythm_age(-2.25, 35.0, false);
  CHECK_FALSE(a.negative);
  CHECK(a.years == 32);
  CHECK(a.months == doctest::Approx(9.0));
  // tenths in the table, whole months in the graph
  CHECK(rhythm_age(1.2, 0.0, false).months == doctest::Approx(2.4));
  CHECK(rhythm_age(1.2, 0.0, true).months == doctest::Approx(2.0));
}
