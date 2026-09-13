// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <cmath>

#include "doctest.h"
#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
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

TEST_CASE("the wheel puts the ascendant on the left") {
  const Chart c = sample_chart();
  REQUIRE(c.ok);
  const AspectResult a = scan_aspects(c, {}, {});
  const DisplayList dl = build_wheel(c, {}, a);
  // the AC axis line must reach the label radius on the left side of the
  // centre 430, 224
  bool found = false;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kText && p.text == "AC") {
      found = true;
      CHECK(p.x1 < 430.0 - 0.9 * 190.0);
      CHECK(std::abs(p.y1 - 224.0) < 15.0);
    }
  }
  CHECK(found);
}

TEST_CASE("the display list carries the twelve sectors and three rings") {
  const Chart c = sample_chart();
  const AspectResult a = scan_aspects(c, {}, {});
  const DisplayList dl = build_wheel(c, {}, a);
  int sectors = 0;
  int circles = 0;
  int glyphs = 0;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kSector) {
      ++sectors;
    }
    if (p.kind == Primitive::Kind::kCircle) {
      ++circles;
    }
    if (p.kind == Primitive::Kind::kGlyph) {
      ++glyphs;
    }
  }
  CHECK(sectors == 12);
  CHECK(circles == 3);
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
      raw.emplace_back(430.0 + 0.95 * 128.0 * std::cos(-w), 224.0 + 0.95 * 128.0 * std::sin(-w));
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
