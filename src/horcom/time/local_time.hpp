// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

// The local times of historic charts, ported from the ORTSZEIT branch of
// the original HORCOM procedure zuo. A clock may run on a time zone, on
// mean local time or on true local time, the sundial time, which the
// equation of time turns into mean local time first.
namespace horcom {

/// Which clock a birth time was read on.
enum class ClockKind {
  kZone,       // a zone time, the zone and summer time lead to UT
  kMeanLocal,  // mittlere Ortszeit, LMT = MOZ
  kTrueLocal,  // wahre Ortszeit, LTT = WOZ
};

/// What zuo assumes for a local time clock of a given year.
enum class LocalTimeRule {
  kTrue,  // before 1810 true local time is converted without asking
  kAsk,   // 1810 to 1889 he asked whether the clock was true or mean
  kMean,  // from 1890 mean local time was the custom
};

/// The first year zuo no longer treats as true local time by default.
inline constexpr int kFirstMeanLocalYear = 1810;
/// The first year zuo takes mean local time without asking.
inline constexpr int kFirstMeanOnlyYear = 1890;

/// The zuo rule for a local time clock of the given year.
///
/// @param year astronomical year of the birth
/// @return true, ask or mean
[[nodiscard]] LocalTimeRule local_time_rule(int year);

/// The equation of time at an epoch, apparent minus mean solar time,
/// ported from the original zeitgleichung.
///
/// @param jd Julian day of the moment
/// @return days, positive when the sundial runs ahead of the mean clock
[[nodiscard]] double equation_of_time_days(double jd);

/// Universal Time from a local clock, ported from zuo.
///
/// @param jd_local     the local clock read as a Julian day
/// @param lon_deg_east geographic longitude, east positive
/// @param kind         kMeanLocal or kTrueLocal
/// @return Julian day UT
[[nodiscard]] double ut_from_local_clock(double jd_local, double lon_deg_east, ClockKind kind);

}  // namespace horcom
