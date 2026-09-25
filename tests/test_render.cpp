// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <algorithm>
#include <cmath>

#include "doctest.h"
#include "horcom/chart/aspects.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/chart/degree_list.hpp"
#include "horcom/data/konsta.hpp"
#include "horcom/render/aspektarium.hpp"
#include "horcom/render/grad_sheet.hpp"
#include "horcom/render/linear.hpp"
#include "horcom/render/midpoint_trees.hpp"
#include "horcom/render/pair_sheet.hpp"
#include "horcom/render/rhythm_panel.hpp"
#include "horcom/render/svg.hpp"
#include "horcom/render/wheel.hpp"

using namespace horcom;

namespace {

VsopTables& vsop() {
  static VsopTables t = VsopTables::load(HORCOM_TEST_DATA_DIR "/planets.ndx", HORCOM_TEST_DATA_DIR "/planets.dat");
  return t;
}

Ephemerides& eph() {
  static Ephemerides e{HORCOM_TEST_DATA_DIR "/eph"};
  return e;
}

Chart sample_chart() {
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  return compute_chart(in, {}, vsop(), eph());
}

}  // namespace

TEST_CASE("the transit wheel rides the running sky outside the signs") {
  const Chart radix = sample_chart();
  REQUIRE(radix.ok);
  ChartInput in;
  in.date_ut = {13, 9, 2026, 12, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  const Chart transit = compute_chart(in, {}, vsop(), eph());
  REQUIRE(transit.ok);
  const AspectResult a = scan_aspects(radix, {}, {});
  WheelOptions opt;
  opt.center_label = "TRANSIT=>13.09.2026";
  const DisplayList dl = build_transit_wheel(radix, transit, {}, a, opt);
  // two suns, the radix one on the glyph ring and the transit one at 212,
  // both at the smaller a20 scale
  int suns = 0;
  bool outer_sun = false;
  bool label = false;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kGlyph && p.text.rfind("☉", 0) == 0) {
      ++suns;
      const double r = std::hypot(p.x1 - kWheelCenterX, p.y1 - kWheelCenterY);
      if (r > kTransitWheelScale * 190.0) {
        outer_sun = true;
        CHECK(r < kTransitWheelScale * 225.0);
      } else {
        // the radix sun stays on the inner glyph ring, declump aside
        CHECK(r > kTransitWheelScale * (kGlyphRingRadius - 20.0));
        CHECK(r < kTransitWheelScale * (kGlyphRingRadius + 12.0));
      }
    }
    if (p.kind == Primitive::Kind::kText && p.text == "TRANSIT=>13.09.2026") {
      label = true;
    }
  }
  CHECK(suns == 2);
  CHECK(outer_sun);
  CHECK(label);
}

TEST_CASE("the marked running factors of plan_col! stay red on a blue ring") {
  const Chart radix = sample_chart();
  REQUIRE(radix.ok);
  ChartInput in;
  in.date_ut = {13, 9, 2026, 12, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  const Chart transit = compute_chart(in, {}, vsop(), eph());
  REQUIRE(transit.ok);
  const AspectResult a = scan_aspects(radix, {}, {});
  WheelOptions opt;
  // hard& = 3 paints the ring blue, the running sun is marked
  opt.outer_color = 3;
  opt.outer_marked = {body::kSun};
  const DisplayList dl = build_transit_wheel(radix, transit, {}, a, opt);
  bool red_sun = false;
  bool blue_moon = false;
  for (const Primitive& p : dl.items) {
    if (p.kind != Primitive::Kind::kGlyph) {
      continue;
    }
    const double r = std::hypot(p.x1 - kWheelCenterX, p.y1 - kWheelCenterY);
    if (r < kTransitWheelScale * 190.0) {
      continue;
    }
    if (p.text.rfind("☉", 0) == 0) {
      red_sun = p.color == kMarkRed;
    }
    if (p.text.rfind("☽", 0) == 0) {
      blue_moon = p.color == 0x0000FF;
    }
  }
  CHECK(red_sun);
  CHECK(blue_moon);
}

TEST_CASE("the double wheel carries the partner outside at the a12 scale") {
  const Chart inner = sample_chart();
  REQUIRE(inner.ok);
  ChartInput in;
  in.date_ut = {1, 6, 1990, 12, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  const Chart outer = compute_chart(in, {}, vsop(), eph());
  REQUIRE(outer.ok);
  const AspectResult a = scan_aspects(inner, {}, {});
  const DisplayList dl = build_double_wheel(inner, outer, {}, a);
  int suns = 0;
  bool outer_sun = false;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kGlyph && p.text.rfind("☉", 0) == 0) {
      ++suns;
      const double r = std::hypot(p.x1 - kWheelCenterX, p.y1 - kWheelCenterY);
      //RR km = 0.8, a12 shrinks the wheel so the second ring keeps
      // inside the sheet
      if (r > kDoubleWheelScale * 185.0) {
        outer_sun = true;
        CHECK(r < kDoubleWheelScale * 215.0);
      }
    }
  }
  CHECK(suns == 2);
  CHECK(outer_sun);
}

TEST_CASE("the wheel puts the ascendant on the left") {
  const Chart c = sample_chart();
  REQUIRE(c.ok);
  const AspectResult a = scan_aspects(c, {}, {});
  const DisplayList dl = build_wheel(c, {}, a);
  // the AC axis line must reach the label radius on the left side of the
  // centre 430, 224
  bool found = false;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kText && p.text.rfind("AC", 0) == 0) {
      found = true;
      CHECK(p.x1 < kWheelCenterX - 0.9 * 190.0);
      CHECK(std::abs(p.y1 - kWheelCenterY) < 15.0);
    }
  }
  CHECK(found);
}

TEST_CASE("the equal systems name their cusp axes and draw the true angles") {
  ChartInput in;
  in.date_ut = {13, 10, 1992, 3, 0.0};
  in.lon_deg_east = 11.3244;
  in.lat_deg = 48.1742;
  const auto find_text = [](const DisplayList& dl, const std::string& tag) -> const Primitive* {
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kText && p.text == tag) {
        return &p;
      }
    }
    return nullptr;
  };
  ChartSettings vehlow;
  vehlow.houses = HouseSystem::kEqualVehlow;
  const Chart c = compute_chart(in, vehlow, vsop(), eph());
  REQUIRE(c.ok);
  const DisplayList dl = build_wheel(c, vehlow, scan_aspects(c, vehlow, {}));
  //RR aeqh, H1 H4 H7 H10 in blue for Vehlow
  for (const char* tag : {"H1", "H4", "H7", "H10"}) {
    CAPTURE(tag);
    const Primitive* p = find_text(dl, tag);
    REQUIRE(p != nullptr);
    CHECK(p->color == 0x0000FF);
  }
  // the true AC and MC stand in black on their own axes
  const Primitive* ac = find_text(dl, "AC");
  const Primitive* mc = find_text(dl, "MC");
  const Primitive* h1 = find_text(dl, "H1");
  REQUIRE(ac != nullptr);
  REQUIRE(mc != nullptr);
  CHECK(ac->color == 0x000000);
  CHECK(mc->color == 0x000000);
  // the first Vehlow cusp sits fifteen degrees before the AC
  const double a_ac = std::atan2(-(ac->y1 - kWheelCenterY), ac->x1 - kWheelCenterX);
  const double a_h1 = std::atan2(-(h1->y1 - kWheelCenterY), h1->x1 - kWheelCenterX);
  double d = std::abs(a_ac - a_h1) * kRadToDeg;
  if (d > 180.0) {
    d = 360.0 - d;
  }
  CHECK(d == doctest::Approx(15.0).epsilon(0.02));
  // a quadrant system keeps the plain angle names
  const Chart p = compute_chart(in, {}, vsop(), eph());
  const DisplayList pl = build_wheel(p, {}, scan_aspects(p, {}, {}));
  CHECK(find_text(pl, "H1") == nullptr);
  CHECK(find_text(pl, "IC") != nullptr);
}

TEST_CASE("the display list carries the twelve sectors and three rings") {
  const Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  const DisplayList dl = build_wheel(c, {}, a);
  int sectors = 0;
  int circles = 0;
  int glyphs = 0;
  int conjunctions = 0;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kSector) {
      ++sectors;
    }
    // the conjunction rings of asp1 have radius 3, the wheel rings far more
    if (p.kind == Primitive::Kind::kCircle && p.r1 > 10.0) {
      ++circles;
    }
    if (p.kind == Primitive::Kind::kCircle && p.r1 == doctest::Approx(3.0) && p.color == 0xFF0000) {
      ++conjunctions;
    }
    if (p.kind == Primitive::Kind::kGlyph) {
      ++glyphs;
    }
  }
  CHECK(sectors == 12);
  CHECK(circles == 3);
  CHECK(conjunctions == a.zh[1]);
  // twelve sign glyphs plus at least Sun through Pluto and the nodes
  CHECK(glyphs >= 12 + 12);
}

TEST_CASE("every glyph stays inside the canvas") {
  const Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  const DisplayList dl = build_wheel(c, {}, a);
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kGlyph || p.kind == Primitive::Kind::kText) {
      CAPTURE(p.text);
      CHECK(p.x1 > 0.0);
      CHECK(p.x1 < 640.0);
      CHECK(p.y1 > 0.0);
      CHECK(p.y1 < 480.0);
    }
  }
}

