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
constexpr double kGlyphSize = 11.0;
constexpr double kNumberSize = 9.0;
constexpr double kAxisTextSize = 12.0;
// the transit ring of a20, glyphs from plein1 and markers from plmk
constexpr double kTransitGlyphRing = 212.0;
// the comparison ring of a12, the second chart of the double wheel
constexpr double kCompareGlyphRing = 204.0;
constexpr double kCompareMarkRing = 180.0;
constexpr double kMarkInset = 3.0;
constexpr double kMarkOutset = 5.0;
constexpr double kLabelSize = 10.0;
// his screen drew rings, cusp lines and ticks two pixels wide on a
// 624 pixel radius, about 0.6 units here
constexpr double kThinLine = 0.6;

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
    //RR eigfarb!, own ring colours from hor_farb outrank the shades
    const std::size_t elem = static_cast<std::size_t>((j - 1) % 4);
    sec.fill = opt.ring_colors[elem + 1] != 0 ? opt.ring_colors[elem + 1] : kElementColor[elem];
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
    c.width = kThinLine;
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
    Primitive sl{Primitive::Kind::kLine, a.x, a.y, b.x, b.y};
    sl.width = kThinLine;
    add(sl);
  }
  // sign glyphs centred in their sign like zein1, the dial shows the
  // three qualities as their first signs
  for (int j = 1; j <= signs; ++j) {
    const double w = wheel_angle((j - 0.5) * span, fza);
    const Pt p = at(w, kSignGlyphRing);
    // zeichp_dspl stamps the sign sprite SRCCOPY, the white sprite
    // ground rides along as a small box on the coloured band
    Primitive box;
    box.kind = Primitive::Kind::kRect;
    box.x1 = p.x;
    box.y1 = p.y;
    box.r1 = kGlyphSize * 0.55;
    box.r2 = kGlyphSize * 0.55;
    box.fill = 0xFFFFFF;
    add(box);
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
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF, Primitive::Style::kSolid, 1.3});
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
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF, Primitive::Style::kSolid, 1.3});
      const Pt pl = at(w, kAxisLabel);
      Primitive t;
      t.kind = Primitive::Kind::kText;
      t.x1 = pl.x;
      t.y1 = pl.y;
      t.size = kAxisTextSize;
      // his layout numbers only AC and MC, DC and IC mirror them and
      // carry the bare tag, rounded to the nearest like planziff1
      t.text = kAxisLabelText[a];
      add(t);
      if (a == 0 || a == 3) {
        // habes stacks the degree centered below the tag
        const double q = norm_deg(cusp * kRadToDeg);
        Primitive n = t;
        n.size = kNumberSize;
        n.y1 += (kAxisTextSize + kNumberSize) * 0.5 + 1.0;
        n.text = std::to_string(static_cast<int>(std::lround(q - 30.0 * std::floor(q / 30.0))));
        add(n);
      }
    }
  }
  if (houses_drawn) {
    for (int i : {2, 3, 5, 6, 8, 9, 11, 12}) {
      const double w = wheel_angle(chart.houses.cusp[static_cast<std::size_t>(i)], fza);
      const Pt p1 = at(w, kAspectRing);
      const Pt p2 = at(w, kSignInner);
      Primitive hl{Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y};
      hl.width = kThinLine;
      add(hl);
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

  // ticks and glyphs like plein1 and plein11. The paper cutouts of all
  // glyphs land first, then every symbol, so crowded neighbours never
  // erase each other.
  // the SRCINVERT set of bmp_color_pl, both geb_herr rulers, the true
  // node pair under moknw and the true apogee Lilith under apogw
  const auto inverted_slot = [&opt](int slot) {
    if (slot == opt.ruler_slot || slot == opt.ruler_slot2) {
      return true;
    }
    if ((slot == body::kNodeAsc || slot == body::kNodeDesc) && opt.invert_nodes) {
      return true;
    }
    return slot == body::kApogee && opt.invert_apogee;
  };
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const double w_true = wheel_angle(pl[si], fza);
    // the two position marks of plein11, r 150 and, with aspects on,
    // r 90 on the aspect circle, each spanning r minus 3 to r plus 5
    const Pt tick1 = at(w_true, kTickRing - kMarkInset);
    const Pt tick2 = at(w_true, kTickRing + kMarkOutset);
    Primitive tk{Primitive::Kind::kLine, tick1.x, tick1.y, tick2.x, tick2.y};
    tk.width = kThinLine;
    add(tk);
    if (opt.aspect_lines) {
      const Pt in1 = at(w_true, kAspectRing - kMarkInset);
      const Pt in2 = at(w_true, kAspectRing + kMarkOutset);
      Primitive ik{Primitive::Kind::kLine, in1.x, in1.y, in2.x, in2.y};
      ik.width = kThinLine;
      add(ik);
    }
    // the paper cutouts, his putbm sprites erased the lines beneath
    // with their white background. The inverted patch pushes its
    // number and R marker a little further out.
    const Pt g = at(wl[si], kGlyphRing + dc[si]);
    const bool inv = inverted_slot(slot);
    add({Primitive::Kind::kDot, g.x, g.y, 0, 0, kGlyphSize * 0.60, 0, 0, 0, 0, kPaper});
    if (chart.b[si].tb < 0.0 && slot >= 3 && slot <= 10) {
      add({Primitive::Kind::kDot, g.x + kGlyphSize * (inv ? 1.05 : 0.85), g.y - kGlyphSize * 0.3,
           0, 0, kNumberSize * 0.6, 0, 0, 0, 0, kPaper});
    }
    if (opt.degree_numbers) {
      // planziff hangs the number below the glyph on screen, one glyph
      // height under its centre, never into the radial stack
      add({Primitive::Kind::kDot, g.x, g.y + kGlyphSize + (inv ? 2.5 : 1.0), 0, 0,
           kNumberSize * 0.75, 0, 0, 0, 0, kPaper});
    }
  }
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const Pt g = at(wl[si], kGlyphRing + dc[si]);
    // nodes and the birth ruler sit inverted on a dark patch, the
    // putbm SRCINVERT stamping of rulers and nodes
    const bool inverted = inverted_slot(slot);
    if (inverted) {
      Primitive box;
      box.kind = Primitive::Kind::kDot;
      box.x1 = g.x;
      box.y1 = g.y;
      // measured over every sprite, the widest ink reaches 0.66 of
      // the glyph size, the patch clears it with a small margin
      box.r1 = kGlyphSize * 0.68;
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
  }
  // numbers and retrograde marks last, they stay readable over any
  // crowded neighbour glyph
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const Pt g = at(wl[si], kGlyphRing + dc[si]);
    const bool inv = inverted_slot(slot);
    // the red R of his retrograde marker beside the glyph
    if (chart.b[si].tb < 0.0 && slot >= 3 && slot <= 10) {
      Primitive r;
      r.kind = Primitive::Kind::kText;
      r.x1 = g.x + kGlyphSize * (inv ? 1.05 : 0.85);
      r.y1 = g.y - kGlyphSize * 0.3;
      r.size = kNumberSize;
      r.color = 0xFF0000;
      r.text = "R";
      add(r);
    }
    if (opt.degree_numbers) {
      Primitive num;
      num.kind = Primitive::Kind::kText;
      num.x1 = g.x;
      num.y1 = g.y + kGlyphSize + (inv ? 2.5 : 1.0);
      num.size = kNumberSize;
      //RR CINT, planziff1 rounds the degree in sign to the nearest
      const double q = norm_deg(pl[si] * kRadToDeg);
      num.text = std::to_string(static_cast<int>(std::lround(q - 30.0 * std::floor(q / 30.0))));
      add(num);
    }
  }

  // node axis, blue dashed like the Mondknotenlinie
  if (opt.node_axis && chart.b[body::kNodeAsc].present) {
    const double w1 = wheel_angle(chart.b[body::kNodeAsc].el, fza);
    const double w2 = wheel_angle(chart.b[body::kNodeDesc].el, fza);
    const Pt a = at(w1, kAspectRing);
    const Pt b = at(w2, kAspectRing);
    add({Primitive::Kind::kLine, a.x, a.y, b.x, b.y, 0, 0, 0, 0, 0, 0x0000FF, 0xFFFFFF, Primitive::Style::kDashed, 0.7});
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
        add({Primitive::Kind::kDot, p.x, p.y, 0, 0, 2.0, 0, 0, 0, 0, 0xFF0000});
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
      line.width = 0.7;
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
  // cutouts first, then the symbols, then the small texts on top, the
  // same three passes as the radix ring
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const double w_true = wheel_angle(pl[si], fza);
    const Pt m1 = at(w_true, mark_ring - kMarkInset);
    const Pt m2 = at(w_true, mark_ring + kMarkOutset);
    Primitive mk{Primitive::Kind::kLine, m1.x, m1.y, m2.x, m2.y};
    mk.width = kThinLine;
    add(mk);
    const Pt g = at(wl[si], glyph_ring + dc[si]);
    add({Primitive::Kind::kDot, g.x, g.y, 0, 0, kGlyphSize * 0.60, 0, 0, 0, 0, kPaper});
    if (chart.b[si].tb < 0.0 && slot >= 3 && slot <= 10) {
      add({Primitive::Kind::kDot, g.x + kGlyphSize * 0.85, g.y - kGlyphSize * 0.3, 0, 0,
           kNumberSize * 0.6, 0, 0, 0, 0, kPaper});
    }
    if (opt.degree_numbers) {
      add({Primitive::Kind::kDot, g.x, g.y + kGlyphSize + 1.0, 0, 0, kNumberSize * 0.75, 0, 0, 0, 0,
           kPaper});
    }
  }
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const Pt g = at(wl[si], glyph_ring + dc[si]);
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = g.x;
    p.y1 = g.y;
    p.size = kGlyphSize;
    p.text = kBodyGlyph[si];
    //RR hard&, ÄUßERE SYMBOLE wahlweise ROT oder BLAU färben
    if (opt.outer_color == 1) {
      p.color = 0xFF0000;
    } else if (opt.outer_color == 3) {
      p.color = 0x0000FF;
    }
    add(p);
  }
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const Pt g = at(wl[si], glyph_ring + dc[si]);
    if (chart.b[si].tb < 0.0 && slot >= 3 && slot <= 10) {
      Primitive r;
      r.kind = Primitive::Kind::kText;
      r.x1 = g.x + kGlyphSize * 0.85;
      r.y1 = g.y - kGlyphSize * 0.3;
      r.size = kNumberSize;
      r.color = 0xFF0000;
      r.text = "R";
      add(r);
    }
    if (opt.degree_numbers) {
      Primitive num;
      num.kind = Primitive::Kind::kText;
      num.x1 = g.x;
      num.y1 = g.y + kGlyphSize + 1.0;
      num.size = kNumberSize;
      //RR CINT, planziff1 rounds the degree in sign to the nearest
      const double q = norm_deg(pl[si] * kRadToDeg);
      num.text = std::to_string(static_cast<int>(std::lround(q - 30.0 * std::floor(q / 30.0))));
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
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF, Primitive::Style::kSolid, 1.3});
    }
  }
  if (!opt.heliocentric && !opt.dial && !(s.houses == HouseSystem::kAcMcOnly || s.houses == HouseSystem::kNone)) {
    for (int i : {2, 3, 5, 6, 8, 9, 11, 12}) {
      const double w = wheel_angle(outer.houses.cusp[static_cast<std::size_t>(i)], fza);
      const Pt p1 = at(w, kAspectRing);
      const Pt p2 = at(w, kSignInner);
      Primitive hl{Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y};
      hl.width = kThinLine;
      add(hl);
    }
  }
  draw_outer_bodies(dl, outer, fza, kKm, kCompareMarkRing, kCompareGlyphRing, opt);
  return dl;
}

