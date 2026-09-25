// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <string>

#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"
#include "horcom/ephem/eph_file.hpp"
#include "horcom/ephem/precession.hpp"
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

namespace {

double angle_between(const std::array<double, 3>& a, const std::array<double, 3>& b) {
  const double dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
  const double na = std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
  const double nb = std::sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
  return std::acos(std::clamp(dot / (na * nb), -1.0, 1.0));
}

// the numeric derivative of the interpolated longitude, the yardstick
// for the interpolated velocity
double lon_rate(const EphFile& f, const EphBodyInfo& info, double jd) {
  constexpr double kHalfStep = 0.01;
  const EphFile::Sample a = f.evaluate(jd - kHalfStep, info.fplanet, info.frame);
  const EphFile::Sample b = f.evaluate(jd + kHalfStep, info.fplanet, info.frame);
  double d = b.lon - a.lon;
  if (d > kPi) {
    d -= kTwoPi;
  } else if (d < -kPi) {
    d += kTwoPi;
  }
  return d / (2.0 * kHalfStep);
}

}  // namespace

TEST_CASE("Halley's shipped file is B1950 like the reader's CASE n2&,n18&") {
  // the start elements of elem_halley, Montenbruck page 165, B1950,
  // osculating at 10.02.1986, give the direction of the perihelion
  const double inc = 162.2384 * kDegToRad;
  const double node = 58.1540 * kDegToRad;
  const double peri = 111.8570 * kDegToRad;
  std::array<double, 3> p = {
      std::cos(node) * std::cos(peri) - std::sin(node) * std::sin(peri) * std::cos(inc),
      std::sin(node) * std::cos(peri) + std::cos(node) * std::sin(peri) * std::cos(inc),
      std::sin(peri) * std::sin(inc)};
  const EphFile halley = open_body("halley");
  const EphBodyInfo* info = eph_body("halley");
  REQUIRE(info != nullptr);
  CHECK(info->frame == EphFrame::kEclipticB1950);
  // the perihelion passage of the file, the smallest radius
  double best_jd = 0.0;
  double best_r = 1.0e9;
  for (double jd = 2446465.0; jd <= 2446476.0; jd += 0.01) {
    const EphFile::Sample s = halley.evaluate(jd, info->fplanet, info->frame);
    REQUIRE(s.in_range);
    if (s.r < best_r) {
      best_r = s.r;
      best_jd = jd;
    }
  }
  //RR q = 0.587157
  CHECK(best_r == doctest::Approx(0.587157).epsilon(2e-4));
  precess_ecliptic(p, best_jd, kJdB1950);
  const EphFile::Sample right = halley.evaluate(best_jd, info->fplanet, EphFrame::kEclipticB1950);
  const EphFile::Sample wrong = halley.evaluate(best_jd, info->fplanet, EphFrame::kEclipticJ2000);
  // read as B1950 the file meets his elements, the J2000 reading the
  // generator listing suggests lands about 0.7 degrees away, fifty years
  // of precession. The duplicated CASE looked like an editing accident,
  // the shipped data proves the reader right
  CHECK(angle_between(right.xyz, p) * kRadToDeg < 0.05);
  CHECK(angle_between(wrong.xyz, p) * kRadToDeg > 0.6);
}

TEST_CASE("the interpolated rates follow the positions with their sign") {
  // Ceres steps every ten days, the original's yip = jdip - djd / 2 read
  // the velocity four and a half steps away and wrapped it in ABS
  const EphFile ceres = open_body("ceres");
  const EphBodyInfo* ci = eph_body("ceres");
  REQUIRE(ci != nullptr);
  for (double jd = 2451545.0; jd < 2451545.0 + 2000.0; jd += 37.0) {
    const EphFile::Sample s = ceres.evaluate(jd, ci->fplanet, ci->frame);
    REQUIRE(s.in_range);
    CAPTURE(jd);
    // first differences at the interval midpoints plus the quadratic
    // interpolation carry about 2e-4 of relative error at a ten day step,
    // the far extrapolation of the original was off by whole percents
    CHECK(s.lont == doctest::Approx(lon_rate(ceres, *ci, jd)).epsilon(5e-4));
  }
  // Halley runs retrograde, its heliocentric longitude falls, and the
  // radius shrinks on the way in to the perihelion of 9 February 1986
  const EphFile halley = open_body("halley");
  const EphBodyInfo* hi = eph_body("halley");
  REQUIRE(hi != nullptr);
  const EphFile::Sample in = halley.evaluate(2446440.5, hi->fplanet, hi->frame);
  REQUIRE(in.in_range);
  CHECK(in.lont < 0.0);
  CHECK(in.lont == doctest::Approx(lon_rate(halley, *hi, 2446440.5)).epsilon(5e-3));
  CHECK(in.rt < 0.0);
  const EphFile::Sample out = halley.evaluate(2446500.5, hi->fplanet, hi->frame);
  CHECK(out.rt > 0.0);
  // the rates come from one helper shared with the equatorial rotation
  const SphericalRates r = spherical_rates({1.0, 0.0, 0.0}, {0.0, 0.01, -0.02});
  CHECK(r.lont == doctest::Approx(0.01));
  CHECK(r.latt == doctest::Approx(-0.02));
  CHECK(r.rt == doctest::Approx(0.0));
}
