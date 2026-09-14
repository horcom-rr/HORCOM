// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/rhythm.hpp"

#include <cmath>
#include <cstdio>
#include <fstream>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/mundane.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/statist_eval.hpp"

namespace horcom {

namespace {

struct Walk {
  const Chart& chart;
  const AspectResult& aspects;
  const RhythmOptions& opt;
  std::vector<RhythmTrigger> rows;
  // the mirror matrix of spieg1, sums near half or the whole circle
  std::array<std::array<bool, body::kSlotCount>, body::kSlotCount> mirror{};
  int phase = 0;
  int house = 0;

  [[nodiscard]] double position(int slot) const {
    if (slot == body::kAscendant) {
      //RR AC und MC hauchdünn ins Haus gelegt
      return chart.houses.cusp[1] + (opt.leftward ? kEps : -kEps);
    }
    if (slot == body::kMc) {
      return chart.houses.cusp[10] + (opt.leftward ? kEps : -kEps);
    }
    return chart.b[static_cast<std::size_t>(slot)].el;
  }

  [[nodiscard]] bool usable(int slot) const {
    if (slot == body::kAscendant || slot == body::kMc) {
      return chart.houses.ok;
    }
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    return b.present && b.valid;
  }

  // the age stamp of a171, the phase gives the years, the body's
  // place within its own house the fraction
  void stamp(int slot, int source, RhythmKind kind, double angle_deg, double w3_raw) {
    for (int a = 1; a <= 12; ++a) {
      double w1 = chart.houses.cusp[static_cast<std::size_t>(a)];
      double w2 = chart.houses.cusp[static_cast<std::size_t>(a) + 1];
      double w3 = w3_raw;
      vergl2(w1, w2, w3);
      if (w3 > w1 && w3 < w2) {
        RhythmTrigger t;
        t.phase = phase;
        t.house = house;
        t.slot = slot;
        t.source = source;
        t.kind = kind;
        t.angle_deg = angle_deg;
        const double vp = opt.phase_years;
        if (opt.leftward) {
          t.value = (house - 1) * vp + vp * (w3 - w1) / (kEps + w2 - w1);
        } else {
          t.value = (12 - house) * vp + vp * (w2 - w3) / (kEps + w2 - w1);
        }
        if (opt.months) {
          t.value /= 12.0;
        }
        rows.push_back(t);
      }
    }
  }

  // the aspect chain of a172, main aspects out of the radix matrix,
  // the sextile on demand
  void aspect_chain(int ps) {
    const auto follow = [this, ps](int o, double wa) {
      if (wa <= 0.0) {
        return;
      }
      const int n = aspect_symbol(wa, opt.sextile ? 6 : 4);
      const bool wanted = (n > 0 && n < 5 && !opt.sextile) || (opt.sextile && n > 0 && n < 7 && n != 5);
      if (!wanted) {
        return;
      }
      double folded = wa;
      if (folded > kPi && folded < kTwoPi) {
        folded = kTwoPi - folded;
      }
      stamp(o, ps, RhythmKind::kAspect, folded * kRadToDeg, position(o));
    };
    for (int o = 0; o < ps; ++o) {
      if (usable(o) && o != body::kNodeDesc) {
        follow(o, aspects.asp[static_cast<std::size_t>(o)][static_cast<std::size_t>(ps)]);
      }
    }
    for (int o = 0; o < body::kSlotCount; ++o) {
      if (usable(o) && o != body::kNodeDesc) {
        follow(o, aspects.asp[static_cast<std::size_t>(ps)][static_cast<std::size_t>(o)]);
      }
    }
  }

  // the mirror chain of a173
  void mirror_chain(int ss) {
    for (int r = 0; r < body::kSlotCount; ++r) {
      if (!usable(r) || r == body::kNodeDesc) {
        continue;
      }
      if (mirror[static_cast<std::size_t>(r)][static_cast<std::size_t>(ss)] ||
          mirror[static_cast<std::size_t>(ss)][static_cast<std::size_t>(r)]) {
        stamp(r, ss, RhythmKind::kMirror, 0.0, position(r));
      }
    }
  }

