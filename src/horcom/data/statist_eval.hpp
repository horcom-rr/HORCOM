// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <string>
#include <vector>

#include "horcom/chart/aspects.hpp"
#include "horcom/data/statist.hpp"

// The statistics evaluation of the original stat_ausw world. One query
// picks an object, where it must stand and how sharply, and runs over a
// loaded dataset. Conditions chain through a survivor mask exactly like
// his mz per record markers, UND keeps only records that carried a
// previous match, everything else unions. Windows that wrap the zero
// point run as extra passes like his two follow up loops.
namespace horcom {

/// One operand of a condition, the m choices of the original menus, a
/// body slot, a house cusp or the ruler of a house cusp's sign.
struct StatOperand {
  enum class Kind { kBody, kCusp, kRuler };
  Kind kind = Kind::kBody;
  /// body slot for kBody, 1 to 14 plus the extras of the dataset
  int body = 1;
  /// house number 1 to 12 for kCusp and kRuler
  int house = 1;
};

/// What stands under test, the obj menu of stat2.
enum class StatObject {
  kBody,           // one body, cusp or ruler through operand a
  kLights,         // sun, moon and AC, the cnm walk
  kAllBodies,      // every body of the set, the dnm walk
  kHouseRuler,     // the ruler of house a.house, obj 2
  kMidpoint,       // the midpoint of a and b, obj 3
  kAspect,         // an aspect between a and b, obj 4
  kMidpointAspect, // a aspecting the midpoint of b and c
  kMirror,         // the mirror point of a, obj 5
  kName,           // a name fragment, obj 6
  kArabicPart,     // a + c - b with the day night swap, obj 7
};

/// Where the object must stand, the suc menu.
enum class StatWindow { kAtDegree, kInSign, kInHouse, kNearBody, kAnywhere };

/// The mirror axis choice spg of the original.
enum class MirrorAxis { kAriesLibra = 1, kCancerCapricorn = 2, kBoth = 3 };

/// One evaluation condition.
struct StatQuery {
  StatObject object = StatObject::kBody;
  StatOperand a;
  StatOperand b;
  StatOperand c;
  StatWindow window = StatWindow::kAnywhere;
  /// kAtDegree centre in radians
  double degree = 0.0;
  /// orb in radians for kAtDegree and kNearBody, in degrees for kInSign
  double orb = 0.0;
  /// kInSign sign 1 to 12
  int sign = 1;
  /// kInHouse house number and orb in percent of the house length
  int house = 1;
  double house_orb_pct = 0.0;
  /// the kNearBody target, his m(4)
  StatOperand near_body;
  /// aspect divisor range naspe to nas, and the fixed single orb obas
  /// in radians, zero takes the orb table
  int asp_low = 1;
  int asp_high = 12;
  double asp_orb = 0.0;
  MirrorAxis mirror = MirrorAxis::kBoth;
  /// kName fragment, matched against the uppercased names
  std::string name;
  /// the ara choice, true swaps the arabic formula on day charts
  bool arabic_day_night = true;
  /// the original alt, classic sign rulers before the outer planets
  bool classic_rulers = false;
  /// his UND chaining over the survivors of the previous condition
  bool combine_and = false;
};

/// One satisfied test.
struct StatMatch {
  int record = 0;
  /// the body slot that satisfied, 0 for cusp and derived values
  int slot = 0;
  /// the tested longitude in radians
  double value = 0.0;
};

/// Result of one condition run.
struct StatEvalResult {
  std::vector<StatMatch> matches;
  /// sum(1..12) of the original sum_z_h, the sign distribution of every
  /// tested value, houses under the kInHouse window, index 0 the total
  std::array<int, 13> distribution{};
};

/// The twelve reconstructed cusps of haus_def, f(1..12) at indices 1 to
/// 12 with the AC copy at 13, index 0 unused.
///
/// @param r a dataset record with its six stored cusps
/// @return the cusp array, the missing six mirrored through the centre
[[nodiscard]] std::array<double, 14> stat_houses(const StatRecord& r);

/// The ruler of the sign standing at w, the original ze_pl.
///
/// @param w       a longitude in radians
/// @param classic true takes the old rulers, Mars for Scorpio, Saturn
///                for Aquarius, Jupiter for Pisces
/// @return the ruling body slot, 0 when w is not inside the circle
[[nodiscard]] int sign_ruler(double w, bool classic);

/// Runs one condition over a dataset like stat_ausw.
///
/// The mask carries the survivors between chained conditions, one value
/// per record, zero means out. UND requires a previous value and clears
/// failures, otherwise matches union in. The lights and all bodies
/// walks never write the mask on a match, only their failures clear it
/// under UND, exactly the original's split between mz per record and
/// per match.
///
/// @param set  the loaded dataset
/// @param q    the condition
/// @param a    orb configuration for the aspect windows
/// @param mask the survivor values, resized to the record count
/// @return matches of this run and the distribution of tested values
[[nodiscard]] StatEvalResult evaluate_statistics(const StatSet& set, const StatQuery& q, const AspectSettings& a, std::vector<double>& mask);

}  // namespace horcom
