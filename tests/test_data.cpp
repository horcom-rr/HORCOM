// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cstdio>
#include <filesystem>
#include <fstream>

#include "doctest.h"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/encoding.hpp"
#include "horcom/data/countries.hpp"
#include "horcom/data/place_file.hpp"
#include "horcom/data/statist.hpp"
#include "horcom/data/zone_names.hpp"

using namespace horcom;

namespace {

std::filesystem::path temp_file(const char* name) {
  return std::filesystem::temp_directory_path() / name;
}

// a synthetic chart, no real person
ChartRecord sample() {
  ChartRecord r;
  r.day = 3;
  r.month = 11;
  r.year = 1948;
  r.hour = 14;
  r.minute = 26.4;
  r.lon = 11.3244;
  r.lat = 48.1742;
  r.name = "Testfall Grünwald";
  r.place = "München / D";
  r.remark = "synthetischer Datensatz";
  return r;
}

}  // namespace

TEST_CASE("cp1252 round trips the German letters") {
  const std::string utf8 = "MÜNCHEN äöüß Straße";
  const std::string bytes = utf8_to_cp1252(utf8);
  CHECK(bytes.size() == 19);
  CHECK(cp1252_to_utf8(bytes) == utf8);
  // the euro sign lives in the 0x80 row
  CHECK(utf8_to_cp1252("€") == std::string(1, static_cast<char>(0x80)));
}

TEST_CASE("a chart record encodes to the exact original layout") {
  const std::string rec = encode_chart_record(sample());
  REQUIRE(rec.size() == kChartRecordBytes);
  // right aligned numbers in their fields
  CHECK(rec.substr(0, 2) == " 3");
  CHECK(rec.substr(2, 2) == "11");
  CHECK(rec.substr(4, 5) == " 1948");
  CHECK(rec.substr(9, 2) == "14");
  CHECK(rec.substr(11, 5) == " 26.4");
  // left aligned text with space padding, umlaut as one 1252 byte
  CHECK(rec.substr(32, 8) == std::string("Testfall"));
  CHECK(rec[56] == ' ');
  CHECK(static_cast<unsigned char>(rec[58]) == 0xFC);  // ü of München
}

TEST_CASE("chart records round trip through a file") {
  const auto path = temp_file("horcom_test_chart.dat");
  std::vector<ChartRecord> in{sample(), sample()};
  in[1].year = -43;
  in[1].name = "Antike Probe";
  in[1].remark = "(JULIAN.) vor der Reform";
  REQUIRE(write_chart_file(path, in));
  const auto out = read_chart_file(path);
  REQUIRE(out.has_value());
  REQUIRE(out->size() == 2);
  const ChartRecord& a = (*out)[0];
  CHECK(a.day == 3);
  CHECK(a.month == 11);
  CHECK(a.year == 1948);
  CHECK(a.hour == doctest::Approx(14.0));
  CHECK(a.minute == doctest::Approx(26.4));
  CHECK(a.lon == doctest::Approx(11.3244));
  CHECK(a.lat == doctest::Approx(48.1742));
  CHECK(a.name == "Testfall Grünwald");
  CHECK(a.place == "München / D");
  CHECK(a.calendar() == Calendar::kAuto);
  const ChartRecord& b = (*out)[1];
  CHECK(b.year == -43);
  CHECK(b.calendar() == Calendar::kJulian);
  std::filesystem::remove(path);
}

TEST_CASE("a file with a broken size is rejected") {
  const auto path = temp_file("horcom_test_broken.dat");
  std::ofstream f(path, std::ios::binary);
  f << "short";
  f.close();
  CHECK_FALSE(read_chart_file(path).has_value());
  std::filesystem::remove(path);
}

TEST_CASE("place records round trip and keep the layout") {
  PlaceRecord p;
  p.lon = 6.075555;
  p.lat = 50.77611;
  p.name = "AACHEN";
  const std::string rec = encode_place_record(p);
  REQUIRE(rec.size() == kPlaceRecordBytes);
  const PlaceRecord back = decode_place_record(rec);
  CHECK(back.lon == doctest::Approx(6.075555));
  CHECK(back.lat == doctest::Approx(50.77611));
  CHECK(back.name == "AACHEN");
  CHECK_FALSE(back.zone_to_ut().has_value());
}

