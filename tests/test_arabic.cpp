// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "doctest.h"
#include "horcom/chart/arabic.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

using namespace horcom;

namespace {

Chart synthetic() {
  Chart c;
  c.ok = true;
  const std::initializer_list<std::pair<int, double>> bodies = {
      {body::kSun, 15.0},  {body::kMoon, 95.0},  {body::kMercury, 200.0}, {body::kVenus, 250.0},
      {body::kMars, 130.0}, {body::kJupiter, 300.0}, {body::kSaturn, 220.0}, {body::kUranus, 33.0},
      {body::kNeptune, 275.0}, {body::kPluto, 210.0}, {body::kAscendant, 10.0}, {body::kMc, 280.0}};
  for (const auto& [slot, deg] : bodies) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  }
  c.houses.ok = true;
  const double cusps[12] = {10.0, 40.0, 70.0, 100.0, 130.0, 160.0, 190.0, 220.0, 250.0, 280.0, 310.0, 340.0};
  for (int k = 1; k <= 12; ++k) {
    c.houses.cusp[static_cast<std::size_t>(k)] = cusps[k - 1] * kDegToRad;
  }
  return c;
}

double deg(double rad) {
  return norm_rad(rad) * kRadToDeg;
}

}  // namespace

TEST_CASE("the arabic parts follow the base plus first minus second rule") {
  const Chart c = synthetic();
  // this chart is a night birth, the sun below the ascendant axis, so
  // the traditional formula swaps the moving terms
  const std::vector<ArabicPart> parts = arabic_parts(c, ArabicFormula::kTraditional);
  REQUIRE(parts.size() == 37);
  CHECK(parts[6].name == "Glück");
  CHECK(deg(parts[6].la) == doctest::Approx(290.0));
  // forced day birth reads the plain direction
  const std::vector<ArabicPart> day = arabic_parts(c, ArabicFormula::kAlwaysDay);
  CHECK(deg(day[6].la) == doctest::Approx(90.0));
  // the wealth point adds the ruler of the second house to the
  // ascendant either way, its second term is empty
  const double hv2 = c.b[body::kVenus].el;
  CHECK(deg(parts[24].la) == doctest::Approx(deg(c.houses.cusp[1] + hv2)));
  CHECK(deg(day[24].la) == doctest::Approx(deg(c.houses.cusp[1] + hv2)));
  // the point of captivity rides on the already built point of luck
  CHECK(deg(parts[34].la) == doctest::Approx(deg(c.houses.cusp[1] + c.b[body::kSaturn].el - parts[6].la)));
}
