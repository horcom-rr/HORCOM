// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/dynamogram.hpp"

#include <array>
#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/calendar.hpp"

namespace horcom {

namespace {

/// samples per progressed day, his one hundred twenty pixels a year
constexpr int kPerDay = 120;
/// sample days around the asked age, twenty five before and after
constexpr int kDays = 50;
/// the length of the summed curves
constexpr int kCurve = 6001;

// the amplitudes of the radix arcs, his ampl_rad in display units
double ampl_rad(int pl) {
  switch (pl) {
    case 0: return 2.0 * 40.0;
    case 1: return 3.0 * 40.0;
    case 2: return 1.0 * 40.0;
    case 11:
    case 12: return 1.5 * 40.0;
    case 13:
    case 14: return 3.0 * 40.0;
    default: return 2.0 * 40.0;
  }
}

// the amplitudes of the running pairs, his ampl_mund
double ampl_mund(int pl) {
  switch (pl) {
    case 0: return 1.5 * 40.0;
    case 1: return 2.0 * 40.0;
    case 2: return 0.0;
    case 11:
    case 12: return 0.0;
    case 13:
    case 14: return 2.5 * 40.0;
    default: return 1.5 * 40.0;
  }
}

// the orb of a running point in degrees, his orbh, bell curves shrink
// it by a third
double orbh(int pl, bool gauss) {
  double o = 1.0;
  switch (pl) {
    case 1: o = 2.0; break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12: o = 0.25; break;
    case 13:
    case 14: o = 0.5; break;
    default: o = 1.0; break;
  }
  if (gauss) {
    o *= 0.66666;
  }
  return o;
}

// the valuation of one pair under one angle, his wert scheme, plus
// one strengthening, minus one straining
int valuation(int na, int ma, int pl, int rd) {
  if (pl < 1 || pl > 14 || rd < 1 || rd > 14 || pl == 11 || pl == 12) {
    return 0;
  }
  if ((pl == 13 || pl == 14) && (rd == 13 || rd == 14)) {
    return 0;
  }
  const auto benefic = [](int s) { return s == 1 || s == 2 || s == 3 || s == 4 || s == 6 || (s >= 11 && s <= 14); };
  switch (na) {
    case 1:
      //RR Konjunktion
      if (ma != 0) {
        return 0;
      }
      switch (pl) {
        case 5: return (rd == 1 || rd == 3 || rd == 4 || rd == 6 || rd == 13) ? 1 : -1;
        case 7: return -1;
        case 8: return rd == 6 ? 1 : -1;
        case 9: return -1;
        case 10: return (rd == 2 || (rd >= 7 && rd <= 10)) ? -1 : 1;
        default: return benefic(rd) ? 1 : -1;
      }
    case 2:
      //RR Opposition
      return ma == 1 ? -1 : 0;
    case 3:
      //RR Trigon, nur positiv
      return (ma == 1 || ma == 2) ? 1 : 0;
    case 4:
      //RR Quadrat
      return (ma == 1 || ma == 3) ? -1 : 0;
    case 5:
      return (ma >= 1 && ma <= 4) ? (benefic(pl) ? 1 : -1) : 0;
    case 6:
      //RR Sextil
      return (ma == 1 || ma == 5) ? 1 : 0;
    case 8:
      //RR Halbquadrat, Anderthalbquadrat
      return (ma == 1 || ma == 3 || ma == 5 || ma == 7) ? -1 : 0;
    case 12:
      //RR Halbsextil, Quinkunx
      return (ma == 1 || ma == 5 || ma == 7 || ma == 11) ? (benefic(pl) ? 1 : -1) : 0;
    default:
      return 0;
  }
}

// the multiples each divisor scans, exactly the sets of asp_analy
std::vector<int> multiples(int na, bool classic_minors, bool quincunx) {
  switch (na) {
    case 1: return {0};
    case 2: return {1};
    case 3: return {1, 2};
    case 4: return {1, 3};
    case 6: return {1, 5};
    case 8: return classic_minors ? std::vector<int>{1, 3, 5, 7} : std::vector<int>{};
    case 12:
      if (classic_minors) {
        return {1, 5, 7, 11};
      }
      if (quincunx) {
        return {5, 7};
      }
      return {};
    default: return {};
  }
}

struct Run {
  const Chart& radix;
  const DynamogramOptions& opt;
  const SearchContext& ctx;
  Dynamogram out;
  // one progressed longitude per sample day and running point
  std::array<std::array<double, 15>, kDays + 1> elint{};
  // the daily motion of each point, his anst
  std::array<std::array<double, kDays + 1>, 15> anst{};

