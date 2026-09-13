// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include <vector>

#include "doctest.h"
#include "horcom/ephem/eclipses.hpp"
#include "horcom/time/calendar.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/ephem/moon.hpp"
#include "horcom/ephem/sunmoon.hpp"

using namespace horcom;

TEST_CASE("nutation against the Meeus example for 1987-04-10") {
  // Meeus gives dpsi -3.788 arcsec and deps +9.443 arcsec from the full
  // series, the original carries the largest terms and lands within half
  // an arcsecond
  const TimeArguments t = time_arguments(2446895.5);
  const SunMoonState s = somo(t, CalendarDate{10, 4, 1987, 0, 0});
  CHECK(std::abs(s.dpsi / kArcsecToRad - (-3.788)) < 0.5);
  CHECK(std::abs(s.deps / kArcsecToRad - 9.443) < 0.5);
  // true obliquity 23 deg 26' 36.85"
  CHECK(s.ekls * kRadToDeg == doctest::Approx(23.443569).epsilon(2e-5));
}

TEST_CASE("the equation of time around early November is near its maximum") {
  // about plus 16.4 minutes on 1992-11-03, Meeus example value 13.42277
  // degrees of hour angle equals 3.427 degrees... the check runs in
  // minutes of time, 4 minutes per degree
  const TimeArguments t = time_arguments(2448929.5);
  const SunMoonState s = somo(t, CalendarDate{3, 11, 1992, 0, 0});
  const double minutes = equation_of_time(s) * kRadToDeg * 4.0;
  CHECK(minutes == doctest::Approx(16.42).epsilon(0.02));
}

TEST_CASE("Moon against the Meeus example for 1992-04-12 0h TD") {
  const TimeArguments t = time_arguments(2448724.5);
  const SunMoonState s = somo(t, CalendarDate{12, 4, 1992, 0, 0});
  const MoonPosition m = moon_position(t, s);
  // geometric longitude 133.162655 degrees, the apparent value carries
  // dpsi like the original el(2)
  CHECK((m.el - s.dpsi) * kRadToDeg == doctest::Approx(133.162655).epsilon(2e-6));
  // latitude -3.229126 degrees, the original folds deps into eb(2)
  CHECK((m.eb - s.deps) * kRadToDeg == doctest::Approx(-3.229126).epsilon(5e-5));
  // distance 368409.7 km
  CHECK(m.r * 149600000.0 == doctest::Approx(368409.7).epsilon(1e-6));
  // equatorial horizontal parallax 0.991990 degrees
  CHECK(m.parallax * kRadToDeg == doctest::Approx(0.991990).epsilon(1e-4));
}

TEST_CASE("lunar velocity vector matches a numerical difference") {
  const double jd = 2448724.5;
  const TimeArguments t = time_arguments(jd);
  const SunMoonState s = somo(t, CalendarDate{12, 4, 1992, 0, 0});
  const MoonPosition m = moon_position(t, s);
  const double h = 0.05;
  const TimeArguments ta = time_arguments(jd - h);
  const TimeArguments tb = time_arguments(jd + h);
  const MoonPosition ma = moon_position(ta, somo(ta, CalendarDate{12, 4, 1992, 0, 0}));
  const MoonPosition mb = moon_position(tb, somo(tb, CalendarDate{12, 4, 1992, 0, 0}));
  for (int i = 0; i < 3; ++i) {
    CAPTURE(i);
    const double num = (mb.x[static_cast<std::size_t>(i)] - ma.x[static_cast<std::size_t>(i)]) / (2.0 * h);
    CHECK(m.v[static_cast<std::size_t>(i)] == doctest::Approx(num).epsilon(5e-3));
  }
}

TEST_CASE("true node and apogee stay near their mean counterparts") {
  // the true node librates up to about 1.7 degrees around the mean node,
  // the true apogee up to roughly 30 degrees around the mean apogee
  const TimeArguments t = time_arguments(2448724.5);
  const SunMoonState s = somo(t, CalendarDate{12, 4, 1992, 0, 0});
  const MoonPosition m = moon_position(t, s);
  const LunarPoints p = lunar_points(m, s, t);
  const double node_diff = std::remainder(p.true_node - p.mean_node, kTwoPi) * kRadToDeg;
  CHECK(std::abs(node_diff) < 2.0);
  const double apogee_diff = std::remainder(p.true_apogee - p.mean_apogee, kTwoPi) * kRadToDeg;
  CHECK(std::abs(apogee_diff) < 35.0);
  CHECK(p.mean_node_speed == doctest::Approx(-0.00092422029));
}

TEST_CASE("the lunation series finds the 1999 total eclipse") {
  // the new moons of the summer of 1999, the August one darkened
  // central europe on the eleventh at 11:08 UT
  const std::vector<Lunation> nm = lunations(julian_day({1, 7, 1999, 0, 0.0}), 4, false);
  bool found = false;
  for (const Lunation& l : nm) {
    const CalendarDate d = calendar_date(l.jd_ut);
    if (d.year == 1999 && d.month == 8 && d.day == 11) {
      found = true;
      CHECK(l.eclipse);
      CHECK(l.kind == "ZT TOT N");
      CHECK(d.hour * 60.0 + d.minute == doctest::Approx(11.0 * 60 + 8).epsilon(0.01));
    }
  }
  CHECK(found);
  // the full moon of 2000 January 21 sank into the umbra at 4:44 UT
  const std::vector<Lunation> fm = lunations(julian_day({1, 1, 2000, 0, 0.0}), 3, true);
  bool umbral = false;
  for (const Lunation& l : fm) {
    const CalendarDate d = calendar_date(l.jd_ut);
    if (d.year == 2000 && d.month == 1 && d.day == 21) {
      umbral = l.kind == "KERNSCH";
      CHECK(d.hour * 60.0 + d.minute == doctest::Approx(4.0 * 60 + 44).epsilon(0.02));
    }
  }
  CHECK(umbral);
}
