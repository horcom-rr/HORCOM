// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

// Ecliptic and equatorial coordinates. Ported from the original HORCOM
// procedures ko_tr1 and ko_tr2. All angles radians, ekls is the obliquity
// of the ecliptic, in the original usually the TRUE obliquity after somo.
namespace horcom {

struct Equatorial {
  double ra = 0.0;
  double dec = 0.0;
};

struct Ecliptic {
  double lon = 0.0;
  double lat = 0.0;
};

/// The original ko_tr1, ecliptic to equatorial.
///
/// @param la   ecliptic longitude, radians
/// @param br   ecliptic latitude
/// @param ekls obliquity of the ecliptic
/// @return right ascension and declination, the kk guard on the
/// denominator is the original's own
[[nodiscard]] inline Equatorial ecliptic_to_equatorial(double la, double br, double ekls) noexcept {
  Equatorial out;
  out.dec = std::asin(std::sin(br) * std::cos(ekls) + std::cos(br) * std::sin(ekls) * std::sin(la));
  const double z = std::sin(la) * std::cos(ekls) * std::cos(br) - std::sin(br) * std::sin(ekls);
  const double n = std::cos(la) * std::cos(br) + kEps;
  out.ra = atn(z, n);
  return out;
}

/// The original ko_tr2, equatorial to ecliptic.
///
/// @param ar   right ascension, radians
/// @param de   declination
/// @param ekls obliquity of the ecliptic
/// @return ecliptic longitude and latitude
[[nodiscard]] inline Ecliptic equatorial_to_ecliptic(double ar, double de, double ekls) noexcept {
  Ecliptic out;
  out.lat = std::asin(std::sin(de) * std::cos(ekls) - std::cos(de) * std::sin(ekls) * std::sin(ar));
  const double z = std::sin(ar) * std::cos(ekls) * std::cos(de) + std::sin(de) * std::sin(ekls);
  const double n = std::cos(ar) * std::cos(de) + kEps;
  out.lon = atn(z, n);
  return out;
}

}  // namespace horcom
