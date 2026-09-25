// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <algorithm>
#include <cmath>

#include "doctest.h"
#include "horcom/chart/composite.hpp"
#include "horcom/chart/dynamogram.hpp"
#include "horcom/chart/progressions.hpp"
#include "horcom/chart/riseset.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/linear.hpp"
#include "horcom/time/sidereal.hpp"

using namespace horcom;

namespace {

VsopTables& vsop() {
  static VsopTables t = VsopTables::load(HORCOM_TEST_DATA_DIR "/planets.ndx", HORCOM_TEST_DATA_DIR "/planets.dat");
  return t;
}

Ephemerides& eph() {
  static Ephemerides e{HORCOM_TEST_DATA_DIR "/eph"};
  return e;
}

SearchContext context() {
  SearchContext ctx;
  ctx.base.lon_deg_east = 11.3244;
  ctx.base.lat_deg = 48.1742;
  ctx.vsop = &vsop();
  ctx.eph = &eph();
  return ctx;
}

// the residual of a found crossing in arc seconds
double residual_arcsec(const LongitudeCrossing& hit, int slot, double target, const SearchContext& ctx) {
  const BodyLongitude e = body_longitude(hit.jd_ut, slot, ctx);
  double d = std::abs(norm_rad(e.el) - norm_rad(target));
  if (d > kPi) {
    d = kTwoPi - d;
  }
  return d * kRadToDeg * 3600.0;
}

}  // namespace

TEST_CASE("the sun search lands on the 1993 spring equinox") {
  const SearchContext ctx = context();
  // start well after the equinox and walk back to the sun at 0 Aries
  const double start = julian_day({15, 4, 1993, 12, 0.0});
  const LongitudeCrossing hit = find_longitude_backward(start, body::kSun, 0.0, ctx);
  REQUIRE(hit.ok);
  CHECK(hit.jd_ut <= start);
  CHECK_FALSE(hit.retrograde);
  const CalendarDate d = calendar_date(hit.jd_ut);
  CHECK(d.year == 1993);
  CHECK(d.month == 3);
  CHECK(d.day == 20);
  // the almanac puts the moment at 14:41 UT, two minutes of band cover
  // the delta T table and the Newcomb sun
  CHECK(d.hour + d.minute / 60.0 == doctest::Approx(14.0 + 41.0 / 60.0).epsilon(0.0023));
  CHECK(residual_arcsec(hit, body::kSun, 0.0, ctx) < 0.5);
}

TEST_CASE("the moon returns to its radix place within one period") {
  const SearchContext ctx = context();
  const double radix_moon = body_longitude(julian_day({13, 10, 1992, 3, 0.0}), body::kMoon, ctx).el;
  const double start = julian_day({15, 11, 1992, 0, 0.0});
  const LongitudeCrossing hit = find_longitude_backward(start, body::kMoon, radix_moon, ctx);
  REQUIRE(hit.ok);
  CHECK(hit.jd_ut <= start);
  CHECK(start - hit.jd_ut < 28.0);
  CHECK(residual_arcsec(hit, body::kMoon, radix_moon, ctx) < 0.5);
}

TEST_CASE("the mercury search converges near its retrograde loops") {
  const SearchContext ctx = context();
  const double target = body_longitude(julian_day({13, 10, 1992, 3, 0.0}), body::kMercury, ctx).el;
  const double start = julian_day({1, 6, 1993, 0, 0.0});
  const LongitudeCrossing hit = find_longitude_backward(start, body::kMercury, target, ctx);
  REQUIRE(hit.ok);
  CHECK(hit.jd_ut <= start);
  CHECK(residual_arcsec(hit, body::kMercury, target, ctx) < 0.5);
}

TEST_CASE("the solar return comes home near the birthday") {
  const SearchContext ctx = context();
  const CalendarDate birth{13, 10, 1992, 3, 0.0};
  const double radix_sun = body_longitude(julian_day(birth), body::kSun, ctx).el;
  const LongitudeCrossing hit = solar_return(birth, radix_sun, 1993, ctx);
  REQUIRE(hit.ok);
  const CalendarDate d = calendar_date(hit.jd_ut);
  CHECK(d.year == 1993);
  CHECK(d.month == 10);
  CHECK(std::abs(d.day - 13) <= 1);
  CHECK(residual_arcsec(hit, body::kSun, radix_sun, ctx) < 0.5);
}

TEST_CASE("the lunar return precedes the asked moment") {
  const SearchContext ctx = context();
  const double radix_moon = body_longitude(julian_day({13, 10, 1992, 3, 0.0}), body::kMoon, ctx).el;
  const double before = julian_day({1, 1, 1993, 0, 0.0});
  const LongitudeCrossing hit = lunar_return(before, radix_moon, ctx);
  REQUIRE(hit.ok);
  CHECK(hit.jd_ut <= before);
  CHECK(before - hit.jd_ut < 28.0);
  CHECK(residual_arcsec(hit, body::kMoon, radix_moon, ctx) < 0.5);
}

TEST_CASE("the transit sweep finds the sun over radix venus") {
  const SearchContext ctx = context();
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  const Chart radix = compute_chart(in, ctx.settings, vsop(), eph());
  REQUIRE(radix.ok);
  TransitScan scan;
  scan.jd_from_ut = julian_day({1, 11, 1992, 0, 0.0});
  scan.jd_to_ut = julian_day({1, 12, 1992, 0, 0.0});
  const std::vector<TransitEvent> events = scan_transits(radix, scan, ctx);
  REQUIRE(!events.empty());
  bool sun_venus = false;
  double last = 0.0;
  for (const TransitEvent& e : events) {
    CHECK(e.jd_ut >= scan.jd_from_ut - 2.0);
    CHECK(e.jd_ut <= scan.jd_to_ut + kEps);
    CHECK(e.jd_ut >= last);
    last = e.jd_ut;
    if (!e.station_touch) {
      // every refined event sits on its target to under an arc second
      const double target = norm_rad(radix.b[static_cast<std::size_t>(e.radix)].el + e.multiple * 30.0 * kDegToRad);
      const double got = body_longitude(e.jd_ut, e.transiting, ctx).el;
      double d = std::abs(got - target);
      if (d > kPi) {
        d = kTwoPi - d;
      }
      CHECK(d * kRadToDeg * 3600.0 < 1.0);
    }
    if (e.transiting == body::kSun && e.radix == body::kVenus && e.multiple == 0) {
      sun_venus = true;
      const CalendarDate d = calendar_date(e.jd_ut);
      CHECK(d.month == 11);
      CHECK(std::abs(d.day - 14) <= 1);
    }
  }
  CHECK(sun_venus);
}

