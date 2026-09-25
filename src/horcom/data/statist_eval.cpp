// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/statist_eval.hpp"

#include <algorithm>
#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/composite.hpp"
#include "horcom/chart/signs.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/statist_list.hpp"

namespace horcom {

namespace {

// the six signs of half a circle, his PI / 6 and 6 * w / PI
constexpr double kSignsPerPi = 6.0;
// the twelve houses of haus_def
constexpr int kHouseCount = 12;

}  // namespace

// ported from HORCOM haus_def
std::array<double, 14> stat_houses(const StatRecord& r) {
  std::array<double, 14> f{};
  f[1] = r.ac;
  f[2] = r.h2;
  f[3] = r.h3;
  f[4] = norm_rad(kPi + r.mc);
  f[5] = r.h5;
  f[6] = r.h6;
  f[7] = norm_rad(kPi + r.ac);
  f[8] = norm_rad(kPi + r.h2);
  f[9] = norm_rad(kPi + r.h3);
  f[10] = r.mc;
  f[11] = norm_rad(kPi + r.h5);
  f[12] = norm_rad(kPi + r.h6);
  f[13] = f[1];
  return f;
}

// ported from HORCOM ze_pl
int sign_ruler(double w, bool classic) {
  if (w <= 0.0 || w >= kTwoPi) {
    return 0;
  }
  //RR ZEICHEN
  const int jp = static_cast<int>(std::floor(kSignsPerPi * w / kPi)) + 1;
  switch (jp) {
    case 1: return body::kMars;
    case 2:
    case 7: return body::kVenus;
    case 3:
    case 6: return body::kMercury;
    case 4: return body::kMoon;
    case 5: return body::kSun;
    case 8: return classic ? body::kMars : body::kPluto;
    case 9: return body::kJupiter;
    case 10: return body::kSaturn;
    case 11: return classic ? body::kSaturn : body::kUranus;
    case 12: return classic ? body::kJupiter : body::kNeptune;
    default: return 0;
  }
}

// ported from HORCOM geb_herr
std::pair<int, int> birth_rulers(double cusp1, double cusp2, bool classic) {
  std::pair<int, int> k{0, 0};
  if (cusp1 == 0.0 || cusp2 == 0.0) {
    return k;
  }
  k.first = sign_ruler(norm_rad(cusp1), classic);
  const int j1 = static_cast<int>(std::floor(kSignsPerPi * norm_rad(cusp1) / kPi)) + 1;
  int j2 = static_cast<int>(std::floor(kSignsPerPi * norm_rad(cusp2) / kPi)) + 1;
  if (j2 < j1) {
    j2 += kSignCount;
  }
  // a whole sign intercepted in the first house makes its ruler a
  // second birth ruler, the sign sits 30 degrees before the second cusp
  if (j2 - j1 == 2) {
    k.second = sign_ruler(norm_rad(cusp2 - kPi / kSignsPerPi), classic);
  }
  return k;
}

namespace {

// one longitude of a record, AC and MC live beside the slot array
double slot_position(const StatRecord& r, int slot) {
  if (slot == body::kAscendant) {
    return r.ac;
  }
  if (slot == body::kMc) {
    return r.mc;
  }
  return r.el[static_cast<std::size_t>(slot)];
}

// the original stat_auswh, one operand to a longitude, slot reports the
// body that stood behind it, zero for a cusp
double operand_value(const StatRecord& r, const StatOperand& op, bool classic, int& slot) {
  switch (op.kind) {
    case StatOperand::Kind::kCusp:
      slot = 0;
      return stat_houses(r)[static_cast<std::size_t>(op.house)];
    case StatOperand::Kind::kRuler: {
      const double w = stat_houses(r)[static_cast<std::size_t>(op.house)];
      slot = sign_ruler(w, classic);
      return slot > 0 ? r.el[static_cast<std::size_t>(slot)] : 0.0;
    }
    default:
      slot = op.body;
      return slot_position(r, op.body);
  }
}

// the orb fraction of an operand, org over the body weight, a cusp or
// ruler operand counts neutral, the original read past the planet table
// there
double operand_orb(const AspectSettings& a, const StatOperand& op) {
  return op.kind == StatOperand::Kind::kBody ? org(a, op.body, 1) : 1.0;
}

struct Eval {
  const StatSet& set;
  const StatQuery& q;
  const AspectSettings& a;
  std::vector<double>& mask;
  StatEvalResult res;
  // the cusps of the record under test, the f array of the original
  std::array<double, 14> f{};

