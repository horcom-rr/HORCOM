// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/aspects.hpp"

#include <cmath>
#include <memory>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the default window is the base angle over thirty, his pn / 30
constexpr double kDefaultOrbDivisor = 30.0;

// the original a18st for the geocentric scan, jumps the gap between the
// angles and the extra bodies and skips bodies outside their ephemeris
int next_slot(const Chart& chart, const ChartSettings& s, int slot, int np) {
  if (s.extra_bodies) {
    if (slot == 15) {
      slot = 19;
    }
    if (slot >= 19 && slot <= np && chart.b[static_cast<std::size_t>(slot)].present &&
        !chart.b[static_cast<std::size_t>(slot)].valid) {
      ++slot;
      if (slot > np) {
        slot = np;
      }
    }
  }
  return slot;
}

// the positions array of the original asp10, AC and MC live on 13 and 14
std::array<double, body::kSlotCount> positions(const Chart& chart) {
  std::array<double, body::kSlotCount> as{};
  for (int t = 0; t < body::kSlotCount; ++t) {
    const BodyState& b = chart.b[static_cast<std::size_t>(t)];
    as[static_cast<std::size_t>(t)] = (b.present && b.valid) ? b.el : 0.0;
  }
  return as;
}

bool node_pair(int t, int w) {
  return (t == 11 && w == 12) || (t == 12 && w == 11);
}

}  // namespace

// ported from HORCOM org
double org(const AspectSettings& a, int slot, int nh) {
  return a.weight[static_cast<std::size_t>(slot)] / (nh * kPercent);
}

// ported from HORCOM orbis_discr2
double orbis_discr2(double o1, double o2, double dd) {
  if (o1 > 0.0 && o2 > 0.0) {
    return std::max(o1, o2) * dd;
  }
  return 0.0;
}

// ported from HORCOM orbis_discr3
double orbis_discr3(double o1, double o2, double o3, double dd) {
  if (o1 > 0.0 && o2 > 0.0 && o3 > 0.0) {
    return std::max(o1, o2) * dd;
  }
  return 0.0;
}

// the shared table of asp1 and bed_erf_asp1
bool multiple_allowed(int n, int m) {
  switch (n) {
    case 2: return true;
    case 3: return m == 1 || m == 2;
    case 4: return m == 1 || m == 3;
    case 5: return m >= 1 && m <= 4;
    case 6: return m == 1 || m == 5;
    case 7: return m >= 1 && m <= 6;
    case 8: return m == 1 || m == 3 || m == 5 || m == 7;
    case 9: return m == 1 || m == 2 || m == 4 || m == 5 || m == 7 || m == 8;
    case 10: return m == 1 || m == 3 || m == 7 || m == 9;
    case 11: return m >= 1 && m <= 10;
    case 12: return m == 1 || m == 5 || m == 7 || m == 11;
    default: return false;
  }
}

void AspectSettings::preset_equal_orbs() {
  //RR Grundwinkel
  for (int i = 1; i <= 12; ++i) {
    orbe[static_cast<std::size_t>(i)] = kDegToRad * 12.0 / i;
  }
  orbe[13] = kDegToRad * 2.0;
  orbe[14] = kDegToRad * 1.0;
}

