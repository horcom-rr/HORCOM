// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/harmonics.hpp"

#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/houses.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

// ported from HORCOM harm21, a901_m and mc_armcb CASE 7
Chart harmonic_chart(const Chart& base, double n, HarmonicHouses mode, HouseSystem system, double lat_deg) {
  Chart h = base;
  // planets and the north node multiply onto the order
  for (int slot = 1; slot <= 11; ++slot) {
    BodyState& b = h.b[static_cast<std::size_t>(slot)];
    if (b.present && b.valid) {
      b.el = norm_rad(n * base.b[static_cast<std::size_t>(slot)].el);
    }
  }
  // the south node follows the transformed north node, not its own turn
  if (h.b[body::kNodeDesc].present) {
    h.b[body::kNodeDesc].el = norm_rad(h.b[body::kNodeAsc].el + kPi);
  }
  for (int slot = 19; slot < body::kSlotCount; ++slot) {
    BodyState& b = h.b[static_cast<std::size_t>(slot)];
    if (!b.present) {
      continue;
    }
    if (slot == body::kTranspluto) {
      // his CASE list leaves Transpluto out, the slot goes dark
      b.present = false;
      continue;
    }
    if (b.valid) {
      b.el = norm_rad(n * base.b[static_cast<std::size_t>(slot)].el);
    }
  }

  switch (mode) {
    case HarmonicHouses::kLikeBodies:
      //RR HÄUSER wie PLANETEN BEHANDELN
      for (int k = 1; k <= 12; ++k) {
        h.houses.cusp[static_cast<std::size_t>(k)] = norm_rad(n * base.houses.cusp[static_cast<std::size_t>(k)]);
      }
      h.houses.cusp[13] = h.houses.cusp[1];
      h.houses.angles.ac = h.houses.cusp[1];
      h.houses.angles.mc = h.houses.cusp[10];
      break;
    case HarmonicHouses::kFromNewMc: {
      //RR neues MC
      const double mc = norm_rad(n * base.houses.angles.mc);
      const double z = std::sin(mc) * std::cos(base.ekls0);
      const double armcb = atn(z, std::cos(mc));
      h.houses = compute_houses(system, armcb, lat_deg, base.ekls0);
      break;
    }
  }
  if (h.b[body::kAscendant].present) {
    h.b[body::kAscendant].el = h.houses.cusp[1];
  }
  if (h.b[body::kMc].present) {
    h.b[body::kMc].el = h.houses.cusp[10];
  }

  // a901_m rebuilds the Part of Fortune from the harmonic angles, the
  // day test reads the radix like the ambient arrays it used
  BodyState& gl = h.b[body::kFortune];
  if (gl.present) {
    const int day = ta_na(base.b[body::kAscendant].el, base.b[body::kSun].el);
    const double so = h.b[body::kSun].el;
    const double mo = h.b[body::kMoon].el;
    if (day == 1) {
      gl.el = norm_rad(h.houses.cusp[1] + mo - so);
    } else {
      gl.el = norm_rad(h.houses.cusp[1] - mo + so);
    }
    gl.eb = 0.0;
  }
  return h;
}

// ported from HORCOM a12f
Chart dial_chart(const Chart& base, double dop) {
  Chart d = base;
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    BodyState& b = d.b[static_cast<std::size_t>(slot)];
    if (b.present && b.valid) {
      b.el = norm_rad(dop * base.b[static_cast<std::size_t>(slot)].el);
    }
  }
  // only the axes ride along, f(1) to f(10) in steps of three
  for (int k : {1, 4, 7, 10}) {
    d.houses.cusp[static_cast<std::size_t>(k)] = norm_rad(dop * base.houses.cusp[static_cast<std::size_t>(k)]);
  }
  for (int k : {2, 3, 5, 6, 8, 9, 11, 12}) {
    d.houses.cusp[static_cast<std::size_t>(k)] = 0.0;
  }
  d.houses.cusp[13] = d.houses.cusp[1];
  // the rotation of horbeg keeps reading the stored radix ascendant,
  // a12f never touches fz, so the angles stay
  return d;
}

// ported from HORCOM a12f, the pl(13) and pl(14) lines
void dial_display(Chart& c, double dop) {
  for (int slot : {body::kAscendant, body::kMc}) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    if (b.present) {
      b.el = norm_rad(b.el / dop);
    }
  }
}

}  // namespace horcom
