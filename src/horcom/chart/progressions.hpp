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

/// The event moment of taho_proho_ini, the chosen day at the radix
/// clock.
///
/// @param radix       the birth chart
/// @param jd_day0_ut  0h UT of the chosen day
/// @param progression true for proho, whose event reads noon when the
///                    radix clock stands at exactly midnight
/// @return Julian day UT of the event
[[nodiscard]] double event_at_radix_clock(const Chart& radix, double jd_day0_ut, bool progression);

/// The day chart of taho, the moment on the chosen day whose true
/// solar time equals the birth's.
///
/// @param radix     the birth chart
/// @param jd_day_ut the seed, the chosen day at the radix clock from
///                  event_at_radix_clock. The damped walk settles within
///                  the solar day around the seed, a noon seed pushed a
///                  morning birth onto the next day
/// @param ctx       observer and settings, the place of the day chart
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
/// @param radix the birth chart
/// @param life  the life window in jd_from_ut and jd_to_ut with the
///              grid, the choices and the targets of the a18eing boxes
/// @param ctx   observer and settings
/// @return the events ordered by life time
[[nodiscard]] std::vector<DirectedEvent> secondary_direction_events(const Chart& radix, const TransitScan& life, const SearchContext& ctx);

/// The sun or moon arc direction list. The whole radix moves rigidly by
/// the progressed light's arc, so every directed contact is a crossing
/// of that light alone over shifted targets.
///
/// @param radix    the birth chart
/// @param moon_arc true takes the moon's arc, the tertiary flavour
/// @param life     the life window in jd_from_ut and jd_to_ut with the
///                 grid, the choices and the targets of the a18eing
///                 boxes, the choices pick the directed bodies
/// @param ctx      observer and settings
/// @return the events ordered by life time, the directed body in the
///         transiting field, the reached radix point in the radix field
[[nodiscard]] std::vector<DirectedEvent> arc_direction_events(const Chart& radix, bool moon_arc, const TransitScan& life, const SearchContext& ctx);

/// The radix moved by the sun or moon arc of a life date, the directed
/// ring of the HOROSKOP-GRAPHIK branch of a19. The moon arc leaves AC
/// and MC in place like the lists.
///
/// @param radix       the birth chart
/// @param moon_arc    true takes the moon's arc
/// @param jd_life_ut  the life date the arc belongs to
/// @param ctx         observer and settings of the progressed light
/// @return the directed chart, ok false outside the ephemeris
[[nodiscard]] Chart arc_directed_chart(const Chart& radix, bool moon_arc, double jd_life_ut, const SearchContext& ctx);

}  // namespace horcom
