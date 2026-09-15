// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/wheel.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

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
constexpr double kAxisLabel = 208.0;
// the position mark ring of plein11, ticks span minus 3 to plus 5
constexpr double kTickRing = 150.0;
constexpr double kConjDotRing = 85.0;  // the original red conjunction dot
constexpr double kGlyphSize = 14.0;
constexpr double kNumberSize = 10.0;
constexpr double kAxisTextSize = 11.0;
// the transit ring of a20, glyphs from plein1 and markers from plmk
constexpr double kTransitGlyphRing = 212.0;
// the comparison ring of a12, the second chart of the double wheel
constexpr double kCompareGlyphRing = 204.0;
constexpr double kCompareMarkRing = 180.0;
constexpr double kMarkInset = 3.0;
constexpr double kMarkOutset = 5.0;
constexpr double kLabelSize = 10.0;

// the paper of the sheet, the cutouts under the glyphs wear it
constexpr Rgb kPaper = 0xFCFAF4;

// element colours of the original fill_color as they appeared on his
// screen, fire, earth, air, water. His code asked for red, olive, teal
// and cyan, but the 48 colour palette and the hatch fills of deffi
// rendered salmon, green, pale cyan grey and blue. These are the
// hatch densities pre blended on white, drawn opaque.
constexpr Rgb kElementColor[4] = {0xFF9086, 0x14CD14, 0xDEF2F2, 0x4646FF};

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
      // his layout carries the rounded degree in sign beside the label
      t.text = std::string(kAxisLabelText[a]) + " " +
               std::to_string(static_cast<int>(norm_deg(cusp * kRadToDeg)) % 30);
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
    // the two position marks of plein11, r 150 and, with aspects on,
    // r 90 on the aspect circle, each spanning r minus 3 to r plus 5
    const Pt tick1 = at(w_true, kTickRing - kMarkInset);
    const Pt tick2 = at(w_true, kTickRing + kMarkOutset);
    add({Primitive::Kind::kLine, tick1.x, tick1.y, tick2.x, tick2.y});
    if (opt.aspect_lines) {
      const Pt in1 = at(w_true, kAspectRing - kMarkInset);
      const Pt in2 = at(w_true, kAspectRing + kMarkOutset);
      add({Primitive::Kind::kLine, in1.x, in1.y, in2.x, in2.y});
    }
    const Pt g = at(wl[si], kGlyphRing + dc[si]);
    // the paper cutout under every glyph, his putbm sprites erased the
    // lines beneath with their white background
    add({Primitive::Kind::kDot, g.x, g.y, 0, 0, kGlyphSize * 0.72, 0, 0, 0, 0, kPaper});
    // his node glyphs sit inverted on a dark patch, putbm SRCINVERT
    const bool inverted = slot == body::kNodeAsc || slot == body::kNodeDesc;
    if (inverted) {
      Primitive box;
      box.kind = Primitive::Kind::kDot;
      box.x1 = g.x;
      box.y1 = g.y;
      box.r1 = kGlyphSize * 0.62;
      box.color = 0x000000;
      add(box);
    }
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = g.x;
    p.y1 = g.y;
    p.size = kGlyphSize;
    if (inverted) {
      p.color = 0xFFFFFF;
    }
    // in the hrg mode the moon slot carries the earth
    p.text = (opt.heliocentric && slot == body::kMoon) ? "\xE2\x8A\x95" : kBodyGlyph[si];
    //RR einzelne Planeten ROT markieren
    if (opt.emphasis[si] > 0) {
      p.color = 0xFF0000;
    }
    add(p);
    // the red R of his retrograde marker beside the glyph
    if (chart.b[si].tb < 0.0 && slot >= 3 && slot <= 10) {
      Primitive r;
      r.kind = Primitive::Kind::kText;
      r.x1 = g.x + kGlyphSize * 0.85;
      r.y1 = g.y - kGlyphSize * 0.3;
      r.size = kNumberSize;
      r.color = 0xFF0000;
      r.text = "R";
      add({Primitive::Kind::kDot, r.x1, r.y1, 0, 0, kNumberSize * 0.6, 0, 0, 0, 0, kPaper});
      add(r);
    }
    if (opt.degree_numbers) {
      const Pt n = at(wl[si], kGlyphRing + dc[si] - kGlyphSize);
      add({Primitive::Kind::kDot, n.x, n.y, 0, 0, kNumberSize * 0.75, 0, 0, 0, 0, kPaper});
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
      // the DEFLINE styles of aspz1 per divisor, dash for the
      // opposition, dot for the trigon, dash dot for the quadrat
      static constexpr Primitive::Style kChordStyle[13] = {
          Primitive::Style::kSolid,   Primitive::Style::kSolid,  Primitive::Style::kDashed,
          Primitive::Style::kDotted,  Primitive::Style::kDashDot, Primitive::Style::kDotted,
          Primitive::Style::kDashed,  Primitive::Style::kDotted, Primitive::Style::kDashDot,
          Primitive::Style::kDotted,  Primitive::Style::kDotted, Primitive::Style::kDotted,
          Primitive::Style::kDashDot};
      if (h.n >= 2 && h.n <= 12) {
        line.style = kChordStyle[h.n];
      }
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
    credit.anchor = Primitive::Anchor::kCredit;
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
      t.anchor = Primitive::Anchor::kCorner;
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
    add({Primitive::Kind::kDot, g.x, g.y, 0, 0, kGlyphSize * 0.72, 0, 0, 0, 0, kPaper});
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = g.x;
    p.y1 = g.y;
    p.size = kGlyphSize;
    p.text = kBodyGlyph[si];
    add(p);
    if (chart.b[si].tb < 0.0 && slot >= 3 && slot <= 10) {
      Primitive r;
      r.kind = Primitive::Kind::kText;
      r.x1 = g.x + kGlyphSize * 0.85;
      r.y1 = g.y - kGlyphSize * 0.3;
      r.size = kNumberSize;
      r.color = 0xFF0000;
      r.text = "R";
      add({Primitive::Kind::kDot, r.x1, r.y1, 0, 0, kNumberSize * 0.6, 0, 0, 0, 0, kPaper});
      add(r);
    }
    if (opt.degree_numbers) {
      const Pt n = at(wl[si], glyph_ring + dc[si] - kGlyphSize);
      add({Primitive::Kind::kDot, n.x, n.y, 0, 0, kNumberSize * 0.75, 0, 0, 0, 0, kPaper});
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

DisplayList centered_sheet(const DisplayList& dl) {
  DisplayList out = dl;
  // the wheel centre of the classic sheet moves to the middle of a
  // square sheet, the corner notes and the credit stay on the margins
  const double dx = kCanvasHeight / 2.0 - kCx;
  const double dy = kCanvasHeight / 2.0 - kCy;
  out.width = kCanvasHeight;
  for (Primitive& p : out.items) {
    if (p.anchor != Primitive::Anchor::kSheet) {
      continue;
    }
    p.x1 += dx;
    p.y1 += dy;
    p.x2 += dx;
    p.y2 += dy;
  }
  return out;
}

void add_classic_text(DisplayList& dl, const Chart& chart, const ChartSettings& s, const ClassicSheetText& txt) {
  // the corner notes of the screen sheet give way to his block
  dl.items.erase(std::remove_if(dl.items.begin(), dl.items.end(),
                                [](const Primitive& p) { return p.anchor == Primitive::Anchor::kCorner; }),
                 dl.items.end());
  auto text = [&](double x, double y, std::string t, double size = 10.0) {
    Primitive p;
    p.kind = Primitive::Kind::kText;
    p.x1 = x;
    p.y1 = y;
    p.size = size;
    p.align_left = true;
    p.anchor = Primitive::Anchor::kCorner;
    p.text = std::move(t);
    dl.items.push_back(std::move(p));
  };
  static constexpr const char* kSign3[12] = {"AR", "TA", "GM", "CN", "LE", "VI",
                                             "LI", "SC", "SG", "CP", "AQ", "PS"};
  const auto zod = [](double rad, char mark) {
    const double deg = norm_deg(rad * kRadToDeg);
    int sg = static_cast<int>(deg / 30.0);
    const double in_sign = deg - sg * 30.0;
    int d = static_cast<int>(in_sign);
    int m = static_cast<int>((in_sign - d) * 60.0 + 0.5);
    if (m == 60) {
      m = 0;
      if (++d == 30) {
        d = 0;
        sg = (sg + 1) % 12;
      }
    }
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%2d %s %2d%c", d, kSign3[sg], m, mark);
    return std::string(buf);
  };

  //RR Länge:A1, the length mode tag of his table header
  const char* mode = s.apparent == ApparentMode::kLightTime         ? "A1"
                     : s.apparent == ApparentMode::kLightTimeAberration ? "A2"
                                                                        : "T";
  const double x0 = 6.0;
  text(x0, 16.0, std::string("L\xC3\xA4nge:") + mode);
  text(118.0, 16.0, "Vel.");
  static constexpr const char* kRowTag[11] = {"SO", "MO", "ME", "VE", "MA", "JU",
                                              "SA", "UR", "NE", "PL", "DR"};
  double y = 31.0;
  for (int row = 0; row < 11; ++row) {
    const int slot = (row < 10) ? row + 1 : body::kNodeAsc;
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid) {
      continue;
    }
    // P marks the parallax corrected rows, W the true node like his W
    char mark = ' ';
    if (row < 10 && s.topocentric_parallax) {
      mark = 'P';
    } else if (row == 10) {
      mark = s.true_node ? 'W' : 'M';
    }
    char buf[48];
    std::snprintf(buf, sizeof(buf), "%s %s %7.1f", kRowTag[row],
                  zod(b.el, mark).c_str(), b.tb * kRadToDeg * 60.0);
    text(x0, y, buf);
    y += 12.5;
  }
  y += 4.0;
  dl.items.push_back({Primitive::Kind::kLine, x0, y, 190.0, y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF,
                      Primitive::Style::kSolid, 1.0, false, Primitive::Anchor::kCorner, ""});
  y += 14.0;
  //RR Häusersp., his house summary under the table
  if (chart.houses.ok) {
    text(x0, y, "H\xC3\xA4usersp.");
    y += 12.5;
    std::string hn(chart.houses.name.data(), chart.houses.name.size());
    while (!hn.empty() && hn.back() == ' ') {
      hn.pop_back();
    }
    text(x0, y, "(" + hn + ")");
    y += 12.5;
    static constexpr const char* kHouseTag[6] = {" AC", "H 2", "H 3", " MC", "H11", "H12"};
    static constexpr int kHouseIdx[6] = {1, 2, 3, 10, 11, 12};
    for (int i = 0; i < 6; ++i) {
      const double c = chart.houses.cusp[static_cast<std::size_t>(kHouseIdx[i])];
      const double deg = norm_deg(c * kRadToDeg);
      int sg = static_cast<int>(deg / 30.0);
      const double in_sign = deg - sg * 30.0;
      int d = static_cast<int>(in_sign);
      int m = static_cast<int>((in_sign - d) * 60.0 + 0.5);
      if (m == 60) {
        m = 0;
        if (++d == 30) {
          d = 0;
          sg = (sg + 1) % 12;
        }
      }
      char buf[40];
      std::snprintf(buf, sizeof(buf), "%s:%2d\xC2\xB0 %s %2d'", kHouseTag[i], d, kSign3[sg], m);
      text(x0, y, buf);
      y += 12.5;
    }
  }
  y += 4.0;
  dl.items.push_back({Primitive::Kind::kLine, x0, y, 190.0, y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF,
                      Primitive::Style::kSolid, 1.0, false, Primitive::Anchor::kCorner, ""});
  y += 14.0;
  //RR Spiegelung:
  text(x0, y, "Spiegelung:");

  // the record corners of his HOROSKOP GRAPHIK screen
  if (!txt.name.empty()) {
    text(205.0, 16.0, "Name: " + txt.name);
  }
  if (!txt.mode.empty()) {
    text(360.0, 16.0, txt.mode);
  }
  if (!txt.stz.empty()) {
    text(500.0, 16.0, txt.stz);
  }
  text(205.0, 420.0, "Ort:");
  if (!txt.place.empty()) {
    text(205.0, 432.0, txt.place);
  }
  if (!txt.lon.empty()) {
    text(205.0, 444.0, txt.lon);
  }
  if (!txt.lat.empty()) {
    text(205.0, 456.0, txt.lat);
  }
  if (!txt.date.empty()) {
    text(470.0, 432.0, txt.date);
  }
  if (!txt.ut.empty()) {
    text(470.0, 444.0, txt.ut);
  }
  if (!txt.weekday.empty()) {
    text(470.0, 456.0, txt.weekday);
  }
}

const char* body_glyph(int slot) {
  return (slot >= 0 && slot < body::kSlotCount) ? kBodyGlyph[slot] : "";
}

const char* sign_glyph(int index) {
  return (index >= 0 && index < 12) ? kSignGlyph[index] : "";
}

}  // namespace horcom