TEST_CASE("the sun ingress table lands every sign in its year") {
  const SearchContext ctx = context();
  const auto table = sign_ingresses(julian_day({31, 12, 1993, 12, 0.0}), body::kSun, ctx);
  for (int t = 0; t < 12; ++t) {
    REQUIRE(table[static_cast<std::size_t>(t)].ok);
    const CalendarDate d = calendar_date(table[static_cast<std::size_t>(t)].jd_ut);
    CHECK(d.year == 1993);
    const double target = kEps + t * kPi / 6.0;
    CHECK(residual_arcsec(table[static_cast<std::size_t>(t)], body::kSun, target, ctx) < 0.5);
  }
  // Aries is the equinox of the earlier pin
  const CalendarDate aries = calendar_date(table[0].jd_ut);
  CHECK(aries.month == 3);
  CHECK(aries.day == 20);
}

TEST_CASE("an extra body outside his ephemeris rides the element fallback") {
  SearchContext ctx = context();
  ctx.settings.enable_standard_extras();
  // far outside the integrated span the pipeline follows the orbital
  // elements, the search converges on that path too
  const LongitudeCrossing hit = find_longitude_backward(julian_day({1, 1, 900, 0, 0.0}), body::kChiron, 0.0, ctx);
  REQUIRE(hit.ok);
  CHECK(residual_arcsec(hit, body::kChiron, 0.0, ctx) < 0.5);
}

TEST_CASE("the planetar and the personar land on their radix targets") {
  const SearchContext ctx = context();
  const double birth = julian_day({13, 10, 1992, 3, 0.0});
  const double radix_mars = body_longitude(birth, body::kMars, ctx).el;
  // the first mars return comes after one mars period of near 687 days
  const LongitudeCrossing ret = planetar_return(birth, body::kMars, radix_mars, 1, true, ctx);
  REQUIRE(ret.ok);
  CHECK(residual_arcsec(ret, body::kMars, radix_mars, ctx) < 5.0);
  CHECK(ret.jd_ut - birth > 500.0);
  CHECK(ret.jd_ut - birth < 900.0);
  // the personar is the sun reaching the radix body within the first year
  const double tja = 365.2422;
  const LongitudeCrossing pers = find_longitude_backward(birth + tja, body::kSun, radix_mars, ctx);
  REQUIRE(pers.ok);
  CHECK(residual_arcsec(pers, body::kSun, radix_mars, ctx) < 5.0);
  CHECK(pers.jd_ut > birth - 40.0);
  CHECK(pers.jd_ut < birth + tja);
}

// the tester's v5 lunar bug, +1 must land on the first lunation after
// birth (about 27 days later) and -1 on the first lunation before birth
// (about 27 days earlier). The old seed formula treated the moon like
// an outer body and drifted by whole years
TEST_CASE("the lunar planetar counts signed months from birth") {
  const SearchContext ctx = context();
  const double birth = julian_day({13, 10, 1992, 3, 0.0});
  const double radix_moon = body_longitude(birth, body::kMoon, ctx).el;
  const double lunation = 27.321582;

  const LongitudeCrossing next = planetar_return(birth, body::kMoon, radix_moon, 1, true, ctx);
  REQUIRE(next.ok);
  CHECK(next.jd_ut > birth);
  CHECK(std::abs((next.jd_ut - birth) - lunation) < 2.0);
  CHECK(residual_arcsec(next, body::kMoon, radix_moon, ctx) < 5.0);

  const LongitudeCrossing prev = planetar_return(birth, body::kMoon, radix_moon, 1, false, ctx);
  REQUIRE(prev.ok);
  CHECK(prev.jd_ut < birth);
  CHECK(std::abs((birth - prev.jd_ut) - lunation) < 2.0);
  CHECK(residual_arcsec(prev, body::kMoon, radix_moon, ctx) < 5.0);

  // the second past lunation sits two lunar periods before birth
  const LongitudeCrossing prev2 = planetar_return(birth, body::kMoon, radix_moon, 2, false, ctx);
  REQUIRE(prev2.ok);
  CHECK(std::abs((birth - prev2.jd_ut) - 2.0 * lunation) < 3.0);
}

TEST_CASE("the progressions map one day onto one year") {
  const SearchContext ctx = context();
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = ctx.base.lon_deg_east;
  in.lat_deg = ctx.base.lat_deg;
  const Chart radix = compute_chart(in, ctx.settings, vsop(), eph());
  REQUIRE(radix.ok);
  const double event = julian_day({13, 10, 2022, 3, 0.0});
  // the strict interpolation lands thirty days after birth
  const ProgressedMoment prop = progressed_moment(radix, event, ProgressionMode::kProportional, ctx);
  REQUIRE(prop.ok);
  CHECK(prop.years == doctest::Approx(30.0).epsilon(1e-3));
  CHECK(prop.jd_ut - radix.jd_ut == doctest::Approx(prop.years).epsilon(1e-9));
  // the radix clock mode keeps the birth clock on the progressed day
  const ProgressedMoment clock = progressed_moment(radix, event, ProgressionMode::kRadixClock, ctx);
  REQUIRE(clock.ok);
  const double birth_frac = radix.jd_ut + 0.5 - std::floor(radix.jd_ut + 0.5);
  const double prog_frac = clock.jd_ut + 0.5 - std::floor(clock.jd_ut + 0.5);
  CHECK(prog_frac == doctest::Approx(birth_frac).epsilon(1e-9));
  CHECK(std::abs(clock.jd_ut - prop.jd_ut) < 1.0);
  // the true solar time mode reproduces the birth's sun hour angle
  const ProgressedMoment wahr = progressed_moment(radix, event, ProgressionMode::kTrueSolarTime, ctx);
  REQUIRE(wahr.ok);
  ChartInput win = in;
  win.date_ut = calendar_date(wahr.jd_ut);
  const Chart wc = compute_chart(win, ctx.settings, vsop(), eph());
  double dv = std::abs(norm_rad(sun_hour_angle(wc)) - norm_rad(sun_hour_angle(radix)));
  if (dv > kPi) {
    dv = kTwoPi - dv;
  }
  CHECK(dv < 2.0e-7);
  // the house rotation mode advances the sidereal time by the yearly
  // surplus of near four clock minutes
  const ProgressedMoment rot = progressed_moment(radix, event, ProgressionMode::kHouseRotation, ctx);
  REQUIRE(rot.ok);
  ChartInput rin = in;
  rin.date_ut = calendar_date(rot.jd_ut);
  const Chart rc = compute_chart(rin, ctx.settings, vsop(), eph());
  const double hs_target = radix.hs + 0.98565 * prop.years / 15.0;
  double dh = std::abs(rc.hs - hs_target);
  while (dh > 12.0) {
    dh = std::abs(dh - 24.0);
  }
  CHECK(dh < 0.01);
}

TEST_CASE("the day chart holds the birth's true solar time") {
  const SearchContext ctx = context();
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = ctx.base.lon_deg_east;
  in.lat_deg = ctx.base.lat_deg;
  const Chart radix = compute_chart(in, ctx.settings, vsop(), eph());
  REQUIRE(radix.ok);
  const ProgressedMoment day = day_chart_moment(radix, julian_day({1, 6, 2026, 3, 0.0}), ctx);
  REQUIRE(day.ok);
  ChartInput din = in;
  din.date_ut = calendar_date(day.jd_ut);
  const Chart dc = compute_chart(din, ctx.settings, vsop(), eph());
  double dv = std::abs(norm_rad(sun_hour_angle(dc)) - norm_rad(sun_hour_angle(radix)));
  if (dv > kPi) {
    dv = kTwoPi - dv;
  }
  CHECK(dv < 2.0e-7);
}

