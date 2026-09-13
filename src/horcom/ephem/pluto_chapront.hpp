// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/ephem/kepler.hpp"
#include "horcom/time/calendar.hpp"

// Pluto from the Chapront perturbation series. The original HORCOM uses
// this analytic theory whenever the requested epoch falls outside the
// numerically integrated pluto.eph, his comment reads
//RR NACH CHAPRONT außerhalb des num. berechneten Bereichs
// Ported from the original procedures pl_ko, plelempl and pl_praez.
namespace horcom {

/// Result of the Chapront evaluation.
struct ChaprontPluto {
  Orbit elements;      // perturbed elements, p as argument of perihelion
  OrbitPosition pos;   // Kepler solution at the epoch, equinox J2000
  double hel = 0.0;    // heliocentric longitude of DATE after precession
  double heb = 0.0;    // heliocentric latitude of date
  double r = 0.0;      // radius, AU
};

/// Evaluates Pluto with the Chapront series.
///
/// The corrections run over the mean longitudes of Jupiter through
/// Neptune and Pluto's own mean longitude. One polynomial inside uses the
/// cube of centuries from 1900 where its siblings use centuries from
/// J2000, the original does exactly that and it is preserved.
///
/// @param t    time arguments of the epoch, Ephemeris Time
/// @param ekls true obliquity of date in radians, from somo
/// @return elements and the position of date
[[nodiscard]] ChaprontPluto pluto_chapront(const TimeArguments& t, double ekls);

}  // namespace horcom
