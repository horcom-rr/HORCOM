// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/rhythm_panel.hpp"

#include <algorithm>
#include <numeric>

#include "horcom/render/items.hpp"

namespace horcom {

namespace {

// the white of his RGB(255,255,255) sprite ground, an inverted sprite
// wears it as ink, not the paper tone of the sheet
constexpr Rgb kWhite = 0xFFFFFF;

// x1 = 101, the time axis between the two columns
constexpr double kAxisX = 101.0;
// the strip ends at 217, his separator line stands at 218
constexpr double kStripRight = 218.0;
// his label rows keep sixteen pixels apart
constexpr double kRowStep = 16.0;

void line(DisplayList& dl, double x1, double y1, double x2, double y2) {
  dl.items.push_back(line_item(x1, y1, x2, y2));
}

// his text and textc, x the left edge and y the bottom line
void text(DisplayList& dl, double x, double bottom, double size, const std::string& t, Rgb c = kInkColor) {
  if (!t.empty()) {
    dl.items.push_back(screen_text(x, bottom, size, t, c));
  }
}

// plein2 and plinv, the inverted sprite sits on a dark patch
void glyph(DisplayList& dl, double x, double y, int slot, bool inverse) {
  if (inverse) {
    dl.items.push_back(inverted_patch(x, y, kSpriteSize));
  }
  Primitive p;
  p.kind = Primitive::Kind::kGlyph;
  p.x1 = x;
  p.y1 = y;
  p.size = kSpriteSize;
  p.text = rhythm_glyph(slot);
  p.color = inverse ? kWhite : kInkColor;
  dl.items.push_back(p);
}

}  // namespace

double rhythm_axis_y(double age, double start, double length) {
  if (length == 0.0) {
    return kRhythmAxisBottom;
  }
  return kRhythmAxisBottom - (kRhythmAxisBottom - kRhythmAxisTop) * (age - start) / length;
}

// ported from a17911, the sort of a1791 and a1792 and his push upward
std::vector<double> rhythm_label_rows(const std::vector<double>& ys) {
  std::vector<std::size_t> order(ys.size());
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::stable_sort(order.begin(), order.end(), [&ys](std::size_t a, std::size_t b) { return ys[a] > ys[b]; });
  std::vector<double> out(ys.size());
  bool first = true;
  double y1 = 0.0;
  for (const std::size_t k : order) {
    double y = ys[k];
    // IF dy& < 16, y& = y& - 16 + dy&
    if (!first && y1 - y < kRowStep) {
      y = y1 - kRowStep;
    }
    out[k] = y;
    y1 = y;
    first = false;
  }
  return out;
}

std::string rhythm_glyph(int slot) {
  if (slot == body::kAscendant) {
    return "AC";
  }
  if (slot == body::kMc) {
    return "MC";
  }
  if (body::cardinal(slot)) {
    return sign_glyph((slot - body::kAriesPoint) * 3);
  }
  return body_glyph(slot);
}

void add_rhythm_panel(DisplayList& dl, const RhythmPanel& p) {
  // @line(218,2,218,458), the strip against the wheel
  line(dl, kStripRight, 2.0, kStripRight, 458.0);
  // the time axis with its end ticks
  line(dl, kAxisX, kRhythmAxisTop, kAxisX, kRhythmAxisBottom);
  line(dl, kAxisX - 2.0, kRhythmAxisTop, kAxisX + 2.0, kRhythmAxisTop);
  line(dl, kAxisX - 2.0, kRhythmAxisBottom, kAxisX + 2.0, kRhythmAxisBottom);
  // a170_1tit
  text(dl, 2.0, 14.0, 13.0, p.title);
  text(dl, 2.0, 28.0, 13.0, p.period);
  const double d = p.span.empty() ? 0.0 : 16.0;
  text(dl, 2.0, 42.0 + d, 13.0, p.negative1, kMarkRed);
  text(dl, 2.0, 54.0 + d, 13.0, p.negative2, kMarkRed);
  text(dl, 2.0, 38.0, 13.0, p.span);
  text(dl, 60.0, 48.0, 13.0, p.span_unit);

  // a1791, the direct and ruler triggers left of the axis
  {
    std::vector<double> ys;
    for (const RhythmPanelEntry& e : p.left) {
      ys.push_back(e.y);
    }
    const std::vector<double> rows = rhythm_label_rows(ys);
    const double dx = p.dated ? 0.0 : 6.0;
    for (std::size_t i = 0; i < p.left.size(); ++i) {
      const RhythmPanelEntry& e = p.left[i];
      // @line(x1& - 4,a&,x1&,a&), the tick keeps the exact height
      line(dl, kAxisX - 4.0, e.y, kAxisX, e.y);
      text(dl, dx + 2.0, rows[i] + 3.0, 9.0, e.label);
      glyph(dl, kAxisX - 14.0, rows[i], e.slot, e.inverse);
    }
  }
  // a1792, the aspect and mirror partners right of the axis
  {
    std::vector<double> ys;
    for (const RhythmPanelEntry& e : p.right) {
      ys.push_back(e.y);
    }
    const std::vector<double> rows = rhythm_label_rows(ys);
    const double dx = p.dated ? 13.0 : 6.0;
    for (std::size_t i = 0; i < p.right.size(); ++i) {
      const RhythmPanelEntry& e = p.right[i];
      line(dl, kAxisX, e.y, kAxisX + 4.0, e.y);
      const double y = rows[i];
      glyph(dl, 113.0, y, e.slot, false);
      text(dl, dx + 111.0, y + 3.0, 9.0, e.label);
      if (e.mirror) {
        text(dl, 189.0, y + 3.0, 9.0, "S");
      } else {
        Primitive a;
        a.kind = Primitive::Kind::kGlyph;
        a.x1 = 187.0;
        a.y1 = y;
        a.size = kSpriteSize;
        a.text = aspect_glyph(e.family);
        dl.items.push_back(a);
      }
      glyph(dl, 205.0, y, e.source, false);
    }
  }
  // @text(20,456,13,"WEITER mit " + lt$)
  text(dl, 20.0, 456.0, 13.0, p.footer);
}

}  // namespace horcom
