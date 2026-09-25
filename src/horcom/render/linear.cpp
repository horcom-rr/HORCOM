// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/linear.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <vector>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/signs.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/items.hpp"
#include "horcom/time/calendar.hpp"

namespace horcom {

namespace {

// his sheet, the band of 360 pixels folds the base angle
constexpr double kBandTop = kLinearBandTop;
constexpr double kBandBottom = kLinearBandBottom;
constexpr double kFrameLeft = kLinearFrameLeft;
constexpr double kFrameRight = kLinearFrameRight;
constexpr double kLineLeft = 54.0;
constexpr double kLineRight = 622.0;
// xa& = 80, xe& = 568, the time axis of skalh
constexpr double kAxisStart = 80.0;
constexpr double kAxisEnd = 568.0;
// CLIP 75 * gdx,60 * gdy TO 575 * gdx,459 * gdy
constexpr double kClipLeft = 75.0;
constexpr double kClipRight = 575.0;
// xx&(i&) = 42, the glyph column left and the one at 536 + xx&
constexpr double kGlyphLeft = 42.0;
constexpr double kGlyphRightShift = 536.0;
constexpr double kSignShift = 548.0;
// a glyph is stamped along a curve after this much drawn path and only
// clear of earlier stamps, his sl gate and the lins linw memory
constexpr double kStampPath = 80.0;
constexpr double kStampClearX = 30.0;
constexpr double kStampClearY = 20.0;
// his grid step of the directions
constexpr int kGridLife = 24;

// the width of one sign in radians, his PI / 6
constexpr double kSignRad = kPi / 6.0;

constexpr Rgb kGreen = 0x00A000;
constexpr Rgb kGridGreen = 0x009600;
constexpr Rgb kBlue = 0x0000FF;

// ported from HORCOM a18_line_col, the three colour classes
Rgb line_color(int slot) {
  switch (slot) {
    case 1:
    case 2:
    case 3:
      return kGreen;
    case 5:
    case 7:
    case 8:
    case 9:
    case 10:
      return kMarkRed;
    case 4:
    case 6:
    case 11:
    case 12:
      return kBlue;
    default:
      return kInkColor;
  }
}

// ported from rot_grn_line, the colour of a hit line by the running
// body, a mundane hit reads red only when both bodies are heavy
Rgb hit_line_color(int slot) {
  switch (slot) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
    case 11:
    case 12:
    case 13:
    case 14:
      return kGridGreen;
    case 5:
    case 7:
    case 8:
    case 9:
    case 10:
      return kMarkRed;
    default:
      return kBlue;
  }
}

// the heavy bodies of the mundane hit lines in a181, Mars to Pluto
bool mundane_heavy(int slot) {
  return slot >= body::kMars && slot <= body::kPluto;
}

// his mo_na, the months and their days
constexpr const char* kMonth[12] = {"Januar", "Februar", "M\xC3\xA4rz",   "April",   "Mai",      "Juni",
                                    "Juli",   "August",  "September", "Oktober", "November", "Dezember"};

int month_days(int month, int year) {
  static constexpr int kDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) {
    return 29;
  }
  return kDays[month - 1];
}

struct Canvas {
  DisplayList dl;
  void line(double x1, double y1, double x2, double y2, Rgb c = kInkColor, double w = 1.0,
            Primitive::Style st = Primitive::Style::kSolid) {
    Primitive p;
    p.kind = Primitive::Kind::kLine;
    p.x1 = x1;
    p.y1 = y1;
    p.x2 = x2;
    p.y2 = y2;
    p.color = c;
    p.width = w;
    p.style = st;
    dl.items.push_back(p);
  }
  // his text and textc, x the left edge and y the bottom line
  void text(double x, double bottom, double size, const std::string& t, Rgb c = kInkColor) {
    dl.items.push_back(screen_text(x, bottom, size, t, c));
  }
  void glyph(double x, double y, double size, const std::string& t, Rgb c) {
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = x;
    p.y1 = y;
    p.size = size;
    p.text = t;
    p.color = c;
    dl.items.push_back(p);
  }
};

}  // namespace

