// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <filesystem>
#include <fstream>

#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/statist.hpp"

using namespace horcom;

namespace {

// an empty folder of its own for every case
std::filesystem::path fresh_folder(const char* name) {
  const std::filesystem::path dir = std::filesystem::temp_directory_path() / name;
  std::error_code ec;
  std::filesystem::remove_all(dir, ec);
  std::filesystem::create_directories(dir);
  return dir;
}

// one invented chart with a Quaoar value in the .STH twin
StatSet one_record(double quaoar) {
  StatSet set;
  set.params.nk[17] = body::kQuaoar;
  StatRecord r;
  r.name = "ANNA MUSTER";
  r.day = 1;
  r.month = 2;
  r.year = 1990;
  r.el[body::kSun] = 1.0;
  r.ac = 0.5;
  r.el[body::kQuaoar] = quaoar;
  set.records.push_back(r);
  return set;
}

}  // namespace

TEST_CASE("two datasets sharing seven letters keep their own .STH twin") {
  // stat2_teil named both ABCDEFGX1.STH and ABCDEFGY1.STH ABCDEFG1.STH,
  // the second dataset overwrote the extras of the first and deleting one
  // took the twin of the other
  const auto dir = fresh_folder("horcom_statist_twins");
  const auto x = dir / "ABCDEFGX.STA";
  const auto y = dir / "ABCDEFGY.STA";
  REQUIRE(save_statistics(x, one_record(1.25)));
  // the first keeps the original name the old program reads
  CHECK(sth_path(x).filename() == "ABCDEFG1.STH");
  REQUIRE(save_statistics(y, one_record(2.5)));
  CHECK(sth_path(y).filename() == "ABCDEFGY1.STH");
  const auto bx = load_statistics(x);
  const auto by = load_statistics(y);
  REQUIRE(bx.has_value());
  REQUIRE(by.has_value());
  CHECK(bx->records[0].el[body::kQuaoar] == doctest::Approx(1.25).epsilon(1e-6));
  CHECK(by->records[0].el[body::kQuaoar] == doctest::Approx(2.5).epsilon(1e-6));
  // deleting the second leaves the twin of the first alone
  CHECK(remove_statistics(y));
  CHECK_FALSE(std::filesystem::exists(y));
  CHECK_FALSE(std::filesystem::exists(dir / "ABCDEFGY1.STH"));
  CHECK_FALSE(std::filesystem::exists(dir / "ABCDEFGY.PAR"));
  CHECK(std::filesystem::exists(dir / "ABCDEFG1.STH"));
  const auto again = load_statistics(x);
  REQUIRE(again.has_value());
  CHECK(again->records[0].el[body::kQuaoar] == doctest::Approx(1.25).epsilon(1e-6));
  CHECK(remove_statistics(x));
  CHECK_FALSE(std::filesystem::exists(dir / "ABCDEFG1.STH"));
}

TEST_CASE("a dataset in small letters finds its .PAR and .STH") {
  // his DOS names ignored case, a Linux copy of his folder may mix it
  const auto dir = fresh_folder("horcom_statist_case");
  const auto upper = dir / "DEMO.STA";
  REQUIRE(save_statistics(upper, one_record(0.75)));
  std::filesystem::rename(upper, dir / "demo.sta");
  const auto lower = dir / "demo.sta";
  CHECK(std::filesystem::exists(par_path(lower)));
  REQUIRE(existing_sth_path(lower).has_value());
  const auto set = load_statistics(lower);
  REQUIRE(set.has_value());
  CHECK(set->records[0].el[body::kQuaoar] == doctest::Approx(0.75).epsilon(1e-6));
  CHECK(std::filesystem::exists(find_ignoring_case(dir, "Demo.Par")));
  CHECK(find_ignoring_case(dir, "NONE.PAR") == dir / "NONE.PAR");
  // a new dataset writes the upper case names of the original
  const auto fresh = dir / "NEU.STA";
  REQUIRE(save_statistics(fresh, one_record(0.25)));
  CHECK(par_path(fresh).filename() == "NEU.PAR");
  CHECK(sth_path(fresh).filename() == "NEU1.STH");
}

