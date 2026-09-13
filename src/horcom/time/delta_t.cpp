// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/time/delta_t.hpp"

#include <array>

#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// (year, delta T in seconds) from the original initialisation. Index 0 is
// the zero element of the GFA arrays y&() and z(), the interpolation loop
// touches it when the year is exactly 1620 and yields 124 there as well.
constexpr std::array<double, 23> kYear = {0,    1620, 1640, 1660, 1680, 1700, 1720, 1740,
                                          1760, 1780, 1800, 1820, 1840, 1860, 1880, 1900,
                                          1920, 1940, 1960, 1991, 1997, 2000, 2008};
constexpr std::array<double, 23> kSeconds = {0,    124,  62,    37,    16,   9,     11,    12,
                                             15,   17,   13.7,  12,    5.7,  7.88,  -5.4,  -2.72,
                                             21.16, 24.33, 33.15, 57.2, 63,   65,    67};

}  // namespace

double delta_t_minutes(double jd) {
  const double j = 1900.0 + (jd - 2415020.0) / 365.25;
  const double t1 = (jd - 2415020.0) / 36525.0;
  double delt = 0.0;
  if (j >= 1620.0 && j <= 2008.0) {
    int f = 1;
    while (kYear[static_cast<unsigned>(f)] - j < 0.0) {
      ++f;
    }
    const auto fu = static_cast<unsigned>(f);
    delt = kSeconds[fu - 1] + (j - kYear[fu - 1]) * (kSeconds[fu] - kSeconds[fu - 1]) / (kEps + kYear[fu] - kYear[fu - 1]);
    delt = delt / 60.0;
  }
  if (j > 2008.0) {
    delt = 1.2053 * t1 + 0.4992 * t1 * t1 - 0.7506263;  //RR 2008 a.d. ........
  }
  if (j < 1620.0 && j > 948.0) {
    delt = 0.425 + 0.85 * t1 + 0.425 * t1 * t1;  //RR 948 a.d.  ..... 1600 a.d.
  }
  if (j <= 948.0) {
    delt = 28.74 + 6.81 * t1 + 0.7383 * t1 * t1;  //RR 390 v.chr.  .... 948 a.d.
  }
  return delt;
}

//RR UT in ET
double ut_to_et(double jd_ut) {
  return jd_ut + delta_t_minutes(jd_ut) * kDeltaTDaysPerMinute;
}

//RR ET in UT
double et_to_ut(double jd_et, double delt_minutes) {
  return jd_et - delt_minutes * kDeltaTDaysPerMinute;
}

}  // namespace horcom