  [[nodiscard]] double radix_pos(int rd) const {
    if (rd == 13) {
      return radix.houses.cusp[1];
    }
    if (rd == 14) {
      return radix.houses.cusp[10];
    }
    return radix.b[static_cast<std::size_t>(rd)].el;
  }

  // fills the fifty one sample days, one day of sky per year of life,
  // his hubephp forward and hubephn backward
  void sample(bool regressive) {
    for (int n = 0; n <= kDays; ++n) {
      const double jd = regressive ? radix.jd_ut - opt.from_age + 25.0 - n
                                   : radix.jd_ut + opt.from_age - 25.0 + n;
      ChartInput in = ctx.base;
      in.date_ut = calendar_date(jd, ctx.settings.calendar);
      const Chart c = compute_chart(in, ctx.settings, *ctx.vsop, *ctx.eph);
      for (int pl = 1; pl <= 14; ++pl) {
        double v = 0.0;
        if (pl == 13) {
          v = c.houses.cusp[1];
        } else if (pl == 14) {
          v = c.houses.cusp[10];
        } else if (c.b[static_cast<std::size_t>(pl)].present && c.b[static_cast<std::size_t>(pl)].valid) {
          v = c.b[static_cast<std::size_t>(pl)].el;
        }
        elint[static_cast<std::size_t>(n)][static_cast<std::size_t>(pl)] = v;
        if (n > 0) {
          double w1 = elint[static_cast<std::size_t>(n) - 1][static_cast<std::size_t>(pl)];
          double w2 = v;
          vergl1(w1, w2);
          anst[static_cast<std::size_t>(pl)][static_cast<std::size_t>(n)] = w2 - w1;
        }
      }
    }
  }

  // one finished arc summed into the curves, his hub_auswert
  void arc(int pl, int rd, int na, int ma, int n1, int n2, double dds, bool mutual) {
    const double pn = kTwoPi / na;
    double d1 = 0.0;
    double d2 = 0.0;
    {
      const double eli1 = mutual ? norm_rad(elint[static_cast<std::size_t>(n1)][static_cast<std::size_t>(rd)] + pn * ma)
                                 : norm_rad(radix_pos(rd) + pn * ma);
      const double eli2 = mutual ? norm_rad(elint[static_cast<std::size_t>(n2)][static_cast<std::size_t>(rd)] + pn * ma)
                                 : eli1;
      double w1 = elint[static_cast<std::size_t>(n1)][static_cast<std::size_t>(pl)];
      double w2 = eli1;
      vergl1(w1, w2);
      d1 = std::abs(w2 - w1);
      w1 = elint[static_cast<std::size_t>(n2)][static_cast<std::size_t>(pl)];
      w2 = eli2;
      vergl1(w1, w2);
      d2 = std::abs(w2 - w1);
    }
    const double bw = 2.0 * dds * (n2 - n1) / (d1 + d2 + kEps);
    const double tb = (d1 * n2 + d2 * n1) / (d1 + d2 + kEps);
    const double b = bw / 2.0;
    const int bwp = static_cast<int>(std::lround(bw * kPerDay));
    const int i1 = static_cast<int>(std::lround((opt.gauss ? tb - 6.0 * b : tb - b) * kPerDay));
    const int i2 = static_cast<int>(std::lround((opt.gauss ? tb + 6.0 * b : tb + b) * kPerDay));
    const int im = static_cast<int>(std::lround(tb * kPerDay));
    if (!(bwp > 0 && i1 > 0 && i2 < kCurve - 1 && i2 > 0 && i1 < kCurve - 1)) {
      return;
    }
    const int sign = valuation(na, ma, pl, rd);
    for (int i = i1; i <= i2; ++i) {
      double ci = 0.0;
      if (opt.gauss) {
        const double ep = static_cast<double>(i - im) / (bwp + kEps);
        ci = sign * std::abs(std::exp(-ep * ep));
      } else {
        ci = sign * std::abs(std::cos(kPi * (i - im) / (bwp + kEps)));
      }
      double anz = 0.0;
      bool existential = false;
      if (mutual) {
        anz = std::max(ampl_mund(pl), ampl_mund(rd)) * ci;
      } else {
        anz = ampl_rad(pl) * ci;
        if (pl == 13 || pl == 14) {
          //RR nur AC,MC = existenzielle Situation
          existential = true;
        } else if (anst[static_cast<std::size_t>(pl)][static_cast<std::size_t>(n1)] >
                   anst[14][static_cast<std::size_t>(n1)]) {
          existential = bw <= 3.0;
        }
      }
      if (existential) {
        out.existential[static_cast<std::size_t>(i)] += anz;
      } else {
        out.mood[static_cast<std::size_t>(i)] += anz;
      }
    }
  }