DisplayList centered_sheet(const DisplayList& dl, double width) {
  DisplayList out = dl;
  // the wheel centre of the classic sheet moves to the middle of a
  // sheet whose width follows the view, so the paper meets the panels
  // without a dark gap. The wheel itself grows a little, the sheet has
  // no data column to leave room for, while the symbols keep their
  // size. The corner notes follow the sheet edges.
  const double cx = width / 2.0;
  const double cy = kCanvasHeight / 2.0;
  const double dx = cx - kCx;
  const double dy = cy - kCy;
  const double f = 1.055;
  out.width = width;
  for (Primitive& p : out.items) {
    if (p.anchor == Primitive::Anchor::kCorner && p.kind == Primitive::Kind::kText) {
      if (p.align_right) {
        p.x1 = width - 8.0;
      } else if (!p.align_left) {
        p.x1 = cx;
      }
      continue;
    }
    if (p.anchor != Primitive::Anchor::kSheet) {
      continue;
    }
    p.x1 = cx + (p.x1 + dx - cx) * f;
    p.y1 = cy + (p.y1 + dy - cy) * f;
    p.x2 = cx + (p.x2 + dx - cx) * f;
    p.y2 = cy + (p.y2 + dy - cy) * f;
    p.r1 *= f;
    p.r2 *= f;
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
  text(x0, 16.0, (txt.len_header.empty() ? std::string("L\xC3\xA4nge:") : txt.len_header) + mode);
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
                      Primitive::Style::kSolid, 1.0, false, false, Primitive::Anchor::kCorner, ""});
  y += 14.0;
  //RR Häusersp., his house summary under the table
  if (chart.houses.ok) {
    text(x0, y, txt.houses_header.empty() ? "H\xC3\xA4usersp." : txt.houses_header);
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
                      Primitive::Style::kSolid, 1.0, false, false, Primitive::Anchor::kCorner, ""});
  y += 14.0;
  //RR Spiegelung:
  text(x0, y, txt.mirror_label.empty() ? "Spiegelung:" : txt.mirror_label);

  add_corner_text(dl, txt, 205.0, 383.0, kCanvasWidth - 8.0);
  // the DC axis label sits close to the sheet edge on the classic
  // layout, pin overflowing centred labels inside the paper
  for (Primitive& p : dl.items) {
    if (p.anchor == Primitive::Anchor::kSheet && p.kind == Primitive::Kind::kText &&
        !p.align_left && !p.align_right && p.x1 > kCanvasWidth - 24.0) {
      p.align_right = true;
      p.x1 = kCanvasWidth - 2.0;
    }
  }
}

