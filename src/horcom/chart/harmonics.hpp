// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <vector>

#include "horcom/chart/chart.hpp"
#include "horcom/chart/settings.hpp"

// The harmonic charts of the original MULTI world and the 90 degree
// circle of the a12 double wheel. A harmonic multiplies every longitude
// by an integer order, the dial multiplies by four so a quarter of the
// circle fills the wheel and the three qualities stack.
namespace horcom {

/// The two answers of his haus_ber question.
enum class HarmonicHouses {
  /// HÄUSER wie PLANETEN BEHANDELN, every cusp times the order
  kLikeBodies,
  /// AUFGRUND des NEUEN MC NEU BERECHNEN, the directed ARMC path
  kFromNewMc,
};

/// Builds the harmonic of a chart, the original harm21 with the houses
/// of mc_armcb CASE 7.
///
/// Planets and extras multiply onto the order, the Part of Fortune
/// among them since harm21 keeps its a901_m rebuild commented out, the
/// south node follows the transformed north node by half a circle.
/// Transpluto, the Hamburg factors and the fixed point drop out like the
/// original's CASE 1 TO 11 list leaves them. The caller hands in the
/// radix with the mean nodes his harm forced.
///
/// @param base    the radix
/// @param n       the ORDNUNGS-ZAHL, an integer order
/// @param mode    house handling, his haus_ber question
/// @param system  house system for the kFromNewMc recomputation
/// @param lat_deg latitude for the recomputation
/// @return the harmonic chart
[[nodiscard]] Chart harmonic_chart(const Chart& base, double n, HarmonicHouses mode, HouseSystem system, double lat_deg);

/// The six multiple direction modes of the MULTI menu, his mul&.
enum class MultiMode {
  /// own position plus age times the degree within its sign
  kMulti1 = 1,
  /// own position plus age times the whole longitude
  kMulti2 = 2,
  /// a reference point plus age times the degree within sign
  kMulti3 = 3,
  /// MULTI-0-OST, every body runs from its day rulership sign start
  kZeroEast = 4,
  /// MULTI-0-WEST, the night rulership sign starts
  kZeroWest = 5,
  /// MULTI-ARC, reference plus age times the arc to the body
  kArc = 6,
};

/// The Bezugspunkt of MULTI 3 and MULTI-ARC, his mc_armcb1 choices.
struct MultiReference {
  /// The rows of his BEZUGS-FAKTOR box.
  enum class Kind {
    kBody,      ///< a body, the fixed point or an angle
    kCusp,      ///< HAUS NR., a radix cusp
    kRuler,     ///< HERR v. HAUS NR., the classic ruler of a cusp
    kSignStart  ///< 0 GRAD eines ZEICHENS
  };
  /// which row was chosen
  Kind kind = Kind::kBody;
  /// body slot for kBody, 0 the fixed point, 13 the ascendant, 14 the
  /// midheaven, the tenth cusp like his fz(1,ze,10)
  int body = 1;
  /// house number for kCusp and kRuler, the ruler reads the classic
  /// table like his forced alt switch
  int house = 1;
  /// sign 1 to 12 for kSignStart, his hz
  int sign = 1;
};

/// Builds one multiple direction over the radix, the multiN1 transforms
/// with the houses of mc_armcb.
///
/// Bodies, extras and the fixed point follow the mode formula,
/// Transpluto and the Hamburg factors stay dark like everywhere in the
/// MULTI world, the zero point modes leave the fixed point dark as well,
/// the south node follows the directed north node. Under kLikeBodies the
/// axes run the same formula and the intermediate cusps clear, the zero
/// point modes keep only AC and MC, under kFromNewMc the directed tenth
/// cusp hands a fresh ARMC to the full house computation and, outside
/// the zero point modes, a901_m rebuilds the Part of Fortune from the
/// directed lights with the day or night rule of the radix.
///
/// @param base    the radix
/// @param mode    which MULTI
/// @param lja     the age in tropical years at the event, his lja
/// @param ref     the reference point, read for kMulti3 and kArc only
/// @param houses  his haus_ber question
/// @param system  house system for the kFromNewMc recomputation
/// @param lat_deg latitude for the recomputation
/// @return the directed chart
[[nodiscard]] Chart multi_chart(const Chart& base, MultiMode mode, double lja, const MultiReference& ref, HarmonicHouses houses, HouseSystem system, double lat_deg);

/// One line of the halbsm list of the MULTI sheet, a directed pair whose
/// near midpoint meets a sign axis, a radix cusp or a directed angle.
struct MultiMidpoint {
  enum class Target {
    kSignAxis,    ///< the boundary of two opposite signs, AR/LI and so on
    kRadixCusp,   ///< a cusp of the radix, HS 1/7 R
    kMultiAngle,  ///< AC or MC of the directed chart, HS 1/7 M
  };
  int u = 0;  ///< first body of the pair
  int w = 0;  ///< second body
  Target target = Target::kSignAxis;
  /// the lower end of the axis, sign 1 to 6 or house 1 to 6
  int first = 0;
  /// the upper end, sign 7 to 12 or house 7 to 12
  int second = 0;
};

/// Scans the directed pairs of a MULTI or HARMONIC chart like halbsm,
/// a fifth of a degree times the orb factor around the twelve sign
/// boundaries, the radix cusps and the directed AC and MC.
///
/// @param radix the radix, its cusps are the targets
/// @param multi the directed chart, its bodies form the pairs
/// @param orb   the orb factor
/// @return the lines in his order, pair by pair
[[nodiscard]] std::vector<MultiMidpoint> multi_midpoints(const Chart& radix, const Chart& multi, double orb);

/// The a12f transform of the 90 degree circle. Every present slot
/// multiplies by the factor, the axes of the house array ride along and
/// the intermediate cusps clear, the aspect scan runs on this state
/// like the original a92.
///
/// @param base the chart to transform
/// @param dop  the factor, four for the 90 degree circle
/// @return the transformed chart
[[nodiscard]] Chart dial_chart(const Chart& base, double dop);

/// The divide back of a12f, run after the aspect scan. The AC and MC
/// glyphs move to their transformed position over the factor, which
/// folds them into the first quarter like the original pl(13) line.
///
/// @param c   a chart from dial_chart
/// @param dop the same factor
void dial_display(Chart& c, double dop);

}  // namespace horcom