TEST_CASE("the sun rises and sets on the solstice clock") {
  const SearchContext ctx = context();
  const RiseSet rs = rise_transit_set(julian_day({21, 6, 2000, 12, 0.0}), body::kSun, false, ctx);
  REQUIRE(rs.ok);
  // the almanac for eleven degrees east and forty eight north puts the
  // solstice sunrise near 3:13 and the sunset near 19:17 UT
  const CalendarDate rise = calendar_date(rs.rise.jd_ut);
  const CalendarDate set = calendar_date(rs.set.jd_ut);
  CHECK(rise.hour * 60.0 + rise.minute == doctest::Approx(3.0 * 60 + 13).epsilon(0.02));
  CHECK(set.hour * 60.0 + set.minute == doctest::Approx(19.0 * 60 + 17).epsilon(0.02));
  const CalendarDate noon = calendar_date(rs.transit.jd_ut);
  CHECK(noon.hour * 60.0 + noon.minute == doctest::Approx(11.0 * 60 + 15).epsilon(0.02));
}

TEST_CASE("the moon's clock closes on its own standard altitude") {
  // the moon's depth once took asin of one over the distance in AU,
  // far outside the domain, and the panel parallax rode into the
  // coordinates on top. This pins the auf_unt behaviour, geocentric
  // positions with the pm(2) standard altitude.
  const SearchContext ctx = context();
  const double jd0 = julian_day({21, 6, 2000, 12, 0.0});
  const RiseSet rs = rise_transit_set(jd0, body::kMoon, false, ctx);
  REQUIRE(rs.ok);
  const double jde = std::floor(jd0 - 0.5) + 0.5;
  for (const double jd : {rs.rise.jd_ut, rs.transit.jd_ut, rs.set.jd_ut}) {
    CHECK(jd >= jde);
    CHECK(jd < jde + 1.0);
  }
  // the geocentric moon at each found moment, altitude and hour angle
  struct Sky {
    double h_deg;
    double h0_deg;
    double hg_deg;
  };
  const auto sky = [&](double jd) {
    ChartInput in = ctx.base;
    in.date_ut = calendar_date(jd);
    ChartSettings s = ctx.settings;
    s.topocentric_parallax = false;
    s.apparent = ApparentMode::kLightTime;
    const Chart c = compute_chart(in, s, vsop(), eph());
    REQUIRE(c.ok);
    const double theta = gmst0_hours(jde) * kDegPerHour + 360.985647 * (jd - jde);
    double hg = theta + ctx.base.lon_deg_east - norm_rad(c.b[body::kMoon].ar) * kRadToDeg;
    hg = norm_deg(hg);
    if (hg > 180.0) {
      hg -= 360.0;
    }
    const double phi = ctx.base.lat_deg * kDegToRad;
    const double de = c.b[body::kMoon].de;
    const double h = std::asin(std::sin(phi) * std::sin(de) +
                               std::cos(phi) * std::cos(hg * kDegToRad) * std::cos(de));
    return Sky{h * kRadToDeg, 0.7275 * c.moon.parallax * kRadToDeg - 0.56666666, hg};
  };
  const Sky rise = sky(rs.rise.jd_ut);
  CHECK(std::abs(rise.h_deg - rise.h0_deg) < 0.02);
  const Sky set = sky(rs.set.jd_ut);
  CHECK(std::abs(set.h_deg - set.h0_deg) < 0.02);
  const Sky noon = sky(rs.transit.jd_ut);
  CHECK(std::abs(noon.hg_deg) < 0.02);
  // in antiquity delta T runs to hours, his second delta T moved the
  // moon by about a degree there, the found moments still close
  const double old_day = julian_day({21, 6, 100, 12, 0.0});
  const RiseSet ancient = rise_transit_set(old_day, body::kMoon, false, ctx);
  REQUIRE(ancient.ok);
  const auto sky_at = [&](double jd, double day) {
    const double jde0 = std::floor(day - 0.5) + 0.5;
    ChartInput in = ctx.base;
    in.date_ut = calendar_date(jd);
    ChartSettings s = ctx.settings;
    s.topocentric_parallax = false;
    s.apparent = ApparentMode::kLightTime;
    const Chart c = compute_chart(in, s, vsop(), eph());
    REQUIRE(c.ok);
    const double theta = gmst0_hours(jde0) * kDegPerHour + 360.985647 * (jd - jde0);
    const double hg = (theta + ctx.base.lon_deg_east - norm_rad(c.b[body::kMoon].ar) * kRadToDeg) * kDegToRad;
    const double phi = ctx.base.lat_deg * kDegToRad;
    const double de = c.b[body::kMoon].de;
    const double h = std::asin(std::sin(phi) * std::sin(de) + std::cos(phi) * std::cos(hg) * std::cos(de));
    return h * kRadToDeg - (0.7275 * c.moon.parallax * kRadToDeg - 0.56666666);
  };
  CHECK(std::abs(sky_at(ancient.rise.jd_ut, old_day)) < 0.05);
  CHECK(std::abs(sky_at(ancient.set.jd_ut, old_day)) < 0.05);
  // his standard depth stays close to the 0.125 start value he notes
  CHECK(rise.h0_deg > 0.0);
  CHECK(rise.h0_deg < 0.3);
}

TEST_CASE("the dynamogram sums arcs into its two curves") {
  const SearchContext ctx = context();
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = ctx.base.lon_deg_east;
  in.lat_deg = ctx.base.lat_deg;
  const Chart radix = compute_chart(in, ctx.settings, vsop(), eph());
  REQUIRE(radix.ok);
  DynamogramOptions opt;
  opt.from_age = 20.0;
  const Dynamogram d = dynamogram(radix, opt, ctx);
  REQUIRE(d.mood.size() == 6001);
  double mood_peak = 0.0;
  double exist_peak = 0.0;
  for (std::size_t i = 3000; i < 3600; ++i) {
    mood_peak = std::max(mood_peak, std::abs(d.mood[i]));
    exist_peak = std::max(exist_peak, std::abs(d.existential[i]));
  }
  // arcs land in the visible window and stay within a sane band
  CHECK(mood_peak > 0.0);
  CHECK(exist_peak > 0.0);
  CHECK(mood_peak < 5000.0);
}

