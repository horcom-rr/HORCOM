// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>
#include <initializer_list>
#include <string>

#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"
#include "horcom/ephem/eph_file.hpp"
#include "horcom/ephem/pluto_chapront.hpp"
#include "horcom/ephem/sunmoon.hpp"
#include "horcom/time/calendar.hpp"

using namespace horcom;

namespace {

EphFile open_body(const char* name) {
  auto f = EphFile::open(std::string(HORCOM_TEST_DATA_DIR "/eph/") + name + ".eph");
  REQUIRE(f.has_value());
  return *f;
}

}  // namespace

TEST_CASE("file geometry of his integrated ephemerides") {
  const EphFile pluto = open_body("pluto");
  CHECK(pluto.step() == doctest::Approx(40.0));
  CHECK(pluto.jd_newest() == doctest::Approx(2525280.0));
  const EphFile chiron = open_body("chiron");
  CHECK(chiron.step() == doctest::Approx(20.0));
  const EphFile ceres = open_body("ceres");
  CHECK(ceres.step() == doctest::Approx(10.0));
  // the shipped file steps every 10 days although the generator listing
  // says 20, the reader always trusts the file
  const EphFile halley = open_body("halley");
  CHECK(halley.step() == doctest::Approx(10.0));
  // out of range requests report themselves like jdplanetex! does
  const EphBodyInfo* info = eph_body("pluto");
  REQUIRE(info != nullptr);
  CHECK_FALSE(pluto.evaluate(pluto.upper_bound() + 1.0, info->fplanet, info->frame).in_range);
  CHECK_FALSE(pluto.evaluate(pluto.lower_bound() - 1.0, info->fplanet, info->frame).in_range);
}

TEST_CASE("Pluto from the file against the Meeus example for 1992-10-13 0h TD") {
  // Meeus gives heliocentric J2000 l 232.74071, b 14.58782, r 29.711111
  const double jd = 2448908.5;
  const EphFile pluto = open_body("pluto");
  const EphBodyInfo* info = eph_body("pluto");
  REQUIRE(info != nullptr);
  const EphFile::Sample s = pluto.evaluate(jd, info->fplanet, info->frame);
  REQUIRE(s.in_range);
  // the radius is frame free and pins the whole decoding chain
  CHECK(s.r == doctest::Approx(29.711111).epsilon(2e-5));
  // the stored frame is equatorial, convert with the true obliquity of
  // date like the original does after ephem_auswert
  const TimeArguments t = time_arguments(jd);
  const SunMoonState st = somo(t, CalendarDate{13, 10, 1992, 0, 0});
  const Ecliptic ec = equatorial_to_ecliptic(s.lon, s.lat, st.ekls);
  // the sample is precessed to date, Meeus quotes J2000, the general
  // precession over 7.22 years is close to 0.1006 degrees
  const double lon_j2000 = ec.lon * kRadToDeg + 0.1006;
  CHECK(lon_j2000 == doctest::Approx(232.74071).epsilon(0).scale(0).epsilon(5e-5));
  CHECK(ec.lat * kRadToDeg == doctest::Approx(14.58782).epsilon(2e-3));
}

TEST_CASE("interpolation is continuous across a record boundary") {
  const EphFile pluto = open_body("pluto");
  const EphBodyInfo* info = eph_body("pluto");
  const double node = 2448880.5;  // a stored epoch, integer day multiple of 40
  const EphFile::Sample a = pluto.evaluate(node - 0.25, info->fplanet, info->frame);
  const EphFile::Sample b = pluto.evaluate(node + 0.25, info->fplanet, info->frame);
  REQUIRE(a.in_range);
  REQUIRE(b.in_range);
  CHECK(std::abs(a.lon - b.lon) * kRadToDeg < 0.01);
  CHECK(std::abs(a.r - b.r) < 0.001);
}

TEST_CASE("the Chapront fallback meets the integrated file") {
  // inside the file span both sources describe the same planet, the
  // analytic theory is good to a few arc minutes there
  const double jd = 2448908.5;
  const TimeArguments t = time_arguments(jd);
  const SunMoonState st = somo(t, CalendarDate{13, 10, 1992, 0, 0});
  const ChaprontPluto cp = pluto_chapront(t, st.ekls);
  const EphFile pluto = open_body("pluto");
  const EphBodyInfo* info = eph_body("pluto");
  const EphFile::Sample s = pluto.evaluate(jd, info->fplanet, info->frame);
  const Ecliptic ec = equatorial_to_ecliptic(s.lon, s.lat, st.ekls);
  CHECK(std::abs(std::remainder(cp.hel - ec.lon, kTwoPi)) * kRadToDeg < 0.1);
  CHECK(std::abs(cp.heb - ec.lat) * kRadToDeg < 0.05);
  CHECK(cp.r == doctest::Approx(s.r).epsilon(2e-3));
}

TEST_CASE("Chiron stays on a sane heliocentric arc") {
  //RR Extreme Radien von CHIRON etwa zwischen 8.45 und 18.864
  const EphFile chiron = open_body("chiron");
  const EphBodyInfo* info = eph_body("chiron");
  REQUIRE(info != nullptr);
  for (double jd : {2415020.5, 2440400.5, 2448908.5}) {
    CAPTURE(jd);
    const EphFile::Sample s = chiron.evaluate(jd, info->fplanet, info->frame);
    REQUIRE(s.in_range);
    CHECK(s.r > 8.4);
    CHECK(s.r < 18.9);
  }
}
