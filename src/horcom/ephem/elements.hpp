// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/ephem/kepler.hpp"
#include "horcom/time/calendar.hpp"

// Mean orbital elements. Ported from the original HORCOM procedure plelem,
// which carries Robert Rettig's own attribution, 'Elemente nach MEEUS.
// Body numbering follows the original slots, 1 Earth standing in for the
// Sun, 3 Mercury, 4 Venus, 5 Mars, 6 Jupiter, 7 Saturn, 8 Uranus,
// 9 Neptune. The extra factors of the Hamburg school and Transpluto come
// from his fixed element tables.
namespace horcom {

/// Mean elements for a classical body.
///
/// @param body original slot number, 1 or 3 through 9
/// @param t    time arguments, the polynomials run on centuries from J2000
/// @return elements with p as LONGITUDE of perihelion like the original
///         arrays, man derived as mel minus p. Callers that run kepler
///         subtract o from p first, exactly as the original plele1 does
[[nodiscard]] Orbit mean_elements(int body, const TimeArguments& t);

/// Transpluto from the original's fixed table.
///
/// @param t time arguments, these polynomials run on centuries from 1900
/// @return elements ready for kepler, p is already an argument of
///         perihelion
[[nodiscard]] Orbit transpluto_elements(const TimeArguments& t);

/// A Hamburg school factor on its circular orbit.
///
/// @param k the original nk slot family index, 9 Cupido, 10 Hades,
///          11 Zeus, 12 Kronos, 13 Apollon, 14 Admetos, 15 Vulkanus,
///          16 Poseidon
/// @param t time arguments, the mean anomaly runs on centuries from 1900
/// @return elements with only a and man populated, everything else zero
[[nodiscard]] Orbit uranian_elements(int k, const TimeArguments& t);

}  // namespace horcom
