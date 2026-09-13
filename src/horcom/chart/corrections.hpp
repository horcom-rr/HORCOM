// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/chart/settings.hpp"
#include "horcom/core/coords.hpp"

// The correction chain a body passes after its geocentric position is
// known, the original par_ap_ktr. plkoap applies light time and optional
// aberration, plkotr converts to equatorial, and par is Robert Rettig's
// topocentric parallax, PROTECTED logic, ported faithfully from his
// Montenbruck formulation and changed only under reference tests.
namespace horcom {

/// One body's position while it runs through the correction chain.
struct BodyPosition {
  double el = 0.0;   // geocentric ecliptic longitude, apparent after plkoap
  double eb = 0.0;   // latitude
  double ar = 0.0;   // right ascension
  double de = 0.0;   // declination
  double dr = 0.0;   // geocentric distance, AU
  double tb = 0.0;   // daily motion, retrograde when negative
};

/// The original plkoap, apparent position.
///
/// @param p    body position with el, eb, tb and dr set
/// @param mode the original appa&, light time, light time with
///             aberration, or the true position
/// @param sun_el  apparent longitude of the Sun for the aberration branch
/// @param sun_eb  latitude term the original feeds the same branch
void apparent_position(BodyPosition& p, ApparentMode mode, double sun_el, double sun_eb);

/// The original plkotr, fills ar and de from el and eb.
///
/// @param p    body position
/// @param ekls true obliquity of date
void to_equatorial(BodyPosition& p, double ekls);

//RR aus Montenbruck S.25  Ohne Ber. d.Meereshöhe
/// The original par, the topocentric parallax. PROTECTED.
///
/// Converts the equatorial place from the geocentre to the event location
/// on a spherical Earth and writes the result back to both coordinate
/// pairs. The original applies it to slots 1 through 10 only, the caller
/// enforces that rule.
///
/// @param p        body position with ar, de and dr set
/// @param lat_deg  geographic latitude of the event location, degrees
/// @param armc_deg local sidereal time as right ascension of the
///                 midheaven, degrees like the original global armc
/// @param ekls     true obliquity of date for the way back to ecliptic
void parallax(BodyPosition& p, double lat_deg, double armc_deg, double ekls);

}  // namespace horcom
