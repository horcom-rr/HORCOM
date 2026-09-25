// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

// The zodiac signs as the original wrote them in its tables.
namespace horcom {

/// The twelve signs of the zodiac, Aries first.
inline constexpr int kSignCount = 12;

/// The two letter sign tags of his zei$ table, Aries first.
inline constexpr const char* kSignTag[kSignCount] = {"AR", "TA", "GM", "CN", "LE", "VI",
                                                     "LI", "SC", "SG", "CP", "AQ", "PS"};

/// A longitude split into sign, degree, minute and second like his grze
/// forms, rounded at the last unit shown.
struct ZodiacSplit {
  /// the sign, 0 Aries to 11 Pisces
  int sign = 0;
  /// whole degrees within the sign, 0 to 29
  int deg = 0;
  /// whole minutes, 0 to 59
  int min = 0;
  /// whole seconds, 0 to 59, zero when only minutes are shown
  int sec = 0;
};

/// Splits a longitude for the zodiac forms of the tables and sheets.
///
/// @param lon_rad the ecliptic longitude in radians
/// @param seconds true rounds to the arc second, false to the arc minute
/// @return the parts, a rounded unit at the end of a sign carries into
///         the next sign like his grze_0, 29 PS 59'59.6" reads 0 AR 00'00"
[[nodiscard]] inline ZodiacSplit split_zodiac(double lon_rad, bool seconds) {
  const double unit = seconds ? kArcsecPerDeg : kArcminPerDeg;
  const auto per_circle = static_cast<long long>(kDegPerCircle * unit);
  long long total = std::llround(norm_deg(lon_rad * kRadToDeg) * unit);
  if (total >= per_circle) {
    total -= per_circle;
  }
  const auto per_sign = static_cast<long long>(kDegPerSign * unit);
  ZodiacSplit out;
  out.sign = static_cast<int>(total / per_sign);
  long long rest = total % per_sign;
  if (seconds) {
    out.sec = static_cast<int>(rest % 60);
    rest /= 60;
  }
  out.min = static_cast<int>(rest % 60);
  out.deg = static_cast<int>(rest / 60);
  return out;
}

}  // namespace horcom
