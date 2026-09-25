// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <functional>
#include <vector>

#include "horcom/chart/chart.hpp"
#include "horcom/core/constants.hpp"

// The exact hit search of the original, plant with its evaluation kernel
// plant1. Given a start moment it walks backward in time with per body
// step sizes, brackets the crossing with his branch reconcilers, then
// converges with his damped secant and one linear interpolation. The
// original states the remaining time error is mostly under five seconds.
namespace horcom {

/// One arc minute a day in radians. His tg of a181tx1 marks a body this
/// slow as near its station, the guard of a180_1 scales it by the
/// distance.
inline constexpr double kStationDailyMotion = 0.000290888;

/// A sweep reports the done fraction of its window through this and
/// stops early when it answers false, the ESC of his raus.
using SweepProgress = std::function<bool(double)>;

/// How many multiples of a base angle fit the circle, his
/// INT(360.1 / (w4d + kk)). The tenth of a degree keeps a divisor like
/// seven from rounding down to six.
///
/// @param base_angle_deg the base angle, his w4d
/// @return the number of multiples, the conjunction included
[[nodiscard]] int base_multiples(double base_angle_deg);

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

/// The whole sky of one moment at the observer of a search, the chain
/// every sweep evaluates.
///
/// @param jd_ut the moment in UT
/// @param ctx   observer and settings
/// @return the chart, ok false outside the ephemerides
[[nodiscard]] Chart sky_chart(double jd_ut, const SearchContext& ctx);

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
/// @param ctx         observer and settings, the heliocentric mode seeds
///                    every body a tenth of its period past the count
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

/// The sign entries of the ascendant or the midheaven after a start
/// moment. The angles turn through the whole circle every day, so
/// every entry lies within the following day. A damped iteration on
/// the daily turn lands on each one, a deliberate simplification over
/// the planetary stepping which the daily wrap would mislead.
///
/// @param jd_start_ut the search anchor, entries follow it
/// @param slot        13 the ascendant, 14 the midheaven
/// @param ctx         observer and settings
/// @return one crossing per sign, Aries first
[[nodiscard]] std::array<LongitudeCrossing, 12> angle_ingresses(double jd_start_ut, int slot, const SearchContext& ctx);

/// The combined GRUND-ASPEKT choices of a18eing leave some multiples of
/// their base angle out, his f3045, f6045 and f6090 filters.
enum class AspectGrid {
  kPlain,     ///< every multiple of the base angle
  k30And45,   ///< 15 degree base, the multiples of 30 and of 45
  k60And45,   ///< 15 degree base, the multiples of 45 and of 60
  k60And90,   ///< 30 degree base, the multiples of 60 and of 90
};

/// Tells whether a combined grid leaves a multiple of its base out.
/// Ported from HORCOM a18_3045.
///
/// @param grid the grid of the GRUND-ASPEKT
/// @param w    the multiple of the base angle, zero the conjunction
/// @return true when the multiple is no aspect of the grid
[[nodiscard]] bool grid_skips(AspectGrid grid, int w);

/// One transit event, a running body crossing a radix target.
struct TransitEvent {
  double jd_ut = 0.0;
  int transiting = 0;       // body slot of the running sky
  int radix = 0;            // radix slot, 13 AC and 14 MC included,
                            // 15 to 18 the cardinal points 0 AR to 0 CP
  int radix2 = 0;           // the second factor of a midpoint target
  int cusp = 0;             // an intermediate cusp 2, 3, 5 or 6, radix 0
  int multiple = 0;         // k of the base angle, 0 is the conjunction
  double angle_deg = 0.0;   // k times the base angle
  bool retrograde = false;  // the running body moved backward
  double speed = 0.0;       // its daily motion in the interval, radians
  /// a stationary touch near the target, flagged like the original's
  /// slow motion guard, the moment is the middle of its interval
  bool station_touch = false;
};

/// The window and grid of a transit scan, the a180 inputs.
struct TransitScan {
  double jd_from_ut = 0.0;
  double jd_to_ut = 0.0;
  /// the original w4d, radix targets sit at every multiple of this angle
  double base_angle_deg = kDefaultBaseAngleDeg;
  /// sweep interval in days, zero picks the a180 table from the bodies
  double step_days = 0.0;
  /// his lin! = 0, the tables list the stationary touches of a180_1,
  /// the linear graph draws only exact crossings
  bool station_touches = true;
  /// reports the progress of the sweep and cancels it, empty runs through
  SweepProgress progress;
  /// the original mas flag, without it the fast moon stays filtered by
  /// the speed cap exactly like his default
  bool moon_aspects = false;
  /// restricts the sweep to one running body, zero runs them all, the
  /// arc directions send only their light through
  int only_slot = 0;
  /// the combined GRUND-ASPEKT filter over the base angle
  AspectGrid grid = AspectGrid::kPlain;
  /// his pl1, 5 starts the running bodies at Mars, 6 at Jupiter
  int first_slot = 1;
  /// NUR DIESE DARSTELLEN, the chosen running bodies, empty runs all
  std::vector<int> chosen;
  /// his delblm, the true black moon stays out as a running body
  bool drop_true_apogee = false;
  /// DIREKTIONEN MIT ZWISCHENHÄUSERN, the cusps of houses 2, 3, 5 and 6
  /// join the targets, their opposites come with the multiples
  bool house_targets = false;
  /// MIT KARDINAL-PUNKTEN, 0 AR, 0 CN, 0 LI and 0 CP join the targets
  bool cardinal_targets = false;
  /// his halbs_dir, 1 adds the midpoint of every pair of radix points,
  /// 2 only those the running body is no factor of
  int midpoints = 0;
  /// the arc directions move every radix point by one arc, the fixed
  /// cardinal points ride along by this shift
  double cardinal_shift = 0.0;
};

/// Sweeps the window for transits over the radix like a180, the interval
/// bracket tests and the station guard of a180_1 transcribed. His
/// a180aus quadratic seeds each hit, a guarded secant inside the
/// interval then closes on the exact moment, direct and retrograde
/// passages alike.
///
/// @param radix the birth chart whose positions form the targets
/// @param scan  window and grid
/// @param ctx   observer and settings for the running sky
/// @return the events ordered by time, the ones found so far when the
///         progress callback cancelled
[[nodiscard]] std::vector<TransitEvent> scan_transits(const Chart& radix, const TransitScan& scan, const SearchContext& ctx);

/// The earlier passages of a planetar, the walk of planth. A body in a
/// retrograde loop crosses the returned point three times, the search
/// finds the last one and his menu walks back to the retrograde and the
/// first direct passage, up to two more for Mercury to Saturn and the
/// belt and eight for the slow bodies, stopping when no passage turns up
/// within his djd of the one before.
///
/// @param found      the passage the search found
/// @param slot       the returning body
/// @param target_rad the radix longitude
/// @param ctx        observer and settings
/// @return the earlier passages, the nearest first
[[nodiscard]] std::vector<LongitudeCrossing> planetar_earlier(const LongitudeCrossing& found, int slot, double target_rad, const SearchContext& ctx);

/// The period a planetar is counted and shown with. His a16_ta made the
/// periods of Jupiter to Pluto too long, the count takes the sidereal
/// periods of the mean motions, and the geocentric Mercury and Venus
/// return once a tropical year like his own NR seeds count them.
///
/// @param slot the returning body
/// @param tja  the tropical year in days
/// @param helio the heliocentric mode, where Mercury and Venus count
///              their revolutions
/// @return the counting period in days
[[nodiscard]] double planetar_count_period(int slot, double tja, bool helio);

/// One exact aspect between two running bodies, a row of the MUNDAN-
/// ASPEKTE table.
struct MundaneAspect {
  double jd_ut = 0.0;
  int first = 0;             // the lower body slot, his u
  int second = 0;            // the higher body slot, his t
  int multiple = 0;          // k of the base angle, 0 is the conjunction
  double angle_deg = 0.0;    // folded into 0 to 180 like his was5
  double first_lon = 0.0;    // the first body at the moment, radians
  double second_lon = 0.0;   // the second body at the moment, radians
};

/// The window and the a18eing answers of a MUNDAN-ASPEKTE run.
struct MundaneScan {
  double jd_from_ut = 0.0;
  double jd_to_ut = 0.0;
  /// the original w4d, the running bodies meet at its multiples
  double base_angle_deg = kDefaultBaseAngleDeg;
  AspectGrid grid = AspectGrid::kPlain;
  /// his pl1, 5 starts the running bodies at Mars, 6 at Jupiter
  int first_slot = 1;
  /// NUR DIESE DARSTELLEN, the chosen running bodies, empty runs all
  std::vector<int> chosen;
  /// MOND BERÜCKSICHTIGEN, the moon joins the running bodies
  bool moon = false;
  /// his delblm, the true black moon stays out
  bool drop_true_apogee = false;
  /// his stund_ast, a body counts only until it leaves the sign it
  /// stood in at the start, the void of course rule
  bool within_sign = false;
  /// reports the progress of the sweep and cancels it, empty runs through
  SweepProgress progress;
};

/// Sweeps the window for the exact aspects the running bodies make with
/// each other. Every window of three samples gets his bracket test and
/// the quadratic interpolation of the difference for the moment.
/// Ported from HORCOM mund1, its table branch.
///
/// @param scan window, grid and choices
/// @param ctx  observer and settings of the running sky
/// @return the aspects ordered by time
[[nodiscard]] std::vector<MundaneAspect> scan_mundane_aspects(const MundaneScan& scan, const SearchContext& ctx);

}  // namespace horcom