TEST_CASE("a name outside the ANSI code page makes a path") {
  // path::string() threw on Windows for such a name
  const auto dir = fresh_folder("horcom_statist_names");
  const std::filesystem::path sta = dir / std::filesystem::path(u8"ŁÓDŹ_中文X.STA");
  REQUIRE(save_statistics(sta, one_record(0.5)));
  CHECK(std::filesystem::exists(par_path(sta)));
  // seven characters stay whole, no byte of a character is cut
  const std::filesystem::path cut(u8"ŁÓDŹ_中文1.STH");
  CHECK(legacy_sth_path(sta).filename() == cut);
  const auto set = load_statistics(sta);
  REQUIRE(set.has_value());
  CHECK(set->records[0].el[body::kQuaoar] == doctest::Approx(0.5).epsilon(1e-6));
}

TEST_CASE("stat1_0 writes the name in capitals and cut to the field") {
  ChartRecord r;
  r.name = "  anna m\xC3\xBCller-l\xC3\xBC" "denscheid von hausen";
  r.place = "m\xC3\xBCnchen";
  r.day = 3;
  r.month = 4;
  r.year = 1950;
  r.hour = 7.0;
  r.minute = 12.5;
  r.lon = 11.58;
  r.lat = 48.14;
  Chart c;
  c.ok = true;
  c.b[body::kSun] = {true, true, 0.25};
  c.houses.ok = true;
  c.houses.cusp[1] = 1.0;
  c.houses.cusp[10] = 2.0;
  const StatRecord geo = stat_record(r, c, false);
  // LSET na$ = LEFT$(TRIM$(UPPER$(naa$)),25)
  CHECK(geo.name == "ANNA M\xC3\x9CLLER-L\xC3\x9C" "DENSCHEID V");
  CHECK(geo.place == "M\xC3\x9CNCHEN");
  CHECK(geo.el[body::kSun] == 0.25);
  CHECK(geo.ac == 1.0);
  CHECK(geo.mc == 2.0);
  CHECK_FALSE(geo.heliocentric());
  // the helio file writes no cusps
  const StatRecord helio = stat_record(r, c, true);
  CHECK(helio.ac == 0.0);
  CHECK(helio.mc == 0.0);
}

TEST_CASE("a dataset record becomes a RADIX record in UT") {
  StatRecord r;
  r.name = "ANNA MUSTER";
  r.day = 5;
  r.month = 6;
  r.year = 1970;
  r.hour = 13;
  r.minute = 59.99;
  r.lon = -73.99;
  r.lat = 40.75;
  const AafRecord a = stat_aaf_record(r);
  CHECK(a.zone == kUtZoneText);
  CHECK(a.hour == 13);
  CHECK(a.minute == 59);
  CHECK(a.second == 59);
  CHECK(a.lon_ew == 'W');
  CHECK(a.lon_deg == 73);
  CHECK(a.lon_min == 59);
  CHECK(a.lon_sec == 24);
  CHECK(a.lat_ns == 'N');
  CHECK(a.lat_deg == 40);
  CHECK(a.lat_min == 45);
  CHECK(a.lat_sec == 0);
}

TEST_CASE("stat2parl puts the parameters of the dataset over the profile") {
  Konsta k;
  k.haw = 1;
  k.par = 2.0;
  k.stzw = 1;
  StatParams p;
  p.haw = static_cast<int>(HouseSystem::kKochGoh);
  p.appa = static_cast<int>(ApparentMode::kTrue);
  p.apogw = true;
  p.moknw = true;
  p.par = 1.0;
  p.nk[2] = 19;
  const ChartSettings s = stat_chart_settings(k, p, false);
  CHECK(s.houses == HouseSystem::kKochGoh);
  CHECK(s.apparent == ApparentMode::kTrue);
  CHECK(s.true_apogee);
  CHECK(s.true_node);
  CHECK(s.topocentric_parallax);
  CHECK(s.nk[2] == body::kChiron);
  CHECK(s.extra_bodies);
  CHECK_FALSE(s.heliocentric);
  CHECK(s.apparent_sidereal);
  // a stray appa& keeps the one of the profile
  p.appa = 9;
  CHECK(stat_chart_settings(k, p, true).apparent == static_cast<ApparentMode>(k.appa));
  CHECK(stat_chart_settings(k, p, true).heliocentric);
  // and the writer takes the runtime labels back
  const StatParams back = stat_params(s, k);
  CHECK(back.haw == p.haw);
  CHECK(back.appa_name == "Wahr");
  CHECK(back.gena == k.gena);
  CHECK(back.par == 1.0);
}
