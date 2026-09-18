// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/gfa_stream.hpp"
#include "horcom/data/konsta.hpp"

using namespace horcom;

TEST_CASE("the GFA reader ignores line boundaries and keeps commas in quotes") {
  GfaReader r("1,\"Placidus\",2\r\n\" Ephem ::App.1,MitParall.\",-1\r\n3.5");
  CHECK(r.next_int() == 1);
  CHECK(r.next_string() == "Placidus");
  CHECK(r.next_int() == 2);
  CHECK(r.next_string() == " Ephem ::App.1,MitParall.");
  CHECK(r.next_bool());
  CHECK(r.next_number() == doctest::Approx(3.5));
  CHECK(r.remaining() == 0);
}

TEST_CASE("KONSTA round trips through format and parse") {
  Konsta k;
  k.haw = 3;
  k.haus = "Koch-GOH";
  k.appa = 2;
  k.par = 1.0;
  k.apogw = true;
  k.stzw = 1;
  k.orbe_on = true;
  for (int i = 0; i <= 14; ++i) {
    k.orb_text[static_cast<std::size_t>(i)] = " 5.40";
  }
  k.or_weight[5] = 80;
  k.nk[1] = 19;
  k.nk[2] = 20;
  k.gena = " Ephem ::App.1,MitParall.";
  const std::string text = format_konsta(k);
  const Konsta back = parse_konsta(text);
  CHECK(back.haw == 3);
  CHECK(back.haus == "Koch-GOH");
  CHECK(back.appa == 2);
  CHECK(back.par == doctest::Approx(1.0));
  CHECK(back.apogw);
  CHECK(back.orbe_on);
  CHECK(back.orb_text[3] == " 5.40");
  CHECK(back.or_weight[5] == 80);
  CHECK(back.nk[1] == 19);
  CHECK(back.nk[2] == 20);
  CHECK(back.gena == " Ephem ::App.1,MitParall.");
}

TEST_CASE("KONSTA maps onto the pipeline settings like kon_dhol") {
  Konsta k;
  k.haw = 1;
  k.par = 1.0;
  k.apogw = true;
  k.moknw = true;
  k.orbe_on = true;
  k.nasp = 16;  // the original folds 16 to 12 in equal probability mode
  for (int i = 0; i <= 14; ++i) {
    k.orb_text[static_cast<std::size_t>(i)] = "2.0";
  }
  k.nk[2] = 20;
  const ChartSettings s = k.chart_settings();
  CHECK(s.topocentric_parallax);
  CHECK(s.true_apogee);
  CHECK(s.true_node);
  CHECK(s.extra_bodies);
  CHECK(s.nk[2] == 20);
  const AspectSettings a = k.aspect_settings();
  CHECK(a.equal_probability);
  CHECK(a.divisors == 12);
  CHECK(a.orbe[7] == doctest::Approx(2.0 * kDegToRad));
}

TEST_CASE("his profile carries Robert Rettig's switches and round trips") {
  const Konsta k = robert_profile();
  CHECK(k.haw == 1);
  CHECK(k.haus == "Placidus");
  CHECK(k.orb == doctest::Approx(1.0));
  CHECK(k.nasp == 12);
  const ChartSettings s = k.chart_settings();
  // he ran with the topocentric parallax on, true node and true apogee
  CHECK(s.topocentric_parallax);
  CHECK(s.true_node);
  CHECK(s.true_apogee);
  CHECK(s.apparent_sidereal);
  CHECK_FALSE(s.extra_bodies);
  const AspectSettings a = k.aspect_settings();
  // his own orb table in the equal probability mode, his weights
  CHECK(a.equal_probability);
  CHECK(a.divisors == 12);
  CHECK(a.orbe[1] == doctest::Approx(5.40 * kDegToRad));
  CHECK(a.orbe[11] == doctest::Approx(0.60 * kDegToRad));
  CHECK(a.weight[1] == 150);
  // Merkur trägt jetzt wieder 100 statt der versehentlich in KONSTA7P.INT
  // stehen gebliebenen 1, so wie es die weiteren Fassungen KONSTA5P.INT
  // und KONSTA8P.INT ohnehin vorgaben
  CHECK(a.weight[3] == 100);
  const Konsta back = parse_konsta(format_konsta(k));
  CHECK(format_konsta(back) == format_konsta(k));
}

