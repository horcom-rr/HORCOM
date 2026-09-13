// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>
#include <initializer_list>

#include "doctest.h"
#include "horcom/chart/houses.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

using namespace horcom;

namespace {

constexpr double kEkls = 23.4393 * kDegToRad;

// textbook angles from standard atan2 formulas, independent of the port's
// atn plumbing
double textbook_ac(double armcb, double lat_rad, double ekls) {
  return norm_rad(std::atan2(std::cos(armcb), -(std::sin(ekls) * std::tan(lat_rad) + std::cos(ekls) * std::sin(armcb))));
}

double textbook_mc(double armcb, double ekls) {
  return norm_rad(std::atan2(std::sin(armcb), std::cos(armcb) * std::cos(ekls)));
}

}  // namespace

TEST_CASE("angles agree with the textbook formulas") {
  for (double armc_deg : {10.0, 100.0, 190.0, 280.0}) {
    for (double lat : {-52.0, 0.0, 48.1}) {
      CAPTURE(armc_deg);
      CAPTURE(lat);
      const double armcb = armc_deg * kDegToRad;
      const Angles a = chart_angles(armcb, lat, kEkls);
      CHECK(a.ac == doctest::Approx(textbook_ac(armcb, lat * kDegToRad, kEkls)).epsilon(1e-10));
      CHECK(a.mc == doctest::Approx(textbook_mc(armcb, kEkls)).epsilon(1e-10));
      CHECK(norm_rad(a.dc - a.ac) == doctest::Approx(kPi));
      CHECK(norm_rad(a.ic - a.mc) == doctest::Approx(kPi));
    }
  }
}

TEST_CASE("quadrant systems share the angles and mirror their cusps") {
  const double armcb = 130.0 * kDegToRad;
  const double lat = 48.1;
  for (HouseSystem sys : {HouseSystem::kPlacidus, HouseSystem::kTopocentric, HouseSystem::kKochGoh,
                          HouseSystem::kRegiomontanus, HouseSystem::kCampanus}) {
    CAPTURE(static_cast<int>(sys));
    const Houses h = compute_houses(sys, armcb, lat, kEkls);
    REQUIRE(h.ok);
    CHECK(h.cusp[1] == doctest::Approx(h.angles.ac));
    CHECK(h.cusp[10] == doctest::Approx(h.angles.mc));
    for (int i = 1; i <= 6; ++i) {
      CAPTURE(i);
      const double a = h.cusp[static_cast<std::size_t>(i)];
      const double b = h.cusp[static_cast<std::size_t>(i + 6)];
      CHECK(std::abs(std::remainder(b - a - kPi, kTwoPi)) < 1e-9);
    }
    // the twelve cusps march counter clockwise
    for (int i = 1; i <= 11; ++i) {
      CAPTURE(i);
      const double step = norm_rad(h.cusp[static_cast<std::size_t>(i + 1)] - h.cusp[static_cast<std::size_t>(i)]);
      CHECK(step > 0.0);
      CHECK(step < kPi);
    }
  }
}

TEST_CASE("at the equator every quadrant system nearly meets Regiomontanus") {
  // with latitude zero the semi arc and great circle constructions
  // coincide up to the systems' small definitional differences
  const double armcb = 200.0 * kDegToRad;
  const Houses reg = compute_houses(HouseSystem::kRegiomontanus, armcb, 0.0, kEkls);
  const Houses pla = compute_houses(HouseSystem::kPlacidus, armcb, 0.0, kEkls);
  const Houses cam = compute_houses(HouseSystem::kCampanus, armcb, 0.0, kEkls);
  for (int i : {2, 3, 11, 12}) {
    CAPTURE(i);
    const auto idx = static_cast<std::size_t>(i);
    CHECK(std::abs(std::remainder(pla.cusp[idx] - reg.cusp[idx], kTwoPi)) * kRadToDeg < 0.51);
    CHECK(std::abs(std::remainder(cam.cusp[idx] - reg.cusp[idx], kTwoPi)) * kRadToDeg < 0.51);
  }
}

TEST_CASE("equal systems run in exact thirty degree steps") {
  const double armcb = 310.0 * kDegToRad;
  const Houses h = compute_houses(HouseSystem::kEqualAsc, armcb, 30.0, kEkls);
  REQUIRE(h.ok);
  CHECK(h.cusp[1] == doctest::Approx(h.angles.ac));
  for (int i = 1; i <= 11; ++i) {
    const double step = norm_rad(h.cusp[static_cast<std::size_t>(i + 1)] - h.cusp[static_cast<std::size_t>(i)]);
    CHECK(step == doctest::Approx(kPi / 6.0));
  }
  const Houses v = compute_houses(HouseSystem::kEqualVehlow, armcb, 30.0, kEkls);
  CHECK(norm_rad(h.cusp[1] - v.cusp[1]) == doctest::Approx(kPi / 12.0));
}

TEST_CASE("the maxbreit guard refuses polar latitudes for Placidus and Koch") {
  const double armcb = 100.0 * kDegToRad;
  CHECK_FALSE(compute_houses(HouseSystem::kPlacidus, armcb, 68.0, kEkls).ok);
  CHECK_FALSE(compute_houses(HouseSystem::kKochGoh, armcb, -68.0, kEkls).ok);
  CHECK(compute_houses(HouseSystem::kRegiomontanus, armcb, 68.0, kEkls).ok);
  CHECK(compute_houses(HouseSystem::kPlacidus, armcb, 66.0, kEkls).ok);
}

TEST_CASE("houses off leaves the requested cusps at zero") {
  const double armcb = 100.0 * kDegToRad;
  const Houses h8 = compute_houses(HouseSystem::kAcMcOnly, armcb, 48.0, kEkls);
  CHECK(h8.cusp[2] == 0.0);
  CHECK(h8.cusp[1] == doctest::Approx(h8.angles.ac));
  const Houses h9 = compute_houses(HouseSystem::kNone, armcb, 48.0, kEkls);
  CHECK(h9.cusp[1] == 0.0);
  CHECK(h9.cusp[10] == 0.0);
}
