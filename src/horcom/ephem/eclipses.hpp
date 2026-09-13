// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string>
#include <vector>

// New and full moons with the eclipse rules, ported from the original
// finst screen. The lunation series after Meeus hands out the exact
// syzygy moments, and the gamma and u quantities decide whether the
// shadow reaches the Earth or the moon, central or excentric, total,
// annular, umbral or penumbral, north or south.
namespace horcom {

/// One new or full moon.
struct Lunation {
  /// the exact moment in Universal Time
  double jd_ut = 0.0;
  /// false a new moon, true a full moon
  bool full = false;
  /// true when the syzygy carries an eclipse
  bool eclipse = false;
  /// his screen letters, ZT or EX with TOT or RF and N or S for the
  /// sun, KERNSCH or HALBSCH for the moon, empty without an eclipse
  std::string kind;
};

/// Lists lunations around a start date, the columns of his screen.
///
/// @param jd_start_ut the first row lands near this moment
/// @param count       how many lunations forward
/// @param full_moons  true walks the full moons, false the new moons
/// @return the moments with their eclipse classification
[[nodiscard]] std::vector<Lunation> lunations(double jd_start_ut, int count, bool full_moons);

}  // namespace horcom
