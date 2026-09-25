// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/grad_sheet.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

#include "horcom/chart/signs.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/items.hpp"

namespace horcom {

namespace {

constexpr Rgb kInk = 0x000000;
// his dx& = 104, dz& = 10, dh& = 410, five columns of 41 rows
constexpr double kColumnStep = 104.0;
constexpr double kRowStep = 10.0;
constexpr int kRows = 41;
constexpr int kColumns = 5;
// his xt& = 3, yt& = 36, a18list steps before the first row
constexpr double kFirstX = 3.0;
constexpr double kFirstY = 46.0;
// the panel of listscal, 0 degrees at y 440 and 360 at y 80
constexpr double kPanelBottom = 440.0;
constexpr double kPanelTop = 80.0;
constexpr double kScaleLeft = 574.0;
constexpr double kScaleRight = 624.0;
constexpr double kBarStart = 576.0;
// the whole degrees of the scale, one pixel each
constexpr auto kCircleDegrees = static_cast<int>(kDegPerCircle);
// his text heights, 13 for the heads of grlinit and listscal, 12 for the
// rows, 10 for the page of bl_anz, 9 for the scale labels of skalv
constexpr double kHeadText = 13.0;
constexpr double kRowText = 12.0;
constexpr double kPageText = 10.0;
constexpr double kScaleText = 9.0;

void line(DisplayList& dl, double x1, double y1, double x2, double y2, double w = 1.0) {
  Primitive p;
  p.kind = Primitive::Kind::kLine;
  p.x1 = x1;
  p.y1 = y1;
  p.x2 = x2;
  p.y2 = y2;
  p.color = kInk;
  p.width = w;
  dl.items.push_back(p);
}

// his textc, x the left edge and y the bottom line
void text(DisplayList& dl, double x, double bottom, double size, const std::string& t) {
  if (!t.empty()) {
    dl.items.push_back(screen_text(x, bottom, size, t, kInk));
  }
}

// ported from skalv with w4d = 0, one pixel per degree. His global w4d
// kept the base angle of the last transit or direction run and bent this
// scale, the list has no base angle
void scale(DisplayList& dl, double x, bool labels) {
  line(dl, x, kPanelBottom, x, kPanelTop);
  for (int t = 0; t <= kCircleDegrees; ++t) {
    const double y = kPanelBottom - t;
    if (t % 2 == 0) {
      line(dl, x - 3.0, y, x, y);
    }
    if (t % 10 == 0) {
      if (labels) {
        // his RSET f$ = STR$(t&), three places
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%3d", t);
        text(dl, x - 26.0, y + 2.0, kScaleText, buf);
      }
      line(dl, x - 6.0, y, x, y);
    }
  }
}

// ported from listscal
void panel(DisplayList& dl, const std::vector<GradEntry>& entries, int unit, const GradSheetText& t) {
  text(dl, 530.0, 48.0, kHeadText, t.panel_head);
  text(dl, 530.0, 60.0, kHeadText, t.panel_sub);
  scale(dl, kScaleLeft, true);
  scale(dl, kScaleRight, false);
  const int h = std::max(1, unit);
  const int columns = static_cast<int>(kScaleRight - kBarStart) / h;
  for (int i = 1; i <= columns; ++i) {
    const double x = kBarStart + i * h;
    line(dl, x, kPanelBottom, x, kPanelTop, 0.5);
    // his IF EVEN(i&) OR h& > 10
    if (i % 2 == 0 || h > 10) {
      text(dl, x - 4.0, kPanelTop, kScaleText, std::to_string(i));
    }
  }
  // his ADD fsk|(yd&),h&, one bar a degree
  std::array<int, kCircleDegrees + 1> sum{};
  for (const GradEntry& e : entries) {
    const int yd = std::clamp(static_cast<int>(std::trunc(e.deg)), 0, kCircleDegrees);
    sum[static_cast<std::size_t>(yd)] += h;
    line(dl, kBarStart, kPanelBottom - yd, std::min(kBarStart + sum[static_cast<std::size_t>(yd)], kScaleRight),
         kPanelBottom - yd);
  }
  // the sign sprites and their boundaries
  for (int u = 1; u <= kSignCount; ++u) {
    Primitive g;
    g.kind = Primitive::Kind::kGlyph;
    g.x1 = 532.0;
    g.y1 = 454.0 - 30.0 * u;
    g.size = 11.0;
    g.text = sign_glyph(u - 1);
    g.color = kInk;
    dl.items.push_back(g);
  }
  for (int w = 0; w <= kSignCount; ++w) {
    line(dl, 524.0, kPanelBottom - 30.0 * w, 540.0, kPanelBottom - 30.0 * w);
  }
}

}  // namespace

// ported from grlinit, the grid of a18list and bl_anz
DisplayList build_grad_sheet(const std::vector<GradEntry>& entries, int page, const GradSheetText& t, bool with_panel,
                             int unit) {
  DisplayList dl;
  dl.width = 640.0;
  dl.height = 480.0;
  text(dl, 8.0, 14.0, kHeadText, t.title);
  text(dl, 520.0, 14.0, kHeadText, t.frame);
  line(dl, 3.0, 30.0, 638.0, 30.0);
  text(dl, 8.0, 28.0, kHeadText, t.ephem);
  for (int c = 1; c <= kColumns; ++c) {
    line(dl, c * kColumnStep, 30.0, c * kColumnStep, 457.0);
  }
  const std::size_t first = static_cast<std::size_t>(page) * kGradPerPage;
  for (std::size_t k = first; k < entries.size() && k < first + kGradPerPage; ++k) {
    const int i = static_cast<int>(k - first);
    //RR @textc(xt&,yt&,12,q$)
    text(dl, kFirstX + (i / kRows) * kColumnStep, kFirstY + (i % kRows) * kRowStep, kRowText, entries[k].text);
  }
  // his bl_anz(bla&,610,26), the page number in its box
  char num[16];
  std::snprintf(num, sizeof(num), "%2d", page + 1);
  text(dl, 610.0, 26.0, kPageText, num);
  line(dl, 606.0, 14.0, 630.0, 14.0);
  line(dl, 630.0, 14.0, 630.0, 26.0);
  line(dl, 630.0, 26.0, 606.0, 26.0);
  line(dl, 606.0, 26.0, 606.0, 14.0);
  if (with_panel) {
    panel(dl, entries, unit, t);
  }
  return dl;
}

}  // namespace horcom
