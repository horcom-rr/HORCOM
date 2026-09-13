// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cstdio>
#include <filesystem>
#include <fstream>

#include "doctest.h"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/encoding.hpp"
#include "horcom/data/place_file.hpp"

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
