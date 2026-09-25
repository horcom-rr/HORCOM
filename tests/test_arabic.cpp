// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <filesystem>
#include <fstream>

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

TEST_CASE("own arabic points write his records and replace the first rows") {
  const auto dir = std::filesystem::temp_directory_path() / "horcom_arabtei";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  // AC + SO - MO and H4 + Hv10 - 105 degrees
  REQUIRE(append_own_arabic_name(dir, 0, "EIGENER PUNKT", "TEST"));
  REQUIRE(append_own_arabic_term(dir, 0, 1, {0, body::kAscendant}));
  REQUIRE(append_own_arabic_term(dir, 0, 2, {0, body::kSun}));
  REQUIRE(append_own_arabic_term(dir, 0, 3, {0, body::kMoon}));
  REQUIRE(append_own_arabic_name(dir, 1, "ZWEITER", ""));
  REQUIRE(append_own_arabic_term(dir, 1, 1, {12, 4}));
  REQUIRE(append_own_arabic_term(dir, 1, 2, {13, 10}));
  REQUIRE(append_own_arabic_term(dir, 1, 3, {14, 105}));
  // his 39 and 16 byte records
  CHECK(std::filesystem::file_size(dir / "arabtei1.int") == 2 * 39);
  CHECK(std::filesystem::file_size(dir / "arabtei2.int") == 6 * 16);
  std::ifstream raw(dir / "arabtei2.int", std::ios::binary);
  std::string first(16, ' ');
  raw.read(first.data(), 16);
  CHECK(first == "   0   1  13   0");
  raw.close();
  const auto own = read_own_arabic(dir);
  REQUIRE(own.size() == 2);
  CHECK(own[0].name == "EIGENER PUNKT");
  CHECK(own[1].terms[2].kind == 14);
  CHECK(own[1].terms[2].value == 105);
  const Chart c = synthetic();
  const std::vector<ArabicPart> parts = arabic_parts(c, ArabicFormula::kAlwaysDay, dir);
  // still 37 rows, the own points took rows 0 and 1 like his arabte
  REQUIRE(parts.size() == 37);
  CHECK(parts[0].name == "EIGENER PUNKT");
  CHECK(parts[0].own);
  CHECK(deg(parts[0].la) == doctest::Approx(deg(c.houses.cusp[1] + c.b[body::kSun].el - c.b[body::kMoon].el)));
  //RR gz4$ labels a whole degree with its sign, 105 reads 15°CN
  CHECK(parts[1].formula.find("15°CN") != std::string::npos);
  CHECK(parts[1].formula.find("Hv10") != std::string::npos);
  CHECK(parts[2].name == "Occultismus");
  // his old dispatch could store a phantom body, the row is flagged
  REQUIRE(append_own_arabic_name(dir, 2, "KAPUTT", ""));
  REQUIRE(append_own_arabic_term(dir, 2, 1, {0, 27}));
  REQUIRE(append_own_arabic_term(dir, 2, 2, {0, body::kSun}));
  REQUIRE(append_own_arabic_term(dir, 2, 3, {0, body::kMoon}));
  const std::vector<ArabicPart> bad = arabic_parts(c, ArabicFormula::kAlwaysDay, dir);
  CHECK_FALSE(bad[2].valid);
  delete_own_arabic(dir);
  CHECK(read_own_arabic(dir).empty());
  std::filesystem::remove_all(dir);
}
