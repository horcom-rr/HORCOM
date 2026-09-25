// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/directions.hpp"
#include "horcom/chart/rectification.hpp"
#include "horcom/chart/houses.hpp"
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

// a synthetic birth east of Greenwich, the longitude term of korr21 shows
SearchContext context(double lon = 11.3244, double lat = 48.1742) {
  SearchContext ctx;
  ctx.base.lon_deg_east = lon;
  ctx.base.lat_deg = lat;
  ctx.base.date_ut = {13, 10, 1992, 3, 0.0};
  ctx.settings.houses = HouseSystem::kPlacidus;
  ctx.vsop = &vsop();
  ctx.eph = &eph();
  return ctx;
}

Chart chart_at(double jd_ut, const SearchContext& ctx) {
  ChartInput in = ctx.base;
  in.date_ut = calendar_date(jd_ut, ctx.settings.calendar);
  return compute_chart(in, ctx.settings, vsop(), eph());
}

double off_arcsec(double a, double b) {
  return std::abs(fold_rad(a - b)) * kRadToDeg * kArcsecPerDeg;
}

}  // namespace

TEST_CASE("korr21 turns a local sidereal angle into the clock of the radix date") {
  const SearchContext ctx = context();
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  // the radix sidereal angle leads back to the radix moment itself
  CHECK(moment_for_armc(radix, radix.armc_deg * kDegToRad, ctx.base.lon_deg_east) ==
        doctest::Approx(radix.jd_ut).epsilon(1e-12).scale(1.0));
}

TEST_CASE("the STERNZEIT mode reads the local sidereal time like korr0") {
  const SearchContext ctx = context();
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  // half an hour of sidereal time more than the radix
  const double wanted = radix.armc_deg / kDegPerHour + 0.5;
  const CorrectionResult r = correct_birth_time(radix, {CorrectionTarget::kSiderealTime, wanted, 2}, ctx);
  REQUIRE(r.ok);
  const Chart c = chart_at(r.jd_ut, ctx);
  // the corrected chart carries that local sidereal time, the old port
  // took it for Greenwich and missed by the longitude, 45 minutes here
  CHECK(c.armc_deg / kDegPerHour == doctest::Approx(wanted).epsilon(1e-8));
  CHECK(std::abs(r.jd_ut - radix.jd_ut) * kHoursPerDay < 0.51);
}

TEST_CASE("the MC and AC modes put the angle on the wanted degree") {
  const SearchContext ctx = context();
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  const double mc = radix.houses.angles.mc + 2.0 * kDegToRad;
  const CorrectionResult rm = correct_birth_time(radix, {CorrectionTarget::kMc, mc, 2}, ctx);
  REQUIRE(rm.ok);
  CHECK(off_arcsec(chart_at(rm.jd_ut, ctx).houses.angles.mc, mc) < 0.5);
  const double ac = radix.houses.angles.ac - 3.0 * kDegToRad;
  const CorrectionResult ra = correct_birth_time(radix, {CorrectionTarget::kAc, ac, 2}, ctx);
  REQUIRE(ra.ok);
  CHECK_FALSE(ra.span_too_large);
  CHECK(off_arcsec(chart_at(ra.jd_ut, ctx).houses.angles.ac, ac) < 0.5);
}

TEST_CASE("the cusp mode walks the Placidus cusp whatever the chart uses") {
  SearchContext ctx = context();
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  const double h2 = radix.houses.cusp[2] + 1.5 * kDegToRad;
  const CorrectionResult r = correct_birth_time(radix, {CorrectionTarget::kCusp, h2, 2}, ctx);
  REQUIRE(r.ok);
  CHECK(off_arcsec(chart_at(r.jd_ut, ctx).houses.cusp[2], h2) < 0.5);
  // a Koch chart corrects on the Placidus cusp too, his plre
  ctx.settings.houses = HouseSystem::kKochGoh;
  const Chart koch = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  const CorrectionResult rk = correct_birth_time(koch, {CorrectionTarget::kCusp, h2, 2}, ctx);
  REQUIRE(rk.ok);
  CHECK(rk.jd_ut == doctest::Approx(r.jd_ut).epsilon(1e-10).scale(1.0));
}

