// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

// Calendar and Julian date conversion. Ported from the original HORCOM
// procedures juld, juld1 and dat.
//
// The original keeps a per-chart calendar flag in jul$ containing
// "(JULIAN.)" or "(GREGOR.)". Auto means the historical rule, Julian
// calendar before 1582-10-15 and Gregorian from then on. The overrides
// force one calendar across the boundary, Gregorian even makes dates
// before 1582 proleptic Gregorian.
namespace horcom {

enum class Calendar { kAuto, kJulian, kGregorian };

// Year counting is astronomical, year 0 exists and -500 means 501 BC.
// hour and minute stay double, the storage format carries seconds as a
// fractional minute.
struct CalendarDate {
  int day = 0;
  int month = 0;
  int year = 0;
  double hour = 0.0;
  double minute = 0.0;
};

/// The original juld, calendar date to Julian day.
///
/// @param d   the date, astronomical year counting
/// @param cal calendar rule, kAuto follows the 1582 reform
/// @return the Julian day, valid over the whole original range including
///         negative years via the FIX(365.25 * y - 0.75) branch
[[nodiscard]] double julian_day(const CalendarDate& d, Calendar cal = Calendar::kAuto);

/// The original dat, the exact inverse of julian_day.
///
/// @param jd  Julian day, domain jd >= 0 like the original
/// @param cal calendar rule matching the one used on the way in
/// @return the calendar date with integral day and hour
[[nodiscard]] CalendarDate calendar_date(double jd, Calendar cal = Calendar::kAuto);

/// The original juld1 without its trailing somo call, which belongs to
/// the ephemeris module. Time arguments for every downstream formula.
struct TimeArguments {
  double jd = 0.0;
  // centuries since 1900.0 (epoch JD 2415020) and powers, original t1..t4
  double t1 = 0.0;
  double t2 = 0.0;
  double t3 = 0.0;
  double t4 = 0.0;
  // centuries since J2000 (epoch JD 2451545) and powers, original t11..t41
  double t11 = 0.0;
  double t21 = 0.0;
  double t31 = 0.0;
  double t41 = 0.0;
  // original tja, time dependent tropical year in days
  double tropical_year_days = 0.0;
  // original ekls directly after juld1, the MEAN obliquity in radians.
  // somo later adds the nutation deps, only then ekls is the true obliquity.
  double mean_obliquity_rad = 0.0;
};

/// Computes the time arguments of an epoch.
///
/// @param jd Julian day of the epoch
/// @return powers of centuries from 1900 and J2000, the tropical year and
///         the mean obliquity
[[nodiscard]] TimeArguments time_arguments(double jd);

}  // namespace horcom
