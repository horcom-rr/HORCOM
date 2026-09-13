// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

// The elliptical orbit solver. Ported from the original HORCOM procedure
// kepler, which annotates its two stages itself, the Odell starting value
// and the Meeus iteration. All angles radians.
namespace horcom {

// One body's orbital elements as the original element arrays hold them.
// p is the argument of perihelion when kepler runs, some callers store the
// longitude of perihelion and subtract o first, exactly like the original.
struct Orbit {
  double a = 0.0;    // semi major axis, AU
  double e = 0.0;    // eccentricity
  double i = 0.0;    // inclination
  double o = 0.0;    // ascending node
  double p = 0.0;    // argument of perihelion
  double mel = 0.0;  // mean longitude
  double man = 0.0;  // mean anomaly
};

// Position in the orbit after solving the Kepler equation.
struct OrbitPosition {
  double v = 0.0;    // true anomaly
  double r = 0.0;    // radius, AU
  double ean = 0.0;  // eccentric anomaly
  double u = 0.0;    // argument of latitude
  double hel = 0.0;  // heliocentric longitude
  double heb = 0.0;  // heliocentric latitude
};

/// Solves the Kepler equation and derives the heliocentric position.
///
/// @param orbit elements with man set and p as argument of perihelion
/// @return the orbit position, the two correction branches that pull u
///         back by pi when the longitude quadrant disagrees are ported
///         verbatim
[[nodiscard]] OrbitPosition kepler(const Orbit& orbit);

}  // namespace horcom