TEST_CASE("the dynamogram keeps the Moon out of the mutual arcs") {
  // asp_analy_mund runs Sun and Mercury to Pluto against Mercury to
  // Pluto, AC and MC, a Sun Moon pair once slipped in
  const auto pairs = dynamogram_mutual_pairs();
  bool moon = false;
  bool nodes = false;
  bool sun_mc = false;
  for (const auto& [a, b] : pairs) {
    moon = moon || a == body::kMoon || b == body::kMoon;
    nodes = nodes || a == body::kNodeAsc || b == body::kNodeAsc || a == body::kNodeDesc || b == body::kNodeDesc;
    sun_mc = sun_mc || (a == body::kSun && b == body::kMc);
    CHECK(a < b);
  }
  CHECK_FALSE(moon);
  CHECK_FALSE(nodes);
  CHECK(sun_mc);
  // the Sun meets the eight planets Mercury to Pluto, Mercury the seven
  // above it and so on, and all nine first planets meet AC and MC
  CHECK(pairs.size() == 8 + 7 + 6 + 5 + 4 + 3 + 2 + 1 + 9 * 2);
}

TEST_CASE("the moon arc leaves AC and MC in place, the sun arc directs them") {
  const SearchContext ctx = context();
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = ctx.base.lon_deg_east;
  in.lat_deg = ctx.base.lat_deg;
  const Chart radix = compute_chart(in, ctx.settings, vsop(), eph());
  REQUIRE(radix.ok);
  const double from = radix.jd_ut;
  const double to = radix.jd_ut + 40.0 * radix.ta.tropical_year_days;
  const auto angle_moves = [](const std::vector<DirectedEvent>& events) {
    for (const DirectedEvent& e : events) {
      if (e.event.transiting == body::kAscendant || e.event.transiting == body::kMc) {
        return true;
      }
    }
    return false;
  };
  TransitScan life;
  life.jd_from_ut = from;
  life.jd_to_ut = to;
  life.base_angle_deg = 90.0;
  const std::vector<DirectedEvent> sun = arc_direction_events(radix, false, life, ctx);
  const std::vector<DirectedEvent> moon = arc_direction_events(radix, true, life, ctx);
  REQUIRE_FALSE(moon.empty());
  CHECK(angle_moves(sun));
  CHECK_FALSE(angle_moves(moon));
}

TEST_CASE("the day chart lands on the chosen day for morning and evening births") {
  const SearchContext ctx = context();
  const double day0 = julian_day({24, 9, 2026, 0, 0.0});
  for (const double birth_hour : {3.0, 7.0, 11.0, 15.0, 20.0}) {
    CAPTURE(birth_hour);
    ChartInput in;
    in.date_ut = {10, 5, 1970, birth_hour, 0.0};
    in.lon_deg_east = ctx.base.lon_deg_east;
    in.lat_deg = ctx.base.lat_deg;
    const Chart radix = compute_chart(in, ctx.settings, vsop(), eph());
    REQUIRE(radix.ok);
    const double seed = event_at_radix_clock(radix, day0, false);
    // a julian day of this size resolves near fifty microseconds, a
    // millisecond of slack still catches every wrong clock
    CHECK(std::abs(seed - (day0 + birth_hour / 24.0)) < 1.0e-8);
    const ProgressedMoment m = day_chart_moment(radix, seed, ctx);
    REQUIRE(m.ok);
    // a noon seed once moved every birth before local noon a day on
    CHECK(m.jd_ut >= day0 - 1.0 / 24.0);
    CHECK(m.jd_ut < day0 + 1.0);
    CHECK(std::abs(m.jd_ut - seed) < 1.0 / 24.0);
  }
  // the progression reads noon for a radix clock of exactly midnight
  ChartInput mid;
  mid.date_ut = {10, 5, 1970, 0, 0.0};
  mid.lon_deg_east = ctx.base.lon_deg_east;
  mid.lat_deg = ctx.base.lat_deg;
  const Chart radix = compute_chart(mid, ctx.settings, vsop(), eph());
  CHECK(std::abs(event_at_radix_clock(radix, day0, true) - (day0 + 0.5)) < 1.0e-8);
  CHECK(std::abs(event_at_radix_clock(radix, day0, false) - day0) < 1.0e-8);
}

TEST_CASE("a retrograde loop yields the multiple planetar moments") {
  const SearchContext ctx = context();
  // find a moment with Mercury retrograde after the sample birth
  double t_retro = 0.0;
  for (int k = 0; k < 120; ++k) {
    const BodyLongitude b = body_longitude(2448908.5 + k, body::kMercury, ctx);
    if (b.valid && b.tb < 0.0) {
      t_retro = 2448908.5 + k + 5.0;
      break;
    }
  }
  REQUIRE(t_retro > 0.0);
  // the longitude in the middle of the loop is crossed three times, the
  // search finds the last passage and planth walks back to the other two
  const double target = body_longitude(t_retro, body::kMercury, ctx).el;
  const LongitudeCrossing last_pass = find_longitude_backward(t_retro + 40.0, body::kMercury, target, ctx);
  REQUIRE(last_pass.ok);
  const auto extra = planetar_earlier(last_pass, body::kMercury, target, ctx);
  CHECK(extra.size() == 2);
  REQUIRE(!extra.empty());
  CHECK(extra.front().retrograde);
  double last = last_pass.jd_ut;
  for (const LongitudeCrossing& c : extra) {
    CHECK(c.jd_ut < last);
    last = c.jd_ut;
    double d = norm_rad(body_longitude(c.jd_ut, body::kMercury, ctx).el - target);
    if (d > kPi) {
      d -= kTwoPi;
    }
    CHECK(std::abs(d) < 1.0e-4);
  }
}

TEST_CASE("the combined grids keep only their own multiples") {
  // a18_3045, 30 und 45 on a 15 degree base
  for (int w = 0; w < 24; ++w) {
    const int deg = w * 15;
    CAPTURE(deg);
    CHECK(grid_skips(AspectGrid::k30And45, w) == !(deg % 30 == 0 || deg % 45 == 0));
    CHECK(grid_skips(AspectGrid::k60And45, w) == !(deg % 60 == 0 || deg % 45 == 0));
  }
  // 60 und 90 on a 30 degree base
  for (int w = 0; w < 12; ++w) {
    const int deg = w * 30;
    CAPTURE(deg);
    CHECK(grid_skips(AspectGrid::k60And90, w) == !(deg % 60 == 0 || deg % 90 == 0));
  }
  CHECK_FALSE(grid_skips(AspectGrid::kPlain, 7));
}

TEST_CASE("the mundane scan finds the new moon at zero Aries") {
  // the new moon of 21 March 2023 at 17:23 UT stood at 0 Aries 49, his
  // separate unwrapping of both bodies lost this conjunction
  const SearchContext ctx = context();
  MundaneScan scan;
  scan.jd_from_ut = julian_day({20, 3, 2023, 0, 0.0});
  scan.jd_to_ut = julian_day({23, 3, 2023, 0, 0.0});
  scan.base_angle_deg = 30.0;
  scan.moon = true;
  const std::vector<MundaneAspect> hits = scan_mundane_aspects(scan, ctx);
  const MundaneAspect* nm = nullptr;
  for (const MundaneAspect& m : hits) {
    if (m.first == body::kSun && m.second == body::kMoon && m.multiple == 0) {
      nm = &m;
    }
  }
  REQUIRE(nm != nullptr);
  const double reference = julian_day({21, 3, 2023, 17, 23.0});
  CHECK(std::abs(nm->jd_ut - reference) * 1440.0 < 5.0);
  CHECK(nm->first_lon * kRadToDeg == doctest::Approx(0.82).epsilon(0.05));
  // without the moon the pair never shows
  scan.moon = false;
  for (const MundaneAspect& m : scan_mundane_aspects(scan, ctx)) {
    CHECK(m.first != body::kMoon);
    CHECK(m.second != body::kMoon);
  }
}