TEST_CASE("sign boxes, inverted rulers and stacked axis numbers") {
  const Chart c = sample_chart();
  REQUIRE(c.ok);
  const AspectResult a = scan_aspects(c, {}, {});
  WheelOptions opt;
  opt.ruler_slot = body::kMars;
  opt.ruler_slot2 = body::kVenus;
  const DisplayList dl = build_wheel(c, {}, a, opt);
  int boxes = 0;
  int patches = 0;
  int white_glyphs = 0;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kRect && p.fill == 0xFFFFFF) {
      ++boxes;
    }
    // his SRCINVERT square, the whole sprite cell
    if (p.kind == Primitive::Kind::kRect && p.fill == 0x000000) {
      CHECK(p.r1 == doctest::Approx(p.r2));
      ++patches;
    }
    if (p.kind == Primitive::Kind::kGlyph && p.color == 0xFFFFFF) {
      ++white_glyphs;
    }
  }
  // every sign sprite keeps its white SRCCOPY ground
  CHECK(boxes == 12);
  // both rulers and the true node pair wear the dark patch
  CHECK(patches == 4);
  CHECK(white_glyphs == 4);
  // the SVG backend knows the box
  CHECK(to_svg(dl).find("<rect") != std::string::npos);
  // a sprite resolver turns glyphs into embedded images, without one
  // the font text stays
  const std::string with_sprites =
      to_svg(dl, [](const std::string&, Rgb) { return std::string("data:image/png;base64,AA=="); });
  CHECK(with_sprites.find("<image") != std::string::npos);
  CHECK(to_svg(dl).find("<image") == std::string::npos);
  // MC carries its degree stacked below the tag like habes
  bool stacked = false;
  for (const Primitive& t : dl.items) {
    if (t.kind != Primitive::Kind::kText || t.text != "MC") {
      continue;
    }
    for (const Primitive& n : dl.items) {
      if (n.kind == Primitive::Kind::kText && n.x1 == t.x1 && n.y1 > t.y1 &&
          n.y1 - t.y1 < 14.0 && !n.text.empty() &&
          n.text.find_first_not_of("0123456789") == std::string::npos) {
        stacked = true;
      }
    }
  }
  CHECK(stacked);
  // mean nodes draw plain, only the rulers stay inverted
  WheelOptions mean = opt;
  mean.invert_nodes = false;
  const DisplayList dm = build_wheel(c, {}, a, mean);
  patches = 0;
  for (const Primitive& p : dm.items) {
    // his SRCINVERT square, the whole sprite cell
    if (p.kind == Primitive::Kind::kRect && p.fill == 0x000000) {
      CHECK(p.r1 == doctest::Approx(p.r2));
      ++patches;
    }
  }
  CHECK(patches == 2);
}

TEST_CASE("crowded bodies separate on the glyph ring") {
  // a stellium chart, 1962-02-05 packed seven bodies into Aquarius
  ChartInput in;
  in.date_ut = {5, 2, 1962, 0, 0.0};
  const Chart c = compute_chart(in, {}, vsop(), eph());
  REQUIRE(c.ok);
  const AspectResult a = scan_aspects(c, {}, {});
  const DisplayList dl = build_wheel(c, {}, a);
  // no two body glyphs may sit closer than a glyph height
  std::vector<const Primitive*> glyphs;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kGlyph && p.size > 10.0) {
      glyphs.push_back(&p);
    }
  }
  int too_close = 0;
  for (std::size_t i = 0; i < glyphs.size(); ++i) {
    for (std::size_t j = i + 1; j < glyphs.size(); ++j) {
      const double dx = glyphs[i]->x1 - glyphs[j]->x1;
      const double dy = glyphs[i]->y1 - glyphs[j]->y1;
      if (std::sqrt(dx * dx + dy * dy) < 8.0) {
        ++too_close;
      }
    }
  }
  // without de clumping the raw ring positions of this stellium collide
  // heavily, the one pass plentz of the original resolves almost all of
  // it and may leave a single touching pair, exactly like the program
  int raw_close = 0;
  std::vector<std::pair<double, double>> raw;
  for (int slot = 1; slot <= 12; ++slot) {
    const BodyState& b = c.b[static_cast<std::size_t>(slot)];
    if (b.present && b.valid) {
      const double w = norm_rad(b.el + kPi - c.houses.angles.ac);
      raw.emplace_back(kWheelCenterX + kWheelScale * kGlyphRingRadius * std::cos(-w), kWheelCenterY + kWheelScale * kGlyphRingRadius * std::sin(-w));
    }
  }
  for (std::size_t i = 0; i < raw.size(); ++i) {
    for (std::size_t j = i + 1; j < raw.size(); ++j) {
      const double dx = raw[i].first - raw[j].first;
      const double dy = raw[i].second - raw[j].second;
      if (std::sqrt(dx * dx + dy * dy) < 8.0) {
        ++raw_close;
      }
    }
  }
  CHECK(too_close <= 1);
  CHECK(raw_close >= 3);
  CHECK(too_close < raw_close);
}

TEST_CASE("the SVG document is well formed and complete") {
  const Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  const std::string svg = to_svg(build_wheel(c, {}, a));
  CHECK(svg.find("<svg") == 0);
  CHECK(svg.find("</svg>") != std::string::npos);
  CHECK(svg.find("viewBox=\"0 0 640.00 480.00\"") != std::string::npos);
  // sectors, rings and the Sun glyph must appear
  CHECK(svg.find("<path") != std::string::npos);
  CHECK(svg.find("<circle") != std::string::npos);
  CHECK(svg.find("☉") != std::string::npos);
}

TEST_CASE("the linear graph folds the radix onto horizontal lines") {
  const Chart radix = sample_chart();
  REQUIRE(radix.ok);
  SearchContext ctx;
  ctx.base.lon_deg_east = 11.3244;
  ctx.base.lat_deg = 48.1742;
  ctx.vsop = &vsop();
  ctx.eph = &eph();
  LinearOptions opt;
  opt.kind = LinearKind::kSunArc;
  opt.base_angle_deg = 90.0;
  opt.from_years = 0.0;
  opt.to_years = 10.0;
  const DisplayList dl = build_linear_graph(radix, opt, ctx);
  // the sun line sits at the fourfold folded longitude under the band top
  const double folded = norm_rad(4.0 * radix.b[body::kSun].el) * kRadToDeg;
  const double want_y = 420.0 - folded;
  bool sun_line = false;
  int curve_segments = 0;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kLine && std::abs(p.y1 - want_y) < 0.5 && std::abs(p.y2 - want_y) < 0.5 && p.x1 <= 54.0 + 0.5 && p.color == 0x00A000) {
      sun_line = true;
    }
    if (p.kind == Primitive::Kind::kLine && p.x1 >= 75.0 && p.x2 <= 575.0 && std::abs(p.y1 - p.y2) > 0.01) {
      ++curve_segments;
    }
  }
  CHECK(sun_line);
  // the sun arc moves near one degree per year, folded near four, the
  // sun curve must drop about forty pixels over the ten years
  CHECK(curve_segments > 50);

  // the Ebertin direction flips the ordinate
  opt.downward = true;
  const DisplayList down = build_linear_graph(radix, opt, ctx);
  bool flipped = false;
  for (const Primitive& p : down.items) {
    if (p.kind == Primitive::Kind::kLine && std::abs(p.y1 - (60.0 + folded)) < 0.5 && std::abs(p.y2 - p.y1) < 0.5 && p.x1 <= 54.0 + 0.5 && p.color == 0x00A000) {
      flipped = true;
    }
  }
  CHECK(flipped);
}

TEST_CASE("the linear curve wraps at the band edge without a jump line") {
  const Chart radix = sample_chart();
  SearchContext ctx;
  ctx.base.lon_deg_east = 11.3244;
  ctx.base.lat_deg = 48.1742;
  ctx.vsop = &vsop();
  ctx.eph = &eph();
  LinearOptions opt;
  opt.kind = LinearKind::kSecondary;
  opt.base_angle_deg = 30.0;
  opt.from_years = 0.0;
  opt.to_years = 40.0;
  const DisplayList dl = build_linear_graph(radix, opt, ctx);
  // with the twelvefold fold the progressed sun wraps several times,
  // every drawn segment must stay well shorter than the band height
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kLine && p.x1 >= 75.0 && p.x2 <= 575.0 && p.x2 > p.x1) {
      CHECK(std::abs(p.y2 - p.y1) < 200.0);
    }
  }
}

TEST_CASE("the linear axis runs in his pixel steps") {
  LinearOptions opt;
  opt.kind = LinearKind::kTransits;
  //RR 1 Monat 16, 4 Monate 4, 16 Monate 1 Pixel pro Tag
  opt.jd_from_ut = 2451545.0;
  opt.jd_to_ut = opt.jd_from_ut + 31.0;
  CHECK(linear_pixels_per_step(opt) == 16);
  opt.jd_to_ut = opt.jd_from_ut + 125.0;
  CHECK(linear_pixels_per_step(opt) == 4);
  opt.jd_to_ut = opt.jd_from_ut + 488.0;
  CHECK(linear_pixels_per_step(opt) == 1);
  //RR 5 10 20 40 80 Jahre, 96 48 24 12 6 Pixel pro Jahr
  opt.kind = LinearKind::kSecondary;
  opt.from_years = 20.0;
  for (const auto& [span, px] : std::vector<std::pair<double, int>>{{5, 96}, {10, 48}, {20, 24}, {40, 12}, {80, 6}}) {
    opt.to_years = 20.0 + span;
    CHECK(linear_pixels_per_step(opt) == px);
  }
}

TEST_CASE("the sign lines of the linear graph fold every boundary") {
  // his a18_lin drew i * nb(w * 30 degrees), under a 45 degree base only
  // the first boundary at 240 landed in the band. Folded one by one they
  // fall on 240 and 120, y 180 and 300
  const std::vector<double> f = linear_sign_lines(45.0);
  REQUIRE(f.size() == 2);
  CHECK(f[0] == doctest::Approx(240.0));
  CHECK(f[1] == doctest::Approx(120.0));
  CHECK(linear_sign_lines(360.0).size() == 11);
  CHECK(linear_sign_lines(30.0).empty());
}

