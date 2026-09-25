// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/harmonics.hpp"

#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/composite.hpp"
#include "horcom/chart/houses.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/statist_eval.hpp"

namespace horcom {

namespace {

// his p = PI / 6, one sign of thirty degrees in radians
constexpr double kSignRad = kPi / 6.0;

// the CASE lists of multi11 to harm21 name the extras n1, n2, n4 to n8
// and n17 to n22. Transpluto, n3, and the Hamburg factors n9 to n16 fall
// to DEFAULT and go dark in every directed chart
bool multi_dark(int slot) {
  return slot == body::kTranspluto || (slot >= body::kCupido && slot <= body::kPoseidon);
}

}  // namespace

// ported from HORCOM harm21, a901_m and mc_armcb CASE 7
Chart harmonic_chart(const Chart& base, double n, HarmonicHouses mode, HouseSystem system, double lat_deg) {
  Chart h = base;
  // harm21 lists CASE 1 TO 11, the fixed point on aa& = 0 falls to
  // DEFAULT and stays empty
  h.b[body::kFixpunkt].present = false;
  // planets and the north node multiply onto the order
  for (int slot = body::kSun; slot <= body::kNodeAsc; ++slot) {
    BodyState& b = h.b[static_cast<std::size_t>(slot)];
    if (b.present && b.valid) {
      b.el = norm_rad(n * base.b[static_cast<std::size_t>(slot)].el);
    }
  }
  // the south node follows the transformed north node, not its own turn
  if (h.b[body::kNodeDesc].present) {
    h.b[body::kNodeDesc].el = norm_rad(h.b[body::kNodeAsc].el + kPi);
  }
  for (int slot = body::kApogee; slot < body::kSlotCount; ++slot) {
    BodyState& b = h.b[static_cast<std::size_t>(slot)];
    if (!b.present) {
      continue;
    }
    if (multi_dark(slot)) {
      b.present = false;
      continue;
    }
    if (b.valid) {
      b.el = norm_rad(n * base.b[static_cast<std::size_t>(slot)].el);
    }
  }

  switch (mode) {
    case HarmonicHouses::kLikeBodies:
      // his answer HÄUSER wie PLANETEN BEHANDELN
      for (int k = 1; k <= 12; ++k) {
        h.houses.cusp[static_cast<std::size_t>(k)] = norm_rad(n * base.houses.cusp[static_cast<std::size_t>(k)]);
      }
      h.houses.cusp[13] = h.houses.cusp[1];
      h.houses.angles.ac = h.houses.cusp[1];
      h.houses.angles.mc = h.houses.cusp[10];
      break;
    case HarmonicHouses::kFromNewMc: {
      //RR neues MC
      // mc_armcb reads fz(1,ze,10), the tenth cusp, which the equal
      // systems keep apart from the true midheaven
      const double mc = norm_rad(n * base.houses.cusp[10]);
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

  // the a901_m rebuild of the Part of Fortune stays commented out in
  // harm21, the point multiplies onto the order like every other extra
  return h;
}

namespace {

//RR auf Zeichen bezogen
double within_sign(double el) {
  return el - kSignRad * std::trunc(el / kSignRad);
}

// the anchor ladders of multi0ost1 and multi0west1, day and night
// rulership sign starts in units of thirty degrees, minus one leaves a
// slot dark
int zero_anchor(int slot, bool east) {
  switch (slot) {
    case body::kSun: return 4;
    case body::kMoon:
    case body::kNodeAsc: return 3;
    case body::kMercury: return east ? 2 : 5;
    case body::kVenus: return east ? 1 : 6;
    case body::kMars: return east ? 0 : 7;
    case body::kJupiter: return east ? 11 : 8;
    case body::kSaturn: return east ? 10 : 9;
    case body::kUranus: return 10;
    case body::kNeptune: return 11;
    case body::kPluto: return 7;
    case body::kChiron: return 5;
    default: return slot >= body::kApogee ? 0 : -1;
  }
}

// the reference point of mc_armcb1, the ruler reads the classic table
// like his forced alt switch
double multi_reference(const Chart& base, const MultiReference& ref) {
  switch (ref.kind) {
    case MultiReference::Kind::kCusp:
      return base.houses.cusp[static_cast<std::size_t>(ref.house)];
    case MultiReference::Kind::kRuler: {
      // alt! = -1, mc_armcb1 forces the classic table
      const int kp = sign_ruler(base.houses.cusp[static_cast<std::size_t>(ref.house)], true);
      return kp > 0 ? base.b[static_cast<std::size_t>(kp)].el : 0.0;
    }
    case MultiReference::Kind::kSignStart:
      return kDegToRad * (ref.sign - 1) * kDegPerSign;
    default:
      if (ref.body == body::kAscendant) {
        return base.houses.cusp[1];
      }
      if (ref.body == body::kMc) {
        return base.houses.cusp[10];
      }
      // mc_armcb1 has no CASE for the south node MONDKNOTEN S that his
      // BEZUGS-FAKTOR box offers, e stayed zero and the direction ran
      // from 0 Aries. The port reads the south node like every other body
      return base.b[static_cast<std::size_t>(ref.body)].el;
  }
}

// one longitude through the mode formula, dark anchors return negative
double multi_direct(MultiMode mode, double el, double lja, double e, int slot) {
  switch (mode) {
    case MultiMode::kMulti1:
      //RR multi
      return norm_rad(el + lja * within_sign(el));
    case MultiMode::kMulti2:
      return norm_rad(el + lja * el);
    case MultiMode::kMulti3:
      return norm_rad(e + lja * within_sign(el));
    case MultiMode::kZeroEast:
    case MultiMode::kZeroWest: {
      const int anchor = zero_anchor(slot, mode == MultiMode::kZeroEast);
      if (anchor < 0) {
        return -1.0;
      }
      return norm_rad(anchor * kSignRad + lja * within_sign(el));
    }
    case MultiMode::kArc: {
      double w1 = e;
      double w2 = el;
      vergl1(w1, w2);
      return norm_rad(e + lja * std::abs(w2 - w1));
    }
  }
  return -1.0;
}

}  // namespace

// ported from HORCOM multi11, multi21, multi31, multi0ost1,
// multi0west1, multiarc1 and mc_armcb
Chart multi_chart(const Chart& base, MultiMode mode, double lja, const MultiReference& ref, HarmonicHouses houses, HouseSystem system, double lat_deg) {
  Chart d = base;
  const double e = multi_reference(base, ref);
  // the loops run from aa&, the fixed point directs like a planet, the
  // zero point modes leave it to DEFAULT
  for (int slot = body::kFixpunkt; slot <= body::kNodeAsc; ++slot) {
    BodyState& b = d.b[static_cast<std::size_t>(slot)];
    if (b.present && b.valid) {
      const double w = multi_direct(mode, base.b[static_cast<std::size_t>(slot)].el, lja, e, slot);
      if (w < 0.0) {
        b.present = false;
      } else {
        b.el = w;
      }
    }
  }
  if (d.b[body::kNodeDesc].present) {
    if (d.b[body::kNodeAsc].present) {
      d.b[body::kNodeDesc].el = norm_rad(d.b[body::kNodeAsc].el + kPi);
    } else {
      d.b[body::kNodeDesc].present = false;
    }
  }
  for (int slot = body::kApogee; slot < body::kSlotCount; ++slot) {
    BodyState& b = d.b[static_cast<std::size_t>(slot)];
    if (!b.present) {
      continue;
    }
    if (multi_dark(slot)) {
      b.present = false;
      continue;
    }
    if (b.valid) {
      const double w = multi_direct(mode, base.b[static_cast<std::size_t>(slot)].el, lja, e, slot);
      if (w < 0.0) {
        b.present = false;
      } else {
        b.el = w;
      }
    }
  }

  if (houses == HarmonicHouses::kFromNewMc) {
    //RR neues MC
    // mc_armcb reads fz(1,ze,10), the tenth cusp, which the equal
    // systems keep apart from the true midheaven
    const double mc0 = base.houses.cusp[10];
    double mc = 0.0;
    switch (mode) {
      case MultiMode::kMulti2:
        mc = norm_rad(mc0 + lja * mc0);
        break;
      case MultiMode::kMulti3:
        mc = norm_rad(e + lja * within_sign(mc0));
        break;
      case MultiMode::kArc:
        mc = norm_rad(e + lja * std::abs(mc0 - e));
        break;
      default:
        mc = norm_rad(mc0 + lja * within_sign(mc0));
        break;
    }
    const double z = std::sin(mc) * std::cos(base.ekls0);
    const double armcb = atn(z, std::cos(mc));
    d.houses = compute_houses(system, armcb, lat_deg, base.ekls0);
  } else {
    //RR Nur Eckpunkte
    for (int k = 1; k <= 12; ++k) {
      d.houses.cusp[static_cast<std::size_t>(k)] = 0.0;
    }
    switch (mode) {
      case MultiMode::kZeroEast:
      case MultiMode::kZeroWest:
        // the zero point modes hand out only AC and MC, the original
        // left cusps four and seven stale, the port clears them
        d.houses.cusp[1] = norm_rad(lja * within_sign(base.houses.cusp[1]));
        d.houses.cusp[10] = norm_rad(9.0 * kSignRad + lja * within_sign(base.houses.cusp[10]));
        break;
      case MultiMode::kArc:
        // mc_armcb CASE 6 differences without the branch reconciler
        for (int k : {1, 4, 7, 10}) {
          const double f = base.houses.cusp[static_cast<std::size_t>(k)];
          d.houses.cusp[static_cast<std::size_t>(k)] = norm_rad(e + lja * std::abs(f - e));
        }
        break;
      case MultiMode::kMulti2:
        for (int k : {1, 4, 7, 10}) {
          const double f = base.houses.cusp[static_cast<std::size_t>(k)];
          d.houses.cusp[static_cast<std::size_t>(k)] = norm_rad(f + lja * f);
        }
        break;
      case MultiMode::kMulti3:
        for (int k : {1, 4, 7, 10}) {
          const double f = base.houses.cusp[static_cast<std::size_t>(k)];
          d.houses.cusp[static_cast<std::size_t>(k)] = norm_rad(e + lja * within_sign(f));
        }
        break;
      default:
        for (int k : {1, 4, 7, 10}) {
          const double f = base.houses.cusp[static_cast<std::size_t>(k)];
          d.houses.cusp[static_cast<std::size_t>(k)] = norm_rad(f + lja * within_sign(f));
        }
        break;
    }
    d.houses.cusp[13] = d.houses.cusp[1];
    d.houses.angles.ac = d.houses.cusp[1];
    d.houses.angles.mc = d.houses.cusp[10];
  }
  if (d.b[body::kAscendant].present) {
    d.b[body::kAscendant].el = d.houses.cusp[1];
  }
  if (d.b[body::kMc].present) {
    d.b[body::kMc].el = d.houses.cusp[10];
  }

  //RR GL
  // a901_m runs only under the new MC answer and never in the zero point
  // modes. The formula takes the directed lights and the new first house.
  // The day test reads the radix, the sect of the birth like a901 of the
  // radix itself. His ta_na read the working arrays, multi11 had zeroed
  // pl(13) and so always chose the night formula, the other modes set
  // the radix Sun against the directed ascendant
  BodyState& gl = d.b[body::kFortune];
  const bool zero_point = mode == MultiMode::kZeroEast || mode == MultiMode::kZeroWest;
  if (houses == HarmonicHouses::kFromNewMc && !zero_point && gl.present) {
    const int day = ta_na(base.b[body::kAscendant].el, base.b[body::kSun].el);
    const double so = d.b[body::kSun].el;
    const double mo = d.b[body::kMoon].el;
    gl.el = day == 1 ? norm_rad(d.houses.cusp[1] + mo - so) : norm_rad(d.houses.cusp[1] - mo + so);
    gl.eb = 0.0;
  }
  return d;
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

// ported from halbsm. His loop over the sign boundaries started at aa&,
// one without a fixed point, so 0 degrees Aries never matched and only
// the 180 degree end of that axis did, the port walks all twelve
std::vector<MultiMidpoint> multi_midpoints(const Chart& radix, const Chart& multi, double orb) {
  std::vector<MultiMidpoint> out;
  // dd = 0.2 * orb * pu
  constexpr double kFifthDegree = 0.2;
  const double dd = kFifthDegree * orb * kDegToRad;
  const auto live = [&multi](int slot) {
    const BodyState& b = multi.b[static_cast<std::size_t>(slot)];
    return b.present && b.valid;
  };
  // CASE 1 TO 11,13,14 and the extras for u&, 1 TO 14 for w&, never
  // Transpluto
  const auto pair_slot = [&](int slot, bool first) {
    if (slot == body::kTranspluto || (slot > body::kMc && slot < body::kApogee)) {
      return false;
    }
    if (first && slot == body::kNodeDesc) {
      return false;
    }
    return live(slot);
  };
  const auto near = [dd](double target, double ph) {
    double w1 = norm_rad(target - dd);
    double w2 = norm_rad(target + dd);
    double w3 = ph;
    vergl2(w1, w2, w3);
    return w1 < w3 && w3 < w2;
  };
  for (int u = 1; u < body::kSlotCount; ++u) {
    if (!pair_slot(u, true)) {
      continue;
    }
    for (int w = u + 1; w < body::kSlotCount; ++w) {
      if (!pair_slot(w, false)) {
        continue;
      }
      // ph = @halbsmin(p1,p2)
      const double ph = midpoint_near(multi.b[static_cast<std::size_t>(u)].el, multi.b[static_cast<std::size_t>(w)].el);
      for (int t = 0; t <= 11; ++t) {
        if (near(t * kSignRad, ph)) {
          const int lower = t > 5 ? t + 1 - 6 : t + 1;
          out.push_back({u, w, MultiMidpoint::Target::kSignAxis, lower, lower + 6});
        }
      }
      if (!radix.houses.ok) {
        continue;
      }
      for (int t = 1; t <= 12; ++t) {
        const int lower = t > 6 ? t - 6 : t;
        if (near(radix.houses.cusp[static_cast<std::size_t>(t)], ph)) {
          out.push_back({u, w, MultiMidpoint::Target::kRadixCusp, lower, lower + 6});
        }
        // CASE 1,10, the directed AC and MC
        if ((t == 1 || t == 10) && multi.houses.ok && near(multi.houses.cusp[static_cast<std::size_t>(t)], ph)) {
          out.push_back({u, w, MultiMidpoint::Target::kMultiAngle, lower, lower + 6});
        }
      }
    }
  }
  return out;
}

}  // namespace horcom
