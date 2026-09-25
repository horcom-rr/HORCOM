// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/collection.hpp"
#include "horcom/data/file_io.hpp"
#include "horcom/data/record_order.hpp"
#include "horcom/data/encoding.hpp"
#include "horcom/data/countries.hpp"
#include "horcom/data/place_file.hpp"
#include "horcom/data/kommen.hpp"
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

TEST_CASE("appending a place grows the file and round trips") {
  namespace fs = std::filesystem;
  const fs::path path = fs::temp_directory_path() / "horcom_place_write.int";
  fs::remove(path);
  PlaceRecord a;
  a.lon = 13.05;
  a.lat = 47.7967;
  a.name = "SALZBURG";
  REQUIRE(append_place(path, a));
  PlaceRecord b;
  b.lon = -0.1275;
  b.lat = 51.5072;
  b.name = "LONDON";
  REQUIRE(append_place(path, b));
  const auto back = read_place_file(path);
  REQUIRE(back.has_value());
  REQUIRE(back->size() == 2);
  CHECK((*back)[0].name == "SALZBURG");
  CHECK((*back)[0].lon == doctest::Approx(13.05));
  CHECK((*back)[1].name == "LONDON");
  CHECK((*back)[1].lon == doctest::Approx(-0.1275));
  fs::remove(path);
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
  // his LEFT$(daa$,8) counts the leading backslash, seven letters stay
  CHECK(sth_path(base).filename().string() == "HORCTES1.STH");
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


TEST_CASE("a sparse extra selection keeps each body in its own slot") {
  // Robert's own final profile, Chiron, Quaoar and Xena chosen. His nk
  // numbers them compactly 19, 20 and 21, the store keys field k by the
  // extra index, so a reader that took nk for the slot handed Quaoar's
  // positions to Chiron and Xena's to Transpluto
  StatSet set;
  set.params.nk[2] = body::kChiron;
  set.params.nk[17] = body::kQuaoar;
  set.params.nk[22] = body::kXena;
  StatRecord r;
  r.name = "Testfall";
  r.day = 1;
  r.month = 2;
  r.year = 1990;
  r.el[body::kChiron] = 1.1111111;
  r.el[body::kQuaoar] = 2.2222222;
  r.el[body::kXena] = 3.3333333;
  set.records.push_back(r);
  const auto base = std::filesystem::temp_directory_path() / "HORCSPAR.STA";
  REQUIRE(save_statistics(base, set));
  std::filesystem::path par = base;
  par.replace_extension(".PAR");
  {
    // the parameter file carries his compact numbering
    std::ifstream f(par, std::ios::binary);
    std::string line;
    std::getline(f, line);
    std::vector<int> nk;
    while (std::getline(f, line)) {
      nk.push_back(std::atoi(line.c_str()));
    }
    REQUIRE(nk.size() == 22);
    CHECK(nk[1] == 19);
    CHECK(nk[16] == 20);
    CHECK(nk[21] == 21);
    CHECK(nk[0] == 0);
  }
  const auto back = load_statistics(base);
  REQUIRE(back.has_value());
  REQUIRE(back->records.size() == 1);
  const StatRecord& b = back->records[0];
  CHECK(b.el[body::kChiron] == doctest::Approx(1.1111111).epsilon(1e-6));
  CHECK(b.el[body::kQuaoar] == doctest::Approx(2.2222222).epsilon(1e-6));
  CHECK(b.el[body::kXena] == doctest::Approx(3.3333333).epsilon(1e-6));
  // the unchosen neighbours stay empty
  CHECK(b.el[body::kApogee] == 0.0);
  CHECK(b.el[body::kTranspluto] == 0.0);
  CHECK(extra_slot(2) == body::kChiron);
  CHECK(extra_slot(22) == body::kXena);
  std::filesystem::remove(base);
  std::filesystem::remove(sth_path(base));
  std::filesystem::remove(par);
}

TEST_CASE("compact numbering follows the plgen selection loop") {
  std::array<bool, 23> chosen{};
  chosen[1] = true;
  chosen[5] = true;
  chosen[9] = true;
  const std::array<int, 23> nk = compact_nk(chosen);
  CHECK(nk[1] == 19);
  CHECK(nk[5] == 20);
  CHECK(nk[9] == 21);
  CHECK(nk[2] == 0);
}

TEST_CASE("the kommen reader keeps the lese_text rules") {
  const auto dir = std::filesystem::temp_directory_path();
  const auto path = dir / "KOMM1.TXT";
  {
    std::ofstream out(path, std::ios::binary);
    // his 1252 bytes, a tilde ruler line, the dash terminator
    out << "Willkommen, sch\xF6ne Gr\xFC\xDF" << "e !\r\n";
    out << "~~~~~~~~~~~~\r\n";
    out << "Zweite Zeile\r\n";
    out << "-\r\n";
    out << "unsichtbar\r\n";
  }
  const auto text = read_kommen(path);
  REQUIRE(text.has_value());
  CHECK(*text == "Willkommen, sch\xC3\xB6ne Gr\xC3\xBC\xC3\x9F""e !\nZweite Zeile\n");
  const auto entries = kommen_entries(dir);
  REQUIRE(!entries.empty());
  CHECK(entries[0].index == 1);
  CHECK(entries[0].title == "Einf\xC3\xBChrender Kommentar");
  // a markdown edition of the same topic wins over the original file
  const auto md = dir / "komm1.md";
  {
    std::ofstream out(md);
    out << "# Kommentar\n";
  }
  const auto preferred = kommen_entries(dir);
  REQUIRE(!preferred.empty());
  CHECK(preferred[0].path == md);
  std::filesystem::remove(md);
  std::filesystem::remove(path);
}

TEST_CASE("his time texts decode the Atari letters beside Windows 1252") {
  // Preu\x9Eische and f\x81r from his Atari days, gem\xE4\xDF typed later
  const std::string atari = "f\x81r die Preu\x9Eische Bahn, Neuch\x83tel, F\x9AR, erfahrungsgem\xE4\xDF";
  CHECK(atari_cp1252_to_utf8(atari) ==
        "f\xC3\xBCr die Preu\xC3\x9F" "ische Bahn, Neuch\xC3\xA2tel, F\xC3\x9CR, erfahrungsgem\xC3\xA4\xC3\x9F");
  CHECK(looks_like_utf8("Gr\xC3\xBC\xC3\x9F" "e"));
  CHECK_FALSE(looks_like_utf8(atari));
}

TEST_CASE("the ZEITBEST reader keeps dash lines like his handle seven") {
  const auto path = std::filesystem::temp_directory_path() / "HORCTEST.TXT";
  {
    std::ofstream out(path, std::ios::binary);
    out << "--------\r\n";
    out << "      DEUTSCHLAND\r\n";
    out << "~ unsichtbar\r\n";
    out << "--------\r\n";
    out << "f\x81r die Preu\x9Eische Eisenbahn\r\n";
  }
  const auto text = read_zeitbest(path);
  REQUIRE(text.has_value());
  CHECK(*text == "--------\n      DEUTSCHLAND\n--------\nf\xC3\xBCr die Preu\xC3\x9F" "ische Eisenbahn\n");
  std::filesystem::remove(path);
  // the shipped editions are UTF-8 and read unchanged
  const auto germany = read_zeitbest(std::filesystem::path(HORCOM_TEST_DATA_DIR) / "zeitbest" / "GERMANY.TXT");
  REQUIRE(germany.has_value());
  CHECK(germany->find("DEUTSCHLAND") != std::string::npos);
  CHECK(germany->find("Preu\xC3\x9F" "ische") != std::string::npos);
}

TEST_CASE("the zone of a picker name reads like his FUNCTION VAL") {
  const auto zone = [](const char* name) { return PlaceRecord{0.0, 0.0, name}.zone_to_ut(); };
  CHECK(*zone("Agram / YU        -1") == doctest::Approx(-1.0));
  // a long name reaching into the zone bytes, his filter keeps the number
  CHECK(*zone("Tirana / Albanie  -1") == doctest::Approx(-1.0));
  CHECK(*zone("Allahabad / Indi-5.5") == doctest::Approx(-5.5));
  CHECK(*zone("Kourou/Frz.Guyan+3.5") == doctest::Approx(3.5));
  CHECK(*zone("Brisbane/Austral -10") == doctest::Approx(-10.0));
  CHECK(*zone("Beccles / GB     +-0") == doctest::Approx(0.0));
  // his VAL read ".-1" as zero, the trailing signed number is the zone
  CHECK(*zone("Bialystok / Pol.  -1") == doctest::Approx(-1.0));
  CHECK_FALSE(zone("LJUBLJANA/SLOVENIA").has_value());
}

TEST_CASE("the shipped zone lists carry the corrected records") {
  const std::filesystem::path dir = std::filesystem::path(HORCOM_TEST_DATA_DIR) / "places";
  const auto find = [](const std::vector<PlaceRecord>& list, const std::string& prefix) -> const PlaceRecord* {
    for (const PlaceRecord& p : list) {
      if (p.name.rfind(prefix, 0) == 0) {
        return &p;
      }
    }
    return nullptr;
  };
  const auto europa = read_place_file(dir / "europa.int");
  REQUIRE(europa.has_value());
  // his EUROPA.INT had Saloniki and MADRID at +2, Tirana at -2
  const PlaceRecord* saloniki = find(*europa, "Saloniki");
  REQUIRE(saloniki != nullptr);
  CHECK(*saloniki->zone_to_ut() == doctest::Approx(-2.0));
  const PlaceRecord* madrid = find(*europa, "MADRID");
  REQUIRE(madrid != nullptr);
  CHECK(*madrid->zone_to_ut() == doctest::Approx(-1.0));
  const PlaceRecord* tirana = find(*europa, "Tirana");
  REQUIRE(tirana != nullptr);
  CHECK(*tirana->zone_to_ut() == doctest::Approx(-1.0));
  // his Coimbra stood east at 8.43, Cambridge at 5.85 east, Powderham on
  // the coordinates 53.47 N 1.57 E
  const PlaceRecord* coimbra = find(*europa, "Coimbra");
  REQUIRE(coimbra != nullptr);
  CHECK(coimbra->lon == doctest::Approx(-8.43333));
  const PlaceRecord* cambridge = find(*europa, "Cambridge");
  REQUIRE(cambridge != nullptr);
  CHECK(cambridge->lon == doctest::Approx(0.116666));
  const PlaceRecord* powderham = find(*europa, "Powderham");
  REQUIRE(powderham != nullptr);
  CHECK(powderham->lon == doctest::Approx(-3.45));
  CHECK(powderham->lat == doctest::Approx(50.63333));
  const auto welt = read_place_file(dir / "welt.int");
  REQUIRE(welt.has_value());
  // his WELT.INT had Kapstadt at +2
  const PlaceRecord* kapstadt = find(*welt, "Kapstadt");
  REQUIRE(kapstadt != nullptr);
  CHECK(*kapstadt->zone_to_ut() == doctest::Approx(-2.0));
  // every zone of both lists now points the way of its longitude, a zone
  // difference is UT minus local time and falls to the east
  for (const auto* list : {&*europa, &*welt}) {
    for (const PlaceRecord& p : *list) {
      const auto z = p.zone_to_ut();
      if (z && std::abs(p.lon) > 7.5) {
        CHECK_MESSAGE((*z == 0.0 || (*z < 0.0) == (p.lon > 0.0)), p.name);
      }
    }
  }
}

TEST_CASE("place files trim and delete like a2f_tr_ort") {
  std::vector<PlaceRecord> p(5);
  p[0] = {11.5, 48.1, "Muenchen / D"};
  p[1] = {0.0, 51.4769, "Greenwich / GB"};
  p[2] = {0.0, 0.0, "Nirgendwo"};
  p[3] = {2.35, 48.85, ""};
  p[4] = {13.4, 52.5, "Berlin / D"};
  std::vector<PlaceRecord> t = p;
  trim_places(t);
  // the Greenwich meridian stays, his longitude test alone dropped it
  REQUIRE(t.size() == 3);
  CHECK(t[1].name == "Greenwich / GB");
  std::vector<PlaceRecord> d = p;
  delete_places(d, {0});
  REQUIRE(d.size() == 2);
  CHECK(d[0].name == "Greenwich / GB");
  CHECK(d[1].name == "Berlin / D");
}

TEST_CASE("the preferred place writes like ortp and reads back") {
  const auto path = std::filesystem::temp_directory_path() / "horctest_ort.ext";
  REQUIRE(write_preferred_place(path, {10.123456, 47.654321, "TESTORT"}));
  {
    std::ifstream in(path, std::ios::binary);
    std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    // integer micro degrees in eight bytes like his ortp
    CHECK(bytes.size() == kPlaceRecordBytes);
    CHECK(bytes.substr(0, 16) == "1012345647654321");
  }
  auto back = read_preferred_place(path);
  REQUIRE(back.has_value());
  CHECK(back->lon == doctest::Approx(10.123456));
  CHECK(back->lat == doctest::Approx(47.654321));
  CHECK(back->name == "TESTORT");
  // west of ten degrees the micro degrees overflow, a decimal fills in
  REQUIRE(write_preferred_place(path, {-73.9857, 40.7484, "NEW YORK"}));
  back = read_preferred_place(path);
  REQUIRE(back.has_value());
  CHECK(back->lon == doctest::Approx(-73.9857).epsilon(1e-6));
  CHECK(back->lat == doctest::Approx(40.7484).epsilon(1e-6));
  std::filesystem::remove(path);
}

TEST_CASE("trimming drops dayless and placeless records like a2f_tr_dat") {
  std::vector<ChartRecord> r(4);
  r[0].name = "  Muster Hans";
  r[0].day = 13;
  r[0].lon = 11.3;
  r[1].name = "OHNE TAG";
  r[1].day = 0;
  r[1].lon = 11.3;
  r[2].name = "OHNE ORT";
  r[2].day = 5;
  r[2].lon = 0.0;
  r[2].lat = 0.0;
  r[3].name = "Beispiel Eva";
  r[3].day = 7;
  r[3].lat = 48.1;
  trim_records(r);
  REQUIRE(r.size() == 2);
  CHECK(r[0].name == "Muster Hans");
  CHECK(r[1].name == "Beispiel Eva");
}

TEST_CASE("deleting removes the marked records and trims alongside") {
  std::vector<ChartRecord> r(4);
  for (std::size_t i = 0; i < r.size(); ++i) {
    r[i].name = "SATZ " + std::to_string(i);
    r[i].day = 1;
    r[i].lon = 10.0;
  }
  r[2].day = 0;
  delete_records(r, {1});
  REQUIRE(r.size() == 2);
  CHECK(r[0].name == "SATZ 0");
  CHECK(r[1].name == "SATZ 3");
}

TEST_CASE("overwrite by name drops every copy case blind") {
  std::vector<ChartRecord> r(3);
  r[0].name = "ZENTNER WILHELM AUGUST";
  r[1].name = "Zentner Wilhelm August ";
  r[2].name = "MAYER JONAS";
  CHECK(remove_records_by_name(r, " zentner wilhelm august") == 2);
  REQUIRE(r.size() == 1);
  CHECK(r[0].name == "MAYER JONAS");
}

TEST_CASE("record order follows the SORTIER-MODUS keys") {
  std::vector<OrderKeySource> r;
  // invented records, no real people
  r.push_back({"ZENTNER WILHELM AUGUST", 27, 1, 1956});
  r.push_back({"MAYER JONAS", 31, 3, 1932});
  r.push_back({"ADLER JOHANNES SEPP", 31, 3, 1885});
  const auto by_name = record_order(r, RecordOrder::kName123);
  CHECK(by_name == std::vector<std::size_t>{2, 1, 0});
  // 2. und 3. Name keys on five characters of each word, JOHAN SEPP
  // against JONAS against WILHE AUGUS
  const auto by_given = record_order(r, RecordOrder::kName23);
  REQUIRE(by_given.size() == 3);
  CHECK(by_given[0] == 2);
  CHECK(by_given[1] == 1);
  CHECK(by_given[2] == 0);
  // birthday ignores the year, both 31.3. keep file order, 27.1. first
  const auto by_birthday = record_order(r, RecordOrder::kBirthday);
  CHECK(by_birthday == std::vector<std::size_t>{0, 1, 2});
  const auto by_date = record_order(r, RecordOrder::kDate);
  CHECK(by_date == std::vector<std::size_t>{2, 1, 0});
  CHECK(record_order(r, RecordOrder::kFile) == std::vector<std::size_t>{0, 1, 2});
}

TEST_CASE("aaf twin paths swap his folder pair and fall back to siblings") {
  namespace fs = std::filesystem;
  const fs::path root = fs::temp_directory_path() / "horcom_twin_test";
  fs::create_directories(root / "SPEZIAL");
  fs::create_directories(root / "AAFDATEN");
  const fs::path dat = root / "SPEZIAL" / "MUSIKER.DAT";
  CHECK(aaf_twin_path(dat) == root / "AAFDATEN" / "MUSIKER.AAF");
  CHECK(dat_twin_path(root / "AAFDATEN" / "MUSIKER.AAF") == root / "SPEZIAL" / "MUSIKER.DAT");
  const fs::path lone = root / "NEU.DAT";
  CHECK(aaf_twin_path(lone) == root / "NEU.AAF");
  fs::remove_all(root);
}

TEST_CASE("minimizing collapses same name and birth clock") {
  std::vector<ChartRecord> r(3);
  r[0].name = "Muster Hans";
  r[0].day = 13;
  r[0].hour = 3.0;
  r[1] = r[0];
  r[1].place = "Anderswo";
  r[2] = r[0];
  r[2].day = 14;
  minimize_records(r);
  REQUIRE(r.size() == 2);
  CHECK(r[1].day == 14);
}

TEST_CASE("appending to a place file the reader refuses leaves it untouched") {
  namespace fs = std::filesystem;
  const fs::path path = fs::temp_directory_path() / "horcom_place_broken.int";
  {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f << std::string(40, 'x');
  }
  PlaceRecord p;
  p.lon = 13.05;
  p.lat = 47.7967;
  p.name = "SALZBURG";
  // the first port rewrote the file with the new place alone, 36 bytes
  CHECK_FALSE(append_place(path, p));
  CHECK(fs::file_size(path) == 40U);
  fs::remove(path);
}

TEST_CASE("whole file writes replace the file and leave no scratch behind") {
  namespace fs = std::filesystem;
  const fs::path dir = fs::temp_directory_path() / "horcom_replace_test";
  fs::remove_all(dir);
  fs::create_directories(dir);
  const fs::path dat = dir / "PAAR.DAT";
  REQUIRE(write_chart_file(dat, {sample(), sample()}));
  REQUIRE(write_chart_file(dat, {sample()}));
  CHECK(fs::file_size(dat) == kChartRecordBytes);
  int entries = 0;
  for ([[maybe_unused]] const auto& e : fs::directory_iterator(dir)) {
    ++entries;
  }
  CHECK(entries == 1);
  // a folder in the way makes the write fail without touching anything
  CHECK_FALSE(replace_file(dir, "x"));
  fs::remove_all(dir);
}

TEST_CASE("twins are found whatever the case of their names") {
  namespace fs = std::filesystem;
  const fs::path dir = fs::temp_directory_path() / "horcom_case_twin";
  fs::remove_all(dir);
  fs::create_directories(dir);
  REQUIRE(write_aaf(dir / "musik.aaf", {}));
  REQUIRE(write_chart_file(dir / "musik.dat", {}));
  CHECK(fs::equivalent(aaf_twin_path(dir / "musik.dat"), dir / "musik.aaf"));
  CHECK(fs::equivalent(dat_twin_path(dir / "musik.aaf"), dir / "musik.dat"));
  CHECK(fs::equivalent(find_case_blind(dir, "MUSIK.AAF"), dir / "musik.aaf"));
  // a missing twin keeps his capital extension
  CHECK(aaf_twin_path(dir / "neu.dat") == dir / "neu.AAF");
  fs::remove_all(dir);
}

TEST_CASE("the name sort folds the umlauts like his vg| table") {
  std::vector<OrderKeySource> r;
  r.push_back({"OTTO ZWEI", 1, 1, 1950});
  r.push_back({"ÖSTERREICHER EINS", 1, 1, 1950});
  r.push_back({"ORT DREI", 1, 1, 1950});
  // the first port sorted the Ö behind every capital, {2, 0, 1}
  CHECK(record_order(r, RecordOrder::kName123) == std::vector<std::size_t>{2, 1, 0});
  CHECK(collation_key("Müller Straße") == "MULLER STRASE");
}

TEST_CASE("degrees split into rounded seconds with the carry") {
  const Dms d = split_dms(11.99999);
  // his horcom_aaf3 wrote FIX and CINT without a carry, 11 59 60
  CHECK(d.deg == 12);
  CHECK(d.min == 0);
  CHECK(d.sec == 0);
  const Dms m = split_dms(-48.1);
  CHECK(m.deg == 48);
  CHECK(m.min == 6);
  CHECK(m.sec == 0);
  AafRecord r;
  r.set_latitude(-33.5);
  r.set_longitude(-70.25);
  CHECK(r.lat_ns == 'S');
  CHECK(r.lon_ew == 'W');
  CHECK(r.latitude() == doctest::Approx(-33.5));
  CHECK(r.longitude() == doctest::Approx(-70.25));
}

TEST_CASE("a one word DAT name comes back without a star") {
  ChartRecord c = sample();
  c.name = "EINWORT";
  const AafRecord a = aaf_from_chart_record(c);
  CHECK(a.surname == "EINWORT");
  CHECK(a.given.empty());
  // the first port set the given name to *, the DAT name came back as
  // EINWORT * and missed its own record
  CHECK(chart_record_from_aaf(a).name == "EINWORT");
  CHECK(a.zone == kUtZoneText);
  // the clock of the DAT is UT, 14 h 26.4 min is 14:26:24
  CHECK(a.hour == 14);
  CHECK(a.minute == 26);
  CHECK(a.second == 24);
  const ChartRecord back = chart_record_from_aaf(a);
  CHECK(back.day == 3);
  CHECK(back.hour == doctest::Approx(14.0));
  CHECK(back.minute == doctest::Approx(26.4));
  CHECK(back.lat == doctest::Approx(48.1742).epsilon(1e-5));
}

TEST_CASE("DAT text fields take his capitals over Windows 1252") {
  CHECK(dat_field("Müller") == "MÜLLER");
  // ß has no capital in the code page and stays
  CHECK(dat_field("Straße") == "STRAßE");
  CHECK(dat_field("Müller", 3) == "MÜL");
  AafRecord r;
  r.surname = " Testfall ";
  r.given = "*";
  CHECK(record_name(r) == "Testfall");
  r.given = "Neu";
  r.place = "München";
  r.country = "D";
  r.calendar = Calendar::kJulian;
  r.day = 1;
  r.month = 1;
  r.year = 1600;
  r.zone = kUtZoneText;
  const ChartRecord c = chart_record_from_aaf(r);
  CHECK(c.name == "TESTFALL NEU");
  CHECK(c.place == "MÜNCHEN D");
  CHECK(c.remark == "(JULIAN.) ");
}

TEST_CASE("aaf_ident looks for the exact name before his fallback") {
  std::vector<AafRecord> aaf(3);
  aaf[0].surname = "MAIER";
  aaf[1].surname = "MAIERHOFER";
  aaf[1].given = "ANNA";
  aaf[2].surname = "EINZEL";
  // his fallback found MAIER inside the name first, the record 0
  CHECK(aaf_ident(aaf, "MAIERHOFER ANNA") == std::optional<std::size_t>(1));
  CHECK(aaf_ident(aaf, "maier") == std::optional<std::size_t>(0));
  // the name with the star that older files of the port wrote
  CHECK(aaf_ident(aaf, "EINZEL *") == std::optional<std::size_t>(2));
  // his rule for a surname with a one letter given name remains
  CHECK(aaf_ident(aaf, "EINZELFALL X") == std::optional<std::size_t>(2));
  CHECK_FALSE(aaf_ident(aaf, "NIEMAND").has_value());
}

TEST_CASE("the AAF box finds a record of the same name like CASE 168") {
  std::vector<AafRecord> aaf(3);
  aaf[0].surname = "Muster";
  aaf[0].given = "Hans";
  aaf[1].surname = "Anders";
  aaf[1].given = "Eva";
  aaf[2] = aaf[0];
  AafRecord r;
  r.surname = "MUSTER";
  r.given = "hans";
  // like his loop the last hit counts
  CHECK(aaf_same_name(aaf, r) == std::optional<std::size_t>(2));
  r.given = "Otto";
  CHECK_FALSE(aaf_same_name(aaf, r).has_value());
  // his INSTR over the head of the #A93 line
  AafRecord head;
  head.surname = "Anders";
  head.given = "E";
  CHECK(aaf_same_name(aaf, head) == std::optional<std::size_t>(1));
}

TEST_CASE("the chain keeps dated records with a place like a200dat") {
  AafRecord r;
  r.day = 4;
  r.month = 5;
  r.lat_deg = 48;
  CHECK(chain_keeps(r));
  AafRecord no_day = r;
  no_day.day = 0;
  CHECK_FALSE(chain_keeps(no_day));
  AafRecord no_place = r;
  no_place.lat_deg = 0;
  CHECK_FALSE(chain_keeps(no_place));
}

TEST_CASE("the exporter writes a DAT record like horcom_aaf3") {
  ChartRecord c = sample();
  c.place = "";
  c.remark = "(julian.) alter Vermerk";
  const AafRecord a = aaf_export_record(c);
  CHECK(a.place == "NICHT GENANNT !");
  CHECK(a.comment == "alter Vermerk");
  CHECK(a.zone_name == "GMT");
  // UPPER$(LEFT$(bem$,9)), a small flag counts like his capitals
  CHECK(a.calendar == Calendar::kJulian);
  c.place = "SEHR LANGER ORTSNAME*X";
  CHECK(aaf_export_record(c).place == "SEHR LANGER ORTSN");
}
