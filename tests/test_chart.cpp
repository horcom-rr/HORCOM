// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <algorithm>
#include <cctype>
#include <cmath>

#include "doctest.h"
#include "horcom/chart/chart.hpp"
#include "horcom/ephem/eclipses.hpp"
#include "horcom/ephem/elements.hpp"
#include "horcom/ephem/moon.hpp"
#include "horcom/ephem/sunmoon.hpp"
#include "horcom/chart/great_year.hpp"
#include "horcom/chart/planet_points.hpp"
#include "horcom/chart/stars.hpp"
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

TEST_CASE("the mean sidereal composite takes the near half of the day") {
  // sidereal times 23 h and 1 h apart by the midnight, the composite
  // ARMC belongs near zero, his linear mean put it at twelve hours
  Chart a;
  Chart b;
  a.ok = b.ok = true;
  a.hs = 23.0;
  b.hs = 1.0;
  a.smo.ekls = b.smo.ekls = 23.44 * kDegToRad;
  ChartInput ia;
  ChartInput ib;
  ia.lat_deg = ib.lat_deg = 48.0;
  const Chart m = composite_chart(a, ia, b, ib, CompositeHouses::kMeanSidereal, 48.0, {});
  double armc = norm_deg(m.armc_deg);
  if (armc > 180.0) {
    armc -= 360.0;
  }
  CHECK(std::abs(armc) < 1e-6);
  // a pair less than twelve hours apart keeps the plain mean
  a.hs = 5.0;
  b.hs = 9.0;
  CHECK(composite_chart(a, ia, b, ib, CompositeHouses::kMeanSidereal, 48.0, {}).armc_deg == doctest::Approx(105.0));
  // the local sidereal times decide, not the Greenwich ones. At 170 east
  // and 170 west the Greenwich times 1 h and 23 h give local ARMCs of 185
  // and 175, mean 180, his linear mean was right there
  ia.lon_deg_east = 170.0;
  ib.lon_deg_east = -170.0;
  a.hs = 1.0;
  b.hs = 23.0;
  CHECK(composite_chart(a, ia, b, ib, CompositeHouses::kMeanSidereal, 48.0, {}).armc_deg == doctest::Approx(180.0));
  // both at 6 h give local ARMCs of 260 and 280, mean 270, his linear
  // mean said 90
  a.hs = 6.0;
  b.hs = 6.0;
  CHECK(composite_chart(a, ia, b, ib, CompositeHouses::kMeanSidereal, 48.0, {}).armc_deg == doctest::Approx(270.0));
}

TEST_CASE("the equal house systems take the schematic composite like a13") {
  ChartInput ia;
  ia.date_ut = {13, 10, 1992, 3, 0.0};
  ia.lon_deg_east = 11.3244;
  ia.lat_deg = 48.1742;
  ChartInput ib;
  ib.date_ut = {1, 6, 1990, 12, 0.0};
  ib.lon_deg_east = 13.4;
  ib.lat_deg = 52.5;
  for (const HouseSystem hs : {HouseSystem::kEqualAsc, HouseSystem::kEqualVehlow}) {
    ChartSettings s;
    s.houses = hs;
    const Chart a = compute_chart(ia, s, vsop(), eph());
    const Chart b = compute_chart(ib, s, vsop(), eph());
    REQUIRE(a.ok);
    REQUIRE(b.ok);
    const Chart schematic = composite_chart(a, ia, b, ib, CompositeHouses::kSchematic, ia.lat_deg, s);
    //RR IF haw& = 6 OR haw& = 7 : CLR comp_hand!,comp_mstz!
    for (const CompositeHouses m : {CompositeHouses::kMeanSidereal, CompositeHouses::kRobertHand}) {
      const Chart c = composite_chart(a, ia, b, ib, m, ia.lat_deg, s);
      for (int k = 1; k <= 12; ++k) {
        CAPTURE(k);
        CHECK(c.houses.cusp[static_cast<std::size_t>(k)] ==
              doctest::Approx(schematic.houses.cusp[static_cast<std::size_t>(k)]));
      }
    }
  }
}