  // ported from sum_z_h, the distribution of every tested value
  void tally(double w3) {
    if (q.object == StatObject::kAspect || q.object == StatObject::kMidpointAspect || q.object == StatObject::kName) {
      return;  // suc stands at zero there
    }
    if (q.window == StatWindow::kInHouse) {
      //RR HÄUSER-VERT
      for (int j = 1; j <= kHouseCount; ++j) {
        const double w1 = f[static_cast<std::size_t>(j)];
        double w2 = f[static_cast<std::size_t>(j) + 1];
        double w = w3;
        if (w2 + kPi < w1) {
          w2 += kTwoPi;
        }
        if (w + kPi < w1 && w + kTwoPi < w2) {
          w += kTwoPi;
        }
        if (w > w1 && w < w2) {
          ++res.distribution[static_cast<std::size_t>(j)];
          ++res.distribution[0];
        }
      }
      return;
    }
    //RR ZEICHEN-VERT
    for (int j = 1; j <= kSignCount; ++j) {
      double w1 = (j - 1) * kPi / kSignsPerPi + kEps;
      double w2 = j * kPi / kSignsPerPi;
      double w = w3;
      vergl2(w1, w2, w);
      if (w > w1 && w < w2) {
        ++res.distribution[static_cast<std::size_t>(j)];
        ++res.distribution[0];
      }
    }
  }

  // the original bed_erf_2 plus bed_erf_1's list keeping
  void take(int i, int slot, double w3) {
    res.matches.push_back({i, slot, norm_rad(w3)});
    mask[static_cast<std::size_t>(i)] = norm_rad(w3);
  }

  // the original bed_erf
  void bed(int i, int slot, double w1, double w2, double w3) {
    tally(w3);
    vergl2(w1, w2, w3);
    if (q.window == StatWindow::kAnywhere) {
      //RR 0...360
      if (w1 < w3 && w3 < w2 && w3 > 0.0) {
        take(i, slot, w3);
      }
      return;
    }
    if (q.combine_and) {
      if (w1 < w3 && w3 < w2 && w1 > 0.0 && mask[static_cast<std::size_t>(i)] > 0.0) {
        take(i, slot, w3);
      } else {
        mask[static_cast<std::size_t>(i)] = 0.0;
      }
    } else if (w1 < w3 && w3 < w2 && w1 > 0.0) {
      take(i, slot, w3);
    }
  }

  // the original bed_erf_asp1 and bed_erf_asp2 for one divisor, the
  // number of its multiples the separation holds
  [[nodiscard]] int asp_hits(double dds, double pn, int n, double w3) const {
    if (n == 1) {
      //RR Konj
      double w1 = 0.0;
      double w2 = dds;
      vergl2(w1, w2, w3);
      if (std::abs(w3 - kTwoPi) < dds) {
        w3 = std::abs(kTwoPi - w3);
      }
      return w3 > 0.0 && w3 < dds ? 1 : 0;
    }
    int hits = 0;
    for (int m = 1; m <= n - 1; ++m) {
      if (a.equal_probability && !multiple_allowed(n, m)) {
        continue;
      }
      const double w1 = norm_rad(m * pn - dds);
      const double w2 = norm_rad(m * pn + dds);
      if (w1 < w3 && w3 < w2) {
        ++hits;
      }
    }
    return hits;
  }

