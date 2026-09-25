// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/aspektarium.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the cell grid of aspar, 26 wide and 20 high, column c centred at
// 16 + 26 c, row r at 10 + 20 r
constexpr double kCellW = 26.0;
constexpr double kCellH = 20.0;
// his CLIP 0,0,525,401 around the matrix
constexpr double kClipX = 525.0;
constexpr double kClipY = 401.0;
// the whole block (2,2)-(636,442) moves down to y 20 under the title
constexpr double kBlockShift = 18.0;
// the hatch of DEFFILL 7 on the diagonal as it rendered, a light grey
constexpr Rgb kDiagonalHatch = 0xE0E0E0;
// w > 0.36, only wider angles get a name
constexpr double kNamedFrom = 0.36;

double column_x(int c) { return 16.0 + kCellW * c; }
double row_y(int r) { return 10.0 + kCellH * r; }

}  // namespace

// ported from aspar with aspsenk, aspsenk1, aspwag, aspwag1, aspdis, nam
// and klplanz
DisplayList build_aspektarium(const AspektariumInput& in, const AspektariumText& text) {
  DisplayList dl;
  dl.width = kCanvasWidth;
  dl.height = 460.0;
  const Chart& chart = *in.chart;
  const AspectResult& asp = *in.aspects;
  const bool helio = in.settings.heliocentric;
  auto add = [&](Primitive p) { dl.items.push_back(std::move(p)); };
  // his texts stand on their bottom line, block content moves with it
  const auto text_at = [&](double x, double bottom, const std::string& s, double size, Rgb color = kInkColor,
                           bool block = true) {
    Primitive p;
    p.kind = Primitive::Kind::kText;
    p.x1 = x;
    p.y1 = bottom - 0.5 * size + (block ? kBlockShift : 0.0);
    p.size = size;
    // te_w& = @textg(te_gr&), FONT WIDTH te_w&
    p.pitch = font_pitch(size);
    p.color = color;
    p.align_left = true;
    p.text = s;
    add(p);
  };
  const auto line = [&](double x1, double y1, double x2, double y2, bool block = true) {
    const double dy = block ? kBlockShift : 0.0;
    Primitive p{Primitive::Kind::kLine, x1, y1 + dy, x2, y2 + dy};
    p.width = 0.6;
    add(p);
  };
  const auto box = [&](double x1, double y1, double x2, double y2, bool block = true) {
    line(x1, y1, x2, y1, block);
    line(x2, y1, x2, y2, block);
    line(x2, y2, x1, y2, block);
    line(x1, y2, x1, y1, block);
  };
  const auto sprite = [&](const std::string& glyph, double x, double y, Rgb color, bool block = true) {
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = x;
    p.y1 = y + (block ? kBlockShift : 0.0);
    p.size = kSpriteSize;
    p.color = color;
    p.text = glyph;
    add(p);
  };
  const auto red = [&](int slot) { return in.emphasis[static_cast<std::size_t>(slot)] > 0; };
  // plinkl of plein2, the nodes under moknw! and the Black Moon under
  // apogw! stand inverted
  const auto inverted = [&](int slot) {
    return ((slot == body::kNodeAsc || slot == body::kNodeDesc) && in.invert_nodes) ||
           (slot == body::kApogee && in.invert_apogee);
  };
  // plein2, the fixed point is always the red F, the axes wear his AC and
  // MC sprites, a body without one its tag
  const auto body_sprite = [&](int slot, double x, double y, bool block = true) {
    if (slot == 0) {
      sprite("F", x, y, kMarkRed, block);
      return;
    }
    const std::string g = body_glyph(slot);
    if (g.empty()) {
      text_at(x - 6.0, y + 4.0, std::string(body::kName[static_cast<std::size_t>(slot)]), 9.0,
              red(slot) ? kMarkRed : kInkColor, block);
      return;
    }
    Rgb ink = red(slot) ? kMarkRed : kInkColor;
    if (inverted(slot)) {
      add(inverted_patch(x, y + (block ? kBlockShift : 0.0), kSpriteSize));
      ink = red(slot) ? kMarkRed : kInvertedInk;
    }
    sprite(g, x, y, ink, block);
  };
  // his plz(od,ze,o&) > kk took a zero for an empty slot, so a body at
  // exactly 0 Aries lost its column. The slot tells presence in the port
  const auto present = [&](int slot) {
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    return b.present && b.valid;
  };

  // aa& = 0 with the fixed point, 2 in the hrg mode, 1 otherwise
  const bool fp = present(0) && !helio;
  const int shift = fp ? 1 : 0;
  const int aa = fp ? 0 : (helio ? 2 : 1);
  // e& = 14 or 10 without extras, the extras follow MC up to column 19
  std::vector<std::pair<int, int>> cols;  // slot and column
  const int last_base = helio ? body::kPluto : body::kMc;
  for (int slot = aa; slot <= last_base; ++slot) {
    cols.emplace_back(slot, slot + shift);
  }
  if (in.settings.extra_bodies) {
    int c = last_base + 1 + shift;
    for (int slot = body::kApogee; slot < body::kSlotCount && c <= kAspektariumColumns; ++slot) {
      if (present(slot)) {
        cols.emplace_back(slot, c++);
      }
    }
  }
  const int e = cols.empty() ? 0 : cols.back().second;

  // aspsenk and aspwag, the header sprites and the diagonal boxes
  for (const auto& [slot, c] : cols) {
    if (!present(slot)) {
      continue;
    }
    const double x = column_x(c);
    const double y = row_y(c);
    body_sprite(slot, x, 9.0);
    box(x - 13.0, y - 10.0, x + 13.0, y + 10.0);
    box(x - 12.0, y - 9.0, x + 12.0, y + 9.0);
    body_sprite(slot, 13.0, y);
  }
  line(523.0, 2.0, 523.0, 399.0);

  // the pairs, separation above the diagonal, the name below it
  for (std::size_t i = 0; i < cols.size(); ++i) {
    const auto [o, o1] = cols[i];
    if (!present(o)) {
      continue;
    }
    for (std::size_t j = i + 1; j < cols.size(); ++j) {
      const auto [m, m1] = cols[j];
      if (!present(m)) {
        continue;
      }
      double w = asp.asp[static_cast<std::size_t>(o)][static_cast<std::size_t>(m)];
      if (w > kPi && w < kTwoPi) {
        w = kTwoPi - w;
      }
      const double x0 = m1 * kCellW;
      const double y0 = o1 * kCellH;
      const double x1 = o1 * kCellW;
      const double y1 = m1 * kCellH;
      double di = std::abs(chart.b[static_cast<std::size_t>(o)].el - chart.b[static_cast<std::size_t>(m)].el);
      if (di > kPi && di < kTwoPi) {
        di = kTwoPi - di;
      }
      const Rgb ink = (red(o) || red(m)) ? kMarkRed : kInkColor;
      //RR nur diskrete Aspekte
      if (w > kEps) {
        // his STR$(FIX(di)) and STR$(60 * FRAC(di),2,0) printed 60' under
        // the degree it belonged to, 10°59.8' read 10° 60'. The port rounds
        // the whole minute and carries it, 11° 0'
        const long total = std::lround(di * kRadToDeg * kArcminPerDeg);
        const long per_deg = static_cast<long>(kArcminPerDeg);
        char deg[24];
        char min[24];
        std::snprintf(deg, sizeof(deg), "%3ld\xC2\xB0", total / per_deg);
        std::snprintf(min, sizeof(min), "%2ld'", total % per_deg);
        text_at(3.0 + x0, 10.0 + y0, deg, 9.0, ink);
        text_at(10.0 + x0, 20.0 + y0, min, 9.0, ink);
      }
      const bool no_name = helio && (m == body::kApogee || m == body::kFortune);
      if (w > kNamedFrom && !no_name) {
        const int nm = aspect_symbol(w, in.orbs.divisors);
        const bool main = nm >= 1 && nm <= 6;
        const bool minor = nm == 8 || nm == 12 || nm == 17 || nm == 18 || nm == 19;
        if (main) {
          // DRAW ... TO ... TO, the hook of a main aspect
          line(3.0 + x0, 15.0 + y0, 8.0 + x0, 15.0 + y0);
          line(8.0 + x0, 15.0 + y0, 8.0 + x0, 20.0 + y0);
        } else {
          line(3.0 + x0, 15.0 + y0, 8.0 + x0, 20.0 + y0);
        }
        if (main || minor) {
          // putbm(10 + x1&,4 + y1&), his 14 pixel asps& sprite
          sprite(aspect_glyph(nm), 17.0 + x1, 11.0 + y1, ink);
        } else {
          char a[16];
          char b[24];
          std::snprintf(a, sizeof(a), "%2d", nm);
          std::snprintf(b, sizeof(b), "%3ld", std::lround(w * kRadToDeg));
          text_at(14.0 + x1, 10.0 + y1, a, 9.0, ink);
          text_at(6.0 + x1, 20.0 + y1, b, 9.0, ink);
        }
      }
    }
  }

  // aspsenk1 and aspwag1 for w& = aa& TO e&, the right and lower borders
  // of the columns aa& to e& + 1 inside his CLIP
  for (int c = aa; c <= e - shift + 1; ++c) {
    const double x = 29.0 + kCellW * c;
    const double y = kCellH * (c + 1);
    if (x <= kClipX) {
      line(x, 0.0, x, 400.0);
    }
    if (y <= kClipY) {
      line(2.0, y, 524.0, y);
    }
  }
  // the diagonal, DEFFILL 7 inside the inner box and az&(t&) on it
  for (const auto& [slot, c] : cols) {
    const double x = column_x(c);
    const double y = row_y(c);
    if (x + 13.0 > kClipX || y + 10.0 > kClipY) {
      continue;
    }
    Primitive hatch;
    hatch.kind = Primitive::Kind::kRect;
    hatch.x1 = x;
    hatch.y1 = y + kBlockShift;
    hatch.r1 = 12.0;
    hatch.r2 = 9.0;
    hatch.fill = kDiagonalHatch;
    hatch.color = kDiagonalHatch;
    add(hatch);
    char count[8];
    std::snprintf(count, sizeof(count), "%2d", asp.az[static_cast<std::size_t>(slot)]);
    text_at(x - 6.0, y + 6.0, count, 12.0);
  }
  line(2.0, 399.0, 636.0, 399.0);

  // the divisor table, Teiler, Winkel and Orbis
  text_at(526.0, 11.0, text.heads[0], 12.0);
  text_at(526.0, 19.0, text.heads[1], 12.0);
  text_at(560.0, 11.0, text.heads[2], 12.0);
  text_at(560.0, 19.0, text.heads[3], 12.0);
  text_at(604.0, 11.0, text.heads[4], 12.0);
  text_at(604.0, 19.0, text.heads[5], 12.0);
  char buf[48];
  std::snprintf(buf, sizeof(buf), "%3d%%", static_cast<int>(std::lround(kPercent * in.orbs.orb)));
  text_at(526.0, 200.0, text.factor1, 13.0);
  text_at(526.0, 210.0, text.factor2 + buf, 13.0);
  text_at(526.0, 228.0, text.legend[0], 14.0);
  text_at(526.0, 240.0, text.legend[1], 9.0);
  text_at(526.0, 250.0, text.legend[2], 9.0);
  text_at(526.0, 260.0, text.legend[3], 9.0);
  text_at(526.0, 272.0, text.legend[4], 9.0);
  text_at(526.0, 282.0, text.legend[5], 9.0);
  text_at(526.0, 300.0, text.name_label, 14.0);
  // nam(12,12,526,310), split at the first space
  {
    const std::size_t sp = text.name.find(' ');
    if (sp == std::string::npos) {
      text_at(526.0, 310.0, text.name.substr(0, 12), 14.0);
    } else {
      std::string rest = text.name.substr(sp + 1, 12);
      while (!rest.empty() && rest.back() == ' ') {
        rest.pop_back();
      }
      text_at(526.0, 310.0, text.name.substr(0, sp), 14.0);
      text_at(526.0, 322.0, rest, 14.0);
    }
  }
  text_at(526.0, 340.0, text.date_label, 14.0);
  text_at(526.0, 352.0, text.date, 14.0);
  const int nasp = std::clamp(in.orbs.divisors, 1, 16);
  for (int t = 1; t <= nasp; ++t) {
    const double dd = divisor_orb(in.orbs, t);
    char n[8];
    char f[16];
    char k[16];
    std::snprintf(n, sizeof(n), "%2d", t);
    std::snprintf(f, sizeof(f), "%5.1f\xC2\xB0", kDegPerCircle / t);
    std::snprintf(k, sizeof(k), "%4.1f", dd * kRadToDeg);
    text_at(528.0, t * 10.0 + 24.0, n, 12.0);
    text_at(550.0, t * 10.0 + 24.0, f, 12.0);
    text_at(594.0, t * 10.0 + 24.0, k, 12.0);
  }
  line(28.0, 2.0, 28.0, 399.0);
  line(29.0, 2.0, 29.0, 399.0);
  line(2.0, 19.0, 636.0, 19.0);
  line(2.0, 20.0, 636.0, 20.0);
  // klplanz(518,370), up to five extras with their tags
  if (in.settings.extra_bodies) {
    int z = 0;
    for (const auto& [slot, c] : cols) {
      if (slot < body::kApogee || !present(slot) || z >= 5) {
        continue;
      }
      ++z;
      const double x = 518.0 + 15.0 * z;
      body_sprite(slot, x, 370.0);
      text_at(x - 5.0, 385.0, std::string(body::kName[static_cast<std::size_t>(slot)]), 9.0);
    }
  }

  // textzent(16,16,...) over the pasted block
  {
    Primitive t;
    t.kind = Primitive::Kind::kText;
    t.x1 = kCanvasWidth / 2.0;
    t.y1 = 8.0;
    t.size = 16.0;
    t.text = text.title;
    add(t);
  }
  // the planet weights under the block, FOR t& = aa& TO 14, the hrg mode
  // leaves out only AC and MC, the nodes keep their weights
  text_at(4.0, 435.0, text.weights1, 13.0, kInkColor, false);
  text_at(4.0, 445.0, text.weights2, 13.0, kInkColor, false);
  for (int t = aa; t <= body::kMc; ++t) {
    if (helio && (t == body::kAscendant || t == body::kMc)) {
      continue;
    }
    body_sprite(t, 96.0 + t * 28.0, 427.0, false);
    char p[16];
    std::snprintf(p, sizeof(p), t == body::kMc ? "%3d%%" : "%3d", in.weights[static_cast<std::size_t>(t)]);
    text_at(84.0 + t * 28.0, 447.0, p, 13.0, red(t) ? kMarkRed : kInkColor, false);
  }
  if (in.settings.extra_bodies) {
    // his @textc(510,435,16,...) ran 160 units wide past the 640 edge of
    // his own screen, the 13 of the Gewichtung block on the left ends it
    // at 622
    text_at(510.0, 435.0, text.extras, 13.0, kInkColor, false);
    char p[16];
    std::snprintf(p, sizeof(p), "%3d%%", in.weights[body::kApogee]);
    text_at(542.0, 445.0, p, 13.0, kInkColor, false);
  }
  // IF ryt! = 0 && e& < 15 && fixpunkt& = 2, the histogram inset
  if (!in.rhythm && e < 15 && !fp) {
    Primitive white;
    white.kind = Primitive::Kind::kRect;
    white.x1 = 459.0;
    white.y1 = 135.0;
    white.r1 = 57.0;
    white.r2 = 85.0;
    white.fill = kPaperColor;
    white.color = kPaperColor;
    add(white);
    // his ahi bitmap of asphist grabbed at xt 19 from the second heading
    // down lands at 404, 50, the first heading stayed above the grab
    add_aspect_histogram(dl, asp, nasp, 404.0 + 19.0, 50.0, in.hist_colors, in.hist_fill, 10.0, false);
    box(402.0, 50.0, 516.0, 220.0, false);
  }
  return dl;
}

}  // namespace horcom