TEST_CASE("the zone field composes like zeitzon") {
  CHECK(aaf_zone(0.0) == "00hE00:00");
  CHECK(aaf_zone(1.0) == "01hE00:00");
  // Newfoundland west three and a half hours
  CHECK(aaf_zone(-3.5) == "03hW30:00");
  CHECK(aaf_zone(5.75) == "05hE45:00");
}

TEST_CASE("AAF parses a synthetic record with every quirk") {
  const char* text =
      "~ eine Zeile mit Tilde wird komplett verworfen\r\n"
      "#A93:Musterfrau,Erika,w,03.11.1948,21h55:00,Teststadt            ,D\r\n"
      "#B93: 2432859.3715,47N38:00,007E40:00,01hE00:00,*\r\n"
      "#COM:erster Teil\r\n"
      "und die Fortsetzung\r\n"
      "#GZQ:AA\r\n"
      "#A93:Zweitfall,*,m,29.02.1948j,06:30:15,Altort,A\r\n"
      "<b>#B93:</b>      0.00000,12S05:30,120W15:00,*,2\r\n";
  const std::vector<AafRecord> recs = parse_aaf(text);
  REQUIRE(recs.size() == 2);
  const AafRecord& a = recs[0];
  CHECK(a.surname == "Musterfrau");
  CHECK(a.given == "Erika");
  CHECK(a.sex == "w");
  CHECK(a.day == 3);
  CHECK(a.month == 11);
  CHECK(a.year == 1948);
  CHECK(a.calendar == Calendar::kAuto);
  CHECK(a.hour == 21);
  CHECK(a.minute == 55);
  CHECK(a.second == 0);
  CHECK(a.place == "Teststadt");
  CHECK(a.country == "D");
  CHECK(a.jd == doctest::Approx(2432859.3715));
  CHECK(a.latitude() == doctest::Approx(47.0 + 38.0 / 60.0));
  CHECK(a.longitude() == doctest::Approx(7.0 + 40.0 / 60.0));
  CHECK(a.zone == "01hE00:00");
  CHECK(a.dst.empty());
  CHECK(a.comment == "erster Teil und die Fortsetzung");
  CHECK(a.quality == "AA");
  const AafRecord& b = recs[1];
  CHECK(b.given.empty());
  CHECK(b.calendar == Calendar::kJulian);
  CHECK(b.hour == 6);
  CHECK(b.minute == 30);
  CHECK(b.second == 15);
  CHECK(b.jd == doctest::Approx(0.0));
  CHECK(b.latitude() == doctest::Approx(-(12.0 + 5.0 / 60.0 + 30.0 / 3600.0)));
  CHECK(b.longitude() == doctest::Approx(-(120.0 + 15.0 / 60.0)));
  CHECK(b.dst == "2");
}

TEST_CASE("AAF round trips through format and parse") {
  AafRecord r;
  r.surname = "Musterfrau";
  r.given = "Erika";
  r.sex = "w";
  r.day = 3;
  r.month = 11;
  r.year = 1948;
  r.hour = 21;
  r.minute = 55;
  r.second = 0;
  r.place = "Teststadt";
  r.country = "D";
  r.jd = 2432859.41319;
  r.lat_deg = 47;
  r.lat_min = 38;
  r.lat_ns = 'N';
  r.lon_deg = 7;
  r.lon_ew = 'E';
  r.lon_min = 40;
  r.zone = "01hE00:00";
  r.comment = "synthetischer Prüffall mit Umlauten äöü";
  const std::string text = format_aaf({r});
  CHECK(text.find("#A93:Musterfrau,Erika,w,03.11.1948,21h55:00,Teststadt,D") != std::string::npos);
  CHECK(text.find("#B93:2432859.41319,47N38:00,007E40:00,01hE00:00,*") != std::string::npos);
  const std::vector<AafRecord> back = parse_aaf(text);
  REQUIRE(back.size() == 1);
  CHECK(back[0].surname == r.surname);
  CHECK(back[0].jd == doctest::Approx(r.jd));
  CHECK(back[0].comment == r.comment);
  CHECK(back[0].lon_deg == 7);
}