  // the orb entry and exit walk of asp_analy, both crossing directions
  void analyze(int pl, int rd, int na, int ma, bool mutual) {
    const double pn = kTwoPi / na;
    int n1 = 0;
    int n2 = 0;
    bool armed_fwd = false;
    bool armed_back = false;
    for (int n = 0; n <= kDays; ++n) {
      const double wa1 = elint[static_cast<std::size_t>(n)][static_cast<std::size_t>(pl)];
      const double wa2 = mutual ? norm_rad(elint[static_cast<std::size_t>(n)][static_cast<std::size_t>(rd)] + pn * ma)
                                : norm_rad(radix_pos(rd) + pn * ma);
      double dds = 0.0;
      if (mutual) {
        //RR der schnellere Läufer gibt den Orbis
        dds = std::abs(anst[static_cast<std::size_t>(pl)][static_cast<std::size_t>(n)]) >
                      std::abs(anst[static_cast<std::size_t>(rd)][static_cast<std::size_t>(n)])
                  ? kDegToRad * orbh(pl, opt.gauss)
                  : kDegToRad * orbh(rd, opt.gauss);
      } else {
        dds = kDegToRad * orbh(pl, opt.gauss);
      }
      if (!armed_fwd && !armed_back) {
        double w1 = wa1;
        double w2 = wa2;
        vergl1(w1, w2);
        if (w2 - w1 < dds && w2 - w1 > kEps) {
          n1 = n;
          armed_fwd = true;
          continue;
        }
        w1 = wa1;
        w2 = wa2;
        vergl1r(w1, w2);
        if (w1 - w2 < dds && w1 - w2 > kEps) {
          n1 = n;
          armed_back = true;
          continue;
        }
      } else if (armed_fwd) {
        double w1 = wa1;
        double w2 = wa2;
        vergl1(w1, w2);
        if (w1 - w2 > dds && w1 - w2 < kPi) {
          n2 = n;
          if (n2 > n1) {
            arc(pl, rd, na, ma, n1, n2, dds, mutual);
          }
          armed_fwd = false;
        }
      } else if (armed_back) {
        double w1 = wa1;
        double w2 = wa2;
        vergl1r(w1, w2);
        if (w2 - w1 > dds && w2 - w1 < kPi) {
          n2 = n;
          if (n2 > n1) {
            arc(pl, rd, na, ma, n1, n2, dds, mutual);
          }
          armed_back = false;
        }
      }
    }
  }

  // the running points against the radix, his asp_analy_rad
  void against_radix() {
    for (int pl = 1; pl <= 14; ++pl) {
      if (pl == 12) {
        continue;
      }
      if (pl == 2 && !opt.with_moon) {
        continue;
      }
      for (int rd = 1; rd <= 14; ++rd) {
        if (rd == 12) {
          continue;
        }
        const bool angle_pl = pl == 13 || pl == 14;
        const bool angle_rd = rd == 13 || rd == 14;
        if ((angle_pl && angle_rd) || (pl == 11 && angle_rd) || (angle_pl && rd == 11)) {
          continue;
        }
        for (const int na : {1, 2, 3, 4, 6, 8, 12}) {
          for (const int ma : multiples(na, opt.classic_minors, opt.quincunx)) {
            if (na == 1 && pl == rd) {
              //RR Radix-Werte selbst ausschalten
              continue;
            }
            analyze(pl, rd, na, ma, false);
          }
        }
      }
    }
  }

  // the running points among themselves, his asp_analy_mund
  void mutual_pairs() {
    for (int pl = 1; pl <= 10; ++pl) {
      if (pl == 2) {
        continue;
      }
      for (int rd = pl + 1; rd <= 14; ++rd) {
        if (rd == 11 || rd == 12) {
          continue;
        }
        for (const int na : {1, 2, 3, 4, 6, 8, 12}) {
          for (const int ma : multiples(na, opt.classic_minors, opt.quincunx)) {
            analyze(pl, rd, na, ma, true);
          }
        }
      }
    }
  }
};

}  // namespace

// ported from HORCOM huber with wert, hubephp, hubephn, asp_analy_rad,
// asp_analy_mund and the two hub_auswert builders
Dynamogram dynamogram(const Chart& radix, const DynamogramOptions& opt, const SearchContext& ctx) {
  Run r{radix, opt, ctx, {}, {}, {}};
  r.out.from_age = opt.from_age;
  r.out.existential.assign(kCurve, 0.0);
  r.out.mood.assign(kCurve, 0.0);
  r.sample(false);
  r.against_radix();
  r.mutual_pairs();
  if (opt.regressive) {
    r.sample(true);
    r.against_radix();
    r.mutual_pairs();
  }
  return r.out;
}

}  // namespace horcom