  // the original bed_erf_asp over one folded separation. Under UND his
  // bed_erf_asp cleared a record only after an earlier hit of the same
  // multiple in it, a record without any aspect stayed and a conjunction
  // was wiped by the next divisor's miss. KOMMSTAT says UND keeps the
  // records every earlier condition holds for, the port keeps a record
  // when any divisor hits and clears it otherwise
  void bed_asp_all(int i, double w3, double o1, double o2, double o3, bool three) {
    tally(w3);
    int hits = 0;
    const bool single = q.asp_low == q.asp_high;
    const int high = std::clamp(q.asp_high, 1, single ? kStatMaxSingleDivisor : kStatMaxRangeDivisor);
    const int low = std::clamp(q.asp_low, 1, high);
    //RR EINZEL-ASPEKT MIT VORGEG. ORBIS
    if (q.asp_orb > 0.0 && single) {
      const int n = high;
      const double pn = kTwoPi / n;
      const double dds = three ? orbis_discr3(o1, o2, o3, q.asp_orb) : orbis_discr2(o1, o2, q.asp_orb);
      hits = asp_hits(dds, pn, n, w3);
    } else {
      for (int n = low; n <= high; ++n) {
        const double pn = kTwoPi / n;
        const double dd =
            a.equal_probability ? a.orb * a.orbe[static_cast<std::size_t>(n)] : a.orb * pn / kDefaultOrbDivisor;
        const double dds = three ? orbis_discr3(o1, o2, o3, dd) : orbis_discr2(o1, o2, dd);
        hits += asp_hits(dds, pn, n, w3);
      }
    }
    if (q.combine_and && (hits == 0 || mask[static_cast<std::size_t>(i)] <= 0.0)) {
      mask[static_cast<std::size_t>(i)] = 0.0;
      return;
    }
    for (int k = 0; k < hits; ++k) {
      take(i, 0, w3);
    }
  }

  // SO/MO/AC and ALLE PLANETEN, KOMMSTAT reads the group as an OR over
  // its bodies. His bed_erf cleared the record on any body's miss and kept
  // the mask at the list index, under UND the group acted as an AND
  void bed_group(int i, const std::vector<std::pair<int, double>>& bodies, double w1, double w2) {
    std::vector<std::pair<int, double>> hits;
    for (const auto& [slot, position] : bodies) {
      tally(position);
      double lo = w1;
      double hi = w2;
      double w3 = position;
      vergl2(lo, hi, w3);
      const bool in = lo < w3 && w3 < hi && (q.window == StatWindow::kAnywhere ? w3 > 0.0 : lo > 0.0);
      if (in) {
        hits.emplace_back(slot, w3);
      }
    }
    if (q.combine_and && q.window != StatWindow::kAnywhere &&
        (hits.empty() || mask[static_cast<std::size_t>(i)] <= 0.0)) {
      mask[static_cast<std::size_t>(i)] = 0.0;
      return;
    }
    for (const auto& [slot, w3] : hits) {
      take(i, slot, w3);
    }
  }

