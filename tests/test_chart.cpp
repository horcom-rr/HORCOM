// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/chart.hpp"
#include "horcom/chart/composite.hpp"
#include "horcom/chart/directions.hpp"
#include "horcom/chart/mundane.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/delta_t.hpp"

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

double deg_dist(double a_rad, double b_deg) {
  return std::abs(std::remainder(a_rad * kRadToDeg - b_deg, 360.0));
}

}  // namespace

TEST_CASE("the Sun crosses zero Aries at the 2000 March equinox") {
  // the equinox was 2000-03-20 07:35 UT, the apparent longitude must
  // vanish there to well under a hundredth of a degree
  ChartInput in;
  in.date_ut = {20, 3, 2000, 7, 35.0};
  in.lon_deg_east = 0.0;
  in.lat_deg = 51.5;
  const Chart c = compute_chart(in, {}, vsop(), eph());
  REQUIRE(c.ok);
  const double lon = c.b[body::kSun].el * kRadToDeg;
  CHECK(std::min(lon, 360.0 - lon) < 0.01);
}

TEST_CASE("a full chart for 1992-10-13 0h TD carries the Meeus anchors") {
  // pick the UT so that the pipeline's ET lands on the book epoch
  const double jd_et_target = 2448908.5;
  const double delt = delta_t_minutes(jd_et_target);
  const double jd_ut = jd_et_target - delt * kDeltaTDaysPerMinute;
  const CalendarDate d = calendar_date(jd_ut);
  ChartInput in;
  in.date_ut = d;
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  ChartSettings s;
  s.enable_standard_extras();
  const Chart c = compute_chart(in, s, vsop(), eph());
  REQUIRE(c.ok);
  CHECK(c.jd_et == doctest::Approx(jd_et_target).epsilon(1e-9));
  // the apparent Sun of the Meeus example, 199.90605 degrees within the
  // truncated series' tolerance
  CHECK(deg_dist(c.b[body::kSun].el, 199.90605) < 0.001);
  // heliocentric Pluto from his integrated file
  CHECK(c.b[body::kPluto].valid);
  CHECK(c.b[body::kPluto].r == doctest::Approx(29.711111).epsilon(2e-5));
  // Chiron rides along as an extra body
  const int chiron = s.nk[2];
  CHECK(c.b[static_cast<std::size_t>(chiron)].present);
  CHECK(c.b[static_cast<std::size_t>(chiron)].valid);
  CHECK(c.b[static_cast<std::size_t>(chiron)].r > 8.4);
  CHECK(c.b[static_cast<std::size_t>(chiron)].r < 18.9);
  // Mars stood weeks before its retrograde loop of late 1992, Venus ran
  // direct, both daily motions must sign accordingly
  CHECK(c.b[body::kVenus].tb > 0.0);
  CHECK(c.b[body::kMars].tb > 0.0);
  // angles are present on the body slots like pl(13) and pl(14)
  CHECK(c.b[body::kAscendant].el == doctest::Approx(c.houses.angles.ac));
  CHECK(c.b[body::kMc].el == doctest::Approx(c.houses.angles.mc));
}

TEST_CASE("Mars runs retrograde at the end of December 1992") {
  ChartInput in;
  in.date_ut = {28, 12, 1992, 0, 0.0};
  const Chart c = compute_chart(in, {}, vsop(), eph());
  REQUIRE(c.ok);
  CHECK(c.b[body::kMars].tb < 0.0);
  CHECK(c.b[body::kJupiter].tb > 0.0);
}

TEST_CASE("the Moon of the pipeline meets the Meeus example") {
  const double jd_et_target = 2448724.5;
  const double delt = delta_t_minutes(jd_et_target);
  const double jd_ut = jd_et_target - delt * kDeltaTDaysPerMinute;
  ChartInput in;
  in.date_ut = calendar_date(jd_ut);
  const Chart c = compute_chart(in, {}, vsop(), eph());
  REQUIRE(c.ok);
  // apparent longitude 133.167265 degrees, geometric plus his nutation
  CHECK(deg_dist(c.b[body::kMoon].el, 133.167265) < 0.001);
  CHECK(c.b[body::kMoon].dr * kKmPerAu == doctest::Approx(368409.7).epsilon(1e-5));
}

