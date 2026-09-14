// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
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
// the comparison ring of a12, the second chart of the double wheel
constexpr double kCompareGlyphRing = 204.0;
constexpr double kCompareMarkRing = 180.0;
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
void declump(const std::vector<int>& slots, const std::array<double, body::kSlotCount>& pl, std::array<double, body::kSlotCount>& wl, std::array<double, body::kSlotCount>& dc) {
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

  // the rotation origin, the default begz& = 1 puts the AC left, horbeg
  // clears it in the hrg mode so zero Aries anchors the wheel
  const double fza = opt.heliocentric ? 0.0 : chart.houses.angles.ac;

  // sign band as explicit annular sectors with the element colours, the
  // 90 degree circle keeps three sectors like the zein fill under dop 4
  const int signs = opt.dial ? 3 : 12;
  const double span = kTwoPi / signs;
  for (int j = 1; j <= signs; ++j) {
    const double a0 = wheel_angle((j - 1) * span, fza);
    Primitive sec;
    sec.kind = Primitive::Kind::kSector;
    sec.x1 = kCx;
    sec.y1 = kCy;
    sec.r1 = km * kSignInner;
    sec.r2 = km * kSignOuter;
    sec.a1 = a0;
    sec.a2 = a0 + span;
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
  //RR SO, horg1 marks the sun as small unscaled circles in the centre
  if (opt.heliocentric) {
    for (double r : {1.0, 7.0}) {
      Primitive c;
      c.kind = Primitive::Kind::kCircle;
      c.x1 = kCx;
      c.y1 = kCy;
      c.r1 = r;
      add(c);
    }
  }
  //RR Zeichentrenn-Linien
  for (int j = 0; j < signs; ++j) {
    const double w = wheel_angle(j * span, fza);
    const Pt a = at(w, kSignInner - 1.0);
    const Pt b = at(w, kSignOuter + 1.0);
    add({Primitive::Kind::kLine, a.x, a.y, b.x, b.y});
  }
  // sign glyphs centred in their sign like zein1, the dial shows the
  // three qualities as their first signs
  for (int j = 1; j <= signs; ++j) {
    const double w = wheel_angle((j - 0.5) * span, fza);
    const Pt p = at(w, kSignGlyphRing);
    Primitive g;
    g.kind = Primitive::Kind::kGlyph;
    g.x1 = p.x;
    g.y1 = p.y;
    g.size = kGlyphSize;
    g.text = kSignGlyph[j - 1];
    add(g);
  }

  // houses like horg11, thick axes to 188 with labels at 200, thin
  // cusps, the whole block stays dark in the hrg mode like the original
  const bool houses_drawn = !opt.heliocentric && !opt.dial && !(s.houses == HouseSystem::kAcMcOnly || s.houses == HouseSystem::kNone);
  if (opt.dial) {
    // only AC and MC survive the times four, DC and IC land on them,
    // aeqh blanks their labels
    static constexpr const char* kDialLabel[2] = {"AC", "MC"};
    const int dial_axes[2] = {1, 10};
    for (int a = 0; a < 2; ++a) {
      const double w = wheel_angle(chart.houses.cusp[static_cast<std::size_t>(dial_axes[a])], fza);
      const Pt p1 = at(w, kAspectRing);
      const Pt p2 = at(w, kAxisEnd);
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF, Primitive::Style::kSolid, 2.0});
      const Pt pl = at(w, kAxisLabel);
      Primitive t;
      t.kind = Primitive::Kind::kText;
      t.x1 = pl.x;
      t.y1 = pl.y;
      t.size = kAxisTextSize;
      t.text = kDialLabel[a];
      add(t);
    }
  }
  if (!opt.heliocentric && !opt.dial && s.houses != HouseSystem::kNone) {
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
  std::array<double, body::kSlotCount> pl{};
  std::array<double, body::kSlotCount> wl{};
  std::array<double, body::kSlotCount> dc{};
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    if (slot == body::kAscendant || slot == body::kMc) {
      continue;
    }
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid) {
      continue;
    }
    //RR EINZELNE PLANETEN auswählen, sodaß NUR DIESE sichtbar sind
    if (opt.emphasis[static_cast<std::size_t>(slot)] < 0) {
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
    // in the hrg mode the moon slot carries the earth
    p.text = (opt.heliocentric && slot == body::kMoon) ? "\xE2\x8A\x95" : kBodyGlyph[si];
    if (chart.b[si].tb < 0.0 && slot >= 3 && slot <= 10) {
      p.text += " R";
    }
    //RR einzelne Planeten ROT markieren
    if (opt.emphasis[si] > 0) {
      p.color = 0xFF0000;
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

  // the credit line like drad2 stamped on every output, his name first
  {
    Primitive credit;
    credit.kind = Primitive::Kind::kText;
    credit.x1 = 8.0;
    credit.y1 = kCanvasHeight - 8.0;
    credit.size = 8.0;
    credit.color = 0x808080;
    credit.align_left = true;
    credit.text = "HORCOM \xC2\xB7 Robert Rettig \xC2\xB7 \xC2\xA9 Dominik Schwimmbeck";
    add(credit);
  }

  // the chart data block like bes11, down the left margin the original
  // kept free of the wheel
  {
    double y = 18.0;
    for (const std::string& line : opt.info_lines) {
      Primitive t;
      t.kind = Primitive::Kind::kText;
      t.x1 = 8.0;
      t.y1 = y;
      t.size = kLabelSize;
      t.align_left = true;
      t.text = line;
      add(t);
      y += 13.0;
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
  build_base(dl, chart, s, aspects, opt, opt.scale > 0.0 ? opt.scale : kKm);
  return dl;
}

// the outer ring shared by the a20 transit and the a12 comparison, a
// tick marker like plmk and the glyph outside like plein1
static void draw_outer_bodies(DisplayList& dl, const Chart& chart, double fza, double km, double mark_ring, double glyph_ring, const WheelOptions& opt) {
  auto add = [&](Primitive p) { dl.items.push_back(std::move(p)); };
  const auto at = [km](double w, double r) {
    return Pt{kCx + km * r * std::cos(-w), kCy + km * r * std::sin(-w)};
  };
  std::vector<int> slots;
  std::array<double, body::kSlotCount> pl{};
  std::array<double, body::kSlotCount> wl{};
  std::array<double, body::kSlotCount> dc{};
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
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
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const double w_true = wheel_angle(pl[si], fza);
    const Pt m1 = at(w_true, mark_ring - kMarkInset);
    const Pt m2 = at(w_true, mark_ring + kMarkOutset);
    add({Primitive::Kind::kLine, m1.x, m1.y, m2.x, m2.y});
    const Pt g = at(wl[si], glyph_ring + dc[si]);
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
      const Pt n = at(wl[si], glyph_ring + dc[si] - kGlyphSize);
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
}

DisplayList build_transit_wheel(const Chart& radix, const Chart& transit, const ChartSettings& s, const AspectResult& radix_aspects, const WheelOptions& opt) {
  DisplayList dl;
  build_base(dl, radix, s, radix_aspects, opt, kTransitWheelScale);
  // the radix rules the rotation, the running sky turns with it
  const double fza = opt.heliocentric ? 0.0 : radix.houses.angles.ac;
  draw_outer_bodies(dl, transit, fza, kTransitWheelScale, kSignOuter, kTransitGlyphRing, opt);
  return dl;
}

DisplayList build_double_wheel(const Chart& inner, const Chart& outer, const ChartSettings& s, const AspectResult& inner_aspects, const WheelOptions& opt) {
  DisplayList dl;
  build_base(dl, inner, s, inner_aspects, opt, kKm);
  auto add = [&](Primitive p) { dl.items.push_back(std::move(p)); };
  const auto at = [](double w, double r) {
    return Pt{kCx + kKm * r * std::cos(-w), kCy + kKm * r * std::sin(-w)};
  };
  const double fza = opt.heliocentric ? 0.0 : inner.houses.angles.ac;
  // the second chart's house lines draw over the shared ring like the
  // original's second horg11 pass, without a second set of labels
  if (!opt.heliocentric && s.houses != HouseSystem::kNone) {
    for (int a : {1, 4, 7, 10}) {
      const double w = wheel_angle(outer.houses.cusp[static_cast<std::size_t>(a)], fza);
      const Pt p1 = at(w, kAspectRing);
      const Pt p2 = at(w, kAxisEnd);
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF, Primitive::Style::kSolid, 2.0});
    }
  }
  if (!opt.heliocentric && !opt.dial && !(s.houses == HouseSystem::kAcMcOnly || s.houses == HouseSystem::kNone)) {
    for (int i : {2, 3, 5, 6, 8, 9, 11, 12}) {
      const double w = wheel_angle(outer.houses.cusp[static_cast<std::size_t>(i)], fza);
      const Pt p1 = at(w, kAspectRing);
      const Pt p2 = at(w, kSignInner);
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y});
    }
  }
  draw_outer_bodies(dl, outer, fza, kKm, kCompareMarkRing, kCompareGlyphRing, opt);
  return dl;
}

const char* body_glyph(int slot) {
  return (slot >= 0 && slot < body::kSlotCount) ? kBodyGlyph[slot] : "";
}

const char* sign_glyph(int index) {
  return (index >= 0 && index < 12) ? kSignGlyph[index] : "";
}

}  // namespace horcom