TEST_CASE("the mundane scan finds the great conjunction of 2020") {
  // Jupiter and Saturn met on 21 December 2020 around 18:20 UT
  const SearchContext ctx = context();
  MundaneScan scan;
  scan.jd_from_ut = julian_day({1, 12, 2020, 0, 0.0});
  scan.jd_to_ut = julian_day({31, 12, 2020, 0, 0.0});
  scan.base_angle_deg = 30.0;
  scan.first_slot = body::kJupiter;
  const std::vector<MundaneAspect> hits = scan_mundane_aspects(scan, ctx);
  const MundaneAspect* gc = nullptr;
  for (const MundaneAspect& m : hits) {
    CHECK(m.first >= body::kJupiter);
    if (m.first == body::kJupiter && m.second == body::kSaturn && m.multiple == 0) {
      gc = &m;
    }
  }
  REQUIRE(gc != nullptr);
  const double reference = julian_day({21, 12, 2020, 18, 20.0});
  CHECK(std::abs(gc->jd_ut - reference) * 24.0 < 2.0);
  // the two stand together at 0 Aquarius 29
  CHECK(gc->first_lon * kRadToDeg == doctest::Approx(300.49).epsilon(0.001));
}

TEST_CASE("the void of course rule drops the aspects after the sign change") {
  // the moon left Pisces on 21 March 2023 at 0 Aries, starting the scan
  // on the 20th its aspects in Aries stay out
  const SearchContext ctx = context();
  MundaneScan scan;
  scan.jd_from_ut = julian_day({20, 3, 2023, 0, 0.0});
  scan.jd_to_ut = julian_day({24, 3, 2023, 0, 0.0});
  scan.base_angle_deg = 15.0;
  scan.grid = AspectGrid::k30And45;
  scan.moon = true;
  scan.within_sign = true;
  for (const MundaneAspect& m : scan_mundane_aspects(scan, ctx)) {
    if (m.first == body::kMoon || m.second == body::kMoon) {
      const double moon = m.first == body::kMoon ? m.first_lon : m.second_lon;
      CHECK(moon * kRadToDeg > 300.0);
    }
  }
}

TEST_CASE("the transit sweep reaches his extra targets") {
  const SearchContext ctx = context();
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  const Chart radix = compute_chart(in, ctx.settings, vsop(), eph());
  REQUIRE(radix.ok);
  // the residual of an event against the point it names
  const auto residual = [&](const TransitEvent& e, double base_deg) {
    double point = 0.0;
    if (e.cusp > 0) {
      point = radix.houses.cusp[static_cast<std::size_t>(e.cusp)];
    } else if (e.radix2 > 0) {
      point = midpoint_near(radix.b[static_cast<std::size_t>(e.radix)].el,
                            radix.b[static_cast<std::size_t>(e.radix2)].el);
    } else if (e.radix >= 15 && e.radix <= 18) {
      point = (e.radix - 15) * kPi / 2.0;
    } else {
      point = radix.b[static_cast<std::size_t>(e.radix)].el;
    }
    const double target = norm_rad(point + e.multiple * base_deg * kDegToRad);
    double d = std::abs(body_longitude(e.jd_ut, e.transiting, ctx).el - target);
    if (d > kPi) {
      d = kTwoPi - d;
    }
    return d * kRadToDeg * 3600.0;
  };

  SUBCASE("the sun reaches 0 Aries as a cardinal point") {
    TransitScan scan;
    scan.jd_from_ut = julian_day({15, 3, 1993, 0, 0.0});
    scan.jd_to_ut = julian_day({25, 3, 1993, 0, 0.0});
    scan.chosen = {body::kSun};
    scan.cardinal_targets = true;
    bool equinox = false;
    for (const TransitEvent& e : scan_transits(radix, scan, ctx)) {
      CHECK(e.transiting == body::kSun);
      CHECK(residual(e, 30.0) < 1.0);
      if (e.radix == 15 && e.multiple == 0) {
        equinox = true;
        // the 1993 spring equinox fell on 20 March 14:41 UT
        CHECK(std::abs(e.jd_ut - julian_day({20, 3, 1993, 14, 41.0})) * 1440.0 < 5.0);
      }
    }
    CHECK(equinox);
  }

  SUBCASE("intermediate cusps and midpoints join as targets") {
    TransitScan scan;
    scan.jd_from_ut = julian_day({1, 11, 1992, 0, 0.0});
    scan.jd_to_ut = julian_day({1, 1, 1993, 0, 0.0});
    scan.chosen = {body::kSun};
    scan.house_targets = true;
    scan.midpoints = 1;
    bool cusp = false;
    bool midpoint = false;
    bool own_midpoint = false;
    for (const TransitEvent& e : scan_transits(radix, scan, ctx)) {
      CHECK(residual(e, 30.0) < 1.0);
      cusp = cusp || e.cusp > 0;
      midpoint = midpoint || e.radix2 > 0;
      own_midpoint = own_midpoint || (e.radix2 > 0 && (e.radix == body::kSun || e.radix2 == body::kSun));
    }
    CHECK(cusp);
    CHECK(midpoint);
    CHECK(own_midpoint);
    // NUR DIE MIT 3 UNTERSCHIEDLICHEN Faktoren leaves the sun's own midpoints out
    scan.midpoints = 2;
    for (const TransitEvent& e : scan_transits(radix, scan, ctx)) {
      if (e.radix2 > 0) {
        CHECK(e.radix != body::kSun);
        CHECK(e.radix2 != body::kSun);
      }
    }
  }

  SUBCASE("the grid, the first slot and the moon rule the running sky") {
    TransitScan scan;
    scan.jd_from_ut = julian_day({1, 11, 1992, 0, 0.0});
    scan.jd_to_ut = julian_day({1, 2, 1993, 0, 0.0});
    scan.base_angle_deg = 15.0;
    scan.grid = AspectGrid::k30And45;
    scan.first_slot = body::kMars;
    const std::vector<TransitEvent> events = scan_transits(radix, scan, ctx);
    REQUIRE(!events.empty());
    for (const TransitEvent& e : events) {
      CHECK(e.transiting >= body::kMars);
      const int deg = static_cast<int>(std::lround(e.angle_deg));
      CHECK((deg % 30 == 0 || deg % 45 == 0));
    }
  }
}

