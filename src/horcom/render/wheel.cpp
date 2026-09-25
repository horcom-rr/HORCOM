// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/wheel.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

#include "horcom/chart/signs.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/konsta.hpp"

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
// km * 154, the ring of the Rhythmenlehre phase arc
constexpr double kPhaseArcRing = 154.0;
// the arc runs as short chords, two degrees each
constexpr double kPhaseArcStep = kPi / 90.0;
constexpr double kConjDotRing = 85.0;  // the original red conjunction ring
constexpr double kConjRingRadius = 3.0;
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
constexpr double kChartLabelSize = 10.0;
// below the sun mark of the hrg centre, its outer circle has radius 7
constexpr double kHelioLabelDrop = 16.0;
// his screen drew rings, cusp lines and ticks two pixels wide on a
// 624 pixel radius, about 0.6 units here. Achsen zeichnet er über
// DEFLINE 0,2 dicker als das Grundraster
constexpr double kThinLine = 0.6;
constexpr double kAxisLine = 1.3;

// the degree within its sign rounded like planziff1 and habes. His CINT
// printed 30 for the last half degree of a sign, the port stops at 29
int degree_in_sign(double q_deg) {
  const long d = std::lround(q_deg - kDegPerSign * std::floor(q_deg / kDegPerSign));
  return static_cast<int>(std::min(d, static_cast<long>(kDegPerSign) - 1));
}

// the sign band of his own screen, fire, earth, air, water. His KONSTA
// ran the SCHRAFFIERT EIGENE FARBEN mode with cols% red, green, cyan and
// blue, and the hatch fills of deffi (patterns 5, 1, 7 and 3) rendered
// them as salmon, green, pale cyan grey and blue. These are the measured
// shades, drawn opaque.
constexpr Rgb kOwnColor[4] = {0xFF0000, 0x00FF00, 0x00FFFF, 0x0000FF};
constexpr Rgb kElementShade[4] = {0xFF9086, 0x14CD14, 0xDEF2F2, 0x4646FF};
// the ink share of each hatch pattern, read off the shades above on the
// channels his colour leaves empty, any other colour takes the same
// density
constexpr double kHatchInk[4] = {0.45, 0.92, 0.13, 0.73};
// pure cyan drowns on a white page, the A4 sheet prints it darker in
// step with element_color of the coordinate table
constexpr Rgb kCyan = 0x00FFFF;
constexpr Rgb kPrintCyan = 0x00C8C8;

// the colour of the element of sign band slot elem hatched like deffi
Rgb hatch_shade(std::size_t elem, Rgb c) {
  if (c == kOwnColor[elem]) {
    return kElementShade[elem];
  }
  const auto mix = [&](unsigned shift) {
    const double ink = static_cast<double>((c >> shift) & 0xFFu);
    return static_cast<Rgb>(std::lround(255.0 + kHatchInk[elem] * (ink - 255.0))) << shift;
  };
  return mix(16) | mix(8) | mix(0);
}

// RGB(255,0,0), RGB(0,200,0), RGB(0,0,200), RGB(0,0,0) of aspz1
constexpr Rgb kChordRed = 0xFF0000;
constexpr Rgb kChordGreen = 0x00C800;
constexpr Rgb kChordBlue = 0x0000C8;
constexpr Rgb kChordBlack = 0x000000;

// KLEIN-SYMBOLE, plsyver half widths 5 against 7 and the 0.7 of plentz11
constexpr double kSmallSymbolScale = 5.0 / 7.0;
constexpr double kSmallStagger = 0.7;

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

// ported from horbeg, the longitude that lands on the left of the wheel
double wheel_origin(const Chart& chart, const WheelOptions& opt) {
  if (opt.heliocentric) {
    return 0.0;
  }
  switch (opt.begin) {
    case 2: return chart.houses.angles.mc;
    case 3: return 0.0;
    case 4: return kPi;
    case 5: return opt.begin_lon;
    default: return chart.houses.angles.ac;
  }
}

// planziff1 marks the planets and every extra body, never the nodes
// and the angles
bool retro_slot(int slot) {
  return (slot >= body::kSun && slot <= body::kPluto) || slot >= body::kApogee;
}

// planziff2 writes the R in the colour of the outer symbols, hard&
Rgb retro_color(int outer_color) {
  switch (outer_color) {
    case 1: return 0xFF0000;
    case 3: return 0x0000FF;
    default: return 0x000000;
  }
}

// the original plentz chain, radial stagger and angular push apart of
// crowded glyphs. Operates on the display longitudes wl and offsets dc.
void declump(const std::vector<int>& slots, const std::array<double, body::kSlotCount>& pl, std::array<double, body::kSlotCount>& wl, std::array<double, body::kSlotCount>& dc, double shrink) {
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
      //RR IF klsy! dc&(kj&) = 0.7 * dc&(kj&)
      dc[static_cast<std::size_t>(k)] *= shrink;
      dc[static_cast<std::size_t>(l)] *= shrink;
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

// planziff1 stands the R of a retrograde body at the upper right of its
// glyph, in a crowded stretch of the ring it met the number or the glyph
// of a neighbour. Such an R turns with its paper patch to the upper left
// of its own glyph when that side is free, the glyphs and the numbers
// stay where plentz set them
void keep_retro_marks_clear(DisplayList& dl, std::size_t first) {
  struct Box {
    double l, t, r, b;
  };
  // a centred text of the fixed pitch face, a sprite of its size
  const auto text_box = [](const Primitive& p) {
    const double half = 0.5 * kMonoAdvance * p.size * static_cast<double>(p.text.size());
    return Box{p.x1 - half, p.y1 - 0.5 * p.size, p.x1 + half, p.y1 + 0.5 * p.size};
  };
  const auto glyph_box = [](const Primitive& p) {
    const double half = 0.5 * kSpriteBox * p.size;
    return Box{p.x1 - half, p.y1 - half, p.x1 + half, p.y1 + half};
  };
  constexpr double kClear = 0.5;
  const auto hit = [](const Box& a, const Box& b) {
    return a.l < b.r - kClear && b.l < a.r - kClear && a.t < b.b - kClear && b.t < a.b - kClear;
  };
  std::vector<std::size_t> marks;
  std::vector<std::size_t> numbers;
  std::vector<std::size_t> glyphs;
  for (std::size_t i = first; i < dl.items.size(); ++i) {
    const Primitive& p = dl.items[i];
    if (p.kind == Primitive::Kind::kGlyph) {
      glyphs.push_back(i);
    } else if (p.kind == Primitive::Kind::kText && p.size == kNumberSize) {
      (p.text == "R" ? marks : numbers).push_back(i);
    }
  }
  for (const std::size_t m : marks) {
    Primitive& r = dl.items[m];
    // its own glyph is the nearest one
    std::size_t own = glyphs.empty() ? m : glyphs.front();
    for (const std::size_t g : glyphs) {
      if (std::hypot(dl.items[g].x1 - r.x1, dl.items[g].y1 - r.y1) <
          std::hypot(dl.items[own].x1 - r.x1, dl.items[own].y1 - r.y1)) {
        own = g;
      }
    }
    const auto blocked = [&](const Box& b) {
      for (const std::size_t n : numbers) {
        if (hit(b, text_box(dl.items[n]))) {
          return true;
        }
      }
      for (const std::size_t o : marks) {
        if (o != m && hit(b, text_box(dl.items[o]))) {
          return true;
        }
      }
      for (const std::size_t g : glyphs) {
        if (g != own && hit(b, glyph_box(dl.items[g]))) {
          return true;
        }
      }
      return false;
    };
    if (glyphs.empty() || !blocked(text_box(r))) {
      continue;
    }
    const double dx = 2.0 * (dl.items[own].x1 - r.x1);
    Primitive left = r;
    left.x1 += dx;
    if (blocked(text_box(left))) {
      continue;
    }
    // the paper patch under the R moves along
    for (std::size_t i = first; i < dl.items.size(); ++i) {
      Primitive& patch = dl.items[i];
      if (patch.kind == Primitive::Kind::kDot && patch.color == kPaperColor && patch.x1 == r.x1 && patch.y1 == r.y1) {
        patch.x1 += dx;
      }
    }
    r.x1 += dx;
  }
}

}  // namespace

