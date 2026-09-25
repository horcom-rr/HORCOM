// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/time/local_time.hpp"

#include "horcom/core/constants.hpp"
#include "horcom/ephem/sunmoon.hpp"
#include "horcom/time/calendar.hpp"

namespace horcom {

LocalTimeRule local_time_rule(int year) {
  //RR WAHRE Ortszeit wird in MITTLERE Ortszeit umgerechnet !
  if (year < kFirstMeanLocalYear) {
    return LocalTimeRule::kTrue;
  }
  //RR Meist war zu diesem Datum bereits MITTLERE Ortszeit üblich !
  if (year < kFirstMeanOnlyYear) {
    return LocalTimeRule::kAsk;
  }
  return LocalTimeRule::kMean;
}

// ported from zeitgleichung, the somo state of the moment feeds it
double equation_of_time_days(double jd) {
  const SunMoonState s = somo(time_arguments(jd), calendar_date(jd));
  //RR zeitgl * up / 15 / 24
  return equation_of_time(s) * kRadToDeg / kDegPerHour / kHoursPerDay;
}

// ported from the ORTSZEIT branch of zuo
double ut_from_local_clock(double jd_local, double lon_deg_east, ClockKind kind) {
  double jd = jd_local;
  if (kind == ClockKind::kTrueLocal) {
    //RR jd = jd - zeitgl * up / 15 / 24
    jd -= equation_of_time_days(jd);
  }
  //RR jd = jd - gl / 360
  return jd - lon_deg_east / kDegPerCircle;
}

}  // namespace horcom
