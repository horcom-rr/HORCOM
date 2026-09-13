// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <vector>

#include "horcom/chart/chart.hpp"

// The exact hit search of the original, plant with its evaluation kernel
// plant1. Given a start moment it walks backward in time with per body
// step sizes, brackets the crossing with his branch reconcilers, then
// converges with his damped secant and one linear interpolation. The
// original states the remaining time error is mostly under five seconds.
namespace horcom {

/// One longitude evaluation, the plant1 view of a body.
struct BodyLongitude {
  bool valid = false;
  double el = 0.0;  // apparent ecliptic longitude, normalised radians
  double tb = 0.0;  // daily motion, negative when retrograde
};

/// The fixed observer of a search, the original globals around plant.
struct SearchContext {
  ChartInput base;        // place of the observer, date part is ignored
  ChartSettings settings; // parallax and calendar rule the evaluations
  const VsopTables* vsop = nullptr;
  const Ephemerides* eph = nullptr;
};

/// A found crossing.
struct LongitudeCrossing {
  bool ok = false;
  double jd_ut = 0.0;
  bool retrograde = false;  // the body ran backward through the point
};

/// Evaluates one body like plant1.
///
/// @param jd_ut the moment in UT
/// @param slot  body slot, 1 Sun through the extra bodies
/// @param ctx   observer and settings
/// @return longitude and daily motion, valid false outside an ephemeris
[[nodiscard]] BodyLongitude body_longitude(double jd_ut, int slot, const SearchContext& ctx);

/// Searches backward from a start moment for the body reaching the
/// target longitude, the first passage of the original plant.
///
/// @param jd_start_ut search starts here and walks into the past
/// @param slot        body slot
/// @param target_rad  the longitude to reach, radians
/// @param ctx         observer and settings
/// @return the crossing, ok false when an ephemeris ends or no crossing
///         converges within the step budget
[[nodiscard]] LongitudeCrossing find_longitude_backward(double jd_start_ut, int slot, double target_rad, const SearchContext& ctx);

/// The solar return of the a16 flow, seeded at the birthday clock of the
/// target year plus fifteen days and searched backward to the radix sun.
///
/// @param birth_ut       the radix moment in UT, day and clock seed the year
/// @param radix_sun_rad  the radix sun longitude, radians
/// @param year           the calendar year of the wanted return
/// @param ctx            observer and settings, the place may differ from
///                       the birth place like the original ort_wahl
/// @return the return moment
[[nodiscard]] LongitudeCrossing solar_return(const CalendarDate& birth_ut, double radix_sun_rad, int year, const SearchContext& ctx);

/// The orbital period of a body in days, the table the return searches
/// pace themselves with.
///
/// @param slot body slot
/// @param tja  length of the tropical year in days
/// @return the period, zero for slots without an orbit
[[nodiscard]] double body_period_days(int slot, double tja);

/// The return of a body onto its own radix longitude, the planetar of
/// the a16 flow. The Nth return seeds itself from the birth moment plus
/// N periods with a per body head start, generous for the slow and
/// eccentric bodies, then the backward search lands on the crossing.
///
/// @param jd_birth_ut the birth moment
/// @param slot        the returning body, Mercury through the extras
/// @param radix_rad   the body's radix longitude, radians
/// @param n           which return, counted from birth
/// @param future      true counts forward in life, false backward
/// @param ctx         observer and settings
/// @return the return moment
[[nodiscard]] LongitudeCrossing planetar_return(double jd_birth_ut, int slot, double radix_rad, int n, bool future, const SearchContext& ctx);

/// The lunar return preceding the given moment, the a16 lunar flow.
///
/// @param jd_before_ut   the search starts here, usually a date at 0h UT
/// @param radix_moon_rad the radix moon longitude, radians
/// @param ctx            observer and settings
/// @return the return moment
[[nodiscard]] LongitudeCrossing lunar_return(double jd_before_ut, double radix_moon_rad, const SearchContext& ctx);

/// The twelve sign entries of a body around a start moment, the ingre1
/// table. Each target is the sign start plus the epsilon that keeps the
/// zero longitude guard quiet, searched backward with plant, the sun
/// corrected into the calendar year of the start.
///
/// @param jd_start_ut the search anchor
/// @param slot        body slot, sun and moon like his menu, planets too
/// @param ctx         observer and settings
/// @return one crossing per sign, Aries first
[[nodiscard]] std::array<LongitudeCrossing, 12> sign_ingresses(double jd_start_ut, int slot, const SearchContext& ctx);

/// One transit event, a running body crossing a radix target.
struct TransitEvent {
  double jd_ut = 0.0;
  int transiting = 0;       // body slot of the running sky
  int radix = 0;            // radix slot, 13 AC and 14 MC included
  int multiple = 0;         // k of the base angle, 0 is the conjunction
  double angle_deg = 0.0;   // k times the base angle
  bool retrograde = false;  // the running body moved backward
  /// a stationary touch near the target, flagged like the original's
  /// slow motion guard, the moment is the middle of its interval
  bool station_touch = false;
};

/// The window and grid of a transit scan, the a180 inputs.
struct TransitScan {
  double jd_from_ut = 0.0;
  double jd_to_ut = 0.0;
  /// the original w4d, radix targets sit at every multiple of this angle
  double base_angle_deg = 30.0;
  /// sweep interval in days, zero picks the a180 table from the bodies
  double step_days = 0.0;
  /// the original mas flag, without it the fast moon stays filtered by
  /// the speed cap exactly like his default
  bool moon_aspects = false;
};

/// Sweeps the window for transits over the radix like a180, the interval
/// bracket tests and the station guard of a180_1 transcribed, each hit
/// refined to the exact moment with the plant search.
///
/// @param radix the birth chart whose positions form the targets
/// @param scan  window and grid
/// @param ctx   observer and settings for the running sky
/// @return the events ordered by time
[[nodiscard]] std::vector<TransitEvent> scan_transits(const Chart& radix, const TransitScan& scan, const SearchContext& ctx);

}  // namespace horcom