  // the original stat_ausw_1, one record against one window pass
  void test(int i, double w1, double w2) {
    const StatRecord& r = set.records[static_cast<std::size_t>(i)];
    const bool helio = r.heliocentric();
    int slot = 0;
    switch (q.object) {
      case StatObject::kBody: {
        const double w3 = operand_value(r, q.a, q.classic_rulers, slot);
        bed(i, slot, w1, w2, w3);
        break;
      }
      case StatObject::kLights: {
        std::vector<std::pair<int, double>> group;
        if (!helio) {
          group.emplace_back(body::kSun, r.el[body::kSun]);
        }
        group.emplace_back(body::kMoon, r.el[body::kMoon]);
        group.emplace_back(body::kAscendant, r.ac);
        bed_group(i, group, w1, w2);
        break;
      }
      case StatObject::kAllBodies: {
        std::vector<std::pair<int, double>> group;
        for (int k = body::kSun; k <= body::kMc; ++k) {
          group.emplace_back(k, slot_position(r, k));
        }
        for (int e = 1; e <= kStatExtraCount; ++e) {
          if (set.params.nk[static_cast<std::size_t>(e)] > 0) {
            const int extra = extra_slot(e);
            group.emplace_back(extra, r.el[static_cast<std::size_t>(extra)]);
          }
        }
        bed_group(i, group, w1, w2);
        break;
      }
      case StatObject::kHouseRuler: {
        if (helio) {
          break;
        }
        const double w = stat_houses(r)[static_cast<std::size_t>(q.a.house)];
        const int kp = sign_ruler(w, q.classic_rulers);
        //RR Nicht selbst
        if (kp > 0 && !(q.window == StatWindow::kNearBody && q.near_body.kind == StatOperand::Kind::kBody && kp == q.near_body.body)) {
          bed(i, kp, w1, w2, r.el[static_cast<std::size_t>(kp)]);
        }
        break;
      }
      case StatObject::kMidpoint: {
        const double p1 = operand_value(r, q.a, q.classic_rulers, slot);
        const double p2 = operand_value(r, q.b, q.classic_rulers, slot);
        bed(i, 0, w1, w2, midpoint_near(p1, p2));
        break;
      }
      case StatObject::kAspect: {
        const double p1 = operand_value(r, q.a, q.classic_rulers, slot);
        const double p2 = operand_value(r, q.b, q.classic_rulers, slot);
        double w3 = norm_rad(p1 - p2);
        if (w3 > kPi) {
          w3 = kTwoPi - w3;
        }
        bed_asp_all(i, w3, operand_orb(a, q.a), operand_orb(a, q.b), 1.0, false);
        break;
      }
      case StatObject::kMidpointAspect: {
        const double p1 = operand_value(r, q.a, q.classic_rulers, slot);
        const double p2 = operand_value(r, q.b, q.classic_rulers, slot);
        const double p3 = operand_value(r, q.c, q.classic_rulers, slot);
        double w3 = norm_rad(p1 - midpoint_near(p2, p3));
        if (w3 > kPi) {
          w3 = kTwoPi - w3;
        }
        bed_asp_all(i, w3, operand_orb(a, q.a), operand_orb(a, q.b), operand_orb(a, q.c), true);
        break;
      }
      case StatObject::kMirror: {
        const double w = operand_value(r, q.a, q.classic_rulers, slot);
        if (q.mirror == MirrorAxis::kAriesLibra || q.mirror == MirrorAxis::kBoth) {
          bed(i, 0, w1, w2, norm_rad(kTwoPi - w));
        }
        if (q.mirror == MirrorAxis::kCancerCapricorn || q.mirror == MirrorAxis::kBoth) {
          bed(i, 0, w1, w2, norm_rad(kPi - w));
        }
        break;
      }
      case StatObject::kName: {
        // bed_erf_nam matches on the string alone
        const int ls = helio ? body::kMoon : body::kSun;
        const double w3 = r.el[static_cast<std::size_t>(ls)];
        // INSTR(n$(i&),strin$) on the padded field, a lone blank finds
        // every name shorter than the field
        const bool hit = stat_padded_name(r.name).find(q.name) != std::string::npos;
        if (q.combine_and) {
          if (hit && mask[static_cast<std::size_t>(i)] > 0.0) {
            take(i, ls, w3);
          } else {
            mask[static_cast<std::size_t>(i)] = 0.0;
          }
        } else if (hit) {
          take(i, ls, w3);
        }
        break;
      }
      case StatObject::kArabicPart: {
        const double p1 = operand_value(r, q.a, q.classic_rulers, slot);
        const double p2 = operand_value(r, q.b, q.classic_rulers, slot);
        const double p3 = operand_value(r, q.c, q.classic_rulers, slot);
        double w3 = 0.0;
        if (q.arabic_day_night && day_chart(r)) {
          w3 = norm_rad(p1 + p3 - p2);
        } else {
          w3 = norm_rad(p1 + p2 - p3);
        }
        bed(i, 0, w1, w2, w3);
        break;
      }
    }
  }

