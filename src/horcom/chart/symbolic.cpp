// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/symbolic.hpp"

#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/mundane.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"

namespace horcom {

namespace {

// slots 15 to 18 carry the intermediate cusps two, three, five and six
// or the four cardinal points, depending on the extras choice
constexpr int kExtraFirst = 15;
constexpr int kExtraLast = 18;

double extra_value(const Chart& c, DirectionExtras extras, int slot) {
  if (extras == DirectionExtras::kCusps) {
    const int house = slot <= 16 ? slot - 13 : slot - 12;
    return c.houses.cusp[static_cast<std::size_t>(house)];
  }
  //RR Kardinalpunkte
  return (slot - kExtraFirst) * kPi / 2.0 + (slot == kExtraFirst ? kEps : 0.0);
}

// the original dif_vg, a branch fixed difference
double dif_vg(double a1, double a2) {
  double w1 = a1;
  double w2 = a2;
  vergl1(w1, w2);
  return w2 - w1;
}

struct Run {
  const Chart& chart;
  DirectionMethod method;
  const DirectionRange& range;
  double lat_deg;
  double armcb;
  std::vector<DirectionHit> hits;
  // the duplicate cube of the original, one flag per multiple and pair
  std::vector<bool> seen;
  int d1 = 0;

  [[nodiscard]] bool seen_at(int w, int t, int u) const {
    return seen[static_cast<std::size_t>((w * body::kSlotCount + t) * body::kSlotCount + u)];
  }
  void mark(int w, int t, int u) {
    seen[static_cast<std::size_t>((w * body::kSlotCount + t) * body::kSlotCount + u)] = true;
  }

  [[nodiscard]] bool usable(int slot) const {
    if (slot == body::kNodeDesc) {
      // the south node mirrors the north, the original leaves it out
      return false;
    }
    if (slot >= kExtraFirst && slot <= kExtraLast) {
      return range.extras != DirectionExtras::kNone &&
             (range.extras != DirectionExtras::kCusps || chart.houses.cusp[2] > 0.0);
    }
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    return b.present && b.valid;
  }

