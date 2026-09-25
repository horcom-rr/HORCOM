// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/transit_search.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>

#include "horcom/chart/composite.hpp"
#include "horcom/chart/signs.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the interactive escapes and timers of the original become step budgets
constexpr int kCoarseBudget = 20000;
constexpr int kFineBudget = 4000;
constexpr int kFinestBudget = 400;

// the width of one sign in radians, his PI / 6
constexpr double kSignRad = kPi / 6.0;

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
    case 2: return kTropicalMonthDays;
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

}  // namespace

int base_multiples(double base_angle_deg) {
  return static_cast<int>(360.1 / (base_angle_deg + kEps));
}

Chart sky_chart(double jd_ut, const SearchContext& ctx) {
  ChartInput in = ctx.base;
  in.date_ut = calendar_date(jd_ut, ctx.settings.calendar);
  return compute_chart(in, ctx.settings, *ctx.vsop, *ctx.eph);
}

// ported from plant1, the pipeline runs the same chain dat, juld1, the
// parallax branch, utet, soko with the body, etut and a316061
BodyLongitude body_longitude(double jd_ut, int slot, const SearchContext& ctx) {
  const Chart c = sky_chart(jd_ut, ctx);
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
  const double tja = sky_chart(jd_start_ut, ctx).ta.tropical_year_days;
  const double period = period_days(slot, tja);
  const bool planet = slot > 2;

  double jd = jd_start_ut;
  // the backward walk of the Sun and the Moon starts half a step after
  // the moment
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

double body_period_days(int slot, double tja) {
  return period_days(slot, tja);
}

// ported from the planetar branch of a16. The seeds place the search
// start safely past the wanted crossing, the backward walk then finds
// it. Mercury and Venus return almost yearly so their seeds count in
// years, the outer planets get a growing head start, the eccentric
// centaurs and the comet five years, the main belt asteroids a hundred
// days forward and a year backward. The moon has its own seed from the
// a16 lunar branch so a signed monthly step lands one lunation past
// the target, and its search skips the general 15 day nudge that would
// otherwise overshoot by more than one lunation.
LongitudeCrossing planetar_return(double jd_birth_ut, int slot, double radix_rad, int n, bool future, const SearchContext& ctx) {
  const double tja = sky_chart(jd_birth_ut, ctx).ta.tropical_year_days;
  const double ta = period_days(slot, tja);
  const double ns = future ? n : -n;
  double jd = jd_birth_ut;
  const bool centaur = slot == body::kChiron || slot == body::kHalley || slot == body::kPholus ||
                       slot == body::kDamokles || slot == body::kNessus;
  const bool belt = slot >= body::kCeres && slot <= body::kVesta;
  double seed_bump = 15.0;
  // jd = jd(1,ze) + ns1& * ta(2) + 0.1 * ta(2), pl& = 2, jdz = jd
  if (ctx.settings.heliocentric) {
    // the hrg! seeds, jd(1,ze) + ns1& * ta(pl&) + 0.1 * ta(pl&) for every body
    jd += ns * ta + 0.1 * ta;
  } else if (slot == body::kMoon) {
    jd += ns * ta + 0.1 * ta;
    seed_bump = 0.0;
  } else if (slot == body::kMercury) {
    jd += ns * tja + 0.5 * ta;
  } else if (slot == body::kVenus) {
    jd += ns * tja + 0.9 * ta;
  } else if (centaur) {
    jd += ns * ta + 5.0 * tja;
  } else if (belt) {
    jd += ns * ta + (future ? 100.0 : tja);
  } else {
    jd += ns * ta + (future ? tja * (slot - 4) : 0.5 * ta);
  }
  //RR jd-Startwert
  return find_longitude_backward(jd + seed_bump, slot, radix_rad, ctx);
}

// the MC and AC branches of the ingress menu, solved on the daily turn
namespace {

constexpr double kAngleClose = 5.0e-7;

}  // namespace

std::array<LongitudeCrossing, 12> angle_ingresses(double jd_start_ut, int slot, const SearchContext& ctx) {
  std::array<LongitudeCrossing, 12> out{};
  //RR Sterntag
  const double day = 1.0 / kSolarToSiderealRate;
  const auto angle_at = [&](double jd) {
    const Chart c = sky_chart(jd, ctx);
    if (!c.ok) {
      return -1.0;
    }
    return slot == body::kAscendant ? c.houses.angles.ac : c.houses.angles.mc;
  };
  for (int t = 1; t <= 12; ++t) {
    //RR vermeide 29/59/60
    const double pz = kEps + (t - 1) * kPi / 6.0;
    double jd = jd_start_ut;
    LongitudeCrossing hit;
    for (int i = 0; i < 240; ++i) {
      const double a = angle_at(jd);
      if (a < 0.0) {
        break;
      }
      double d = norm_rad(pz - a);
      if (i == 0) {
        // walk forward onto the next passage
        jd += d / kTwoPi * day;
        continue;
      }
      if (d > kPi) {
        d -= kTwoPi;
      }
      // UNTIL ABS(pz - f) < 0.0000005, his korr1 and korr2 stop there. A
      // tighter bound sits below what a julian day in double resolves,
      // the angle moves 7e-5 radians a second
      if (std::abs(d) < kAngleClose) {
        hit.ok = true;
        hit.jd_ut = jd;
        break;
      }
      // half steps keep the uneven rise rates converging
      jd += 0.5 * d / kTwoPi * day;
    }
    out[static_cast<std::size_t>(t - 1)] = hit;
  }
  return out;
}

// ported from ingre1
std::array<LongitudeCrossing, 12> sign_ingresses(double jd_start_ut, int slot, const SearchContext& ctx) {
  std::array<LongitudeCrossing, 12> out{};
  const int year0 = calendar_date(jd_start_ut, ctx.settings.calendar).year;
  for (int t = 1; t <= 12; ++t) {
    //RR vermeide 29/59/60
    const double pz = kEps + (t - 1) * kPi / 6.0;
    double jdz = jd_start_ut;
    LongitudeCrossing hit;
    for (int retry = 0; retry < 4; ++retry) {
      hit = find_longitude_backward(jdz, slot, pz, ctx);
      if (!hit.ok || slot != body::kSun) {
        break;
      }
      // the sun ingress belongs to the calendar year of the start
      const int ja = calendar_date(hit.jd_ut, ctx.settings.calendar).year;
      if (ja > year0) {
        jdz -= 365.0;
        continue;
      }
      if (ja < year0) {
        jdz += 365.0;
        continue;
      }
      break;
    }
    out[static_cast<std::size_t>(t - 1)] = hit;
  }
  return out;
}

namespace {

// the caps of a180_1, the speed limit per interval and the nearness
// window of the stationary guard
constexpr double kSweepSpeedCap = 0.26177;
constexpr double kStationNear = 0.004363;  //RR ca.0.25°

// the closing of a bracketed hit, the gates of the finest plant phase
// and a budget far above the few steps a secant needs
constexpr int kBracketBudget = 60;

// the pace class of a chosen running body in a18eing_plw, the fast
// bodies start the sweep at slot one, Mars and the four asteroids at
// five, the slow ones at six
int pace_class(int slot, const ChartSettings& s) {
  if (slot <= body::kVenus) {
    return 1;
  }
  if (slot == body::kMars || (slot >= body::kCeres && slot <= body::kVesta)) {
    return 5;
  }
  if ((slot == body::kNodeAsc || slot == body::kNodeDesc) && s.true_node) {
    return 1;
  }
  if (slot == body::kApogee && s.true_apogee) {
    return 1;
  }
  return 6;
}

// the interval table of a180 keyed by his pl1, the base angle decides
// for the moon and the swift points, the true black moon halves it. The
// pace classes of the port are 1, 5 and 6 only
double sweep_interval(int pace, const TransitScan& scan, const ChartSettings& s) {
  double ival = 1.0;
  switch (pace) {
    case 1:
    case 5: ival = 2.0; break;
    case 6: ival = 6.0; break;
    default: break;
  }
  // mas! OR ((apogw! OR moknw! OR n4&) && plan_wahl! = 0), a180ival
  const bool fortune = s.extra_bodies && s.nk[4] > 0;
  if (scan.moon_aspects || ((s.true_apogee || s.true_node || fortune) && scan.chosen.empty())) {
    ival = scan.base_angle_deg < 30.0 ? 1.0 : 2.0;
  }
  // apogw, the true black moon runs
  if (s.true_apogee && s.extra_bodies && s.nk[1] > 0) {
    ival = 0.5;
  }
  return ival;
}

// one point the running bodies can reach, his u, with v for a midpoint
struct Target {
  int slot = 0;
  int partner = 0;
  int cusp = 0;
  double lon = 0.0;
};

// the radix points of the a180 loops. Bodies with AC and MC, the south
// node out, then the cardinal points, the intermediate cusps and the
// midpoints of halbs_dir when they are asked for
std::vector<Target> transit_targets(const Chart& radix, const TransitScan& scan, const ChartSettings& s) {
  std::vector<Target> out;
  const bool helio = s.heliocentric;
  const auto usable = [&](int u) {
    const BodyState& b = radix.b[static_cast<std::size_t>(u)];
    return b.present && b.valid;
  };
  // hrg keeps the sun, the black moon, the part of fortune, the nodes
  // and the axes out
  const auto helio_out = [&](int u) {
    return helio && (u == body::kSun || u == body::kApogee || u == body::kFortune || (u > 10 && u < 19));
  };
  for (int u = 1; u < body::kSlotCount; ++u) {
    if (u == body::kNodeDesc || body::cardinal(u) || helio_out(u) || !usable(u)) {
      continue;
    }
    out.push_back({u, 0, 0, norm_rad(radix.b[static_cast<std::size_t>(u)].el)});
  }
  if (!helio && scan.cardinal_targets) {
    // wv = po * (u& - 15)
    for (int u = body::kAriesPoint; u <= body::kCapricornPoint; ++u) {
      out.push_back({u, 0, 0, norm_rad((u - body::kAriesPoint) * kHalfPi + scan.cardinal_shift)});
    }
  }
  if (!helio && scan.house_targets && radix.houses.ok && s.houses < HouseSystem::kAcMcOnly) {
    // primu&(u&) = u& - 13 and u& - 12, the houses 2, 3, 5 and 6
    for (const int h : {2, 3, 5, 6}) {
      out.push_back({0, 0, h, norm_rad(radix.houses.cusp[static_cast<std::size_t>(h)])});
    }
  }
  if (scan.midpoints > 0) {
    // the factors of the midpoint pass, no south node, no cardinal slots
    std::vector<int> factors;
    for (int u = 1; u < body::kSlotCount; ++u) {
      if (u == body::kNodeDesc || body::cardinal(u) || !usable(u)) {
        continue;
      }
      if (helio && (u == body::kSun || u == body::kApogee || u == body::kFortune || (u > 10 && u < 19))) {
        continue;
      }
      factors.push_back(u);
    }
    for (std::size_t i = 0; i < factors.size(); ++i) {
      for (std::size_t j = i + 1; j < factors.size(); ++j) {
        const int u = factors[i];
        const int v = factors[j];
        out.push_back({u, v, 0,
                       midpoint_near(radix.b[static_cast<std::size_t>(u)].el, radix.b[static_cast<std::size_t>(v)].el)});
      }
    }
  }
  return out;
}

// the running bodies the stationary guard of a180_1 watches, his outer
// and inner tests together. The lights, the nodes, the axes, the
// cardinal slots, the black moon and the part of fortune stay out
bool station_watched(int t) {
  return t > body::kMoon && !(t >= body::kNodeAsc && t <= body::kApogee) && t != body::kFortune;
}

// ported from a180aus, the quadratic through the body at the start of
// the previous interval y, at the start w1 and at the end w2 of this one.
// The answer is the fraction of the interval where the curve reaches w3,
// minus one where the root is undefined
double a180aus_fraction(double y, double w1, double w2, double w3) {
  if (w1 > y + kPi && y > 0.0) {
    y += kTwoPi;
  }
  if (y > w1 + kPi) {
    y -= kTwoPi;
  }
  const double a = w1 - y;
  const double b = w2 - w1;
  const double c = b - a;
  const double e = w3 - w1;
  if (a + b == 0.0) {
    return -1.0;
  }
  const double q = 1.0 + 8.0 * e * c / ((a + b) * (a + b));
  if (q < 0.0) {
    return -1.0;
  }
  const double f = std::sqrt(q);
  return (-1.0 + f) * (a + b) / 2.0 / (c + kEps);
}

// closes on the crossing inside one interval whose ends lie on either
// side of the target, fa and fb the signed differences there. A secant
// that halves a stale end, the Illinois rule, starts from the seed and
// never leaves the bracket, so a retrograde passage closes like a direct
// one
LongitudeCrossing close_in_bracket(double ja, double fa, double jb, double fb, double seed, int slot, double target,
                                   const SearchContext& ctx) {
  LongitudeCrossing out;
  if ((fa < 0.0) == (fb < 0.0)) {
    return out;
  }
  double x = seed;
  int side = 0;
  for (int i = 0; i < kBracketBudget; ++i) {
    if (!(x > ja && x < jb)) {
      x = 0.5 * (ja + jb);
    }
    const BodyLongitude e = body_longitude(x, slot, ctx);
    if (!e.valid) {
      return out;
    }
    const double fx = fold_rad(e.el - target);
    if (std::abs(fx) < kAngleGate || jb - ja < kStepGate) {
      out.ok = true;
      out.jd_ut = x;
      out.retrograde = e.tb < 0.0;
      return out;
    }
    if ((fx < 0.0) == (fb < 0.0)) {
      jb = x;
      fb = fx;
      if (side < 0) {
        fa *= 0.5;
      }
      side = -1;
    } else {
      ja = x;
      fa = fx;
      if (side > 0) {
        fb *= 0.5;
      }
      side = 1;
    }
    x = ja - fa * (jb - ja) / (fb - fa);
  }
  return out;
}

}  // namespace

// ported from a180 with a180_1, the interval sweep over the window
std::vector<TransitEvent> scan_transits(const Chart& radix, const TransitScan& scan, const SearchContext& ctx) {
  std::vector<TransitEvent> out;
  if (!(scan.jd_to_ut > scan.jd_from_ut) || scan.base_angle_deg <= 0.0) {
    return out;
  }
  const ChartSettings& s = ctx.settings;
  const bool helio = s.heliocentric;
  const bool moon_runs = scan.moon_aspects ||
                         std::find(scan.chosen.begin(), scan.chosen.end(), body::kMoon) != scan.chosen.end();

  // his pl1, the chosen bodies set it from their pace classes
  int pace = std::max(1, scan.first_slot);
  if (!scan.chosen.empty()) {
    pace = 6;
    for (const int c : scan.chosen) {
      pace = std::min(pace, pace_class(c, s));
    }
  }

  // the running bodies after the a18st filters, the south node, the
  // axes and the part of fortune never transit, the moon only on MOND
  // BERÜCKSICHTIGEN like a18st0
  std::vector<int> transiting;
  for (int t = scan.chosen.empty() ? pace : 1; t < body::kSlotCount; ++t) {
    if (t == body::kNodeDesc || t == body::kAscendant || t == body::kMc || t == body::kFortune) {
      continue;
    }
    if (scan.only_slot > 0 && t != scan.only_slot) {
      continue;
    }
    if (t == body::kMoon && !helio && !moon_runs) {
      continue;
    }
    if (t == body::kApogee && scan.drop_true_apogee) {
      continue;
    }
    if (helio && (t == body::kSun || t == body::kApogee || (t > 10 && t < 19))) {
      continue;
    }
    if (!scan.chosen.empty() && std::find(scan.chosen.begin(), scan.chosen.end(), t) == scan.chosen.end()) {
      continue;
    }
    const BodyState& b = radix.b[static_cast<std::size_t>(t)];
    if (b.present) {
      transiting.push_back(t);
    }
  }
  const std::vector<Target> targets = transit_targets(radix, scan, s);

  const double base = scan.base_angle_deg * kDegToRad;
  const int multiples = base_multiples(scan.base_angle_deg);
  const double ival = scan.step_days > 0.0 ? scan.step_days : sweep_interval(pace, scan, s);

  // the c3 memory of the original, one stationary touch per running
  // body and target for the whole run
  std::map<std::pair<int, std::size_t>, int> station_done;
  // his cc, every running body at the start of the interval before
  std::array<double, body::kSlotCount> before{};
  bool have_before = false;

  Chart a = sky_chart(scan.jd_from_ut, ctx);
  double jd = scan.jd_from_ut;
  while (jd < scan.jd_to_ut && a.ok) {
    const double jd2 = std::min(jd + ival, scan.jd_to_ut);
    const double step = jd2 - jd;
    const Chart b = sky_chart(jd2, ctx);
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
      const double tb_mid = (a.b[ti].tb + b.b[ti].tb) / 2.0;
      const bool watched = scan.station_touches && !helio && station_watched(t);
      const double station_bound = kStationDailyMotion / std::pow(std::sqrt(b.b[ti].r + kEps), 3.0);
      for (std::size_t ix = 0; ix < targets.size(); ++ix) {
        const Target& target = targets[ix];
        // NUR DIE MIT 3 UNTERSCHIEDLICHEN Faktoren ANZEIGEN
        if (target.partner > 0 && scan.midpoints == 2 && (t == target.slot || t == target.partner)) {
          continue;
        }
        for (int k = 0; k < multiples; ++k) {
          if (grid_skips(scan.grid, k)) {
            continue;
          }
          const double wv = norm_rad(target.lon + k * base);
          // the stationary guard of a180_1, a slow body touching the
          // target inside the interval, remembered once per pair. His
          // bound holds the daily motion of the reconciled pair
          bool touch = false;
          if (watched) {
            double w1 = ca;
            double w2 = cb;
            double w3 = wv;
            vergl2(w1, w2, w3);
            const double tba = std::abs((w2 - w1) / step);
            if (tba > 0.0 && tba < station_bound &&
                (std::abs(w3 - w1) < kStationNear || std::abs(w2 - w3) < kStationNear)) {
              const auto key = std::make_pair(t, ix);
              const auto seen = station_done.find(key);
              if (seen == station_done.end() || seen->second != k) {
                station_done[key] = k;
                touch = true;
              }
            }
          }
          // the branch follows the true motion. His vergl2 order sent a
          // retrograde step over 0 Aries into the direct test, where the
          // speed cap dropped it, the mean motion of the interval does not
          bool hit = false;
          double w1 = ca;
          double w2 = cb;
          double w3 = wv;
          if (!touch && tb_mid >= 0.0) {
            vergl2(w1, w2, w3);
            hit = w3 > w1 && w2 > w3 && (w2 - w1 < kSweepSpeedCap || (t == body::kMoon && moon_runs));
          } else if (!touch) {
            vergl2r(w1, w2, w3);
            hit = w1 > w3 && w3 > w2 && w1 - w2 < kSweepSpeedCap;
          }
          if (!hit && !touch) {
            continue;
          }
          TransitEvent e;
          e.transiting = t;
          e.radix = target.slot;
          e.radix2 = target.partner;
          e.cusp = target.cusp;
          e.multiple = k;
          e.angle_deg = k * scan.base_angle_deg;
          e.speed = tb_mid;
          if (touch) {
            e.jd_ut = jd + step / 2.0;
            e.station_touch = true;
            e.retrograde = b.b[ti].tb < 0.0;
            out.push_back(e);
            continue;
          }
          // his a180aus seeds the moment, a full interval behind gives
          // the quadratic, the first or a shortened one the straight line.
          // The secant then closes on the exact crossing inside the
          // interval in either direction
          double dw = -1.0;
          if (have_before && step == ival) {
            dw = a180aus_fraction(before[ti], w1, w2, w3);
          }
          if (!(dw > 0.0 && dw < 1.0)) {
            dw = (w3 - w1) / (w2 - w1);
          }
          const LongitudeCrossing fine =
              close_in_bracket(jd, fold_rad(ca - wv), jd2, fold_rad(cb - wv), jd + dw * step, t, wv, ctx);
          if (fine.ok) {
            e.jd_ut = fine.jd_ut;
            e.retrograde = fine.retrograde;
            out.push_back(e);
          }
        }
      }
    }
    for (const int t : transiting) {
      before[static_cast<std::size_t>(t)] = norm_rad(a.b[static_cast<std::size_t>(t)].el);
    }
    have_before = step == ival;
    a = b;
    jd = jd2;
    if (scan.progress && !scan.progress((jd - scan.jd_from_ut) / (scan.jd_to_ut - scan.jd_from_ut))) {
      break;
    }
  }
  std::sort(out.begin(), out.end(), [](const TransitEvent& x, const TransitEvent& y) { return x.jd_ut < y.jd_ut; });
  return out;
}

