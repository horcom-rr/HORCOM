// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <vector>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"

// The trigger walk of the Münchner Rhythmenlehre after W. Döbereiner,
// ported from the original a170 world. Life walks the twelve houses as
// phases of seven years each, leftward or rightward from a chosen
// house. Every phase triggers the bodies standing in its house, the
// ruler of its sign with the rulers of intercepted signs, and each
// trigger pulls in its aspect partners and its mirror point partners,
// every age read from the body's position within its own house.
namespace horcom {

/// How a body entered the trigger list, his al$ letters.
enum class RhythmKind {
  kDirect,   // D, the body stands in the phase's house
  kRuler,    // P, the ruler of the phase's sign
  kRuler2,   // P2, the ruler of an intercepted sign
  kRuler3,   // P3, the second intercepted sign
  kAspect,   // A, an aspect partner of a trigger
  kMirror,   // S, a mirror point partner of a trigger
};

/// One row of the trigger table.
struct RhythmTrigger {
  /// the walk step, one to twelve
  int phase = 0;
  /// the house this phase walks
  int house = 0;
  /// the triggered body
  int slot = 0;
  /// the body that pulled it in, aspect and mirror rows only
  int source = 0;
  RhythmKind kind = RhythmKind::kDirect;
  /// the folded aspect angle of an aspect row, degrees
  double angle_deg = 0.0;
  /// the age, in years, or in months when the unit says so
  double value = 0.0;
};

/// The knobs of the rhythm walk.
struct RhythmOptions {
  /// the phase length in years, his Döbereiner seven
  double phase_years = 7.0;
  /// true counts the values in the month unit
  bool months = false;
  /// the house the walk begins with
  int begin_house = 1;
  /// true walks leftward through the houses, false rightward
  bool leftward = true;
  /// true lets the sextile into the aspect chains, his sext switch
  bool sextile = false;
  /// true also triggers the Black Moon's opposite point
  bool apogee_opposite = false;
  //RR SONDERPUNKT ( FIXPUNKT ) der Rhythmenlehre, sein rotes F, a
  //RR degree in radians, negative when off, independent of the
  //RR general fixed point
  double special = -1.0;
};

/// Runs the trigger walk over a chart.
///
/// @param chart   the radix with its houses
/// @param aspects a scan whose matrix feeds the aspect chains, run
///                with divisors six when the sextile is open, four
///                otherwise, like the original forced nasp
/// @param a       orb configuration, the mirror matrix reads it
/// @param opt     phase length, unit, start, direction and switches
/// @return the triggers in walk order
[[nodiscard]] std::vector<RhythmTrigger> rhythm_triggers(const Chart& chart, const AspectResult& aspects, const AspectSettings& a, const RhythmOptions& opt);

/// A self defined degree for the GRAD-DATUM-LISTE, his GRADE.INT rows.
struct CustomDegree {
  /// the ecliptic degree, half degree steps
  double degree = 0.0;
  /// the two planet slots of the characteristic
  int p = 0;
  int q = 0;
};

/// One row of the GRAD-DATUM-LISTE.
struct DegreeDate {
  double degree = 0.0;
  /// life years at the crossing, months in the month unit
  double value = 0.0;
  int house = 0;
  /// the planet pair of a Gruppenschicksals-Grad, zero without one
  int p = 0;
  int q = 0;
  /// a self defined degree, its 0 Aries-Libra mirror rides along
  bool custom = false;
  bool mirror = false;
};

/// The degree the walk stands on at a given age, the inverse of the
/// degree date list. The date defined Sonderpunkt of a17sonderpkt
/// rides on it.
///
/// @param chart the radix with its houses
/// @param opt   phase length and direction
/// @param years the age, in years even when the options count months
/// @return the ecliptic degree in radians, negative without houses
[[nodiscard]] double degree_at_age(const Chart& chart, const RhythmOptions& opt, double years);

/// Reads self defined degrees, his GRADE.INT lines of degree and two
/// planet slots separated by commas.
///
/// @param file the file beside the data
/// @return the rows, empty when the file is absent
[[nodiscard]] std::vector<CustomDegree> read_degrees(const std::filesystem::path& file);

/// Writes the self defined degrees back in the GRADE.INT shape.
bool write_degrees(const std::filesystem::path& file, const std::vector<CustomDegree>& rows);

/// The GRAD-DATUM-LISTE, every half ecliptic degree with the age the
/// rhythm walk crosses it, the published Gruppenschicksals-Grade of
/// W. Döbereiner marked with their planet pairs. Ported from HORCOM
/// a17_3 and a171.
///
/// @param chart   the radix with its houses
/// @param opt     phase length, unit and direction
/// @param own     self defined degrees, mirrored across 0 Aries-Libra
/// @param mundane project each degree along its semi arc first, the
///                dates then follow the equatorial geometry
/// @param lat_deg the observer's latitude, the projection needs it
/// @return 720 rows in degree order
[[nodiscard]] std::vector<DegreeDate> degree_dates(const Chart& chart, const RhythmOptions& opt, const std::vector<CustomDegree>& own, bool mundane, double lat_deg);

}  // namespace horcom
