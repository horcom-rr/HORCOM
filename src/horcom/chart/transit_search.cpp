// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/transit_search.hpp"

#include <algorithm>
#include <cmath>
#include <map>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the interactive escapes and timers of the original become step budgets
constexpr int kCoarseBudget = 20000;
constexpr int kFineBudget = 4000;
constexpr int kFinestBudget = 400;

// the damped reversal factor of the finest phase
constexpr double kReversalDamping = -0.31;
// the convergence gates of the finest phase
constexpr double kAngleGate = 0.00000001;
constexpr double kStepGate = 0.0000001;

// the step groups of plant, main bodies divide by their slot number,
// the Chiron group by ten, the asteroid group by five
bool group_main(int slot) {
  return (slot >= 3 && slot <= 10) || slot == body::kQuaoar || slot == body::kXena;
}

bool group_chiron(int slot) {
  return slot == body::kChiron || slot == body::kHalley || slot == body::kPholus ||
         slot == body::kDamokles || slot == body::kNessus;
}

bool group_asteroid(int slot) {
  return slot >= body::kCeres && slot <= body::kVesta;
}

// the search table of plant, initial step in days, search interval and
// convergence tolerance per body
struct StepTable {
  double dt1 = 1.0;
  double djd = 0.0;
  double tbg = 0.0;
};

StepTable step_table(int slot) {
  switch (slot) {
    case 1: return {1.0, 0.0, 0.0};
    case 2: return {2.0, 0.0, 0.0};
    case 3: return {3.0, 81.0, 0.017453293};   //RR 1 Grad
    case 4: return {4.0, 100.0, 0.008726646};  //RR 1/2 Grad
    case 5: return {6.0, 200.0, 0.007272205};  //RR 25'
    case 6: return {5.0, 260.0, 0.002327106};  //RR 8'
    case 7: return {5.0, 320.0, 0.001454444};  //RR 5'
    case 8: return {5.0, 320.0, 0.0007272};    //RR 2.5'
    case 9: return {5.0, 320.0, 0.00046542};   //RR 1.6'
    default:
      if (group_asteroid(slot)) {
        return {3.0, 200.0, 0.001454444};
      }
      if (group_chiron(slot)) {
        return {5.0, 320.0, 0.001454444};
      }
      // Pluto, Quaoar and Xena share the widest steps
      return {10.0, 320.0, 0.00046542};
  }
}

// the period table of a16_ta in days, tja is the tropical year
double period_days(int slot, double tja) {
  switch (slot) {
    case 1: return tja;
    case 2: return 27.321582;
    case 3: return 0.24085 * tja;
    case 4: return 0.61521 * tja;
    case 5: return 1.88089 * tja;
    case 6: return 11.869 * tja;
    case 7: return 29.628 * tja;
    case 8: return 84.665 * tja;
    case 9: return 165.49 * tja;
    case 10: return 251.86 * tja;
    default:
      switch (slot) {
        case body::kChiron: return 18339.66994;
        case body::kCeres: return 4.61 * tja;
        case body::kPallas: return 4.613 * tja;
        case body::kJuno: return 4.36 * tja;
        case body::kVesta: return 3.63 * tja;
        case body::kQuaoar: return 288.573638 * tja;
        case body::kHalley: return 360.0 / 0.01296738419;
        case body::kPholus: return 360.0 / 0.0108356829269;
        case body::kDamokles: return 360.0 / 0.0242523113827;
        case body::kNessus: return 360.0 / 0.0081492640834;
        case body::kXena: return 556.5767 * tja;
        default: return tja;
      }
  }
}

// ported from ver1r, reconciles the projected angle w4 with the pair
void ver1r(double& w1, double& w2, double w4) {
  const double w11 = w1 + kTwoPi;
  const double w22 = w2 + kTwoPi;
  if (w4 > w1 + kPi && w1 > 0.0) {
    if (w2 < w1 && w22 > w4 && w11 > w22) {
      w2 = w22;
    }
    w1 = w11;
  }
}