TEST_CASE("the linear graph marks his hits with the aspect and a hit line") {
  const Chart radix = sample_chart();
  SearchContext ctx;
  ctx.base.lon_deg_east = 11.3244;
  ctx.base.lat_deg = 48.1742;
  ctx.vsop = &vsop();
  ctx.eph = &eph();
  LinearOptions opt;
  opt.kind = LinearKind::kSecondary;
  opt.base_angle_deg = 90.0;
  opt.from_years = 0.0;
  opt.to_years = 20.0;
  // a square of the progressed Mars to the radix Sun ten years on
  LinearHit h;
  h.jd_ut = radix.jd_ut + 10.0 * radix.ta.tropical_year_days;
  h.running = body::kMars;
  h.radix = body::kSun;
  h.angle_deg = 90.0;
  opt.hits.push_back(h);
  const DisplayList dl = build_linear_graph(radix, opt, ctx);
  // x = 80 + 24 pixels per year times 10, y the sun line
  const double x = 80.0 + 24.0 * 10.0;
  const double y = 420.0 - norm_rad(4.0 * radix.b[body::kSun].el) * kRadToDeg;
  bool square = false;
  bool hit_line = false;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kGlyph && std::abs(p.x1 - x) < 0.6 && std::abs(p.y1 - y) < 0.6 && p.text == "□") {
      square = true;
    }
    if (p.kind == Primitive::Kind::kLine && std::abs(p.x1 - x) < 0.6 && std::abs(p.x2 - x) < 0.6 && std::abs(p.y1 - y) < 0.6 &&
        p.y2 == doctest::Approx(420.0) && p.color == 0xFF0000) {
      hit_line = true;
    }
  }
  CHECK(square);
  CHECK(hit_line);
}

TEST_CASE("the A4 sheet draws his element and quality columns") {
  const Chart c = sample_chart();
  REQUIRE(c.ok);
  const ChartSettings s;
  const AspectResult asp = scan_aspects(c, s, {});
  const MidpointResult mids = scan_midpoints(c, s, {});
  Histogram h;
  h.element_sign = {0, 12, 6, 0, 3};
  h.element_house = {0, 4, 8, 2, 0};
  h.quality_sign = {0, 10, 5, 6};
  h.quality_house = {0, 3, 6, 9};
  h.houses_counted = true;
  // the columns stand left of the wheel, below x 80
  const auto column = [](const DisplayList& dl, Rgb fill, double ground) {
    std::vector<const Primitive*> out;
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kRect && p.fill == fill && p.x1 < 80.0 && p.y1 <= ground &&
          p.y1 > ground - 70.0) {
        out.push_back(&p);
      }
    }
    return out;
  };
  const auto has_text = [](const DisplayList& dl, const std::string& t) {
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kText && p.text == t) {
        return true;
      }
    }
    return false;
  };
  constexpr double kElementGround = 576.0;
  constexpr double kQualityGround = 186.0;
  // the standard colours of elem_col, the SCHRAFFIERT STANDARD-FARBEN mode
  WheelOptions standard;
  standard.ring_colors = {0x000000, 0xFF0000, 0x808000, 0x008080, 0x00FFFF};

  // elem 1, the sign counts alone, sixty high at the maximum
  const DisplayList one = a4_print_sheet(c, s, asp, mids, {}, standard, h, HistogramMode::kSigns);
  const auto fire = column(one, 0xFF0000, kElementGround);
  REQUIRE(fire.size() == 1);
  CHECK(fire[0]->r2 == doctest::Approx(30.0));
  CHECK(fire[0]->y1 == doctest::Approx(kElementGround - 30.0));
  const auto water = column(one, 0x00C8C8, kElementGround);
  REQUIRE(water.size() == 1);
  CHECK(water[0]->r2 == doctest::Approx(7.5));
  // an empty element keeps a flat box on the ground like elemhist1
  const auto air = column(one, 0x008080, kElementGround);
  REQUIRE(air.size() == 1);
  CHECK(air[0]->r2 == doctest::Approx(0.0));
  CHECK(has_text(one, "12"));
  CHECK(has_text(one, "Elemente"));
  CHECK(has_text(one, "Kard-Fix-Ver"));
  const auto cardinal = column(one, 0xFF0000, kQualityGround);
  REQUIRE(cardinal.size() == 1);
  CHECK(cardinal[0]->r2 == doctest::Approx(30.0));

  // elem 2, sign count below, the house count stacked two units above
  const DisplayList two = a4_print_sheet(c, s, asp, mids, {}, standard, h, HistogramMode::kSignsAndHouses);
  const auto fire2 = column(two, 0xFF0000, kElementGround);
  REQUIRE(fire2.size() == 2);
  CHECK(fire2[0]->r2 == doctest::Approx(15.0));
  CHECK(fire2[1]->r2 == doctest::Approx(7.5));
  CHECK(fire2[1]->y1 == doctest::Approx(kElementGround - 30.0 - 2.0 - 7.5));
  // the number carries sign and house count together
  CHECK(has_text(two, "16"));

  // heliocentric has no houses, elem 2 falls back to the full signs
  ChartSettings helio;
  helio.heliocentric = true;
  const DisplayList hel = a4_print_sheet(c, helio, asp, mids, {}, standard, h, HistogramMode::kSignsAndHouses);
  const auto fire3 = column(hel, 0xFF0000, kElementGround);
  REQUIRE(fire3.size() == 1);
  CHECK(fire3[0]->r2 == doctest::Approx(30.0));

  // elem 3 leaves both histograms out
  const DisplayList none = a4_print_sheet(c, s, asp, mids, {}, standard, h, HistogramMode::kNone);
  CHECK_FALSE(has_text(none, "Elemente"));
  CHECK(column(none, 0xFF0000, kElementGround).empty());

  // his own cols% of eigfarb, red, green, cyan and blue, cyan printed
  // darker, the qualities take the first three
  const DisplayList own = a4_print_sheet(c, s, asp, mids, {}, {}, h, HistogramMode::kSigns);
  CHECK(column(own, 0x00FF00, kElementGround).size() == 1);
  CHECK(column(own, 0x00C8C8, kElementGround).size() == 1);
  CHECK(column(own, 0x0000FF, kElementGround).size() == 1);
  CHECK(column(own, 0x00FF00, kQualityGround).size() == 1);

  // weiss! leaves the frames empty
  WheelOptions white;
  white.hist_fill = RingFill::kWhite;
  const DisplayList empty = a4_print_sheet(c, s, asp, mids, {}, white, h, HistogramMode::kSigns);
  CHECK(column(empty, 0xFF0000, kElementGround).empty());
  CHECK(has_text(empty, "Elemente"));
}

TEST_CASE("the degree beside a glyph never reads 30") {
  // his CINT printed 30 for a body in the last half degree of a sign
  Chart c = sample_chart();
  REQUIRE(c.ok);
  c.b[body::kSun].el = (30.0 + 29.8) * kDegToRad;
  c.b[body::kSun].tb = 0.0;
  const DisplayList dl = build_wheel(c, {}, scan_aspects(c, {}, {}));
  bool thirty = false;
  bool twenty_nine = false;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kText) {
      thirty = thirty || p.text == "30";
      twenty_nine = twenty_nine || p.text == "29";
    }
  }
  CHECK_FALSE(thirty);
  CHECK(twenty_nine);
}

TEST_CASE("the Rhythmenlehre phase flips the rulers and arcs its house") {
  const Chart c = sample_chart();
  REQUIRE(c.ok);
  const AspectResult a = scan_aspects(c, {}, {});
  WheelOptions opt;
  opt.ruler_slot = body::kMars;
  opt.invert_nodes = false;
  // the dark squares of his SRCINVERT stamping
  const auto count = [](const DisplayList& dl, Rgb colour) {
    int n = 0;
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kRect && p.fill == colour) {
        ++n;
      }
    }
    return n;
  };
  CHECK(count(build_wheel(c, {}, a, opt), 0x000000) == 1);
  //RR plinv stamps the phase ruler with SRCINVERT, Venus turns dark and
  // the inverted birth ruler Mars turns back
  opt.flip_inverted = {body::kVenus, body::kMars};
  const DisplayList flipped = build_wheel(c, {}, a, opt);
  CHECK(count(flipped, 0x000000) == 1);
  // a1795, the red arc of width two at radius 154 over the fourth house
  opt.phase_house = 4;
  const DisplayList arc = build_wheel(c, {}, a, opt);
  int segments = 0;
  bool on_ring = true;
  for (const Primitive& p : arc.items) {
    if (p.kind == Primitive::Kind::kLine && p.color == 0xFF0000 && p.width == 2.0) {
      ++segments;
      const double r = std::hypot(p.x1 - kWheelCenterX, p.y1 - kWheelCenterY);
      on_ring = on_ring && std::abs(r - 154.0 * kWheelScale) < 0.01;
    }
  }
  const double span = norm_rad(c.houses.cusp[5] - c.houses.cusp[4]);
  CHECK(segments == static_cast<int>(std::ceil(span / (kPi / 90.0))));
  CHECK(on_ring);
}