TEST_CASE("the zone picker suffix reads like the original") {
  // MEZ is stored as the correction to UT, minus one
  PlaceRecord p;
  p.lon = 16.37;
  p.lat = 48.21;
  p.name = "Wien / A         -1";
  const PlaceRecord back = decode_place_record(encode_place_record(p));
  REQUIRE(back.zone_to_ut().has_value());
  CHECK(*back.zone_to_ut() == doctest::Approx(-1.0));
  // Acapulco west of Greenwich carries plus six
  PlaceRecord q;
  q.name = "Acapulco / Mex   +6";
  REQUIRE(q.zone_to_ut().has_value());
  CHECK(*q.zone_to_ut() == doctest::Approx(6.0));
}

TEST_CASE("the shipped europa picker file decodes with zones") {
  const auto places = read_place_file(HORCOM_TEST_DATA_DIR "/places/europa.int");
  REQUIRE(places.has_value());
  CHECK(places->size() == 465);
  bool found = false;
  for (const PlaceRecord& r : *places) {
    if (r.name.rfind("Agram", 0) == 0) {
      found = true;
      REQUIRE(r.zone_to_ut().has_value());
      CHECK(*r.zone_to_ut() == doctest::Approx(-1.0));
      CHECK(r.lon > 15.0);
      CHECK(r.lon < 17.0);
      CHECK(r.lat > 45.0);
      CHECK(r.lat < 47.0);
    }
  }
  CHECK(found);
}

TEST_CASE("zone catalogue lines parse like zeitzon_nam_horc") {
  const ZoneEntry amt =
      parse_zone_line("Amsterdamer Time ( 1892 - 1940 )              AMT       -00 h 20 m\r");
  CHECK(amt.name == "Amsterdamer Time ( 1892 - 1940 )");
  CHECK(amt.abbrev == "AMT");
  REQUIRE(amt.to_ut_hours.has_value());
  CHECK(*amt.to_ut_hours == doctest::Approx(-20.0 / 60.0));
  // the sign lives in the first character, VAL of the hours loses it
  const ZoneEntry deep =
      parse_zone_line("Somewhere                                     XYZ       -11 h 23 m");
  REQUIRE(deep.to_ut_hours.has_value());
  CHECK(*deep.to_ut_hours == doctest::Approx(-(11.0 + 23.0 / 60.0)));
  // a plus field stays positive, Madrid west of Greenwich
  const ZoneEntry madrid =
      parse_zone_line("Madrider Time ( ca.1880 - 1901 )                        +00 h 15 m");
  CHECK(madrid.abbrev.empty());
  REQUIRE(madrid.to_ut_hours.has_value());
  CHECK(*madrid.to_ut_hours == doctest::Approx(0.25));
  // the local time rows carry no number
  const ZoneEntry lmt =
      parse_zone_line("Local Mean Time (after  ca. 1810 s. Erl.2)    LMT                 ");
  CHECK(lmt.is_local_time());
  CHECK_FALSE(lmt.to_ut_hours.has_value());
}

TEST_CASE("the shipped zone catalogue decodes to its 176 entries") {
  const auto zones = load_zone_names(HORCOM_TEST_DATA_DIR "/zonnamen.int");
  REQUIRE(zones.has_value());
  CHECK(zones->size() == 176);
  CHECK((*zones)[0].name == "Greenwich Mean Time");
  CHECK((*zones)[0].abbrev == "GMT");
  REQUIRE((*zones)[0].to_ut_hours.has_value());
  CHECK(*(*zones)[0].to_ut_hours == doctest::Approx(0.0));
  int local_rows = 0;
  for (const ZoneEntry& z : *zones) {
    if (z.is_local_time()) {
      ++local_rows;
    }
  }
  CHECK(local_rows == 2);
}

