// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/dynamogram.hpp"
#include "horcom/chart/progressions.hpp"
#include "horcom/chart/riseset.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
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
  const CalendarDate rise = calendar_date(rs.jd_rise_ut);
  const CalendarDate set = calendar_date(rs.jd_set_ut);
  CHECK(rise.hour * 60.0 + rise.minute == doctest::Approx(3.0 * 60 + 13).epsilon(0.02));
  CHECK(set.hour * 60.0 + set.minute == doctest::Approx(19.0 * 60 + 17).epsilon(0.02));
  const CalendarDate noon = calendar_date(rs.jd_transit_ut);
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
  for (const double jd : {rs.jd_rise_ut, rs.jd_transit_ut, rs.jd_set_ut}) {
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
  const Sky rise = sky(rs.jd_rise_ut);
  CHECK(std::abs(rise.h_deg - rise.h0_deg) < 0.1);
  const Sky set = sky(rs.jd_set_ut);
  CHECK(std::abs(set.h_deg - set.h0_deg) < 0.1);
  const Sky noon = sky(rs.jd_transit_ut);
  CHECK(std::abs(noon.hg_deg) < 0.1);
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
  // the longitude in the middle of the loop is crossed again on both
  // sides, the forward walk must find those extra passes
  const double target = body_longitude(t_retro, body::kMercury, ctx).el;
  LongitudeCrossing first;
  first.ok = true;
  first.jd_ut = t_retro - 25.0;
  const auto extra = planetar_multiples(first, body::kMercury, target, ctx);
  CHECK(extra.size() >= 2);
  double last = first.jd_ut;
  for (const LongitudeCrossing& c : extra) {
    CHECK(c.jd_ut > last);
    last = c.jd_ut;
    double d = norm_rad(body_longitude(c.jd_ut, body::kMercury, ctx).el - target);
    if (d > kPi) {
      d -= kTwoPi;
    }
    CHECK(std::abs(d) < 1.0e-4);
  }
}
