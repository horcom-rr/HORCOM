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
  kDirect,  ///< D, the body stands in the phase's house
  kRuler,   ///< P, the ruler of the phase's sign
  kRuler2,  ///< P2, the ruler of an intercepted sign
  kRuler3,  ///< P3, the second intercepted sign
  kAspect,  ///< A, an aspect partner of a trigger
  kMirror,  ///< S, a mirror point partner of a trigger
};

/// @param k a trigger kind
/// @return true for the three ruler kinds P, P2 and P3
[[nodiscard]] constexpr bool rhythm_ruler(RhythmKind k) {
  return k == RhythmKind::kRuler || k == RhythmKind::kRuler2 || k == RhythmKind::kRuler3;
}

/// One row of the trigger table.
struct RhythmTrigger {
  /// the walk step, one to twelve
  int phase = 0;
  /// the house this phase walks
  int house = 0;
  /// the house the triggered body stands in, his u& of lj(u&,w&)
  int body_house = 0;
  /// the triggered body, 15 to 18 the cardinal points
  int slot = 0;
  /// the body that pulled it in, aspect and mirror rows only
  int source = 0;
  RhythmKind kind = RhythmKind::kDirect;
  /// the folded aspect angle of an aspect row, degrees
  double angle_deg = 0.0;
  /// the aspect family of an aspect row, his al1& of a1720, the divisor
  /// of the folded angle, zero on the other rows
  int family = 0;
  /// the age in years, a period counted in months is divided by twelve
  double value = 0.0;
};

/// The knobs of the rhythm walk.
struct RhythmOptions {
  /// the period per house, his phas$, years or months after the unit,
  /// negative runs the walk into the past
  double phase_years = 7.0;
  /// true counts the period in months, his mon$ = "E"
  bool months = false;
  /// the house the walk begins with, his BEGINN-PHASE one, four or seven
  int begin_house = 1;
  /// true walks leftward through the houses, false rightward
  bool leftward = true;
  /// true lets the sextile into the aspect chains, his sext switch
  bool sextile = false;
  /// true also triggers the Black Moon's opposite point, his apog!, the
  /// apogee among the chosen bodies
  bool apogee_opposite = false;
  /// true lets the four cardinal points trigger directly, his kard! of
  /// MIT KARDINAL-PUNKTEN
  bool cardinals = false;
  /// the mundane frame of horm& 2, his a173 knows no mirror chain there
  bool mundane = false;
  /// his alt!, the old rulers Mars, Saturn and Jupiter for Scorpio,
  /// Aquarius and Pisces
  bool classic_rulers = false;
  /// the SONDERPUNKT ( FIXPUNKT ) of the rhythm theory, his red F, a
  /// degree in radians, negative when off, independent of the general
  /// fixed point
  double special = -1.0;
};

/// The number of phases a walk steps through, his loop of a170 from the
/// BEGINN-PHASE to the last house.
///
/// @param opt the start house of the walk
/// @return thirteen less the start house
[[nodiscard]] int rhythm_phase_count(const RhythmOptions& opt);

/// The house a phase of the walk stands on, his l& of a170.
///
/// @param opt   the start house and the direction
/// @param phase the walk step, one for the first
/// @return the house, counted up from the start leftward and down from
///         its mirror rightward
[[nodiscard]] int rhythm_phase_house(const RhythmOptions& opt, int phase);

/// Runs the trigger walk over a chart.
///
/// @param chart   the radix with its houses
/// @param aspects a scan whose matrix feeds the aspect chains, run
///                with divisors six when the sextile is open, four
///                otherwise, like the original forced nasp. A Sonderpunkt
///                on slot zero of the scanned chart chains like a body
/// @param a       orb configuration, the mirror matrix reads it
/// @param opt     phase length, unit, start, direction and switches
/// @return the triggers in walk order
[[nodiscard]] std::vector<RhythmTrigger> rhythm_triggers(const Chart& chart, const AspectResult& aspects, const AspectSettings& a, const RhythmOptions& opt);