Rgb ring_fill_color(int element, Rgb color, RingFill fill) {
  switch (fill) {
    case RingFill::kShaded: return hatch_shade(static_cast<std::size_t>(std::clamp(element, 1, 4) - 1), color);
    case RingFill::kSolid: return color;
    case RingFill::kWhite: break;
  }
  return kPaperColor;
}

// the shared wheel body, drawn at the given km so the radix wheel and
// the smaller a20 transit wheel reuse the same geometry
static void build_base(DisplayList& dl, const Chart& chart, const ChartSettings& s, const AspectResult& aspects, const WheelOptions& opt, double km) {
  auto add = [&](Primitive p) { dl.items.push_back(std::move(p)); };
  const auto at = [km](double w, double r) {
    return Pt{kCx + km * r * std::cos(-w), kCy + km * r * std::sin(-w)};
  };

  // the rotation origin of horbeg, the default begz& = 1 puts the AC
  // left, the hrg mode anchors zero Aries
  const double fza = wheel_origin(chart, opt);

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
    // zein fills with the brush of fill_color, hatched, solid or not at
    // all in the WEIß and NUR SYMBOLE modes
    const int elem = (j - 1) % 4 + 1;
    // fill_color under dop 4 hatches the third sector like air but takes
    // cols%(4), the water colour, when the own colours are on
    const int colour = (opt.dial && elem == 3 && opt.own_colors) ? 4 : elem;
    sec.fill = ring_fill_color(elem, opt.ring_colors[static_cast<std::size_t>(colour)], opt.ring_fill);
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
    box.r1 = kSpriteSize * 0.55;
    box.r2 = kSpriteSize * 0.55;
    box.fill = 0xFFFFFF;
    add(box);
    Primitive g;
    g.kind = Primitive::Kind::kGlyph;
    g.x1 = p.x;
    g.y1 = p.y;
    g.size = kSpriteSize;
    g.text = kSignGlyph[j - 1];
    //RR Symbole färben, bmp_color_ze under nursymb& = 1
    if (opt.colored_signs) {
      g.color = opt.ring_colors[static_cast<std::size_t>((j - 1) % 4) + 1];
    }
    add(g);
  }

  // houses like horg11, thick axes to 188 with labels at 200, thin
  // cusps, the whole block stays dark in the hrg mode like the original
  const bool houses_drawn = !opt.heliocentric && !opt.dial && s.houses < HouseSystem::kAcMcOnly;
  if (opt.dial) {
    // only AC and MC survive the times four, DC and IC land on them,
    // aeqh blanks their labels
    static constexpr const char* kDialLabel[2] = {"AC", "MC"};
    const int dial_axes[2] = {1, 10};
    for (int a = 0; a < 2; ++a) {
      const double w = wheel_angle(chart.houses.cusp[static_cast<std::size_t>(dial_axes[a])], fza);
      const Pt p1 = at(w, kAspectRing);
      const Pt p2 = at(w, kAxisEnd);
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF, Primitive::Style::kSolid, kAxisLine});
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
  if (!opt.heliocentric && !opt.dial && !without_angles(s.houses)) {
    // one thick axis with its tag, the degree stacked under AC and MC
    // like habes, rounded to the nearest like planziff1
    const auto axis = [&](double lon, const char* tag, bool numbered, Rgb ink, double lift = 0.0) {
      const double w = wheel_angle(lon, fza);
      const Pt p1 = at(w, kAspectRing);
      const Pt p2 = at(w, kAxisEnd);
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF, Primitive::Style::kSolid, kAxisLine});
      const Pt pl = at(w, kAxisLabel);
      Primitive t;
      t.kind = Primitive::Kind::kText;
      t.x1 = pl.x;
      t.y1 = pl.y + lift;
      t.size = kAxisTextSize;
      t.color = ink;
      t.text = tag;
      add(t);
      if (numbered) {
        const double q = norm_deg(lon * kRadToDeg);
        Primitive n = t;
        n.size = kNumberSize;
        n.y1 += (kAxisTextSize + kNumberSize) * 0.5 + 1.0;
        n.text = std::to_string(degree_in_sign(q));
        add(n);
      }
    };
    //RR aeqh, the equal systems name their cusp axes as houses in blue,
    // Äqual keeps AC and DC on the first and seventh cusp, Vehlow's
    // cusps sit fifteen degrees before the true angles
    const bool equal = s.houses == HouseSystem::kEqualAsc || s.houses == HouseSystem::kEqualVehlow;
    const bool vehlow = s.houses == HouseSystem::kEqualVehlow;
    static constexpr const char* kQuadrantTag[4] = {"AC", "IC", "DC", "MC"};
    static constexpr const char* kEqualTag[4] = {"AC", "H4", "DC", "H10"};
    static constexpr const char* kVehlowTag[4] = {"H1", "H4", "H7", "H10"};
    const char* const* tags = vehlow ? kVehlowTag : (equal ? kEqualTag : kQuadrantTag);
    //RR RGBCOLOR RGB(0,0,255) for haw& 6 and 7
    constexpr Rgb kEqualCuspInk = 0x0000FF;
    //RR IF comp! && (comp_mstz! OR comp_hand!), h$ = "H1 "
    const bool composite_h1 = opt.composite_axes && !equal;
    const int axes[4] = {1, 4, 7, 10};
    for (int a = 0; a < 4; ++a) {
      axis(chart.houses.cusp[static_cast<std::size_t>(axes[a])], (a == 0 && composite_h1) ? "H1" : tags[a],
           a == 0 || a == 3, equal ? kEqualCuspInk : 0x000000);
    }
    if (composite_h1 && chart.b[body::kAscendant].present) {
      // his AC and H1 tags shared x8 - 5, y8 + 5 and ran into one
      // another when the midpoint stood close to the cusp, the AC tag
      // then stacks above the H1 tag
      constexpr double kTagClash = 8.0 * kDegToRad;
      double apart = norm_rad(chart.b[body::kAscendant].el - chart.houses.cusp[1]);
      apart = std::min(apart, kTwoPi - apart);
      const double lift = apart < kTagClash ? -(kAxisTextSize + kNumberSize + 2.0) : 0.0;
      //RR w = FN nb(plz(0,1,13) + PI - fza), the AC midpoint with its degree
      axis(chart.b[body::kAscendant].el, "AC", true, 0x000000, lift);
    }
    if (equal) {
      //RR winkel_eckp and horg110, the true angles on top of the cusps
      const double ac = chart.houses.angles.ac;
      const double mc = chart.houses.angles.mc;
      axis(ac, "AC", true, 0x000000);
      axis(norm_rad(mc + kPi), "IC", false, 0x000000);
      axis(norm_rad(ac + kPi), "DC", false, 0x000000);
      axis(mc, "MC", true, 0x000000);
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
  declump(slots, pl, wl, dc, opt.small_symbols ? kSmallStagger : 1.0);
  const double gs = opt.small_symbols ? kSpriteSize * kSmallSymbolScale : kSpriteSize;

  // ticks and glyphs like plein1 and plein11. The paper cutouts of all
  // glyphs land first, then every symbol, so crowded neighbours never
  // erase each other.
  // the SRCINVERT set of bmp_color_pl, both geb_herr rulers, the true
  // node pair under moknw and the true apogee Lilith under apogw
  const auto inverted_slot = [&opt](int slot) {
    bool inv = slot == opt.ruler_slot || slot == opt.ruler_slot2;
    if ((slot == body::kNodeAsc || slot == body::kNodeDesc) && opt.invert_nodes) {
      inv = true;
    }
    if (slot == body::kApogee && opt.invert_apogee) {
      inv = true;
    }
    if (std::find(opt.flip_inverted.begin(), opt.flip_inverted.end(), slot) != opt.flip_inverted.end()) {
      inv = !inv;
    }
    return inv;
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
    add({Primitive::Kind::kDot, g.x, g.y, 0, 0, gs * 0.60, 0, 0, 0, 0, kPaperColor});
    if (opt.retro_marks && chart.b[si].tb < 0.0 && retro_slot(slot)) {
      add({Primitive::Kind::kDot, g.x + gs * (inv ? 1.05 : 0.85), g.y - gs * 0.3,
           0, 0, kNumberSize * 0.6, 0, 0, 0, 0, kPaperColor});
    }
    if (opt.degree_numbers) {
      // planziff hangs the number below the glyph on screen, one glyph
      // height under its centre, never into the radial stack
      add({Primitive::Kind::kDot, g.x, g.y + gs + (inv ? 2.5 : 1.0), 0, 0,
           kNumberSize * 0.75, 0, 0, 0, 0, kPaperColor});
    }
  }
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const Pt g = at(wl[si], kGlyphRing + dc[si]);
    // nodes and the birth ruler sit inverted on a dark patch, the
    // putbm SRCINVERT stamping of rulers and nodes
    const bool inverted = inverted_slot(slot);
    if (inverted) {
      add(inverted_patch(g.x, g.y, gs));
    }
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = g.x;
    p.y1 = g.y;
    p.size = gs;
    if (inverted) {
      p.color = kInvertedInk;
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
    // the R of his retrograde marker beside the glyph, pziff 1 only
    if (opt.retro_marks && chart.b[si].tb < 0.0 && retro_slot(slot)) {
      Primitive r;
      r.kind = Primitive::Kind::kText;
      r.x1 = g.x + gs * (inv ? 1.05 : 0.85);
      r.y1 = g.y - gs * 0.3;
      r.size = kNumberSize;
      r.color = retro_color(opt.outer_color);
      r.text = "R";
      add(r);
    }
    if (opt.degree_numbers) {
      Primitive num;
      num.kind = Primitive::Kind::kText;
      num.x1 = g.x;
      num.y1 = g.y + gs + (inv ? 2.5 : 1.0);
      num.size = kNumberSize;
      //RR CINT, planziff1 rounds the degree in sign to the nearest
      const double q = norm_deg(pl[si] * kRadToDeg);
      num.text = std::to_string(degree_in_sign(q));
      add(num);
    }
  }
  keep_retro_marks_clear(dl, 0);

  //RR apog!, DEFLINE 1,1 from the Black Moon through the centre
  if (opt.apogee_axis && opt.aspect_lines && chart.b[body::kApogee].present && chart.b[body::kApogee].valid) {
    const double w1 = wheel_angle(chart.b[body::kApogee].el, fza);
    const Pt a = at(w1, kAspectRing);
    const Pt b = at(norm_rad(w1 + kPi), kAspectRing);
    add({Primitive::Kind::kLine, a.x, a.y, b.x, b.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF, Primitive::Style::kDashed, 0.7});
  }
  // node axis, blue dashed like the Mondknotenlinie
  if (opt.node_axis && opt.aspect_lines && chart.b[body::kNodeAsc].present) {
    const double w1 = wheel_angle(chart.b[body::kNodeAsc].el, fza);
    const double w2 = wheel_angle(chart.b[body::kNodeDesc].el, fza);
    const Pt a = at(w1, kAspectRing);
    const Pt b = at(w2, kAspectRing);
    add({Primitive::Kind::kLine, a.x, a.y, b.x, b.y, 0, 0, 0, 0, 0, 0x0000FF, 0xFFFFFF, Primitive::Style::kDashed, 0.7});
  }

  // aspect chords on the inner ring like aspz0 and aspz1, gated per
  // row like the aspli flags of the ASPEKT-LINIEN screen
  if (opt.aspect_lines) {
    for (const AspectHit& h : aspects.hits) {
      const int row = h.n == 1 ? 0 : chord_row(h.n, h.m, opt.rhythm);
      if (h.n != 1 && (row == 0 || !opt.chords[static_cast<std::size_t>(row)].on)) {
        continue;
      }
      const double w1 = wheel_angle(chart.b[static_cast<std::size_t>(h.t)].el, fza);
      const double w2 = wheel_angle(chart.b[static_cast<std::size_t>(h.w)].el, fza);
      if (h.n == 1) {
        //RR @kreis(x1&,y1&,3,0,360), the conjunction is a red ring at 85
        double wa = (w1 + w2) / 2.0;
        if (std::abs(wa - w1) > 1.0 || std::abs(wa - w2) > 1.0) {
          wa += kPi;
        }
        const Pt p = at(wa, kConjDotRing);
        Primitive ring;
        ring.kind = Primitive::Kind::kCircle;
        ring.x1 = p.x;
        ring.y1 = p.y;
        ring.r1 = kConjRingRadius;
        ring.color = 0xFF0000;
        ring.width = kThinLine;
        add(ring);
        continue;
      }
      const Pt a = at(w1, kAspectRing);
      const Pt b = at(w2, kAspectRing);
      Primitive line{Primitive::Kind::kLine, a.x, a.y, b.x, b.y};
      const ChordLine& chord = opt.chords[static_cast<std::size_t>(row)];
      line.color = chord.color;
      line.style = chord.style;
      line.width = 0.7;
      add(line);
    }
  }

  // ported from a1795, the running phase's house marked on the sign
  // ring by a red arc of width two
  if (opt.phase_house >= 1 && opt.phase_house <= 12 && chart.houses.ok) {
    const double wa = wheel_angle(chart.houses.cusp[static_cast<std::size_t>(opt.phase_house)], fza);
    const double arc = norm_rad(chart.houses.cusp[static_cast<std::size_t>(opt.phase_house) + 1] -
                                 chart.houses.cusp[static_cast<std::size_t>(opt.phase_house)]);
    const int steps = std::max(1, static_cast<int>(std::ceil(arc / kPhaseArcStep)));
    for (int k = 0; k < steps; ++k) {
      const Pt p1 = at(wa + arc * k / steps, kPhaseArcRing);
      const Pt p2 = at(wa + arc * (k + 1) / steps, kPhaseArcRing);
      Primitive seg{Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y};
      seg.color = 0xFF0000;
      seg.width = 2.0;
      add(seg);
    }
  }

  // the credit line like drad2 stamped on every output, his name first.
  // The tester's mockup places the credit on the right below the moment
  // block so the left side of the sheet stays free for a Composit or
  // Combin mini table
  {
    Primitive credit;
    credit.kind = Primitive::Kind::kText;
    credit.x1 = kCanvasWidth - 8.0;
    credit.y1 = kCanvasHeight - 8.0;
    credit.size = 8.0;
    credit.color = 0x808080;
    credit.align_right = true;
    credit.anchor = Primitive::Anchor::kCredit;
    credit.text = std::string(kCreditLine);
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

  //RR bes2, sol$ at amh - 4 * LEN on the bottom line bmh + 5, size 10.
  // The hrg wheel carries the sun mark in its centre, his label went to
  // the top there, where the port's sheet shows the mode line, so it
  // stands under the sun mark instead
  if (!opt.chart_label.empty()) {
    Primitive t;
    t.kind = Primitive::Kind::kText;
    t.x1 = kCx;
    t.y1 = opt.heliocentric ? kCy + kHelioLabelDrop : kCy;
    t.size = kChartLabelSize;
    t.text = opt.chart_label;
    add(t);
    if (!opt.chart_sub_label.empty()) {
      //RR bes2_comp at bmh + 15
      Primitive sub = t;
      sub.y1 += kChartLabelSize;
      sub.text = opt.chart_sub_label;
      add(sub);
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
    if ((slot == body::kAscendant || slot == body::kMc) && !opt.outer_axes) {
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
  declump(slots, pl, wl, dc, opt.small_symbols ? kSmallStagger : 1.0);
  const double gs = opt.small_symbols ? kSpriteSize * kSmallSymbolScale : kSpriteSize;
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
    add({Primitive::Kind::kDot, g.x, g.y, 0, 0, gs * 0.60, 0, 0, 0, 0, kPaperColor});
    if (opt.retro_marks && chart.b[si].tb < 0.0 && retro_slot(slot)) {
      add({Primitive::Kind::kDot, g.x + gs * 0.85, g.y - gs * 0.3, 0, 0,
           kNumberSize * 0.6, 0, 0, 0, 0, kPaperColor});
    }
    if (opt.degree_numbers) {
      add({Primitive::Kind::kDot, g.x, g.y + gs + 1.0, 0, 0, kNumberSize * 0.75, 0, 0, 0, 0,
           kPaperColor});
    }
  }
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const Pt g = at(wl[si], glyph_ring + dc[si]);
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = g.x;
    p.y1 = g.y;
    p.size = gs;
    p.text = body_glyph(slot);
    // planp_dspl asks plw& under plan_col! first, a marked running factor
    // stays red whatever hard& holds
    const bool marked =
        std::find(opt.outer_marked.begin(), opt.outer_marked.end(), slot) != opt.outer_marked.end();
    //RR hard&, ÄUßERE SYMBOLE wahlweise ROT oder BLAU färben
    if (marked || (opt.outer_tint && opt.outer_color == 1)) {
      p.color = 0xFF0000;
    } else if (opt.outer_tint && opt.outer_color == 3) {
      p.color = 0x0000FF;
    }
    add(p);
  }
  for (int slot : slots) {
    const auto si = static_cast<std::size_t>(slot);
    const Pt g = at(wl[si], glyph_ring + dc[si]);
    if (opt.retro_marks && chart.b[si].tb < 0.0 && retro_slot(slot)) {
      Primitive r;
      r.kind = Primitive::Kind::kText;
      r.x1 = g.x + gs * 0.85;
      r.y1 = g.y - gs * 0.3;
      r.size = kNumberSize;
      r.color = retro_color(opt.outer_color);
      r.text = "R";
      add(r);
    }
    if (opt.degree_numbers) {
      Primitive num;
      num.kind = Primitive::Kind::kText;
      num.x1 = g.x;
      num.y1 = g.y + gs + 1.0;
      num.size = kNumberSize;
      //RR CINT, planziff1 rounds the degree in sign to the nearest
      const double q = norm_deg(pl[si] * kRadToDeg);
      num.text = std::to_string(degree_in_sign(q));
      add(num);
    }
  }
  keep_retro_marks_clear(dl, 0);
}

DisplayList build_transit_wheel(const Chart& radix, const Chart& transit, const ChartSettings& s, const AspectResult& radix_aspects, const WheelOptions& opt) {
  DisplayList dl;
  build_base(dl, radix, s, radix_aspects, opt, kTransitWheelScale);
  // the radix rules the rotation, the running sky turns with it
  const double fza = wheel_origin(radix, opt);
  draw_outer_bodies(dl, transit, fza, kTransitWheelScale, kSignOuter, kTransitGlyphRing, opt);
  return dl;
}

DisplayList build_double_wheel(const Chart& inner, const Chart& outer, const ChartSettings& s, const AspectResult& inner_aspects, const WheelOptions& opt) {
  DisplayList dl;
  //RR km = 0.8, a12 shrinks the wheel so the second ring with its outer
  // band stays on the sheet, multi1 hands its km = 0.82 in as scale
  const double km = opt.scale > 0.0 ? opt.scale : kDoubleWheelScale;
  build_base(dl, inner, s, inner_aspects, opt, km);
  auto add = [&](Primitive p) { dl.items.push_back(std::move(p)); };
  const auto at = [km](double w, double r) {
    return Pt{kCx + km * r * std::cos(-w), kCy + km * r * std::sin(-w)};
  };
  const double fza = wheel_origin(inner, opt);
  // the outer houses, horg11mult draws the cusp spokes of the second
  // chart in an outer band of their own from the sign rim r3 to the
  // outer boundary circle r4, the axes thick (DEFLINE 0,2) and the
  // intermediate cusps thin (DEFLINE 0,1), horg11mult_1 sets the labels
  // AC IC DC MC and 2 to 12 just outside r4. Robert used r4 = 235 on his
  // 640 by 480 field, the compare axis here is tighter, kAxisLabel = 208
  // carries the axes of the inner chart and kCompareGlyphRing = 204 the
  // planets of the outer one, so the outer band starts behind the glyphs
  // of the outer chart and ends on the boundary circle. The spokes cross
  // no symbol and read like the short outer strokes of common double
  // wheel drawings
  constexpr double kOuterCuspInner = kCompareGlyphRing + kSpriteSize / 2.0 + 1.0;
  constexpr double kOuterCuspOuter = 220.0;
  constexpr double kOuterCuspLabel = 230.0;
  //RR IF hrg! = 0 && haw& < 9, horg11mult draws nothing while the dial
  // view runs or the house system is silent, so the 90 degree wheel
  // stays free of the boundary circle and the axis labels
  if (!opt.heliocentric && !opt.dial && !without_angles(s.houses)) {
    // the outer boundary circle
    Primitive ring;
    ring.kind = Primitive::Kind::kCircle;
    ring.x1 = kCx;
    ring.y1 = kCy;
    ring.r1 = km * kOuterCuspOuter;
    ring.width = kThinLine;
    add(ring);
    static constexpr int kAxes[4] = {1, 4, 7, 10};
    static constexpr const char* kOuterAxis[4] = {"AC", "IC", "DC", "MC"};
    for (int a_idx = 0; a_idx < 4; ++a_idx) {
      const int a = kAxes[a_idx];
      const double w = wheel_angle(outer.houses.cusp[static_cast<std::size_t>(a)], fza);
      const Pt p1 = at(w, kOuterCuspInner);
      const Pt p2 = at(w, kOuterCuspOuter);
      add({Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF,
           Primitive::Style::kSolid, kAxisLine});
      const Pt pl = at(w, kOuterCuspLabel);
      Primitive t;
      t.kind = Primitive::Kind::kText;
      t.x1 = pl.x;
      t.y1 = pl.y;
      t.size = kLabelSize;
      t.text = kOuterAxis[a_idx];
      add(t);
    }
    // the intermediate cusps drop out when only AC and MC are asked for
    if (s.houses != HouseSystem::kAcMcOnly) {
      static constexpr int kMid[8] = {2, 3, 5, 6, 8, 9, 11, 12};
      for (int idx = 0; idx < 8; ++idx) {
        const int i = kMid[idx];
        const double w = wheel_angle(outer.houses.cusp[static_cast<std::size_t>(i)], fza);
        const Pt p1 = at(w, kOuterCuspInner);
        const Pt p2 = at(w, kOuterCuspOuter);
        Primitive hl{Primitive::Kind::kLine, p1.x, p1.y, p2.x, p2.y};
        hl.width = kThinLine;
        add(hl);
        const Pt pl = at(w, kOuterCuspLabel);
        Primitive t;
        t.kind = Primitive::Kind::kText;
        t.x1 = pl.x;
        t.y1 = pl.y;
        t.size = kNumberSize;
        t.text = std::to_string(i);
        add(t);
      }
    }
  }
  draw_outer_bodies(dl, outer, fza, km, kCompareMarkRing, kCompareGlyphRing, opt);
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
    // the credit line follows the sheet edge like the corner block, so
    // the right-aligned stamp stays visible when the sheet width changes
    if (p.anchor == Primitive::Anchor::kCredit && p.kind == Primitive::Kind::kText) {
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

void add_classic_text(DisplayList& dl, const Chart& chart, const ChartSettings& s, const ClassicSheetText& txt,
                      const std::vector<std::pair<int, int>>& mirrors) {
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
  const auto zod = [](double rad, char mark) {
    const double deg = norm_deg(rad * kRadToDeg);
    int sg = static_cast<int>(deg / kDegPerSign);
    const double in_sign = deg - sg * kDegPerSign;
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
    std::snprintf(buf, sizeof(buf), "%2d %s %2d%c", d, kSignTag[sg], m, mark);
    return std::string(buf);
  };

  //RR Länge:A1, the length mode tag of his table header
  const char* mode = s.apparent == ApparentMode::kLightTime         ? "A1"
                     : s.apparent == ApparentMode::kLightTimeAberration ? "A2"
                                                                        : "T";
  const double x0 = 6.0;
  text(x0, 16.0, (txt.len_header.empty() ? std::string("L\xC3\xA4nge:") : txt.len_header) + mode);
  if (!txt.longitudes_only) {
    text(118.0, 16.0, "Vel.");
  }
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
    if (txt.longitudes_only) {
      std::snprintf(buf, sizeof(buf), "%s %s", kRowTag[row], zod(b.el, mark).c_str());
    } else {
      std::snprintf(buf, sizeof(buf), "%s %s %7.1f", kRowTag[row],
                    zod(b.el, mark).c_str(), b.tb * kRadToDeg * 60.0);
    }
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
    const auto cusp_row = [&](const char* tag, double c) {
      const double deg = norm_deg(c * kRadToDeg);
      int sg = static_cast<int>(deg / kDegPerSign);
      const double in_sign = deg - sg * kDegPerSign;
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
      std::snprintf(buf, sizeof(buf), "%s:%2d\xC2\xB0 %s %2d'", tag, d, kSignTag[sg], m);
      text(x0, y, buf);
      y += 12.5;
    };
    for (int i = 0; i < 6; ++i) {
      //RR h$ = "H1 " under comp_mstz! or comp_hand!, then t$ = " AC" + " :" + s1$
      cusp_row((i == 0 && txt.h1_axis) ? " H1" : kHouseTag[i], chart.houses.cusp[static_cast<std::size_t>(kHouseIdx[i])]);
      if (i == 0 && txt.h1_axis) {
        cusp_row(" AC", chart.b[body::kAscendant].el);
      }
    }
  }
  y += 4.0;
  dl.items.push_back({Primitive::Kind::kLine, x0, y, 190.0, y, 0, 0, 0, 0, 0, 0x000000, 0xFFFFFF,
                      Primitive::Style::kSolid, 1.0, false, false, Primitive::Anchor::kCorner, ""});
  y += 14.0;
  //RR Spiegelung:
  text(x0, y, txt.mirror_label.empty() ? "Spiegelung:" : txt.mirror_label);
  // spieg1 writes pl$(t&) at xx& and "-" + pl$(u&) 14 further, 50 apart,
  // and starts a row 10 lower once xx& passes xt& + 94 beside the Länge:
  // header, two pairs to a row
  constexpr double kMirrorStep = 50.0;
  constexpr double kMirrorPartner = 14.0;
  constexpr double kMirrorRow = 10.0;
  constexpr double kMirrorWrap = 92.0;
  double mx = x0;
  y += kMirrorRow;
  for (const auto& [t, u] : mirrors) {
    if (mx > x0 + kMirrorWrap) {
      mx = x0;
      y += kMirrorRow;
    }
    text(mx, y, std::string(body::kName[static_cast<std::size_t>(t)]));
    text(mx + kMirrorPartner, y, "-" + std::string(body::kName[static_cast<std::size_t>(u)]));
    mx += kMirrorStep;
  }

  add_corner_text(dl, txt, 205.0, 383.0, kCanvasWidth - 8.0, true);
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

// ported from textg
double font_pitch(double size) {
  //RR CASE 7,8,9 TO 14 : tw& = 7, CASE 15,16 : tw& = 8, CASE 17 TO 20 : tw& = 10, CASE 19 TO 30 : tw& = 16
  const int te_gr = static_cast<int>(std::lround(size));
  if (te_gr >= 7 && te_gr <= 14) {
    return 7.0;
  }
  if (te_gr == 15 || te_gr == 16) {
    return 8.0;
  }
  if (te_gr >= 17 && te_gr <= 20) {
    return 10.0;
  }
  if (te_gr >= 21 && te_gr <= 30) {
    return 16.0;
  }
  return 0.0;
}

// ported from drad2 and dradst, the printed page carries its moment
void stamp_credit(DisplayList& dl, const std::string& stamp) {
  bool stamped = false;
  for (Primitive& p : dl.items) {
    if (p.anchor == Primitive::Anchor::kCredit && p.kind == Primitive::Kind::kText) {
      p.text += " " + stamp;
      stamped = true;
    }
  }
  if (stamped) {
    return;
  }
  //RR @textzentr(457,8,adr$ + " " + datumakt$ + " " + tim$)
  Primitive credit;
  credit.kind = Primitive::Kind::kText;
  credit.x1 = dl.width / 2.0;
  credit.y1 = dl.height - 3.0;
  credit.size = 8.0;
  credit.color = 0x808080;
  credit.anchor = Primitive::Anchor::kCredit;
  credit.text = std::string(kCreditLine) + " " + stamp;
  dl.items.push_back(credit);
}

void add_corner_text(DisplayList& dl, const ClassicSheetText& txt, double left_x, double center_x, double right_x,
                     bool classic_place) {
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
  if (!txt.pair_name1.empty()) {
    //RR @text(202,16,13,"1: " + LEFT$(na$(oo1,zz1),20)), a13aus puts the
    // first chart in place of the Name box
    text(left_x, 16.0, txt.pair_name1);
  } else {
    text(left_x, 16.0, txt.name_label.empty() ? "Name:" : txt.name_label);
    if (!txt.name.empty()) {
      text(left_x, 28.0, txt.name);
    }
  }
  if (!txt.mode.empty()) {
    text(center_x, 16.0, txt.mode, Align::kCenter);
  }
  if (!txt.stz.empty()) {
    text(right_x, 16.0, txt.stz, Align::kRight);
  }
  const bool pair_rows_shown = !txt.pair_moment1.empty() || !txt.pair_name2.empty() || !txt.pair_moment2.empty() ||
                               !txt.pair_list.empty() || !txt.pair_note.empty();
  if (classic_place && !pair_rows_shown) {
    //RR @text(x&,420,tg&,"Ort:"), go$ at 430 and @ort(10,x&,440), his bes1
    // corner at the lower left of the 640 sheet. There the lower right
    // lies inside the wheel, where the IC label ends
    constexpr double kClassicPlaceTop = 414.5;
    constexpr double kClassicPlaceStep = 10.0;
    double y = kClassicPlaceTop;
    for (const std::string* row : {&txt.place_label, &txt.place, &txt.lon, &txt.lat}) {
      const std::string& t = row == &txt.place_label && row->empty() ? std::string("Ort:") : *row;
      if (!t.empty() && (row != &txt.place_label || !txt.place.empty() || !txt.lon.empty() || !txt.lat.empty())) {
        text(left_x, y, t);
      }
      y += kClassicPlaceStep;
    }
  } else {
    // the Ort box stands at the lower right above the moment on the
    // screen sheet, so the lower left stays free for the composite,
    // combin and double wheel lines that used to cut into the wheel. The
    // lines sit flush right on the same edge as the STZ line top right,
    // a composite without a residence has no place and no box
    if (!txt.place.empty() || !txt.lon.empty() || !txt.lat.empty()) {
      text(right_x, 384.0, txt.place_label.empty() ? "Ort:" : txt.place_label, Align::kRight);
    }
    if (!txt.place.empty()) {
      text(right_x, 396.0, txt.place, Align::kRight);
    }
    if (!txt.lon.empty()) {
      text(right_x, 408.0, txt.lon, Align::kRight);
    }
    if (!txt.lat.empty()) {
      text(right_x, 420.0, txt.lat, Align::kRight);
    }
  }
  if (!txt.date.empty()) {
    text(right_x, 436.0, txt.date, Align::kRight);
  }
  if (!txt.ut.empty()) {
    text(right_x, 448.0, txt.ut, Align::kRight);
  }
  if (!txt.weekday.empty()) {
    text(right_x, 460.0, txt.weekday, Align::kRight);
  }
  //RR @text(202,454,13,"2: " + LEFT$(na$(oo2,zz2),20)), a13aus puts the
  // second chart as a short line under the wheel. The block stacks the
  // short pair lines from the bottom up, none reaches into the IC area of
  // the full wheel or the outer band of the smaller double wheel
  constexpr double kPairRowStep = 12.0;
  constexpr double kPairRowBase = 466.0;
  std::vector<const std::string*> pair_rows = {&txt.pair_moment1, &txt.pair_name2, &txt.pair_moment2};
  for (const std::string& row : txt.pair_list) {
    pair_rows.push_back(&row);
  }
  pair_rows.push_back(&txt.pair_note);
  int filled = 0;
  for (const std::string* row : pair_rows) {
    if (!row->empty()) {
      ++filled;
    }
  }
  double py = kPairRowBase - kPairRowStep * (filled > 0 ? filled - 1 : 0);
  for (const std::string* row : pair_rows) {
    if (row->empty()) {
      continue;
    }
    text(left_x, py, *row);
    py += kPairRowStep;
  }
}

// ported from a11 in moda 3, the DIN A4 page, tables first and the
// wheel over them, the order matters like in the original
DisplayList a4_print_sheet(const Chart& chart, const ChartSettings& s, const AspectResult& aspects,
                           const MidpointResult& midpoints, const ClassicSheetText& txt,
                           const WheelOptions& opt, const Histogram& hist, HistogramMode hist_mode) {
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
  const auto zodsec = [](double rad) {
    const double deg = norm_deg(rad * kRadToDeg);
    int sg = static_cast<int>(deg / kDegPerSign);
    const double in_sign = deg - sg * kDegPerSign;
    int total = static_cast<int>(in_sign * 3600.0 + 0.5);
    if (total >= static_cast<int>(kDegPerSign) * 3600) {
      total = 0;
      sg = (sg + 1) % 12;
    }
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%2d %s %2d'%2d\"", total / 3600, kSignTag[sg], (total / 60) % 60,
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

  // bes_big_kafige and bes_big_elem, the columns of kard_fix_gem1 and
  // elemhist1. A class column stands sixty high at the group maximum.
  // In his elem mode 2 the sign count takes the lower thirty and the
  // house count stacks above it, the number on top carries the sum
  if (hist_mode != HistogramMode::kNone) {
    const bool stacked = hist_mode == HistogramMode::kSignsAndHouses && !s.heliocentric;
    // his text stood on its bottom edge, the display list centres it
    const auto sitting = [&](double x, double bottom, std::string str, double size) {
      text(x, bottom - 0.5 * size, std::move(str), size);
    };
    //RR IF weiss! = 0, pboxn fills, otherwise the frame stays empty
    const auto fill_box = [&](double x1, double y1, double x2, double y2, Rgb color) {
      if (opt.hist_fill != RingFill::kWhite) {
        Primitive p;
        p.kind = Primitive::Kind::kRect;
        p.x1 = 0.5 * (x1 + x2);
        p.y1 = 0.5 * (y1 + y2);
        p.r1 = 0.5 * std::abs(x2 - x1);
        p.r2 = 0.5 * std::abs(y2 - y1);
        p.fill = color;
        p.color = color;
        add(p);
      }
      box(x1, y1, x2, y2);
    };
    // elem_col, the element colours in force, the first three also
    // paint the qualities
    const auto ink = [&](int e) {
      const Rgb c = opt.ring_colors[static_cast<std::size_t>(e)];
      return c == kCyan ? kPrintCyan : c;
    };
    struct Column {
      const char* tag;
      int signs;
      int houses;
      Rgb color;
    };
    // flat_zero keeps the empty element column as a line on the ground
    // like elemhist1, number_lift the extra unit kard_fix_gem1 leaves
    const auto columns = [&](double xt, double yy1, const std::vector<Column>& cols, const char* heading,
                             bool flat_zero, double number_lift) {
      constexpr double b = 14.0;
      int m1 = 0;
      int m2 = 0;
      for (const Column& c : cols) {
        m1 = std::max(m1, c.signs);
        m2 = std::max(m2, c.houses);
      }
      int top = 0;
      for (std::size_t k = 0; k < cols.size(); ++k) {
        const Column& c = cols[k];
        const double x = xt + static_cast<double>(k) * (b + 1.0);
        int e = 0;
        int f = 0;
        if (stacked) {
          e = m1 > 0 ? 30 * c.signs / m1 : 0;
          f = m2 > 0 ? 30 * c.houses / m2 : 0;
        } else {
          e = m1 > 0 ? 60 * c.signs / m1 : 0;
        }
        if (e > 0 || (flat_zero && hist_mode == HistogramMode::kSigns)) {
          fill_box(x, yy1, x + b, yy1 - e, c.color);
        }
        if (stacked && (e > 0 || f > 0)) {
          fill_box(x, yy1 - e - 2, x + b, yy1 - e - f - 2, c.color);
        }
        const int sum = c.signs + c.houses;
        const double number_bottom = yy1 - e - f - 2 - number_lift;
        char buf[16];
        if (sum > 99) {
          // his texts turned three digits on their side over the narrow
          // column, reading upward from the number line
          std::snprintf(buf, sizeof(buf), "%d", sum);
          Primitive p;
          p.kind = Primitive::Kind::kText;
          p.x1 = x + 0.5 * b + 0.5 * 8.5;
          p.y1 = number_bottom;
          p.size = 8.5;
          p.vertical = true;
          p.text = buf;
          add(p);
        } else {
          std::snprintf(buf, sizeof(buf), "%2d", sum);
          sitting(xt + static_cast<double>(k) * b + 2.0, number_bottom, buf, 8.5);
        }
        sitting(xt + 6.0 + static_cast<double>(k) * b, yy1 + 10.0, c.tag, 8.5);
        top = std::max(top, e + f);
      }
      sitting(xt - 3.0, yy1 - top - 10.0, heading, 9.5);
    };
    const Histogram none;
    const Histogram& h = stacked ? hist : none;
    // Kardinal, Fix and the third quality in the first three element
    // colours. The family reads the mutable signs as Veränderlich, so his
    // G for Gemischt and the Gem of the heading became V and Ver
    columns(16.0, 186.0,
            {{"K", hist.quality_sign[1], h.quality_house[1], ink(1)},
             {"F", hist.quality_sign[2], h.quality_house[2], ink(2)},
             {"V", hist.quality_sign[3], h.quality_house[3], ink(3)}},
            txt.quality_heading.empty() ? "Kard-Fix-Ver" : txt.quality_heading.c_str(), false, 1.0);
    columns(16.0, 576.0,
            {{"F", hist.element_sign[1], h.element_house[1], ink(1)},
             {"E", hist.element_sign[2], h.element_house[2], ink(2)},
             {"L", hist.element_sign[3], h.element_house[3], ink(3)},
             {"W", hist.element_sign[4], h.element_house[4], ink(4)}},
            txt.element_heading.empty() ? "Elemente" : txt.element_heading.c_str(), true, 0.0);
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

  //RR bes_big_asp, the histogram of asp1 and the Spiegelungen under it
  {
    box(212.0, 650.0, 386.0, 960.0);
    //RR xt& = 256, yt& = 670, IF nasp& > 1 && horm& = 1, @asp1
    double y = 670.0;
    if (opt.aspect_lines) {
      y = add_aspect_histogram(dl, aspects, opt.divisors, 256.0, y, opt.hist_colors, opt.hist_fill, 9.5);
    }
    //RR ADD yt&,24, LINE 212,yt&,386,yt&, ADD yt&,16, @spieg1
    y += 24.0;
    line(212.0, y, 386.0, y);
    y += 16.0;
    if (!aspects.mirrors.empty()) {
      text(220.0, y, txt.mirror_label.empty() ? std::string("Spiegelung:") : txt.mirror_label, 8.0);
      double my = y;
      double mx = 220.0;
      for (const auto& [t, w] : aspects.mirrors) {
        const std::string pair = tag(t) + "/" + tag(w);
        if (mx > 330.0) {
          mx = 220.0;
          my += 11.0;
        }
        if (my > 956.0) {
          break;
        }
        text(mx, my + 11.0, pair, 8.0);
        mx += 46.0;
      }
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
    credit.text = std::string(kCreditLine);
    add(std::move(credit));
  }
  return dl;
}

ChordLine standard_chord(int row) {
  // the colour and ls& pairs of aspz1, ml& 2 to 19
  struct Row {
    Rgb color;
    int style;
  };
  static constexpr Row kRows[kChordRows] = {
      {kChordBlack, 0}, {kChordBlack, 0}, {kChordRed, 0},   {kChordGreen, 2}, {kChordRed, 0},
      {kChordBlue, 0},  {kChordGreen, 2}, {kChordBlue, 3},  {kChordRed, 0},   {kChordRed, 2},
      {kChordBlue, 2},  {kChordBlack, 2}, {kChordGreen, 2}, {kChordBlue, 0},  {kChordBlue, 3},
      {kChordRed, 0},   {kChordRed, 2},   {kChordBlue, 2},  {kChordBlack, 2}, {kChordGreen, 2}};
  if (row < 2 || row >= kChordRows) {
    return {};
  }
  const Row& r = kRows[row];
  return {true, r.color, chord_line_style(r.style)};
}

Primitive::Style chord_line_style(int aspst) {
  switch (aspst) {
    case 1: return Primitive::Style::kDashed;
    case 2: return Primitive::Style::kDotted;
    case 3: return Primitive::Style::kDashDot;
    default: return Primitive::Style::kSolid;
  }
}

// ported from the SWITCH n& of aspz1
int chord_row(int n, int m, bool rhythm) {
  const bool edge = m == 1 || m == n - 1;
  switch (n) {
    case 2: return 2;
    case 3: return (m == 1 || m == 2) ? 3 : 0;
    case 4: return (m == 1 || m == 3) ? 4 : 0;
    case 5: return edge ? 5 : ((m == 2 || m == 3) ? 13 : 0);
    case 6: return (m == 1 || m == 5) ? 6 : 0;
    case 7: return edge ? 7 : ((m >= 2 && m <= 5) ? 14 : 0);
    case 8:
      if (rhythm) {
        return 0;
      }
      return edge ? 8 : ((m == 3 || m == 5) ? 15 : 0);
    case 9: return edge ? 9 : ((m == 2 || m == 4 || m == 5 || m == 7) ? 16 : 0);
    case 10: return edge ? 10 : ((m == 2 || m == 3 || m == 4 || m == 6 || m == 7 || m == 8) ? 17 : 0);
    case 11: return edge ? 11 : ((m >= 2 && m <= 9) ? 18 : 0);
    case 12:
      if (rhythm) {
        return 0;
      }
      //RR QUINKUNX
      return edge ? 12 : ((m == 5 || m == 7) ? 19 : 0);
    default: return 0;
  }
}

std::array<Rgb, 17> aspect_hist_colors(const ChordTable* own) {
  //RR asphist, blue conjunction, red for 2 4 8 9, green for 3 6 12, blue
  // for 5 7 10 11, white above twelve
  static constexpr Rgb kStandard[13] = {0x000000, 0x0000FF, 0xFF0000, 0x00FF00, 0xFF0000, 0x0000FF, 0x00FF00,
                                        0x0000FF, 0xFF0000, 0xFF0000, 0x0000FF, 0x0000FF, 0x00FF00};
  //RR vg% = RGB(192,192,192) for an own row without colour
  constexpr Rgb kNoOwnColor = 0xC0C0C0;
  std::array<Rgb, 17> c{};
  c.fill(0xFFFFFF);
  for (std::size_t i = 1; i <= 12; ++i) {
    c[i] = kStandard[i];
    if (own != nullptr && i >= 2) {
      c[i] = (*own)[i].on ? (*own)[i].color : kNoOwnColor;
    }
  }
  return c;
}

// ported from asphist
double add_aspect_histogram(DisplayList& dl, const AspectResult& aspects, int divisors, double xt, double yt,
                            const std::array<Rgb, 17>& colors, RingFill fill, double head_size, bool first_heading) {
  // his texts stand on their bottom line, the display list centres them
  const auto text = [&](double x, double bottom, std::string str, double size) {
    Primitive p;
    p.kind = Primitive::Kind::kText;
    p.x1 = x;
    p.y1 = bottom - 0.5 * size;
    p.size = size;
    p.align_left = true;
    p.text = std::move(str);
    dl.items.push_back(std::move(p));
  };
  // the bars are sw& = 6 high with 2 between, 70 long at the maximum
  constexpr double kBar = 6.0;
  constexpr double kGap = 2.0;
  constexpr double kLongest = 70.0;
  const double number_size = head_size * 10.0 / 13.0;
  const int nas = std::clamp(divisors, 1, 16);
  int top = 0;
  int sum = 0;
  for (int i = 1; i <= nas; ++i) {
    top = std::max(top, aspects.zh[static_cast<std::size_t>(i)]);
    sum += aspects.zh[static_cast<std::size_t>(i)];
  }
  if (first_heading) {
    text(xt - 15.0, yt + 2.0, "Aspekt 360/N :", head_size);
  }
  yt += 12.0;
  text(xt - 15.0, yt, "Teiler H\xC3\xA4ufigkt.", head_size);
  text(xt + 75.0, yt, " H", head_size);
  yt -= 2.0;
  for (int i = 1; i <= nas; ++i) {
    const int zh = aspects.zh[static_cast<std::size_t>(i)];
    //RR hh& = 70 * zh&(i&) / (kk + xhmax&)
    const double hh = top > 0 ? std::trunc(kLongest * zh / top) : 0.0;
    yt += kBar;
    Primitive bar;
    bar.kind = Primitive::Kind::kRect;
    bar.x1 = xt + 0.5 * hh;
    bar.y1 = yt + 0.5 * kBar;
    bar.r1 = 0.5 * hh;
    bar.r2 = 0.5 * kBar;
    bar.color = 0x000000;
    bar.width = 0.6;
    bar.fill = fill == RingFill::kWhite ? 0xFFFFFF : colors[static_cast<std::size_t>(i)];
    dl.items.push_back(bar);
    yt += kGap;
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%2d", i);
    text(xt - 19.0, yt + kBar, buf, number_size);
    //RR RSET aa$ = STR$(zh&(i&)), two places
    std::snprintf(buf, sizeof(buf), "%2d", zh);
    text(xt + 75.0, yt + kBar, buf, number_size);
  }
  text(xt - 13.0, yt + kBar + 10.0, "N", number_size);
  char total[24];
  std::snprintf(total, sizeof(total), "Summe H =%3d", sum);
  text(xt + 3.0, yt + kBar + 10.0, total, number_size);
  return yt;
}

// the dress of avh9 and aspz1_2. fill_color takes the own cols% under
// eigfarb! and the standard colours of col_zeich otherwise
WheelDress konsta_dress(const Konsta& k) {
  WheelDress d;
  for (int row = 2; row <= 19; ++row) {
    const auto r = static_cast<std::size_t>(row);
    if (k.selbst_cl_st) {
      if (k.aspli_col[r] != 0) {
        d.chords[r] = {true, rgb_of_colorref(k.aspli_col[r]), chord_line_style(k.aspst[r])};
      }
    } else if (k.aspli_flag[r] != 0) {
      d.chords[r] = standard_chord(row);
    }
  }
  d.hist_colors = aspect_hist_colors(k.selbst_cl_st ? &d.chords : nullptr);
  // col_zeich(255,0,0), (128,128,0), (0,128,128) and (0,255,255)
  static constexpr Rgb kStandard[5] = {0x000000, 0xFF0000, 0x808000, 0x008080, 0x00FFFF};
  d.own_colors = k.eigfarb;
  for (std::size_t e = 1; e <= 4; ++e) {
    // an own colour never set falls back to the standard one, his
    // fill_color would have painted it black
    d.ring_colors[e] = (k.eigfarb && k.cols[e] > 0) ? rgb_of_colorref(k.cols[e]) : kStandard[e];
  }
  const RingFill brush = k.farbp ? RingFill::kSolid : RingFill::kShaded;
  d.ring_fill = (k.weiss || k.nursymb != 0) ? RingFill::kWhite : brush;
  d.hist_fill = k.weiss ? RingFill::kWhite : brush;
  d.colored_signs = k.nursymb == 1;
  d.outer_color = std::clamp(k.hard, 1, 3);
  return d;
}

const WheelDress& profile_dress() {
  static const WheelDress d = konsta_dress(robert_profile());
  return d;
}

const char* body_glyph(int slot) {
  // pls&(13) and pls&(14), his AC and MC sprites, the wheel draws the
  // radix angles as axes and never asks
  if (slot == body::kAscendant) {
    return "AC";
  }
  if (slot == body::kMc) {
    return "MC";
  }
  return (slot >= 0 && slot < body::kSlotCount) ? kBodyGlyph[slot] : "";
}

const char* sign_glyph(int index) {
  return (index >= 0 && index < 12) ? kSignGlyph[index] : "";
}

// his asps& sprites, empty where he had none
const char* aspect_glyph(int family) {
  switch (family) {
    case 1: return "\xE2\x98\x8C";
    case 2: return "\xE2\x98\x8D";
    case 3: return "\xE2\x96\xB3";
    case 4: return "\xE2\x96\xA1";
    case 5: return "Q";
    case 6: return "\xE2\x9A\xB9";
    case 8: return "\xE2\x88\xA0";
    case 12: return "\xE2\x9A\xBA";
    case 17: return "bQ";
    case 18: return "\xE2\x9A\xBB";
    case 19: return "\xE2\x9A\xBC";
    default: return "";
  }
}

}  // namespace horcom
