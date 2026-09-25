// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>

#include "horcom/ephem/sunmoon.hpp"

// The Moon. Ported from the original HORCOM procedures moko1 and moko.
// moko1 is his transcription of the Meeus lunar theory, 60 terms each for
// longitude, radius and latitude, evaluated together with analytic
// derivatives so the state vector carries a velocity. moko derives the
// TRUE lunar node and the true apogee, the Black Moon, from the osculating
// orbit via the angular momentum vector, a computation he annotates with
// Montenbruck page references.
namespace horcom {

// Result of the lunar position computation, all angles radians.
struct MoonPosition {
  double el = 0.0;         // apparent ecliptic longitude, includes dpsi
  double eb = 0.0;         // latitude
  double r = 0.0;          // distance in AU
  double parallax = 0.0;   // the original pm(2), equatorial horizontal parallax
  double elp = 0.0;        // the original tb(2), longitude rate per day
  double mel = 0.0;        // mean longitude as recomputed inside moko1
  std::array<double, 3> x{};  // position vector of date, AU
  std::array<double, 3> v{};  // velocity vector, AU per day
};

// Lunar node and apogee in both of the program's variants, all radians.
struct LunarPoints {
  double true_node = 0.0;         // el(11) with dpsi applied
  double true_apogee = 0.0;       // el of the Black Moon with dpsi applied
  double true_apogee_lat = 0.0;   // the original stores this latitude on the apogee slot
  double mean_node = 0.0;         // o(2) + dpsi
  double mean_node_speed = 0.0;   // the original's fixed -0.00092422029 rad per day
  double mean_apogee = 0.0;
  double mean_apogee_speed = 0.0;
  double mean_apogee_lat = 0.0;
};

/// Computes the lunar position from the Meeus series, the original moko1.
///
/// @param t time arguments of the epoch, Ephemeris Time
/// @param s the somo state of the same epoch, supplies the series
///          arguments and the nutation that makes the longitude apparent
/// @return the apparent position, distance, parallax, state vector and
///         rate
/// @note the mean longitude is recomputed here with the original's
///       combined literal, which rounds slightly differently from the
///       split form in somo, both are kept faithfully
[[nodiscard]] MoonPosition moon_position(const TimeArguments& t, const SunMoonState& s);

/// Derives true and mean lunar node and apogee, from the osculating orbit
/// part of the original moko.
///
/// @param m the lunar position with its state vector
/// @param s the somo state of the same epoch
/// @param t time arguments of the epoch
/// @return both variants, the chart layer picks by the moknw and apogw
///         settings
[[nodiscard]] LunarPoints lunar_points(const MoonPosition& m, const SunMoonState& s, const TimeArguments& t);

}  // namespace horcom
