// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/render/midpoint_trees.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "horcom/chart/signs.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the geometry of aspar2, the top band hangs from y 40, the bottom band
// from y 247, trees stand 57 apart from x 28
constexpr double kTopBand = 40.0;
constexpr double kBottomBand = 247.0;
constexpr double kTopTrunkEnd = 202.0;
constexpr double kBottomTrunkEnd = 411.0;
constexpr double kTreeStep = 57.0;
constexpr double kTreeOffset = 29.0;
constexpr int kTreesPerBand = 11;
// nine branches 18 apart, the partner sprites 17 and 18 beside the trunk
constexpr int kBranches = 9;
constexpr double kBranchStep = 18.0;
constexpr double kTagSize = 9.0;
constexpr double kHeadSize = 16.0;
constexpr double kSmallText = 10.0;
constexpr double kHeaderText = 12.0;

}  // namespace

double tree_dial(double lon) {
  // FN nb(8 * pl(t&))
  return norm_rad(8.0 * lon);
}

std::vector<MidpointTree> sort_trees_by_dial(std::vector<MidpointTree> trees) {
  // ce%(t&) = 10000 * FN nb(8 * pl(t&)), sorted ascending
  std::stable_sort(trees.begin(), trees.end(), [](const MidpointTree& x, const MidpointTree& y) {
    return std::trunc(10000.0 * tree_dial(x.lon)) < std::trunc(10000.0 * tree_dial(y.lon));
  });
  return trees;
}

std::vector<bool> tree_dial_close(const std::vector<MidpointTree>& trees, const AspectSettings& orbs) {
  const AspectSettings a = tree_orb_settings(orbs);
  std::vector<bool> out(trees.size(), false);
  for (std::size_t i = 0; i < trees.size(); ++i) {
    for (std::size_t j = 0; j < trees.size(); ++j) {
      const int n = trees[i].slot;
      const int m = trees[j].slot;
      // IF n% <> np% && NOT((n% = 11 && np% = 12) OR (n% = 12 && np% = 11))
      if (n == m || (n == body::kNodeAsc && m == body::kNodeDesc) || (n == body::kNodeDesc && m == body::kNodeAsc)) {
        continue;
      }
      double w1 = tree_dial(trees[i].lon);
      double w2 = tree_dial(trees[j].lon);
      vergl1(w1, w2);
      const double gap = std::abs(w1 - w2);
      // IF ABS(w1 - w2) < MAX(op1,op2) && ABS(w1 - w2) > 0
      if (gap < std::max(org(a, n, 8), org(a, m, 8)) && gap > 0.0) {
        out[i] = true;
        break;
      }
    }
  }
  return out;
}

