// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

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
