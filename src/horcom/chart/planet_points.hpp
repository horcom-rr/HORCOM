// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/chart/chart.hpp"

// The mean planetary nodes and apsides of the coordinate table,
// introduced after Th. Landscheidt's geocentric planetary nodes.
// Geocentrically these are real points in space, the crossings of the
// orbit with the ecliptic plane and the ends of the orbit's long axis
// seen from the Earth, heliocentrically they are directions. Ported
// from HORCOM plko10, plko100, plko1001 and plko12 with the Pluto
// mean elements of plelempl.
namespace horcom {

/// The four columns of one body's row.
struct PlanetPoints {
  bool ok = false;
  /// the ascending node, geocentric ecliptic longitude in radians
  double node = 0.0;
  double node_south = 0.0;
  double perihelion = 0.0;
  double aphelion = 0.0;
};

/// Computes the mean points for one body of a chart.
///
/// The Sun's row carries the solar equator node after Landscheidt and
/// the perigee of the apparent orbit, the Moon's row its mean node and
/// mean apogee, the planets Mercury through Pluto project their mean
/// orbit points. Every value is a mean, like the Mittel label of his
/// screen.
///
/// @param chart the computed chart, the Earth rides on the sun slot
/// @param slot  body slot 1 through 10
/// @param s     the settings, heliocentric switches to directions
/// @return the four longitudes, ok false for other slots, a negative
///         node where none exists, the Earth of the hrg mode
[[nodiscard]] PlanetPoints planet_points(const Chart& chart, int slot, const ChartSettings& s);

}  // namespace horcom