int linear_pixels_per_step(const LinearOptions& opt) {
  if (opt.kind == LinearKind::kTransits || opt.kind == LinearKind::kMundane) {
    // nj& = CINT(jdend - jdbeg), 1 Monat, 4 Monate, 16 Monate
    const int nj = static_cast<int>(std::lround(opt.jd_to_ut - opt.jd_from_ut));
    if (nj < 120) {
      return 16;
    }
    return nj <= 125 ? 4 : 1;
  }
  // j& = CINT(lje - lja)
  const int j = static_cast<int>(std::lround(opt.to_years - opt.from_years));
  if (j <= 6) {
    return 96;
  }
  if (j <= 11) {
    return 48;
  }
  if (j <= 21) {
    return 24;
  }
  if (j <= 41) {
    return 12;
  }
  return j <= 81 ? 6 : 3;
}

// ported from the zeichen lines of a18_lin, the boundaries folded one by
// one
std::vector<double> linear_sign_lines(double base_angle_deg) {
  std::vector<double> out;
  const double w = kDegPerCircle / base_angle_deg;
  for (int i = 1; i <= 12; ++i) {
    const double f = norm_deg(w * i * kDegPerSign);
    if (f < kEps || f > kDegPerCircle - kEps) {
      continue;
    }
    if (std::none_of(out.begin(), out.end(), [f](double o) { return std::abs(o - f) < 1.0e-6; })) {
      out.push_back(f);
    }
  }
  return out;
}

