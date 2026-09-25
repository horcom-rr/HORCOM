// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <numbers>

/// @file
/// Shared constants. The angle factors are ported from the original HORCOM
/// procedure funkt, original variable names are noted so formulas stay
/// literally comparable. Epochs and unit conversions collect the recurring
/// values of the port, one off series coefficients stay literal in their
/// routines.
namespace horcom {

inline constexpr double kPi = std::numbers::pi;
inline constexpr double kHalfPi = kPi / 2.0;            // original po
inline constexpr double kTwoPi = 2.0 * kPi;             // original pv2
inline constexpr double kDegToRad = kPi / 180.0;        // original pu
inline constexpr double kRadToDeg = 180.0 / kPi;        // original up
inline constexpr double kArcsecToRad = kDegToRad / 3600.0;   // original puu
inline constexpr double kDegPerCenturyToRad = kDegToRad / 36525.0;  // original pup

/// original kk, the epsilon Robert Rettig adds exactly where a division or
/// an ATN argument could hit zero. Guards are placed only where the
/// listing places them, never generally.
inline constexpr double kEps = 1.0e-10;

/// epoch of the original time argument t1, Julian date of 1900 January 0.5
inline constexpr double kJdEpoch1900 = 2415020.0;

/// epoch J2000, Julian date of 2000 January 1.5
inline constexpr double kJdJ2000 = 2451545.0;

//RR Jan. 0.923,1950 =1950.0
inline constexpr double kJdB1950 = 2433282.423;

/// epoch of the Newcomb precession in the original praez
inline constexpr double kJdBessel1900 = 2415020.313;

inline constexpr double kDaysPerCentury = 36525.0;
inline constexpr double kDaysPerMillennium = 365250.0;

/// the original's kilometre value of one astronomical unit
inline constexpr double kKmPerAu = 149600000.0;

/// solar to sidereal rate with the original's rounding
inline constexpr double kSolarToSiderealRate = 1.002737908;

inline constexpr double kDegPerHour = 15.0;
inline constexpr double kDegPerSign = 30.0;
inline constexpr double kDegPerCircle = 360.0;

inline constexpr double kDegPerQuadrant = 90.0;
inline constexpr double kArcminPerDeg = 60.0;
inline constexpr double kArcsecPerDeg = 3600.0;
inline constexpr double kMonthsPerYear = 12.0;

/// the tropical month in days, his tmo, the Moon's return to the same
/// longitude
inline constexpr double kTropicalMonthDays = 27.321582;

/// civil hours of one solar day, the divisor from clock hours to day
/// fraction that the panel, the transits and the progressions all lean on
inline constexpr double kHoursPerDay = 24.0;

/// civil minutes of one solar day, one over this converts minute-of-day
/// to day fraction, the second half of Robert Rettig's julian_day formula
inline constexpr double kMinutesPerDay = 1440.0;

/// per body orb weights and house orb percentages count in percent
inline constexpr double kPercent = 100.0;

/// the default aspect orb is the base angle over thirty, his pn / 30
inline constexpr double kDefaultOrbDivisor = 30.0;

/// his w4d default, the GRUNDWINKEL of the transit and direction runs in
/// degrees, the multiples of it are the aspects searched
inline constexpr double kDefaultBaseAngleDeg = 30.0;

/// seconds of one clock hour, the time twin of kArcsecPerDeg
inline constexpr double kSecondsPerHour = 3600.0;

/// the zone text of Greenwich time in his AAF field, 00 hours east
inline constexpr const char* kUtZoneText = "00hE00:00";

/// clock seconds of one civil day, the clamp ceiling of the panels
inline constexpr int kSecondsPerDay = 86400;

/// radians of nutation to hours of right ascension, the original's
/// rounded twelve over pi
inline constexpr double kRadToRaHours = 3.8197186;

}  // namespace horcom