TEST_CASE("the sun arc moves the cardinal points and cusps with every point") {
  const SearchContext ctx = context();
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  const Chart radix = compute_chart(in, ctx.settings, vsop(), eph());
  REQUIRE(radix.ok);
  TransitScan life;
  life.jd_from_ut = radix.jd_ut;
  life.jd_to_ut = radix.jd_ut + 60.0 * radix.ta.tropical_year_days;
  life.base_angle_deg = 30.0;
  life.cardinal_targets = true;
  life.house_targets = true;
  // Mars, so the targets must shift by the distance of Mars to the sun
  life.chosen = {body::kMars};
  const std::vector<DirectedEvent> events = arc_direction_events(radix, false, life, ctx);
  REQUIRE(!events.empty());
  bool cardinal = false;
  bool cusp = false;
  for (const DirectedEvent& e : events) {
    CHECK(e.event.transiting == body::kMars);
    // directed Mars moves by the progressed sun's travel and stands on
    // the point it names at the life date
    const double years = (e.jd_life_ut - radix.jd_ut) / radix.ta.tropical_year_days;
    const double arc = body_longitude(radix.jd_ut + years, body::kSun, ctx).el - radix.b[body::kSun].el;
    const double progressed = norm_rad(radix.b[body::kMars].el + arc);
    double point = 0.0;
    if (e.event.cusp > 0) {
      cusp = true;
      point = radix.houses.cusp[static_cast<std::size_t>(e.event.cusp)];
    } else if (e.event.radix >= 15 && e.event.radix <= 18) {
      cardinal = true;
      point = (e.event.radix - 15) * kPi / 2.0;
    } else {
      continue;
    }
    double d = std::abs(norm_rad(progressed - norm_rad(point + e.event.multiple * 30.0 * kDegToRad)));
    if (d > kPi) {
      d = kTwoPi - d;
    }
    CHECK(d * kRadToDeg * 3600.0 < 5.0);
  }
  CHECK(cardinal);
  CHECK(cusp);
}

TEST_CASE("the rise and set moments carry his auf_unt3 block values") {
  const SearchContext ctx = context();
  const RiseSet rs = rise_transit_set(julian_day({21, 6, 2000, 12, 0.0}), body::kSun, true, ctx);
  REQUIRE(rs.ok);
  REQUIRE(rs.rise.ok);
  REQUIRE(rs.transit.ok);
  REQUIRE(rs.set.ok);
  // at the meridian passage the Greenwich sidereal time plus the east
  // longitude meets the right ascension
  double hg = norm_deg(rs.transit.gst_deg + ctx.base.lon_deg_east - norm_rad(rs.transit.ar) * kRadToDeg);
  if (hg > 180.0) {
    hg -= 360.0;
  }
  CHECK(std::abs(hg) < 0.01);
  // the sun stands at the solstice point that day
  CHECK(rs.transit.el * kRadToDeg == doctest::Approx(90.0).epsilon(0.005));
  CHECK(std::abs(rs.transit.eb) < 1.0e-4);
  // WAHR drops the refraction depth, the geometric sun rises later and
  // sets earlier than the apparent one
  const RiseSet apparent = rise_transit_set(julian_day({21, 6, 2000, 12, 0.0}), body::kSun, false, ctx);
  CHECK(rs.rise.jd_ut > apparent.rise.jd_ut);
  CHECK(rs.set.jd_ut < apparent.set.jd_ut);
}

TEST_CASE("the moon keeps its own standard altitude under WAHR like auf_pl") {
  // auf_pl sets hh0 = 0.7275 * pm(2) * up - 0.56666666 on every call for
  // the moon, the WAHR answer only reaches the other bodies
  const SearchContext ctx = context();
  const double jd0 = julian_day({21, 6, 2000, 12, 0.0});
  const RiseSet rs = rise_transit_set(jd0, body::kMoon, true, ctx);
  REQUIRE(rs.ok);
  REQUIRE(rs.rise.ok);
  const double jde = std::floor(jd0 - 0.5) + 0.5;
  ChartInput in = ctx.base;
  in.date_ut = calendar_date(rs.rise.jd_ut);
  ChartSettings s = ctx.settings;
  s.topocentric_parallax = false;
  s.apparent = ApparentMode::kTrue;
  const Chart c = compute_chart(in, s, vsop(), eph());
  REQUIRE(c.ok);
  const double theta = gmst0_hours(jde) * kDegPerHour + 360.985647 * (rs.rise.jd_ut - jde);
  const double hg = (theta + ctx.base.lon_deg_east - norm_rad(c.b[body::kMoon].ar) * kRadToDeg) * kDegToRad;
  const double phi = ctx.base.lat_deg * kDegToRad;
  const double de = c.b[body::kMoon].de;
  const double h = std::asin(std::sin(phi) * std::sin(de) + std::cos(phi) * std::cos(hg) * std::cos(de)) * kRadToDeg;
  CHECK(std::abs(h - (0.7275 * c.moon.parallax * kRadToDeg - 0.56666666)) < 0.02);
}

TEST_CASE("the dynamogram records his single arcs and the MITTEL over all samples") {
  const SearchContext ctx = context();
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = ctx.base.lon_deg_east;
  in.lat_deg = ctx.base.lat_deg;
  const Chart radix = compute_chart(in, ctx.settings, vsop(), eph());
  REQUIRE(radix.ok);
  DynamogramOptions opt;
  opt.from_age = 20.0;
  opt.regressive = true;
  const Dynamogram d = dynamogram(radix, opt, ctx);
  REQUIRE_FALSE(d.arcs.empty());
  bool progressive = false;
  bool regressive = false;
  // the passes come in his order, the radix arcs before the mutual ones,
  // the progressive run before the regressive one
  int pass = 0;
  for (const DynamogramArc& a : d.arcs) {
    // every recorded arc touches the visible window of 3000 to 3600
    CHECK(a.i2 > kDynamogramWindowStart);
    CHECK(a.i1 < kDynamogramArcWindowEnd);
    CHECK(a.bog.size() == static_cast<std::size_t>(a.i2 - a.i1 + 1));
    progressive = progressive || !a.regressive;
    regressive = regressive || a.regressive;
    const int here = (a.regressive ? 2 : 0) + (a.radix ? 0 : 1);
    CHECK(here >= pass);
    pass = here;
  }
  CHECK(progressive);
  CHECK(regressive);
  CHECK(kDynamogramArcWindowEnd == 3600);
  // his mittel% summed the unscaled samples outside the window into an
  // integer, the mean now scales every sample. The sum over the samples 0
  // to 6000 divides by 6000 like his STR$(mittel% / 6000), fifty years of
  // one hundred twenty samples
  Dynamogram flat;
  flat.existential.assign(6001, 10.0);
  flat.mood.assign(6001, -4.0);
  CHECK(dynamogram_mean(flat, 0.3) == doctest::Approx(1.8 * 6001.0 / 6000.0));
  flat.mood[0] = -4.0 + 6000.0;
  CHECK(dynamogram_mean(flat, 0.3) == doctest::Approx(1.8 * 6001.0 / 6000.0 + 0.3));
}