TEST_CASE("the phase strip places its axis, ticks and pushed labels") {
  // his c3&, the start of the phase at 416, the end at 80
  CHECK(rhythm_axis_y(14.0, 14.0, 7.0) == doctest::Approx(416.0));
  CHECK(rhythm_axis_y(21.0, 14.0, 7.0) == doctest::Approx(80.0));
  CHECK(rhythm_axis_y(17.5, 14.0, 7.0) == doctest::Approx(248.0));
  // a negative period walks the past, the phase start still sits low
  CHECK(rhythm_axis_y(-3.5, 0.0, -7.0) == doctest::Approx(248.0));
  //RR a17911, from the bottom up every label keeps sixteen pixels
  const std::vector<double> rows = rhythm_label_rows({300.0, 310.0, 200.0, 305.0});
  CHECK(rows[1] == doctest::Approx(310.0));
  CHECK(rows[3] == doctest::Approx(294.0));
  CHECK(rows[0] == doctest::Approx(278.0));
  CHECK(rows[2] == doctest::Approx(200.0));
  RhythmPanel p;
  p.title = "Auslösung nach DÖBEREINER:";
  p.footer = "WEITER mit Leertaste";
  p.left.push_back({body::kMars, 0, 0, false, true, 300.0, "12. 3.87 P"});
  p.right.push_back({body::kSun, body::kMars, 4, false, false, 310.0, "12. 3.87"});
  p.right.push_back({body::kMoon, body::kMars, 0, true, false, 200.0, "1. 5.90"});
  DisplayList dl;
  add_rhythm_panel(dl, p);
  bool axis = false;
  bool tick_left = false;
  bool tick_right = false;
  bool inverted = false;
  bool square = false;
  bool mirror = false;
  for (const Primitive& q : dl.items) {
    if (q.kind == Primitive::Kind::kLine && q.x1 == 101.0 && q.x2 == 101.0 && q.y1 == 80.0 && q.y2 == 416.0) {
      axis = true;
    }
    if (q.kind == Primitive::Kind::kLine && q.x1 == 97.0 && q.x2 == 101.0 && q.y1 == 300.0) {
      tick_left = true;
    }
    if (q.kind == Primitive::Kind::kLine && q.x1 == 101.0 && q.x2 == 105.0 && q.y1 == 310.0) {
      tick_right = true;
    }
    if (q.kind == Primitive::Kind::kGlyph && q.x1 == 87.0 && q.color == 0xFFFFFF) {
      inverted = true;
    }
    if (q.kind == Primitive::Kind::kGlyph && q.text == aspect_glyph(4)) {
      square = true;
    }
    if (q.kind == Primitive::Kind::kText && q.text == "S") {
      mirror = true;
    }
  }
  CHECK(axis);
  CHECK(tick_left);
  CHECK(tick_right);
  CHECK(inverted);
  CHECK(square);
  CHECK(mirror);
  CHECK(rhythm_glyph(body::kAscendant) == "AC");
  CHECK(rhythm_glyph(body::kAriesPoint + 2) == sign_glyph(6));
}

namespace {

Chart grad_chart(std::initializer_list<std::pair<int, double>> bodies) {
  Chart c;
  c.ok = true;
  c.houses.ok = true;
  for (int i = 1; i <= 13; ++i) {
    c.houses.cusp[static_cast<std::size_t>(i)] = norm_rad((5.0 + (i - 1) * 30.0) * kDegToRad);
  }
  for (const auto& [slot, deg] : bodies) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  }
  return c;
}

bool has_text(const std::vector<GradEntry>& list, const std::string& t) {
  return std::any_of(list.begin(), list.end(), [&t](const GradEntry& e) { return e.text == t; });
}

}  // namespace

TEST_CASE("the GRAD-LISTE prints his entries in his order") {
  const Chart c = grad_chart({{body::kSun, 10.0},
                              {body::kMoon, 100.5},
                              {body::kNodeAsc, 50.0},
                              {body::kNodeDesc, 230.0},
                              {body::kChiron, 200.25}});
  ChartSettings s;
  const auto list = grad_list(c, s, false);
  REQUIRE(!list.empty());
  // the bodies first in slot order with his two precisions
  CHECK(list[0].text == " 10.00 SO");
  CHECK(list[1].text == "100.50 MO");
  CHECK(has_text(list, "200.2  CH"));
  // the midpoints of every pair, the precision after the first partner
  CHECK(has_text(list, " 55.25 SO-MO"));
  CHECK(has_text(list, "105.1  CH-CN") == false);
  // DR and DS never pair
  CHECK_FALSE(std::any_of(list.begin(), list.end(), [](const GradEntry& e) { return e.text.find("DR-DS") != std::string::npos; }));
  // without extras the cardinal points pair but never stand alone
  ChartSettings plain;
  plain.extra_bodies = false;
  const auto bare = grad_list(grad_chart({{body::kSun, 10.0}}), plain, false);
  CHECK(has_text(bare, " 50.00 SO-CN"));
  CHECK(has_text(bare, " 95.00 SO-LI"));
  // his a921 lifted 0 Aries to exactly kk and the pair guard pl > kk then
  // dropped it, the original listed no SO-AR at all. All four pair now
  CHECK(has_text(bare, "  5.00 SO-AR"));
  CHECK_FALSE(std::any_of(bare.begin(), bare.end(), [](const GradEntry& e) { return e.text.size() > 7 && e.text.substr(7) == "CN"; }));
  CHECK_FALSE(std::any_of(bare.begin(), bare.end(), [](const GradEntry& e) { return e.text.size() > 7 && e.text.substr(7) == "AR"; }));
  // the cusps H2 to H12 without the MC, the late ones never pair
  const auto with_cusps = grad_list(c, plain, true);
  CHECK(has_text(with_cusps, " 35.00 H2"));
  CHECK_FALSE(has_text(with_cusps, "275.00 H10"));
  CHECK(has_text(with_cusps, " 22.50 SO- 2"));
  CHECK_FALSE(std::any_of(with_cusps.begin(), with_cusps.end(), [](const GradEntry& e) { return e.text.find(" 8- 9") != std::string::npos; }));
  CHECK(has_text(with_cusps, " 50.00  2- 3"));
  // his SWITCH u& gives the cardinal points STR$(w,5,1) + " " for the
  // pairs they open, the port printed two decimals there before
  CHECK(has_text(bare, "135.0  CN-LI"));
  CHECK_FALSE(has_text(bare, "135.00 CN-LI"));
  // his CASE 33 TO 45 reads the pl() index, the chosen extras fill it
  // from 19 on. With every extra QU stands at 35 and opens its pairs
  // with two decimals, with the tester's CH QU XE it stands at 21
  const Chart far = grad_chart({{body::kSun, 10.0}, {body::kChiron, 200.0}, {body::kQuaoar, 280.0}, {body::kXena, 21.0}});
  ChartSettings every;
  every.enable_standard_extras();
  CHECK(has_text(grad_list(far, every, false), "330.50 QU-XE"));
  ChartSettings tester;
  tester.extra_bodies = true;
  tester.nk[2] = body::kChiron;
  tester.nk[17] = body::kQuaoar;
  tester.nk[22] = body::kXena;
  CHECK(has_text(grad_list(far, tester, false), "330.5  QU-XE"));
  // his h& = INT(180 / bb&)
  CHECK(grad_unit(plain) == 12);
  ChartSettings helio;
  helio.heliocentric = true;
  helio.extra_bodies = false;
  CHECK(grad_unit(helio) == 36);
}

TEST_CASE("the GRAD-LISTE pages hold five columns of 41 and the sorted panel") {
  std::vector<GradEntry> list;
  for (int i = 0; i < 300; ++i) {
    list.push_back({i * 1.2, "entry"});
  }
  GradSheetText t{"Grad-Liste |TEST", "Geozentrisch", " Ephemeride:App.1,Mit Parallaxe"};
  const DisplayList first = build_grad_sheet(list, 0, t, false, 12);
  int entries = 0;
  double last_x = 0.0;
  double last_bottom = 0.0;
  double row_size = 0.0;
  double row_pitch = 0.0;
  bool panel = false;
  for (const Primitive& p : first.items) {
    if (p.kind == Primitive::Kind::kText && p.text == "entry") {
      ++entries;
      last_x = p.x1;
      last_bottom = p.y1 + 0.5 * p.size;
      row_size = p.size;
      row_pitch = p.pitch;
    }
    panel = panel || (p.kind == Primitive::Kind::kText && p.text == "Gesamt -");
  }
  CHECK(entries == kGradPerPage);
  // the last entry of a page stands in the fifth column, forty first row,
  // on his bottom line yt& with @textc(xt&,yt&,12,q$) and its FONT WIDTH
  CHECK(last_x == doctest::Approx(3.0 + 4 * 104.0));
  CHECK(last_bottom == doctest::Approx(46.0 + 40 * 10.0));
  CHECK(row_size == doctest::Approx(12.0));
  CHECK(row_pitch == doctest::Approx(7.0));
  CHECK_FALSE(panel);
  const DisplayList second = build_grad_sheet(list, 1, t, true, 12);
  entries = 0;
  int bars = 0;
  for (const Primitive& p : second.items) {
    if (p.kind == Primitive::Kind::kText && p.text == "entry") {
      ++entries;
    }
    panel = panel || (p.kind == Primitive::Kind::kText && p.text == "Gesamt -");
    if (p.kind == Primitive::Kind::kLine && p.x1 == 576.0 && p.y1 == p.y2) {
      ++bars;
    }
  }
  CHECK(entries == 300 - kGradPerPage);
  CHECK(panel);
  // one bar stroke per entry over the whole list
  CHECK(bars == 300);
}

TEST_CASE("aspz1 maps every hit to his aspli row") {
  // first and last multiple take the base row, compounds the rows 13 on
  CHECK(chord_row(2, 1, false) == 2);
  CHECK(chord_row(3, 2, false) == 3);
  CHECK(chord_row(4, 3, false) == 4);
  CHECK(chord_row(4, 2, false) == 0);
  CHECK(chord_row(5, 4, false) == 5);
  CHECK(chord_row(5, 2, false) == 13);
  CHECK(chord_row(6, 5, false) == 6);
  CHECK(chord_row(7, 3, false) == 14);
  CHECK(chord_row(8, 3, false) == 15);
  CHECK(chord_row(8, 2, false) == 0);
  CHECK(chord_row(9, 4, false) == 16);
  CHECK(chord_row(9, 3, false) == 0);
  CHECK(chord_row(10, 3, false) == 17);
  CHECK(chord_row(10, 5, false) == 0);
  CHECK(chord_row(11, 6, false) == 18);
  CHECK(chord_row(12, 5, false) == 19);
  CHECK(chord_row(12, 11, false) == 12);
  CHECK(chord_row(12, 3, false) == 0);
  CHECK(chord_row(13, 1, false) == 0);
  // the Rhythmenlehre leaves the eighth and twelfth divisors out
  CHECK(chord_row(8, 1, true) == 0);
  CHECK(chord_row(12, 5, true) == 0);
  CHECK(chord_row(4, 1, true) == 4);
}

