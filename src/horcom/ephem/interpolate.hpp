// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/core/constants.hpp"

// Interpolation over equidistant samples. Ported from the original HORCOM
// functions ipol and ipol3. Both carry a branch cut guard that lifts later
// samples by a full turn when a gap larger than 3 appears, because the
// original reuses them for angle series. On the cartesian ephemeris
// coordinates the guard never triggers.
namespace horcom {

/// The original ipol, five point central difference formula.
///
/// @param y1 oldest sample
/// @param y2 second sample
/// @param y3 centre sample
/// @param y4 fourth sample
/// @param y5 newest sample
/// @param n  dimensionless offset from y3 in steps
/// @return the interpolated value
[[nodiscard]] inline double ipol(double y1, double y2, double y3, double y4, double y5, double n) noexcept {
  if (y1 - y2 > 3.0) {
    y2 += kTwoPi;
    y3 += kTwoPi;
    y4 += kTwoPi;
    y5 += kTwoPi;
  }
  if (y2 - y3 > 3.0) {
    y3 += kTwoPi;
    y4 += kTwoPi;
    y5 += kTwoPi;
  }
  if (y3 - y4 > 3.0) {
    y4 += kTwoPi;
    y5 += kTwoPi;
  }
  if (y4 - y5 > 3.0) {
    y5 += kTwoPi;
  }
  const double a = y2 - y1;
  const double b = y3 - y2;
  const double c = y4 - y3;
  const double d = y5 - y4;
  const double e = b - a;
  const double f = c - b;
  const double g = d - c;
  const double h = f - e;
  const double j = g - f;
  const double k = j - h;
  return y3 + (b + c) * n / 2.0 + f * n * n / 2.0 + (h + j) * n * (n * n - 1.0) / 12.0 + k * n * n * (n * n - 1.0) / 24.0;
}

/// The original ipol3, three point quadratic around the centre sample y2.
///
/// @param y1 oldest sample
/// @param y2 centre sample
/// @param y3 newest sample
/// @param n  dimensionless offset from y2 in steps
/// @return the interpolated value
[[nodiscard]] inline double ipol3(double y1, double y2, double y3, double n) noexcept {
  if (y1 - y2 > 3.0) {
    y2 += kTwoPi;
    y3 += kTwoPi;
  }
  if (y2 - y3 > 3.0) {
    y3 += kTwoPi;
  }
  const double a = y2 - y1;
  const double b = y3 - y2;
  const double c = b - a;
  return y2 + (a + b + n * c) * n / 2.0;
}

}  // namespace horcom
