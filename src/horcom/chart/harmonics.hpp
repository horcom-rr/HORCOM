// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
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
/// Planets and extras multiply onto the order, the south node follows
/// the transformed north node by half a circle, Transpluto drops out
/// like the original's CASE list leaves it, and the Part of Fortune
/// recomputes from the harmonic angles with the day night formula of
/// a901_m. Nodes stay, the original forced the mean ones there.
///
/// @param base    the radix
/// @param n       the ORDNUNGS-ZAHL, an integer order
/// @param mode    house handling, his haus_ber question
/// @param system  house system for the kFromNewMc recomputation
/// @param lat_deg latitude for the recomputation
/// @return the harmonic chart
[[nodiscard]] Chart harmonic_chart(const Chart& base, double n, HarmonicHouses mode, HouseSystem system, double lat_deg);

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
