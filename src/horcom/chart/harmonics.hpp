// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

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
/// south node follows the transformed north node by half a circle and
/// Transpluto drops out like the original's CASE list leaves it.
/// Nodes stay, the original forced the mean ones there.
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
  enum class Kind { kBody, kCusp, kRuler, kSignStart };
  Kind kind = Kind::kBody;
  /// body slot for kBody, 13 the ascendant, 14 the midheaven
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
/// Bodies and extras follow the mode formula, Transpluto stays dark
/// like everywhere in the MULTI world, the south node follows the
/// directed north node. Under kLikeBodies the axes run the same
/// formula and the intermediate cusps clear, the zero point modes keep
/// only AC and MC, under kFromNewMc the directed MC hands a fresh ARMC
/// to the full house computation and a901_m rebuilds the Part of
/// Fortune from the directed lights.
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
