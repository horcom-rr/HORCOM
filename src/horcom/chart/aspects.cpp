// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/aspects.hpp"

#include <algorithm>

#include <cmath>
#include <memory>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the asp matrix holds m times two pi over n, far above this rounding
constexpr double kExactMultipleTolerance = 1.0e-6;

// the base orb of the mirror points, his spieg1 orb*2 and the orbe(13)
// preset both rest on two degrees
constexpr double kMirrorOrbDeg = 2.0;

// the HALBSUMMEN-GRAPHIK puts its cusp trees H2, H3, H5 and H6 on the
// slots 15 to 18, where the cardinal points stand elsewhere
constexpr int kCuspTreeFirst = 15;
constexpr int kCuspTreeLast = 18;

// the original a18st for the geocentric scan, jumps the gap between the
// angles and the extra bodies and skips bodies outside their ephemeris
int next_slot(const Chart& chart, const ChartSettings& s, int slot, int np) {
  if (s.extra_bodies) {
    if (slot == 15) {
      slot = 19;
    }
    // his a18st stepped over one invalid extra, the fixed slot layout also
    // leaves the extras nobody chose empty between the chosen ones
    while (slot >= 19 && slot <= np &&
           !(chart.b[static_cast<std::size_t>(slot)].present && chart.b[static_cast<std::size_t>(slot)].valid)) {
      ++slot;
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

bool live(const Chart& chart, int slot) {
  const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
  return b.present && b.valid;
}

bool node_pair(int t, int w) {
  return (t == 11 && w == 12) || (t == 12 && w == 11);
}

// his aa&, lpkt and fixpunkt_def set it to zero when the fixed point or
// the Sonderpunkt of the Rhythmenlehre stands on slot zero
int first_slot(const Chart& chart) {
  return live(chart, body::kFixpunkt) ? body::kFixpunkt : body::kSun;
}

}  // namespace

// ported from HORCOM org
double org(const AspectSettings& a, int slot, int nh) {
  return a.weight[static_cast<std::size_t>(slot)] / (nh * kPercent);
}

double divisor_orb(const AspectSettings& a, int n) {
  // the orbe row ends at index 14, the equal probability scans stop at
  // twelve before they could read past it
  const std::size_t row = static_cast<std::size_t>(std::clamp(n, 0, static_cast<int>(a.orbe.size()) - 1));
  return a.equal_probability ? a.orb * a.orbe[row] : a.orb * (kTwoPi / n) / kDefaultOrbDivisor;
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
  orbe[13] = kDegToRad * kMirrorOrbDeg;
  orbe[14] = kDegToRad * 1.0;
}

AspectResult scan_aspects(const Chart& chart, const ChartSettings& s, const AspectSettings& a) {
  AspectResult out;
  const std::array<double, body::kSlotCount> as = positions(chart);
  const int np = s.body_count();
  const int bb = s.extra_bodies ? np : 14;
  // (haw& = 9 OR haw& = 10) && hrg! = 0, NUR AC und MC keeps its angles
  const bool no_angle_aspects = without_angles(s.houses) && !s.heliocentric;
  // (t& = 11 OR t& = 12 OR w& = 11 OR w& = 12) && haw& = 10 && hrg! = 0
  const bool no_node_aspects = s.houses == HouseSystem::kNoneNoNodes && !s.heliocentric;

  std::vector<std::pair<int, int>> conj;   // the original tkonj/wkonj
  std::vector<std::pair<int, int>> trine;  // the original tgrt/wgrt

  // IF orbe! && nasp& = 16 : nasp& = 12, the orbe row ends at twelve
  const int nas = a.equal_probability ? std::min(a.divisors, kMaxEqualOrbDivisor) : a.divisors;
  // the fixed point or the Sonderpunkt of the Rhythmenlehre on slot zero
  // joins both scans
  const int first = first_slot(chart);
  for (int n = 1; n <= nas; ++n) {
    const double pn = kTwoPi / n;
    const double dd = divisor_orb(a, n);
    for (int t = first; t <= bb - 1; ++t) {
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
        // his pl = 0 stood for an empty slot, a body at exactly 0 Aries, the
        // fixed point typed as 0 Aries or a dial position folded onto
        // zero never aspected
        if (!live(chart, t) || !live(chart, w)) {
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
          // his w3 > 0 dropped the exact conjunction of two equal places
          const double w3 = std::abs(wa1 - wa2);
          if (w3 < dds || w3 > kTwoPi - dds) {
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

  // spieg1, the mirror points, a pair mirrors when the two longitudes
  // sum to PI about the solstice axis or to 2 PI about the equinox axis,
  // the base orb is two degrees times his orb factor, the south node
  // slot 12 stays out as it only mirrors its own head
  const double dd_m = a.orb * kMirrorOrbDeg * kDegToRad;
  for (int t = first; t <= bb - 1; ++t) {
    t = next_slot(chart, s, t, np);
    if (t > bb - 1) {
      break;
    }
    if (!live(chart, t) || t == 12) {
      continue;
    }
    const double o1 = org(a, t, 1);
    const double at = as[static_cast<std::size_t>(t)];
    for (int w = t + 1; w <= bb; ++w) {
      w = next_slot(chart, s, w, np);
      if (w > bb) {
        break;
      }
      if (w == 12 || !live(chart, w)) {
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

namespace {

using DrkCube = std::array<std::array<std::array<bool, body::kSlotCount>, body::kSlotCount>, body::kSlotCount>;

// the original halbs111 for the point t on the level nh, the hits land in
// hits. partners, when given, silences the pairs his asp_wahl leaves out
void midpoint_pass(const Chart& chart, const ChartSettings& s, const AspectSettings& a,
                   const std::array<double, body::kSlotCount>& as, int np, int bb, int t, int nh, DrkCube& drk,
                   const std::array<bool, body::kSlotCount>* partners, std::vector<MidpointHit>& hits) {
  const double dd = a.equal_probability ? a.orb * a.orbe[14] : a.orb * kDegToRad;
  const double o1 = org(a, t, nh);
  const auto chosen = [partners](int slot) { return partners == nullptr || (*partners)[static_cast<std::size_t>(slot)]; };
  // u runs from aa& like t, the fixed point pairs as well
  for (int u = first_slot(chart); u <= bb - 1; ++u) {
    u = next_slot(chart, s, u, np);
    if (u > bb - 1) {
      break;
    }
    const double o2 = org(a, u, nh);
    if (!chosen(u)) {
      continue;
    }
    for (int w = u + 1; w <= bb; ++w) {
      w = next_slot(chart, s, w, np);
      if (w > bb) {
        break;
      }
      const double o3 = org(a, w, nh);
      const double dds = orbis_discr3(o1, o2, o3, dd);
      const double c1 = (nh == 1) ? 0.0 : kPi / nh;
      if (w == t || w == u || u == t || !chosen(w)) {
        continue;
      }
      // a slot without a position would count as zero Aries
      if (!live(chart, u) || !live(chart, w)) {
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
          auto& mark = drk[static_cast<std::size_t>(w)][static_cast<std::size_t>(t)][static_cast<std::size_t>(u)];
          const auto& mark_a = drk[static_cast<std::size_t>(w)][static_cast<std::size_t>(t)][11];
          const auto& mark_b = drk[11][static_cast<std::size_t>(t)][static_cast<std::size_t>(u)];
          const auto& mark_c = drk[12][static_cast<std::size_t>(t)][static_cast<std::size_t>(u)];
          const auto& mark_d = drk[static_cast<std::size_t>(w)][static_cast<std::size_t>(u)][12];
          if (w1 < w3 && w3 < w2 && !mark) {
            // the original node duplicate condition kept literally
            if (!((mark_a && u == 12) || (mark_b && w == 12) || (mark_c && w == 11) || (mark_d && u == 12))) {
              mark = true;
              hits.push_back({t, u, w, nh});
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

}  // namespace

MidpointResult scan_midpoints(const Chart& chart, const ChartSettings& s, const AspectSettings& a, bool with_45) {
  MidpointResult out;
  const std::array<double, body::kSlotCount> as = positions(chart);
  const int np = s.body_count();
  const int bb = s.extra_bodies ? np : 14;
  // the drk! cube spans all three passes like the DIM in halbs1
  auto drk = std::make_unique<DrkCube>();

  // the midpoint tree screen adds the 45 degree level
  const int levels[4] = {1, 2, 4, 8};
  for (int li = 0; li < (with_45 ? 4 : 3); ++li) {
    const int nh = levels[li];
    // halbs11 runs t from aa& like the aspect scan
    for (int t = first_slot(chart); t <= bb; ++t) {
      t = next_slot(chart, s, t, np);
      if (t > bb) {
        break;
      }
      // a slot without a position would count as zero Aries
      if (!live(chart, t)) {
        continue;
      }
      const std::size_t before = out.hits.size();
      midpoint_pass(chart, s, a, as, np, bb, t, nh, *drk, nullptr, out.hits);
      const int found = static_cast<int>(out.hits.size() - before);
      switch (nh) {
        case 1: out.direct += found; break;
        case 2: out.square += found; break;
        case 4: out.semi += found; break;
        default: break;
      }
    }
  }
  return out;
}

AspectSettings tree_orb_settings(AspectSettings a) {
  for (int slot = kCuspTreeFirst; slot <= kCuspTreeLast; ++slot) {
    a.weight[static_cast<std::size_t>(slot)] = static_cast<int>(kPercent);
  }
  return a;
}

// ported from aspar2, the trees of the HALBSUMMEN-GRAPHIK. Every point
// runs the four levels in turn on one shared drk! cube, the cusps two,
// three, five and six stand on the slots 15 to 18 like his pl(t&) there
std::vector<MidpointTree> midpoint_trees(const Chart& chart, const ChartSettings& s, const AspectSettings& orbs,
                                         const std::array<bool, body::kSlotCount>& partners) {
  const AspectSettings a = tree_orb_settings(orbs);
  std::vector<MidpointTree> out;
  std::array<double, body::kSlotCount> as = positions(chart);
  const bool cusps = !s.heliocentric && chart.houses.ok;
  // his pl(t&) = fz(od,ze,t& - 13) for 15 and 16, fz(od,ze,t& - 12) for 17 and 18
  static constexpr int kCuspOfSlot[4] = {2, 3, 5, 6};
  if (cusps) {
    for (int i = 0; i < 4; ++i) {
      as[static_cast<std::size_t>(kCuspTreeFirst + i)] = chart.houses.cusp[static_cast<std::size_t>(kCuspOfSlot[i])];
    }
  }
  const int np = s.body_count();
  const int bb = s.extra_bodies ? np : 14;
  auto drk = std::make_unique<DrkCube>();
  for (int t = 0; t < body::kSlotCount; ++t) {
    MidpointTree tree;
    tree.slot = t;
    if (t >= kCuspTreeFirst && t <= kCuspTreeLast) {
      if (!cusps) {
        continue;
      }
      tree.cusp = kCuspOfSlot[t - kCuspTreeFirst];
    } else if (!live(chart, t)) {
      continue;
    }
    // IF NOT(hrg! && ((t& > 10 && t& < 19) OR t& = n1& OR t& = n4&))
    if (s.heliocentric && (t == body::kSun || (t > body::kPluto && t < body::kApogee) || t == body::kApogee ||
                           t == body::kFortune)) {
      continue;
    }
    tree.lon = as[static_cast<std::size_t>(t)];
    for (const int nh : {1, 2, 4, 8}) {
      midpoint_pass(chart, s, a, as, np, bb, t, nh, *drk, &partners, tree.hits);
    }
    out.push_back(std::move(tree));
  }
  return out;
}

// the orb of the MULTI comparisons, a fifth of a degree
constexpr double kMultiOrbDeg = 0.2;

// ported from a12asp
std::vector<CrossAspectHit> scan_aspects_between(const Chart& first, const Chart& second, const AspectSettings& a, bool transit_orbs) {
  CrossScanOptions opt;
  opt.orbs = transit_orbs ? CrossOrbs::kTransit : CrossOrbs::kNormal;
  opt.extras = true;
  return scan_aspects_between(first, second, a, opt);
}

std::vector<CrossAspectHit> scan_aspects_between(const Chart& first, const Chart& second, const AspectSettings& a,
                                                 const CrossScanOptions& opt) {
  std::vector<CrossAspectHit> out;
  const Chart& other = opt.within ? first : second;
  // NOT (t& = 12 OR w& = 12), IF NOT (mult! && t& = nk&(3)), and the
  // hrg! guard of the display
  const auto active = [&opt](const Chart& c, int slot) {
    const BodyState& b = c.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid || slot == body::kNodeDesc) {
      return false;
    }
    // harm sets mult! as well, both leave Transpluto out
    if ((opt.orbs == CrossOrbs::kMulti || opt.orbs == CrossOrbs::kHarmonic) && slot == body::kTranspluto) {
      return false;
    }
    if (opt.heliocentric && (slot == body::kNodeAsc || slot == body::kApogee || slot == body::kFortune)) {
      return false;
    }
    return true;
  };
  // the first chart from aa&, the fixed point included, to bb& or np&,
  // the second from 1 to np&, twelve without the extras
  const int t_end = opt.extras ? body::kSlotCount - 1 : body::kMc;
  const int w_end = opt.extras ? body::kSlotCount - 1 : body::kNodeDesc;
  for (int t = 0; t <= t_end; ++t) {
    if ((t > body::kMc && t < body::kApogee) || !active(first, t)) {
      continue;
    }
    const double wa1 = norm_rad(first.b[static_cast<std::size_t>(t)].el);
    // an& = t& + 1 under aspmult!
    for (int w = opt.within ? t + 1 : 1; w <= w_end; ++w) {
      if ((w > body::kMc && w < body::kApogee) || !active(other, w)) {
        continue;
      }
      const double wa2 = norm_rad(other.b[static_cast<std::size_t>(w)].el);
      int n = 0;
      while (n != 6) {
        ++n;
        // the fifth harmonic stays out of every comparison
        if (n == 5) {
          ++n;
        }
        const double pn = kTwoPi / n;
        const double dd = a.orb * pn / kDefaultOrbDivisor;
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
          switch (opt.orbs) {
            case CrossOrbs::kTransit:
              //RR 1 Grad
              dds = o1 * kDegToRad;
              break;
            case CrossOrbs::kMulti:
              //RR 0.2 Grad
              dds = a.orb * kMultiOrbDeg * kDegToRad;
              break;
            case CrossOrbs::kHarmonic:
              dds = a.orb * kDegToRad;
              break;
            default:
              break;
          }
          double w2 = norm_rad(std::abs(wa2 - wa1));
          if (w2 < dds) {
            w2 = kTwoPi - w2;
          }
          double w1 = pnm;
          vergl1(w1, w2);
          const double diff = std::abs(w2 - w1);
          // his w > kk, wa1 > kk and wa2 > kk took a zero for an empty
          // slot, so an exact aspect and every body at 0 Aries dropped out,
          // a chart laid over itself lost all its conjunctions. The slot
          // tells presence in the port like in scan_aspects
          if (diff < dds && w2 > dds) {
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

// ported from HORCOM aspdis. His loop divided the angle by t and matched
// the FIX truncated degrees 360, 180, ... 22 of the known families, so
// five elevenths, 163.6 degrees, truncated to 40 at t = 4 and named the
// ninth. Every angle of the asp matrix is an exact multiple of its
// divisor, the port reads the family from the reduced fraction
int aspect_symbol(double w, int divisors) {
  if (w > kPi && w < kTwoPi) {
    w = kTwoPi - w;
  }
  for (int n = 1; n <= divisors; ++n) {
    const double k = w * n / kTwoPi;
    const long m = std::lround(k);
    if (std::abs(k - static_cast<double>(m)) > kExactMultipleTolerance) {
      continue;
    }
    // the smallest divisor holding the angle leaves the fraction reduced
    if (n == 5 && m == 2) {
      return 17;  //RR Biquintil
    }
    if (n == 12 && m == 5) {
      return 18;  //RR Quinkunx
    }
    if (n == 8 && m == 3) {
      return 19;  //RR Anderthalbquad.
    }
    return n;
  }
  return 0;
}

}  // namespace horcom
