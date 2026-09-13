// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/time/calendar.hpp"

using namespace horcom;

namespace {

struct Sample {
  CalendarDate d;
  double jd;
};

// worked examples from Jean Meeus, Astronomische Algorithmen chapter 7,
// the book Robert Rettig cites as the source of his formulas
const Sample kSamples[] = {
    {{4, 10, 1957, 19, 26.4}, 2436116.31},
    {{27, 1, 333, 12, 0}, 1842713.0},
    {{1, 1, 2000, 12, 0}, 2451545.0},
    {{1, 1, 1999, 0, 0}, 2451179.5},
    {{27, 1, 1987, 0, 0}, 2446822.5},
    {{19, 6, 1987, 12, 0}, 2446966.0},
    {{19, 6, 1988, 12, 0}, 2447332.0},
    {{1, 1, 1900, 0, 0}, 2415020.5},
    {{1, 1, 1600, 0, 0}, 2305447.5},
    {{31, 12, 1600, 0, 0}, 2305812.5},
    {{10, 4, 837, 7, 12}, 2026871.8},
    {{31, 12, -123, 0, 0}, 1676496.5},
    {{1, 1, -122, 0, 0}, 1676497.5},
    {{12, 7, -1000, 12, 0}, 1356001.0},
    {{29, 2, -1000, 0, 0}, 1355866.5},
    {{17, 8, -1001, 21, 36}, 1355671.4},
    {{1, 1, -4712, 12, 0}, 0.0},
};

}  // namespace

TEST_CASE("julian_day reproduces the Meeus examples") {
  for (const auto& s : kSamples) {
    CAPTURE(s.d.year);
    CAPTURE(s.d.month);
    CAPTURE(s.d.day);
    CHECK(julian_day(s.d) == doctest::Approx(s.jd).epsilon(1e-12));
  }
}

TEST_CASE("calendar_date inverts julian_day") {
  for (const auto& s : kSamples) {
    CAPTURE(s.jd);
    const CalendarDate back = calendar_date(s.jd);
    CHECK(back.day == s.d.day);
    CHECK(back.month == s.d.month);
    CHECK(back.year == s.d.year);
    CHECK(back.hour == doctest::Approx(s.d.hour));
    CHECK(back.minute == doctest::Approx(s.d.minute).scale(60.0));
  }
}

TEST_CASE("the Gregorian reform boundary") {
  CHECK(julian_day({4, 10, 1582, 0, 0}) == doctest::Approx(2299159.5));
  CHECK(julian_day({15, 10, 1582, 0, 0}) == doctest::Approx(2299160.5));
}

TEST_CASE("calendar overrides shift dates as the jul$ flag does") {
  // Julian 1990-03-01 is Gregorian 1990-03-14, both name the same day
  CHECK(julian_day({1, 3, 1990, 0, 0}, Calendar::kJulian) ==
        doctest::Approx(julian_day({14, 3, 1990, 0, 0})));
  // proleptic Gregorian before 1582 round-trips under the same override
  const CalendarDate d{1, 7, 1500, 6, 30};
  const double jd = julian_day(d, Calendar::kGregorian);
  const CalendarDate back = calendar_date(jd, Calendar::kGregorian);
  CHECK(back.day == d.day);
  CHECK(back.month == d.month);
  CHECK(back.year == d.year);
}

TEST_CASE("time_arguments matches juld1") {
  const TimeArguments t = time_arguments(2451545.0);
  CHECK(t.t1 == doctest::Approx(1.0).epsilon(1e-12));
  CHECK(t.t11 == doctest::Approx(t.t1 - 1.0));
  CHECK(t.t2 == doctest::Approx(t.t1 * t.t1));
  // mean obliquity for J2000 near 23.439 degrees
  CHECK(t.mean_obliquity_rad * 180.0 / 3.14159265358979323846 == doctest::Approx(23.4392).epsilon(1e-4));
  CHECK(t.tropical_year_days == doctest::Approx(365.24219879 - 6.14e-06 * t.t1));
}