  // one arc through the sign rule and the age filter, his aprimout
  void take(int t, int u, int w, double dib, bool symbolic_frame) {
    while (dib > kTwoPi) {
      dib -= kTwoPi;
    }
    while (dib < -kTwoPi) {
      dib += kTwoPi;
    }
    if (dib > kPi && dib < kTwoPi) {
      dib = -std::abs(kTwoPi - dib);
    }
    if (dib < -kPi && dib > -kTwoPi) {
      dib = kTwoPi + dib;
    }
    // the sign of the arc says direct or converse, the symbolic frames
    // read it the other way round than the primary
    bool converse = false;
    if (dib >= 0.0 && dib <= kPi) {
      converse = symbolic_frame;
    } else if (dib < 0.0 && dib > -kPi) {
      converse = !symbolic_frame;
    }
    const double a = std::abs(dib);
    const double b = a * kRadToDeg;
    const double years = kRadToDeg * range.key * a;
    if (range.from_years < b && b <= range.to_years && years > 0.0 && !seen_at(w, t, u) && !(w == 0 && u == t) &&
        years < 180.0) {
      if (symbolic_frame && (seen_at(w, u, t) || seen_at(d1 - w, u, t))) {
        // a symbolic arc between two points reads the same from both
        // ends, the mirror stays silent
        mark(w, t, u);
        return;
      }
      mark(w, t, u);
      hits.push_back({t, u, w, b, years, converse});
    }
  }
};

// one position in the chosen symbolic frame
double frame_value(const Run& r, int slot) {
  double la = 0.0;
  double br = 0.0;
  if (slot >= kExtraFirst && slot <= kExtraLast) {
    la = extra_value(r.chart, r.range.extras, slot);
  } else {
    la = r.chart.b[static_cast<std::size_t>(slot)].el;
    br = r.chart.b[static_cast<std::size_t>(slot)].eb;
  }
  switch (r.method) {
    case DirectionMethod::kSymbolicEquatorial: {
      const Equatorial eq = ecliptic_to_equatorial(la, br, r.chart.smo.ekls);
      return eq.ra;
    }
    case DirectionMethod::kSymbolicMundane:
      return mundane_longitude(la, br, r.chart.smo.ekls, r.armcb, r.lat_deg);
    default:
      return la;
  }
}

// ported from asymb1 and asy111, the pair walk of the symbolic frames
void run_symbolic(Run& r) {
  std::array<double, body::kSlotCount> elp{};
  for (int t = 0; t < body::kSlotCount; ++t) {
    if (r.usable(t)) {
      elp[static_cast<std::size_t>(t)] = frame_value(r, t);
    }
  }
  const double w4d = r.range.base_angle_deg * kDegToRad;
  for (int t = 0; t < body::kSlotCount; ++t) {
    if (!r.usable(t) || t == body::kNodeDesc) {
      continue;
    }
    const double w2 = elp[static_cast<std::size_t>(t)];
    for (int u = 0; u < body::kSlotCount; ++u) {
      if (!r.usable(u) || u == t || u == body::kNodeDesc) {
        continue;
      }
      if (t >= kExtraFirst && t <= kExtraLast && u >= kExtraFirst && u <= kExtraLast) {
        continue;
      }
      for (int w = 0; w < r.d1; ++w) {
        // the cardinal points answer only the conjunction
        if (r.range.extras == DirectionExtras::kCardinal && w > 0 && u >= kExtraFirst && u <= kExtraLast) {
          continue;
        }
        const double f = w * w4d;
        if (elp[static_cast<std::size_t>(u)] == 0.0 && !(u >= kExtraFirst && u <= kExtraLast)) {
          continue;
        }
        const double w1 = norm_rad(elp[static_cast<std::size_t>(u)] + f);
        if (w1 < kEps || w2 < kEps) {
          continue;
        }
        // both readings of the same arc, forward and backward
        double a1 = w1;
        double a2 = w2;
        vergl1(a1, a2);
        r.take(t, u, w, a2 - a1, true);
        a1 = w1;
        a2 = w2;
        vergl1r(a1, a2);
        r.take(t, u, w, a2 - a1, true);
      }
    }
  }
}

// ported from aprim1, the Kühr primary walk in oblique ascension
void run_primary(Run& r) {
  const double w4d = r.range.base_angle_deg * kDegToRad;
  //RR mit Breite
  const double brep = r.range.with_latitude ? 1.0 : 0.0;
  for (int t = 0; t < body::kSlotCount; ++t) {
    if (!r.usable(t) || t == body::kNodeDesc) {
      continue;
    }
    double la = 0.0;
    double br = 0.0;
    if (t >= kExtraFirst && t <= kExtraLast) {
      la = extra_value(r.chart, r.range.extras, t);
    } else {
      la = r.chart.b[static_cast<std::size_t>(t)].el;
      br = r.chart.b[static_cast<std::size_t>(t)].eb;
    }
    if (la <= kEps) {
      continue;
    }
    const Equatorial sig = ecliptic_to_equatorial(la, br, r.chart.smo.ekls);
    const SemiArcPoint arc = semi_arc_point(sig.ra, sig.dec, r.armcb, r.lat_deg);
    for (int u = 0; u < body::kSlotCount; ++u) {
      if (!r.usable(u) || u == t || u == body::kNodeDesc) {
        continue;
      }
      if (t >= kExtraFirst && t <= kExtraLast && u >= kExtraFirst && u <= kExtraLast) {
        continue;
      }
      // the pole's arctangent slips outside its domain for extreme
      // declinations, the last good value carries on like the original
      double ad = 0.0;
      for (int w = 0; w < r.d1; ++w) {
        const double f = w * w4d;
        double pla = 0.0;
        double pbr = 0.0;
        if (u >= kExtraFirst && u <= kExtraLast) {
          if (r.range.extras != DirectionExtras::kCusps) {
            continue;
          }
          pla = norm_rad(extra_value(r.chart, r.range.extras, u) + f);
        } else {
          pla = norm_rad(r.chart.b[static_cast<std::size_t>(u)].el + f);
          //RR Breite des Aspekt-Punktes klingt mit dem Winkel ab
          pbr = std::asin(std::sin(brep * r.chart.b[static_cast<std::size_t>(u)].eb) * std::sin(kPi / 2.0 - f));
        }
        if (pla <= kEps) {
          continue;
        }
        const Equatorial prom = ecliptic_to_equatorial(pla, pbr, r.chart.smo.ekls);
        const double pr = std::tan(arc.pole) * std::tan(prom.dec);
        if (pr >= -1.0 && pr <= 1.0) {
          ad = std::asin(pr);
        }
        double dib = 0.0;
        if (arc.east) {
          const double ao = dif_vg(ad, prom.ra);
          dib = dif_vg(arc.oblique, ao);
        } else {
          const double dod = dif_vg(-ad, prom.ra);
          dib = dif_vg(arc.oblique, dod);
        }
        r.take(t, u, w, dib, false);
      }
    }
  }
}

}  // namespace

// ported from asymb, ar_sys and aprim
std::vector<DirectionHit> direction_hits(const Chart& chart, DirectionMethod method, const DirectionRange& range, double lat_deg) {
  Run r{chart, method, range, lat_deg, chart.armc_deg * kDegToRad, {}, {}, 0};
  r.d1 = static_cast<int>(360.0 / range.base_angle_deg);
  r.seen.assign(static_cast<std::size_t>((r.d1 + 1) * body::kSlotCount * body::kSlotCount), false);
  if (method == DirectionMethod::kPrimary) {
    run_primary(r);
  } else {
    run_symbolic(r);
  }
  return r.hits;
}

}  // namespace horcom
