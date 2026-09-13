// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
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

TEST_CASE("an extra body outside his ephemeris rides the element fallback") {
  SearchContext ctx = context();
  ctx.settings.enable_standard_extras();
  // far outside the integrated span the pipeline follows the orbital
  // elements, the search converges on that path too
  const LongitudeCrossing hit = find_longitude_backward(julian_day({1, 1, 900, 0, 0.0}), body::kChiron, 0.0, ctx);
  REQUIRE(hit.ok);
  CHECK(residual_arcsec(hit, body::kChiron, 0.0, ctx) < 0.5);
}