// ported from HORCOM a18_lin, a180trpr_lin, skalh and skalv
DisplayList build_linear_graph(const Chart& radix, const LinearOptions& opt, const SearchContext& ctx) {
  Canvas cv;
  cv.dl.height = 460.0;
  const double w = kDegPerCircle / opt.base_angle_deg;  // his Teiler
  const bool mundane = opt.kind == LinearKind::kMundane;
  const bool dates = opt.kind == LinearKind::kTransits || mundane;
  const int nbl = linear_pixels_per_step(opt);
  const double tja = radix.ta.tropical_year_days;
  const Calendar cal = ctx.settings.calendar;
  // lin_inv(ylin&) = 480 - ylin& when the ordinate runs downward
  const auto inv = [&](double y) { return opt.downward ? 480.0 - y : y; };
  const auto fold_deg = [&](double rad) { return norm_rad(w * rad) * kRadToDeg; };
  const auto to_y = [&](double rad) { return inv(kBandBottom - fold_deg(rad)); };
  // one pixel of the axis is 1/nbl day or year. His date axis counts the
  // day of the month ta1 in, the first of the month stands on the tick 1
  // of skalh one step right of the axis start
  const double ta1 = dates ? calendar_date(opt.jd_from_ut, cal).day : 0.0;
  const auto x_of_jd = [&](double jd) {
    if (dates) {
      return kAxisStart + nbl * (ta1 + jd - opt.jd_from_ut);
    }
    return kAxisStart + nbl * ((jd - radix.jd_ut) / tja - opt.from_years);
  };
  char buf[160];

  // the header of a18kopf in its lin! branch, mund! names no record
  const auto header = [&](double x, double y, const std::string& t) {
    if (!t.empty()) {
      cv.text(x, y, 13, t);
    }
  };
  header(4, 15, opt.title);
  header(8.0 * opt.title.size() + 24, 14, opt.record);
  header(4, 25, opt.name);
  header(364, 25, opt.place);
  if (dates) {
    header(504, 14, opt.start);
  }
  if (ctx.settings.heliocentric) {
    if (mundane) {
      cv.text(264, 30, 13, "Heliozentrisch ");
    }
    cv.text(250, 50, 13, " Heliozentrisch ");
  }
  // the frame, the double rule under the header and the band box
  cv.line(2, 36, 638, 36);
  cv.line(2, 37, 638, 37);
  cv.line(1, 1, 639, 1);
  cv.line(639, 1, 639, 458);
  cv.line(639, 458, 1, 458);
  cv.line(1, 458, 1, 1);
  cv.line(kFrameLeft, kBandTop, kFrameRight, kBandTop);
  cv.line(kFrameRight, kBandTop, kFrameRight, kBandBottom);
  cv.line(kFrameRight, kBandBottom, kFrameLeft, kBandBottom);
  cv.line(kFrameLeft, kBandBottom, kFrameLeft, kBandTop);
  cv.text(4, 48, 9, "GRUNDWINKEL");
  cv.text(4, 56, 9, "( GRAD )");

  // skalv, the degree scale down the left edge of the band
  {
    const int steps = static_cast<int>(std::lround(opt.base_angle_deg));
    for (int t = 0; t <= steps; ++t) {
      const double y = std::round(inv(kBandBottom - t * kDegPerCircle / opt.base_angle_deg));
      const long yi = std::lround(y);
      if (yi % 2 == 0) {
        cv.line(kFrameLeft - 3, y, kFrameLeft, y);
      }
      if (yi % 10 == 0) {
        std::snprintf(buf, sizeof(buf), "%3d", t);
        cv.text(kFrameLeft + 2, y + 5, 9, buf);
        cv.line(kFrameLeft - 6, y, kFrameLeft, y);
      }
      if (yi % 30 == 0) {
        cv.line(kFrameLeft - 6, y, kFrameLeft, y);
      }
    }
  }

  // the sign boundaries, blue dash dot, his zeichen!
  if (opt.signs) {
    for (const double f : linear_sign_lines(opt.base_angle_deg)) {
      const double y = inv(kBandBottom - f);
      if (y > kBandTop && y < kBandBottom) {
        cv.line(kLineLeft, y, kLineRight, y, kBlue, 1.0, Primitive::Style::kDashDot);
      }
    }
  }

  // the radix lines with the glyph columns, a18_entz spreads close ones
  struct RadixLine {
    int slot = 0;
    double y = -1.0;
    double y2 = -1.0;  // the opposite cusp of a house line
    double x = kGlyphLeft;
    std::string label;
  };
  std::vector<RadixLine> lines;
  for (int slot = 1; slot < body::kSlotCount; ++slot) {
    if (slot >= 15 && slot <= 18) {
      continue;
    }
    const BodyState& b = radix.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid) {
      continue;
    }
    lines.push_back({slot, to_y(b.el), -1.0, kGlyphLeft, {}});
  }
  if (opt.with_houses && radix.houses.ok) {
    // 15,16 carry H2 H3 with H8 H9, 17,18 carry H5 H6 with H11 H12
    for (const int c : {2, 3, 5, 6}) {
      RadixLine l;
      l.slot = 100 + c;
      l.y = to_y(radix.houses.cusp[static_cast<std::size_t>(c)]);
      l.y2 = to_y(radix.houses.cusp[static_cast<std::size_t>(c + 6)]);
      l.label = "H" + std::to_string(c);
      lines.push_back(l);
    }
  }
  // a18_entz, a label closer than ten pixels steps nine to the right
  for (std::size_t i = 0; i < lines.size(); ++i) {
    for (std::size_t j = 0; j < lines.size(); ++j) {
      if (i != j && std::abs(lines[j].y - lines[i].y) < 10.0 && std::abs(lines[i].x - lines[j].x) < 4.0 &&
          lines[i].y > 0 && lines[j].y > 0) {
        lines[j].x += 9.0;
      }
    }
  }
  // a18_lin draws the radix only under tras!, prog!, sobg! and mob!, at
  // 360 degrees the mundane graph names the signs down both edges
  if (mundane && opt.base_angle_deg >= kDegPerCircle) {
    for (int u = 1; u <= kSignCount; ++u) {
      const int sign = opt.downward ? kSignCount - u : u - 1;
      const double y = 434.0 - 30.0 * u;
      cv.glyph(45, y, 9, sign_glyph(sign), kInkColor);
      cv.glyph(605, y, 9, sign_glyph(sign), kInkColor);
    }
  }
  for (const RadixLine& l : lines) {
    if (mundane) {
      break;
    }
    const Rgb col = l.slot < 100 ? line_color(l.slot) : kInkColor;
    for (const double y : {l.y, l.y2}) {
      if (y > kBandTop && y < kBandBottom) {
        cv.line(kLineLeft, y, kLineRight, y, col, opt.line_width);
      }
    }
    if (l.slot >= 100) {
      cv.text(l.x, l.y + 3, 9, l.label);
      std::snprintf(buf, sizeof(buf), "H%d", l.slot - 100 + 6);
      cv.text(l.x, l.y2 + 3, 9, buf);
      continue;
    }
    const double el = radix.b[static_cast<std::size_t>(l.slot)].el;
    cv.glyph(kSignShift + l.x, l.y - 5, 9, sign_glyph(static_cast<int>(norm_rad(el) / kSignRad) % kSignCount), kInkColor);
    // IF NOT i& = 12
    if (l.slot != body::kNodeDesc) {
      cv.glyph(l.x, l.y - 5, 9, body_glyph(l.slot), col);
      cv.glyph(kGlyphRightShift + l.x, l.y - 5, 9, body_glyph(l.slot), col);
    }
  }

  // skalh, the time axis under the band
  {
    const double ya = kBandBottom;
    // h& = 446, 450 without an event place
    const double h = 446.0;
    if (dates) {
      const CalendarDate d0 = calendar_date(opt.jd_from_ut, cal);
      int month = d0.month;
      int year = d0.year;
      std::snprintf(buf, sizeof(buf), "%5d", year);
      cv.text(70, h, 10, year > 0 ? buf : std::to_string(1 - year) + " vC");
      cv.text(15, h, 10, "Jahr");
      int i = 0;
      int za = 0;
      int ie = nbl * month_days(month, year);
      for (double x = kAxisStart + 1.0; x <= kAxisEnd + nbl; x += 1.0) {
        ++i;
        if (i == 5 && kAxisEnd - kAxisStart - i > 20) {
          // x0& = x& + 14 * (nbl% - 1), the month in its middle
          const double x0 = x + 14.0 * (nbl - 1);
          if (x0 < 570.0) {
            if (nbl == 16) {
              cv.text(x0, 436, 10, kMonth[month - 1]);
            } else {
              cv.text(x0, 436, 10, std::string(kMonth[month - 1]).substr(0, month == 3 ? 4 : 3));
              if (month == 1 && (month != d0.month || x0 - kAxisStart > 50)) {
                std::snprintf(buf, sizeof(buf), "%5d", year);
                cv.text(x0 - 8, h, 10, buf);
              }
            }
          }
        } else {
          // skalh1 with k& = 2, day ticks and a longer one every fifth
          const double a = static_cast<double>(i) / 2.0 / nbl;
          const bool day = nbl == 1 ? (a == std::floor(a)) : (i % nbl == 0);
          if (day) {
            cv.line(x, ya, x, ya + 4);
            if (nbl != 1 && ++za == 5) {
              cv.line(x, ya, x, ya + 6);
              za = 0;
            }
          }
        }
        const int md = month_days(month, year);
        if (i == nbl * 10 || i == nbl * 20 || i == nbl * 30 || i == ie) {
          cv.line(x, ya, x, ya + 8);
        }
        if (i == nbl && nbl == 1) {
          cv.line(x, ya, x, ya + 12);
        }
        // the day numbers in red, 1 10 20 and the month's last
        if (nbl == 16 || nbl == 4) {
          std::string label;
          if (i == nbl && (nbl == 16 || x < 100.0)) {
            label = "1";
          } else if (i == nbl * 10) {
            label = "10";
          } else if (i == nbl * 20) {
            label = "20";
          } else if (i == nbl * 30 && md != 29 && md != 28) {
            label = "30";
          } else if (month == 2 && i == nbl * 29 && md == 29) {
            label = "29";
          } else if (month == 2 && i == nbl * 28 && md == 28) {
            label = "28";
          } else if (nbl == 16 && i == nbl * 5) {
            label = "5";
          } else if (nbl == 16 && i == nbl * 25) {
            label = "25";
          }
          if (!label.empty()) {
            cv.text(x - (label.size() == 1 && i == nbl ? 4.0 : 8.0), ya + 18, 9, label, kMarkRed);
          }
        }
        if (i == ie) {
          cv.line(x, ya, x, ya + 12);
          i = 0;
          za = 0;
          if (month < 12) {
            ++month;
          } else {
            month = 1;
            ++year;
          }
          ie = nbl * month_days(month, year);
        }
      }
    } else {
      cv.text(4, h - 14, 10, "Lebensj.");
      cv.text(4, h + 2, 10, "Datum");
      const CalendarDate birth = calendar_date(radix.jd_ut, cal);
      double xm = 0.0;
      int i = 0;
      for (double x = kAxisStart + 1.0; x <= kAxisEnd + nbl; x += 1.0) {
        ++i;
        const int age = static_cast<int>(std::floor(opt.from_years + (x - kAxisStart) / nbl + kEps));
        const std::string a = std::to_string(age);
        const auto mark = [&]() {
          // mark_jahr, the birthday of that year of life
          if (x - 16.0 - xm > 64.0 || xm == 0.0) {
            cv.line(x, ya + 15, x, ya + 23);
            const int y = birth.year + age;
            std::snprintf(buf, sizeof(buf), "%d.%d.%d", birth.day, birth.month, y > 0 ? y : 1 - y);
            cv.text(x - 16 - 12, ya + 32, 10, std::string(buf) + (y > 0 ? "" : " vC"));
            xm = x - 16.0;
          }
        };
        if ((i % (5 * nbl) == 0) || i == 1) {
          cv.line(x, ya, x, ya + 8);
          cv.text(x - 8 + (nbl == 3 ? 15.0 : 0.0), ya + 16, 10, a);
          mark();
        } else if ((nbl == 48 || nbl == 96) && i % nbl == 0) {
          cv.line(x, ya, x, ya + 8);
          cv.text(x - 8, ya + 16, 9, a);
          mark();
        } else if ((nbl == 48 || nbl == 96) && (i * 12) % nbl == 0) {
          cv.line(x, ya, x, ya + ((i * 2) % nbl == 0 ? 6.0 : 4.0));
        } else if (i % nbl == 0) {
          cv.line(x, ya, x, ya + 4);
        }
      }
    }
    // skalh_gitter, his fixed grid when no hit lines are drawn
    if (opt.grid && !opt.hit_lines) {
      const int s = dates ? (nbl == 4 ? 20 : 16) : kGridLife;
      for (int x = 80; x <= 560; x += s) {
        cv.line(x, kBandTop, x, kBandBottom, kGridGreen, 1.0, Primitive::Style::kDotted);
      }
    }
  }

  // the running bodies, his nd1& to be& with the auswahl_flag
  const bool arc_mode = opt.kind == LinearKind::kSunArc || opt.kind == LinearKind::kMoonArc;
  const int light = opt.kind == LinearKind::kMoonArc ? body::kMoon : body::kSun;
  std::vector<int> movers;
  for (const RadixLine& l : lines) {
    const int slot = l.slot;
    if (slot >= 100 || slot < opt.first_slot) {
      continue;
    }
    if (!opt.chosen.empty() && std::find(opt.chosen.begin(), opt.chosen.end(), slot) == opt.chosen.end()) {
      continue;
    }
    if (slot == body::kMoon && !opt.moon && !arc_mode) {
      continue;
    }
    // the angles run only with the arcs, the moon arc leaves them out
    if (slot == body::kAscendant || slot == body::kMc) {
      if (!arc_mode || opt.kind == LinearKind::kMoonArc) {
        continue;
      }
    }
    // IF NOT (tras! OR prog! OR mund!) && p& = nk&(4), the part of
    // fortune moves only with the arcs
    if (slot == body::kFortune && !arc_mode) {
      continue;
    }
    if (slot == body::kNodeDesc && !arc_mode) {
      continue;
    }
    movers.push_back(slot);
  }
  // one column per pixel of the clipped axis, one sky per column
  const int x_from = static_cast<int>(kClipLeft);
  const int x_to = static_cast<int>(kClipRight);
  const auto jd_at = [&](int x) {
    const double step = (x - kAxisStart) / nbl;
    return dates ? opt.jd_from_ut + step - ta1 : radix.jd_ut + (opt.from_years + step) * tja;
  };
  std::vector<std::vector<double>> fold(movers.size(), std::vector<double>(static_cast<std::size_t>(x_to - x_from + 1), -1.0));
  std::vector<std::vector<double>> raw(movers.size(), std::vector<double>(static_cast<std::size_t>(x_to - x_from + 1), -1.0));
  for (int x = x_from; x <= x_to; ++x) {
    if (opt.progress && !opt.progress(static_cast<double>(x - x_from) / (x_to - x_from))) {
      break;
    }
    const double jd = jd_at(x);
    const std::size_t k = static_cast<std::size_t>(x - x_from);
    if (arc_mode) {
      // a18so, the progressed light against its radix place
      const BodyLongitude bl = body_longitude(radix.jd_ut + (jd - radix.jd_ut) / tja, light, ctx);
      if (!bl.valid) {
        continue;
      }
      const double arc = bl.el - radix.b[static_cast<std::size_t>(light)].el;
      for (std::size_t i = 0; i < movers.size(); ++i) {
        raw[i][k] = norm_rad(radix.b[static_cast<std::size_t>(movers[i])].el + arc);
        fold[i][k] = fold_deg(raw[i][k]);
      }
    } else {
      const double jd_run = opt.kind == LinearKind::kSecondary ? radix.jd_ut + (jd - radix.jd_ut) / tja : jd;
      const Chart sky = sky_chart(jd_run, ctx);
      if (!sky.ok) {
        continue;
      }
      for (std::size_t i = 0; i < movers.size(); ++i) {
        const BodyState& b = sky.b[static_cast<std::size_t>(movers[i])];
        if (b.present && b.valid) {
          raw[i][k] = norm_rad(b.el);
          fold[i][k] = fold_deg(b.el);
        }
      }
    }
  }
  std::vector<std::pair<double, double>> stamps;
  for (std::size_t i = 0; i < movers.size(); ++i) {
    const int slot = movers[i];
    const Rgb col = line_color(slot);
    double path = kStampPath;
    for (int x = x_from + 1; x <= x_to; ++x) {
      const std::size_t k = static_cast<std::size_t>(x - x_from);
      const double f1 = fold[i][k - 1];
      const double f2 = fold[i][k];
      if (f1 < 0.0 || f2 < 0.0) {
        continue;
      }
      const double y1 = inv(kBandBottom - f1);
      const double y2 = inv(kBandBottom - f2);
      // IF ABS(w2 - w1) < 180, else the curve leaves one band edge and
      // enters at the other
      if (std::abs(f2 - f1) < kDegPerCircle / 2.0) {
        cv.line(x - 1, y1, x, y2, col, opt.line_width);
      } else {
        const double up_edge = inv(kBandTop);
        const double down_edge = inv(kBandBottom);
        if (f1 > f2) {
          cv.line(x - 1, y1, x - 0.5, up_edge, col, opt.line_width);
          cv.line(x - 0.5, down_edge, x, y2, col, opt.line_width);
        } else {
          cv.line(x - 1, y1, x - 0.5, down_edge, col, opt.line_width);
          cv.line(x - 0.5, up_edge, x, y2, col, opt.line_width);
        }
      }
      path += std::hypot(1.0, y2 - y1);
      if (path > kStampPath) {
        const bool clear = std::none_of(stamps.begin(), stamps.end(), [&](const std::pair<double, double>& s) {
          return std::abs(s.first - x) < kStampClearX && std::abs(s.second - y2) < kStampClearY;
        });
        // IF yy&(p&) + g < 417 && yy&(p&) + g > 63
        if (clear && y2 - 2 < 417.0 && y2 - 2 > 63.0) {
          cv.glyph(x - 2, y2 - 2, 9, body_glyph(slot), col);
          // IF FRAC(p) > 0.1 && FRAC(p) < 0.9, the sign beside the body
          const double p = raw[i][k] / kSignRad;
          if (opt.signs && p - std::floor(p) > 0.1 && p - std::floor(p) < 0.9) {
            cv.glyph(x + 11, y2 - 2, 9, sign_glyph(static_cast<int>(p) % kSignCount), kInkColor);
          }
          stamps.emplace_back(x, y2);
          path = 0.0;
        }
      }
    }
  }

  // the hits, the aspect sprite on the radix line and his hit line down
  // to the axis. A mundane hit sits on the first body at the moment, its
  // sprite three pixels up and left like a181
  for (const LinearHit& h : opt.hits) {
    const double x = x_of_jd(h.jd_ut);
    if (x < kClipLeft || x > kClipRight) {
      continue;
    }
    double y = -1.0;
    if (mundane) {
      y = to_y(h.lon);
    } else if (h.cusp > 0 && radix.houses.ok) {
      y = to_y(radix.houses.cusp[static_cast<std::size_t>(h.cusp)]);
    } else if (h.radix > 0 && h.radix < body::kSlotCount && !body::cardinal(h.radix)) {
      const BodyState& b = radix.b[static_cast<std::size_t>(h.radix)];
      if (b.present && b.valid) {
        y = to_y(b.el);
      }
    }
    if (y <= kBandTop || y >= kBandBottom) {
      continue;
    }
    // IF w4d < 360 && asps&(s&) > 0
    if (opt.base_angle_deg < kDegPerCircle) {
      const double wrad = norm_rad(h.angle_deg * kDegToRad);
      const char* g = aspect_glyph(aspect_symbol(wrad < kEps ? kTwoPi : wrad, 16));
      if (*g != '\0') {
        const double shift = mundane ? 3.0 : 0.0;
        cv.glyph(x - shift, y - shift, 9, g, kInkColor);
      }
    }
    if (opt.hit_lines) {
      // his screen draws the red lines solid, the others dotted
      Rgb c = hit_line_color(h.running);
      if (mundane) {
        c = mundane_heavy(h.running) && mundane_heavy(h.radix) ? kMarkRed : kGridGreen;
      }
      cv.line(x, y, x, kBandBottom, c, 1.0, c == kMarkRed ? Primitive::Style::kSolid : Primitive::Style::kDotted);
    }
  }
  return cv.dl;
}

}  // namespace horcom
