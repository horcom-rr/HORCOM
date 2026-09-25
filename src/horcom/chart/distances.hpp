// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <cmath>
#include <cstddef>

#include "horcom/chart/bodies.hpp"

// The distance tables of his coordinate screen, ported from the original
// HORCOM procedures ko_ta and entf_min_max. The Entf. column shows a
// distance in percent of the mean, the geocentric extremes of Mercury to
// Saturn mark a planet near its closest or farthest point.
namespace horcom {

/// The geocentric mean distances in AU his ko_ta keeps for SO to PL,
/// indexed by body slot, the Moon slot stays empty.
//RR Mittelwerte nach MEYERS Lexikon Weltall
inline constexpr std::array<double, body::kPluto + 1> kMeanGeoAu = {0.0,   1.0,   0.0,    1.0,    1.0,    1.52,
                                                                     5.195, 9.525, 19.215, 30.055, 39.44};

/// The heliocentric mean distances in AU, the Moon's slot holds the Earth.
inline constexpr std::array<double, body::kPluto + 1> kMeanHelioAu = {0.0,   1.0,   1.0,    0.387,  0.723,  1.542,
                                                                       5.205, 9.567, 19.281, 30.142, 39.880};

/// The smallest geocentric distances in AU.
//RR Minimalwerte
inline constexpr std::array<double, body::kPluto + 1> kMinGeoAu = {0.0,  0.98, 0.0,   0.53,  0.26, 0.37,
                                                                    3.93, 7.97, 17.31, 28.77, 28.58};

/// The largest geocentric distances in AU.
//RR Maximalwerte
inline constexpr std::array<double, body::kPluto + 1> kMaxGeoAu = {0.0,  1.02,  0.0,   1.47,  1.74, 2.67,
                                                                    6.46, 11.08, 21.12, 31.34, 50.3};

/// The share of an extreme within which a distance counts as near it.
inline constexpr double kNearExtremeShare = 0.05;

/// His entf_min_max, whether a planet stands near its geocentric
/// perigee or apogee.
///
/// @param slot the body slot, only Mercury to Saturn are judged
/// @param dr   the geocentric distance in AU
/// @return true within five percent of either extreme
[[nodiscard]] inline bool near_distance_extreme(int slot, double dr) {
  if (slot < body::kMercury || slot > body::kSaturn) {
    return false;
  }
  const auto i = static_cast<std::size_t>(slot);
  return std::abs(dr - kMinGeoAu[i]) / kMinGeoAu[i] < kNearExtremeShare ||
         std::abs(dr - kMaxGeoAu[i]) / kMaxGeoAu[i] < kNearExtremeShare;
}

}  // namespace horcom