// ported from planth, his zr walk back over the passages of one return,
// each passage bracketed on the near side difference and bisected
std::vector<LongitudeCrossing> planetar_earlier(const LongitudeCrossing& found, int slot, double target_rad, const SearchContext& ctx) {
  std::vector<LongitudeCrossing> out;
  if (!found.ok) {
    return out;
  }
  // the second, retrograde point up to the ninth, zr& up to 2 for ME..SA
  // and the belt, up to 8 for UR..PL and the far bodies
  const bool short_walk = (slot >= body::kMercury && slot <= body::kSaturn) || group_asteroid(slot);
  const std::size_t most = short_walk ? 2 : 8;
  // his djd, the interval a further point must turn up in
  const StepTable st = step_table(slot);
  const double window = st.djd > 0.0 ? st.djd : 320.0;
  const double tja = sky_chart(found.jd_ut, ctx).ta.tropical_year_days;
  const double period = body_period_days(slot, tja);
  const double step = std::clamp(period / 2000.0, 0.2, 4.0);
  const auto diff = [&](double jd) {
    const BodyLongitude b = body_longitude(jd, slot, ctx);
    if (!b.valid) {
      return 1.0e9;
    }
    return fold_rad(b.el - target_rad);
  };
  double anchor = found.jd_ut;
  double t = found.jd_ut - step;
  double prev = diff(t);
  while (t > anchor - window && out.size() < most) {
    const double next_t = t - step;
    const double cur = diff(next_t);
    if (prev < 1.0e8 && cur < 1.0e8 && ((prev < 0.0) != (cur < 0.0)) && std::abs(prev) < 1.0 && std::abs(cur) < 1.0) {
      double a = next_t;
      double b = t;
      double fa = cur;
      for (int i = 0; i < 40; ++i) {
        const double m = 0.5 * (a + b);
        const double fm = diff(m);
        if ((fa < 0.0) == (fm < 0.0)) {
          a = m;
          fa = fm;
        } else {
          b = m;
        }
      }
      LongitudeCrossing hit;
      hit.ok = true;
      hit.jd_ut = 0.5 * (a + b);
      const BodyLongitude bl = body_longitude(hit.jd_ut, slot, ctx);
      hit.retrograde = bl.valid && bl.tb < 0.0;
      if (anchor - hit.jd_ut > 0.5) {
        out.push_back(hit);
        anchor = hit.jd_ut;
      }
    }
    t = next_t;
    prev = cur;
  }
  return out;
}