TEST_CASE("the standard chords carry the colours and DEFLINE styles of aspz1") {
  // ls& is the second argument of aspz1_0, 0 solid, 2 dot, 3 dash dot
  const ChordLine opposition = standard_chord(2);
  CHECK(opposition.on);
  CHECK(opposition.color == 0xFF0000);
  CHECK(opposition.style == Primitive::Style::kSolid);
  CHECK(standard_chord(3).color == 0x00C800);
  CHECK(standard_chord(3).style == Primitive::Style::kDotted);
  CHECK(standard_chord(4).style == Primitive::Style::kSolid);
  CHECK(standard_chord(7).color == 0x0000C8);
  CHECK(standard_chord(7).style == Primitive::Style::kDashDot);
  CHECK(standard_chord(11).color == 0x000000);
  CHECK(standard_chord(19).color == 0x00C800);
  CHECK(standard_chord(19).style == Primitive::Style::kDotted);
  CHECK_FALSE(standard_chord(1).on);
  CHECK_FALSE(standard_chord(20).on);
  CHECK(chord_line_style(1) == Primitive::Style::kDashed);
  CHECK(chord_line_style(3) == Primitive::Style::kDashDot);
}

TEST_CASE("the chord table decides which aspect lines the wheel draws") {
  const Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  const auto lines_in = [](const DisplayList& dl, Rgb color) {
    int n = 0;
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kLine && p.color == color && p.width == doctest::Approx(0.7)) {
        ++n;
      }
    }
    return n;
  };
  int trines = 0;
  for (const AspectHit& h : a.hits) {
    if (chord_row(h.n, h.m, false) == 3) {
      ++trines;
    }
  }
  REQUIRE(trines > 0);
  // one own colour on the trine row alone
  WheelOptions opt;
  opt.chords = {};
  opt.chords[3] = {true, 0x123456, Primitive::Style::kDashDot};
  const DisplayList dl = build_wheel(c, {}, a, opt);
  CHECK(lines_in(dl, 0x123456) == trines);
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kLine && p.color == 0x123456) {
      CHECK(p.style == Primitive::Style::kDashDot);
    }
  }
  // every row off keeps only the red conjunction dots
  opt.chords = {};
  const DisplayList bare = build_wheel(c, {}, a, opt);
  CHECK(lines_in(bare, 0x123456) == 0);
  CHECK(lines_in(bare, 0x00C800) == 0);
}

TEST_CASE("the sign band follows the colour modes of avh") {
  const Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  const auto fills = [&](const WheelOptions& opt) {
    std::vector<Rgb> out;
    for (const Primitive& p : build_wheel(c, {}, a, opt).items) {
      if (p.kind == Primitive::Kind::kSector) {
        out.push_back(p.fill);
      }
    }
    return out;
  };
  // his own cols% hatched render as the shades of his screen
  const std::vector<Rgb> shaded = fills({});
  REQUIRE(shaded.size() == 12);
  CHECK(shaded[0] == 0xFF9086);
  CHECK(shaded[1] == 0x14CD14);
  CHECK(shaded[2] == 0xDEF2F2);
  CHECK(shaded[3] == 0x4646FF);
  CHECK(shaded[4] == 0xFF9086);
  // another colour takes the same hatch density, air at 13 percent ink
  WheelOptions teal;
  teal.ring_colors = {0x000000, 0xFF0000, 0x808000, 0x008080, 0x00FFFF};
  const std::vector<Rgb> standard = fills(teal);
  CHECK(standard[2] == 0xDEEEEE);
  // FARBIG PUR paints the colour itself, WEIß and the symbol modes none
  WheelOptions solid;
  solid.ring_fill = RingFill::kSolid;
  CHECK(fills(solid)[1] == 0x00FF00);
  WheelOptions white;
  white.ring_fill = RingFill::kWhite;
  CHECK(fills(white)[0] == kPaperColor);
  // nursymb 1 dresses the sign glyphs in their element colour
  WheelOptions colored = white;
  colored.colored_signs = true;
  int red_signs = 0;
  for (const Primitive& p : build_wheel(c, {}, a, colored).items) {
    if (p.kind == Primitive::Kind::kGlyph && p.color == 0xFF0000 && p.size == doctest::Approx(11.0)) {
      ++red_signs;
    }
  }
  CHECK(red_signs >= 3);
}

TEST_CASE("pziff, klsy and begz shape the planet ring") {
  const Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  const auto texts = [&](const WheelOptions& opt, const std::string& t) {
    int n = 0;
    for (const Primitive& p : build_wheel(c, {}, a, opt).items) {
      if (p.kind == Primitive::Kind::kText && p.text == t) {
        ++n;
      }
    }
    return n;
  };
  bool retro = false;
  for (int slot = 3; slot <= 10; ++slot) {
    retro = retro || c.b[static_cast<std::size_t>(slot)].tb < 0.0;
  }
  REQUIRE(retro);
  // pziff 1 writes the R in the colour of hard&, pziff 2 drops it
  WheelOptions one;
  one.outer_color = 3;
  CHECK(texts(one, "R") > 0);
  for (const Primitive& p : build_wheel(c, {}, a, one).items) {
    if (p.kind == Primitive::Kind::kText && p.text == "R") {
      CHECK(p.color == 0x0000FF);
    }
  }
  WheelOptions two;
  two.retro_marks = false;
  CHECK(texts(two, "R") == 0);
  // KLEIN-SYMBOLE shrink the body glyphs to five sevenths, the signs stay
  WheelOptions small;
  small.small_symbols = true;
  int small_glyphs = 0;
  int sign_glyphs = 0;
  for (const Primitive& p : build_wheel(c, {}, a, small).items) {
    if (p.kind == Primitive::Kind::kGlyph && p.size == doctest::Approx(11.0 * 5.0 / 7.0)) {
      ++small_glyphs;
    }
    if (p.kind == Primitive::Kind::kGlyph && p.size == doctest::Approx(11.0)) {
      ++sign_glyphs;
    }
  }
  CHECK(small_glyphs >= 12);
  CHECK(sign_glyphs == 12);
  // horbeg, 0 WIDDER puts the first sector start on the left
  const auto first_sector = [&](const WheelOptions& opt) {
    for (const Primitive& p : build_wheel(c, {}, a, opt).items) {
      if (p.kind == Primitive::Kind::kSector) {
        return p.a1;
      }
    }
    return -1.0;
  };
  WheelOptions aries;
  aries.begin = 3;
  CHECK(first_sector(aries) == doctest::Approx(kPi));
  WheelOptions libra;
  libra.begin = 4;
  CHECK(first_sector(libra) == doctest::Approx(0.0));
  WheelOptions own;
  own.begin = 5;
  own.begin_lon = kPi / 2.0;
  CHECK(first_sector(own) == doctest::Approx(kPi / 2.0));
  WheelOptions mc;
  mc.begin = 2;
  CHECK(first_sector(mc) == doctest::Approx(norm_rad(kPi - c.houses.angles.mc)));
}

TEST_CASE("asp1 draws the apogee line, the chart label and his histogram") {
  const Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  const auto count = [](const DisplayList& dl, auto pred) {
    return static_cast<int>(std::count_if(dl.items.begin(), dl.items.end(), pred));
  };
  // the Black Moon line runs black and dashed through the centre, both
  // axes vanish with the chords
  Chart with_ag = c;
  with_ag.b[body::kApogee].present = true;
  with_ag.b[body::kApogee].valid = true;
  with_ag.b[body::kApogee].el = 1.0;
  WheelOptions opt;
  opt.apogee_axis = true;
  const auto apogee_line = [](const Primitive& p) {
    return p.kind == Primitive::Kind::kLine && p.color == 0x000000 && p.style == Primitive::Style::kDashed;
  };
  const auto node_line = [](const Primitive& p) {
    return p.kind == Primitive::Kind::kLine && p.color == 0x0000FF && p.style == Primitive::Style::kDashed;
  };
  CHECK(count(build_wheel(with_ag, {}, a, opt), apogee_line) == 1);
  CHECK(count(build_wheel(with_ag, {}, a, opt), node_line) == 1);
  opt.aspect_lines = false;
  CHECK(count(build_wheel(with_ag, {}, a, opt), apogee_line) == 0);
  CHECK(count(build_wheel(with_ag, {}, a, opt), node_line) == 0);
  // bes2 writes sol$ into the centre, bes2_comp the line under it
  WheelOptions named;
  named.chart_label = "RADIX";
  named.chart_sub_label = "Mundan";
  bool label = false;
  bool sub = false;
  for (const Primitive& p : build_wheel(c, {}, a, named).items) {
    if (p.kind == Primitive::Kind::kText && p.text == "RADIX") {
      label = p.x1 == doctest::Approx(kWheelCenterX) && p.y1 == doctest::Approx(kWheelCenterY);
    }
    if (p.kind == Primitive::Kind::kText && p.text == "Mundan") {
      sub = p.y1 == doctest::Approx(kWheelCenterY + 10.0);
    }
  }
  CHECK(label);
  CHECK(sub);
  // asphist, one bar per divisor, the longest seventy units
  DisplayList hist;
  const double end = add_aspect_histogram(hist, a, 12, 256.0, 670.0, aspect_hist_colors(nullptr), RingFill::kShaded, 9.5);
  int bars = 0;
  double longest = 0.0;
  for (const Primitive& p : hist.items) {
    if (p.kind == Primitive::Kind::kRect) {
      ++bars;
      longest = std::max(longest, 2.0 * p.r1);
    }
  }
  CHECK(bars == 12);
  CHECK(longest == doctest::Approx(70.0));
  // two heading lines, then eight units a bar
  CHECK(end == doctest::Approx(670.0 + 12.0 - 2.0 + 12 * 8.0));
  int sum = 0;
  for (int i = 1; i <= 12; ++i) {
    sum += a.zh[static_cast<std::size_t>(i)];
  }
  char total[24];
  std::snprintf(total, sizeof(total), "Summe H =%3d", sum);
  CHECK(count(hist, [&](const Primitive& p) { return p.kind == Primitive::Kind::kText && p.text == total; }) == 1);
  // the own colours of asphi1, a row without colour turns grey
  ChordTable own{};
  own[3] = {true, 0x123456, Primitive::Style::kSolid};
  const auto colors = aspect_hist_colors(&own);
  CHECK(colors[1] == 0x0000FF);
  CHECK(colors[3] == 0x123456);
  CHECK(colors[4] == 0xC0C0C0);
  CHECK(colors[13] == 0xFFFFFF);
  CHECK(aspect_hist_colors(nullptr)[9] == 0xFF0000);
}

