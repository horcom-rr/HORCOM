// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/wheel.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

constexpr double kCx = kWheelCenterX;
constexpr double kCy = kWheelCenterY;
constexpr double kKm = kWheelScale;

// the ring radii of the original horg and its helpers
constexpr double kAspectRing = 90.0;
constexpr double kGlyphRing = kGlyphRingRadius;
constexpr double kSignInner = 152.0;
constexpr double kSignGlyphRing = 165.0;
constexpr double kSignOuter = 182.0;
constexpr double kAxisEnd = 188.0;
constexpr double kAxisLabel = 200.0;
constexpr double kTickInnerRing = 148.0;
constexpr double kConjDotRing = 85.0;  // the original red conjunction dot
constexpr double kGlyphSize = 14.0;
constexpr double kNumberSize = 8.0;
constexpr double kAxisTextSize = 11.0;
// the transit ring of a20, glyphs from plein1 and markers from plmk
constexpr double kTransitGlyphRing = 212.0;
constexpr double kMarkInset = 3.0;
constexpr double kMarkOutset = 5.0;
constexpr double kLabelSize = 10.0;

// element colours of the original fill_color, fire, earth, air, water
constexpr Rgb kElementColor[4] = {0xFF0000, 0x808000, 0x008080, 0x00FFFF};

// aspect chord colours of the original aspz1 per divisor
constexpr Rgb kAspectColor[13] = {0, 0, 0xFF0000, 0x00C800, 0xFF0000, 0x0000C8, 0x00C800,
                                  0x0000C8, 0xFF0000, 0xFF0000, 0x0000C8, 0x000000, 0x00C800};

// unicode glyphs per body slot, text tags where no glyph exists
constexpr const char* kBodyGlyph[body::kSlotCount] = {
    "F",       "☉", "☽", "☿", "♀", "♂", "♃", "♄", "♅",
    "♆",  "♇", "☊", "☋", "",       "",       "",       "",       "",
    "",        "⚸", "⚷", "TP",     "⊗", "⚳", "⚴", "⚵", "⚶",
    "CU",      "HA",     "ZE",     "KR",     "AP",     "AD",     "VU",     "PO",     "QU",
    "☄",  "PH",     "DA",     "NS",     "XE"};

constexpr const char* kSignGlyph[12] = {"♈", "♉", "♊", "♋", "♌", "♍",
                                        "♎", "♏", "♐", "♑", "♒", "♓"};

struct Pt {
  double x;
  double y;
};

// the polar mapping of horg10, plein1 and aspz0
Pt at(double w, double r, double km) {
  return {kCx + km * r * std::cos(-w), kCy + km * r * std::sin(-w)};
}

double wheel_angle(double lambda, double fza) {
  return norm_rad(lambda + kPi - fza);
}

