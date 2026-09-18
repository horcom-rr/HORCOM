// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <cmath>

#include "horcom/core/constants.hpp"

// The angle kernel. Ported from the original HORCOM functions ng, nb, nh
// and wz (defined in funkt), the quadrant function atn and the comparison
// helpers vergl1, vergl1r, vergl2 and vergl2r. The aspect logic depends on
// the exact branch behaviour of these functions, so they are transcribed,
// not redesigned.
namespace horcom {

// GFA INT rounds down (floor) while FIX truncates toward zero. The listing
// uses both deliberately, so ng/nb/nh land in [0, period) and wz keeps the
// sign of its argument.

/// DEFFN ng(x) = x - 360 * INT(x / 360)
[[nodiscard]] inline double norm_deg(double x) noexcept {
  return x - 360.0 * std::floor(x / 360.0);
}

/// DEFFN nb(x) = x - pv2 * INT(x / pv2)
[[nodiscard]] inline double norm_rad(double x) noexcept {
  return x - kTwoPi * std::floor(x / kTwoPi);
}

/// DEFFN nh(x) = x - 24 * INT(x / 24)
[[nodiscard]] inline double norm_hours(double x) noexcept {
  return x - 24.0 * std::floor(x / 24.0);
}

/// DEFFN wz(x) = x - 30 * FIX(x / 30), degree within the zodiac sign
[[nodiscard]] inline double deg_in_sign(double x) noexcept {
  return x - kDegPerSign * std::trunc(x / kDegPerSign);
}

/// The original FUNCTION atn, the program's quadrant correct arctangent.
///
/// Replaces the ubiquitous pattern fii = ATN(z / n) followed by @atn(z, fii).
///
/// @param z numerator, its sign decides the second quadrant correction
/// @param n denominator, callers add kEps exactly where the listing does,
///          this function must not guard
/// @return the full angle, in [0, 2 pi) for nonzero z. For z exactly 0
///         with negative n the original returns 0 instead of pi, that
///         behaviour is kept
/// @note the vergl family below reconciles branch cuts before comparisons
[[nodiscard]] inline double atn(double z, double n) noexcept {
  double fii = std::atan(z / n);
  if (fii < 0.0) {
    fii += kPi;
  }
  if (z < 0.0) {
    fii += kPi;
  }
  return fii;
}

// Branch reconcilers used before angular comparisons. Arguments mirror the
// original VAR parameters, values that the routine never writes are taken
// by value.

/// The original vergl1
inline void vergl1(double w1, double& w2) noexcept {
  if (w1 > w2 + kPi && w2 > 0.0 && w1 < kTwoPi) {
    w2 += kTwoPi;
  }
}

/// The original vergl1r
inline void vergl1r(double& w1, double w2) noexcept {
  if (w2 > w1 + kPi && w1 > 0.0 && w2 < kTwoPi) {
    w1 += kTwoPi;
  }
}

/// The original vergl2
inline void vergl2(double w1, double& w2, double& w3) noexcept {
  if (w1 > w3 + kPi && w3 < w2) {
    w3 += kTwoPi;
  }
  if (w1 > w2 + kPi && w2 > 0.0 && w1 < kTwoPi) {
    w2 += kTwoPi;
  }
}

/// The original vergl2r
inline void vergl2r(double& w1, double w2, double& w3) noexcept {
  if (w3 + kPi < w2 && w3 < w1) {
    w3 += kTwoPi;
  }
  if (w1 + kPi < w2 && w1 > 0.0 && w2 < kTwoPi) {
    w1 += kTwoPi;
  }
}

/// The original verv, the symmetric variant used when differencing nearby
/// angles for velocities
inline void verv(double& w1, double& w2) noexcept {
  if (w1 - w2 > 4.0) {
    w2 += kTwoPi;
  }
  if (w2 - w1 > 4.0) {
    w1 += kTwoPi;
  }
}

}  // namespace horcom