TEST_CASE("the R marks retrograde extras but never the nodes") {
  Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  for (int slot = 1; slot < body::kSlotCount; ++slot) {
    c.b[static_cast<std::size_t>(slot)].tb = 1.0;
  }
  c.b[body::kChiron].present = true;
  c.b[body::kChiron].valid = true;
  c.b[body::kChiron].el = 2.0;
  c.b[body::kChiron].tb = -0.01;
  c.b[body::kNodeAsc].tb = -0.05;
  int rs = 0;
  for (const Primitive& p : build_wheel(c, {}, a, {}).items) {
    if (p.kind == Primitive::Kind::kText && p.text == "R") {
      ++rs;
    }
  }
  CHECK(rs == 1);
}

TEST_CASE("the HALBSUMMEN-GRAPHIK stands in two bands of eleven trees") {
  std::vector<MidpointTree> trees;
  for (int slot = 1; slot <= 10; ++slot) {
    MidpointTree t;
    t.slot = slot;
    t.lon = slot * 0.3;
    trees.push_back(t);
  }
  MidpointTree h2;
  h2.slot = 15;
  h2.cusp = 2;
  h2.lon = 1.0;
  trees.push_back(h2);
  // the twelfth tree opens the bottom band, ten hits box the ninth branch
  MidpointTree busy;
  busy.slot = 11;
  busy.lon = 2.0;
  for (int k = 0; k < 10; ++k) {
    busy.hits.push_back({11, 1, 2, k < 5 ? 1 : 8});
  }
  trees.push_back(busy);
  std::array<int, body::kSlotCount> marks{};
  marks[1] = 1;
  const DisplayList dl = build_midpoint_trees(trees, 0, {}, false, {}, marks);
  const auto find_text = [&](const std::string& s) -> const Primitive* {
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kText && p.text == s) {
        return &p;
      }
    }
    return nullptr;
  };
  // his H2 head, size 16 on the bottom line d + 10
  const Primitive* head = find_text("H2");
  REQUIRE(head != nullptr);
  CHECK(head->x1 == doctest::Approx(57.0 * 11 - 29.0 - 8.0));
  CHECK(head->y1 == doctest::Approx(40.0 + 10.0 - 8.0));
  int letters_v = 0;
  int trunks_bottom = 0;
  int red_suns = 0;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kText && p.text == "V") {
      ++letters_v;
    }
    if (p.kind == Primitive::Kind::kLine && p.x1 == doctest::Approx(28.0) && p.y1 == doctest::Approx(259.0) &&
        p.y2 == doctest::Approx(411.0)) {
      ++trunks_bottom;
    }
    if (p.kind == Primitive::Kind::kGlyph && p.color == 0xFF0000) {
      ++red_suns;
    }
  }
  // nine branches, the tenth hit only draws the box
  CHECK(letters_v == 4);
  CHECK(trunks_bottom == 1);
  CHECK(red_suns == 10);
  // the sorted view prints the dial value under every tree
  const std::vector<MidpointTree> sorted = sort_trees_by_dial(trees);
  for (std::size_t i = 1; i < sorted.size(); ++i) {
    CHECK(tree_dial(sorted[i - 1].lon) <= tree_dial(sorted[i].lon));
  }
  std::vector<bool> close(sorted.size(), false);
  close[0] = true;
  const DisplayList sd = build_midpoint_trees(sorted, 0, {}, true, close, {});
  char first[16];
  std::snprintf(first, sizeof(first), "%6.2f", tree_dial(sorted[0].lon) * kRadToDeg);
  bool red_value = false;
  for (const Primitive& p : sd.items) {
    red_value = red_value || (p.kind == Primitive::Kind::kText && p.text == first && p.color == 0xFF0000);
  }
  CHECK(red_value);
}

TEST_CASE("the ASPEKTARIUM sheet follows the cells of aspar") {
  const Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  AspektariumInput in;
  in.chart = &c;
  in.aspects = &a;
  in.weights.fill(100);
  in.emphasis[body::kMoon] = 1;
  AspektariumText text;
  text.title = "Geozentrisches Aspektarium  | RADIX";
  text.name = "TEST FALL";
  const DisplayList dl = build_aspektarium(in, text);
  const auto texts = [&](const std::string& s) {
    std::vector<const Primitive*> out;
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kText && p.text == s) {
        out.push_back(&p);
      }
    }
    return out;
  };
  // the sun row count stands on the hatched diagonal at column one,
  // 18 lower like his pasted block
  char sun[8];
  std::snprintf(sun, sizeof(sun), "%2d", a.az[body::kSun]);
  bool diagonal = false;
  for (const Primitive* p : texts(sun)) {
    diagonal = diagonal || (p->x1 == doctest::Approx(16.0 + 26.0 - 6.0) && p->y1 == doctest::Approx(10.0 + 20.0 + 6.0 - 6.0 + 18.0));
  }
  CHECK(diagonal);
  // the divisor table of the right panel, twelve rows, 360 over one
  CHECK(texts("360.0\xC2\xB0").size() == 1);
  CHECK(texts(" 30.0\xC2\xB0").size() == 1);
  // nam splits the name at its first space
  CHECK(texts("TEST").size() == 1);
  CHECK(texts("FALL").size() == 1);
  // the weights row ends with the percent after MC
  CHECK(texts("100%").size() == 1);
  // a matched Moon pair prints its separation red for plan_col
  bool red_moon = false;
  for (int w = 3; w <= 14; ++w) {
    if (a.asp[body::kMoon][static_cast<std::size_t>(w)] > 0.0) {
      for (const Primitive& p : dl.items) {
        red_moon = red_moon || (p.kind == Primitive::Kind::kText && p.color == 0xFF0000 && p.text.find('\'') != std::string::npos);
      }
    }
  }
  CHECK(red_moon);
  // the histogram inset stands in its box without the first heading
  CHECK(texts("Teiler H\xC3\xA4ufigkt.").size() == 1);
  CHECK(texts("Aspekt 360/N :").empty());
  // a fixed point shifts every column and drops the inset
  Chart f = c;
  f.b[0].present = true;
  f.b[0].valid = true;
  f.b[0].el = 1.0;
  in.chart = &f;
  const DisplayList fd = build_aspektarium(in, text);
  bool red_f = false;
  bool inset = false;
  for (const Primitive& p : fd.items) {
    red_f = red_f || (p.kind == Primitive::Kind::kGlyph && p.text == "F" && p.color == 0xFF0000 &&
                      p.x1 == doctest::Approx(42.0));
    inset = inset || (p.kind == Primitive::Kind::kText && p.text == "Teiler H\xC3\xA4ufigkt.");
  }
  CHECK(red_f);
  CHECK_FALSE(inset);
}

TEST_CASE("the pair sheet writes his bes11 rows, the a12asp grid and the halbsm boxes") {
  // gz7$ and gz$, the minute rounded with its carry into the degree
  CHECK(pair_row_text(body::kSun, (14.0 + 11.0 / 60.0) * kDegToRad + 300.0 * kDegToRad, false) == "SO 14 AQ 11");
  CHECK(pair_row_text(body::kMoon, (3.0 + 59.8 / 60.0) * kDegToRad, true) == "MO  4\xC2\xB0" "AR  0'");
  const Chart c = sample_chart();
  DisplayList dl;
  PairColumnOptions col;
  col.header = "Ekl.L\xC3\xA4nge:A1";
  col.houses_header = "H\xC3\xA4userspitzen";
  col.house_name = "Placidus";
  const double y = add_pair_bodies(dl, c, col, 4.0, 24.0);
  // eleven rows nine apart, the south node never, four to close
  CHECK(y == doctest::Approx(24.0 + 11 * 9.0 + 4.0));
  const double h = add_pair_houses(dl, c, col, 4.0, y);
  CHECK(h == doctest::Approx(y + 12.0 + 12.0 + 2.0 + 6 * 9.0 + 2.0));
  // one hit draws the tags, the glyphs, the sprite and the separation
  std::vector<CrossAspectHit> hits{{body::kSun, body::kMoon, 3, 1, 119.6}};
  DisplayList grid;
  CrossGridOptions opt;
  opt.outer_color = 1;
  add_cross_grid(grid, hits, opt, 2.0, 200.0);
  bool tag = false;
  bool sep = false;
  bool red_moon = false;
  for (const Primitive& p : grid.items) {
    tag = tag || (p.kind == Primitive::Kind::kText && p.text == " I" && p.x1 == doctest::Approx(3.0));
    sep = sep || (p.kind == Primitive::Kind::kText && p.text == "120" && p.x1 == doctest::Approx(2.0 + 17.0));
    red_moon = red_moon || (p.kind == Primitive::Kind::kGlyph && p.x1 == doctest::Approx(2.0 + 43.0) && p.color == 0xFF0000);
  }
  CHECK(tag);
  CHECK(sep);
  CHECK(red_moon);
  // halbsm, the sign axis red on cyan at x 6 from 454 up
  DisplayList boxes;
  add_multi_midpoints(boxes, {{body::kSun, body::kMoon, MultiMidpoint::Target::kSignAxis, 1, 7},
                              {body::kMars, body::kVenus, MultiMidpoint::Target::kRadixCusp, 2, 8}},
                      false);
  bool axis = false;
  bool cusp = false;
  for (const Primitive& p : boxes.items) {
    axis = axis || (p.kind == Primitive::Kind::kText && p.text == "SO-MO M = AR/LI" && p.color == 0xFF0000);
    cusp = cusp || (p.kind == Primitive::Kind::kText && p.text == "MA-VE M = HS 2/8 R" && p.color == 0x000080 &&
                    p.y1 == doctest::Approx(454.0 - 14.0 + 1.0 - 5.5));
  }
  CHECK(axis);
  CHECK(cusp);
}