// the original plentz chain, radial stagger and angular push apart of
// crowded glyphs. Operates on the display longitudes wl and offsets dc.
void declump(const std::vector<int>& slots, const std::array<double, 41>& pl, std::array<double, 41>& wl, std::array<double, 41>& dc) {
  std::vector<int> order = slots;
  std::sort(order.begin(), order.end(), [&](int a, int b) {
    return wl[static_cast<std::size_t>(a)] < wl[static_cast<std::size_t>(b)];
  });
  const double amp = 8.0;
  //RR plentz1
  const double dd1 = kPi / 18.0;
  double kf = 1.0;
  const auto stagger = [&](int k, int l) {
    const double dw = std::abs(wl[static_cast<std::size_t>(l)] - wl[static_cast<std::size_t>(k)]);
    if (dw < dd1 || dw > kTwoPi - dd1) {
      kf = -kf;
      dc[static_cast<std::size_t>(k)] = amp * kf;
      if (dc[static_cast<std::size_t>(k)] < 0.0) {
        dc[static_cast<std::size_t>(k)] = -amp * 1.8;
      }
      dc[static_cast<std::size_t>(l)] = -amp * kf;
      if (dc[static_cast<std::size_t>(l)] < 0.0) {
        dc[static_cast<std::size_t>(l)] = -amp * 1.8;
      }
    }
  };
  for (std::size_t h = 0; h + 1 < order.size(); ++h) {
    stagger(order[h], order[h + 1]);
  }
  if (order.size() >= 2) {
    stagger(order.back(), order.front());
  }
  //RR plentz21
  const double near = kPi / 20.0;
  for (std::size_t h = 0; h + 1 < order.size(); ++h) {
    const int k = order[h];
    if (dc[static_cast<std::size_t>(k)] == 0.0) {
      continue;
    }
    for (std::size_t step = 1; step <= 2 && h + step < order.size(); ++step) {
      const int l = order[h + step];
      if (dc[static_cast<std::size_t>(l)] == 0.0) {
        continue;
      }
      double w1 = pl[static_cast<std::size_t>(k)];
      double w2 = pl[static_cast<std::size_t>(l)];
      vergl1(w1, w2);
      const double dw = std::abs(w2 - w1);
      if (dw < near) {
        double ddw = 0.065;
        if (dc[static_cast<std::size_t>(k)] < 0.0 && dc[static_cast<std::size_t>(l)] < 0.0) {
          ddw = 0.08;
        } else if ((dc[static_cast<std::size_t>(k)] < 0.0) != (dc[static_cast<std::size_t>(l)] < 0.0)) {
          ddw = 0.05;
        }
        const double dw1 = ddw - dw / 2.0;
        if (w1 < w2) {
          wl[static_cast<std::size_t>(k)] = norm_rad(wl[static_cast<std::size_t>(k)] - dw1);
          wl[static_cast<std::size_t>(l)] = norm_rad(wl[static_cast<std::size_t>(l)] + dw1);
        } else {
          wl[static_cast<std::size_t>(k)] = norm_rad(wl[static_cast<std::size_t>(k)] + dw1);
          wl[static_cast<std::size_t>(l)] = norm_rad(wl[static_cast<std::size_t>(l)] - dw1);
        }
      }
    }
  }
}

}  // namespace