// ported from ver, the first passage form with zr at zero
void ver0(double tb1, double& w1, double& w2, double& w3) {
  if (tb1 > 0.0) {
    vergl2(w1, w2, w3);
    if (w2 <= kEps && w2 < kPi) {
      w2 += kTwoPi;
      if (w2 - w1 > kPi) {
        w1 += kTwoPi;
      }
      if (w2 - w3 > kPi) {
        w3 += kTwoPi;
      }
    } else if (w2 <= kEps + kTwoPi) {
      if (w2 - w1 > kPi) {
        w1 += kTwoPi;
      }
      if (w2 - w3 > kPi) {
        w3 += kTwoPi;
      }
    }
  } else {
    vergl2r(w1, w2, w3);
  }
}

// the adaptive backward step of the coarse and fine phases, gap times
// period over the group divisor over six
double adaptive_step(int slot, double from, double to, double period, double floor_days) {
  double divisor = 10.0;
  if (group_main(slot)) {
    divisor = static_cast<double>(slot);
  } else if (group_asteroid(slot)) {
    divisor = 5.0;
  }
  double gap;
  if (to - from > kPi && from + kTwoPi > to) {
    gap = from + kTwoPi - to;
  } else {
    gap = from - to;
  }
  return std::max(std::abs(gap) * period / divisor / 6.0, floor_days);
}

Chart eval_chart(double jd_ut, const SearchContext& ctx) {
  ChartInput in = ctx.base;
  in.date_ut = calendar_date(jd_ut, ctx.settings.calendar);
  return compute_chart(in, ctx.settings, *ctx.vsop, *ctx.eph);
}

}  // namespace

// ported from plant1, the pipeline runs the same chain dat, juld1, the
// parallax branch, utet, soko with the body, etut and a316061
BodyLongitude body_longitude(double jd_ut, int slot, const SearchContext& ctx) {
  const Chart c = eval_chart(jd_ut, ctx);
  const BodyState& b = c.b[static_cast<std::size_t>(slot)];
  BodyLongitude out;
  out.valid = c.ok && b.present && b.valid;
  out.el = norm_rad(b.el);
  out.tb = b.tb;
  return out;
}

