// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/chart.hpp"

#include <cmath>

#include "horcom/chart/corrections.hpp"
#include "horcom/chart/geo.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"
#include "horcom/ephem/elements.hpp"
#include "horcom/ephem/kepler.hpp"
#include "horcom/ephem/pluto_chapront.hpp"
#include "horcom/time/delta_t.hpp"
#include "horcom/time/sidereal.hpp"

namespace horcom {

namespace {

//RR 1h
constexpr double kOneHourDays = 0.0416666666666;

// his symmetric hundredth of a day for numerical rates, the factor 50 is
// one over twice the step and must stay his exact literal
constexpr double kVelStepDays = 0.01;
constexpr double kVelRate = 50.0;

// context of one epoch evaluation shared by the body routines
struct Ctx {
  const ChartSettings& s;
  const VsopTables& vsop;
  const Ephemerides& eph;
  const TimeArguments& ta;
  const SunMoonState& smo;
  double lat_deg;
  double armc_deg;
};

HelioState from_vsop(const VsopTables::Result& v) {
  return {v.l, v.b, v.r, v.lt, v.bt, v.rt};
}

// the correction chain of the original par_ap_ktr. The parallax applies
// to slots 1 through 10 only, like the original SELECT in par.
void par_ap_ktr(const Ctx& c, int slot, BodyState& b, double sun_el, double sun_eb) {
  BodyPosition p{b.el, b.eb, 0.0, 0.0, b.dr, b.tb};
  apparent_position(p, c.s.apparent, sun_el, sun_eb);
  to_equatorial(p, c.smo.ekls);
  if (c.s.topocentric_parallax && slot >= 1 && slot <= 10) {
    parallax(p, c.lat_deg, c.armc_deg, c.smo.ekls);
  }
  b.el = p.el;
  b.eb = p.eb;
  b.ar = p.ar;
  b.de = p.de;
}

// the heliocentric display of hrg, the body keeps its sun centred
// state, the geo conversion and every correction stay out like the
// hel_geo guard and plant1's el from hel
void helio_display(const Ctx& c, const HelioState& h, BodyState& b) {
  b.el = norm_rad(h.l);
  b.eb = h.b;
  b.dr = h.r;
  b.tb = h.lt;
  b.ttb = 0.0;
  const Equatorial eq = ecliptic_to_equatorial(b.el, b.eb, c.smo.ekls);
  b.ar = eq.ra;
  b.de = eq.dec;
}

// the original soko
void compute_sun(const Ctx& c, BodyState& b) {
  const VsopTables::Result e = c.vsop.evaluate(3, c.ta.t11);
  b.present = true;
  b.valid = true;
  b.hel = e.l;
  b.heb = e.b;
  b.r = e.r;
  b.el = norm_rad(e.l + kPi + c.smo.dpsi);
  b.eb = -e.b + c.smo.deps;
  b.tb = e.lt;
  b.dr = e.r;
  par_ap_ktr(c, body::kSun, b, b.el, b.eb);
}

// a classical planet, slots 3 through 9, VSOP plus helio to geo
void compute_planet(const Ctx& c, int slot, const HelioState& earth, double sun_el, double sun_eb, BodyState& b) {
  const int file_index = (slot <= 4) ? slot - 2 : slot - 1;
  const VsopTables::Result v = c.vsop.evaluate(file_index, c.ta.t11);
  const HelioState h = from_vsop(v);
  b.present = true;
  b.valid = true;
  b.hel = h.l;
  b.heb = h.b;
  b.r = h.r;
  if (c.s.heliocentric) {
    helio_display(c, h, b);
    return;
  }
  const GeoResult g = helio_to_geo(h, earth, c.smo.dpsi, c.smo.deps);
  b.el = g.el;
  b.eb = g.eb;
  b.dr = g.dr;
  b.tb = g.tb;
  b.ttb = g.ttb;
  par_ap_ktr(c, slot, b, sun_el, sun_eb);
}

// a body served by one of his ephemeris files, heliocentric spherical
// state of date plus absolute rates
bool eph_helio(const Ctx& c, std::string_view name, double jd, HelioState& out) {
  const EphBodyInfo* info = eph_body(name);
  const EphFile* file = c.eph.get(name);
  if (info == nullptr || file == nullptr) {
    return false;
  }
  const EphFile::Sample smp = file->evaluate(jd, info->fplanet, info->frame);
  if (!smp.in_range) {
    return false;
  }
  if (info->frame == EphFrame::kEquatorialJ2000) {
    const Ecliptic ec = equatorial_to_ecliptic(smp.lon, smp.lat, c.smo.ekls);
    out = {ec.lon, ec.lat, smp.r, smp.lont, smp.latt, smp.rt};
  } else {
    out = {smp.lon, smp.lat, smp.r, smp.lont, smp.latt, smp.rt};
  }
  return true;
}

// Pluto outside his integrated span, heliocentric state from the
// Chapront theory with rates from a symmetric hundredth of a day
HelioState chapront_helio(const Ctx& c, double jd) {
  const ChaprontPluto now = pluto_chapront(c.ta, c.smo.ekls);
  const ChaprontPluto before = pluto_chapront(time_arguments(jd - kVelStepDays), c.smo.ekls);
  const ChaprontPluto after = pluto_chapront(time_arguments(jd + kVelStepDays), c.smo.ekls);
  double w1 = after.hel;
  double w2 = before.hel;
  verv(w1, w2);
  return {now.hel, now.heb, now.r, (w1 - w2) * kVelRate, (after.heb - before.heb) * kVelRate, (after.r - before.r) * kVelRate};
}

void compute_eph_body(const Ctx& c, int slot, std::string_view name, double jd, const HelioState& earth, double sun_el, double sun_eb, BodyState& b) {
  b.present = true;
  HelioState h;
  if (!eph_helio(c, name, jd, h)) {
    //RR jdplanetex
    b.valid = false;
    return;
  }
  b.valid = true;
  b.hel = h.l;
  b.heb = h.b;
  b.r = h.r;
  if (c.s.heliocentric) {
    helio_display(c, h, b);
    return;
  }
  const GeoResult g = helio_to_geo(h, earth, c.smo.dpsi, c.smo.deps);
  b.el = g.el;
  b.eb = g.eb;
  b.dr = g.dr;
  b.tb = g.tb;
  b.ttb = g.ttb;
  par_ap_ktr(c, slot, b, sun_el, sun_eb);
}

// Transpluto and the Hamburg factors on their Kepler orbits, rates from a
// symmetric hundredth of a day like the original pl_vel
void compute_kepler_body(const Ctx& c, int slot, int nk_index, double jd, const HelioState& earth, double sun_el, double sun_eb, BodyState& b) {
  const auto orbit_at = [&](const TimeArguments& t) {
    return (nk_index == 3) ? transpluto_elements(t) : uranian_elements(nk_index, t);
  };
  const OrbitPosition now = kepler(orbit_at(c.ta));
  const OrbitPosition before = kepler(orbit_at(time_arguments(jd - kVelStepDays)));
  const OrbitPosition after = kepler(orbit_at(time_arguments(jd + kVelStepDays)));
  double w1 = after.hel;
  double w2 = before.hel;
  verv(w1, w2);
  const HelioState h = {now.hel, now.heb, now.r, (w1 - w2) * kVelRate, (after.heb - before.heb) * kVelRate, (after.r - before.r) * kVelRate};
  b.present = true;
  b.valid = true;
  b.hel = h.l;
  b.heb = h.b;
  b.r = h.r;
  if (c.s.heliocentric) {
    helio_display(c, h, b);
    return;
  }
  const GeoResult g = helio_to_geo(h, earth, c.smo.dpsi, c.smo.deps);
  b.el = g.el;
  b.eb = g.eb;
  b.dr = g.dr;
  b.tb = g.tb;
  b.ttb = g.ttb;
  par_ap_ktr(c, slot, b, sun_el, sun_eb);
}

// the original vel_om_pd, node and apogee speeds from a symmetric hour
void node_apogee_speeds(const ChartSettings& s, double jd_et, Chart& chart) {
  //RR 1h
  const double djd = kOneHourDays;
  const auto lunar_at = [&](double jd) {
    const TimeArguments t = time_arguments(jd);
    const SunMoonState st = somo(t, calendar_date(jd, s.calendar));
    return lunar_points(moon_position(t, st), st, t);
  };
  const LunarPoints l1 = lunar_at(jd_et - djd);
  const LunarPoints l2 = lunar_at(jd_et + djd);
  if (s.true_apogee && s.nk[1] > 0) {
    double w1 = l1.true_apogee;
    double w2 = l2.true_apogee;
    double w3 = chart.lunar.true_apogee;
    vergl2(w1, w2, w3);
    const int slot = s.nk[1];
    chart.b[static_cast<std::size_t>(slot)].tb = (w2 - w1) / (2.0 * djd);
    chart.b[static_cast<std::size_t>(slot)].ttb = ((w2 - w3) / djd) - ((w3 - w1) / djd);
  }
  if (s.true_node) {
    double w1 = l1.true_node;
    double w2 = l2.true_node;
    double w3 = chart.lunar.true_node;
    vergl2(w1, w2, w3);
    chart.b[body::kNodeAsc].tb = (w2 - w1) / (2.0 * djd);
    chart.b[body::kNodeAsc].ttb = ((w2 - w3) / djd) - ((w3 - w1) / djd);
    chart.b[body::kNodeDesc].tb = chart.b[body::kNodeAsc].tb;
  }
}

}  // namespace

//RR GL
// ported from HORCOM ta_na
int ta_na(double ac, double sun_el) {
  const double w1 = norm_rad(ac + kPi);
  double w2 = ac - kEps;
  double w3 = sun_el;
  vergl2(w1, w2, w3);
  return (w1 < w3 && w3 < w2) ? 1 : 2;
}

Chart compute_chart(const ChartInput& in, const ChartSettings& s, const VsopTables& vsop, const Ephemerides& eph) {
  Chart chart;
  chart.jd_ut = julian_day(in.date_ut, s.calendar);

  // houses in UT. The 0h somo state supplies the nutation for the
  // apparent sidereal time and the obliquity the angles see, exactly the
  // state the original sidt leaves behind.
  CalendarDate midnight = in.date_ut;
  midnight.hour = 0.0;
  midnight.minute = 0.0;
  const double jd0 = julian_day(midnight, s.calendar);
  const SunMoonState smo0 = somo(time_arguments(jd0), midnight);
  chart.ekls0 = smo0.ekls;
  chart.h0 = gmst0_hours(jd0);
  if (s.apparent_sidereal) {
    chart.h0 = apparent_sidereal_hours(chart.h0, smo0.dpsi, smo0.ekls);
  }
  chart.hs = sidereal_at_hours(chart.h0, in.date_ut.hour + in.date_ut.minute / 60.0);
  //RR in Grad
  chart.armc_deg = kDegPerHour * norm_hours(chart.hs + in.lon_deg_east / kDegPerHour);
  if (s.heliocentric) {
    // the hrg mode knows no houses, horg11 and bes111 stay dark there,
    // the empty cusp array must not read as the refused latitude guard
    chart.houses.ok = true;
  } else {
    const double armcb = kDegToRad * chart.armc_deg;
    chart.houses = compute_houses(s.houses, armcb, in.lat_deg, chart.ekls0);
    if (!chart.houses.ok) {
      return chart;
    }
    chart.b[body::kAscendant] = {true, true, chart.houses.angles.ac};
    chart.b[body::kMc] = {true, true, chart.houses.angles.mc};
  }

  // bodies in ET, the original a90 chain
  chart.delt_minutes = delta_t_minutes(chart.jd_ut);
  chart.jd_et = ut_to_et(chart.jd_ut);
  chart.ta = time_arguments(chart.jd_et);
  const CalendarDate date_et = calendar_date(chart.jd_et, s.calendar);
  chart.smo = somo(chart.ta, date_et);
  const Ctx c{s, vsop, eph, chart.ta, chart.smo, in.lat_deg, chart.armc_deg};

  const VsopTables::Result ev = vsop.evaluate(3, chart.ta.t11);
  const HelioState earth = from_vsop(ev);
  double sun_el = 0.0;
  double sun_eb = 0.0;
  if (s.heliocentric) {
    // the hrg mode, slot one stays empty like aa at two, the earth
    // takes the moon's slot as plposhi(3) fills it in plant1
    BodyState& te = chart.b[body::kMoon];
    te.present = true;
    te.valid = true;
    te.hel = earth.l;
    te.heb = earth.b;
    te.r = earth.r;
    helio_display(c, earth, te);
  } else {
    compute_sun(c, chart.b[body::kSun]);
    sun_el = chart.b[body::kSun].el;
    sun_eb = chart.b[body::kSun].eb;
  }

  // the Moon, no light time like the original moko
  chart.moon = moon_position(chart.ta, chart.smo);
  if (!s.heliocentric) {
    BodyState& mo = chart.b[body::kMoon];
    mo.present = true;
    mo.valid = true;
    mo.el = chart.moon.el;
    mo.eb = chart.moon.eb;
    mo.dr = chart.moon.r;
    mo.r = chart.moon.r;
    mo.tb = chart.moon.elp;
    BodyPosition p{mo.el, mo.eb, 0.0, 0.0, mo.dr, mo.tb};
    to_equatorial(p, chart.smo.ekls);
    if (s.topocentric_parallax) {
      parallax(p, in.lat_deg, chart.armc_deg, chart.smo.ekls);
    }
    mo.el = p.el;
    mo.eb = p.eb;
    mo.ar = p.ar;
    mo.de = p.de;
  }

  // lunar nodes and the Black Moon, geocentric ideas that the hrg mode
  // leaves out like the asp0 slot ranges
  chart.lunar = lunar_points(chart.moon, chart.smo, chart.ta);
  if (!s.heliocentric) {
    BodyState& dr = chart.b[body::kNodeAsc];
    BodyState& ds = chart.b[body::kNodeDesc];
    dr.present = true;
    dr.valid = true;
    ds.present = true;
    ds.valid = true;
    dr.el = s.true_node ? chart.lunar.true_node : chart.lunar.mean_node;
    ds.el = norm_rad(dr.el + kPi);
    dr.tb = chart.lunar.mean_node_speed;
    ds.tb = dr.tb;
    BodyPosition p{dr.el, 0.0, 0.0, 0.0, 0.0, 0.0};
    to_equatorial(p, chart.smo.ekls);
    dr.ar = p.ar;
    dr.de = p.de;
    p = {ds.el, 0.0, 0.0, 0.0, 0.0, 0.0};
    to_equatorial(p, chart.smo.ekls);
    ds.ar = p.ar;
    ds.de = p.de;
  }
  if (s.extra_bodies && s.nk[1] > 0 && !s.heliocentric) {
    BodyState& ag = chart.b[static_cast<std::size_t>(s.nk[1])];
    ag.present = true;
    ag.valid = true;
    if (s.true_apogee) {
      ag.el = chart.lunar.true_apogee;
      ag.eb = chart.lunar.true_apogee_lat;
    } else {
      ag.el = chart.lunar.mean_apogee;
      ag.eb = chart.lunar.mean_apogee_lat;
      ag.tb = chart.lunar.mean_apogee_speed;
    }
    ag.r = chart.moon.r;
    BodyPosition p{ag.el, ag.eb, 0.0, 0.0, 0.0, 0.0};
    to_equatorial(p, chart.smo.ekls);
    ag.ar = p.ar;
    ag.de = p.de;
  }
  if ((s.true_node || s.true_apogee) && !s.heliocentric) {
    node_apogee_speeds(s, chart.jd_et, chart);
  }

  // Mercury through Neptune
  for (int slot = body::kMercury; slot <= body::kNeptune; ++slot) {
    compute_planet(c, slot, earth, sun_el, sun_eb, chart.b[static_cast<std::size_t>(slot)]);
  }

  // Pluto, his integrated file first, the Chapront theory outside it
  {
    BodyState& pl = chart.b[body::kPluto];
    pl.present = true;
    HelioState h;
    if (eph_helio(c, "pluto", chart.jd_et, h)) {
      pl.valid = true;
    } else {
      h = chapront_helio(c, chart.jd_et);
      pl.valid = true;
    }
    pl.hel = h.l;
    pl.heb = h.b;
    pl.r = h.r;
    if (s.heliocentric) {
      helio_display(c, h, pl);
    } else {
      const GeoResult g = helio_to_geo(h, earth, chart.smo.dpsi, chart.smo.deps);
      pl.el = g.el;
      pl.eb = g.eb;
      pl.dr = g.dr;
      pl.tb = g.tb;
      pl.ttb = g.ttb;
      par_ap_ktr(c, body::kPluto, pl, sun_el, sun_eb);
    }
  }

  // the extra bodies of the nk table
  if (s.extra_bodies) {
    for (int i = 2; i <= 22; ++i) {
      const int slot = s.nk[static_cast<std::size_t>(i)];
      if (slot <= 0 || i == 4) {
        continue;
      }
      BodyState& b = chart.b[static_cast<std::size_t>(slot)];
      if (i == 3 || (i >= 9 && i <= 16)) {
        compute_kepler_body(c, slot, i, chart.jd_et, earth, sun_el, sun_eb, b);
      } else {
        const std::string_view name = body::eph_name(slot);
        if (!name.empty()) {
          compute_eph_body(c, slot, name, chart.jd_et, earth, sun_el, sun_eb, b);
        }
      }
    }
    // the Part of Fortune after Sun and Moon exist, the original a901,
    // a geocentric idea the hrg mode leaves out
    if (s.nk[4] > 0 && !s.heliocentric) {
      BodyState& gl = chart.b[static_cast<std::size_t>(s.nk[4])];
      gl.present = true;
      gl.valid = true;
      const double ac = chart.houses.angles.ac;
      if (ta_na(ac, sun_el) == 1) {
        gl.el = norm_rad(ac + chart.b[body::kMoon].el - sun_el);
      } else {
        gl.el = norm_rad(ac + sun_el - chart.b[body::kMoon].el);
      }
      gl.eb = 0.0;
      BodyPosition p{gl.el, 0.0, 0.0, 0.0, 0.0, 0.0};
      to_equatorial(p, chart.smo.ekls);
      gl.ar = p.ar;
      gl.de = p.de;
    }
  }

  chart.ok = true;
  return chart;
}

}  // namespace horcom