TEST_CASE("the sun mode stays near the birth even for a far target") {
  const SearchContext ctx = context();
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  const double near = radix.b[body::kSun].el + 0.5 * kDegToRad;
  const CorrectionResult rn = correct_birth_time(radix, {CorrectionTarget::kSun, near, 2}, ctx);
  REQUIRE(rn.ok);
  CHECK(off_arcsec(chart_at(rn.jd_ut, ctx).b[body::kSun].el, near) < 1.0);
  CHECK(std::abs(rn.jd_ut - radix.jd_ut) < 1.0);
  // forty degrees ahead, his year check keeps it in the birth year
  const double far = radix.b[body::kSun].el + 40.0 * kDegToRad;
  const CorrectionResult rf = correct_birth_time(radix, {CorrectionTarget::kSun, far, 2}, ctx);
  REQUIRE(rf.ok);
  CHECK(rf.jd_ut > radix.jd_ut);
  CHECK(rf.jd_ut - radix.jd_ut < 45.0);
}

TEST_CASE("the moon mode finds the crossing next to the birth") {
  const SearchContext ctx = context();
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  // two degrees either side lie a few hours away, the old port searched
  // back from a month later and landed a whole lunation off
  for (const double step : {2.0, -2.0}) {
    CAPTURE(step);
    const double target = radix.b[body::kMoon].el + step * kDegToRad;
    const CorrectionResult r = correct_birth_time(radix, {CorrectionTarget::kMoon, target, 2}, ctx);
    REQUIRE(r.ok);
    CHECK(off_arcsec(chart_at(r.jd_ut, ctx).b[body::kMoon].el, target) < 1.0);
    CHECK(std::abs(r.jd_ut - radix.jd_ut) < 0.5);
    CHECK((r.jd_ut > radix.jd_ut) == (step > 0.0));
  }
}

TEST_CASE("the moon mode keeps the lunation across zero Aries") {
  SearchContext ctx = context();
  // walk the birth until the moon stands a few degrees past zero Aries
  double jd = julian_day(ctx.base.date_ut);
  for (int i = 0; i < 6; ++i) {
    const Chart c = chart_at(jd, ctx);
    jd -= fold_rad(c.b[body::kMoon].el - 3.0 * kDegToRad) / kTwoPi * kTropicalMonthDays;
  }
  ctx.base.date_ut = calendar_date(jd);
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  REQUIRE(radix.b[body::kMoon].el * kRadToDeg == doctest::Approx(3.0).epsilon(0.01));
  // ten degrees back sits across the zero point, about eighteen hours
  // before the birth, his unfolded seed looked a lunation earlier
  const double target = norm_rad(radix.b[body::kMoon].el - 10.0 * kDegToRad);
  const CorrectionResult r = correct_birth_time(radix, {CorrectionTarget::kMoon, target, 2}, ctx);
  REQUIRE(r.ok);
  CHECK(radix.jd_ut - r.jd_ut > 0.0);
  CHECK(radix.jd_ut - r.jd_ut < 1.5);
}

TEST_CASE("korr21 takes the sidereal day nearest the birth across midnight UT") {
  SearchContext ctx = context();
  ctx.base.date_ut = {13, 10, 1992, 23, 50.0};
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  // three degrees of MC ahead lie some twelve minutes later, past midnight
  const double mc = radix.houses.angles.mc + 3.0 * kDegToRad;
  const CorrectionResult r = correct_birth_time(radix, {CorrectionTarget::kMc, mc, 2}, ctx);
  REQUIRE(r.ok);
  CHECK(off_arcsec(chart_at(r.jd_ut, ctx).houses.angles.mc, mc) < 0.5);
  // his korr21 kept the UT date and answered 00:06 of the 13th, almost a
  // day early, the port gives 00:02 of the 14th
  CHECK(r.jd_ut > radix.jd_ut);
  CHECK((r.jd_ut - radix.jd_ut) * kMinutesPerDay < 20.0);
  CHECK(calendar_date(r.jd_ut).day == 14);
}