// the shared wheel body, drawn at the given km so the radix wheel and
// the smaller a20 transit wheel reuse the same geometry
static void build_base(DisplayList& dl, const Chart& chart, const ChartSettings& s, const AspectResult& aspects, const WheelOptions& opt, double km) {
  auto add = [&](Primitive p) { dl.items.push_back(std::move(p)); };
  const auto at = [km](double w, double r) {
    return Pt{kCx + km * r * std::cos(-w), kCy + km * r * std::sin(-w)};
  };

  // the rotation origin, the default begz& = 1 puts the AC left
  const double fza = chart.houses.angles.ac;

  // sign band as explicit annular sectors with the element colours
  for (int j = 1; j <= 12; ++j) {
    const double a0 = wheel_angle((j - 1) * kPi / 6.0, fza);
    Primitive sec;
    sec.kind = Primitive::Kind::kSector;
    sec.x1 = kCx;
    sec.y1 = kCy;
    sec.r1 = km * kSignInner;
    sec.r2 = km * kSignOuter;
    sec.a1 = a0;
    sec.a2 = a0 + kPi / 6.0;
    sec.fill = kElementColor[(j - 1) % 4];
    sec.color = 0x000000;
    add(sec);
  }
  // the three ring circles of horg1
  for (double r : {kAspectRing, kSignInner, kSignOuter}) {
    Primitive c;
    c.kind = Primitive::Kind::kCircle;
    c.x1 = kCx;
    c.y1 = kCy;
    c.r1 = km * r;
    add(c);
  }
  //RR Zeichentrenn-Linien
  for (int j = 0; j < 12; ++j) {
    const double w = wheel_angle(j * kPi / 6.0, fza);
    const Pt a = at(w, kSignInner - 1.0);
    const Pt b = at(w, kSignOuter + 1.0);
    add({Primitive::Kind::kLine, a.x, a.y, b.x, b.y});
  }
  // sign glyphs centred in their sign like zein1
  for (int j = 1; j <= 12; ++j) {
    const double w = norm_rad(j * kPi / 6.0 + 11.0 * kPi / 12.0 - fza);
    const Pt p = at(w, kSignGlyphRing);
    Primitive g;
    g.kind = Primitive::Kind::kGlyph;
    g.x1 = p.x;
    g.y1 = p.y;
    g.size = kGlyphSize;
    g.text = kSignGlyph[j - 1];
    add(g);
  }

  // houses like horg11, thick axes to 188 with labels at 200, thin cusps
  const bool houses_drawn = !(s.houses == HouseSystem::kAcMcOnly || s.houses == HouseSystem::kNone);
  if (s.houses != HouseSystem::kNone) {
    static constexpr const char* kAxisLabelText[4] = {"AC", "IC", "DC", "MC"};
    const int axes[4] = {1, 4, 7, 10};
    for (int a = 0; a < 4; ++a) {
      const double cusp = chart.houses.cusp[static_cast<std::size_t>(axes[a])];
      const double w = wheel_angle(cusp, fza);
      const Pt p1 = at(w, kAspectRing);
      const Pt p2 = at(w, kAxisEnd);
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF, Primitive::Style::kSolid, 2.0});
      const Pt pl = at(w, kAxisLabel);
      Primitive t;
      t.kind = Primitive::Kind::kText;
      t.x1 = pl.x;
      t.y1 = pl.y;
      t.size = kAxisTextSize;
      t.text = kAxisLabelText[a];
      add(t);
    }
  }
  if (houses_drawn) {
    for (int i : {2, 3, 5, 6, 8, 9, 11, 12}) {
      const double w = wheel_angle(chart.houses.cusp[static_cast<std::size_t>(i)], fza);
      const Pt p1 = at(w, kAspectRing);
      const Pt p2 = at(w, kSignInner);
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y});
    }
  }

  // collect the visible body slots
  std::vector<int> slots;
  std::array<double, 41> pl{};
  std::array<double, 41> wl{};
  std::array<double, 41> dc{};
  for (int slot = 0; slot <= 40; ++slot) {
    if (slot == body::kAscendant || slot == body::kMc) {
      continue;
    }
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid) {
      continue;
    }
    slots.push_back(slot);
    pl[static_cast<std::size_t>(slot)] = b.el;
    wl[static_cast<std::size_t>(slot)] = wheel_angle(b.el, fza);
  }
  declump(slots, pl, wl, dc);

  // glyphs with tick marks like plein1 and plein11
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const double w_true = wheel_angle(pl[si], fza);
    const Pt tick1 = at(w_true, kTickInnerRing);
    const Pt tick2 = at(w_true, kSignInner);
    add({Primitive::Kind::kLine, tick1.x, tick1.y, tick2.x, tick2.y});
    const Pt g = at(wl[si], kGlyphRing + dc[si]);
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = g.x;
    p.y1 = g.y;
    p.size = kGlyphSize;
    p.text = kBodyGlyph[si];
    if (chart.b[si].tb < 0.0 && slot >= 3 && slot <= 10) {
      p.text += " R";
    }
    add(p);
    if (opt.degree_numbers) {
      const Pt n = at(wl[si], kGlyphRing + dc[si] - kGlyphSize);
      Primitive num;
      num.kind = Primitive::Kind::kText;
      num.x1 = n.x;
      num.y1 = n.y;
      num.size = kNumberSize;
      const int deg = static_cast<int>(norm_deg(pl[si] * kRadToDeg)) % 30;
      num.text = std::to_string(deg);
      add(num);
    }
  }

  // node axis, blue dashed like the Mondknotenlinie
  if (opt.node_axis && chart.b[body::kNodeAsc].present) {
    const double w1 = wheel_angle(chart.b[body::kNodeAsc].el, fza);
    const double w2 = wheel_angle(chart.b[body::kNodeDesc].el, fza);
    const Pt a = at(w1, kAspectRing);
    const Pt b = at(w2, kAspectRing);
    add({Primitive::Kind::kLine, a.x, a.y, b.x, b.y, 0, 0, 0, 0, 0, 0x0000FF, 0xFFFFFF, Primitive::Style::kDashed, 1.0});
  }

  // aspect chords on the inner ring like aspz0 and aspz1, gated per
  // divisor like the original aspli flags
  if (opt.aspect_lines) {
    for (const AspectHit& h : aspects.hits) {
      if (h.n < 1 || h.n > 16 || !opt.chord_divisor[static_cast<std::size_t>(h.n)]) {
        continue;
      }
      const double w1 = wheel_angle(chart.b[static_cast<std::size_t>(h.t)].el, fza);
      const double w2 = wheel_angle(chart.b[static_cast<std::size_t>(h.w)].el, fza);
      if (h.n == 1) {
        // the conjunction is a red dot at 85 like the original
        double wa = (w1 + w2) / 2.0;
        if (std::abs(wa - w1) > 1.0 || std::abs(wa - w2) > 1.0) {
          wa += kPi;
        }
        const Pt p = at(wa, kConjDotRing);
        add({Primitive::Kind::kDot, p.x, p.y, 0, 0, 3.0, 0, 0, 0, 0, 0xFF0000});
        continue;
      }
      const Pt a = at(w1, kAspectRing);
      const Pt b = at(w2, kAspectRing);
      Primitive line{Primitive::Kind::kLine, a.x, a.y, b.x, b.y};
      line.color = (h.n >= 2 && h.n <= 12) ? kAspectColor[h.n] : 0x000000;
      add(line);
    }
  }

  // the centre label like textc in zeitwi and in the uhr loop
  if (!opt.center_label.empty()) {
    Primitive t;
    t.kind = Primitive::Kind::kText;
    t.x1 = kCx;
    t.y1 = kCy - 12.0;
    t.size = kLabelSize;
    t.text = opt.center_label;
    add(t);
  }
}

