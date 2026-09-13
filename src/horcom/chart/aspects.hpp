// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <vector>

#include "horcom/chart/chart.hpp"

// The aspect engine and the midpoints. Ported from the original HORCOM
// procedures asp0, asp10, asp1 and asp11 with their orb helpers org and
// orbis_discr2, and from the midpoint chain halbs1, halbs11 and halbs111.
// The scan runs over harmonic divisors, a pair that matched under a
// smaller divisor is never tested again, so the classical aspect always
// wins. The Schiemenz counters for Trigas and grand trines are part of
// asp1 and ride along.
namespace horcom {

/// Orb configuration, the original orb, or&() and orbe() world.
struct AspectSettings {
  /// the original orb, the ORBIS-FAKTOR, 1 equals 100 percent
  double orb = 1.0;
  /// the original nasp&, the highest divisor scanned, 16 at most
  int divisors = 12;
  /// the original orbe!, equal probability orbs with the multiple filter
  bool equal_probability = false;
  /// the original orbe(0..14) in radians, index by divisor, 13 mirror
  /// points, 14 midpoints, used when equal_probability is on
  std::array<double, 15> orbe{};
  /// the original or&(0..40), per body orb weight in percent, 100 neutral,
  /// zero switches a body's aspects off
  std::array<int, 41> weight{};

  AspectSettings() { weight.fill(100); }

  /// Fills orbe with the program's preset, twelve over the divisor in
  /// degrees, two degrees for mirror points, one degree for midpoints.
  void preset_equal_orbs();
};

/// One found aspect.
struct AspectHit {
  int t = 0;  // first body slot
  int w = 0;  // second body slot
  int n = 0;  // divisor, 1 conjunction, 2 opposition, 3 trine family
  int m = 0;  // multiple of the base angle, 1 for the conjunction
};

/// Result of one aspect scan.
struct AspectResult {
  /// the original asp(t,w), the matched angle, two pi for the conjunction
  std::array<std::array<double, 41>, 41> asp{};
  /// the original zh&(n), hits per divisor
  std::array<int, 17> zh{};
  /// the original az&(t), hits per body
  std::array<int, 41> az{};
  /// the original aspz%(14), the Schiemenz Triga count
  int triga = 0;
  /// the original aspz%(15), the Schiemenz grand trine count
  int grand_trines = 0;
  std::vector<AspectHit> hits;
};

/// Scans a chart for aspects, the original asp1 without the drawing.
///
/// @param chart the computed chart, positions with AC and MC on their
///              slots
/// @param s     the chart settings, houses off suppresses angle aspects
///              like the original haw& gates
/// @param a     orb configuration
/// @return the aspect matrix, counters and hit list
[[nodiscard]] AspectResult scan_aspects(const Chart& chart, const ChartSettings& s, const AspectSettings& a);

/// One found midpoint contact.
struct MidpointHit {
  int t = 0;   // body on the midpoint
  int u = 0;   // first of the midpoint pair
  int w = 0;   // second of the midpoint pair
  int nh = 0;  // 1 direct, 2 square, 4 semi square family
};

/// Result of the midpoint scan.
struct MidpointResult {
  int direct = 0;  // the original halbsz1%
  int square = 0;  // halbsz2%
  int semi = 0;    // halbsz3%
  std::vector<MidpointHit> hits;
};

/// Scans a chart for midpoint contacts, the original halbs1 chain.
///
/// The base orb is one degree, or orbe(14) in the equal probability mode.
/// The duplicate guard spans all three passes like the original drk!()
/// cube, and the third body weight only gates positivity, exactly the
/// quirk of orbis_discr3.
///
/// @param chart the computed chart
/// @param s     the chart settings
/// @param a     orb configuration
/// @return counters per pass and the hit list
[[nodiscard]] MidpointResult scan_midpoints(const Chart& chart, const ChartSettings& s, const AspectSettings& a);

}  // namespace horcom