TEST_CASE("a composite with real houses names H1 and gives the AC midpoint its own axis") {
  Chart c = sample_chart();
  REQUIRE(c.ok);
  // the AC point a sign away from the first cusp, like a composite AC
  // midpoint beside its computed house
  c.b[body::kAscendant].el = norm_rad(c.houses.cusp[1] + 30.0 * kDegToRad);
  const auto texts = [](const DisplayList& dl, const std::string& tag) {
    std::vector<const Primitive*> out;
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kText && p.text == tag) {
        out.push_back(&p);
      }
    }
    return out;
  };
  WheelOptions opt;
  opt.composite_axes = true;
  const DisplayList dl = build_wheel(c, {}, scan_aspects(c, {}, {}), opt);
  //RR h$ = "H1 " on the first cusp, "AC" on plz(0,1,13)
  REQUIRE(texts(dl, "H1").size() == 1);
  REQUIRE(texts(dl, "AC").size() == 1);
  const Primitive* h1 = texts(dl, "H1").front();
  const Primitive* ac = texts(dl, "AC").front();
  const double a_h1 = std::atan2(-(h1->y1 - kWheelCenterY), h1->x1 - kWheelCenterX);
  const double a_ac = std::atan2(-(ac->y1 - kWheelCenterY), ac->x1 - kWheelCenterX);
  double d = std::abs(a_ac - a_h1) * kRadToDeg;
  if (d > 180.0) {
    d = 360.0 - d;
  }
  CHECK(d == doctest::Approx(30.0).epsilon(0.03));
  // the schematic composite keeps the plain AC on the first cusp
  const DisplayList plain = build_wheel(c, {}, scan_aspects(c, {}, {}));
  CHECK(texts(plain, "H1").empty());
  CHECK(texts(plain, "AC").size() == 1);
}

TEST_CASE("the composite sheet drops Vel. and lists H1 with the AC row") {
  Chart c = sample_chart();
  REQUIRE(c.ok);
  c.b[body::kAscendant].el = norm_rad(c.houses.cusp[1] + 30.0 * kDegToRad);
  const auto has = [](const DisplayList& dl, const std::string& prefix) {
    return std::any_of(dl.items.begin(), dl.items.end(), [&](const Primitive& p) {
      return p.kind == Primitive::Kind::kText && p.text.rfind(prefix, 0) == 0;
    });
  };
  ClassicSheetText txt;
  txt.longitudes_only = true;
  txt.h1_axis = true;
  DisplayList dl = build_wheel(c, {}, scan_aspects(c, {}, {}));
  add_classic_text(dl, c, {}, txt);
  CHECK_FALSE(has(dl, "Vel."));
  CHECK(has(dl, " H1:"));
  CHECK(has(dl, " AC:"));
  // no place, no Ort: box, a composite has none of its own
  CHECK_FALSE(has(dl, "Ort:"));
  // the radix sheet keeps its speed column, the AC row and its place
  ClassicSheetText radix;
  radix.place = "TESTORT";
  DisplayList rl = build_wheel(c, {}, scan_aspects(c, {}, {}));
  add_classic_text(rl, c, {}, radix);
  CHECK(has(rl, "Vel."));
  CHECK(has(rl, " AC:"));
  CHECK_FALSE(has(rl, " H1:"));
  CHECK(has(rl, "Ort:"));
}

TEST_CASE("the classic sheet lists the Spiegelung pairs like spieg1") {
  const Chart c = sample_chart();
  REQUIRE(c.ok);
  DisplayList dl = build_wheel(c, {}, scan_aspects(c, {}, {}));
  const std::vector<std::pair<int, int>> mirrors{
      {body::kMoon, body::kChiron}, {body::kNodeAsc, body::kXena}, {body::kSun, body::kVenus}};
  add_classic_text(dl, c, {}, ClassicSheetText{}, mirrors);
  const auto find = [&dl](const std::string& t) -> const Primitive* {
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kText && p.text == t) {
        return &p;
      }
    }
    return nullptr;
  };
  const Primitive* label = find("Spiegelung:");
  const Primitive* mo = find("MO");
  const Primitive* ch = find("-CH");
  const Primitive* dr = find("DR");
  const Primitive* so = find("SO");
  REQUIRE(label != nullptr);
  REQUIRE(mo != nullptr);
  REQUIRE(ch != nullptr);
  REQUIRE(dr != nullptr);
  REQUIRE(so != nullptr);
  // the partner 14 right of the first tag, the pairs 50 apart, the first
  // row 10 under the caption and the third pair on a new row
  CHECK(ch->x1 - mo->x1 == doctest::Approx(14.0));
  CHECK(dr->x1 - mo->x1 == doctest::Approx(50.0));
  CHECK(mo->y1 - label->y1 == doctest::Approx(10.0));
  CHECK(dr->y1 == doctest::Approx(mo->y1));
  CHECK(so->x1 == doctest::Approx(mo->x1));
  CHECK(so->y1 - mo->y1 == doctest::Approx(10.0));
}

TEST_CASE("a composite AC close to H1 stacks its tag above the cusp tag") {
  Chart c = sample_chart();
  REQUIRE(c.ok);
  c.b[body::kAscendant].el = norm_rad(c.houses.cusp[1] + 2.0 * kDegToRad);
  WheelOptions opt;
  opt.composite_axes = true;
  const DisplayList dl = build_wheel(c, {}, scan_aspects(c, {}, {}), opt);
  const Primitive* h1 = nullptr;
  const Primitive* ac = nullptr;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kText && p.text == "H1") {
      h1 = &p;
    }
    if (p.kind == Primitive::Kind::kText && p.text == "AC") {
      ac = &p;
    }
  }
  REQUIRE(h1 != nullptr);
  REQUIRE(ac != nullptr);
  // a whole tag and its degree apart, never on top of one another
  CHECK(h1->y1 - ac->y1 > h1->size + 4.0);
}

TEST_CASE("the printed page carries its moment behind the credit like drad2") {
  DisplayList sheet;
  Primitive credit;
  credit.kind = Primitive::Kind::kText;
  credit.anchor = Primitive::Anchor::kCredit;
  credit.text = std::string(kCreditLine);
  sheet.items.push_back(credit);
  stamp_credit(sheet, "24.09.2026 14:05");
  REQUIRE(sheet.items.size() == 1);
  CHECK(sheet.items[0].text == std::string(kCreditLine) + " 24.09.2026 14:05");

  // a table without a credit gets the dradst line centred at its foot
  DisplayList table;
  table.width = 640.0;
  table.height = 459.0;
  stamp_credit(table, "24.09.2026 14:05");
  REQUIRE(table.items.size() == 1);
  CHECK(table.items[0].anchor == Primitive::Anchor::kCredit);
  CHECK(table.items[0].x1 == doctest::Approx(320.0));
  CHECK(table.items[0].y1 == doctest::Approx(456.0));
  CHECK(table.items[0].text == std::string(kCreditLine) + " 24.09.2026 14:05");
}

TEST_CASE("his textg gives every text height its character width") {
  // the screen sizes of text and textc, 16 high and 8 wide for the tables
  CHECK(font_pitch(8.0) == 7.0);
  CHECK(font_pitch(14.0) == 7.0);
  CHECK(font_pitch(16.0) == 8.0);
  CHECK(font_pitch(19.0) == 10.0);
  CHECK(font_pitch(24.0) == 16.0);
  // the heights his table left to the printer keep the face
  CHECK(font_pitch(6.0) == 0.0);
  CHECK(font_pitch(40.0) == 0.0);

  // the SVG writer stretches such a text to his advance
  DisplayList dl;
  Primitive p;
  p.kind = Primitive::Kind::kText;
  p.size = 16.0;
  p.pitch = font_pitch(16.0);
  p.text = "9. 5.2000";
  dl.items.push_back(p);
  CHECK(to_svg(dl).find("textLength=\"72.00\"") != std::string::npos);
}

namespace {

// a synthetic chart, positions in degrees on chosen slots
Chart placed(std::initializer_list<std::pair<int, double>> positions) {
  Chart c;
  c.ok = true;
  for (const auto& [slot, deg] : positions) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  }
  return c;
}

bool has_item_text(const DisplayList& dl, const std::string& t) {
  return std::any_of(dl.items.begin(), dl.items.end(),
                     [&](const Primitive& p) { return p.kind == Primitive::Kind::kText && p.text == t; });
}

}  // namespace

