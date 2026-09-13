// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include "horcom/chart/chart.hpp"
#include "horcom/chart/transit_search.hpp"

// Secondary progressions and the day chart, ported from the original
// proho and taho. A progression maps one day of sky motion onto one
// year of life, so the chart of the thirtieth day after birth stands
// for the thirtieth year. The day chart finds the clock time on any
// chosen day at which the sun holds the same hour angle it held at
// birth, the moment of the same true solar time.
namespace horcom {

/// The four clock answers of the progression dialog, his prog_mode.
enum class ProgressionMode {
  /// the progressed day keeps the birth clock time
  kRadixClock = 1,
  /// the progressed day takes the birth's true solar time
  kTrueSolarTime = 2,
  /// the houses turn at one day per year, near four clock minutes
  /// of sidereal time per year of life
  kHouseRotation = 3,
  /// the julian date interpolates strictly, days over the year length
  kProportional = 4,
};

/// A solved moment, the found julian date in UT.
struct ProgressedMoment {
  bool ok = false;
  double jd_ut = 0.0;
  /// elapsed tropical years between birth and event, his lja
  double years = 0.0;
};

/// The hour angle of the sun, sidereal time minus the sun's right
/// ascension. Birth and day chart share this value when their clocks
/// show the same true solar time.
///
/// @param chart a computed chart
/// @return the angle in radians, zero to two pi
[[nodiscard]] double sun_hour_angle(const Chart& chart);

/// The secondary progression of proho, one day per tropical year.
///
/// @param radix       the birth chart
/// @param jd_event_ut the life moment the progression stands for
/// @param mode        which clock the progressed day carries
/// @param ctx         observer and settings, the place may differ from
///                    the birth place
/// @return the progressed moment
[[nodiscard]] ProgressedMoment progressed_moment(const Chart& radix, double jd_event_ut, ProgressionMode mode, const SearchContext& ctx);

/// The day chart of taho, the moment on the chosen day whose true
/// solar time equals the birth's.
///
/// @param radix     the birth chart
/// @param jd_day_ut any moment of the chosen day, the seed
/// @param ctx       observer and settings
/// @return the solved moment
[[nodiscard]] ProgressedMoment day_chart_moment(const Chart& radix, double jd_day_ut, const SearchContext& ctx);

/// One direction event on the life axis. The transit machinery finds it
/// on the compressed day for a year scale, the life date says when the
/// direction becomes exact in real time.
struct DirectedEvent {
  TransitEvent event;
  double jd_life_ut = 0.0;
};

/// The secondary direction list. Every progressed body against the
/// radix targets over a life window, the a180 sweep run on the day for
/// a year axis and mapped back.
///
/// @param radix        the birth chart
/// @param jd_from_ut   life window start
/// @param jd_to_ut     life window end
/// @param base_angle_deg the aspect grid in degrees
/// @param ctx          observer and settings
/// @return the events ordered by life time
[[nodiscard]] std::vector<DirectedEvent> secondary_direction_events(const Chart& radix, double jd_from_ut, double jd_to_ut, double base_angle_deg, const SearchContext& ctx);

/// The sun or moon arc direction list. The whole radix moves rigidly by
/// the progressed light's arc, so every directed contact is a crossing
/// of that light alone over shifted targets.
///
/// @param radix        the birth chart
/// @param moon_arc     true takes the moon's arc, the tertiary flavour
/// @param jd_from_ut   life window start
/// @param jd_to_ut     life window end
/// @param base_angle_deg the aspect grid in degrees
/// @param ctx          observer and settings
/// @return the events ordered by life time, the directed body in the
///         transiting field, the reached radix point in the radix field
[[nodiscard]] std::vector<DirectedEvent> arc_direction_events(const Chart& radix, bool moon_arc, double jd_from_ut, double jd_to_ut, double base_angle_deg, const SearchContext& ctx);

}  // namespace horcom