TEST_CASE("the composite AC point turns to the side of its first cusp") {
  // ACs at 5 and 200 degrees have their near midpoint at 282.5, the MC
  // midpoint of 270 and 300 puts the ROBERT HAND first cusp at 48 north
  // near 33 degrees. His a13 flipped the AC point to that side and then
  // wrote the unflipped midpoint back, the final program showed 282.5
  // beside the MC. The port keeps the flip his code was written for
  Chart a;
  Chart b;
  a.ok = b.ok = true;
  a.smo.ekls = b.smo.ekls = 23.44 * kDegToRad;
  for (Chart* c : {&a, &b}) {
    for (const int slot : {body::kAscendant, body::kMc}) {
      c->b[static_cast<std::size_t>(slot)].present = true;
      c->b[static_cast<std::size_t>(slot)].valid = true;
    }
  }
  a.b[body::kAscendant].el = 5.0 * kDegToRad;
  b.b[body::kAscendant].el = 200.0 * kDegToRad;
  a.b[body::kMc].el = 270.0 * kDegToRad;
  b.b[body::kMc].el = 300.0 * kDegToRad;
  ChartInput ia;
  ChartInput ib;
  const Chart h = composite_chart(a, ia, b, ib, CompositeHouses::kRobertHand, 48.0, {});
  REQUIRE(h.houses.ok);
  CHECK(norm_rad(h.houses.cusp[1]) * kRadToDeg == doctest::Approx(32.6).epsilon(0.01));
  // the original 282.5, the port 102.5 in the houses of the AC
  CHECK(h.b[body::kAscendant].el * kRadToDeg == doctest::Approx(102.5));
  // the MC point agrees with its cusp and stays
  CHECK(h.b[body::kMc].el * kRadToDeg == doctest::Approx(285.0));
}