// ported from plant, the first passage with the interactive escapes as
// step budgets
LongitudeCrossing find_longitude_backward(double jd_start_ut, int slot, double target_rad, const SearchContext& ctx) {
  LongitudeCrossing out;
  const StepTable st = step_table(slot);
  const double tja = eval_chart(jd_start_ut, ctx).ta.tropical_year_days;
  const double period = period_days(slot, tja);
  const bool planet = slot > 2;

  double jd = jd_start_ut;
  //RR wegen Sonne/Mond Start etwas vor dem Zeitpunkt
  if (!planet) {
    jd += st.dt1 / 2.0;
  }

  double dt = st.dt1;
  double dtm = 0.0;
  double w1 = 0.0;
  double w2 = 0.0;
  double w3 = 0.0;
  double w2m = 0.0;
  double tb1 = 0.0;
  double tb_now = 0.0;
  int budget = kCoarseBudget;

  // the coarse phase, backward until the target is bracketed
  bool restart = true;
  while (restart) {
    restart = false;
    BodyLongitude e = body_longitude(jd, slot, ctx);
    if (!e.valid) {
      //RR KEINE EPHEMERIDE MEHR !
      return out;
    }
    w2 = norm_rad(e.el);
    tb_now = e.tb;
    w3 = target_rad;
    w2m = w2;
    ver0(tb1, w1, w2, w3);
    tb1 = tb_now;
    for (;;) {
      if (--budget < 0) {
        return out;
      }
      w1 = w2m;
      tb1 = tb_now;
      const double tbm = tb1;
      if (planet) {
        dt = adaptive_step(slot, w1, w3, period, st.dt1);
        if (std::abs(dtm - dt) < st.dt1) {
          dt = st.dt1;
        }
      } else {
        dt = st.dt1;
      }
      jd -= dt;
      dtm = dt;
      e = body_longitude(jd, slot, ctx);
      if (!e.valid) {
        return out;
      }
      w2 = e.el;
      w2m = w2;
      tb1 = (tb1 + e.tb) / 2.0;
      tb_now = e.tb;
      w3 = target_rad;
      ver0(tb1, w1, w2, w3);
      double w4 = norm_rad(w1 - std::abs(tbm * dt));
      ver1r(w1, w2, w4);
      if (w1 > w3 && (w4 < w3 || w2 < w3) && std::abs(w1 - w2) < kPi && std::abs(w3 - w2) < kPi) {
        break;
      }
    }
    if (jd > jd_start_ut) {
      jd -= st.dt1;
      dt = st.dt1;
      restart = true;
      continue;
    }
    jd += dtm;
    if (dt > st.dt1 && planet) {
      restart = true;
    }
  }

  // the fine phase of the planets, crossing detected on the way down
  if (planet) {
    jd += dt;
    BodyLongitude e = body_longitude(jd, slot, ctx);
    if (!e.valid) {
      return out;
    }
    w2 = e.el;
    w2m = w2;
    tb_now = e.tb;
    w3 = target_rad;
    budget = kFineBudget;
    for (;;) {
      if (--budget < 0) {
        return out;
      }
      w1 = w2m;
      tb1 = tb_now;
      // the asteroid group steps by the seen motion, the others by the
      // remaining gap, the floor is the slot number in days
      const double to = group_asteroid(slot) ? w2 : w3;
      dt = adaptive_step(slot, w1, to, period, static_cast<double>(slot));
      jd -= dt;
      e = body_longitude(jd, slot, ctx);
      if (!e.valid) {
        return out;
      }
      w2 = e.el;
      w2m = w2;
      tb1 = (tb1 + e.tb) / 2.0;
      tb_now = e.tb;
      w3 = target_rad;
      ver0(tb1, w1, w2, w3);
      const double dw = std::abs(w2 - w1);
      if (w2 < w3 && w1 > w3 && dw < kPi && w2 - w1 < 0.0 && dt > 0.0) {
        break;
      }
    }
    jd += dt;
  }

  // the finest phase, a damped secant with the original reversal factor
  {
    w1 = w2m;
    BodyLongitude e = body_longitude(jd, slot, ctx);
    if (!e.valid) {
      return out;
    }
    w2 = e.el;
    w3 = target_rad;
    ver0(tb1, w1, w2, w3);
    tb1 = e.tb;
    tb_now = e.tb;
    w2m = w2;
    double jdm = jd;
    double d = std::abs(w3 - w2);
    budget = kFinestBudget;
    for (;;) {
      if (--budget < 0) {
        break;
      }
      const int s1 = w3 - w2 > 0.0 ? 1 : (w3 - w2 < 0.0 ? -1 : 0);
      w1 = w2m;
      jd -= dt;
      jdm = jd;
      tb1 = tb_now;
      e = body_longitude(jd, slot, ctx);
      if (!e.valid) {
        return out;
      }
      w2 = e.el;
      w2m = w2;
      tb1 = (tb1 + e.tb) / 2.0;
      tb_now = e.tb;
      w3 = target_rad;
      ver0(tb1, w1, w2, w3);
      d = std::abs(w3 - w2);
      if (d > kPi) {
        if (w2 < kPi && w1 < kPi && w3 > kPi) {
          w2 += kTwoPi;
          w1 += kTwoPi;
          d = std::abs(w2 - w3);
        } else if (w2 > kPi && w1 > kPi && w3 < kPi) {
          w3 += kTwoPi;
          d = std::abs(w2 - w3);
        }
      }
      const int s2 = w3 - w2 > 0.0 ? 1 : (w3 - w2 < 0.0 ? -1 : 0);
      if (s1 != s2) {
        dt *= kReversalDamping;
      }
      if (d < kAngleGate || std::abs(dt) < kStepGate) {
        break;
      }
    }
    //RR Der INTERPOLATIONSFEHLER in ZEIT beträgt meist < 5 Sekunden !
    if (std::abs(w1 - w2) > kEps) {
      const double dtk = std::abs(dt * (w2 - w3) / (w1 - w2));
      jd = jdm - dtk;
    }
  }

  const BodyLongitude done = body_longitude(jd, slot, ctx);
  if (!done.valid) {
    return out;
  }
  out.ok = true;
  out.jd_ut = jd;
  out.retrograde = done.tb < 0.0;
  return out;
}

