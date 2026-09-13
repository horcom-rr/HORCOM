// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/statist_eval.hpp"

#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/composite.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

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
  const int jp = static_cast<int>(std::floor(6.0 * w / kPi)) + 1;
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

  // his mz per record cell stays untouched by the lights and all
  // bodies walks, only their failures clear it under UND
  [[nodiscard]] bool mask_writes() const {
    return q.object != StatObject::kLights && q.object != StatObject::kAllBodies;
  }

  //RR ZEICHEN-VERT / HÄUSER-VERT, the original sum_z_h
  void tally(double w3) {
    if (q.object == StatObject::kAspect || q.object == StatObject::kMidpointAspect || q.object == StatObject::kName) {
      return;  // suc stands at zero there
    }
    if (q.window == StatWindow::kInHouse) {
      for (int j = 1; j <= 12; ++j) {
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
    for (int j = 1; j <= 12; ++j) {
      double w1 = (j - 1) * kPi / 6.0 + kEps;
      double w2 = j * kPi / 6.0;
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
    if (mask_writes()) {
      mask[static_cast<std::size_t>(i)] = norm_rad(w3);
    }
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

  // the original bed_erf_asp1 and bed_erf_asp2, fa keeps the sticky
  // multiple flags of one record
  void bed_asp(int i, double dds, double pn, int n, double w3, std::array<bool, 13>& fa) {
    if (n == 1) {
      //RR Konj
      double w1 = 0.0;
      double w2 = dds;
      vergl2(w1, w2, w3);
      if (std::abs(w3 - kTwoPi) < dds) {
        w3 = std::abs(kTwoPi - w3);
      }
      const bool hit = w3 > 0.0 && w3 < dds;
      if (q.combine_and) {
        if (hit && mask[static_cast<std::size_t>(i)] > 0.0) {
          take(i, 0, w3);
          fa[1] = true;
        } else if (fa[1]) {
          mask[static_cast<std::size_t>(i)] = 0.0;
        }
      } else if (hit) {
        take(i, 0, w3);
      }
      return;
    }
    for (int m = 1; m <= n - 1; ++m) {
      if (a.equal_probability && !multiple_allowed(n, m)) {
        continue;
      }
      const double w1 = norm_rad(m * pn - dds);
      const double w2 = norm_rad(m * pn + dds);
      const bool hit = w1 < w3 && w3 < w2;
      if (q.combine_and) {
        if (hit && mask[static_cast<std::size_t>(i)] > 0.0) {
          take(i, 0, w3);
          fa[static_cast<std::size_t>(m)] = true;
        } else if (fa[static_cast<std::size_t>(m)]) {
          mask[static_cast<std::size_t>(i)] = 0.0;
        }
      } else if (hit) {
        take(i, 0, w3);
      }
    }
  }

  // the original bed_erf_asp over one folded separation
  void bed_asp_all(int i, double w3, double o1, double o2, double o3, bool three) {
    tally(w3);
    std::array<bool, 13> fa{};
    //RR EINZEL-ASPEKT MIT VORGEG. ORBIS
    if (q.asp_orb > 0.0 && q.asp_low == q.asp_high) {
      const int n = q.asp_high;
      const double pn = kTwoPi / n;
      const double dds = three ? orbis_discr3(o1, o2, o3, q.asp_orb) : orbis_discr2(o1, o2, q.asp_orb);
      bed_asp(i, dds, pn, n, w3, fa);
      return;
    }
    for (int n = q.asp_low; n <= q.asp_high; ++n) {
      const double pn = kTwoPi / n;
      const double dd = a.equal_probability ? a.orb * a.orbe[static_cast<std::size_t>(n)] : a.orb * pn / 30.0;
      const double dds = three ? orbis_discr3(o1, o2, o3, dd) : orbis_discr2(o1, o2, dd);
      bed_asp(i, dds, pn, n, w3, fa);
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
        if (!helio) {
          bed(i, body::kSun, w1, w2, r.el[body::kSun]);
          bed(i, body::kMoon, w1, w2, r.el[body::kMoon]);
        } else {
          bed(i, body::kMoon, w1, w2, r.el[body::kMoon]);
        }
        bed(i, body::kAscendant, w1, w2, r.ac);
        break;
      }
      case StatObject::kAllBodies: {
        for (int k = 1; k <= 14; ++k) {
          bed(i, k, w1, w2, slot_position(r, k));
        }
        for (int e = 1; e <= 22; ++e) {
          const int nk = set.params.nk[static_cast<std::size_t>(e)];
          if (nk > 18) {
            bed(i, nk, w1, w2, r.el[static_cast<std::size_t>(nk)]);
          }
        }
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
        const bool hit = r.name.find(q.name) != std::string::npos;
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
      w1 = std::abs(wu);
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
        const double dw = (w2 - w1) * q.house_orb_pct / 100.0;
        w1 = norm_rad(e.f[static_cast<std::size_t>(q.house)] - dw + kEps);
        w2 = norm_rad(e.f[static_cast<std::size_t>(q.house) + 1] + dw);
        break;
      }
      case StatWindow::kNearBody: {
        //RR BEI PLAN
        int slot = 0;
        const double t = operand_value(r, q.near_body, q.classic_rulers, slot);
        w1 = kEps - q.orb + t;
        w2 = q.orb + t;
        break;
      }
      default:
        break;
    }
    e.test(i, w1, w2);
  }

  //RR Überschneidung bei 0/360
  for (int i = 0; i < laf; ++i) {
    if (wu < 0.0) {
      switch (q.window) {
        case StatWindow::kAtDegree:
          w1 = kTwoPi - (q.orb - q.degree);
          w2 = kTwoPi;
          break;
        case StatWindow::kInSign:
          w1 = kTwoPi - kDegToRad * (q.orb - kDegPerSign * (q.sign - 1));
          w2 = kTwoPi;
          break;
        default:
          continue;
      }
      e.test(i, w1, w2);
    }
    if (wo > kTwoPi) {
      switch (q.window) {
        case StatWindow::kAtDegree:
          w1 = kEps;
          w2 = q.orb + q.degree - kTwoPi;
          break;
        case StatWindow::kInSign:
          w1 = kEps;
          w2 = wo - kTwoPi;
          break;
        default:
          continue;
      }
      e.test(i, w1, w2);
    }
  }
  return e.res;
}

}  // namespace horcom
