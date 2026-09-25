// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <algorithm>
#include <string>

#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/stat_sheet.hpp"

using namespace horcom;

namespace {

// every text of a page in drawing order
std::vector<std::string> texts(const DisplayList& dl) {
  std::vector<std::string> out;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kText) {
      out.push_back(p.text);
    }
  }
  return out;
}

bool has(const std::vector<std::string>& all, const std::string& s) {
  return std::find(all.begin(), all.end(), s) != all.end();
}

// the text item of a given string, the first one
const Primitive* find_text(const DisplayList& dl, const std::string& s) {
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kText && p.text == s) {
      return &p;
    }
  }
  return nullptr;
}

StatSheetRow row_at(double deg) {
  StatSheetRow r;
  r.slot = body::kSun;
  r.value = deg * kDegToRad;
  r.label = "ANNA MUSTER";
  r.moment = " 1. 2. 1990   3 h  0'";
  r.has_angle = true;
  r.angle = 10.0 * kDegToRad;
  return r;
}

}  // namespace

TEST_CASE("list_ausg prints the header, the rows and the footer of a page") {
  StatSheetText t;
  t.heads = "Datum     Zeit(UT)    AC";
  t.file = " Datei : DEMO.STA ";
  t.page = 3;
  t.object_line = "Länge SO";
  t.window_line = "Im Bereich 0..360°";
  t.footer = "* Blättern *";
  const DisplayList dl = build_stat_page({row_at(15.5)}, t);
  CHECK(dl.width == kCanvasWidth);
  CHECK(dl.height == kCanvasHeight);
  const auto all = texts(dl);
  CHECK(has(all, "Datum     Zeit(UT)    AC"));
  CHECK(has(all, " Datei : DEMO.STA "));
  // @seite, STR$(a&,3)
  CHECK(has(all, "  3"));
  CHECK(has(all, "Länge SO"));
  CHECK(has(all, "ANNA MUSTER"));
  CHECK(has(all, "* Blättern *"));
  // grzemise of 15.5 degrees Aries, degree and minute beside the sprite
  CHECK(has(all, "15\xC2\xB0"));
  CHECK(has(all, "30'"));
  // the first row stands on y% = 1 * 16 + 34
  const Primitive* name = find_text(dl, "ANNA MUSTER");
  REQUIRE(name != nullptr);
  CHECK(name->x1 == 164.0);
  CHECK(name->y1 + 0.5 * name->size == doctest::Approx(50.0));
}

TEST_CASE("grzemise carries a rounded minute into the next sign") {
  // 29 degrees 59.6 minutes Aries, his grze_0 printed 29 and 60'
  StatSheetText t;
  StatSheetRow r = row_at(29.0 + 59.6 / kArcminPerDeg);
  r.has_angle = false;
  const DisplayList dl = build_stat_page({r}, t);
  const auto all = texts(dl);
  CHECK(has(all, " 0\xC2\xB0"));
  CHECK(has(all, " 0'"));
  CHECK_FALSE(has(all, "60'"));
  CHECK_FALSE(has(all, "29\xC2\xB0"));
}

TEST_CASE("inf_box2 draws the sign bars of a single condition") {
  StatSheetText t;
  t.bars = true;
  t.sums = {4, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0};
  t.framed = 1;
  t.object_tag = "SO..AC ";
  t.total = 10;
  t.partial_count = 4;
  const DisplayList dl = build_stat_page({}, t);
  const auto all = texts(dl);
  CHECK(has(all, "SO..AC "));
  CHECK(has(all, "Zeichen"));
  CHECK(has(all, "Total ="));
  CHECK(has(all, "  10"));
  // STR$(100 * s,4,1) + "%" of one and three out of four
  CHECK(has(all, "25.0%"));
  CHECK(has(all, "75.0%"));
  CHECK(has(all, " 0.0%"));
  // Partial = zdm& with its share of the file
  CHECK(has(all, "4= 40.0%"));
  // twelve sign sprites down the left edge
  const auto glyphs = std::count_if(dl.items.begin(), dl.items.end(),
                                    [](const Primitive& p) { return p.kind == Primitive::Kind::kGlyph; });
  CHECK(glyphs == 12);
}

TEST_CASE("inf_box4 lays its counts and conditions over the rows") {
  StatSheetText t;
  t.multi = true;
  t.conditions = {" 1 NAME  A  ", " 2 UND  NAME  B  "};
  t.info_counts = {"   3 DATENSÄTZE wurden durchgesucht", "   2 DATENSÄTZE ERFÜLLEN Eine Beding."};
  const DisplayList off = build_stat_page({}, t);
  CHECK_FALSE(has(texts(off), "   3 DATENSÄTZE wurden durchgesucht"));
  t.info = true;
  const DisplayList on = build_stat_page({}, t);
  const Primitive* first = find_text(on, "   3 DATENSÄTZE wurden durchgesucht");
  REQUIRE(first != nullptr);
  // e& = 290 - anzb& * 12, the first line at e& + 16
  CHECK(first->x1 == 295.0);
  CHECK(first->y1 + 0.5 * first->size == doctest::Approx(290.0 - 2 * 12.0 + 16.0));
  // the conditions stand in the lower box from d& = 354 - anzb& * 12
  const Primitive* cond = find_text(on, " 2 UND  NAME  B  ");
  REQUIRE(cond != nullptr);
  const bool in_box = std::any_of(on.items.begin(), on.items.end(), [](const Primitive& p) {
    return p.kind == Primitive::Kind::kText && p.x1 == 295.0 && p.text == " 2 UND  NAME  B  " &&
           p.y1 + 0.5 * p.size == 354.0 - 2 * 12.0 + 2 * 12.0 + 4.0;
  });
  CHECK(in_box);
  // the corner is cleared white first
  const bool cleared = std::any_of(on.items.begin(), on.items.end(), [](const Primitive& p) {
    return p.kind == Primitive::Kind::kRect && p.fill == 0xFFFFFF && p.x1 == doctest::Approx(461.0);
  });
  CHECK(cleared);
}

TEST_CASE("several conditions print the chain labels and invert the complete ones") {
  StatSheetText t;
  t.multi = true;
  t.several = true;
  StatSheetRow done;
  done.label = "ANNA MUSTER          1u2";
  done.inverse = true;
  StatSheetRow open;
  open.label = "BERND BEISPIEL             1";
  const DisplayList dl = build_stat_page({done, open}, t);
  CHECK(has(texts(dl), "Mehrere Bedingungen !"));
  const Primitive* inv = find_text(dl, "ANNA MUSTER          1u2");
  REQUIRE(inv != nullptr);
  CHECK(inv->color == 0xFFFFFF);
  CHECK(inv->x1 == 142.0);
  const Primitive* plain = find_text(dl, "BERND BEISPIEL             1");
  REQUIRE(plain != nullptr);
  CHECK(plain->color == 0x000000);
}