/// his tja before the first chart sets it, the tropical year in days
inline constexpr double kInitialTropicalYearDays = 365.24219878;

/// Where the ages of a walk stand in time, the jd and sn of a174init.
struct RhythmClock {
  /// the moment of age zero of the walk, the birth for the radix, the
  /// chart's own moment for Solar, Lunar and the other derived charts,
  /// the birth plus the Septar offset for a Septar
  double base_jd = 0.0;
  /// the life years before the walk, his sn, the age of a Solar or the
  /// offset of a Septar, zero elsewhere
  double sn = 0.0;
  /// the tropical year in days, his tja
  double tja = kInitialTropicalYearDays;
};

/// The life years before the walk of a Septar, his sn of a174init.
///
/// @param sen the Septar number, one for the first
/// @param opt the period and its unit
/// @return (sen - 1) periods of twelve houses in years
/// @note The original took (sen - 1) * vp in either unit. A Septar spans
///       vp years in the month unit but twelve vp in the year unit, so
///       the year unit dated the third Septar with one year per house two
///       years after birth instead of twenty four.
[[nodiscard]] double septar_offset(int sen, const RhythmOptions& opt);

/// The moment a trigger age falls on.
///
/// @param c     the clock of the walk
/// @param years the trigger age in years
/// @return the julian day in UT
[[nodiscard]] double rhythm_jd(const RhythmClock& c, double years);

/// The age of the walk at a moment, the exact inverse of rhythm_jd.
///
/// @param c  the clock of the walk
/// @param jd the julian day in UT
/// @return the age in years
/// @note His date defined Sonderpunkt took lpkt against the chart's own
///       moment and then subtracted sn once more, a Solar landed its age
///       too early by the whole age of the Solar.
[[nodiscard]] double rhythm_years(const RhythmClock& c, double jd);

/// A trigger age in his LJ and MO columns.
struct RhythmAge {
  bool negative = false;
  int years = 0;
  /// the month inside the year, whole in the graph, one decimal in the
  /// table
  double months = 0.0;
};

/// Splits an age into years and months, a175 for the table and a178
/// for the graph.
///
/// @param years        the trigger age in years
/// @param sn           the life years before the walk
/// @param whole_months true rounds to whole months like a178, false to
///                     tenths like a175
/// @return sign and magnitude, a rounded twelfth month carries
/// @note His a175 took the months from the walk age alone and the years
///       from sn plus the age with INT, a178 cut the years with FIX, so
///       negative periods read -3 -3.0 for minus two years three months
///       and no rounded twelfth month ever carried.
[[nodiscard]] RhythmAge rhythm_age(double years, double sn, bool whole_months);

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
  /// life years at the crossing, a month period divided by twelve
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
/// @return the ecliptic degree in radians, negative without houses and
///         outside the twelve phases of the walk
[[nodiscard]] double degree_at_age(const Chart& chart, const RhythmOptions& opt, double years);

/// Reads self defined degrees, his GRADE.INT lines of degree and two
/// planet slots separated by commas.
///
/// @param file the file beside the data
/// @return the rows, empty when the file is absent
[[nodiscard]] std::vector<CustomDegree> read_degrees(const std::filesystem::path& file);

/// Writes the self defined degrees back in the GRADE.INT shape.
///
/// @param file the file beside the data, replaced as a whole
/// @param rows the degrees to keep
/// @return true when every row was written
bool write_degrees(const std::filesystem::path& file, const std::vector<CustomDegree>& rows);

/// Whether a half degree already carries a characteristic, one of the
/// published Gruppenschicksals-Grade or a self defined degree, his
/// gs&(g&,0) = g& test.
///
/// @param half the degree times two
/// @param own  the self defined degrees
/// @return true for a known point
[[nodiscard]] bool degree_known(int half, const std::vector<CustomDegree>& own);

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