// ported from the a16 solar branch and korh
LongitudeCrossing solar_return(const CalendarDate& birth_ut, double radix_sun_rad, int year, const SearchContext& ctx) {
  //RR jd-Startwert
  constexpr double kSolarSeedDays = 15.0;
  CalendarDate seed = birth_ut;
  seed.year = year;
  const double jd = julian_day(seed, ctx.settings.calendar);
  return find_longitude_backward(jd + kSolarSeedDays, body::kSun, radix_sun_rad, ctx);
}

// ported from the a16 lunar branch, the return preceding the moment
LongitudeCrossing lunar_return(double jd_before_ut, double radix_moon_rad, const SearchContext& ctx) {
  return find_longitude_backward(jd_before_ut, body::kMoon, radix_moon_rad, ctx);
}

namespace {

// the caps of a180_1, the speed limit per interval, the stationary
// motion bound and its nearness window
constexpr double kSweepSpeedCap = 0.26177;
constexpr double kStationMotion = 0.000290888;
constexpr double kStationNear = 0.004363;  //RR ca.0.25°

// the interval table of a180 keyed by the slowest running body
double sweep_interval(const std::vector<int>& transiting, const ChartSettings& s) {
  double ival = 1.0;
  int fastest = 41;
  for (const int t : transiting) {
    fastest = std::min(fastest, t);
  }
  switch (fastest) {
    case 1:
    case 2: ival = 2.0; break;
    case 5: ival = 2.0; break;
    case 6:
    case 7:
    case 8: ival = 6.0; break;
    default: ival = 1.0; break;
  }
  //RR apogw
  if (s.true_apogee || s.nk[1] > 0) {
    ival = 0.5;
  }
  return ival;
}

}  // namespace

