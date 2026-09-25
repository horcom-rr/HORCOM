// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/symbolic.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <tuple>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/composite.hpp"
#include "horcom/chart/houses.hpp"
#include "horcom/chart/mundane.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"

namespace horcom {

namespace {

using body::cardinal;
// the intermediate cusps of zwihat and zwihau
constexpr std::array<int, 4> kIntermediateCusps = {2, 3, 5, 6};
// a cusp gets its own id in the duplicate memory, beyond every slot
constexpr int kCuspId = 100;

// the original dif_vg, a branch fixed difference
double dif_vg(double a1, double a2) {
  double w1 = a1;
  double w2 = a2;
  vergl1(w1, w2);
  return w2 - w1;
}

// one point of a run, a body, a cardinal point or an intermediate cusp
struct Point {
  int slot = 0;
  int cusp = 0;
  double el = 0.0;
  double eb = 0.0;

  [[nodiscard]] int id() const { return cusp > 0 ? kCuspId + cusp : slot; }
};

struct Run {
  const Chart& chart;
  DirectionMethod method;
  const DirectionRange& range;
  double lat_deg;
  double armcb;
  int d1 = 0;
  std::vector<DirectionHit> hits;
  // his drk! cube, one flag per multiple, directed point, target and
  // midpoint partner
  std::set<std::tuple<int, int, int, int>> seen;

  // a body slot the walk may use, the south node mirrors the north and
  // stays out like his u& = 12 tests
  [[nodiscard]] bool usable(int slot) const {
    if (slot == body::kNodeDesc || cardinal(slot)) {
      return false;
    }
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    return b.present && b.valid && b.el > kEps;
  }

  // auswahl_flag, the chosen significators of NUR DIESE DARSTELLEN
  [[nodiscard]] bool chosen(int slot) const {
    return range.chosen.empty() || std::find(range.chosen.begin(), range.chosen.end(), slot) != range.chosen.end();
  }

  [[nodiscard]] Point body_point(int slot) const {
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    return {slot, 0, b.el, b.eb};
  }

  // the bodies from his pl1 on, the cardinal points on their slots when
  // they are asked for
  [[nodiscard]] std::vector<Point> points(bool with_cardinal) const {
    std::vector<Point> out;
    for (int slot = std::max(0, range.first_slot); slot < body::kSlotCount; ++slot) {
      if (cardinal(slot)) {
        if (with_cardinal) {
          // ported from a921
          //RR Kardinalpunkte
          out.push_back({slot, 0, (slot - body::kAriesPoint) * kHalfPi + kEps, 0.0});
        }
        continue;
      }
      if (usable(slot)) {
        out.push_back(body_point(slot));
      }
    }
    return out;
  }

  [[nodiscard]] std::vector<Point> cusps() const {
    std::vector<Point> out;
    if (!range.house_targets || !chart.houses.ok || chart.houses.cusp[2] <= 0.0) {
      return out;
    }
    for (const int h : kIntermediateCusps) {
      out.push_back({0, h, chart.houses.cusp[static_cast<std::size_t>(h)], 0.0});
    }
    return out;
  }

  // one arc through the sign rule, the key and the window, his aprimout
  void take(const Point& t, const Point& u, int partner, int w, double dib, bool symbolic_frame) {
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
    const double b = std::abs(dib) * kRadToDeg;
    const double years = range.key * b;
    // his window held the arc against the Lebensjahre, under NAIBOD the
    // list ran past the asked end, the port holds the years
    if (!(range.from_years < years && years <= range.to_years && years > 0.0 && years < 180.0)) {
      return;
    }
    if (w == 0 && u.id() == t.id() && partner == 0) {
      return;
    }
    const auto key = std::make_tuple(w, t.id(), u.id(), partner);
    if (seen.count(key) > 0) {
      return;
    }
    // a symbolic arc between two points reads the same from both ends,
    // the mirror stays silent like his drk!(w,u,t) test in aprimout
    if (symbolic_frame && partner == 0 &&
        (seen.count(std::make_tuple(w, u.id(), t.id(), 0)) > 0 ||
         seen.count(std::make_tuple(d1 - w, u.id(), t.id(), 0)) > 0)) {
      seen.insert(key);
      return;
    }
    seen.insert(key);
    DirectionHit h;
    h.directed = t.slot;
    h.directed_cusp = t.cusp;
    h.target = u.slot;
    h.target2 = partner;
    h.target_cusp = u.cusp;
    h.multiple = w;
    h.arc_deg = b;
    h.years = years;
    h.converse = converse;
    hits.push_back(h);
  }

