#pragma once

#include <numbers>

// Constants ported from the original HORCOM procedure funkt.
// Original variable names are noted so formulas stay literally comparable.
namespace horcom {

inline constexpr double kPi = std::numbers::pi;
inline constexpr double kHalfPi = kPi / 2.0;            // original po
inline constexpr double kTwoPi = 2.0 * kPi;             // original pv2
inline constexpr double kDegToRad = kPi / 180.0;        // original pu
inline constexpr double kRadToDeg = 180.0 / kPi;        // original up
inline constexpr double kArcsecToRad = kDegToRad / 3600.0;   // original puu
inline constexpr double kDegPerCenturyToRad = kDegToRad / 36525.0;  // original pup

// original kk, the epsilon Robert Rettig adds exactly where a division or
// an ATN argument could hit zero. Guards are placed only where the listing
// places them, never generally.
inline constexpr double kEps = 1.0e-10;

}  // namespace horcom