// the counting periods, the mean motions of Meeus Table 31.A for Jupiter
// to Neptune and 90560 days for Pluto
double planetar_count_period(int slot, double tja, bool helio) {
  if (!helio && (slot == body::kMercury || slot == body::kVenus)) {
    return tja;
  }
  switch (slot) {
    case body::kJupiter: return 4332.6;
    case body::kSaturn: return 10759.2;
    case body::kUranus: return 30688.3;
    case body::kNeptune: return 60182.3;
    case body::kPluto: return 90560.0;
    default: return body_period_days(slot, tja);
  }
}

// ported from a18_3045
bool grid_skips(AspectGrid grid, int w) {
  switch (grid) {
    case AspectGrid::k30And45:
      switch (w) {
        case 1: case 5: case 7: case 11: case 13: case 17: case 19: case 23:  //RR *15
          return true;
        default:
          return false;
      }
    case AspectGrid::k60And45:
      switch (w) {
        case 1: case 2: case 5: case 7: case 10: case 11: case 13: case 14: case 17: case 19: case 22: case 23:  //RR *15
          return true;
        default:
          return false;
      }
    case AspectGrid::k60And90:
      switch (w) {
        case 1: case 5: case 7: case 11:  //RR *30
          return true;
        default:
          return false;
      }
    case AspectGrid::kPlain:
      break;
  }
  return false;
}

