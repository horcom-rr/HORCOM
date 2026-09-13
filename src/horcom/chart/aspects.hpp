// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
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
  std::array<int, body::kSlotCount> weight{};

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
  std::array<std::array<double, body::kSlotCount>, body::kSlotCount> asp{};
  /// the original zh&(n), hits per divisor
  std::array<int, 17> zh{};
  /// the original az&(t), hits per body
  std::array<int, body::kSlotCount> az{};
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

/// The per body orb fraction of the original org, the weight percent
/// over one hundred, halved again on midpoint scans.
///
/// @param a    orb configuration
/// @param slot the body slot whose weight applies
/// @param nh   1 for aspects, 2 for the midpoint passes
/// @return the orb fraction, zero switches the body off
[[nodiscard]] double org(const AspectSettings& a, int slot, int nh);

/// The pair orb of the original orbis_discr2.
///
/// @param o1 first body orb fraction
/// @param o2 second body orb fraction
/// @param dd base orb in radians
/// @return the larger fraction times the base, zero when either body is
///         switched off
[[nodiscard]] double orbis_discr2(double o1, double o2, double dd);

/// The triple orb of the original orbis_discr3.
///
/// @param o1 first body orb fraction
/// @param o2 second body orb fraction
/// @param o3 third body orb fraction, only gates positivity, his quirk
/// @param dd base orb in radians
/// @return the larger of the first two times the base, zero when any
///         body is switched off
[[nodiscard]] double orbis_discr3(double o1, double o2, double o3, double dd);

/// The equal probability multiple filter shared by asp1 and the
/// statistics aspect windows, drops multiples a smaller divisor covers.
///
/// @param n the divisor
/// @param m the multiple 1 to n minus 1
/// @return true when the multiple is scanned under equal orbs
[[nodiscard]] bool multiple_allowed(int n, int m);

/// Names a matched angle for the Aspektarium, the original aspdis.
///
/// The angle folds to the near side, then every divisor up to the scan
/// limit divides it down until a known aspect degree appears, which
/// maps multiples back onto their divisor.
///
/// @param w        the matched angle from the asp matrix, radians, two
///                 pi for the conjunction
/// @param divisors the original nasp&, the highest divisor tried
/// @return his symbol index, 1 to 6 the main aspects, 8 the semi
///         square, 12 the semi sextile, 17 biquintile, 18 quincunx,
///         19 sesquiquadrate, other divisors their own number, 0 when
///         nothing fits
[[nodiscard]] int aspect_symbol(double w, int divisors);

/// One aspect between two charts, the a12asp comparison hit.
struct CrossAspectHit {
  int t = 0;             // first chart slot, the standing radix
  int w = 0;             // second chart slot, the running or compared sky
  int n = 0;             // divisor 1 2 3 4 or 6, the fifth is never scanned
  int m = 0;             // multiple of the base angle
  double sep_deg = 0.0;  // the separation the original lists, 0 to 180
};

/// The comparison scan between two charts, ported from a12asp, the list
/// behind transits and chart comparisons. Divisors one to six without
/// the fifth, the multiples with his skips, and the near side guard
/// that keeps a pair from matching the same angle twice.
///
/// @param first  the standing chart, the radix of a transit view
/// @param second the running or compared chart
/// @param a      orb configuration, the weights like the single scan
/// @param transit_orbs true takes the drgrph rule, the first chart
///               body's weight times one degree
/// @return the hits in scan order
[[nodiscard]] std::vector<CrossAspectHit> scan_aspects_between(const Chart& first, const Chart& second, const AspectSettings& a, bool transit_orbs);

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
/// with_45 opens the fourth pass of the midpoint tree screen, the 45
/// degree level, the three pass counters stay untouched by it
[[nodiscard]] MidpointResult scan_midpoints(const Chart& chart, const ChartSettings& s, const AspectSettings& a, bool with_45 = false);

}  // namespace horcom
