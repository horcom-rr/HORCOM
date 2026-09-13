// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/chart/chart.hpp"

// The mundane mode of the original, horm 2. Positions leave the
// ecliptic and become semi arc proportional house space longitudes
// through mundan with mundh1, md11 and md, the houses become the equal
// grid of mundhorh, and every scanner then runs unchanged on the
// mundane values.
namespace horcom {

/// The mundane longitude of one ecliptic point.
///
/// @param la_rad  ecliptic longitude, radians
/// @param br_rad  ecliptic latitude, callers add the epsilon like the
///                original
/// @param ekls    true obliquity
/// @param armcb   the ARMC in radians
/// @param lat_deg geographic latitude
/// @return the house space longitude, the ascendant at zero
[[nodiscard]] double mundane_longitude(double la_rad, double br_rad, double ekls, double armcb, double lat_deg);

/// Turns a chart into its mundane form like mundhorp and mundhorh, the
/// bodies to house space and the cusps to the equal grid.
///
/// @param c       the chart, changed in place
/// @param lat_deg geographic latitude of the chart's place
void to_mundane(Chart& c, double lat_deg);

}  // namespace horcom