void add_corner_text(DisplayList& dl, const ClassicSheetText& txt, double left_x, double center_x, double right_x) {
  enum class Align { kLeft, kCenter, kRight };
  auto text = [&](double x, double y, std::string t, Align a = Align::kLeft) {
    Primitive p;
    p.kind = Primitive::Kind::kText;
    p.x1 = x;
    p.y1 = y;
    p.size = 11.0;
    p.align_left = a == Align::kLeft;
    p.align_right = a == Align::kRight;
    p.anchor = Primitive::Anchor::kCorner;
    p.text = std::move(t);
    dl.items.push_back(std::move(p));
  };
  // the record corners of his HOROSKOP GRAPHIK screen, the name top
  // left, the place bottom left, the moment bottom right
  text(left_x, 16.0, txt.name_label.empty() ? "Name:" : txt.name_label);
  if (!txt.name.empty()) {
    text(left_x, 28.0, txt.name);
  }
  if (!txt.mode.empty()) {
    text(center_x, 16.0, txt.mode, Align::kCenter);
  }
  if (!txt.stz.empty()) {
    text(right_x, 16.0, txt.stz, Align::kRight);
  }
  text(left_x, 420.0, txt.place_label.empty() ? "Ort:" : txt.place_label);
  if (!txt.place.empty()) {
    text(left_x, 432.0, txt.place);
  }
  if (!txt.lon.empty()) {
    text(left_x, 444.0, txt.lon);
  }
  if (!txt.lat.empty()) {
    text(left_x, 456.0, txt.lat);
  }
  if (!txt.date.empty()) {
    text(right_x, 432.0, txt.date, Align::kRight);
  }
  if (!txt.ut.empty()) {
    text(right_x, 444.0, txt.ut, Align::kRight);
  }
  if (!txt.weekday.empty()) {
    text(right_x, 456.0, txt.weekday, Align::kRight);
  }
}

