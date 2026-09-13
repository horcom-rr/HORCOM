// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/riseset.hpp"

#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/calendar.hpp"
#include "horcom/time/delta_t.hpp"
#include "horcom/time/sidereal.hpp"

namespace horcom {

namespace {

//RR 360.985647, the sky's turn per day in degrees
constexpr double kTurnPerDay = 360.985647;

// keeps a day fraction inside its day, the original mtrim
double mtrim(double m) {
  while (m < 0.0) {
    m += 1.0;
  }
  while (m > 1.0) {
    m -= 1.0;
  }
  return m;
}

struct DayPos {
  bool ok = false;
  double ar = 0.0;  // degrees
  double de = 0.0;  // degrees
  double h0 = 0.0;  // the standard altitude, degrees
};

// one body at a moment through the full chart pipeline, the standard
// altitude of the moon rides its own parallax like auf_pl
DayPos position_at(double jd_ut, int slot, bool true_position, const SearchContext& ctx) {
  DayPos out;
  ChartInput in = ctx.base;
  in.date_ut = calendar_date(jd_ut, ctx.settings.calendar);
  const Chart c = compute_chart(in, ctx.settings, *ctx.vsop, *ctx.eph);
  const BodyState& b = c.b[static_cast<std::size_t>(slot)];
  if (!c.ok || !b.present || !b.valid) {
    return out;
  }
  out.ok = true;
  out.ar = norm_rad(b.ar) * kRadToDeg;
  out.de = b.de * kRadToDeg;
  if (true_position) {
    out.h0 = 0.0;
  } else if (slot == body::kSun) {
    //RR GRAD
    out.h0 = -0.8333333;
  } else if (slot == body::kMoon) {
    // the moon's depth follows its parallax of the moment
    const double par = std::asin(1.0 / c.moon.r);
    out.h0 = 0.7275 * par * kRadToDeg - 0.56666666;
  } else {
    out.h0 = -0.566667;
  }
  return out;
}

}  // namespace

// ported from HORCOM auf_unt with auf_unt2 and auf_2
RiseSet rise_transit_set(double jd_day_ut, int slot, bool true_position, const SearchContext& ctx) {
  RiseSet out;
  const double jde = std::floor(jd_day_ut - 0.5) + 0.5;
  const double lat = ctx.base.lat_deg;
  const double lon = ctx.base.lon_deg_east;
  const DayPos p1 = position_at(jde - 1.0, slot, true_position, ctx);
  const DayPos p2 = position_at(jde, slot, true_position, ctx);
  const DayPos p3 = position_at(jde + 1.0, slot, true_position, ctx);
  if (!p1.ok || !p2.ok || !p3.ok) {
    return out;
  }
  double a1 = p1.ar;
  double a2 = p2.ar;
  double a3 = p3.ar;
  // the three right ascensions onto one branch
  {
    double w1 = a1 * kDegToRad;
    double w2 = a3 * kDegToRad;
    double w3 = a2 * kDegToRad;
    vergl2(w1, w2, w3);
    a1 = w1 * kRadToDeg;
    a2 = w3 * kRadToDeg;
    a3 = w2 * kRadToDeg;
  }
  const double d1 = p1.de;
  const double d2 = p2.de;
  const double d3 = p3.de;
  double h0 = p2.h0;

  //RR 1.NÄH
  const double te0 = gmst0_hours(jde) * kDegPerHour;
  const double arg = (std::sin(h0 * kDegToRad) - std::sin(lat * kDegToRad) * std::sin(d2 * kDegToRad)) /
                     (std::cos(lat * kDegToRad) * std::cos(d2 * kDegToRad));
  if (std::abs(arg) > 1.0) {
    //RR AUßER BEREICH !
    out.circumpolar = true;
    return out;
  }
  double hg0 = std::acos(arg) * kRadToDeg;
  while (hg0 < 0.0 && hg0 > -180.0) {
    hg0 += 180.0;
  }
  while (hg0 > 180.0 && hg0 < 360.0) {
    hg0 -= 180.0;
  }
  double m0 = mtrim((a2 - lon - te0) / 360.0);
  double m1 = mtrim(m0 - hg0 / 360.0);
  double m2 = mtrim(m0 + hg0 / 360.0);
  const double delt = delta_t_minutes(jde);

  // one refinement of the interpolation of auf_unt2
  const auto refine = [&](int kind, double m, double te) {
    const double n = m + delt * 0.0006944444444;
    double a = a2 - a1;
    double b = a3 - a2;
    double c = b - a;
    const double ar = norm_deg(a2 + (n / 2.0) * (a + b + n * c));
    a = d2 - d1;
    b = d3 - d2;
    c = b - a;
    const double de = d2 + (n / 2.0) * (a + b + n * c);
    double hg = te + lon - ar;
    while (hg < -180.0) {
      hg += 360.0;
    }
    while (hg > 180.0) {
      hg -= 360.0;
    }
    const double h = std::asin(std::sin(lat * kDegToRad) * std::sin(de * kDegToRad) +
                               std::cos(lat * kDegToRad) * std::cos(hg * kDegToRad) * std::cos(de * kDegToRad)) *
                     kRadToDeg;
    double dm = 0.0;
    if (kind == 2) {
      dm = -hg / 360.0;
    } else {
      dm = (h - h0) / (360.0 * std::cos(de * kDegToRad) * std::cos(lat * kDegToRad) * std::sin(hg * kDegToRad));
    }
    return mtrim(m + dm);
  };

  // the outer loop of the original, the moon's depth refreshes with
  // every recomputed moment
  const auto solve = [&](int kind, double m) {
    for (int i = 0; i < 40; ++i) {
      const double ms = m;
      m = refine(kind, m, te0 + kTurnPerDay * m);
      if (slot == body::kMoon && !true_position) {
        const DayPos now = position_at(jde + m, slot, true_position, ctx);
        if (now.ok) {
          h0 = now.h0;
        }
      }
      if (std::abs(ms - m) < 0.00001) {
        return m;
      }
    }
    return m;
  };

  m1 = solve(1, m1);
  m0 = solve(2, m0);
  m2 = solve(3, m2);
  out.ok = true;
  out.jd_rise_ut = jde + m1;
  out.jd_transit_ut = jde + m0;
  out.jd_set_ut = jde + m2;
  return out;
}

}  // namespace horcom
