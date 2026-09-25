// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/degree_list.hpp"

#include <cstdio>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/composite.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the cusps H8, H9, H11 and H12 sat above his index 40 and never paired
// with each other
constexpr int kLateCuspFirst = 8;
// his pl() indices 33 to 45 print their pairs with two decimals, the
// cusps sit at 35 to 45
constexpr int kFineIndexFirst = 33;
constexpr int kFineIndexLast = 45;

struct Point {
  int slot = 0;
  int house = 0;
  double el = 0.0;
  std::string tag;
  // his SWITCH u& over the first point of a pair, STR$(w,6,2) or
  // STR$(w,5,1)
  bool pair_fine = true;
};

bool late_cusp(const Point& p) {
  return p.house >= kLateCuspFirst && p.house != 10;
}

// his pl() index of an extra body, the chosen extras fill the slots from
// 19 on in the order of nk& like his plgenkl
int compact_index(int slot, const ChartSettings& s) {
  int index = body::kCapricornPoint;
  for (int i = 1; i <= slot - body::kCapricornPoint && i < static_cast<int>(s.nk.size()); ++i) {
    if (s.nk[static_cast<std::size_t>(i)] > 0) {
      ++index;
    }
  }
  return index;
}

// the two decimals of a pair, his CASE 0 TO 14,n1&,n4& and CASE 33 TO 45
bool opens_fine_pairs(int slot, const ChartSettings& s) {
  if (slot <= body::kMc || slot == body::kApogee || slot == body::kFortune) {
    return true;
  }
  if (body::cardinal(slot)) {
    return false;
  }
  const int index = compact_index(slot, s);
  return index >= kFineIndexFirst && index <= kFineIndexLast;
}

}  // namespace

// ported from grli with grli_f
std::vector<GradEntry> grad_list(const Chart& chart, const ChartSettings& s, bool cusps) {
  std::vector<GradEntry> out;
  std::vector<Point> points;
  const bool helio = s.heliocentric;
  char buf[64];
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    if (body::cardinal(slot)) {
      // a921 puts them at po * (i& - 15) + kk, geo without extras only,
      // grli_f steps from 15 to 19 once extras are on. They pair but never
      // stand in the list. His pair guard pl > kk dropped 0 Aries, which
      // a921 lifts to exactly kk, all four pair here
      if (!helio && !s.extra_bodies) {
        const double el = (slot - body::kAriesPoint) * kHalfPi + kEps;
        const std::string_view n = body::kName[static_cast<std::size_t>(slot)];
        points.push_back({slot, 0, el, std::string(n), false});
      }
      continue;
    }
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid) {
      continue;
    }
    // his IF hrg! && (pl& = n1& OR pl& = n4&), no Black Moon, no Glückspunkt
    if (helio && (slot == body::kApogee || slot == body::kFortune)) {
      continue;
    }
    const double el = norm_rad(b.el);
    // his IF pl(t&) > kk
    if (el <= kEps) {
      continue;
    }
    const std::string_view n = (helio && slot == body::kMoon) ? body::kEarthName : body::kName[static_cast<std::size_t>(slot)];
    // his CASE 0 TO 14,n1&,n4& of the list, STR$(w,6,2) or STR$(w,5,1)
    const bool fine = slot <= body::kMc || slot == body::kApogee || slot == body::kFortune;
    points.push_back({slot, 0, el, std::string(n), opens_fine_pairs(slot, s)});
    const double w = el * kRadToDeg;
    // his za$ + " " + pl$ or STR$(w,5,1) + "  " + pl$
    std::snprintf(buf, sizeof(buf), fine ? "%6.2f %s" : "%5.1f  %s", w, points.back().tag.c_str());
    out.push_back({w, buf});
  }
  if (cusps && !helio && chart.houses.ok) {
    // his FOR t& = 35 TO 45, IF NOT t& = 43, the cusps without the MC
    for (int h = 2; h <= 12; ++h) {
      if (h == 10) {
        continue;
      }
      const double el = norm_rad(chart.houses.cusp[static_cast<std::size_t>(h)]);
      const double w = el * kRadToDeg;
      if (w <= 0.0) {
        continue;
      }
      std::snprintf(buf, sizeof(buf), "%6.2f H%d", w, h);
      out.push_back({w, buf});
      // his pl$(t&) = STR$(t& - 33,2)
      char tag[8];
      std::snprintf(tag, sizeof(tag), "%2d", h);
      points.push_back({0, h, el, tag, true});
    }
  }
  // the midpoint of every pair u < w, his halbsmin
  for (std::size_t i = 0; i + 1 < points.size(); ++i) {
    const Point& u = points[i];
    for (std::size_t j = i + 1; j < points.size(); ++j) {
      const Point& w = points[j];
      // his NOT ((u& = 11 && w& = 12) OR (u& = 12 && w& = 11))
      if (u.house == 0 && w.house == 0 && ((u.slot == body::kNodeAsc && w.slot == body::kNodeDesc) ||
                                           (u.slot == body::kNodeDesc && w.slot == body::kNodeAsc))) {
        continue;
      }
      // his NOT (u& > 40 && w& > 40)
      if (late_cusp(u) && late_cusp(w)) {
        continue;
      }
      const double deg = norm_rad(midpoint_near(u.el, w.el)) * kRadToDeg;
      // the precision follows his u&
      std::snprintf(buf, sizeof(buf), u.pair_fine ? "%6.2f %s-%s" : "%5.1f  %s-%s", deg, u.tag.c_str(), w.tag.c_str());
      out.push_back({deg, buf});
    }
  }
  return out;
}

int grad_unit(const ChartSettings& s) {
  // his bb& = 14, 10 in the hrg mode, np& = 18 + @zusp with extras
  int bb = s.heliocentric ? 10 : 14;
  if (s.extra_bodies) {
    int n = 0;
    for (int i = 1; i <= 22; ++i) {
      if (s.nk[static_cast<std::size_t>(i)] > 0) {
        ++n;
      }
    }
    bb = 18 + n;
  }
  // his h& = INT(180 / bb&), doubled with hrg!
  const int h = 180 / bb;
  return s.heliocentric ? 2 * h : h;
}

}  // namespace horcom
