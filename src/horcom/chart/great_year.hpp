// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

// The age point of the great, the Platonic year. Ported from the original
// HORCOM procedure grossj1. A point of the ecliptic chosen at a reference
// date is carried by the precession to the chart date, the age point
// runs backward through the signs by the amount the fixed direction
// gained in longitude.
namespace horcom {

/// Where the age point stands at the chart date.
struct GreatYearPoint {
  /// his di, the reference longitude minus the precessed one in degrees,
  /// folded into the half open range (-180, 180]
  double di_deg = 0.0;
  /// the age point, ng(zal + di) in degrees
  double point_deg = 0.0;
  /// his NICHT MEHR im ... ZEITALTER warning, the point lies before the
  /// reference or more than one sign behind it
  bool outside = false;
};

/// Carries the start of an age from the reference date to the chart date.
///
/// @param jd      the chart date as Julian day, his jd of the record
/// @param tja     the tropical year in days of the chart's time arguments
/// @param ekls    the obliquity of the chart in radians
/// @param jd_ref  the reference date, his jdgross
/// @param zal_deg the start of the age in whole degrees, 30, 360, 330 or
///                300 in his box
/// @return the difference, the age point and the warning flag
[[nodiscard]] GreatYearPoint great_year_point(double jd, double tja, double ekls, double jd_ref, int zal_deg);

}  // namespace horcom
