// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

// Heliocentric to geocentric conversion, the original hel_geo with its
// helper plko12. The daily motion and its change come from a symmetric
// difference at a tenth of the daily rates, exactly like the original.
namespace horcom {

/// Heliocentric state of one body and of the Earth.
struct HelioState {
  double l = 0.0;   // heliocentric longitude, radians
  double b = 0.0;   // latitude
  double r = 0.0;   // radius, AU
  double lt = 0.0;  // rates per day
  double bt = 0.0;
  double rt = 0.0;
};

/// Result of the conversion.
struct GeoResult {
  double el = 0.0;  // geocentric ecliptic longitude with nutation applied
  double eb = 0.0;  // latitude with the original's deps term
  double dr = 0.0;  // geocentric distance, AU
  double tb = 0.0;  // daily motion, negative when retrograde
  double ttb = 0.0; // change of the daily motion
};

/// The original hel_geo.
///
/// @param body  heliocentric state of the body
/// @param earth heliocentric state of the Earth, the original slot 1
/// @param dpsi  nutation in longitude, radians
/// @param deps  nutation in obliquity, the original adds it to the
///              latitude
/// @return geocentric position, distance and motion
[[nodiscard]] GeoResult helio_to_geo(const HelioState& body, const HelioState& earth, double dpsi, double deps);

}  // namespace horcom