  void ruler_row(double sign_point, RhythmKind kind) {
    const int kp = sign_ruler(sign_point, false);
    if (kp <= 0 || !usable(kp)) {
      return;
    }
    stamp(kp, 0, kind, 0.0, position(kp));
    aspect_chain(kp);
    mirror_chain(kp);
  }
};

// ported from spieg1, marks pairs whose longitudes sum onto the whole
// or the half circle within the doubled orb
void build_mirrors(Walk& w, const AspectSettings& a) {
  //RR Spiegelung
  const double dd = a.orb * 2.0 * kDegToRad;
  for (int t = 1; t < body::kSlotCount - 1; ++t) {
    if (!w.usable(t) || t == body::kNodeDesc) {
      continue;
    }
    const double o1 = org(a, t, 1);
    for (int u = t + 1; u < body::kSlotCount; ++u) {
      if (!w.usable(u) || u == body::kNodeDesc) {
        continue;
      }
      const double o2 = org(a, u, 1);
      const double dds = orbis_discr2(o1, o2, dd);
      const double at = norm_rad(w.position(t));
      const double au = norm_rad(w.position(u));
      const double w11 = std::abs(norm_rad(kPi - dds - au));
      const double w12 = std::abs(norm_rad(kPi + dds - au));
      const double w21 = std::abs(norm_rad(kTwoPi - dds - au));
      const double w22 = std::abs(norm_rad(kTwoPi + dds - au));
      if ((at > w11 && at < w12) || (at > w21 && at < w22) || (at > kTwoPi - dds && w22 < dds) ||
          (at < dds && w22 > kTwoPi - dds)) {
        w.mirror[static_cast<std::size_t>(t)][static_cast<std::size_t>(u)] = true;
      }
    }
  }
}

}  // namespace

// ported from HORCOM a170 with a171, a172, a173 and the sign rules
std::vector<RhythmTrigger> rhythm_triggers(const Chart& chart, const AspectResult& aspects, const AspectSettings& a, const RhythmOptions& opt) {
  Walk w{chart, aspects, opt, {}, {}, 0, 0};
  build_mirrors(w, a);
  const auto cusp = [&chart](int h) { return chart.houses.cusp[static_cast<std::size_t>(h)]; };
  int from = 0;
  int to = 0;
  int step = 0;
  if (opt.leftward) {
    from = opt.begin_house;
    to = 12;
    step = 1;
  } else {
    from = 12 - opt.begin_house + 1;
    to = 1;
    step = -1;
  }
  int l = from - step;
  int phase = 0;
  do {
    l += step;
    ++phase;
    w.phase = phase;
    w.house = l;

    // the ruler of the phase's sign, with the intercepted signs
    const double lead = opt.leftward ? cusp(l) : cusp(l + 1);
    const int m1 = static_cast<int>(std::floor(6.0 * norm_rad(lead) / kPi)) + 1;
    w.ruler_row(lead, RhythmKind::kRuler);
    const double tail = opt.leftward ? cusp(l + 1) : cusp(l);
    int m2 = static_cast<int>(std::floor(6.0 * norm_rad(tail) / kPi)) + 1;
    if (opt.leftward) {
      if (m1 - m2 > 6) {
        m2 += 12;
      }
      const int span = m2 - m1;
      if (span == 2 || span == 3) {
        w.ruler_row(norm_rad(cusp(l + 1) - kPi / 6.0), span == 2 ? RhythmKind::kRuler2 : RhythmKind::kRuler3);
      }
    } else {
      int m1r = m1;
      if (m1r - m2 > 6) {
        m1r += 12;
      }
      const int span = m1r - m2;
      if (span == 2) {
        w.ruler_row(norm_rad(cusp(l) + kPi / 6.0), RhythmKind::kRuler2);
      } else if (span == 3) {
        w.ruler_row(norm_rad(cusp(l + 1) - kPi / 6.0), RhythmKind::kRuler3);
      }
    }

    // every body standing in the phase's house, the direct triggers
    double w1 = cusp(l);
    double w2 = cusp(l + 1);
    for (int pl = 1; pl < body::kSlotCount; ++pl) {
      if (pl >= 15 && pl <= 18) {
        continue;
      }
      if (!w.usable(pl) || pl == body::kNodeDesc) {
        continue;
      }
      double w3 = norm_rad(w.position(pl));
      bool opposite = false;
      for (;;) {
        double a1 = w1;
        double a2 = w2;
        double a3 = w3;
        vergl2(a1, a2, a3);
        if (a3 > a1 && a1 > 0.0 && a3 < a2) {
          const double vp = opt.phase_years;
          RhythmTrigger t;
          t.phase = phase;
          t.house = l;
          t.slot = pl;
          t.kind = RhythmKind::kDirect;
          if (opt.leftward) {
            t.value = (l - 1) * vp + vp * (a3 - a1) / (kEps + a2 - a1);
          } else {
            t.value = (12 - l) * vp + vp * (a2 - a3) / (kEps + a2 - a1);
          }
          if (opt.months) {
            t.value /= 12.0;
          }
          w.rows.push_back(t);
          w.aspect_chain(pl);
          w.mirror_chain(pl);
        }
        //RR das Apogäum spricht auch am Gegenpunkt an
        if (pl == body::kApogee && opt.apogee_opposite && !opposite) {
          w3 = norm_rad(w3 + kPi);
          opposite = true;
          continue;
        }
        break;
      }
    }
  } while (l != to);
  return w.rows;
}

namespace {

// the published Gruppenschicksals-Grade of W. Döbereiner, half degree
// index with the planet pair of the characteristic, his a174g calls
struct Gsp {
  int half;
  int p;
  int q;
};

constexpr Gsp kGsp[] = {
    {9, 5, 9}, {11, 8, 9}, {15, 8, 1}, {18, 8, 9}, {20, 8, 2}, {28, 8, 6},
    {35, 8, 5}, {44, 5, 6}, {48, 5, 4}, {55, 7, 9}, {66, 6, 4}, {70, 3, 8},
    {74, 7, 11}, {80, 1, 10}, {81, 4, 10}, {99, 2, 10}, {100, 1, 10}, {106, 7, 11},
    {110, 8, 3}, {114, 6, 4}, {125, 7, 3}, {132, 4, 7}, {136, 6, 5}, {145, 5, 7},
    {160, 8, 2}, {162, 8, 10}, {165, 8, 1}, {169, 8, 9}, {171, 3, 9}, {189, 2, 9},
    {191, 8, 9}, {195, 8, 1}, {198, 8, 10}, {200, 8, 2}, {214, 2, 7}, {224, 6, 5},
    {228, 2, 4}, {235, 3, 7}, {236, 5, 10}, {246, 6, 10}, {250, 3, 8}, {254, 7, 11},
    {260, 1, 10}, {261, 2, 10}, {279, 4, 10}, {280, 1, 10}, {286, 7, 11}, {290, 3, 8},
    {294, 6, 1}, {305, 7, 1}, {312, 4, 3}, {316, 6, 5}, {325, 8, 7}, {332, 6, 8},
    {340, 8, 2}, {342, 8, 9}, {345, 8, 1}, {349, 8, 9}, {351, 9, 4}, {369, 4, 9},
    {371, 8, 9}, {375, 8, 1}, {378, 8, 9}, {380, 8, 2}, {388, 6, 8}, {395, 8, 7},
    {404, 6, 5}, {408, 4, 3}, {415, 7, 1}, {426, 6, 1}, {430, 8, 3}, {434, 7, 11},
    {440, 1, 10}, {441, 4, 10}, {459, 2, 10}, {460, 1, 10}, {466, 7, 11}, {470, 8, 3},
    {474, 6, 10}, {484, 5, 10}, {485, 7, 3}, {493, 2, 8}, {496, 5, 6}, {505, 7, 2},
    {520, 2, 8}, {522, 10, 8}, {525, 1, 8}, {529, 9, 8}, {531, 2, 9}, {549, 3, 9},
    {551, 9, 8}, {555, 1, 8}, {558, 10, 8}, {560, 2, 8}, {575, 5, 7}, {584, 5, 6},
    {588, 4, 7}, {595, 3, 7}, {606, 4, 6}, {610, 3, 8}, {614, 7, 11}, {620, 1, 10},
    {621, 2, 10}, {639, 4, 10}, {640, 1, 10}, {646, 7, 11}, {650, 3, 8}, {654, 8, 6},
    {665, 7, 9}, {672, 5, 4}, {676, 5, 6}, {685, 5, 8}, {692, 6, 8}, {700, 2, 8},
    {702, 9, 8}, {705, 1, 8}, {709, 9, 8}, {711, 5, 9},
};

}  // namespace

std::vector<CustomDegree> read_degrees(const std::filesystem::path& file) {
  std::vector<CustomDegree> out;
  std::ifstream in(file);
  if (!in) {
    return out;
  }
  std::string line;
  while (std::getline(in, line)) {
    // his INPUT gr$,p1$,p2$ rows, commas separate the three values
    const std::size_t c1 = line.find(',');
    const std::size_t c2 = c1 == std::string::npos ? std::string::npos : line.find(',', c1 + 1);
    if (c2 == std::string::npos) {
      continue;
    }
    try {
      const double deg = std::stod(line.substr(0, c1));
      const int p = std::stoi(line.substr(c1 + 1, c2 - c1 - 1));
      const int q = std::stoi(line.substr(c2 + 1));
      out.push_back({deg, p, q});
    } catch (...) {
      continue;
    }
  }
  return out;
}

bool write_degrees(const std::filesystem::path& file, const std::vector<CustomDegree>& rows) {
  std::ofstream out(file, std::ios::trunc);
  if (!out) {
    return false;
  }
  for (const CustomDegree& r : rows) {
    out << r.degree << "," << r.p << "," << r.q << "\n";
  }
  return static_cast<bool>(out);
}

// ported from HORCOM a17_3 and a171
std::vector<DegreeDate> degree_dates(const Chart& chart, const RhythmOptions& opt, const std::vector<CustomDegree>& own, bool mundane, double lat_deg) {
  std::vector<DegreeDate> out;
  if (!chart.houses.ok) {
    return out;
  }
  std::array<double, 14> fz = chart.houses.cusp;
  if (mundane) {
    //RR im Mundan-Horoskop ... äquale Häuser von je 30 Grad auf dem ÄQUATOR
    for (int i = 1; i <= 12; ++i) {
      fz[static_cast<std::size_t>(i)] = (i - 1) * kPi / 6.0;
    }
  }
  fz[13] = fz[1];
  const double armcb = chart.armc_deg * kDegToRad;
  const double vp = opt.phase_years;
  out.reserve(720);
  for (int i = 0; i < 720; ++i) {
    DegreeDate row;
    row.degree = i / 2.0;
    for (const Gsp& g : kGsp) {
      if (g.half == i) {
        row.p = g.p;
        row.q = g.q;
        break;
      }
    }
    for (const CustomDegree& c : own) {
      const int half = static_cast<int>(c.degree * 2.0 + 0.5);
      if (half == i) {
        row.p = c.p;
        row.q = c.q;
        row.custom = true;
      } else if ((720 - half) % 720 == i) {
        //RR der Spiegelpunkt an 0 Widder-Waage mit gleicher Charakteristik
        row.p = c.p;
        row.q = c.q;
        row.custom = true;
        row.mirror = true;
      }
    }
    double w3 = row.degree * kDegToRad;
    if (mundane) {
      w3 = mundane_longitude(w3, kEps, chart.smo.ekls, armcb, lat_deg);
    }
    for (int a = 1; a <= 12; ++a) {
      double w1 = fz[static_cast<std::size_t>(a)];
      double w2 = fz[static_cast<std::size_t>(a) + 1];
      double v3 = w3;
      vergl2(w1, w2, v3);
      if (v3 > w1 && v3 < w2) {
        row.house = a;
        if (opt.leftward) {
          row.value = (a - 1) * vp + vp * (v3 - w1) / (kEps + w2 - w1);
        } else {
          row.value = (12 - a) * vp + vp * (w2 - v3) / (kEps + w2 - w1);
        }
        if (opt.months) {
          row.value /= 12.0;
        }
        break;
      }
    }
    out.push_back(row);
  }
  return out;
}

}  // namespace horcom