TEST_CASE("the cusp walk across zero keeps its side, his unfolded jump test turned it") {
  const SearchContext ctx = context();
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  const double lat = ctx.base.lat_deg;
  // a sidereal angle short of 360 degrees whose walk starts past zero, the
  // start of korr1 is the wanted cusp less (h - 10) thirty degrees
  int cusp = 0;
  double armc = 0.0;
  double wanted = 0.0;
  for (const int h : {11, 12, 2, 3}) {
    for (double a = 300.0; a < 360.0 && cusp == 0; a += 0.5) {
      const Houses hs = compute_houses(HouseSystem::kPlacidus, a * kDegToRad, lat, radix.ekls0);
      const double wh = hs.cusp[static_cast<std::size_t>(h)];
      const double ab1 = norm_rad(wh + (10 - h) * kPi / 6.0);
      // unfolded more than 2.5 radians apart, folded close together
      if (std::abs(ab1 - a * kDegToRad) > 2.5 && std::abs(fold_rad(ab1 - a * kDegToRad)) < 1.0) {
        cusp = h;
        armc = a;
        wanted = wh;
      }
    }
    if (cusp != 0) {
      break;
    }
  }
  REQUIRE(cusp != 0);
  CAPTURE(cusp);
  CAPTURE(armc);
  const CorrectionResult r = correct_birth_time(radix, {CorrectionTarget::kCusp, wanted, cusp}, ctx);
  REQUIRE(r.ok);
  const Chart c = chart_at(r.jd_ut, ctx);
  // his test flipped the found angle by 180 degrees, the cusp landed on
  // the opposite point twelve sidereal hours away
  CHECK(off_arcsec(c.houses.cusp[static_cast<std::size_t>(cusp)], wanted) < 1.0);
  CHECK(std::abs(fold_rad((c.armc_deg - armc) * kDegToRad)) * kRadToDeg < 0.01);
}

TEST_CASE("the moon mode takes the crossing ahead for a target forty degrees on") {
  const SearchContext ctx = context();
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  const double target = norm_rad(radix.b[body::kMoon].el + 40.0 * kDegToRad);
  const CorrectionResult r = correct_birth_time(radix, {CorrectionTarget::kMoon, target, 2}, ctx);
  REQUIRE(r.ok);
  CHECK(off_arcsec(chart_at(r.jd_ut, ctx).b[body::kMoon].el, target) < 1.0);
  // some three days after the birth, his seed on the far side found the
  // crossing of the lunation before, about twenty four days early
  CHECK(r.jd_ut - radix.jd_ut > 2.0);
  CHECK(r.jd_ut - radix.jd_ut < 4.5);
}

TEST_CASE("the prima variation turns the directed axes degree for degree") {
  const SearchContext ctx = context();
  const Chart radix = compute_chart(ctx.base, ctx.settings, vsop(), eph());
  const double lon = ctx.base.lon_deg_east;
  const double lat = ctx.base.lat_deg;
  const double event = radix.jd_ut + 30.0 * radix.ta.tropical_year_days;
  const DirectedAxes plain = direct_axes(radix, lon, lat, event, false, 0.0, HouseSystem::kPlacidus);
  const DirectedAxes varied = direct_axes(radix, lon, lat, event, false, 3.0, HouseSystem::kPlacidus);
  // his ADD brm,dif * 360 / tja turned three degrees into 2.957
  CHECK(varied.arc_deg - plain.arc_deg == doctest::Approx(3.0).epsilon(1e-9));
  // the radix taken over with the three degrees carries the same axes
  const Chart moved = chart_at(radix.jd_ut + 3.0 / (kDegPerCircle * kSolarToSiderealRate), ctx);
  const DirectedAxes after = direct_axes(moved, lon, lat, event, false, 0.0, HouseSystem::kPlacidus);
  CHECK(std::abs(fold_rad((after.armc_deg - varied.armc_deg) * kDegToRad)) * kRadToDeg < 1.0e-4);
}
