// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/pair_sheet.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>

#include "horcom/chart/signs.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/items.hpp"

namespace horcom {

namespace {

// bes110 under dppel! and mult!, rows nine apart in size nine, the
// headers in size fourteen
constexpr double kRowStep = 9.0;
constexpr double kRowText = 9.0;
constexpr double kHeadText = 14.0;
// a12asp, rows 18 apart in columns 55 wide, the page ends at 457
constexpr double kGridRow = 18.0;
constexpr double kGridColumn = 55.0;
constexpr double kGridBottom = 457.0;
constexpr double kGridLineBottom = 456.0;
constexpr int kGridWraps = 3;
// halbsm boxes, bottom up 14 apart from 454 in size 11
constexpr double kBoxTop = 454.0;
constexpr double kBoxStep = 14.0;
constexpr double kBoxText = 11.0;

void text_at(DisplayList& dl, double x, double bottom, const std::string& s, double size, Rgb color = kInkColor) {
  dl.items.push_back(screen_text(x, bottom, size, s, color));
}

void line(DisplayList& dl, double x1, double y1, double x2, double y2) {
  Primitive p{Primitive::Kind::kLine, x1, y1, x2, y2};
  p.width = 0.6;
  dl.items.push_back(p);
}

void rect(DisplayList& dl, double x1, double y1, double x2, double y2, Rgb fill) {
  Primitive p;
  p.kind = Primitive::Kind::kRect;
  p.x1 = 0.5 * (x1 + x2);
  p.y1 = 0.5 * (y1 + y2);
  p.r1 = 0.5 * std::abs(x2 - x1);
  p.r2 = 0.5 * std::abs(y2 - y1);
  p.fill = fill;
  p.color = kInkColor;
  p.width = 0.6;
  dl.items.push_back(p);
}

// plein2, the sprite of a body, the red F of the fixed point, the tag of
// an axis that has no sprite in the port
void glyph(DisplayList& dl, int slot, double x, double y, Rgb color) {
  if (slot == 0) {
    color = kMarkRed;
  }
  std::string g = slot == 0 ? "F" : body_glyph(slot);
  Primitive p;
  p.kind = Primitive::Kind::kGlyph;
  p.x1 = x;
  p.y1 = y;
  p.size = kSpriteSize;
  p.color = color;
  if (g.empty()) {
    p.kind = Primitive::Kind::kText;
    p.size = kRowText;
    g = std::string(body::kName[static_cast<std::size_t>(slot)]);
  }
  p.text = g;
  dl.items.push_back(std::move(p));
}

bool live(const Chart& c, int slot) {
  const BodyState& b = c.b[static_cast<std::size_t>(slot)];
  return b.present && b.valid;
}

}  // namespace

void add_sheet_text(DisplayList& dl, double x, double bottom, const std::string& s, double size) {
  text_at(dl, x, bottom, s, size);
}

std::string pair_row_text(int slot, double lon, bool compact) {
  // grze_0, the minute rounded with its carry into the degree and sign
  const ZodiacSplit z = split_zodiac(lon, false);
  const std::string tag = slot == 0 ? "FP" : std::string(body::kName[static_cast<std::size_t>(slot)]);
  char buf[40];
  if (compact) {
    // gz$ = pl$(g&) + " " + a$ + h$ + zei$(c2) + " " + d$ + "'"
    std::snprintf(buf, sizeof(buf), "%s %2d\xC2\xB0%s %2d'", tag.c_str(), z.deg, kSignTag[z.sign], z.min);
  } else {
    // gz7$ = pl$(g&) + " " + a$ + " " + zei$(c2) + " " + d$
    std::snprintf(buf, sizeof(buf), "%s %2d %s %2d", tag.c_str(), z.deg, kSignTag[z.sign], z.min);
  }
  return buf;
}

// ported from bes11 under dppel! and mult!
double add_pair_bodies(DisplayList& dl, const Chart& c, const PairColumnOptions& opt, double xt, double yt) {
  text_at(dl, xt + 2.0, yt, opt.header, kHeadText);
  std::vector<int> rows;
  // aa& = 0 with the fixed point, the rows to bb2&, twelve or ten, and
  // the extras under klpl!, the south node never
  if (live(c, 0) && !opt.heliocentric) {
    rows.push_back(0);
  }
  const int last = opt.heliocentric ? body::kPluto : body::kNodeAsc;
  for (int slot = body::kSun; slot <= last; ++slot) {
    rows.push_back(slot);
  }
  if (opt.extras) {
    for (int slot = body::kApogee; slot < body::kSlotCount; ++slot) {
      if (live(c, slot) && !(opt.heliocentric && (slot == body::kApogee || slot == body::kFortune))) {
        rows.push_back(slot);
      }
    }
  }
  for (const int slot : rows) {
    if (opt.present_only && !live(c, slot)) {
      continue;
    }
    yt += kRowStep;
    if (!live(c, slot)) {
      continue;
    }
    // w$, P for SO to MA under the parallax, M or W for node and apogee
    std::string mark = " ";
    if (slot >= body::kSun && slot <= body::kMars && opt.parallax) {
      mark = "P";
    }
    if (slot == body::kNodeAsc) {
      mark = opt.true_node ? "W" : "M";
    }
    if (slot == body::kApogee) {
      mark = opt.true_apogee ? "W" : "M";
    }
    if (opt.heliocentric) {
      mark.clear();
    }
    // IF i& = 0, RGBCOLOR RGB(255,0,0)
    text_at(dl, xt + 2.0, yt + 2.0, pair_row_text(slot, c.b[static_cast<std::size_t>(slot)].el, opt.compact) + mark,
            kRowText, slot == 0 ? kMarkRed : kInkColor);
  }
  return yt + 4.0;
}

// ported from bes111 under dppel! and mult!
double add_pair_houses(DisplayList& dl, const Chart& c, const PairColumnOptions& opt, double xt, double yt) {
  if (opt.heliocentric || !c.houses.ok) {
    return yt;
  }
  yt += 12.0;
  if (!opt.dial) {
    text_at(dl, xt + 2.0, yt + 2.0, opt.houses_header, kHeadText);
    yt += 12.0;
    text_at(dl, xt, yt + 2.0, "(" + opt.house_name + ")", kHeadText);
  }
  yt += 2.0;
  for (const int k : {1, 2, 3, 10, 11, 12}) {
    // dop = 4 and the MULTI NULL modes keep only AC and MC
    if ((opt.dial || opt.angles_only) && k != 1 && k != 10) {
      continue;
    }
    // IF f(k&) > 0, a cleared cusp of the directed chart stays away
    if (c.houses.cusp[static_cast<std::size_t>(k)] <= 0.0) {
      continue;
    }
    const ZodiacSplit z = split_zodiac(c.houses.cusp[static_cast<std::size_t>(k)], false);
    char cusp[24];
    // gz1$ = a$ + h$ + " " + zei$(c2) + " " + d$ + "'"
    std::snprintf(cusp, sizeof(cusp), "%2d\xC2\xB0 %s %2d'", z.deg, kSignTag[z.sign], z.min);
    std::string label;
    if (k == 1) {
      label = " AC:";
    } else if (k == 10) {
      label = " MC:";
    } else {
      char h[8];
      std::snprintf(h, sizeof(h), "H%s%d :", k < 10 ? " " : "", k);
      label = h;
    }
    yt += kRowStep;
    text_at(dl, xt + 3.0, yt, label + cusp, kRowText);
  }
  return yt + 2.0;
}

// ported from the drawing part of a12asp
void add_cross_grid(DisplayList& dl, const std::vector<CrossAspectHit>& hits, const CrossGridOptions& opt, double xt,
                    double yt) {
  const double y1 = yt + 2.0;
  double y = y1;
  double x = xt;
  int z = 0;
  int zb = 0;
  int nmu = 0;
  bool opened = false;
  std::set<std::pair<int, int>> drawn;
  const Rgb outer = opt.outer_color == 1 ? kMarkRed : (opt.outer_color == 3 ? 0x0000FF : kInkColor);
  for (const CrossAspectHit& h : hits) {
    if (!opened) {
      // the column letters over the grid and the rule above them
      text_at(dl, 3.0, y1 - 1.0, opt.left_tag, 11.0);
      text_at(dl, 35.0, y1 - 1.0, opt.right_tag, 11.0);
      text_at(dl, 58.0, y1 - 1.0, opt.left_tag, 11.0);
      text_at(dl, 90.0, y1 - 1.0, opt.right_tag, 11.0);
      if (!opt.left_tag2.empty()) {
        text_at(dl, 112.0, y1 - 1.0, opt.left_tag2, 11.0);
        text_at(dl, 144.0, y1 - 1.0, opt.right_tag2, 11.0);
        text_at(dl, 167.0, y1 - 1.0, opt.left_tag2, 11.0);
        text_at(dl, 199.0, y1 - 1.0, opt.right_tag2, 11.0);
      }
      line(dl, 3.0, y1 - 12.0, 220.0, y1 - 12.0);
      opened = true;
    }
    // IF nmu& = 5 && mult! && xt& < 220, the second column of four
    ++nmu;
    if (nmu == 5 && opt.multi && x < 220.0) {
      x += kGridColumn;
      y = y1;
    }
    // mk&(t&,w&), every pair once
    if (!drawn.insert({h.t, h.w}).second) {
      continue;
    }
    ++z;
    const int z1 = z;
    y += kGridRow;
    if (y > kGridBottom && zb < kGridWraps) {
      ++zb;
      x += kGridColumn;
      y -= z1 * kGridRow - kGridRow;
      z -= z1 - 1;
    }
    if (zb <= kGridWraps && y < kGridBottom) {
      const int first = opt.running_first ? h.w : h.t;
      const int second = opt.running_first ? h.t : h.w;
      glyph(dl, first, x + 9.0, y - 4.0, opt.running_first ? outer : kInkColor);
      Primitive a;
      a.kind = Primitive::Kind::kGlyph;
      a.x1 = x + 26.0;
      a.y1 = y - 6.0;
      a.size = kSpriteSize;
      a.text = aspect_glyph(h.n);
      dl.items.push_back(a);
      // asp& = CINT(up * asp), placed by its width under the sprite
      const long deg = std::lround(h.sep_deg);
      char buf[8];
      std::snprintf(buf, sizeof(buf), "%ld", deg);
      const double dx = deg < 10 ? 21.0 : (deg < 100 ? 19.0 : 17.0);
      text_at(dl, x + dx, y + 9.0, buf, 9.0);
      glyph(dl, second, x + 43.0, y - 4.0, opt.running_first ? kInkColor : outer);
      line(dl, 3.0, y1, 220.0, y1);
      line(dl, 110.0, 3.0, 110.0, y1);
      line(dl, 220.0, 3.0, 220.0, y1);
      for (int ti = 1; ti <= 4; ++ti) {
        line(dl, kGridColumn * ti, y1 - 12.0, kGridColumn * ti, opt.multi ? y + 2.0 : kGridLineBottom);
      }
    }
    if (y > kGridBottom && zb == kGridWraps) {
      text_at(dl, x, kGridBottom, opt.more_label, 9.0);
      break;
    }
  }
  line(dl, 110.0, 3.0, 110.0, y1);
  if (!opt.multi) {
    line(dl, 220.0, 3.0, 220.0, kGridLineBottom);
  }
}

// ported from halbsm with the boxes of textrl
void add_multi_midpoints(DisplayList& dl, const std::vector<MultiMidpoint>& lines, bool white) {
  double e = kBoxTop;
  double f = kBoxTop;
  // deftextcol 3 red on cyan, 2 navy on yellow, 1 black on white
  const auto box = [&](double x, double yte, const std::string& s, Rgb ink, Rgb ground) {
    //RR l& = LEN(tex$) * te_w&, the advance the text is set with
    const double l = std::max(2.0, static_cast<double>(s.size())) * text_advance(screen_text(x, yte, kBoxText, s));
    rect(dl, x - 2.0, yte - kBoxText - 1.0, x + l + 2.0, yte + 2.0, ground);
    // the shadow lines along the top and the right edge
    line(dl, x - 1.0, yte - kBoxText - 2.0, x + l + 3.0, yte - kBoxText - 2.0);
    line(dl, x + l + 3.0, yte - kBoxText - 2.0, x + l + 3.0, yte + 1.0);
    text_at(dl, x, yte + 1.0, s, kBoxText, ink);
  };
  const auto tag = [](int slot) { return std::string(body::kName[static_cast<std::size_t>(slot)]); };
  for (const MultiMidpoint& m : lines) {
    const std::string pair = tag(m.u) + "-" + tag(m.w) + " M = ";
    char axis[24];
    switch (m.target) {
      case MultiMidpoint::Target::kSignAxis:
        std::snprintf(axis, sizeof(axis), "%s/%s", kSignTag[m.first - 1], kSignTag[m.second - 1]);
        box(6.0, e, pair + axis, white ? kInkColor : kMarkRed, white ? 0xFFFFFF : 0x00FFFF);
        e -= kBoxStep;
        break;
      case MultiMidpoint::Target::kRadixCusp:
        std::snprintf(axis, sizeof(axis), "HS %d/%d R", m.first, m.second);
        box(6.0, e, pair + axis, white ? kInkColor : 0x000080, white ? 0xFFFFFF : 0xFFFF00);
        e -= kBoxStep;
        break;
      case MultiMidpoint::Target::kMultiAngle:
        std::snprintf(axis, sizeof(axis), "HS %d/%d M", m.first, m.second);
        box(170.0, f, " " + pair + axis, white ? kInkColor : 0x000080, white ? 0xFFFFFF : 0xFFFF00);
        f -= kBoxStep;
        break;
    }
  }
}

}  // namespace horcom