AspectResult scan_aspects(const Chart& chart, const ChartSettings& s, const AspectSettings& a) {
  AspectResult out;
  const std::array<double, body::kSlotCount> as = positions(chart);
  const int np = s.body_count();
  const int bb = s.extra_bodies ? np : 14;
  const bool no_angle_aspects = s.houses == HouseSystem::kAcMcOnly || s.houses == HouseSystem::kNone;
  const bool no_node_aspects = false;  // the original haw& = 10 mode is not ported yet

  std::vector<std::pair<int, int>> conj;   // the original tkonj/wkonj
  std::vector<std::pair<int, int>> trine;  // the original tgrt/wgrt

  const int nas = a.divisors;
  for (int n = 1; n <= nas; ++n) {
    const double pn = kTwoPi / n;
    const double dd = a.equal_probability ? a.orb * a.orbe[static_cast<std::size_t>(n)] : a.orb * pn / kDefaultOrbDivisor;
    for (int t = 1; t <= bb - 1; ++t) {
      t = next_slot(chart, s, t, np);
      if (t > bb - 1) {
        break;
      }
      const double o1 = org(a, t, 1);
      const double wa1 = norm_rad(as[static_cast<std::size_t>(t)] + kPi);
      for (int w = t + 1; w <= bb; ++w) {
        w = next_slot(chart, s, w, np);
        if (w > bb) {
          break;
        }
        const double o2 = org(a, w, 1);
        const double dds = orbis_discr2(o1, o2, dd);
        const double wa2 = norm_rad(as[static_cast<std::size_t>(w)] + kPi);
        if ((t == 13 || t == 14 || w == 13 || w == 14) && no_angle_aspects) {
          continue;
        }
        if (as[static_cast<std::size_t>(t)] == 0.0 || as[static_cast<std::size_t>(w)] == 0.0) {
          continue;
        }
        auto& cell = out.asp[static_cast<std::size_t>(t)][static_cast<std::size_t>(w)];
        if (cell != 0.0 || node_pair(t, w)) {
          continue;
        }
        if ((t == 11 || t == 12 || w == 11 || w == 12) && no_node_aspects) {
          continue;
        }
        if (n == 1) {
          const double w3 = std::abs(wa1 - wa2);
          if (w3 > 0.0 && (w3 < dds || w3 > kTwoPi - dds)) {
            cell = kTwoPi;
            ++out.zh[1];
            ++out.az[static_cast<std::size_t>(t)];
            ++out.az[static_cast<std::size_t>(w)];
            conj.emplace_back(t, w);
            out.hits.push_back({t, w, 1, 1});
          }
        } else {
          for (int m = 1; m <= n - 1; ++m) {
            if (a.equal_probability && !multiple_allowed(n, m)) {
              continue;
            }
            // the original asp11
            double w1 = norm_rad(m * pn - dds);
            double w2 = norm_rad(m * pn + dds);
            double w3 = std::abs(as[static_cast<std::size_t>(t)] - as[static_cast<std::size_t>(w)]);
            vergl2(w1, w2, w3);
            if (w1 < w3 && w3 < w2) {
              cell = m * pn;
              ++out.zh[static_cast<std::size_t>(n)];
              if (t != w && m * pn < kTwoPi) {
                ++out.az[static_cast<std::size_t>(t)];
                ++out.az[static_cast<std::size_t>(w)];
              }
              if (n == 3) {
                //RR Schiemenz
                trine.emplace_back(t, w);
              }
              out.hits.push_back({t, w, n, m});
            }
          }
        }
      }
    }
  }

  //RR Schiemenz Triga-Zähler:
  if (conj.size() >= 2) {
    for (std::size_t i = 0; i + 1 < conj.size(); ++i) {
      for (std::size_t j = i + 1; j < conj.size(); ++j) {
        const int ti = conj[i].first;
        const int tj = conj[j].first;
        const int wi = conj[i].second;
        const int wj = conj[j].second;
        if (ti > 0 && tj > 0 && wi > 0 && wj > 0) {
          if (ti == wj || ti == tj || tj == wi || wi == wj) {
            ++out.triga;
            if (ti == wj) {
              conj[i].first = 0;
              conj[j].second = 0;
            }
            if (ti == tj) {
              conj[i].first = 0;
              conj[j].first = 0;
            }
            if (tj == wi) {
              conj[j].first = 0;
              conj[i].second = 0;
            }
            if (wi == wj) {
              conj[i].second = 0;
              conj[j].second = 0;
            }
          }
        }
      }
    }
  }
  //RR Schiemenz Großtrigon-Zähler:
  if (trine.size() >= 3) {
    for (std::size_t i = 0; i + 2 < trine.size(); ++i) {
      for (std::size_t j = i + 1; j + 1 < trine.size(); ++j) {
        for (std::size_t k = j + 1; k < trine.size(); ++k) {
          const int ti = trine[i].first;
          const int tj = trine[j].first;
          const int tk = trine[k].first;
          const int wi = trine[i].second;
          const int wj = trine[j].second;
          const int wk = trine[k].second;
          if (ti > 0 && tj > 0 && tk > 0 && wi > 0 && wj > 0 && wk > 0) {
            if (ti == tj || ti == wj || ti == wk) {
              if (tk == wi || tk == wj || tk == tj) {
                if (wj == wk || tj == wk || tj == wi || wi == wj) {
                  const auto pl = [&](int x) { return as[static_cast<std::size_t>(x)]; };
                  if (!(norm_rad(pl(ti) - pl(wi)) < 1.0 && norm_rad(pl(tj) - pl(wj)) < 1.0 && norm_rad(pl(tk) - pl(wk)) < 1.0)) {
                    ++out.grand_trines;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  //RR spieg1, the mirror points, a pair mirrors when the two longitudes
  // sum to PI about the solstice axis or to 2 PI about the equinox axis,
  // the base orb is two degrees times his orb factor, the south node
  // slot 12 stays out as it only mirrors its own head
  const double dd_m = a.orb * 2.0 * kDegToRad;
  for (int t = 1; t <= bb - 1; ++t) {
    t = next_slot(chart, s, t, np);
    if (t > bb - 1) {
      break;
    }
    if (as[static_cast<std::size_t>(t)] == 0.0 || t == 12) {
      continue;
    }
    const double o1 = org(a, t, 1);
    const double at = as[static_cast<std::size_t>(t)];
    for (int w = t + 1; w <= bb; ++w) {
      w = next_slot(chart, s, w, np);
      if (w > bb) {
        break;
      }
      if (w == 12 || as[static_cast<std::size_t>(w)] == 0.0) {
        continue;
      }
      const double o2 = org(a, w, 1);
      const double dds = orbis_discr2(o1, o2, dd_m);
      const double au = as[static_cast<std::size_t>(w)];
      const double w11 = norm_rad(kPi - dds - au);
      const double w12 = norm_rad(kPi + dds - au);
      const double w21 = norm_rad(kTwoPi - dds - au);
      const double w22 = norm_rad(kTwoPi + dds - au);
      if ((at > w11 && at < w12) || (at > w21 && at < w22) || (at > kTwoPi - dds && w22 < dds) ||
          (at < dds && w22 > kTwoPi - dds)) {
        out.mirrors.emplace_back(t, w);
      }
    }
  }
  return out;
}

MidpointResult scan_midpoints(const Chart& chart, const ChartSettings& s, const AspectSettings& a, bool with_45) {
  MidpointResult out;
  const std::array<double, body::kSlotCount> as = positions(chart);
  const int np = s.body_count();
  const int bb = s.extra_bodies ? np : 14;
  // the drk! cube spans all three passes like the DIM in halbs1
  auto drk = std::make_unique<std::array<std::array<std::array<bool, body::kSlotCount>, body::kSlotCount>, body::kSlotCount>>();

  // the midpoint tree screen adds the 45 degree level
  const int levels[4] = {1, 2, 4, 8};
  for (int li = 0; li < (with_45 ? 4 : 3); ++li) {
    const int nh = levels[li];
    const double dd = a.equal_probability ? a.orb * a.orbe[14] : a.orb * kDegToRad;
    for (int t = 1; t <= bb; ++t) {
      t = next_slot(chart, s, t, np);
      if (t > bb) {
        break;
      }
      const double o1 = org(a, t, nh);
      // the original halbs111
      for (int u = 1; u <= bb - 1; ++u) {
        u = next_slot(chart, s, u, np);
        if (u > bb - 1) {
          break;
        }
        const double o2 = org(a, u, nh);
        for (int w = u + 1; w <= bb; ++w) {
          w = next_slot(chart, s, w, np);
          if (w > bb) {
            break;
          }
          const double o3 = org(a, w, nh);
          const double dds = orbis_discr3(o1, o2, o3, dd);
          const double c1 = (nh == 1) ? 0.0 : kPi / nh;
          if (w == t || w == u || u == t) {
            continue;
          }
          if ((t == 11 && u == 12) || (t == 11 && w == 12) || (t == 12 && u == 11) || (t == 12 && w == 11) ||
              (w == 11 && u == 12) || (w == 12 && u == 11)) {
            continue;
          }
          for (int l = 1; ; ++l) {
            const double off = l * c1;
            double pl1 = norm_rad(as[static_cast<std::size_t>(t)] - dds + off);
            double pl2 = norm_rad(as[static_cast<std::size_t>(t)] + dds + off);
            const double ph = norm_rad((as[static_cast<std::size_t>(u)] + as[static_cast<std::size_t>(w)]) / 2.0);
            double aa = ph;
            for (int z = 1; z <= 2; ++z) {
              double w1 = pl1;
              double w2 = pl2;
              double w3 = aa;
              vergl2(w1, w2, w3);
              auto& mark = (*drk)[static_cast<std::size_t>(w)][static_cast<std::size_t>(t)][static_cast<std::size_t>(u)];
              const auto& mark_a = (*drk)[static_cast<std::size_t>(w)][static_cast<std::size_t>(t)][11];
              const auto& mark_b = (*drk)[11][static_cast<std::size_t>(t)][static_cast<std::size_t>(u)];
              const auto& mark_c = (*drk)[12][static_cast<std::size_t>(t)][static_cast<std::size_t>(u)];
              const auto& mark_d = (*drk)[static_cast<std::size_t>(w)][static_cast<std::size_t>(u)][12];
              if (w1 < w3 && w3 < w2 && !mark) {
                // the original node duplicate condition kept literally
                if (!((mark_a && u == 12) || (mark_b && w == 12) || (mark_c && w == 11) || (mark_d && u == 12))) {
                  mark = true;
                  out.hits.push_back({t, u, w, nh});
                  switch (nh) {
                    case 1: ++out.direct; break;
                    case 2: ++out.square; break;
                    case 4: ++out.semi; break;
                    default: break;
                  }
                }
              }
              if (!(t == 11 || t == 12)) {
                aa = norm_rad(aa + kPi);
              }
            }
            if (l >= nh - 1) {
              break;
            }
          }
        }
      }
    }
  }
  return out;
}

// ported from a12asp
std::vector<CrossAspectHit> scan_aspects_between(const Chart& first, const Chart& second, const AspectSettings& a, bool transit_orbs) {
  std::vector<CrossAspectHit> out;
  const auto active = [](const Chart& c, int slot) {
    const BodyState& b = c.b[static_cast<std::size_t>(slot)];
    return b.present && b.valid && slot != body::kNodeDesc;
  };
  for (int t = 1; t < body::kSlotCount; ++t) {
    if (!active(first, t)) {
      continue;
    }
    const double wa1 = norm_rad(first.b[static_cast<std::size_t>(t)].el);
    for (int w = 1; w < body::kSlotCount; ++w) {
      if (!active(second, w)) {
        continue;
      }
      const double wa2 = norm_rad(second.b[static_cast<std::size_t>(w)].el);
      int n = 0;
      while (n != 6) {
        ++n;
        // the fifth harmonic stays out of every comparison
        if (n == 5) {
          ++n;
        }
        const double pn = kTwoPi / n;
        const double dd = a.orb * pn / 30.0;
        const int m_end = std::max(1, n - 1);
        int m = 0;
        while (m != m_end) {
          ++m;
          if (n == 4 && m == 2) {
            ++m;
          }
          if (n == 6 && m == 2) {
            m = 5;
          }
          const double pnm = m * pn;
          const double o1 = org(a, t, 1);
          const double o2 = org(a, w, 1);
          double dds = orbis_discr2(o1, o2, dd);
          if (transit_orbs) {
            //RR 1 Grad
            dds = o1 * kDegToRad;
          }
          double w2 = norm_rad(std::abs(wa2 - wa1));
          if (w2 < dds) {
            w2 = kTwoPi - w2;
          }
          double w1 = pnm;
          vergl1(w1, w2);
          const double diff = std::abs(w2 - w1);
          if (diff > kEps && wa1 > kEps && wa2 > kEps && diff < dds && w2 > dds) {
            CrossAspectHit h;
            h.t = t;
            h.w = w;
            h.n = n;
            h.m = m;
            double sep = std::abs(wa2 - wa1);
            if (sep < dds) {
              sep = kTwoPi - sep;
            }
            if (sep > kPi) {
              sep = kTwoPi - sep;
            }
            h.sep_deg = sep * kRadToDeg;
            out.push_back(h);
          }
        }
      }
    }
  }
  return out;
}

// ported from HORCOM aspdis
int aspect_symbol(double w, int divisors) {
  if (w > kPi && w < kTwoPi) {
    w = kTwoPi - w;
  }
  for (int t = 1; t <= divisors; ++t) {
    const int n = static_cast<int>(0.001 + kRadToDeg * w / t);
    int m = 0;
    switch (n) {
      case 360:
      case 180:
      case 120:
      case 90:
      case 72:
      case 60:
      case 51:
      case 45:
      case 40:
      case 36:
      case 32:
      case 30:
      case 27:
      case 25:
      case 24:
      case 22:
        m = static_cast<int>(360.0 / n + 0.001);
        break;
      case 144:
        m = 17;  //RR Biquintil
        break;
      case 150:
        m = 18;  //RR Quinkunx
        break;
      case 135:
        m = 19;  //RR Anderthalbquad.
        break;
      default:
        break;
    }
    if (m > 0) {
      return m;
    }
  }
  return 0;
}

}  // namespace horcom