TEST_CASE("the 90 degree circle paints its third sector in the water colour of the own colours") {
  const Chart c = sample_chart();
  REQUIRE(c.ok);
  const AspectResult a = scan_aspects(c, {}, {});
  const auto sectors = [&](const WheelOptions& opt) {
    std::vector<Rgb> out;
    for (const Primitive& p : build_wheel(c, {}, a, opt).items) {
      if (p.kind == Primitive::Kind::kSector) {
        out.push_back(p.fill);
      }
    }
    return out;
  };
  // fill_color under dop 4 hatches the third sector like air and takes
  // cols%(4) under eigfarb!, the port once took the air colour cols%(3)
  WheelOptions own;
  own.dial = true;
  REQUIRE(own.own_colors);
  const std::vector<Rgb> o = sectors(own);
  REQUIRE(o.size() == 3);
  CHECK(o[2] == ring_fill_color(3, own.ring_colors[4], RingFill::kShaded));
  CHECK(o[2] != ring_fill_color(3, own.ring_colors[3], RingFill::kShaded));
  CHECK(o[0] == ring_fill_color(1, own.ring_colors[1], RingFill::kShaded));
  // the standard colours of col_zeich keep the air colour there
  WheelOptions standard = own;
  standard.own_colors = false;
  standard.ring_colors = {0x000000, 0xFF0000, 0x808000, 0x008080, 0x00FFFF};
  CHECK(sectors(standard)[2] == ring_fill_color(3, 0x008080, RingFill::kShaded));
  // the full wheel keeps every element in its own colour
  WheelOptions full;
  CHECK(sectors(full)[2] == ring_fill_color(3, full.ring_colors[3], RingFill::kShaded));
}

TEST_CASE("every wheel wears the dress of his own profile unless told otherwise") {
  const WheelOptions opt;
  const WheelDress d = konsta_dress(robert_profile());
  for (int row = 0; row < kChordRows; ++row) {
    CAPTURE(row);
    const auto r = static_cast<std::size_t>(row);
    CHECK(opt.chords[r].on == d.chords[r].on);
    CHECK(opt.chords[r].color == d.chords[r].color);
  }
  // his own lines of selbst_cl_st!, the opposition red, the trine green,
  // the septile dash dotted
  CHECK(opt.chords[2].color == 0xFF0000);
  CHECK(opt.chords[3].color == 0x00FF00);
  CHECK(opt.chords[7].style == Primitive::Style::kDashDot);
  CHECK_FALSE(opt.chords[10].on);
  CHECK(opt.hist_colors[3] == 0x00FF00);
  // eigfarb! with his cols%, and hard& = 1 paints the outer symbols red
  CHECK(opt.own_colors);
  CHECK(opt.ring_colors[3] == 0x00FFFF);
  CHECK(opt.ring_colors[4] == 0x0000FF);
  CHECK(opt.outer_color == 1);
  // the standard rows of aspli| serve a profile without own lines
  Konsta k = robert_profile();
  k.selbst_cl_st = false;
  const WheelDress s = konsta_dress(k);
  CHECK(s.chords[7].on);
  CHECK(s.chords[7].color == standard_chord(7).color);
  CHECK_FALSE(s.chords[9].on);
  CHECK(s.hist_colors[3] == aspect_hist_colors(nullptr)[3]);
  // the standard ring colours of col_zeich without eigfarb!
  k.eigfarb = false;
  CHECK(konsta_dress(k).ring_colors[2] == 0x808000);
  CHECK(rgb_of_colorref(16776960) == 0x00FFFF);
}

TEST_CASE("the ASPEKTARIUM keeps a body at 0 Aries and carries the rounded minute") {
  // the Sun at exactly 0 Aries, the Moon 10 degrees 59.8 minutes away
  const Chart c = placed({{body::kSun, 0.0}, {body::kMoon, 10.0 + 59.8 / 60.0}, {body::kMars, 200.0}});
  const AspectResult a = scan_aspects(c, {}, {});
  REQUIRE(a.asp[body::kSun][body::kMoon] != 0.0);
  AspektariumInput in;
  in.chart = &c;
  in.aspects = &a;
  in.weights.fill(100);
  const DisplayList dl = build_aspektarium(in, {});
  // his plz(od,ze,o&) > kk dropped the Sun, its header sprite and its row
  bool sun_head = false;
  for (const Primitive& p : dl.items) {
    sun_head = sun_head || (p.kind == Primitive::Kind::kGlyph && p.text == body_glyph(body::kSun) &&
                            p.x1 == doctest::Approx(42.0) && p.y1 == doctest::Approx(9.0 + 18.0));
  }
  CHECK(sun_head);
  // STR$(FIX(di)) and STR$(60 * FRAC(di),2,0) read 10 degrees and 60
  // minutes, the port carries the rounded minute into the degree
  CHECK(has_item_text(dl, " 11\xC2\xB0"));
  CHECK(has_item_text(dl, " 0'"));
  CHECK_FALSE(has_item_text(dl, "60'"));
  CHECK_FALSE(has_item_text(dl, " 10\xC2\xB0"));
}

TEST_CASE("the ASPEKTARIUM stamps his inverted sprites, the hrg node weights and the ryt inset") {
  ChartSettings s;
  s.extra_bodies = true;
  s.nk[1] = body::kApogee;
  const Chart c = placed({{body::kSun, 10.0}, {body::kMoon, 130.0}, {body::kNodeAsc, 70.0}, {body::kNodeDesc, 250.0},
                          {body::kApogee, 40.0}});
  const AspectResult a = scan_aspects(c, s, {});
  AspektariumInput in;
  in.chart = &c;
  in.settings = s;
  in.aspects = &a;
  in.weights.fill(100);
  const auto inverted = [](const DisplayList& dl, int slot) {
    int patches = 0;
    int white = 0;
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kRect && p.r1 == doctest::Approx(kSpriteSize * kInvertPatchShare)) {
        ++patches;
      }
      if (p.kind == Primitive::Kind::kGlyph && p.text == body_glyph(slot) && p.color == kInvertedInk) {
        ++white;
      }
    }
    return std::pair{patches, white};
  };
  CHECK(inverted(build_aspektarium(in, {}), body::kApogee).second == 0);
  // plein2 under apogw! draws Lilith inverted, header, row and klplanz
  in.invert_apogee = true;
  const auto [patches, white] = inverted(build_aspektarium(in, {}), body::kApogee);
  CHECK(white >= 2);
  CHECK(patches == white);
  in.invert_apogee = false;
  in.invert_nodes = true;
  CHECK(inverted(build_aspektarium(in, {}), body::kNodeAsc).second >= 2);
  // the hrg weights row runs aa& to 14 without AC and MC, the nodes stay
  in.invert_nodes = false;
  in.settings.heliocentric = true;
  in.settings.extra_bodies = false;
  const DisplayList helio = build_aspektarium(in, {});
  bool ds_weight = false;
  bool mc_weight = false;
  for (const Primitive& p : helio.items) {
    ds_weight = ds_weight || (p.kind == Primitive::Kind::kText && p.text == "100" &&
                              p.x1 == doctest::Approx(84.0 + 12 * 28.0));
    mc_weight = mc_weight || (p.kind == Primitive::Kind::kText && p.text == "100%");
  }
  CHECK(ds_weight);
  CHECK_FALSE(mc_weight);
  // IF ryt! = 0 && e& < 15, the Rhythmenlehre leaves the inset away
  in.settings = {};
  CHECK(has_item_text(build_aspektarium(in, {}), "Teiler H\xC3\xA4ufigkt."));
  in.rhythm = true;
  CHECK_FALSE(has_item_text(build_aspektarium(in, {}), "Teiler H\xC3\xA4ufigkt."));
}

TEST_CASE("the HALBSUMMEN-GRAPHIK stamps the true node inverted under moknw") {
  MidpointTree t;
  t.slot = body::kSun;
  t.lon = 1.0;
  t.hits.push_back({body::kSun, body::kNodeAsc, body::kMoon, 1});
  const auto white_nodes = [&](const TreeGlyphs& g) {
    int n = 0;
    for (const Primitive& p : build_midpoint_trees({t}, 0, {}, false, {}, {}, g).items) {
      if (p.kind == Primitive::Kind::kGlyph && p.text == body_glyph(body::kNodeAsc) && p.color == kInvertedInk) {
        ++n;
      }
    }
    return n;
  };
  CHECK(white_nodes({}) == 0);
  CHECK(white_nodes({true, false}) == 1);
}

TEST_CASE("the paired sheets take their MEHR mark and the A4 headings from the caller") {
  std::vector<CrossAspectHit> hits;
  for (int i = 0; i < 120; ++i) {
    hits.push_back({1 + i % 12, 1 + (i / 12) % 12, 1, 1, 0.5});
  }
  DisplayList grid;
  CrossGridOptions opt;
  opt.more_label = " MORE ";
  add_cross_grid(grid, hits, opt, 2.0, 200.0);
  CHECK(has_item_text(grid, " MORE "));
  CHECK_FALSE(has_item_text(grid, " MEHR "));
  // the English edition hands its headings to the A4 sheet
  const Chart c = sample_chart();
  REQUIRE(c.ok);
  ClassicSheetText txt;
  txt.quality_heading = "Card-Fix-Mut";
  txt.element_heading = "Elements";
  Histogram h;
  h.element_sign = {0, 3, 2, 1, 1};
  h.quality_sign = {0, 2, 2, 3};
  const DisplayList a4 = a4_print_sheet(c, {}, scan_aspects(c, {}, {}), scan_midpoints(c, {}, {}), txt, {}, h,
                                        HistogramMode::kSigns);
  CHECK(has_item_text(a4, "Card-Fix-Mut"));
  CHECK(has_item_text(a4, "Elements"));
  CHECK_FALSE(has_item_text(a4, "Elemente"));
}
