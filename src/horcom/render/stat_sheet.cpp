// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/stat_sheet.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "horcom/chart/signs.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/items.hpp"

namespace horcom {

namespace {

// his text size 13 of the list, 9 of the KLEIN lines and the percents,
// 11 of the vertical boxes
constexpr double kListText = 13.0;
constexpr double kSmallText = 9.0;
constexpr double kBoxText = 11.0;
constexpr double kGlyph = 10.0;
constexpr Rgb kInk = 0x000000;
constexpr Rgb kWhite = 0xFFFFFF;
// DEFFILL 5, his dotted fill of the header, the averaged grey it showed
constexpr Rgb kHeaderGrey = 0xD8D8D8;
// the elem_col inks of inf_box22, fire RGB(255,0,0), earth RGB(160,160,0),
// air RGB(0,255,255) and water RGB(0,0,255)
constexpr Rgb kElementInk[4] = {0xFF0000, 0xA0A000, 0x00FFFF, 0x0000FF};
// the 28 places of his so$ labels, the inverse bar spans them
constexpr double kLabelChars = 28.0;
// the longest bar of inf_box22, s% = sum%(i&) * 60 / m%
constexpr double kBarLength = 60.0;

struct Canvas {
  DisplayList dl;

  // his textc, x the left edge and y the bottom line
  void text(double x, double bottom, const std::string& s, double size = kListText, Rgb ink = kInk) {
    Primitive p;
    p.kind = Primitive::Kind::kText;
    p.x1 = x;
    p.y1 = bottom - 0.5 * size;
    p.size = size;
    // te_w& = @textg(te_gr&), FONT WIDTH te_w&
    p.pitch = font_pitch(size);
    p.align_left = true;
    p.color = ink;
    p.text = s;
    dl.items.push_back(std::move(p));
  }
  // his texts, the escapement 900 font reading upward from x, y
  void vertical(double x, double y, const std::string& s, double size) {
    Primitive p;
    p.kind = Primitive::Kind::kText;
    p.x1 = x - 0.5 * size;
    p.y1 = y;
    p.size = size;
    //RR tw& = @textg(te_gr&), WIDTH tw& * gdx
    p.pitch = font_pitch(size);
    p.align_left = true;
    p.vertical = true;
    p.text = s;
    dl.items.push_back(std::move(p));
  }
  void line(double x1, double y1, double x2, double y2) {
    dl.items.push_back({Primitive::Kind::kLine, x1, y1, x2, y2, 0, 0, 0, 0, 0, kInk, kWhite,
                        Primitive::Style::kSolid, 1.0});
  }
  // his boxn
  void frame(double xa, double ya, double xe, double ye) {
    line(xa, ya, xe, ya);
    line(xe, ya, xe, ye);
    line(xe, ye, xa, ye);
    line(xa, ye, xa, ya);
  }
  void fill(double xa, double ya, double xe, double ye, Rgb colour) {
    Primitive r;
    r.kind = Primitive::Kind::kRect;
    r.x1 = 0.5 * (xa + xe);
    r.y1 = 0.5 * (ya + ye);
    r.r1 = 0.5 * std::abs(xe - xa);
    r.r2 = 0.5 * std::abs(ye - ya);
    r.fill = colour;
    dl.items.push_back(std::move(r));
  }
  void glyph(double x, double y, const std::string& g, double size = kGlyph) {
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = x;
    p.y1 = y;
    p.size = size;
    p.text = g;
    dl.items.push_back(std::move(p));
  }
};

// ported from grzemise(pa,0,x,y,0,-1,0,13), fp! off, the degree at x,
// the small sign sprite and the rounded minute. His grze_0 carried the
// second and the minute before CINT printed the minute, so 59.5' read
// 60', the split rounds first and carries into degree and sign
void grzemise(Canvas& c, double x, double y, double rad) {
  const ZodiacSplit z = split_zodiac(rad, false);
  char buf[16];
  // LEFT$(STR$(c1,2,0),2) + "°"
  std::snprintf(buf, sizeof(buf), "%2d\xC2\xB0", z.deg);
  c.text(x, y, buf);
  // zeichp_dspl(c2,x& + 44,yi& - 11) with x& = xi& - 16
  c.glyph(x + 35.0, y - 6.0, sign_glyph(z.sign));
  std::snprintf(buf, sizeof(buf), "%2d'", z.min);
  c.text(x + 46.0, y, buf);
}

// his grmise with grmi$, degrees and minutes of a separation, the
// rounded minute carries into the degree
std::string grmi(double rad) {
  const double d = std::abs(rad) * kRadToDeg;
  int deg = static_cast<int>(d);
  int minute = static_cast<int>(std::lround((d - deg) * kArcminPerDeg));
  if (minute == static_cast<int>(kArcminPerDeg)) {
    minute = 0;
    ++deg;
  }
  char buf[24];
  std::snprintf(buf, sizeof(buf), "%3d\xC2\xB0%2d'", deg, minute);
  return buf;
}

// ported from list_ausg_ueb, the header box and the column heads
void header(Canvas& c, const StatSheetText& t) {
  c.fill(1.0, 1.0, 638.0, 31.0, kHeaderGrey);
  c.line(1.0, 32.0, 638.0, 32.0);
  c.line(1.0, 33.0, 638.0, 33.0);
  if (!t.heads.empty()) {
    c.text(396.0, 29.0, t.heads);
  }
  c.text(396.0, 17.0, t.file);
  // @seite, STR$(a&,3) at (610,24)
  char buf[8];
  std::snprintf(buf, sizeof(buf), "%3d", t.page);
  c.text(610.0, 24.0, buf);
}

// ported from inf_box1 with inf_box11
void condition_box(Canvas& c, const StatSheetText& t) {
  if (t.multi || (t.object_line.empty() && t.window_line.empty())) {
    return;
  }
  const double x = t.aspect_box ? 4.0 : 90.0;
  // 5 + 8 * IMAX(15,LEN(b$),LEN(c$),LEN(p$))
  const std::size_t chars = std::max<std::size_t>({15, t.object_line.size(), t.window_line.size()});
  const double w = 5.0 + 8.0 * static_cast<double>(chars);
  c.fill(x, 2.0, x + w, 30.0, kWhite);
  c.text(x + 4.0, 18.0, t.object_line);
  // IF obj& = 4, "ISTWERT / GRAD"
  //RR ASPEKT
  c.text(x + 4.0, 30.0, t.aspect_box ? t.label_aspect : t.window_line);
  c.frame(x, 2.0, x + w, 30.0);
}

// ported from inf_box2 with inf_box22, inf_box2_ds and inf_box20
void bars(Canvas& c, const StatSheetText& t) {
  int most = 0;
  int sum = 0;
  for (int i = 1; i <= 12; ++i) {
    most = std::max(most, t.sums[static_cast<std::size_t>(i)]);
    sum += t.sums[static_cast<std::size_t>(i)];
  }
  for (int i = 1; i <= 12; ++i) {
    const double k = i * 24.0;
    const double s = most > 0 ? t.sums[static_cast<std::size_t>(i)] * kBarLength / most : 0.0;
    if (t.by_house) {
      // @texts(12,345 - i& * 24,11,STR$(i&,2))
      char buf[8];
      std::snprintf(buf, sizeof(buf), "%2d", i);
      c.vertical(12.0, 345.0 - k, buf, kBoxText);
    } else {
      // PUT @xk(4),@yk(335 - k&),zes&(i&)
      c.glyph(10.0, 341.0 - k, sign_glyph(i - 1), kGlyph);
    }
    const Rgb ink = kElementInk[(i - 1) % 4];
    if (s > 0.0) {
      c.fill(16.0, 350.0 - k - 24.0, 16.0 + s, 350.0 - k, ink);
    }
    c.frame(16.0, 350.0 - k - 24.0, 16.0 + s, 350.0 - k);
    c.line(1.0, 350.0 - k - 24.0, 16.0, 350.0 - k - 24.0);
    c.line(1.0, 350.0 - k, 16.0, 350.0 - k);
    // @textrc(36,349 - i& * 24 - 7,9,STR$(100 * s,4,1) + "%")
    char pct[16];
    std::snprintf(pct, sizeof(pct), "%4.1f%%", sum > 0 ? kPercent * t.sums[static_cast<std::size_t>(i)] / sum : 0.0);
    const double y = 349.0 - k - 7.0;
    c.fill(36.0, y - kSmallText, 36.0 + 6.0 * 5.0, y + 1.0, kWhite);
    c.text(36.0, y, pct, kSmallText);
    c.frame(34.0, y - kSmallText - 2.0, 36.0 + 6.0 * 5.0, y + 1.0);
  }
  if (t.framed >= 1 && t.framed <= 12 && most > 0) {
    // the searched sign or house gets a second frame
    const double k = t.framed * 24.0;
    const double s = t.sums[static_cast<std::size_t>(t.framed)] * kBarLength / most;
    if (s > 2.0) {
      c.frame(17.0, 350.0 - k - 24.0 + 1.0, 16.0 + s - 1.0, 350.0 - k - 1.0);
    }
  }
  c.line(16.0, 326.0, 16.0, 62.0);
  c.text(4.0, 16.0, t.object_tag);
  c.text(4.0, 28.0, t.by_house ? t.label_houses : t.label_signs);
  c.text(6.0, 344.0, t.label_total);
  char buf[24];
  std::snprintf(buf, sizeof(buf), "%4d", t.total);
  c.text(24.0, 354.0, buf);
  if (t.partial) {
    c.text(6.0, 366.0, t.label_partial);
    std::snprintf(buf, sizeof(buf), "%d=%5.1f%%", t.partial_count,
                  t.total > 0 ? kPercent * t.partial_count / t.total : 0.0);
    c.text(6.0, 376.0, buf);
    c.frame(4.0, 330.0, 76.0, 376.0);
    c.frame(5.0, 331.0, 75.0, 375.0);
  } else {
    c.frame(4.0, 330.0, 76.0, 356.0);
    c.frame(5.0, 331.0, 75.0, 355.0);
  }
}

// ported from inf_box21, the three counts reading upward
void counts_box(Canvas& c, const StatSheetText& t) {
  const double x = t.counts_x;
  c.frame(x, 40.0, x + 38.0, 370.0);
  c.frame(x - 1.0, 39.0, x + 39.0, 371.0);
  for (int i = 0; i < 3; ++i) {
    c.vertical(x + 12.0 * (i + 1), 365.0, t.counts[static_cast<std::size_t>(i)], kBoxText);
  }
}

// ported from inf_box3, the conditions reading upward
void conditions_box(Canvas& c, const StatSheetText& t) {
  c.frame(4.0, 40.0, 96.0, 370.0);
  c.frame(3.0, 39.0, 97.0, 371.0);
  const bool many = t.conditions.size() >= 8;
  // d& = 12, g& = 11, from eight conditions d& = 7, g& = 8
  const double step = many ? 7.0 : 12.0;
  const double size = many ? 8.0 : 11.0;
  for (std::size_t i = 0; i < t.conditions.size(); ++i) {
    c.vertical(4.0 + step * static_cast<double>(i + 1), 365.0, t.conditions[i], size);
  }
}

// ported from inf_box4, the box of the right mouse button over the rows,
// its conditions in a second box below under several conditions
void info_box(Canvas& c, const StatSheetText& t) {
  const double n = t.multi ? static_cast<double>(t.conditions.size()) : 0.0;
  const double e = 290.0 - n * 12.0;
  // getbm and putbm mode 0 clear the corner first
  c.fill(290.0, e, 632.0, 372.0, kWhite);
  c.frame(290.0, e, 632.0, e + 66.0);
  c.frame(291.0, e + 1.0, 631.0, e + 65.0);
  // his lines at e + 16, 28, 40 and under UND at 54 and 66
  static constexpr double kLineY[5] = {16.0, 28.0, 40.0, 54.0, 66.0};
  for (std::size_t i = 0; i < t.info_counts.size() && i < 5; ++i) {
    c.text(295.0, e + kLineY[i], t.info_counts[i]);
  }
  c.line(290.0, e + 41.0, 632.0, e + 41.0);
  if (t.multi) {
    const double d = 354.0 - n * 12.0;
    c.frame(290.0, d, 632.0, d + 5.0 + 12.0 * n);
    c.frame(291.0, d + 1.0, 631.0, d + 4.0 + 12.0 * n);
    for (std::size_t i = 0; i < t.conditions.size(); ++i) {
      c.text(295.0, d + static_cast<double>(i + 1) * 12.0 + 4.0, t.conditions[i]);
    }
  }
}

// ported from kltext, the moment and in the GROß list the AC or MC
void moment(Canvas& c, const StatSheetText& t, const StatSheetRow& r, double y) {
  constexpr double x = 374.0;
  // his rule at x% + 96 cut the tens of a two digit hour, the date ends
  // at 11 of his cells and the hour starts at 13, the rule stands in the
  // two blanks between
  constexpr double kTimeRule = 84.0;
  if (!t.small) {
    if (r.has_angle) {
      grzemise(c, 556.0, y, r.angle);
    }
    c.text(x, y, r.moment);
    c.line(x + kTimeRule, y, x + kTimeRule, y - 16.0);
    c.line(x + 174.0, y, x + 174.0, y - 16.0);
  } else {
    c.text(x, y - 6.0, r.moment, kSmallText);
    c.text(x, y + 1.0, r.moment2, kSmallText);
  }
  c.line(x - 4.0, y, x - 4.0, y - 16.0);
}

}  // namespace

// ported from list_ausg with list_ausg_1 and list_ausg_ueb
DisplayList build_stat_page(const std::vector<StatSheetRow>& rows, const StatSheetText& text) {
  Canvas c;
  c.dl.width = kCanvasWidth;
  c.dl.height = kCanvasHeight;
  header(c, text);
  condition_box(c, text);
  if (text.bars) {
    bars(c, text);
  }
  if (text.several) {
    c.text(8.0, 28.0, text.label_several);
  }
  if (text.counts_x > 0) {
    counts_box(c, text);
  }
  if (!text.conditions.empty()) {
    conditions_box(c, text);
  }
  int zl = 0;
  for (const StatSheetRow& r : rows) {
    if (++zl > kStatRowsPerPage) {
      break;
    }
    // y% = zl& * 16 + 34
    const double y = zl * 16.0 + 34.0;
    if (!text.multi) {
      if (!r.tag.empty()) {
        c.text(80.0, y, r.tag);
      } else if (r.slot > 0) {
        // plan_ds, plein2(mz|,x% + 6,y% - 7)
        c.glyph(86.0, y - 7.0, body_glyph(r.slot));
      }
      if (!r.has_value) {
        c.text(92.0, y, " -------");
      } else if (r.aspect) {
        c.text(90.0, y, grmi(r.value));
      } else {
        grzemise(c, 92.0, y, r.value);
      }
      moment(c, text, r, y);
      c.text(164.0, y, r.label);
      c.line(160.0, y, 160.0, y - 16.0);
      c.line(78.0, y, 638.0, y);
      c.line(78.0, y, 78.0, y - 16.0);
    } else {
      if (r.inverse) {
        // deftextcol(0), white on black
        c.fill(141.0, y - 15.0, 142.0 + 8.0 * kLabelChars, y - 1.0, kInk);
        c.text(142.0, y, r.label, kListText, kWhite);
      } else {
        c.text(142.0, y, r.label);
      }
      moment(c, text, r, y);
      c.line(140.0, y, 638.0, y);
    }
  }
  c.line(1.0, 32.0, 638.0, 32.0);
  c.line(1.0, 33.0, 638.0, 33.0);
  // @textzent(459 - 26,13,...) and 459 - 14 over 145 entries
  c.dl.items.push_back(screen_text_centred(433.0, kListText, text.footer));
  if (!text.footer2.empty()) {
    c.dl.items.push_back(screen_text_centred(445.0, kListText, text.footer2));
  }
  if (text.info) {
    info_box(c, text);
  }
  return c.dl;
}

}  // namespace horcom