TEST_CASE("the angle ingresses land every sign within the day") {
  // his korr1 and korr2 iterated to 5e-7, the port once asked 1e-9, which
  // a julian day in double cannot resolve, and lost most signs
  SearchContext ctx = context();
  for (const int slot : {body::kMc, body::kAscendant}) {
    const double start = julian_day({10, 5, 1970, 0, 0.0});
    const auto t = angle_ingresses(start, slot, ctx);
    for (int i = 0; i < 12; ++i) {
      REQUIRE(t[static_cast<std::size_t>(i)].ok);
      CHECK(t[static_cast<std::size_t>(i)].jd_ut >= start);
      CHECK(t[static_cast<std::size_t>(i)].jd_ut < start + 1.0);
    }
  }
}

TEST_CASE("the planetar counts with the sidereal periods and the tropical year for Mercury") {
  // his a16_ta made Saturn 10821.4 days long and counted the DATUM of
  // geocentric Mercury by 88 days, the same return read 10. by NR and 41.
  // by DATUM
  CHECK(planetar_count_period(body::kSaturn, 365.2422, false) == doctest::Approx(10759.2));
  CHECK(planetar_count_period(body::kJupiter, 365.2422, false) == doctest::Approx(4332.6));
  CHECK(planetar_count_period(body::kMercury, 365.2422, false) == doctest::Approx(365.2422));
  CHECK(planetar_count_period(body::kMercury, 365.2422, true) == doctest::Approx(0.24085 * 365.2422));
}

namespace {

// the crossings of a body over a longitude found without the sweep, a
// quarter day grid and a plain bisection on the ephemeris
std::vector<double> bisected_crossings(int slot, double target, double from, double to, const SearchContext& ctx) {
  std::vector<double> out;
  const auto diff = [&](double jd) { return fold_rad(body_longitude(jd, slot, ctx).el - target); };
  double t0 = from;
  double f0 = diff(t0);
  for (double t1 = from + 0.25; t1 <= to; t1 += 0.25) {
    const double f1 = diff(t1);
    if ((f0 < 0.0) != (f1 < 0.0) && std::abs(f1 - f0) < 1.0) {
      double a = t0;
      double b = t1;
      double fa = f0;
      for (int i = 0; i < 60; ++i) {
        const double m = 0.5 * (a + b);
        const double fm = diff(m);
        if ((fm < 0.0) == (fa < 0.0)) {
          a = m;
          fa = fm;
        } else {
          b = m;
        }
      }
      out.push_back(0.5 * (a + b));
    }
    t0 = t1;
    f0 = f1;
  }
  return out;
}

// a radix of chosen longitudes, the sweep needs nothing else
Chart points_radix(std::initializer_list<std::pair<int, double>> points) {
  Chart c;
  c.ok = true;
  for (const auto& [slot, lon] : points) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = lon;
  }
  return c;
}

Chart sample_radix(const SearchContext& ctx) {
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = ctx.base.lon_deg_east;
  in.lat_deg = ctx.base.lat_deg;
  return compute_chart(in, ctx.settings, vsop(), eph());
}

}  // namespace

TEST_CASE("the transit sweep lists the retrograde passage of a running body") {
  // the port once refined every hit with the backward plant search. That
  // search closes only on a forward crossing, a retrograde passage walked
  // back to the direct one before and was dropped. His a180aus listed it
  const SearchContext ctx = context();
  double t_retro = 0.0;
  for (int k = 0; k < 120; ++k) {
    const BodyLongitude b = body_longitude(2448908.5 + k, body::kMercury, ctx);
    if (b.valid && b.tb < 0.0) {
      t_retro = 2448908.5 + k + 5.0;
      break;
    }
  }
  REQUIRE(t_retro > 0.0);
  // a point in the middle of the loop, crossed direct, retrograde and
  // direct again
  const double target = body_longitude(t_retro, body::kMercury, ctx).el;
  const Chart radix = points_radix({{body::kSun, target}, {body::kMercury, norm_rad(target + kPi)}});
  // the window is set off by a fraction so no crossing falls on a
  // sample, his strict bracket test lets a crossing there slip like the
  // port's
  TransitScan scan;
  scan.jd_from_ut = t_retro - 60.3;
  scan.jd_to_ut = t_retro + 60.0;
  scan.base_angle_deg = kDegPerCircle;
  scan.chosen = {body::kMercury};
  std::vector<TransitEvent> over;
  for (const TransitEvent& e : scan_transits(radix, scan, ctx)) {
    if (e.radix == body::kSun && !e.station_touch) {
      over.push_back(e);
    }
  }
  const std::vector<double> expected = bisected_crossings(body::kMercury, target, scan.jd_from_ut, scan.jd_to_ut, ctx);
  REQUIRE(expected.size() == 3);
  REQUIRE(over.size() == 3);
  for (std::size_t i = 0; i < 3; ++i) {
    CAPTURE(i);
    CHECK(std::abs(over[i].jd_ut - expected[i]) * kSecondsPerDay < 1.0);
  }
  CHECK_FALSE(over[0].retrograde);
  CHECK(over[1].retrograde);
  CHECK_FALSE(over[2].retrograde);
}

TEST_CASE("the mean node transits the Aries point on its retrograde wrap") {
  // his vergl2 order sent a retrograde step over 0 Aries into the direct
  // test, whose speed cap dropped it, so the original never listed the
  // mean node reaching the Aries point. The port picks the branch by the
  // mean motion. Before this its plant refinement lost every passage of
  // the always retrograde mean node too
  const SearchContext ctx = context();
  const double from = julian_day({1, 1, 2000, 0, 0.0});
  double wrap = 0.0;
  double prev = body_longitude(from, body::kNodeAsc, ctx).el;
  for (double jd = from + 5.0; jd < from + 20.0 * 365.25; jd += 5.0) {
    const double el = body_longitude(jd, body::kNodeAsc, ctx).el;
    if (prev < 0.5 && el > kTwoPi - 0.5) {
      wrap = jd;
      break;
    }
    prev = el;
  }
  REQUIRE(wrap > 0.0);
  const Chart radix = points_radix({{body::kNodeAsc, 1.0}});
  TransitScan scan;
  scan.jd_from_ut = wrap - 30.0;
  scan.jd_to_ut = wrap + 10.0;
  scan.base_angle_deg = kDegPerCircle;
  scan.chosen = {body::kNodeAsc};
  scan.cardinal_targets = true;
  const std::vector<TransitEvent> events = scan_transits(radix, scan, ctx);
  const TransitEvent* aries = nullptr;
  for (const TransitEvent& e : events) {
    if (e.radix == body::kAriesPoint && !e.station_touch) {
      aries = &e;
    }
  }
  const std::vector<double> expected = bisected_crossings(body::kNodeAsc, 0.0, scan.jd_from_ut, scan.jd_to_ut, ctx);
  REQUIRE(expected.size() == 1);
  REQUIRE(aries != nullptr);
  // the node moves three arc minutes a day, two seconds of time stay
  // far under a thousandth of an arc second
  CHECK(std::abs(aries->jd_ut - expected[0]) * kSecondsPerDay < 2.0);
  CHECK(aries->retrograde);
}

