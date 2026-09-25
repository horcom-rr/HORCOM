// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/rectification.hpp"

#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/houses.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/calendar.hpp"

namespace horcom {

namespace {

// the damping of the korr1 walk, his g = 0.3 below the discarded g = 1
constexpr double kDamping = 0.3;
// his walk gave up after ten seconds of the GFA interpreter, far more
// steps than any settling walk needs
constexpr int kMaxWalkSteps = 100000;
// the repeated plant searches of the light modes
constexpr int kMaxLightPasses = 8;
//RR !0.49"
constexpr double kLightTolerance = 2.424e-6;
// the search windows after the seed, days
constexpr double kSunWindow = 32.0;
constexpr double kSunRetryWindow = 35.0;
constexpr double kMoonWindow = 5.0;
constexpr double kMoonRetryWindow = 6.0;
// his IF ABS(ab1 - ab) > 2.5 && zw!, radians
constexpr double kOppositeJump = 2.5;

// ported from korr10, the ascendant of a sidereal angle
double ascendant_at(double ab, double lat_deg, double ekls) {
  const double z = std::cos(ab);
  const double n = -(std::sin(ekls) * std::tan(lat_deg * kDegToRad) + std::cos(ekls) * std::sin(ab));
  return norm_rad(atn(z, n));
}

// ported from korr1, the damped walk of the sidereal angle until the
// ascendant or the cusp stands on the wanted degree
CorrectionResult walk_angle(const Chart& radix, const CorrectionRequest& q, const SearchContext& ctx) {
  CorrectionResult out;
  const bool cusp = q.target == CorrectionTarget::kCusp;
  const double lat = ctx.base.lat_deg;
  const double ekls = radix.ekls0;
  const double wh = norm_rad(q.value);
  bool refused = false;
  const auto angle_at = [&](double ab) {
    if (!cusp) {
      return ascendant_at(ab, lat, ekls);
    }
    //RR Nur PLACIDUS !
    const Houses h = compute_houses(HouseSystem::kPlacidus, ab, lat, ekls);
    refused = refused || !h.ok;
    return h.cusp[static_cast<std::size_t>(q.cusp)];
  };
  // his ab = FN nb(wh + (10 - h&) * PI / 6), otherwise ab = pu * armc
  double ab = cusp ? norm_rad(wh + (10 - q.cusp) * kPi / 6.0) : radix.armc_deg * kDegToRad;
  const double ab1 = ab;
  double wb = angle_at(ab);
  int steps = 0;
  while (std::abs(fold_rad(wh - wb)) > kCorrectionTolerance) {
    if (refused) {
      return out;
    }
    ab = norm_rad(ab + kDamping * fold_rad(wh - wb));
    wb = angle_at(ab);
    if (++steps > kMaxWalkSteps) {
      // his Spanne zu groß box
      out.span_too_large = true;
      return out;
    }
  }
  if (refused) {
    return out;
  }
  // his test compared the two angles unfolded, a walk from 2 degrees to
  // 355 degrees counted as a jump of 353 degrees and turned the result
  // twelve hours away
  if (cusp && std::abs(fold_rad(ab1 - ab)) > kOppositeJump) {
    ab = norm_rad(ab + kPi);
  }
  out.ok = true;
  out.jd_ut = moment_for_armc(radix, ab, ctx.base.lon_deg_east);
  return out;
}

// the Sonne branch of korr with the year repair around plant
CorrectionResult walk_sun(const Chart& radix, double pz, const SearchContext& ctx) {
  CorrectionResult out;
  const double jdx = radix.jd_ut;
  const double tja = radix.ta.tropical_year_days;
  // his jd = jdx + tja * (FN nb(el(1)) - pz) / pv2
  double jd = jdx + tja * fold_rad(radix.b[body::kSun].el - pz) / kTwoPi;
  double jdz = jd + kSunWindow;
  for (int pass = 0; pass < kMaxLightPasses; ++pass) {
    const LongitudeCrossing hit = find_longitude_backward(jdz, body::kSun, pz, ctx);
    if (!hit.ok) {
      return out;
    }
    jd = hit.jd_ut;
    const BodyLongitude now = body_longitude(jd, body::kSun, ctx);
    if (!now.valid) {
      return out;
    }
    if (std::abs(fold_rad(now.el - pz)) > kLightTolerance) {
      jdz = jd + kSunRetryWindow;
      continue;
    }
    // his IF ja > ja1 && ABS(jd - jdx) > 0.9 * tja : SUB jd,tja, and the
    // mirror for the year before. His test let a target some 37 degrees
    // ahead slip a year early, the crossing nearest the birth is meant
    if (jd - jdx > 0.5 * tja) {
      jd -= tja;
      jdz = jd + kSunRetryWindow;
      continue;
    }
    if (jdx - jd > 0.5 * tja) {
      jd += tja;
      jdz = jd + kSunRetryWindow;
      continue;
    }
    out.ok = true;
    out.jd_ut = jd;
    return out;
  }
  return out;
}

// the Mond branch of korr, the seed from the mean motion near the birth
CorrectionResult walk_moon(const Chart& radix, double pz, const SearchContext& ctx) {
  CorrectionResult out;
  const double jdx = radix.jd_ut;
  // his seed jd = jdx + tmo * (el(2) - pz) / pv2, the difference folded
  // so a target across zero Aries stays in the lunation of the birth
  double jd = jdx + kTropicalMonthDays * fold_rad(radix.b[body::kMoon].el - pz) / kTwoPi;
  const double jdm = jd;
  double jdz = jd + kMoonWindow;
  for (int pass = 0; pass < kMaxLightPasses; ++pass) {
    const LongitudeCrossing hit = find_longitude_backward(jdz, body::kMoon, pz, ctx);
    if (!hit.ok) {
      return out;
    }
    jd = hit.jd_ut;
    const BodyLongitude now = body_longitude(jd, body::kMoon, ctx);
    if (!now.valid) {
      return out;
    }
    if (ctx.settings.topocentric_parallax && std::abs(fold_rad(now.el - pz)) > kLightTolerance) {
      jd = jdm;
      jdz = jd + kMoonRetryWindow;
      continue;
    }
    // his repairs compared the crossing with the search start, jd - jdz
    // > tmo, which the backward search never meets. His seed stands on
    // the far side of the birth, so a target more than about 33 degrees
    // ahead came out a lunation early. The crossing nearest the birth is
    // meant, like the year repair of the Sun
    if (jd - jdx > 0.5 * kTropicalMonthDays) {
      jd -= kTropicalMonthDays;
      jdz = jd + kMoonRetryWindow;
      continue;
    }
    if (jdx - jd > 0.5 * kTropicalMonthDays) {
      jd += kTropicalMonthDays;
      jdz = jd + kMoonRetryWindow;
      continue;
    }
    out.ok = true;
    out.jd_ut = jd;
    return out;
  }
  return out;
}

}  // namespace

// ported from korr21
double moment_for_armc(const Chart& radix, double armcb, double lon_deg_east) {
  const double jd0 = std::floor(radix.jd_ut + 0.5) - 0.5;
  const double armc_deg = norm_deg(armcb * kRadToDeg);
  // his w = FN nh((armc / 15) - h0 - gl / 15) / 1.002737908
  const double hours = norm_hours(armc_deg / kDegPerHour - radix.h0 - lon_deg_east / kDegPerHour) /
                       kSolarToSiderealRate;
  // his korr21 kept the UT date of the birth, a birth shortly before
  // midnight UT then found the wanted angle almost a day early. The same
  // ARMC returns every sidereal day, the one nearest the birth is meant
  constexpr double kSiderealDayDays = 1.0 / kSolarToSiderealRate;
  double jd = jd0 + hours / kHoursPerDay;
  if (jd - radix.jd_ut > 0.5 * kSiderealDayDays) {
    jd -= kSiderealDayDays;
  } else if (radix.jd_ut - jd > 0.5 * kSiderealDayDays) {
    jd += kSiderealDayDays;
  }
  return jd;
}

// ported from korr
CorrectionResult correct_birth_time(const Chart& radix, const CorrectionRequest& q, const SearchContext& ctx) {
  switch (q.target) {
    case CorrectionTarget::kSiderealTime: {
      // his armcb = FN nb(pu * 15 * (ho + mi / 60))
      const double armcb = norm_rad(q.value * kDegPerHour * kDegToRad);
      return {true, moment_for_armc(radix, armcb, ctx.base.lon_deg_east), false};
    }
    case CorrectionTarget::kMc: {
      // ported from korr2
      const double z = std::cos(radix.ekls0) * std::sin(q.value);
      const double n = std::cos(q.value);
      return {true, moment_for_armc(radix, atn(z, n), ctx.base.lon_deg_east), false};
    }
    case CorrectionTarget::kAc:
    case CorrectionTarget::kCusp:
      return walk_angle(radix, q, ctx);
    case CorrectionTarget::kSun:
      return walk_sun(radix, norm_rad(q.value), ctx);
    case CorrectionTarget::kMoon:
      return walk_moon(radix, norm_rad(q.value), ctx);
  }
  return {};
}

}  // namespace horcom
