// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/progressions.hpp"

#include <algorithm>
#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/ephem/sunmoon.hpp"
#include "horcom/time/calendar.hpp"
#include "horcom/time/sidereal.hpp"

namespace horcom {

namespace {

// keeps the solved clock within half a day of the plain interpolation,
// the original zeit_korr
double clamp_to_day(double jd, double jdp) {
  if (jd - jdp > 0.5) {
    jd -= 1.0;
  }
  if (jdp - jd > 0.5) {
    jd += 1.0;
  }
  return jd;
}

// ported from HORCOM wahrso. Walks the clock of a day until the sun's
// hour angle equals the wanted one, a damped iteration whose step is
// the angle error as a fraction of the day.
ProgressedMoment solve_true_solar(double v, double jd_seed, const SearchContext& ctx) {
  ProgressedMoment out;
  if (v < 0.0) {
    v += kTwoPi;
  }
  double jd = jd_seed;
  for (int i = 0; i < 120; ++i) {
    const Chart c = sky_chart(jd, ctx);
    if (!c.ok) {
      return out;
    }
    double v1 = sun_hour_angle(c);
    if (v1 < 0.0) {
      v1 += kTwoPi;
    }
    if (std::abs(v - v1) < 1.0e-7) {
      out.ok = true;
      out.jd_ut = jd;
      return out;
    }
    // his step damped by 0.9
    jd -= 0.9 * (v1 - v) / kTwoPi;
  }
  return out;
}

}  // namespace

double sun_hour_angle(const Chart& chart) {
  double w1 = chart.b[body::kSun].ar;
  double w2 = kDegToRad * chart.armc_deg;
  vergl1(w1, w2);
  return w2 - w1;
}

// ported from HORCOM proho with the clock modes of prog_mode
ProgressedMoment progressed_moment(const Chart& radix, double jd_event_ut, ProgressionMode mode, const SearchContext& ctx) {
  ProgressedMoment out;
  const double tja = radix.ta.tropical_year_days;
  //RR 1 TAG = 1 JAHR
  const double j = (jd_event_ut - radix.jd_ut) / tja;
  out.years = j;
  const double jdp = radix.jd_ut + j;
  switch (mode) {
    case ProgressionMode::kRadixClock: {
      // midnight of the progressed day plus the birth clock
      const double birth_clock = radix.jd_ut + 0.5 - std::floor(radix.jd_ut + 0.5);
      const double jd = std::trunc(radix.jd_ut + j) + 0.5 + birth_clock;
      out.jd_ut = clamp_to_day(jd, jdp);
      out.ok = true;
      break;
    }
    case ProgressionMode::kTrueSolarTime: {
      const ProgressedMoment solved = solve_true_solar(sun_hour_angle(radix), jdp, ctx);
      if (!solved.ok) {
        return out;
      }
      out.jd_ut = clamp_to_day(solved.jd_ut, jdp);
      out.ok = true;
      break;
    }
    case ProgressionMode::kHouseRotation: {
      // the birth sidereal time advanced by one day's rotation surplus
      // per year of life, then turned back into a clock, his hd_hs
      const double hs = radix.hs + 0.98565 * j / kDegPerHour;
      const double d0 = std::trunc(radix.jd_ut + j) + 0.5;
      double hd = 0.0;
      for (int i = 0; i < 8; ++i) {
        const double jd = d0 + hd / kHoursPerDay;
        const CalendarDate date = calendar_date(jd, ctx.settings.calendar);
        const SunMoonState smo = somo(time_arguments(jd), date);
        double h0 = gmst0_hours(jd);
        h0 = norm_hours(h0 + smo.dpsi * kRadToRaHours * std::cos(smo.ekls));
        const double next = norm_hours(hs - h0) / kSolarToSiderealRate;
        if (std::abs(next - hd) < 1.0e-9) {
          hd = next;
          break;
        }
        hd = next;
      }
      out.jd_ut = clamp_to_day(d0 + hd / kHoursPerDay, jdp);
      out.ok = true;
      break;
    }
    case ProgressionMode::kProportional:
      out.jd_ut = jdp;
      out.ok = true;
      break;
  }
  return out;
}

// ported from taho_proho_ini, a37dat keeps the radix ho and mi
double event_at_radix_clock(const Chart& radix, double jd_day0_ut, bool progression) {
  double clock = radix.jd_ut + 0.5 - std::floor(radix.jd_ut + 0.5);
  // IF proho! && ho = 0 && mi = 0 : ho = 12
  if (progression && clock == 0.0) {
    clock = 0.5;
  }
  return jd_day0_ut + clock;
}

// ported from HORCOM taho
ProgressedMoment day_chart_moment(const Chart& radix, double jd_day_ut, const SearchContext& ctx) {
  ProgressedMoment out = solve_true_solar(sun_hour_angle(radix), jd_day_ut, ctx);
  out.years = (jd_day_ut - radix.jd_ut) / radix.ta.tropical_year_days;
  return out;
}

// the SECDIR table of the original evaluation screens, the sweep runs
// on the compressed axis and the dates stretch back into life
std::vector<DirectedEvent> secondary_direction_events(const Chart& radix, const TransitScan& life, const SearchContext& ctx) {
  std::vector<DirectedEvent> out;
  const double tja = radix.ta.tropical_year_days;
  TransitScan scan = life;
  //RR 1 TAG = 1 JAHR
  scan.jd_from_ut = radix.jd_ut + (life.jd_from_ut - radix.jd_ut) / tja;
  scan.jd_to_ut = radix.jd_ut + (life.jd_to_ut - radix.jd_ut) / tja;
  for (const TransitEvent& e : scan_transits(radix, scan, ctx)) {
    out.push_back({e, radix.jd_ut + (e.jd_ut - radix.jd_ut) * tja});
  }
  return out;
}

// the SOBDIR table. A rigid arc moves every radix point equally, so
// body t reaches target u exactly when the light itself reaches the
// target shifted by the light's own distance to t
std::vector<DirectedEvent> arc_direction_events(const Chart& radix, bool moon_arc, const TransitScan& life, const SearchContext& ctx) {
  std::vector<DirectedEvent> out;
  const double tja = radix.ta.tropical_year_days;
  const int light = moon_arc ? body::kMoon : body::kSun;
  const double light_el = radix.b[static_cast<std::size_t>(light)].el;
  TransitScan scan = life;
  scan.jd_from_ut = radix.jd_ut + (life.jd_from_ut - radix.jd_ut) / tja;
  scan.jd_to_ut = radix.jd_ut + (life.jd_to_ut - radix.jd_ut) / tja;
  // mas! = -1 in a19, the light runs alone
  scan.moon_aspects = true;
  scan.only_slot = light;
  scan.chosen.clear();
  scan.first_slot = 1;
  std::vector<int> directed;
  for (int t = std::max(1, life.first_slot); t < body::kSlotCount; ++t) {
    const BodyState& moving = radix.b[static_cast<std::size_t>(t)];
    if (!moving.present || !moving.valid || t == body::kNodeDesc) {
      continue;
    }
    if (!life.chosen.empty() && std::find(life.chosen.begin(), life.chosen.end(), t) == life.chosen.end()) {
      continue;
    }
    // IF NOT (tras! OR prog! OR mob! OR mund!) && (t& = 13 OR t& = 14),
    // the moon arc leaves AC and MC in place, the sun arc directs them
    if (moon_arc && (t == body::kAscendant || t == body::kMc)) {
      continue;
    }
    directed.push_back(t);
  }
  // one sweep per directed body, the progress spans all of them
  bool cancelled = false;
  for (std::size_t i = 0; i < directed.size() && !cancelled; ++i) {
    const int t = directed[i];
    const BodyState& moving = radix.b[static_cast<std::size_t>(t)];
    if (life.progress) {
      scan.progress = [&, i](double f) {
        cancelled = !life.progress((static_cast<double>(i) + f) / static_cast<double>(directed.size()));
        return !cancelled;
      };
    }
    Chart shifted = radix;
    const double shift = light_el - moving.el;
    for (int u = 0; u < body::kSlotCount; ++u) {
      BodyState& b = shifted.b[static_cast<std::size_t>(u)];
      if (b.present && b.valid) {
        b.el = norm_rad(radix.b[static_cast<std::size_t>(u)].el + shift);
      }
    }
    // the cusps and the cardinal points shift with every other point
    for (int k = 1; k <= 12; ++k) {
      shifted.houses.cusp[static_cast<std::size_t>(k)] = norm_rad(radix.houses.cusp[static_cast<std::size_t>(k)] + shift);
    }
    scan.cardinal_shift = shift;
    for (TransitEvent e : scan_transits(shifted, scan, ctx)) {
      // the running light stands in for the directed body
      e.transiting = t;
      out.push_back({e, radix.jd_ut + (e.jd_ut - radix.jd_ut) * tja});
    }
  }
  std::sort(out.begin(), out.end(), [](const DirectedEvent& a, const DirectedEvent& b) { return a.jd_life_ut < b.jd_life_ut; });
  return out;
}

// the directed ring of a20_horg for sobg and mob, every point moved by
// the progressed light's travel
Chart arc_directed_chart(const Chart& radix, bool moon_arc, double jd_life_ut, const SearchContext& ctx) {
  Chart out = radix;
  const int light = moon_arc ? body::kMoon : body::kSun;
  const double years = (jd_life_ut - radix.jd_ut) / radix.ta.tropical_year_days;
  //RR 1 TAG = 1 JAHR
  const BodyLongitude progressed = body_longitude(radix.jd_ut + years, light, ctx);
  if (!progressed.valid) {
    out.ok = false;
    return out;
  }
  const double arc = progressed.el - radix.b[static_cast<std::size_t>(light)].el;
  for (int u = 0; u < body::kSlotCount; ++u) {
    if (moon_arc && (u == body::kAscendant || u == body::kMc)) {
      continue;
    }
    BodyState& b = out.b[static_cast<std::size_t>(u)];
    if (b.present && b.valid) {
      b.el = norm_rad(b.el + arc);
    }
  }
  if (!moon_arc) {
    for (int k = 1; k <= 12; ++k) {
      out.houses.cusp[static_cast<std::size_t>(k)] = norm_rad(out.houses.cusp[static_cast<std::size_t>(k)] + arc);
    }
    out.houses.angles.ac = out.b[body::kAscendant].el;
    out.houses.angles.mc = out.b[body::kMc].el;
  }
  return out;
}

}  // namespace horcom
