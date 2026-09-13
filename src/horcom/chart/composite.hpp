// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <vector>

#include "horcom/chart/chart.hpp"

// The composite and combin charts of the original, a13 and a14. The
// composite midpoints two charts body by body with his near side rule,
// the combin averages moment and place and runs the normal pipeline.
namespace horcom {

/// The three house modes of the a13 dialog.
enum class CompositeHouses {
  kMeanSidereal = 1,  // comp_mstz, mean sidereal time, longitude, latitude
  kRobertHand = 2,    // comp_hand, the armc from the MC midpoint at the residence
  kSchematic = 3,     // schematic halbsummen from the MC midpoint
};

/// The near side midpoint of two longitudes, ported from halbsmin.
///
/// @param p1 first longitude, radians
/// @param p2 second longitude
/// @return the midpoint on the shorter arc with his far side tie rule
[[nodiscard]] double midpoint_near(double p1, double p2);

/// Builds the composite of two charts, ported from a13.
///
/// Body positions are near side midpoints, the south node follows the
/// north by half a circle, the AC and MC midpoints flip to the side of
/// their cusps, and the houses follow the chosen mode. The result
/// carries longitudes only, latitudes and speeds stay zero because a
/// composite has none.
///
/// @param a    the first chart
/// @param ia   its input, place and moment
/// @param b    the second chart
/// @param ib   its input
/// @param mode house mode
/// @param residence_lat_deg the Robert Hand residence latitude
/// @param s    chart settings deciding the active slots
/// @return the composite as a chart
[[nodiscard]] Chart composite_chart(const Chart& a, const ChartInput& ia, const Chart& b, const ChartInput& ib,
                                    CompositeHouses mode, double residence_lat_deg, const ChartSettings& s);

/// The combin mean of a14, plain averages of moment and place for two
/// to five records, the chart then runs the normal pipeline.
///
/// @param parts the inputs to average
/// @param cal   calendar rule for the averaged moment
/// @return the averaged input
[[nodiscard]] ChartInput combin_input(const std::vector<ChartInput>& parts, Calendar cal);

}  // namespace horcom