TEST_CASE("the halbsmin midpoint takes the near side") {
  CHECK(midpoint_near(350.0 * kDegToRad, 10.0 * kDegToRad) == doctest::Approx(0.0).epsilon(1e-9));
  CHECK(midpoint_near(10.0 * kDegToRad, 50.0 * kDegToRad) == doctest::Approx(30.0 * kDegToRad));
  // pairs around zero Aries whose raw sum passes a full circle, the
  // original returned the far midpoints 200 and 185 here
  CHECK(midpoint_near(100.0 * kDegToRad, 300.0 * kDegToRad) == doctest::Approx(20.0 * kDegToRad));
  CHECK(midpoint_near(20.0 * kDegToRad, 350.0 * kDegToRad) == doctest::Approx(5.0 * kDegToRad));
  CHECK(midpoint_near(350.0 * kDegToRad, 20.0 * kDegToRad) == doctest::Approx(5.0 * kDegToRad));
  // no wrap, no change
  CHECK(midpoint_near(200.0 * kDegToRad, 250.0 * kDegToRad) == doctest::Approx(225.0 * kDegToRad));
  CHECK(midpoint_near(10.0 * kDegToRad, 300.0 * kDegToRad) == doctest::Approx(335.0 * kDegToRad));
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

TEST_CASE("the combin place stays on the side of the date line where the places lie") {
  // a group clear of the date line keeps his plain mean
  CHECK(mean_longitude({-75.0, 10.0}) == doctest::Approx(-32.5));
  CHECK(mean_longitude({0.0, 10.0, 50.0}) == doctest::Approx(20.0));
  CHECK(mean_longitude({-100.0, 30.0, 60.0}) == doctest::Approx(-10.0 / 3.0));
  // 170 east and 170 west, his glc / z& said 0, Greenwich
  CHECK(mean_longitude({170.0, -170.0}) == doctest::Approx(180.0));
  // his 53.33 for the three near the date line
  CHECK(mean_longitude({170.0, -170.0, 160.0}) == doctest::Approx(173.0 + 1.0 / 3.0));
  // Americas and Asia 160 apart over the Pacific, he gave 0
  CHECK(mean_longitude({-100.0, 100.0}) == doctest::Approx(180.0));
  // west of the date line the mean reads west
  CHECK(mean_longitude({-175.0, 165.0, -165.0}) == doctest::Approx(-178.0 - 1.0 / 3.0));
  ChartInput ia;
  ia.date_ut = {1, 1, 2000, 0, 0.0};
  ia.lon_deg_east = 170.0;
  ChartInput ib = ia;
  ib.lon_deg_east = -170.0;
  CHECK(std::abs(combin_input({ia, ib}, Calendar::kAuto).lon_deg_east) == doctest::Approx(180.0));
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
  // a degree of sidereal time variation turns the axes by a degree, his
  // ADD brm,dif * 360 / tja gave 0.9856
  const DirectedAxes varied = direct_axes(radix, in.lon_deg_east, in.lat_deg,
                                          radix.jd_ut + tja, false, 1.0, HouseSystem::kPlacidus);
  CHECK(varied.arc_deg - one_year.arc_deg == doctest::Approx(1.0));
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

TEST_CASE("the fixed stars land on their catalogue places") {
  ChartInput in;
  in.date_ut = {1, 1, 2000, 12, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  ChartSettings s;
  const Chart c = compute_chart(in, s, vsop(), eph());
  REQUIRE(c.ok);
  const std::vector<StarRow> rows = fixed_stars(c, 1.0);
  REQUIRE(rows.size() == 62);
  double regulus = 0.0;
  double aldebaran = 0.0;
  for (const StarRow& r : rows) {
    if (r.name == "REGULUS") {
      regulus = r.la * kRadToDeg;
    }
    if (r.name == "ALDEBARAN") {
      aldebaran = r.la * kRadToDeg;
    }
  }
  // the almanac puts Regulus near 29 degrees 50 Leo at the millennium
  // and Aldebaran near 9 degrees 47 Gemini
  CHECK(regulus == doctest::Approx(149.8).epsilon(0.005));
  CHECK(aldebaran == doctest::Approx(69.8).epsilon(0.005));
}

TEST_CASE("the mean planetary nodes and apsides fill their columns") {
  ChartInput in;
  in.date_ut = {1, 1, 2000, 12, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  const Chart c = compute_chart(in, {}, vsop(), eph());
  REQUIRE(c.ok);
  ChartSettings s;

  // the sun row, Landscheidt's solar equator node and the perigee
  const PlanetPoints sun = planet_points(c, body::kSun, s);
  REQUIRE(sun.ok);
  CHECK(sun.node * kRadToDeg == doctest::Approx(73.6667 + 150.0 * 0.01396).epsilon(0.001));
  CHECK(sun.perihelion * kRadToDeg == doctest::Approx(282.94).epsilon(0.001));
  CHECK(norm_rad(sun.aphelion - sun.perihelion) == doctest::Approx(kPi));

  // the moon row rides the mean lunar points
  const PlanetPoints moon = planet_points(c, body::kMoon, s);
  REQUIRE(moon.ok);
  CHECK(moon.node == doctest::Approx(norm_rad(c.lunar.mean_node)));
  CHECK(moon.aphelion == doctest::Approx(norm_rad(c.lunar.mean_apogee)));

  // heliocentric mode hands out the element directions, Mercury's node
  // near 48.33 degrees, Pluto's mean elements land on the known values
  s.heliocentric = true;
  const PlanetPoints me = planet_points(c, body::kMercury, s);
  REQUIRE(me.ok);
  CHECK(me.node * kRadToDeg == doctest::Approx(48.33).epsilon(0.001));
  CHECK(me.perihelion * kRadToDeg == doctest::Approx(77.46).epsilon(0.001));
  const PlanetPoints pl = planet_points(c, body::kPluto, s);
  REQUIRE(pl.ok);
  CHECK(pl.node * kRadToDeg == doctest::Approx(110.30).epsilon(0.001));
  CHECK(pl.perihelion * kRadToDeg == doctest::Approx(224.07).epsilon(0.01));

  // geocentric the node is the space point of the orbit crossing, the
  // projection must match the triangle formula exactly
  s.heliocentric = false;
  const Orbit orb = mean_elements(body::kMercury, c.ta);
  const double arg = orb.p - orb.o;
  const double rn = orb.a * (1.0 - orb.e * orb.e) / (1.0 + orb.e * std::cos(-arg));
  const BodyState& earth = c.b[body::kSun];
  const double y = rn * std::sin(orb.o) - earth.r * std::sin(earth.hel);
  const double x = rn * std::cos(orb.o) - earth.r * std::cos(earth.hel);
  const double want = norm_rad(atn(y, x + kEps) + c.smo.dpsi);
  const PlanetPoints geo = planet_points(c, body::kMercury, s);
  CHECK(geo.node == doctest::Approx(want).epsilon(1.0e-12));
  // the descending node is not the antipode of the ascending one, a
  // nearby space point can stand on the same side of the Earth
  CHECK(std::abs(norm_rad(geo.node_south - geo.node) - kPi) < kPi);
}

TEST_CASE("body name table matches the pl$ assignments of plnm") {
  // transcription pins against PROCEDURE plnm, uppercase display names
  CHECK(body::kName[body::kSun] == "SO");
  CHECK(body::kName[body::kMoon] == "MO");
  CHECK(body::kName[body::kMercury] == "ME");
  CHECK(body::kName[body::kPluto] == "PL");
  CHECK(body::kName[body::kNodeAsc] == "DR");
  CHECK(body::kName[body::kNodeDesc] == "DS");
  CHECK(body::kName[body::kAscendant] == "AC");
  CHECK(body::kName[body::kMc] == "MC");
  CHECK(body::kName[body::kApogee] == "AG");
  CHECK(body::kName[body::kChiron] == "CH");
  CHECK(body::kName[body::kQuaoar] == "QU");
  CHECK(body::kName[body::kXena] == "XE");
  CHECK(body::kName[body::kFixpunkt] == "FP");
  CHECK(body::kEarthName == "TE");
  // every name is the uppercase twin of the sprite tag, slot 0 aside
  for (int slot = 1; slot < body::kSlotCount; ++slot) {
    const std::string_view tag = body::kTag[static_cast<std::size_t>(slot)];
    const std::string_view name = body::kName[static_cast<std::size_t>(slot)];
    REQUIRE(tag.size() == name.size());
    for (std::size_t i = 0; i < tag.size(); ++i) {
      CHECK(static_cast<char>(std::toupper(static_cast<unsigned char>(tag[i]))) == name[i]);
    }
  }
}

TEST_CASE("the lunation series meets the true new moon of the year 1000") {
  // the conjunction found by bisection on the port's own VSOP87 sun and
  // ELP moon, independent of the lunation series. His finst_0 carried
  // 0.1017438 for the T squared term of the moon's anomaly where Meeus
  // prints 0.0107438, which put these new moons up to 86 minutes off.
  // The corrected series meets them within ten seconds
  const auto elongation = [](double jd_ut) {
    ChartInput in;
    in.date_ut = calendar_date(jd_ut, Calendar::kAuto);
    in.lon_deg_east = 0.0;
    in.lat_deg = 51.5;
    const Chart c = compute_chart(in, ChartSettings{}, vsop(), eph());
    return std::remainder(c.b[body::kMoon].el - c.b[body::kSun].el, kTwoPi);
  };
  const std::vector<Lunation> nm = lunations(julian_day({1, 6, 1000, 0, 0.0}, Calendar::kJulian), 6, false);
  REQUIRE(nm.size() == 6);
  double worst = 0.0;
  for (const Lunation& l : nm) {
    double lo = l.jd_ut - 0.5;
    double hi = l.jd_ut + 0.5;
    REQUIRE(elongation(lo) < 0.0);
    REQUIRE(elongation(hi) > 0.0);
    for (int i = 0; i < 40; ++i) {
      const double mid = 0.5 * (lo + hi);
      (elongation(mid) < 0.0 ? lo : hi) = mid;
    }
    worst = std::max(worst, std::abs(lo - l.jd_ut) * kMinutesPerDay);
  }
  CHECK(worst < 0.5);
}

TEST_CASE("the true node keeps its speed while it crosses zero Aries") {
  // his vel_om_pd tested IF w2 > w1 OR w2 < w1 + PI, always true, so the
  // retrograde fold never ran. An hour around the node's backward step
  // over zero Aries his speed read about +75 radians a day
  ChartSettings s;
  s.true_node = true;
  const auto node_at = [&s](double jd_ut) {
    ChartInput in;
    in.date_ut = calendar_date(jd_ut, Calendar::kAuto);
    in.lat_deg = 48.0;
    const Chart c = compute_chart(in, s, vsop(), eph());
    return c.b[body::kNodeAsc];
  };
  // the true node left Aries backwards around the turn of 2025, find the
  // day of the step
  double lo = julian_day({1, 6, 2024, 0, 0.0});
  double hi = lo;
  for (double jd = lo; jd < lo + 800.0; jd += 1.0) {
    if (node_at(jd).el < 1.0 && node_at(jd + 1.0).el > 5.0) {
      lo = jd;
      hi = jd + 1.0;
      break;
    }
  }
  REQUIRE(hi > lo);
  for (int i = 0; i < 30; ++i) {
    const double mid = 0.5 * (lo + hi);
    (node_at(mid).el < 1.0 ? lo : hi) = mid;
  }
  const BodyState at = node_at(lo);
  // the true node wanders by less than two degrees a day
  CHECK(std::abs(at.tb) * kRadToDeg < 2.0);
}

TEST_CASE("the star aspects of stelk count both sides of exact") {
  // his stelk accepted an opposition, square or trine only on the side
  // before exact, the conjunction on both. A point at 100 degrees with the
  // Sun half a degree before and after each aspect must hit every time
  Chart c;
  c.ok = true;
  BodyState& sun = c.b[body::kSun];
  sun.present = true;
  sun.valid = true;
  const double la = 100.0 * kDegToRad;
  const auto kinds = [&](double sun_deg) {
    sun.el = norm_rad(sun_deg * kDegToRad);
    std::string out;
    for (const auto& [slot, kind] : point_aspects(c, la, 1.0)) {
      if (slot == body::kSun) {
        out += kind;
      }
    }
    return out;
  };
  for (double side : {-0.3, 0.3}) {
    CHECK(kinds(100.0 + side) == "K");
    CHECK(kinds(280.0 + side) == "O");
    CHECK(kinds(190.0 + side) == "Q");
    CHECK(kinds(10.0 + side) == "Q");
    CHECK(kinds(220.0 + side) == "T");
    CHECK(kinds(340.0 + side) == "T");
  }
}

TEST_CASE("a body exactly on the star point still counts") {
  // his x > 0 dropped the exact conjunction and the exact aspects
  Chart c;
  c.ok = true;
  BodyState& sun = c.b[body::kSun];
  sun.present = true;
  sun.valid = true;
  sun.el = 100.0 * kDegToRad;
  bool conj = false;
  for (const auto& [slot, kind] : point_aspects(c, 100.0 * kDegToRad, 1.0)) {
    conj = conj || (slot == body::kSun && kind == 'K');
  }
  CHECK(conj);
}

TEST_CASE("the osculating apogee latitude belongs to the apogee, not to the Moon") {
  // his eb(nk&(1)) = ASIN(SIN(u4) * SIN(i4)) took the Moon's own argument of
  // latitude. The eccentricity vector of the same state points to the
  // perigee, its opposite is the apogee in space
  ChartInput in;
  in.date_ut = {12, 4, 1992, 0, 0.0};
  in.lon_deg_east = 11.0;
  in.lat_deg = 48.0;
  ChartSettings s;
  s.enable_standard_extras();
  s.true_apogee = true;
  const Chart chart = compute_chart(in, s, vsop(), eph());
  REQUIRE(chart.ok);
  const MoonPosition& m = chart.moon;
  const double mu = 0.0002959122083 * 3.0404332e-06;
  const double r = std::sqrt(m.x[0] * m.x[0] + m.x[1] * m.x[1] + m.x[2] * m.x[2]);
  const double h[3] = {m.x[1] * m.v[2] - m.x[2] * m.v[1], m.x[2] * m.v[0] - m.x[0] * m.v[2],
                       m.x[0] * m.v[1] - m.x[1] * m.v[0]};
  double e[3];
  e[0] = (m.v[1] * h[2] - m.v[2] * h[1]) / mu - m.x[0] / r;
  e[1] = (m.v[2] * h[0] - m.v[0] * h[2]) / mu - m.x[1] / r;
  e[2] = (m.v[0] * h[1] - m.v[1] * h[0]) / mu - m.x[2] / r;
  const double en = std::sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
  const double apogee_lat = std::asin(-e[2] / en);
  CHECK(chart.lunar.true_apogee_lat == doctest::Approx(apogee_lat).epsilon(1e-6));
  // the Moon's own latitude, what the original printed, lies elsewhere
  CHECK(std::abs(chart.lunar.true_apogee_lat - m.eb) > 1.0 * kDegToRad);
}

TEST_CASE("the great year age point runs back with the precession like grossj1") {
  //RR jdgross=2370832, CHAUVIN f.AQU.
  constexpr double kRef = 2370832.0;
  constexpr double kTja = 365.2422;
  const double eps = 23.44 * kDegToRad;
  // one Julian century after the reference the start of the age gained
  // Newcomb's general precession of some 5026 arcseconds, the age point
  // runs back by that
  const GreatYearPoint aqu = great_year_point(kRef + kDaysPerCentury, kTja, eps, kRef, 330);
  CHECK(aqu.di_deg == doctest::Approx(-5026.0 / 3600.0).epsilon(0.002));
  CHECK(aqu.point_deg == doctest::Approx(330.0 + aqu.di_deg));
  CHECK_FALSE(aqu.outside);
  // before the reference the age has not begun, 2300 years on the point
  // left the sign
  CHECK(great_year_point(kRef - kDaysPerCentury / 10.0, kTja, eps, kRef, 330).outside);
  CHECK(great_year_point(kRef + 23.0 * kDaysPerCentury, kTja, eps, kRef, 330).outside);
  // his FISCHE start compared 360 degrees with a normalised longitude, di
  // came out as 358.6 and the warning fired on every date. The fold gives
  // the same run back as the other ages
  const GreatYearPoint psc = great_year_point(kRef + kDaysPerCentury, kTja, eps, kRef, 360);
  CHECK(std::abs(psc.di_deg - aqu.di_deg) < 0.01);
  CHECK(psc.point_deg == doctest::Approx(360.0 + psc.di_deg));
  CHECK_FALSE(psc.outside);
  CHECK(great_year_point(kRef - kDaysPerCentury / 10.0, kTja, eps, kRef, 360).outside);
}

TEST_CASE("the Wahr node row of the coordinate table stands at ET") {
  // his ko_ta ran moko for the Wahr rows after etut had set jd back to
  // UT, around -1000 the true apogee came out half a degree off
  ChartInput in;
  in.date_ut = {1, 6, -1000, 12, 0.0};
  in.lon_deg_east = 11.5;
  in.lat_deg = 48.0;
  ChartSettings s;
  s.true_node = true;
  const Chart c = compute_chart(in, s, vsop(), eph());
  REQUIRE(c.ok);
  const TimeArguments t = time_arguments(c.jd_ut);
  const SunMoonState st = somo(t, calendar_date(c.jd_ut, s.calendar));
  const LunarPoints at_ut = lunar_points(moon_position(t, st), st, t);
  const double off = std::abs(std::remainder(c.lunar.true_node - at_ut.true_node, kTwoPi)) * kRadToDeg;
  const double off_ag = std::abs(std::remainder(c.lunar.true_apogee - at_ut.true_apogee, kTwoPi)) * kRadToDeg;
  CHECK(off > 1.0e-3);
  CHECK(off_ag > 0.3);
  CHECK(c.b[body::kNodeAsc].el == doctest::Approx(c.lunar.true_node));
  // the rates of vel_om_pd come for both variants whatever the setting
  const LunarRates r = lunar_rates(c, s.calendar);
  CHECK(r.node_tb * kRadToDeg * 60.0 > -30.0);
  CHECK(r.node_tb * kRadToDeg * 60.0 < 30.0);
  CHECK(c.b[body::kNodeAsc].tb == doctest::Approx(r.node_tb));
}