DisplayList build_wheel(const Chart& chart, const ChartSettings& s, const AspectResult& aspects, const WheelOptions& opt) {
  DisplayList dl;
  build_base(dl, chart, s, aspects, opt, kKm);
  return dl;
}

DisplayList build_transit_wheel(const Chart& radix, const Chart& transit, const ChartSettings& s, const AspectResult& radix_aspects, const WheelOptions& opt) {
  DisplayList dl;
  const double km = kTransitWheelScale;
  build_base(dl, radix, s, radix_aspects, opt, km);
  auto add = [&](Primitive p) { dl.items.push_back(std::move(p)); };
  const auto at = [km](double w, double r) {
    return Pt{kCx + km * r * std::cos(-w), kCy + km * r * std::sin(-w)};
  };
  // the radix rules the rotation, the running sky turns with it
  const double fza = radix.houses.angles.ac;

  std::vector<int> slots;
  std::array<double, 41> pl{};
  std::array<double, 41> wl{};
  std::array<double, 41> dc{};
  for (int slot = 0; slot <= 40; ++slot) {
    if (slot == body::kAscendant || slot == body::kMc) {
      continue;
    }
    const BodyState& b = transit.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid) {
      continue;
    }
    slots.push_back(slot);
    pl[static_cast<std::size_t>(slot)] = b.el;
    wl[static_cast<std::size_t>(slot)] = wheel_angle(b.el, fza);
  }
  declump(slots, pl, wl, dc);

  // the outer ring of a20, a tick on the sign ring like plmk at 182 and
  // the glyph outside like plein1 at 212
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const double w_true = wheel_angle(pl[si], fza);
    const Pt m1 = at(w_true, kSignOuter - kMarkInset);
    const Pt m2 = at(w_true, kSignOuter + kMarkOutset);
    add({Primitive::Kind::kLine, m1.x, m1.y, m2.x, m2.y});
    const Pt g = at(wl[si], kTransitGlyphRing + dc[si]);
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = g.x;
    p.y1 = g.y;
    p.size = kGlyphSize;
    p.text = kBodyGlyph[si];
    if (transit.b[si].tb < 0.0 && slot >= 3 && slot <= 10) {
      p.text += " R";
    }
    add(p);
    if (opt.degree_numbers) {
      const Pt n = at(wl[si], kTransitGlyphRing + dc[si] - kGlyphSize);
      Primitive num;
      num.kind = Primitive::Kind::kText;
      num.x1 = n.x;
      num.y1 = n.y;
      num.size = kNumberSize;
      const int deg = static_cast<int>(norm_deg(pl[si] * kRadToDeg)) % 30;
      num.text = std::to_string(deg);
      add(num);
    }
  }

  return dl;
}

}  // namespace horcom