// ported from a180 with a180_1, the interval sweep over the window
std::vector<TransitEvent> scan_transits(const Chart& radix, const TransitScan& scan, const SearchContext& ctx) {
  std::vector<TransitEvent> out;
  if (!(scan.jd_to_ut > scan.jd_from_ut) || scan.base_angle_deg <= 0.0) {
    return out;
  }

  // the running bodies, the nodes' south end and the axes stay out like
  // the a18st filters, the part of fortune never transits
  std::vector<int> transiting;
  for (int t = 1; t <= 40; ++t) {
    if (t == body::kNodeDesc || t == body::kAscendant || t == body::kMc || t == body::kFortune) {
      continue;
    }
    const BodyState& b = radix.b[static_cast<std::size_t>(t)];
    if (b.present) {
      transiting.push_back(t);
    }
  }
  // the radix targets, bodies plus the AC and MC, the south node out
  std::vector<int> targets;
  for (int u = 1; u <= 40; ++u) {
    if (u == body::kNodeDesc) {
      continue;
    }
    const BodyState& b = radix.b[static_cast<std::size_t>(u)];
    if (b.present && b.valid) {
      targets.push_back(u);
    }
  }

  const double base = scan.base_angle_deg * kDegToRad;
  const int multiples = static_cast<int>(360.1 / scan.base_angle_deg);
  const double ival = scan.step_days > 0.0 ? scan.step_days : sweep_interval(transiting, ctx.settings);

  // the c3 memory of the original, one stationary touch per running
  // body and target for the whole run
  std::map<std::pair<int, int>, int> station_done;

  Chart a = eval_chart(scan.jd_from_ut, ctx);
  double jd = scan.jd_from_ut;
  while (jd < scan.jd_to_ut && a.ok) {
    const double jd2 = std::min(jd + ival, scan.jd_to_ut);
    const Chart b = eval_chart(jd2, ctx);
    if (!b.ok) {
      break;
    }
    for (const int t : transiting) {
      const auto ti = static_cast<std::size_t>(t);
      if (!a.b[ti].valid || !b.b[ti].valid) {
        continue;
      }
      const double ca = norm_rad(a.b[ti].el);
      const double cb = norm_rad(b.b[ti].el);
      for (const int u : targets) {
        const auto ui = static_cast<std::size_t>(u);
        for (int k = 0; k < multiples; ++k) {
          const double wv = norm_rad(radix.b[ui].el + k * base);
          bool hit = false;
          bool touch = false;
          // the stationary guard of a180_1, a slow body touching the
          // target inside the interval, remembered once per pair
          const double tba = std::abs(cb - ca);
          if (t != 1 && t != 2 && !(t > 11 && t < 19) && t != body::kApogee) {
            const double bound = kStationMotion / std::pow(std::sqrt(a.b[ti].r + kEps), 3.0);
            if (tba > 0.0 && tba < bound) {
              if (std::abs(wv - ca) < kStationNear || std::abs(cb - wv) < kStationNear) {
                const auto key = std::make_pair(t, u);
                const auto seen = station_done.find(key);
                if (seen == station_done.end() || seen->second != k) {
                  station_done[key] = k;
                  touch = true;
                }
              }
            }
          }
          // the branch follows the true motion, the wrap of the circle
          // is not a retrograde arc, a deliberate correction of the
          // original's numeric ordering
          const double tb_mid = (a.b[ti].tb + b.b[ti].tb) / 2.0;
          if (!touch && tb_mid >= 0.0) {
            double w1 = ca;
            double w2 = cb;
            double w3 = wv;
            vergl2(w1, w2, w3);
            if (w3 > w1 && w2 > w3 && (w2 - w1 < kSweepSpeedCap || (t == 2 && scan.moon_aspects))) {
              hit = true;
            }
          }
          if (!touch && !hit && tb_mid < 0.0) {
            double w1 = ca;
            double w2 = cb;
            double w3 = wv;
            vergl2r(w1, w2, w3);
            if (w1 > w3 && w3 > w2 && w1 - w2 < kSweepSpeedCap) {
              hit = true;
            }
          }
          if (!hit && !touch) {
            continue;
          }
          TransitEvent e;
          e.transiting = t;
          e.radix = u;
          e.multiple = k;
          e.angle_deg = k * scan.base_angle_deg;
          if (touch) {
            e.jd_ut = jd + (jd2 - jd) / 2.0;
            e.station_touch = true;
            e.retrograde = b.b[ti].tb < 0.0;
            out.push_back(e);
            continue;
          }
          // the exact moment through the plant search from the interval
          // end, replacing the original's linear interpolation, a hit
          // that does not refine into its interval is dropped
          const LongitudeCrossing fine = find_longitude_backward(jd2, t, wv, ctx);
          if (fine.ok && fine.jd_ut >= jd - 0.000001 && fine.jd_ut <= jd2 + kEps) {
            e.jd_ut = fine.jd_ut;
            e.retrograde = fine.retrograde;
            out.push_back(e);
          }
        }
      }
    }
    a = b;
    jd = jd2;
  }
  std::sort(out.begin(), out.end(), [](const TransitEvent& x, const TransitEvent& y) { return x.jd_ut < y.jd_ut; });
  return out;
}

}  // namespace horcom
