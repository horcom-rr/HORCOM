// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/riseset.hpp"

#include <cmath>

#include "horcom/chart/bodies.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/calendar.hpp"
#include "horcom/time/sidereal.hpp"

namespace horcom {

namespace {

// his 360.985647, the sky's turn per day in degrees
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
  double el = 0.0;  // the display values of auf_unt3, radians
  double eb = 0.0;
  double ar_rad = 0.0;
  double de_rad = 0.0;
};

// his loop closes at 0.00001 of a day and gives up after thirty seconds
// on his machine, the port counts iterations instead
constexpr double kClosed = 0.00001;
constexpr int kMaxIterations = 40;

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
  out.el = b.el;
  out.eb = b.eb;
  out.ar_rad = b.ar;
  out.de_rad = b.de;
  // auf_pl sets the moon's depth on every call whatever the WAHR or
  // SCHEINBAR answer, the other bodies keep the value of the answer
  if (slot == body::kMoon) {
    // his hh0 = 0.7275 * pm(2) * up - 0.56666666
    out.h0 = 0.7275 * c.moon.parallax * kRadToDeg - 0.56666666;
  } else if (true_position) {
    out.h0 = 0.0;
  } else if (slot == body::kSun) {
    //RR GRAD
    out.h0 = -0.8333333;
  } else {
    out.h0 = -0.566667;
  }
  return out;
}

}  // namespace

// ported from HORCOM auf_unt with auf_unt2 and auf_2
RiseSet rise_transit_set(double jd_day_ut, int slot, bool true_position, const SearchContext& in_ctx) {
  RiseSet out;
  // his par = 2
  //RR ohne Par.
  // auf_unt forces its own modes, geocentric positions whatever the
  // panel says, hrg! = 0, and appa& = 1 apparent or 3 true, the
  // standard altitude carries the parallax instead
  SearchContext ctx = in_ctx;
  ctx.settings.topocentric_parallax = false;
  ctx.settings.heliocentric = false;
  ctx.settings.apparent = true_position ? ApparentMode::kTrue : ApparentMode::kLightTime;
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
    // his AUßER BEREICH ! box
    out.circumpolar = true;
    return out;
  }
  double hg0 = std::acos(arg) * kRadToDeg;
  while (hg0 < 0.0 && hg0 > -180.0) {
    hg0 += 180.0;
  }
  while (hg0 > 180.0 && hg0 < kDegPerCircle) {
    hg0 -= 180.0;
  }
  double m0 = mtrim((a2 - lon - te0) / kDegPerCircle);
  double m1 = mtrim(m0 - hg0 / kDegPerCircle);
  double m2 = mtrim(m0 + hg0 / kDegPerCircle);

  // one refinement of the interpolation of auf_unt2. His n = m + delt
  // follows Meeus for positions tabulated at 0h TD, the three positions
  // above stand at 0h UT already, the chain converted them. The port
  // interpolates at m itself, his second delt moved the moon's clock by
  // seconds today and by hours in antiquity
  const auto refine = [&](int kind, double m, double te) {
    const double n = m;
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
      hg += kDegPerCircle;
    }
    while (hg > 180.0) {
      hg -= kDegPerCircle;
    }
    const double h = std::asin(std::sin(lat * kDegToRad) * std::sin(de * kDegToRad) +
                               std::cos(lat * kDegToRad) * std::cos(hg * kDegToRad) * std::cos(de * kDegToRad)) *
                     kRadToDeg;
    double dm = 0.0;
    if (kind == 2) {
      dm = -hg / kDegPerCircle;
    } else {
      dm = (h - h0) / (kDegPerCircle * std::cos(de * kDegToRad) * std::cos(lat * kDegToRad) * std::sin(hg * kDegToRad));
    }
    return mtrim(m + dm);
  };

  // the outer loop of the original with auf_2 after every step, the
  // moon's depth refreshes with every recomputed moment and the last
  // position is the one his auf_unt3 block prints
  const auto solve = [&](int kind, double m) {
    RiseSetMoment at;
    for (int i = 0; i < kMaxIterations; ++i) {
      const double ms = m;
      m = refine(kind, m, te0 + kTurnPerDay * m);
      const DayPos now = position_at(jde + m, slot, true_position, ctx);
      if (now.ok) {
        if (slot == body::kMoon) {
          h0 = now.h0;
        }
        at.el = now.el;
        at.eb = now.eb;
        at.ar = now.ar_rad;
        at.de = now.de_rad;
      }
      if (std::abs(ms - m) < kClosed) {
        at.ok = now.ok;
        break;
      }
    }
    at.jd_ut = jde + m;
    at.gst_deg = norm_deg(te0 + kTurnPerDay * m);
    return at;
  };

  out.ok = true;
  out.rise = solve(1, m1);
  out.transit = solve(2, m0);
  out.set = solve(3, m2);
  return out;
}

}  // namespace horcom
