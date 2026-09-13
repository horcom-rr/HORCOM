// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>

// Precession. Ported from the original HORCOM procedures praez,
// praez_kartes_aeqn and praez_kartes_ekln. The cartesian pair rotates the
// stored ephemeris vectors from their catalogue equinox to the epoch of
// date and carries Robert Rettig's own attribution,
//RR nach MONT. 2. S 18  Elemente nach MEEUS
// while praez is the older Newcomb formulation referred to 1900.0 that the
// main program uses for spherical coordinates.
namespace horcom {

/// Rotates an equatorial cartesian vector between equinoxes.
///
/// The original praez_kartes_aeqn with the IAU 1976 angles.
///
/// @param x     vector to rotate in place
/// @param jdn   target epoch as Julian date
/// @param jdaeq catalogue equinox as Julian date
void precess_equatorial(std::array<double, 3>& x, double jdn, double jdaeq);

/// Rotates an ecliptic cartesian vector between equinoxes.
///
/// The original praez_kartes_ekln.
///
/// @param x     vector to rotate in place
/// @param jdn   target epoch as Julian date
/// @param jdaeq catalogue equinox as Julian date
void precess_ecliptic(std::array<double, 3>& x, double jdn, double jdaeq);

/// Precesses spherical equatorial coordinates with the Newcomb angles.
///
/// The original praez, referred to epoch 1900.0. It divides by the time
/// dependent tropical year length, so that value travels as a parameter.
///
/// @param jd       target epoch as Julian date
/// @param tja      tropical year in days from the time arguments
/// @param jda      source epoch as Julian date
/// @param ar0      right ascension at the source epoch, radians
/// @param de0      declination at the source epoch, radians
/// @param ar       receives the right ascension at the target epoch
/// @param de       receives the declination at the target epoch
void precess_newcomb(double jd, double tja, double jda, double ar0, double de0, double& ar, double& de);

}  // namespace horcom
