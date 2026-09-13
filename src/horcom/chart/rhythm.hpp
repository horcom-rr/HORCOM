// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

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

}  // namespace horcom