// ported from aspar2 with aspar2_ini and the graphic branch of halbsa
DisplayList build_midpoint_trees(const std::vector<MidpointTree>& trees, int page, const MidpointTreeText& text,
                                 bool sorted, const std::vector<bool>& close,
                                 const std::array<int, body::kSlotCount>& emphasis, const TreeGlyphs& glyphs) {
  DisplayList dl;
  dl.width = kCanvasWidth;
  dl.height = kCanvasHeight;
  auto add = [&](Primitive p) { dl.items.push_back(std::move(p)); };
  // his texts stand on their bottom line, the display list centres them
  const auto text_at = [&](double x, double bottom, const std::string& s, double size, Rgb color = kInkColor) {
    Primitive p;
    p.kind = Primitive::Kind::kText;
    p.x1 = x;
    p.y1 = bottom - 0.5 * size;
    p.size = size;
    // te_w& = @textg(te_gr&), FONT WIDTH te_w&
    p.pitch = font_pitch(size);
    p.color = color;
    p.align_left = true;
    p.text = s;
    add(p);
  };
  const auto line = [&](double x1, double y1, double x2, double y2, double width = 0.6) {
    Primitive p{Primitive::Kind::kLine, x1, y1, x2, y2};
    p.width = width;
    add(p);
  };
  const auto glyph = [&](int slot, double x, double y) {
    Primitive p;
    p.kind = Primitive::Kind::kGlyph;
    p.x1 = x;
    p.y1 = y;
    p.size = kSpriteSize;
    p.text = body_glyph(slot);
    // a body without a sprite shows its tag, the axes wear his AC and MC
    if (p.text.empty()) {
      p.kind = Primitive::Kind::kText;
      p.size = kTagSize;
      p.text = std::string(body::kName[static_cast<std::size_t>(slot)]);
    } else if (((slot == body::kNodeAsc || slot == body::kNodeDesc) && glyphs.invert_nodes) ||
               (slot == body::kApogee && glyphs.invert_apogee)) {
      // plinkl of plein2, the sprite inverted on a dark patch
      add(inverted_patch(x, y, kSpriteSize));
      p.color = kInvertedInk;
    }
    // plan_col!, the chosen bodies red
    if (emphasis[static_cast<std::size_t>(slot)] > 0) {
      p.color = kMarkRed;
    }
    add(p);
  };

  // aspar2_ini, the two header lines over the 2 pixel rule
  text_at(260.0, 14.0, "  " + text.sol, kHeaderText);
  text_at(340.0, 14.0, "  " + text.frame, kHeaderText);
  text_at(4.0, 14.0, text.name, kHeaderText);
  text_at(450.0, 14.0, "  " + text.date, kHeaderText);
  text_at(4.0, 23.0, text.clock, kHeaderText);
  line(2.0, 23.0, 638.0, 23.0, 1.2);
  line(2.0, 226.0, 627.0, 226.0);
  line(2.0, 436.0, 627.0, 436.0);
  text_at(200.0, 446.0, text.ephem, kSmallText);

  const std::size_t first = static_cast<std::size_t>(std::max(page, 0)) * kTreesPerPage;
  for (std::size_t k = first; k < trees.size() && k < first + kTreesPerPage; ++k) {
    const MidpointTree& tree = trees[k];
    const int i1 = static_cast<int>(k - first) + 1;
    const bool top = i1 <= kTreesPerBand;
    const double d = top ? kTopBand : kBottomBand;
    const double e = kTreeStep * (top ? i1 : i1 - kTreesPerBand) - kTreeOffset;
    line(e, d + 12.0, e, top ? kTopTrunkEnd : kBottomTrunkEnd);
    line(e + kTreeOffset, 23.0, e + kTreeOffset, 436.0);
    // the cusps as H2 to H6, size 16, the bodies as their sprite
    if (tree.cusp > 0) {
      text_at(e - 8.0, d + 10.0, "H" + std::to_string(tree.cusp), kHeadSize);
    } else {
      glyph(tree.slot, e, d + 4.0);
    }
    // STR$(p - 30 * l&,4,1) and zei$(l& + 1)
    const double p = norm_deg(tree.lon * kRadToDeg);
    const int l = static_cast<int>(p / kDegPerSign);
    char deg[16];
    std::snprintf(deg, sizeof(deg), "%4.1f", p - kDegPerSign * l);
    text_at(e - 26.0, d - 6.0, deg, kSmallText);
    text_at(e + 12.0, d - 6.0, kSignTag[std::clamp(l, 0, 11)], kSmallText);
    for (int q = 1; q <= kBranches; ++q) {
      line(e - 10.0, d + q * kBranchStep, e + 10.0, d + q * kBranchStep);
    }
    int z = 0;
    for (const MidpointHit& h : tree.hits) {
      // ELSE IF pl(t&) > kk, the graphic branch of halbsa
      if (z < kBranches) {
        ++z;
        const double y = d + z * kBranchStep;
        glyph(h.u, e - 17.0, y);
        glyph(h.w, e + 18.0, y);
        // the white patch his getbm/putbm pair cut into the trunk
        Primitive patch;
        patch.kind = Primitive::Kind::kRect;
        patch.x1 = e;
        patch.y1 = y;
        patch.r1 = 5.0;
        patch.r2 = 4.0;
        patch.fill = 0xFFFFFF;
        patch.color = 0xFFFFFF;
        add(patch);
        const char* letter = h.nh == 1 ? "D" : h.nh == 2 ? "Q" : h.nh == 4 ? "H" : "V";
        text_at(e - 4.0, y + 5.0, letter, kSmallText);
      } else {
        // @boxn(e& - 26,a& - 9,e& + 27,a& + 9), more than nine
        const double a = d + z * kBranchStep;
        line(e - 26.0, a - 9.0, e + 27.0, a - 9.0);
        line(e + 27.0, a - 9.0, e + 27.0, a + 9.0);
        line(e + 27.0, a + 9.0, e - 26.0, a + 9.0);
        line(e - 26.0, a + 9.0, e - 26.0, a - 9.0);
        break;
      }
    }
    if (sorted) {
      // STR$(p,6,2) of the dial value, red beside a close point
      char dial[16];
      std::snprintf(dial, sizeof(dial), "%6.2f", tree_dial(tree.lon) * kRadToDeg);
      const bool red = k < close.size() && close[k];
      text_at(e - 25.0, d + 186.0, dial, kSmallText, red ? kMarkRed : kInkColor);
    }
  }
  return dl;
}

}  // namespace horcom