namespace {

// the sign of a longitude, 0 Aries to 11 Pisces
int sign_of(double lon) {
  return static_cast<int>(norm_rad(lon) / kSignRad) % kSignCount;
}

}  // namespace

// ported from mund1, the table branch. Three samples per window, the
// windows share their ends like his ca, cb and cc
std::vector<MundaneAspect> scan_mundane_aspects(const MundaneScan& scan, const SearchContext& ctx) {
  std::vector<MundaneAspect> out;
  if (!(scan.jd_to_ut > scan.jd_from_ut) || scan.base_angle_deg <= 0.0) {
    return out;
  }
  const ChartSettings& s = ctx.settings;
  const bool helio = s.heliocentric;
  const bool within_sign = scan.within_sign && !helio;
  // his flg! test ends on the node pair and keeps the one day step, the
  // moon and the true node or apogee halve it
  double ival = 1.0;
  if (scan.moon || s.true_apogee || s.true_node) {
    ival = 0.5;
  }

  // the running bodies after the filters of his u and t loops. The south
  // node, the axes and the slots up to 18 never run, nor the part of
  // fortune, the moon only on MOND BERÜCKSICHTIGEN
  std::vector<int> running;
  for (int p = std::max(1, scan.first_slot); p < body::kSlotCount; ++p) {
    if (p == body::kNodeDesc || (p >= body::kAscendant && p < body::kApogee) || p == body::kFortune) {
      continue;
    }
    if (p == body::kMoon && !helio && !scan.moon) {
      continue;
    }
    if (p == body::kApogee && scan.drop_true_apogee) {
      continue;
    }
    if (helio && (p == body::kSun || p == body::kApogee || p == body::kNodeAsc)) {
      continue;
    }
    if (!scan.chosen.empty() && std::find(scan.chosen.begin(), scan.chosen.end(), p) == scan.chosen.end()) {
      continue;
    }
    running.push_back(p);
  }

  using Row = std::array<double, body::kSlotCount>;
  using Valid = std::array<bool, body::kSlotCount>;
  const auto sample = [&](double jd, Row& lon, Valid& ok) {
    const Chart c = sky_chart(jd, ctx);
    for (const int p : running) {
      const BodyState& b = c.b[static_cast<std::size_t>(p)];
      ok[static_cast<std::size_t>(p)] = c.ok && b.present && b.valid;
      lon[static_cast<std::size_t>(p)] = norm_rad(b.el);
    }
    return c.ok;
  };

  const double base = scan.base_angle_deg * kDegToRad;
  const int multiples = base_multiples(scan.base_angle_deg);
  double jd = within_sign && scan.moon ? scan.jd_from_ut : scan.jd_from_ut - ival;
  Row la{};
  Row lb{};
  Row lc{};
  Valid oka{};
  Valid okb{};
  Valid okc{};
  if (!sample(jd, la, oka)) {
    return out;
  }
  // his ze& signs at the start and the marks of a body that left its sign
  std::array<int, body::kSlotCount> start_sign{};
  std::array<bool, body::kSlotCount> left{};
  for (const int p : running) {
    start_sign[static_cast<std::size_t>(p)] = sign_of(la[static_cast<std::size_t>(p)]);
  }

  while (jd < scan.jd_to_ut) {
    if (!sample(jd + ival, lb, okb) || !sample(jd + 2.0 * ival, lc, okc)) {
      break;
    }
    for (std::size_t i = 0; i + 1 < running.size(); ++i) {
      const auto u = static_cast<std::size_t>(running[i]);
      if (!oka[u] || !okb[u] || !okc[u]) {
        continue;
      }
      for (std::size_t j = i + 1; j < running.size(); ++j) {
        const auto t = static_cast<std::size_t>(running[j]);
        if (!oka[t] || !okb[t] || !okc[t]) {
          continue;
        }
        for (int w = 0; w < multiples; ++w) {
          if (grid_skips(scan.grid, w)) {
            continue;
          }
          // his w11 to w23 as one difference. Folded it keeps its sign
          // change when the pair crosses zero Aries, where his separate
          // unwrapping of both bodies lost the event
          const double off = w * base;
          const double da = fold_rad(la[u] + off - la[t]);
          const double db = fold_rad(lb[u] + off - lb[t]);
          const double dc = fold_rad(lc[u] + off - lc[t]);
          // a jump over the far side is the opposite point, no crossing
          if (std::abs(db - da) > kHalfPi || std::abs(dc - db) > kHalfPi) {
            continue;
          }
          if (!((da < 0.0 && dc > 0.0) || (da > 0.0 && dc < 0.0))) {
            continue;
          }
          // his quadratic through the three differences, the root inside
          // the window measured from the middle sample
          const double s1 = dc - da;
          const double cf = dc - 2.0 * db + da;
          const double yz = db;
          double dw = 0.0;
          bool solved = false;
          if (cf != 0.0) {
            const double sa = -8.0 * cf * yz + s1 * s1;
            if (sa >= 0.0) {
              const double nn1 = 0.5 * (-s1 + std::sqrt(sa)) / cf;
              const double nn2 = 0.5 * (-s1 - std::sqrt(sa)) / cf;
              if (std::abs(nn1) > 0.0 && std::abs(nn1) < 1.0) {
                dw = nn1;
                solved = true;
              } else if (std::abs(nn2) > 0.0 && std::abs(nn2) < 1.0) {
                dw = nn2;
                solved = true;
              }
            }
          }
          if (!solved) {
            // a straight difference or a root on the middle sample, where
            // his solve divided by zero or kept the fraction of the last hit
            dw = std::clamp(-2.0 * yz / s1, -1.0, 1.0);
          }
          const double du = dw > 0.0 ? fold_rad(lc[u] - lb[u]) : fold_rad(lb[u] - la[u]);
          const double dt = dw > 0.0 ? fold_rad(lc[t] - lb[t]) : fold_rad(lb[t] - la[t]);
          MundaneAspect m;
          m.jd_ut = jd + ival + dw * ival;
          m.first = running[i];
          m.second = running[j];
          m.multiple = w;
          m.angle_deg = w * scan.base_angle_deg;
          if (m.angle_deg > 180.0) {
            m.angle_deg = 360.0 - m.angle_deg;
          }
          m.first_lon = norm_rad(lb[u] + dw * du);
          m.second_lon = norm_rad(lb[t] + dw * dt);
          if (within_sign) {
            // his check read the signs at the window end and so dropped
            // aspects made shortly before a body left its sign, the port
            // reads them at the moment of the aspect
            if (left[u] || left[t] || sign_of(m.first_lon) != start_sign[u] ||
                sign_of(m.second_lon) != start_sign[t]) {
              continue;
            }
          }
          if (m.jd_ut >= scan.jd_from_ut && m.jd_ut <= scan.jd_to_ut) {
            out.push_back(m);
          }
        }
      }
    }
    if (within_sign) {
      for (const int p : running) {
        const auto pi = static_cast<std::size_t>(p);
        left[pi] = left[pi] || sign_of(lb[pi]) != start_sign[pi] || sign_of(lc[pi]) != start_sign[pi];
      }
    }
    jd += 2.0 * ival;
    la = lc;
    oka = okc;
    if (scan.progress && !scan.progress((jd - scan.jd_from_ut) / (scan.jd_to_ut - scan.jd_from_ut))) {
      break;
    }
  }
  std::sort(out.begin(), out.end(), [](const MundaneAspect& x, const MundaneAspect& y) { return x.jd_ut < y.jd_ut; });
  return out;
}

}  // namespace horcom
