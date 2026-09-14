// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/histogram.hpp"

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/statist_eval.hpp"

namespace horcom {

namespace {

// his old fixed values before the weights became editable
constexpr std::array<int, 16> kOldPoints = {0, 6, 6, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 6, 6, 0};

// the common skip of both counters, kardinal slots geocentric, node
// block and the axisless extras heliocentric
bool skipped(int t, const ChartSettings& s) {
  if (!s.heliocentric && t > 14 && t < 19) {
    return true;
  }
  if (s.heliocentric && ((t > 10 && t < 19) || t == s.nk[1] || t == s.nk[4])) {
    return true;
  }
  return false;
}

// the doubling factors, first house planets and the birth ruler. The
// original computed them four times and the quality by house pass
// forgot both switches, here one shared pass honours them everywhere
std::array<int, body::kSlotCount> factors(const Chart& chart, const ChartSettings& s, const HistogramOptions& opt) {
  std::array<int, body::kSlotCount> f{};
  f.fill(1);
  if (s.heliocentric || !chart.houses.ok) {
    return f;
  }
  if (opt.double_first_house) {
    //RR 1.Haus doppelt
    for (int t = 1; t <= 10; ++t) {
      double w1 = chart.houses.cusp[1];
      double w2 = chart.houses.cusp[2];
      double w3 = chart.b[static_cast<std::size_t>(t)].el;
      vergl2(w1, w2, w3);
      if (w1 < w3 && w3 < w2) {
        f[static_cast<std::size_t>(t)] = 2;
      }
    }
  }
  if (opt.double_ruler) {
    //RR GebHerr doppelt
    const int kp = sign_ruler(chart.houses.cusp[1], opt.classic_rulers);
    if (kp > 0 && kp < body::kSlotCount) {
      f[static_cast<std::size_t>(kp)] = 2;
    }
  }
  return f;
}

}  // namespace

std::array<int, body::kSlotCount> histogram_points(const std::array<int, 16>& pn) {
  bool any = false;
  for (int i = 1; i <= 15; ++i) {
    any = any || pn[static_cast<std::size_t>(i)] != 0;
  }
  const std::array<int, 16>& row = any ? pn : kOldPoints;
  std::array<int, body::kSlotCount> out{};
  for (int slot = 1; slot <= 14; ++slot) {
    out[static_cast<std::size_t>(slot)] = row[static_cast<std::size_t>(slot)];
  }
  //RR pn(15) gilt für alle Zusatz-Planeten
  for (int slot = 19; slot < body::kSlotCount; ++slot) {
    out[static_cast<std::size_t>(slot)] = row[15];
  }
  return out;
}

// ported from HORCOM elem1, elem2, kard_fix_gem and kard_fix_gemh
Histogram chart_histogram(const Chart& chart, const ChartSettings& s, const HistogramOptions& opt) {
  Histogram out;
  const std::array<int, body::kSlotCount> f = factors(chart, s, opt);
  const double p = kPi / 6.0;

  // his aa to bb walk reached the axes at 13 and 14, the present flags
  // already carry which slots the settings computed
  for (int t = 1; t < body::kSlotCount; ++t) {
    const BodyState& b = chart.b[static_cast<std::size_t>(t)];
    if (!b.present || !b.valid || skipped(t, s)) {
      continue;
    }
    const int score = opt.points[static_cast<std::size_t>(t)] * f[static_cast<std::size_t>(t)];
    for (int u = 0; u < 12; ++u) {
      double w1 = u * p;
      double w2 = w1 + p;
      //RR die kk-Kanten am Nullpunkt, exakt Null zählt nie
      if (u == 0) {
        w1 += kEps;
      }
      if (u == 11) {
        w2 -= kEps;
      }
      double w3 = b.el;
      if (w3 <= kEps) {
        continue;
      }
      vergl2(w1, w2, w3);
      if (w1 < w3 && w3 < w2) {
        out.element_sign[static_cast<std::size_t>(u % 4 + 1)] += score;
        out.quality_sign[static_cast<std::size_t>(u % 3 + 1)] += score;
      }
    }
  }

  //RR nur bei den Quadranten-Systemen, sein haw-Riegel
  if (!s.heliocentric && chart.houses.ok && s.houses < HouseSystem::kAcMcOnly) {
    out.houses_counted = true;
    std::array<double, 14> fz = chart.houses.cusp;
    fz[13] = fz[1];
    for (int t = 1; t < body::kSlotCount; ++t) {
      const BodyState& b = chart.b[static_cast<std::size_t>(t)];
      if (!b.present || !b.valid || skipped(t, s)) {
        continue;
      }
      const int score = opt.points[static_cast<std::size_t>(t)] * f[static_cast<std::size_t>(t)];
      for (int u = 1; u <= 12; ++u) {
        double w1 = fz[static_cast<std::size_t>(u)];
        double w2 = fz[static_cast<std::size_t>(u + 1)];
        double w3 = b.el;
        if (w3 <= kEps) {
          continue;
        }
        vergl2(w1, w2, w3);
        if (w1 < w3 && w3 < w2) {
          out.element_house[static_cast<std::size_t>((u - 1) % 4 + 1)] += score;
          out.quality_house[static_cast<std::size_t>((u - 1) % 3 + 1)] += score;
        }
      }
    }
  }
  return out;
}

}  // namespace horcom