  // the original stat_ausg_ar, the sun above the horizon
  [[nodiscard]] static bool day_chart(const StatRecord& r) {
    double w1 = norm_rad(kPi + r.ac);
    double w2 = r.ac - kEps;
    double w3 = r.el[body::kSun];
    vergl2(w1, w2, w3);
    //RR TAG
    return w1 < w3 && w3 < w2;
  }
};

}  // namespace

// ported from HORCOM stat_ausw
StatEvalResult evaluate_statistics(const StatSet& set, const StatQuery& q, const AspectSettings& a, std::vector<double>& mask) {
  const int laf = static_cast<int>(set.records.size());
  if (mask.size() != set.records.size()) {
    mask.assign(set.records.size(), 0.0);
  }
  Eval e{set, q, a, mask, {}, {}};

  // the static window and its wrap bounds
  double w1 = 0.0;
  double w2 = 0.0;
  double wu = 0.0;
  double wo = 0.0;
  switch (q.window) {
    case StatWindow::kAtDegree: {
      const double p = q.degree;
      const double obg = q.orb;
      wu = p - obg;
      wo = p + obg;
      if (p < obg && p >= 0.0) {
        w1 = kEps;
        w2 = p + obg;
      } else if (p + obg > kTwoPi && p <= kTwoPi) {
        w1 = p - obg;
        w2 = kTwoPi;
      } else {
        w1 = wu;
        w2 = wo;
      }
      break;
    }
    case StatWindow::kInSign: {
      const double ob = q.orb;
      wu = kDegToRad * ((q.sign - 1) * kDegPerSign - ob) + kEps;
      wo = kDegToRad * (q.sign * kDegPerSign + ob);
      // his w1 = ABS(wu) turned the orb before 0 Aries into a gap of the
      // same size after it, the wrap pass below covers the part before
      w1 = std::max(wu, kEps);
      w2 = std::min(wo, kTwoPi);
      break;
    }
    case StatWindow::kAnywhere:
      w1 = kEps;
      w2 = kTwoPi;
      break;
    default:
      break;
  }

  // the first pass over every record, house and body windows move
  const auto first_pass = [&]() {
    for (int i = 0; i < laf; ++i) {
      const StatRecord& r = set.records[static_cast<std::size_t>(i)];
      switch (q.window) {
        case StatWindow::kInHouse: {
          //RR HAUS
          if (r.heliocentric()) {
            w1 = 0.0;
            w2 = 0.0;
            break;
          }
          e.f = stat_houses(r);
          w1 = e.f[static_cast<std::size_t>(q.house)] + kEps;
          w2 = e.f[static_cast<std::size_t>(q.house) + 1];
          // his dw = (w2 - w1) * obp / 100 went negative for a house over
          // 0 Aries, cusps at 340 and 10 degrees with ten percent gave
          // -33 degrees and the window 13 to 337, the rest of the circle
          const double dw = norm_rad(w2 - w1) * q.house_orb_pct / kPercent;
          w1 = norm_rad(e.f[static_cast<std::size_t>(q.house)] - dw + kEps);
          w2 = norm_rad(e.f[static_cast<std::size_t>(q.house) + 1] + dw);
          break;
        }
        case StatWindow::kNearBody: {
          //RR BEI PLAN
          int slot = 0;
          const double t = operand_value(r, q.near_body, q.classic_rulers, slot);
          // his w1 = kk - orb + target stayed unnormalised, bed_erf wants
          // w1 > 0 and a window over 0 Aries never matched
          w1 = norm_rad(kEps - q.orb + t);
          w2 = norm_rad(q.orb + t);
          break;
        }
        default:
          break;
      }
      e.test(i, w1, w2);
    }
  };
  //RR Überschneidung bei 0/360
  // the part of the window before 0 Aries
  const bool wraps_low = wu < 0.0 && (q.window == StatWindow::kAtDegree || q.window == StatWindow::kInSign);
  const bool wraps_high = wo > kTwoPi && (q.window == StatWindow::kAtDegree || q.window == StatWindow::kInSign);
  const auto low_pass = [&]() {
    const double lo = q.window == StatWindow::kAtDegree ? kTwoPi - (q.orb - q.degree)
                                                        : kTwoPi - kDegToRad * (q.orb - kDegPerSign * (q.sign - 1));
    for (int i = 0; i < laf; ++i) {
      e.test(i, lo, kTwoPi);
    }
  };
  // and the part after 360
  const auto high_pass = [&]() {
    const double hi = q.window == StatWindow::kAtDegree ? q.orb + q.degree - kTwoPi : wo - kTwoPi;
    for (int i = 0; i < laf; ++i) {
      e.test(i, kEps, hi);
    }
  };
  if (q.combine_and && (wraps_low || wraps_high)) {
    // under UND every pass cleared the survivors the other pass had
    // kept, a window over 0 Aries never kept a record. Each pass starts
    // from the survivors of the earlier conditions and a record stays
    // when any pass keeps it
    const std::vector<double> before = mask;
    std::vector<double> kept(mask.size(), 0.0);
    const auto merge = [&]() {
      for (std::size_t i = 0; i < mask.size(); ++i) {
        kept[i] = std::max(kept[i], mask[i]);
      }
      mask = before;
    };
    first_pass();
    merge();
    if (wraps_low) {
      low_pass();
      merge();
    }
    if (wraps_high) {
      high_pass();
      merge();
    }
    mask = kept;
  } else {
    first_pass();
    if (wraps_low) {
      low_pass();
    }
    if (wraps_high) {
      high_pass();
    }
  }
  return e.res;
}

}  // namespace horcom