TEST_CASE("the station guard leaves the nodes alone") {
  // a180_1 keeps the nodes out of its stationary test, the port once let
  // the true node through
  SearchContext ctx = context();
  ctx.settings.true_node = true;
  const Chart radix = sample_radix(ctx);
  REQUIRE(radix.ok);
  TransitScan scan;
  scan.jd_from_ut = julian_day({1, 1, 1993, 0, 0.0});
  scan.jd_to_ut = julian_day({1, 1, 1994, 0, 0.0});
  scan.chosen = {body::kNodeAsc};
  for (const TransitEvent& e : scan_transits(radix, scan, ctx)) {
    CHECK_FALSE(e.station_touch);
  }
}

TEST_CASE("the secondary direction honours Ab MARS AUFWÄRTS") {
  // a180 set pl1& = aa& for prog!, sobg! and mob! after a18eing_plw had
  // asked, so Ab MARS and Ab JUPITER changed nothing and the list still
  // named the progressed Sun to Venus. The port directs from the chosen
  // body on
  const SearchContext ctx = context();
  const Chart radix = sample_radix(ctx);
  REQUIRE(radix.ok);
  TransitScan life;
  life.jd_from_ut = radix.jd_ut;
  life.jd_to_ut = radix.jd_ut + 60.0 * radix.ta.tropical_year_days;
  life.base_angle_deg = 90.0;
  const auto has_sun = [](const std::vector<DirectedEvent>& events) {
    return std::any_of(events.begin(), events.end(),
                       [](const DirectedEvent& d) { return d.event.transiting == body::kSun; });
  };
  CHECK(has_sun(secondary_direction_events(radix, life, ctx)));
  life.first_slot = body::kMars;
  for (const DirectedEvent& d : secondary_direction_events(radix, life, ctx)) {
    CHECK(d.event.transiting >= body::kMars);
  }
}

TEST_CASE("the linear date axis puts a day over its own tick") {
  // skalh counts the day of the month ta1 in, the first of the month
  // stands on the tick 1 one day right of the axis start. The port drew
  // every transit curve and hit one day early
  const SearchContext ctx = context();
  const Chart radix = sample_radix(ctx);
  REQUIRE(radix.ok);
  LinearOptions opt;
  opt.kind = LinearKind::kTransits;
  opt.base_angle_deg = 90.0;
  opt.jd_from_ut = julian_day({1, 3, 2001, 0, 0.0});
  opt.jd_to_ut = opt.jd_from_ut + 31.0;
  LinearHit h;
  h.jd_ut = julian_day({5, 3, 2001, 0, 0.0});
  h.running = body::kVenus;
  h.radix = body::kSun;
  h.angle_deg = 90.0;
  opt.hits.push_back(h);
  const DisplayList dl = build_linear_graph(radix, opt, ctx);
  // sixteen pixels a day, the fifth at 80 + 16 * 5
  const double x = 80.0 + 16.0 * 5.0;
  const double y = kLinearBandBottom - norm_rad(4.0 * radix.b[body::kSun].el) * kRadToDeg;
  bool square = false;
  bool green_line = false;
  bool label_five = false;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kGlyph && std::abs(p.x1 - x) < 0.01 && std::abs(p.y1 - y) < 0.01 && p.text == "□") {
      square = true;
    }
    // rot_grn_line draws the hit of Venus green and dotted
    if (p.kind == Primitive::Kind::kLine && std::abs(p.x1 - x) < 0.01 && std::abs(p.x2 - x) < 0.01 &&
        std::abs(p.y1 - y) < 0.01 && p.color == 0x009600 && p.style == Primitive::Style::kDotted) {
      green_line = true;
    }
    // the red day number 5 sits eight pixels left of its tick
    if (p.kind == Primitive::Kind::kText && p.text == "5" && std::abs(p.x1 - (x - 8.0)) < 0.01) {
      label_five = true;
    }
  }
  CHECK(square);
  CHECK(green_line);
  CHECK(label_five);
}

TEST_CASE("the mundane linear graph marks the first body and draws no radix") {
  const SearchContext ctx = context();
  const Chart radix = sample_radix(ctx);
  REQUIRE(radix.ok);
  LinearOptions opt;
  opt.kind = LinearKind::kMundane;
  opt.base_angle_deg = 90.0;
  opt.jd_from_ut = julian_day({1, 3, 2001, 0, 0.0});
  opt.jd_to_ut = opt.jd_from_ut + 31.0;
  opt.title = " Ekliptikale Mundan-Aspekte ";
  // Mars squares Saturn with Mars at 100 degrees on the tenth
  LinearHit h;
  h.jd_ut = julian_day({10, 3, 2001, 0, 0.0});
  h.running = body::kSaturn;
  h.radix = body::kMars;
  h.angle_deg = 90.0;
  h.lon = 100.0 * kDegToRad;
  opt.hits.push_back(h);
  const DisplayList dl = build_linear_graph(radix, opt, ctx);
  const double x = 80.0 + 16.0 * 10.0;
  const double y = kLinearBandBottom - norm_deg(4.0 * 100.0);
  bool square = false;
  bool red_line = false;
  bool radix_line = false;
  for (const Primitive& p : dl.items) {
    // a181 sets the sprite of a mundane hit three pixels up and left
    if (p.kind == Primitive::Kind::kGlyph && std::abs(p.x1 - (x - 3.0)) < 0.01 && std::abs(p.y1 - (y - 3.0)) < 0.01 &&
        p.text == "□") {
      square = true;
    }
    // both bodies heavy, the line is red and solid
    if (p.kind == Primitive::Kind::kLine && std::abs(p.x1 - x) < 0.01 && std::abs(p.y1 - y) < 0.01 &&
        p.color == 0xFF0000 && p.style == Primitive::Style::kSolid) {
      red_line = true;
    }
    // a18_lin draws the radix lines only for the transits and directions,
    // the sign boundaries run dash dot
    radix_line = radix_line || (p.kind == Primitive::Kind::kLine && p.x1 == 54.0 && p.x2 == 622.0 && p.y1 == p.y2 &&
                                p.style == Primitive::Style::kSolid);
  }
  CHECK(square);
  CHECK(red_line);
  CHECK_FALSE(radix_line);
  // at 360 degrees the signs stand down both edges
  opt.base_angle_deg = kDegPerCircle;
  opt.hits.clear();
  int edge_signs = 0;
  for (const Primitive& p : build_linear_graph(radix, opt, ctx).items) {
    if (p.kind == Primitive::Kind::kGlyph && (p.x1 == 45.0 || p.x1 == 605.0)) {
      ++edge_signs;
    }
  }
  CHECK(edge_signs == 24);
}
