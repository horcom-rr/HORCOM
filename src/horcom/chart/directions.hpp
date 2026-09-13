// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/chart/chart.hpp"

// The primary directed axes of the original prima. The radix ARMC turns
// by his Naibod style rate, three hundred sixty degrees per tropical
// year of elapsed days over the year length, direct or converse, and
// the axes and cusps recompute from the directed ARMC at the place. His
// sidereal time variation for rectification rides on the same turn.
namespace horcom {

/// One directed axes result.
struct DirectedAxes {
  double arm_deg = 0.0;   // the radix ARMC at the place
  double armc_deg = 0.0;  // the directed ARMC
  double arc_deg = 0.0;   // the turn, his STZ-DIFF display
  Houses houses;          // directed axes and cusps
};

/// Directs the axes to an event moment, ported from prima and primhorg.
///
/// @param radix        the birth chart, its sidereal time drives the arc
/// @param lon_deg_east place longitude, the event place may differ
/// @param lat_deg      place latitude for the house computation
/// @param jd_event_ut  the event moment
/// @param converse     true turns backward, his KONVERS
/// @param vary_deg     his sidereal time variation in degrees, added at
///                     the rate of the arc, one degree is four clock
///                     minutes of birth time
/// @param sys          the house system for the directed cusps
/// @return the directed axes
[[nodiscard]] DirectedAxes direct_axes(const Chart& radix, double lon_deg_east, double lat_deg,
                                       double jd_event_ut, bool converse, double vary_deg, HouseSystem sys);

}  // namespace horcom