TEST_CASE("the country tables load with the Aruba fix in place") {
  const auto nima = load_nima_countries(HORCOM_TEST_DATA_DIR "/landnima.int");
  REQUIRE(nima.has_value());
  CHECK(nima->size() == 263);
  CHECK((*nima)[0].code == "AA");
  // the original discriminator never reaches its first slot, the
  // rewrite finds Aruba, a documented deviation
  CHECK(nima_country_name(*nima, "aa") == "ARUBA");
  CHECK(nima_country_name(*nima, "gm") == "GERMANY");
  CHECK(nima_country_name(*nima, "qq").empty());
  const auto german = load_german_countries(HORCOM_TEST_DATA_DIR "/laender.int");
  REQUIRE(german.has_value());
  CHECK(german->size() == 57);
  CHECK((*german)[0].abbrev == "A");
  CHECK((*german)[0].name == "Österreich");
}

TEST_CASE("the preferred place accepts both coordinate encodings") {
  const auto path = temp_file("horcom_test_ort.ext");
  {
    std::ofstream f(path, std::ios::binary);
    // the writer's micro degree form, no decimal point
    f << "11324444" << "48174167" << "EICHENAU            ";
  }
  const auto micro = read_preferred_place(path);
  REQUIRE(micro.has_value());
  CHECK(micro->lon == doctest::Approx(11.324444).epsilon(1e-9));
  CHECK(micro->lat == doctest::Approx(48.174167).epsilon(1e-9));
  CHECK(micro->name == "EICHENAU");
  {
    std::ofstream f(path, std::ios::binary);
    f << " 11.3244" << " 48.1742" << "EICHENAU            ";
  }
  const auto plain = read_preferred_place(path);
  REQUIRE(plain.has_value());
  CHECK(plain->lon == doctest::Approx(11.3244));
  std::filesystem::remove(path);
}

TEST_CASE("the statistics store round trips with the original packing") {
  StatSet set;
  set.params.appa_name = "App.1";
  set.params.gena = " Ephem ::App.1,MitParall.";
  set.params.par = 1.0;
  for (int i = 1; i <= 22; ++i) {
    set.params.nk[static_cast<std::size_t>(i)] = 18 + i;
  }
  StatRecord r;
  r.name = "Testfall";
  r.place = "Eichenau";
  r.day = 13;
  r.month = 10;
  r.year = 1992;
  r.hour = 3;
  r.minute = 0.0;
  r.lon = 11.32;
  r.lat = 48.17;
  for (int slot = 1; slot <= 12; ++slot) {
    r.el[static_cast<std::size_t>(slot)] = slot * 0.5;
  }
  r.ac = 1.234567;
  r.mc = 4.567891;
  r.h2 = 1.5;
  r.h3 = 1.9;
  r.h5 = 2.8;
  r.h6 = 3.1;
  r.el[20] = 2.4680135;   // Chiron rides the extras block
  r.el[35] = 0.1234567;   // Quaoar rides the .STH twin
  set.records.push_back(r);

  const auto base = std::filesystem::temp_directory_path() / "HORCTEST.STA";
  REQUIRE(save_statistics(base, set));
  // the twin name follows stat2_teil, eight characters plus a one
  CHECK(sth_path(base).filename().string() == "HORCTEST1.STH");
  CHECK(std::filesystem::file_size(base) == kStaRecordBytes);
  CHECK(std::filesystem::file_size(sth_path(base)) == kSthRecordBytes);

  const auto back = load_statistics(base);
  REQUIRE(back.has_value());
  REQUIRE(back->records.size() == 1);
  const StatRecord& b = back->records[0];
  // the writer uppercases like the original
  CHECK(b.name == "TESTFALL");
  CHECK(b.place == "EICHENAU");
  CHECK(b.day == 13);
  CHECK(b.year == 1992);
  CHECK(b.lon == doctest::Approx(11.32));
  CHECK(b.el[1] == doctest::Approx(0.5).epsilon(1e-6));
  CHECK(b.el[12] == doctest::Approx(6.0).epsilon(1e-6));
  CHECK(b.ac == doctest::Approx(1.234567).epsilon(1e-6));
  CHECK(b.el[20] == doctest::Approx(2.4680135).epsilon(1e-6));
  CHECK(b.el[35] == doctest::Approx(0.1234567).epsilon(1e-6));
  CHECK_FALSE(b.heliocentric());
  std::filesystem::remove(base);
  std::filesystem::remove(sth_path(base));
  std::filesystem::path par = base;
  par.replace_extension(".PAR");
  std::filesystem::remove(par);
}

