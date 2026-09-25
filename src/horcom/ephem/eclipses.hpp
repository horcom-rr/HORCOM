// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string>
#include <vector>

#include "horcom/time/calendar.hpp"

// New and full moons with the eclipse rules, ported from the original
// finst screen. The lunation series after Meeus hands out the exact
// syzygy moments, and the gamma and u quantities decide whether the
// shadow reaches the Earth or the moon, central or excentric, total,
// annular, umbral or penumbral, north or south.
namespace horcom {

/// One new or full moon.
struct Lunation {
  /// his lunation number k, whole for new moons, half for full moons
  double k = 0.0;
  /// the exact moment in Universal Time
  double jd_ut = 0.0;
  /// the moment of greatest eclipse in Universal Time, his finst_1,
  /// zero without an eclipse
  double max_ut = 0.0;
  /// false a new moon, true a full moon
  bool full = false;
  /// true when the syzygy carries an eclipse
  bool eclipse = false;
  /// his screen letters, ZT or EX with TOT or RF and N or S for the
  /// sun, KERNSCH or HALBSCH for the moon, empty without an eclipse
  std::string kind;
};

/// His k1 of a search date, the lunation number the screen counts from.
///
/// @param d the calendar date
/// @return INT((J + (M - 1) / 12 + D / 365.25 - 2000) * 12.3685)
[[nodiscard]] double lunation_number(const CalendarDate& d);

/// One syzygy, the original neu_voll with finst_1.
///
/// @param k    the lunation number, whole for a new moon, half for a full
///             moon
/// @return the moment, the eclipse classification and for an eclipse the
///         moment of greatest eclipse
[[nodiscard]] Lunation lunation_at(double k);

/// Lists lunations around a start date, the columns of his screen. The
/// first new moon is k1 - 1 and the first full moon k1 - 1.5 like finst.
///
/// @param jd_start_ut the search date
/// @param count       how many lunations forward
/// @param full_moons  true walks the full moons, false the new moons
/// @return the moments with their eclipse classification
[[nodiscard]] std::vector<Lunation> lunations(double jd_start_ut, int count, bool full_moons);

}  // namespace horcom
