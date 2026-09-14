// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/linear.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <vector>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/calendar.hpp"

namespace horcom {

namespace {

// his sheet, the band of 360 pixels folds the whole circle
constexpr double kBandTop = 60.0;
constexpr double kBandBottom = 420.0;
constexpr double kFrameLeft = 18.0;
constexpr double kFrameRight = 622.0;
constexpr double kLineLeft = 54.0;
constexpr double kLineRight = 622.0;
constexpr double kCurveLeft = 75.0;
constexpr double kCurveRight = 590.0;
constexpr double kGlyphColLeft = 42.0;
constexpr double kGlyphColRight = 578.0;
constexpr double kSignCol = 596.0;
// a glyph is stamped along a curve after this much drawn path and only
// clear of earlier stamps, his sl gate and the lins linw memory
constexpr double kStampPath = 80.0;
constexpr double kStampClearX = 30.0;
constexpr double kStampClearY = 20.0;
// a sample step that jumps more than this many folded degrees is a
// wrap or an aliasing fast mover and is not connected
constexpr double kJumpLimit = 120.0;

constexpr Rgb kNeutral = 0x202020;
constexpr Rgb kDim = 0x606060;

struct Pt {
  double x;
  double y;
};

// ported from HORCOM a18_line_col, the three colour classes
Rgb line_color(int slot) {
  switch (slot) {
    case 1: case 2: case 3:
      return 0x00A000;
    case 5: case 7: case 8: case 9: case 10:
      return 0xFF0000;
    case 4: case 6: case 11: case 12:
      return 0x0000FF;
    default:
      return kNeutral;
  }
}

}  // namespace

// ported from HORCOM a18_lin and a180trpr_lin
DisplayList build_linear_graph(const Chart& radix, const LinearOptions& opt, const SearchContext& ctx) {
  DisplayList dl;
  dl.height = 460.0;
  const double w = 360.0 / opt.base_angle_deg;  // his Teiler
  const bool down = opt.downward;

  const auto fold_deg = [&](double rad) { return norm_rad(w * rad) * kRadToDeg; };
  const auto fold_y = [&](double f) { return down ? kBandTop + f : kBandBottom - f; };
  const auto to_y = [&](double rad) { return fold_y(fold_deg(rad)); };
  const auto to_x = [&](double jd) {
    return kCurveLeft + (jd - opt.jd_from_ut) / (opt.jd_to_ut - opt.jd_from_ut) * (kCurveRight - kCurveLeft);
  };
  const auto line = [&](double x1, double y1, double x2, double y2, Rgb c, double width = 1.0,
                        Primitive::Style st = Primitive::Style::kSolid) {
    Primitive p;
    p.kind = Primitive::Kind::kLine;
    p.x1 = x1; p.y1 = y1; p.x2 = x2; p.y2 = y2;
    p.color = c; p.width = width; p.style = st;
    dl.items.push_back(p);
  };
  const auto text = [&](double x, double y, double size, const std::string& t, Rgb c) {
    Primitive p;
    p.kind = Primitive::Kind::kText;
    p.x1 = x; p.y1 = y; p.size = size; p.text = t; p.color = c;
    dl.items.push_back(p);
  };
  const auto glyph = [&](double x, double y, double size, const std::string& t, Rgb c) {
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = x; p.y1 = y; p.size = size; p.text = t; p.color = c;
    dl.items.push_back(p);
  };

  // frame, header rule and titles
  line(1, 1, 639, 1, kNeutral);
  line(639, 1, 639, 458, kNeutral);
  line(639, 458, 1, 458, kNeutral);
  line(1, 458, 1, 1, kNeutral);
  line(2, 36, 638, 36, kNeutral);
  line(2, 37, 638, 37, kNeutral);
  line(kFrameLeft, kBandTop, kFrameRight, kBandTop, kNeutral);
  line(kFrameRight, kBandTop, kFrameRight, kBandBottom, kNeutral);
  line(kFrameRight, kBandBottom, kFrameLeft, kBandBottom, kNeutral);
  line(kFrameLeft, kBandBottom, kFrameLeft, kBandTop, kNeutral);
  const char* title = "";
  switch (opt.kind) {
    case LinearKind::kTransits: title = "TRANSITE"; break;
    case LinearKind::kSecondary: title = "SEKUND\xC3\x84R-DIREKTION"; break;
    //RR di$ der Bogenwahl
    case LinearKind::kSunArc: title = "SONNEN-BOGEN-DIR."; break;
    case LinearKind::kMoonArc: title = "MOND-BOGEN-DIR."; break;
  }
  text(320, 22, 12, std::string("LINEAR-GRAPHIK  ") + title, kNeutral);
  char buf[64];
  //RR GRUNDWINKEL ( GRAD )
  std::snprintf(buf, sizeof(buf), "GRUNDWINKEL %g GRAD", opt.base_angle_deg);
  text(60, 50, 9, buf, kDim);
  const CalendarDate d0 = calendar_date(opt.jd_from_ut, ctx.settings.calendar);
  const CalendarDate d1 = calendar_date(opt.jd_to_ut, ctx.settings.calendar);
  std::snprintf(buf, sizeof(buf), "%02d.%02d.%d - %02d.%02d.%d", d0.day, d0.month, d0.year, d1.day, d1.month, d1.year);
  text(520, 50, 9, buf, kDim);

  // dashed sign boundaries, only meaningful when nothing folds
  if (opt.sign_lines && opt.base_angle_deg >= 360.0 - kEps) {
    for (int i = 0; i < 12; ++i) {
      const double y = fold_y(i * 30.0);
      line(kLineLeft, y, kLineRight, y, 0x0000FF, 1.0, Primitive::Style::kDashed);
      glyph(kSignCol + 16, down ? y + 15 : y - 15, 10, sign_glyph(down ? i : (i + 11) % 12), kDim);
    }
  }

  // the year scale under the band
  {
    const double span_years = (opt.jd_to_ut - opt.jd_from_ut) / radix.ta.tropical_year_days;
    const int step = std::max(1, static_cast<int>(span_years / 16.0 + 0.5));
    for (int y = d0.year + 1; y <= d1.year; ++y) {
      const double jd = julian_day({1, 1, y, 0, 0.0}, ctx.settings.calendar);
      const double x = to_x(jd);
      if (x < kCurveLeft || x > kCurveRight) {
        continue;
      }
      line(x, kBandBottom, x, kBandBottom + 6, kDim);
      if ((y - d0.year - 1) % step == 0) {
        std::snprintf(buf, sizeof(buf), "%d", y);
        text(x, kBandBottom + 16, 8, buf, kDim);
      }
    }
  }

  // the radix factor lines with their glyph columns
  std::vector<int> factors;
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    if (slot >= 15 && slot <= 18) {
      continue;
    }
    const BodyState& b = radix.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid) {
      continue;
    }
    factors.push_back(slot);
    const double y = to_y(b.el);
    line(kLineLeft, y, kLineRight, y, line_color(slot));
    //RR das Symbol nicht beim absteigenden Knoten
    if (slot != body::kNodeDesc) {
      glyph(kGlyphColLeft, y - 5, 9, body_glyph(slot), line_color(slot));
      glyph(kGlyphColRight, y - 5, 9, body_glyph(slot), line_color(slot));
      glyph(kSignCol, y - 5, 9, sign_glyph(static_cast<int>(norm_rad(b.el) / (kPi / 6.0)) % 12), kDim);
    }
  }
  if (opt.with_houses && radix.houses.ok) {
    //RR die Zwischenhäuser H2 H3 H5 H6 mit ihren Gegenspitzen
    for (int c : {2, 3, 5, 6, 8, 9, 11, 12}) {
      const double y = to_y(radix.houses.cusp[static_cast<std::size_t>(c)]);
      line(kLineLeft, y, kLineRight, y, kDim);
      std::snprintf(buf, sizeof(buf), "H%d", c);
      text(kGlyphColLeft, y + 3, 8, buf, kDim);
    }
  }

  // running longitudes per sample column
  const int n = std::max(2, opt.samples);
  const double tja = radix.ta.tropical_year_days;
  const auto compressed = [&](double jd) { return radix.jd_ut + (jd - radix.jd_ut) / tja; };
  const bool arc_mode = opt.kind == LinearKind::kSunArc || opt.kind == LinearKind::kMoonArc;
  const int light = opt.kind == LinearKind::kMoonArc ? body::kMoon : body::kSun;

  std::vector<int> movers;
  if (arc_mode) {
    movers = factors;  // the whole radix shifts by the arc
  } else {
    for (int slot : factors) {
      if (slot >= 1 && slot != body::kNodeDesc && slot != body::kAscendant && slot != body::kMc) {
        movers.push_back(slot);
      }
    }
  }

  // one folded position table per sample, arcs need only the light
  std::vector<std::vector<double>> f(movers.size(), std::vector<double>(static_cast<std::size_t>(n), -1.0));
  for (int k = 0; k < n; ++k) {
    const double jd = opt.jd_from_ut + (opt.jd_to_ut - opt.jd_from_ut) * k / (n - 1);
    if (arc_mode) {
      const BodyLongitude bl = body_longitude(compressed(jd), light, ctx);
      if (!bl.valid) {
        continue;
      }
      const double arc = bl.el - radix.b[static_cast<std::size_t>(light)].el;
      for (std::size_t i = 0; i < movers.size(); ++i) {
        f[i][static_cast<std::size_t>(k)] = fold_deg(radix.b[static_cast<std::size_t>(movers[i])].el + arc);
      }
    } else {
      const double jd_run = opt.kind == LinearKind::kSecondary ? compressed(jd) : jd;
      for (std::size_t i = 0; i < movers.size(); ++i) {
        const BodyLongitude bl = body_longitude(jd_run, movers[i], ctx);
        if (bl.valid) {
          f[i][static_cast<std::size_t>(k)] = fold_deg(bl.el);
        }
      }
    }
  }

  // draw the curves, split at the fold wrap like his edge handling
  std::vector<Pt> stamps;
  for (std::size_t i = 0; i < movers.size(); ++i) {
    const int slot = movers[i];
    const Rgb col = line_color(slot);
    double path = kStampPath;  // the first stamp comes early
    for (int k = 1; k < n; ++k) {
      const double f1 = f[i][static_cast<std::size_t>(k - 1)];
      const double f2 = f[i][static_cast<std::size_t>(k)];
      if (f1 < 0.0 || f2 < 0.0) {
        continue;
      }
      const double x1 = to_x(opt.jd_from_ut + (opt.jd_to_ut - opt.jd_from_ut) * (k - 1) / (n - 1));
      const double x2 = to_x(opt.jd_from_ut + (opt.jd_to_ut - opt.jd_from_ut) * k / (n - 1));
      double d = f2 - f1;
      d -= 360.0 * std::round(d / 360.0);
      if (std::abs(d) > kJumpLimit) {
        continue;
      }
      const double target = f1 + d;
      if (target >= 0.0 && target <= 360.0) {
        line(x1, fold_y(f1), x2, fold_y(f2), col);
      } else {
        // the crossing point on the band edge, his x0 interpolation
        const double edge = target < 0.0 ? 0.0 : 360.0;
        const double t = (edge - f1) / d;
        const double xc = x1 + (x2 - x1) * t;
        line(x1, fold_y(f1), xc, fold_y(edge), col);
        line(xc, fold_y(edge == 0.0 ? 360.0 : 0.0), x2, fold_y(f2), col);
      }
      path += std::hypot(x2 - x1, fold_y(f2) - fold_y(f1));
      if (path >= kStampPath) {
        const Pt at{x2, fold_y(f2)};
        bool clear = true;
        for (const Pt& s : stamps) {
          if (std::abs(s.x - at.x) < kStampClearX && std::abs(s.y - at.y) < kStampClearY) {
            clear = false;
            break;
          }
        }
        if (clear && at.y > kBandTop + 3 && at.y < kBandBottom - 3) {
          glyph(at.x - 4, at.y - 4, 9, body_glyph(slot), col);
          stamps.push_back(at);
          path = 0.0;
        }
      }
    }
  }
  return dl;
}

}  // namespace horcom