// ported from a11 in moda 3, the DIN A4 page, tables first and the
// wheel over them, Reihenfolge wichtig wie im Original
DisplayList a4_print_sheet(const Chart& chart, const ChartSettings& s, const AspectResult& aspects,
                           const MidpointResult& midpoints, const ClassicSheetText& txt,
                           const WheelOptions& opt) {
  DisplayList dl;
  dl.width = 640.0;
  dl.height = 980.0;
  auto add = [&](Primitive p) { dl.items.push_back(std::move(p)); };
  auto text = [&](double x, double y, std::string str, double size = 9.5) {
    Primitive p;
    p.kind = Primitive::Kind::kText;
    p.x1 = x;
    p.y1 = y;
    p.size = size;
    p.align_left = true;
    p.text = std::move(str);
    add(p);
  };
  auto line = [&](double x1, double y1, double x2, double y2) {
    add({Primitive::Kind::kLine, x1, y1, x2, y2, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF,
         Primitive::Style::kSolid, 0.8});
  };
  auto box = [&](double x1, double y1, double x2, double y2) {
    line(x1, y1, x2, y1);
    line(x2, y1, x2, y2);
    line(x2, y2, x1, y2);
    line(x1, y2, x1, y1);
  };
  static constexpr const char* kSign3[12] = {"AR", "TA", "GM", "CN", "LE", "VI",
                                             "LI", "SC", "SG", "CP", "AQ", "PS"};
  const auto zodsec = [](double rad) {
    const double deg = norm_deg(rad * kRadToDeg);
    int sg = static_cast<int>(deg / 30.0);
    const double in_sign = deg - sg * 30.0;
    int total = static_cast<int>(in_sign * 3600.0 + 0.5);
    if (total >= 30 * 3600) {
      total = 0;
      sg = (sg + 1) % 12;
    }
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%2d %s %2d'%2d\"", total / 3600, kSign3[sg], (total / 60) % 60,
                  total % 60);
    return std::string(buf);
  };
  const auto tag = [](int slot) {
    const std::string_view v = body::kName[static_cast<std::size_t>(slot)];
    return std::string(v.data(), v.size());
  };

  //RR boxn(1,1,639,979), the page frame
  box(1.0, 1.0, 639.0, 979.0);

  //RR bes1_big, the header block
  const double header = 10.5;
  if (!txt.name.empty()) {
    text(16.0, 18.0, txt.name_label.empty() ? "Name:" : txt.name_label, header);
    text(16.0, 30.0, txt.name, header);
  }
  text(240.0, 18.0, s.heliocentric ? "Heliozentr." : txt.stz, 9.5);
  {
    std::string hn(chart.houses.name.data(), chart.houses.name.size());
    while (!hn.empty() && hn.back() == ' ') {
      hn.pop_back();
    }
    text(240.0, 30.0, "RADIX, " + hn, 9.0);
    text(240.0, 42.0, txt.mode, 9.0);
  }
  if (!s.heliocentric) {
    text(16.0, 44.0, txt.place_label.empty() ? "Ort:" : txt.place_label, 9.0);
    text(16.0, 56.0, txt.place, 9.0);
    text(16.0, 68.0, txt.lon, 9.0);
    text(16.0, 80.0, txt.lat, 9.0);
  }
  text(452.0, 18.0, txt.date, header);
  text(452.0, 34.0, txt.ut, header);
  text(452.0, 48.0, txt.weekday, 9.0);

  // the sign counts of bes_big_kafige and bes_big_elem, plain body
  // counts per quality and element, his pn weight table stays out here
  {
    int quality[3] = {0, 0, 0};
    int element[4] = {0, 0, 0, 0};
    for (int slot = 1; slot <= 14; ++slot) {
      const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
      if (!b.present || !b.valid) {
        continue;
      }
      const int sg = static_cast<int>(norm_deg(b.el * kRadToDeg) / 30.0) % 12;
      ++quality[sg % 3];
      ++element[sg % 4];
    }
    //RR KARDINAL - FIX - GEMISCHT
    double y = 100.0;
    text(16.0, y, "Zeichen-Quali:", 8.5);
    static constexpr const char* kQuali[3] = {"Kardinal", "Fix", "Gemischt"};
    for (int i = 0; i < 3; ++i) {
      y += 12.0;
      char buf[24];
      std::snprintf(buf, sizeof(buf), "%-8s %2d", kQuali[i], quality[i]);
      text(16.0, y, buf, 8.5);
    }
    //RR Elemente F E L W with the little bars of elemhist1, the bars
    // wear the strong zeich_col shades, not the ring fills
    y = 490.0;
    text(16.0, y, "Elemente:", 8.5);
    static constexpr const char* kElemTag[4] = {"F", "E", "L", "W"};
    static constexpr Rgb kBarColor[4] = {0xFF0000, 0x808000, 0x008080, 0x00C8C8};
    for (int i = 0; i < 4; ++i) {
      y += 12.0;
      char buf[16];
      std::snprintf(buf, sizeof(buf), "%s %2d", kElemTag[i], element[i]);
      text(16.0, y, buf, 8.5);
      Primitive bar;
      bar.kind = Primitive::Kind::kRect;
      bar.x1 = 40.0 + element[i] * 2.0;
      bar.y1 = y - 3.0;
      bar.r1 = element[i] * 2.0;
      bar.r2 = 3.5;
      bar.fill = kBarColor[i];
      bar.color = kBarColor[i];
      add(bar);
    }
  }

  //RR bes_big_plan, the position list in its box, @boxn(1,610,212,960)
  {
    box(1.0, 610.0, 212.0, 960.0);
    const char* mode = s.apparent == ApparentMode::kLightTime             ? "(App1)"
                       : s.apparent == ApparentMode::kLightTimeAberration ? "(App2)"
                                                                          : "(Wahr)";
    text(18.0, 624.0, std::string("Ekl.L\xC3\xA4nge:") + mode + "   Vel.", 9.0);
    double y = 624.0;
    for (int slot = 1; slot < body::kSlotCount; ++slot) {
      if (slot == body::kAscendant || slot == body::kMc) {
        continue;
      }
      const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
      if (!b.present || !b.valid) {
        continue;
      }
      y += 15.0;
      if (y > 950.0) {
        //RR Nur MAXIMAL 12 ZUSATZ-Planeten hier darstellbar
        text(18.0, y, "...", 9.0);
        break;
      }
      //RR p bei Parallaxe, w wahrer, m mittlerer Wert
      char mark = ' ';
      if (slot <= 6 && s.topocentric_parallax) {
        mark = 'p';
      } else if (slot == body::kNodeAsc || slot == body::kNodeDesc) {
        mark = s.true_node ? 'w' : 'm';
      } else if (slot == body::kApogee) {
        mark = s.true_apogee ? 'w' : 'm';
      }
      char buf[64];
      if (b.tb != 0.0) {
        std::snprintf(buf, sizeof(buf), "%-2s %s %c %6.1f", tag(slot).c_str(), zodsec(b.el).c_str(),
                      mark, b.tb * kRadToDeg * 60.0);
      } else {
        std::snprintf(buf, sizeof(buf), "%-2s %s %c", tag(slot).c_str(), zodsec(b.el).c_str(), mark);
      }
      text(8.0, y, buf, 9.0);
    }
  }

  //RR bes_big_asp, the aspect list and the Spiegelungen under it
  {
    box(212.0, 650.0, 386.0, 960.0);
    static constexpr const char* kAspTag[13] = {"",    "kon", "opp", "tri", "qua", "qui", "sex",
                                                "sep", "hqu", "nov", "dez", "elf", "qcx"};
    double y = 664.0;
    double x = 220.0;
    text(x, y, "Aspekte:", 9.0);
    for (const AspectHit& h : aspects.hits) {
      if (h.n < 1 || h.n > 12) {
        continue;
      }
      y += 12.0;
      if (y > 920.0 && x < 300.0) {
        // the second column of his 174 unit box
        x = 302.0;
        y = 676.0;
      } else if (y > 920.0) {
        text(x, y, "...", 9.0);
        break;
      }
      char buf[24];
      std::snprintf(buf, sizeof(buf), "%-2s %s %-2s", tag(h.t).c_str(), kAspTag[h.n],
                    tag(h.w).c_str());
      text(x, y, buf, 8.5);
    }
    //RR spieg1, the mirror contacts close the box
    std::string mirrors;
    for (const AspectHit& h : aspects.hits) {
      if (h.n != 13) {
        continue;
      }
      if (!mirrors.empty()) {
        mirrors += "  ";
      }
      mirrors += tag(h.t) + "/" + tag(h.w);
    }
    if (!mirrors.empty()) {
      line(212.0, 936.0, 386.0, 936.0);
      text(220.0, 948.0,
           (txt.mirror_label.empty() ? std::string("Spiegelung:") : txt.mirror_label) + " " +
               mirrors.substr(0, 32),
           8.0);
    }
  }

  //RR bes_big_haus, the house cusp box
  if (!s.heliocentric && chart.houses.ok) {
    box(386.0, 655.0, 540.0, 960.0);
    text(398.0, 669.0, "H\xC3\xA4userspitzen", 9.0);
    std::string hn(chart.houses.name.data(), chart.houses.name.size());
    while (!hn.empty() && hn.back() == ' ') {
      hn.pop_back();
    }
    text(398.0, 681.0, "(" + hn + ")", 9.0);
    double y = 686.0;
    for (int i = 1; i <= 12; ++i) {
      y += 16.0;
      char head[8];
      const char* h = i == 1 ? "AC " : i == 4 ? "IC " : i == 7 ? "DC " : i == 10 ? "MC " : nullptr;
      if (h == nullptr) {
        std::snprintf(head, sizeof(head), "H%-2d", i);
        h = head;
      }
      text(398.0, y, std::string(h) + " " + zodsec(chart.houses.cusp[static_cast<std::size_t>(i)]),
           9.0);
    }
  }

  //RR bes_big_halbs, Direkt, Quadrat und Halbquad
  {
    box(540.0, 548.0, 639.0, 960.0);
    double y = 562.0;
    text(546.0, y, "Halbsummen", 8.5);
    line(540.0, y + 3.0, 639.0, y + 3.0);
    static constexpr int kFamily[3] = {1, 2, 4};
    static constexpr const char* kFamilyName[3] = {"Direkt:", "Quadrat:", "Halbquad:"};
    for (int f = 0; f < 3; ++f) {
      y += 14.0;
      if (y > 940.0) {
        break;
      }
      text(546.0, y, kFamilyName[f], 8.5);
      line(540.0, y + 3.0, 639.0, y + 3.0);
      for (const MidpointHit& h : midpoints.hits) {
        if (h.nh != kFamily[f]) {
          continue;
        }
        y += 11.0;
        if (y > 940.0) {
          text(546.0, y, "...", 8.0);
          break;
        }
        char buf[24];
        std::snprintf(buf, sizeof(buf), "%s=%s/%s", tag(h.t).c_str(), tag(h.u).c_str(),
                      tag(h.w).c_str());
        text(546.0, y, buf, 8.0);
      }
      y += 4.0;
    }
  }

  // the wheel over the tables, centred at 340 like amh& = bmh& = 340
  {
    WheelOptions wopt = opt;
    //RR MUL km,1.48
    wopt.scale = kKm * 1.48;
    DisplayList wheel = build_wheel(chart, s, aspects, wopt);
    const double dx = 340.0 - kCx;
    const double dy = 340.0 - kCy;
    for (Primitive p : wheel.items) {
      if (p.anchor != Primitive::Anchor::kSheet) {
        continue;
      }
      p.x1 += dx;
      p.y1 += dy;
      if (p.kind == Primitive::Kind::kLine) {
        p.x2 += dx;
        p.y2 += dy;
      }
      add(std::move(p));
    }
  }

  //RR drad2, the credit line at the page foot with its separator box
  line(1.0, 960.0, 639.0, 960.0);
  {
    Primitive credit;
    credit.kind = Primitive::Kind::kText;
    credit.x1 = 320.0;
    credit.y1 = 972.0;
    credit.size = 7.0;
    credit.color = 0x808080;
    credit.text = "HORCOM \xC2\xB7 Robert Rettig \xC2\xB7 \xC2\xA9 Dominik Schwimmbeck";
    add(std::move(credit));
  }
  return dl;
}

const char* body_glyph(int slot) {
  return (slot >= 0 && slot < body::kSlotCount) ? kBodyGlyph[slot] : "";
}

const char* sign_glyph(int index) {
  return (index >= 0 && index < 12) ? kSignGlyph[index] : "";
}

}  // namespace horcom