TEST_CASE("the Part of Fortune follows the day and night formula") {
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  ChartSettings s;
  s.enable_standard_extras();
  const Chart c = compute_chart(in, s, vsop(), eph());
  REQUIRE(c.ok);
  const int gl = s.nk[4];
  const BodyState& b = c.b[static_cast<std::size_t>(gl)];
  REQUIRE(b.present);
  const double ac = c.houses.angles.ac;
  const double day = norm_rad(ac + c.b[body::kMoon].el - c.b[body::kSun].el);
  const double night = norm_rad(ac + c.b[body::kSun].el - c.b[body::kMoon].el);
  const bool matches_one = std::abs(std::remainder(b.el - day, kTwoPi)) < 1e-12 ||
                           std::abs(std::remainder(b.el - night, kTwoPi)) < 1e-12;
  CHECK(matches_one);
}

TEST_CASE("his parallax against an independent vector computation") {
  // the protected cluster. An observer on the spherical Earth of the
  // original displaces the body by the vector difference, the declination
  // arm of his formulation is algebraically exact, so an independent
  // cartesian computation must agree to machine precision.
  ChartInput in;
  in.date_ut = {12, 4, 1992, 21, 30.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  ChartSettings plain;
  ChartSettings topo;
  topo.topocentric_parallax = true;
  const Chart geo = compute_chart(in, plain, vsop(), eph());
  const Chart top = compute_chart(in, topo, vsop(), eph());
  REQUIRE(geo.ok);
  REQUIRE(top.ok);

  const BodyState& m0 = geo.b[body::kMoon];
  const BodyState& m1 = top.b[body::kMoon];
  const double ro = 0.000042634515;
  const double g = geo.b[body::kMoon].dr;
  const double phi = in.lat_deg * kDegToRad;
  const double lst = geo.armc_deg * kDegToRad;
  const double x = g * std::cos(m0.de) * std::cos(m0.ar) - ro * std::cos(phi) * std::cos(lst);
  const double y = g * std::cos(m0.de) * std::sin(m0.ar) - ro * std::cos(phi) * std::sin(lst);
  const double z = g * std::sin(m0.de) - ro * std::sin(phi);
  const double ar_ref = norm_rad(std::atan2(y, x));
  const double de_ref = std::atan(z / std::sqrt(x * x + y * y));
  CHECK(std::abs(std::remainder(m1.ar - ar_ref, kTwoPi)) < 1e-9);
  CHECK(m1.de == doctest::Approx(de_ref).epsilon(1e-9));

  // the displacement magnitude stays inside the horizontal parallax
  const double shift = std::abs(std::remainder(m1.el - m0.el, kTwoPi));
  CHECK(shift > 0.0);
  CHECK(shift < geo.moon.parallax * 1.05);

  // the Sun moves by arcseconds only
  const double sun_shift = std::abs(std::remainder(top.b[body::kSun].el - geo.b[body::kSun].el, kTwoPi));
  CHECK(sun_shift * kRadToDeg * 3600.0 < 15.0);
  CHECK(sun_shift * kRadToDeg * 3600.0 > 1.0);

  // nodes stay untouched, the original applies par to slots 1 to 10 only
  CHECK(top.b[body::kNodeAsc].el == doctest::Approx(geo.b[body::kNodeAsc].el));
}

TEST_CASE("true node and apogee switches change the slots as configured") {
  ChartInput in;
  in.date_ut = {12, 4, 1992, 21, 30.0};
  ChartSettings mean;
  mean.enable_standard_extras();
  ChartSettings truem = mean;
  truem.true_node = true;
  truem.true_apogee = true;
  const Chart cm = compute_chart(in, mean, vsop(), eph());
  const Chart ct = compute_chart(in, truem, vsop(), eph());
  REQUIRE(cm.ok);
  REQUIRE(ct.ok);
  CHECK(cm.b[body::kNodeAsc].el == doctest::Approx(cm.lunar.mean_node));
  CHECK(ct.b[body::kNodeAsc].el == doctest::Approx(ct.lunar.true_node));
  // the true node needs a real speed from the symmetric hour
  CHECK(ct.b[body::kNodeAsc].tb != doctest::Approx(cm.b[body::kNodeAsc].tb));
  const int ag = mean.nk[1];
  CHECK(cm.b[static_cast<std::size_t>(ag)].el == doctest::Approx(cm.lunar.mean_apogee));
  CHECK(ct.b[static_cast<std::size_t>(ag)].el == doctest::Approx(ct.lunar.true_apogee));
}

TEST_CASE("the composite midpoints two charts like a13") {
  ChartInput ia;
  ia.date_ut = {13, 10, 1992, 3, 0.0};
  ia.lon_deg_east = 11.3244;
  ia.lat_deg = 48.1742;
  ChartInput ib;
  ib.date_ut = {1, 6, 1990, 12, 0.0};
  ib.lon_deg_east = 11.3244;
  ib.lat_deg = 48.1742;
  const Chart a = compute_chart(ia, {}, vsop(), eph());
  const Chart b = compute_chart(ib, {}, vsop(), eph());
  REQUIRE(a.ok);
  REQUIRE(b.ok);
  const Chart c = composite_chart(a, ia, b, ib, CompositeHouses::kSchematic, ia.lat_deg, {});
  REQUIRE(c.ok);
  // the sun sits on the near side midpoint
  CHECK(c.b[body::kSun].el == doctest::Approx(midpoint_near(a.b[body::kSun].el, b.b[body::kSun].el)));
  // the south node follows the north by half a circle
  CHECK(norm_rad(c.b[body::kNodeDesc].el - c.b[body::kNodeAsc].el) == doctest::Approx(kPi));
  // opposite cusps stay opposite
  CHECK(norm_rad(c.houses.cusp[7] - c.houses.cusp[1]) == doctest::Approx(kPi));
  CHECK(norm_rad(c.houses.cusp[4] - c.houses.cusp[10]) == doctest::Approx(kPi));
  // the mean sidereal mode computes real houses at the mean place
  const Chart m = composite_chart(a, ia, b, ib, CompositeHouses::kMeanSidereal, ia.lat_deg, {});
  CHECK(m.houses.ok);
  CHECK(m.houses.cusp[1] == doctest::Approx(m.houses.angles.ac));
  // the Robert Hand armc reproduces the MC midpoint through the houses
  const Chart h = composite_chart(a, ia, b, ib, CompositeHouses::kRobertHand, ia.lat_deg, {});
  REQUIRE(h.houses.ok);
  const double want = midpoint_near(a.b[body::kMc].el, b.b[body::kMc].el);
  double dmc = std::abs(norm_rad(h.houses.angles.mc) - norm_rad(want));
  if (dmc > kPi) {
    dmc = kTwoPi - dmc;
  }
  CHECK(dmc < 1e-6);
}

TEST_CASE("the halbsmin midpoint takes the near side") {
  CHECK(midpoint_near(350.0 * kDegToRad, 10.0 * kDegToRad) == doctest::Approx(0.0).epsilon(1e-9));
  CHECK(midpoint_near(10.0 * kDegToRad, 50.0 * kDegToRad) == doctest::Approx(30.0 * kDegToRad));
}

TEST_CASE("the combin averages moment and place like a14") {
  ChartInput ia;
  ia.date_ut = {13, 10, 1992, 3, 0.0};
  ia.lon_deg_east = 10.0;
  ia.lat_deg = 48.0;
  ChartInput ib;
  ib.date_ut = {13, 10, 1994, 3, 0.0};
  ib.lon_deg_east = 12.0;
  ib.lat_deg = 50.0;
  const ChartInput c = combin_input({ia, ib}, Calendar::kAuto);
  CHECK(julian_day(c.date_ut) == doctest::Approx((julian_day(ia.date_ut) + julian_day(ib.date_ut)) / 2.0));
  CHECK(c.lon_deg_east == doctest::Approx(11.0));
  CHECK(c.lat_deg == doctest::Approx(49.0));
}

TEST_CASE("the directed axes turn at his naibod style rate") {
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  const Chart radix = compute_chart(in, {}, vsop(), eph());
  REQUIRE(radix.ok);
  const double tja = radix.ta.tropical_year_days;
  const DirectedAxes one_year = direct_axes(radix, in.lon_deg_east, in.lat_deg,
                                            radix.jd_ut + tja, false, 0.0, HouseSystem::kPlacidus);
  // one tropical year turns the axes by 360 over the year length
  CHECK(one_year.arc_deg == doctest::Approx(360.0 / tja));
  CHECK(one_year.armc_deg == doctest::Approx(norm_deg(one_year.arm_deg + one_year.arc_deg)));
  CHECK(one_year.houses.ok);
  const DirectedAxes back = direct_axes(radix, in.lon_deg_east, in.lat_deg,
                                        radix.jd_ut + tja, true, 0.0, HouseSystem::kPlacidus);
  CHECK(back.arc_deg == doctest::Approx(-360.0 / tja));
  // a degree of sidereal time variation adds at the same rate
  const DirectedAxes varied = direct_axes(radix, in.lon_deg_east, in.lat_deg,
                                          radix.jd_ut + tja, false, 1.0, HouseSystem::kPlacidus);
  CHECK(varied.arc_deg - one_year.arc_deg == doctest::Approx(360.0 / tja));
}

TEST_CASE("the mundane longitude follows the semi arc quadrants") {
  // a flat sky, obliquity zero and bodies on the equator, makes the
  // proportion linear and the quadrants readable
  const double armcb = 0.0;
  CHECK(mundane_longitude(100.0 * kDegToRad, kEps, 0.0, armcb, 48.0) == doctest::Approx(10.0 * kDegToRad).epsilon(1e-6));
  CHECK(mundane_longitude(30.0 * kDegToRad, kEps, 0.0, armcb, 48.0) == doctest::Approx(300.0 * kDegToRad).epsilon(1e-6));
  CHECK(mundane_longitude(200.0 * kDegToRad, kEps, 0.0, armcb, 48.0) == doctest::Approx(110.0 * kDegToRad).epsilon(1e-6));
  // the ascendant lands at zero on the circle
  const double ac = mundane_longitude(90.0 * kDegToRad, kEps, 0.0, armcb, 48.0);
  CHECK(std::min(ac, kTwoPi - ac) < 1e-6);
}

TEST_CASE("the mundane chart carries the equal grid of mundhorh") {
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  Chart c = compute_chart(in, {}, vsop(), eph());
  REQUIRE(c.ok);
  to_mundane(c, in.lat_deg);
  CHECK(c.houses.cusp[1] == doctest::Approx(kEps));
  CHECK(c.houses.cusp[10] == doctest::Approx(kEps + 9.0 * kPi / 6.0));
  CHECK(c.b[body::kAscendant].el == doctest::Approx(c.houses.cusp[1]));
  for (int t = 1; t <= 10; ++t) {
    const double v = c.b[static_cast<std::size_t>(t)].el;
    CHECK(v >= 0.0);
    CHECK(v < kTwoPi);
  }
}

TEST_CASE("the heliocentric mode of hrg puts the earth on the moon slot") {
  const double jd_et_target = 2448908.5;
  const double delt = delta_t_minutes(jd_et_target);
  const double jd_ut = jd_et_target - delt * kDeltaTDaysPerMinute;
  ChartInput in;
  in.date_ut = calendar_date(jd_ut);
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  ChartSettings helio;
  helio.heliocentric = true;
  helio.enable_standard_extras();
  const Chart h = compute_chart(in, helio, vsop(), eph());
  REQUIRE(h.ok);
  // slot one stays empty like aa at two
  CHECK_FALSE(h.b[body::kSun].present);
  // the Meeus example puts the geometric earth at 19.907372 degrees
  CHECK(deg_dist(h.b[body::kMoon].el, 19.907372) < 0.001);
  // the planets keep their own sun centred state
  ChartSettings geo;
  geo.enable_standard_extras();
  const Chart g = compute_chart(in, geo, vsop(), eph());
  CHECK(h.b[body::kMars].el == doctest::Approx(norm_rad(g.b[body::kMars].hel)));
  CHECK(h.b[body::kMars].eb == doctest::Approx(g.b[body::kMars].heb));
  CHECK(h.b[body::kMars].dr == doctest::Approx(g.b[body::kMars].r));
  const int chiron = geo.nk[2];
  CHECK(h.b[static_cast<std::size_t>(chiron)].el ==
        doctest::Approx(norm_rad(g.b[static_cast<std::size_t>(chiron)].hel)));
  // the geocentric ideas stay out
  CHECK_FALSE(h.b[body::kNodeAsc].present);
  CHECK_FALSE(h.b[static_cast<std::size_t>(geo.nk[1])].present);
  CHECK_FALSE(h.b[static_cast<std::size_t>(geo.nk[4])].present);
  // hrg knows no houses, horg11 and bes111 stay dark
  CHECK_FALSE(h.b[body::kAscendant].present);
  CHECK_FALSE(h.b[body::kMc].present);
  CHECK(h.houses.cusp[1] == 0.0);
  // the sidereal chain still runs, the banner keeps its armc
  CHECK(h.armc_deg == doctest::Approx(g.armc_deg));
}