  // one position in the chosen symbolic frame
  [[nodiscard]] double frame_value(const Point& p) const {
    switch (method) {
      case DirectionMethod::kSymbolicEquatorial:
        // a cusp has no latitude, his zwhd pass kept the last body's
        return ecliptic_to_equatorial(p.el, p.cusp > 0 ? 0.0 : p.eb, chart.smo.ekls).ra;
      case DirectionMethod::kSymbolicMundane:
        return mundane_longitude(p.el, p.cusp > 0 ? 0.0 : p.eb, chart.smo.ekls, armcb, lat_deg);
      default:
        return p.el;
    }
  }
};

// asy111, the directed point against every target, both readings
void symbolic_pairs(Run& r, const Point& t, double w2, const std::vector<Point>& targets) {
  const double w4d = r.range.base_angle_deg * kDegToRad;
  for (const Point& u : targets) {
    if (u.id() == t.id() || (cardinal(u.slot) && (cardinal(t.slot) || t.cusp > 0))) {
      continue;
    }
    const double pu = r.frame_value(u);
    for (int w = 0; w < r.d1; ++w) {
      //RR nur direkt
      // the cardinal points answer only the conjunction
      if (cardinal(u.slot) && w > 0) {
        continue;
      }
      if (grid_skips(r.range.grid, w)) {
        continue;
      }
      const double w1 = norm_rad(pu + w * w4d);
      if (w1 < kEps || w2 < kEps) {
        continue;
      }
      double a1 = w1;
      double a2 = w2;
      vergl1(a1, a2);
      r.take(t, u, 0, w, a2 - a1, true);
      a1 = w1;
      a2 = w2;
      vergl1r(a1, a2);
      r.take(t, u, 0, w, a2 - a1, true);
    }
  }
}

// asy112, the directed point against the midpoints of every pair, the
// ecliptic frame only. His drk! memory had no room for the partner, the
// first midpoint of a factor silenced every other one at the same
// multiple, the port keys the partner too. Like his auswahl_flag(u&)
// only the first factor of a midpoint must be a chosen one
void symbolic_midpoints(Run& r, const Point& t, double w2, const std::vector<Point>& factors) {
  const double w4d = r.range.base_angle_deg * kDegToRad;
  for (std::size_t i = 0; i < factors.size(); ++i) {
    const Point& u = factors[i];
    if (u.id() == t.id() || !r.chosen(u.slot)) {
      continue;
    }
    for (int w = 0; w < r.d1; ++w) {
      if (grid_skips(r.range.grid, w)) {
        continue;
      }
      for (std::size_t j = i + 1; j < factors.size(); ++j) {
        const Point& v = factors[j];
        // NUR DIE MIT 3 UNTERSCHIEDLICHEN Faktoren
        if (r.range.midpoints == 2 && (t.id() == u.id() || t.id() == v.id())) {
          continue;
        }
        const double w1 = norm_rad(midpoint_near(u.el, v.el) + w * w4d);
        if (w1 < kEps || w2 < kEps) {
          continue;
        }
        double a1 = w1;
        double a2 = w2;
        vergl1(a1, a2);
        r.take(t, u, v.slot, w, a2 - a1, true);
        a1 = w1;
        a2 = w2;
        vergl1r(a1, a2);
        r.take(t, u, v.slot, w, a2 - a1, true);
      }
    }
  }
}

// ported from asymb1 with asy111 and asy112, the pair walk of the
// symbolic frames. The cardinal points take part in the main pass, the
// intermediate cusps direct in a pass of their own
void run_symbolic(Run& r) {
  const bool ecliptic = r.method == DirectionMethod::kSymbolicEcliptic;
  const std::vector<Point> all = r.points(r.range.cardinal_targets);
  for (const Point& t : all) {
    if (!r.chosen(t.slot)) {
      continue;
    }
    const double w2 = r.frame_value(t);
    symbolic_pairs(r, t, w2, all);
    if (ecliptic && r.range.midpoints > 0) {
      symbolic_midpoints(r, t, w2, all);
    }
  }
  //RR Zwischenhäuser
  // directed only, the chosen list holds no cusps
  if (!r.range.chosen.empty()) {
    return;
  }
  for (const Point& t : r.cusps()) {
    const double w2 = r.frame_value(t);
    symbolic_pairs(r, t, w2, all);
    if (ecliptic && r.range.midpoints > 0) {
      symbolic_midpoints(r, t, w2, all);
    }
  }
}

// ported from aprim1, the Kühr primary walk in oblique ascension. The
// intermediate cusps take both parts, significator and promissor
void run_primary(Run& r) {
  const double w4d = r.range.base_angle_deg * kDegToRad;
  // his brep&, the promissors with latitude
  const double brep = r.range.with_latitude ? 1.0 : 0.0;
  std::vector<Point> all = r.points(false);
  const std::vector<Point> cusps = r.cusps();
  all.insert(all.end(), cusps.begin(), cusps.end());
  // the ascensional difference of the promissor under the significator's
  // pole. The arcsine slips outside its domain for extreme declinations,
  // the last good value of the whole run carries on like his LOCAL ad of
  // aprim1
  double ad = 0.0;
  for (const Point& t : all) {
    if (t.cusp == 0 && !r.chosen(t.slot)) {
      continue;
    }
    if (t.cusp > 0 && !r.range.chosen.empty()) {
      continue;
    }
    // SIGNIFIKAT. OHNE Breite was shown in his header yet the walk kept
    // the latitude, the port honours the answer
    const double br = t.cusp > 0 || !r.range.significator_latitude ? 0.0 : t.eb;
    const Equatorial sig = ecliptic_to_equatorial(t.el, br, r.chart.smo.ekls);
    const SemiArcPoint arc = semi_arc_point(sig.ra, sig.dec, r.armcb, r.lat_deg);
    for (const Point& u : all) {
      if (u.id() == t.id() || (t.cusp > 0 && u.cusp > 0)) {
        continue;
      }
      // aprim1 asks auswahl_flag for the promissors too
      if ((u.cusp == 0 && !r.chosen(u.slot)) || (u.cusp > 0 && !r.range.chosen.empty())) {
        continue;
      }
      for (int w = 0; w < r.d1; ++w) {
        if (grid_skips(r.range.grid, w)) {
          continue;
        }
        const double f = w * w4d;
        const double pla = norm_rad(u.el + f);
        // the latitude of the aspect point fades with the angle
        const double pbr = u.cusp > 0 ? 0.0 : std::asin(std::sin(brep * u.eb) * std::sin(kHalfPi - f));
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
        r.take(t, u, 0, w, dib, false);
      }
    }
  }
}

}  // namespace

// ported from asymb, ar_sys and aprim
std::vector<DirectionHit> direction_hits(const Chart& chart, DirectionMethod method, const DirectionRange& range, double lat_deg) {
  // ar_sys and aprim set haus$ = "Placidus" for their whole run, the
  // intermediate cusps they direct are Placidus ones whatever the chart
  // uses, only the ecliptic asymb keeps the chosen system
  Chart work = chart;
  if (method != DirectionMethod::kSymbolicEcliptic) {
    const Houses placidus = compute_houses(HouseSystem::kPlacidus, chart.armc_deg * kDegToRad, lat_deg, chart.ekls0);
    if (placidus.ok) {
      work.houses.cusp = placidus.cusp;
    }
  }
  DirectionRange run_range = range;
  // kard! = 0 in aprim, the Kühr primary knows no cardinal points
  if (method == DirectionMethod::kPrimary) {
    run_range.cardinal_targets = false;
  }
  // the midpoints of halbs_dir count only in the ecliptic frame
  if (method != DirectionMethod::kSymbolicEcliptic) {
    run_range.midpoints = 0;
  }
  Run r{work, method, run_range, lat_deg, chart.armc_deg * kDegToRad, 0, {}, {}};
  r.d1 = base_multiples(range.base_angle_deg);
  if (method == DirectionMethod::kPrimary) {
    run_primary(r);
  } else {
    run_symbolic(r);
  }
  return r.hits;
}

}  // namespace horcom
