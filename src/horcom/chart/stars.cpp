// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/chart/stars.hpp"

#include <cmath>
#include <cstring>

#include "horcom/chart/bodies.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"
#include "horcom/ephem/precession.hpp"

namespace horcom {

namespace {

//RR nach MONT., his annual aberration constant
constexpr double kAberration = 0.000099338;

// the catalogue rows of stella, right ascension and declination in
// degrees with their motion per century, distance in light years
struct StarData {
  const char* name;
  const char* quality;
  const char* astro;
  double ar;
  double dar;
  double de;
  double dde;
  int lightyears;
};

constexpr StarData kStars[] = {
    {"DENEB KAITOS", "SA", "BETA  CETUS", 10.89583, 1.254167, -17.98333, 0.55, 57},
    {"ALGENIB", "MA ME", "GAMMA PEGASUS", 3.30833, 1.2875, 15.18333, 0.55, 270},
    {"SIRRAH", "JU VE", "ALFA  ANDROM.", 2.09583, 1.29167, 29.08333, 0.55, 136},
    {"MIRACH", "VE", "BETA  ANDROM.", 17.433333, 1.4, 35.61667, 0.53333, 76},
    {"MIRA", "SA JU", "OMIK. CETUS", 34.8375, 1.2625, -2.983333, 0.45, 251},
    {"HAMAL", "MA SA", "ALFA  ARIES", 31.79167, 1.408333, 23.45, 0.46666, 76},
    {"SCHEDIR", "SA VE", "ALFA  CASS.", 10.1291667, 1.4208333, 56.53333, 0.55, 362},
    {"ALAMAK", "VE MA", "GAMMA ANDROM.", 30.970833, 1.533333, 42.33333, 0.48333, 650},
    {"MENKAR", "SA", "ALFA  CETUS", 45.57083, 1.308333, 4.1, 0.4, 1080},
    {"ZANRAK", "SA", "GAMMA ERID.", 59.508333, 1.166666, -13.5166666, 0.283333, 1080},
    {"ALGOL", "MA SA PL", "BETA  PERSEUS", 47.04583, 1.62916, 40.95, 0.3833, 105},
    {"PLEJADEN ***", "MO MA", "2 GRAD/120 *", 56.575, 1, 24.0833, 1, 400},
    {"HYADEN   ***", "MA NE UR", "5 GRAD/100 *", 66.55, 1, 15.83333, 1, 130},
    {"ALDEBARAN", "MA", "ALFA  TAURI", 68.979166, 1.43333, 16.5, 0.200, 50},
    {"RIGEL", "MA JU", "BETA  ORION", 78.63333, 1.2, -8.2, 0.1167, 850},
    {"BELLATRIX", "MA ME", "GAMMA ORION", 81.28333, 1.34167, 6.35, 0.0833, 450},
    {"CAPELLA", "ME MA", "ALFA  AURIGA", 79.17083, 1.84583, 46.0, 0.1, 45},
    {"PHAKT", "ME VE", "ALFA  COLUM.", 84.9125, 0.904167, -34.08333, 0.05, 0},
    {"BETEIGEUSE", "MA ME", "ALFA  ORION", 88.79167, 1.35417, 7.4, 0.0167, 650},
    {"POLARSTERN", "SA SO VE", "ALFA  URSMIN.", 37.804167, 17.1667, 89.25, 0.48333, 1090},
    {"ALHENA", "VE JU", "GAMMA GEMINI", 99.429167, 1.4458, 16.4, -0.0833, 105},
    {"SIRIUS", "MA JU", "ALFA  CAN.MAJ", 101.2875, 1.1, -16.7167, -0.1333, 9},
    {"CANOPUS", "JU SA", "ALFA  CARINA", 95.9875, 0.554167, -52.6833, -0.05, 181},
    {"CASTOR", "ME JU", "ALFA  GEMINI", 113.65, 1.59583, 31.88333, -0.21667, 45},
    {"POLLUX", "MA", "BETA  GEMINI", 116.3292, 1.529167, 28.0167, -0.25, 35},
    {"PROCYON", "MA ME", "ALFA  CAN.MIN", 114.825, 1.30833, 5.233333, -0.25, 13},
    {"PRAESEPE ***", "MO MA", "1GRAD/100 *", 129.85, 1, 20.03, 1, 520},
    {"KOCHAB", "MA", "BETA  URS.MIN", 222.67917, -4.25, 74.15, -0.41667, 105},
    {"DUBHE", "MA", "ALFA  URS.MAJ", 165.9333, 1.54167, 61.75, -0.5333, 105},
    {"MERAK", "MA", "BETA  URS.MAJ", 165.9333, 1.8083, 56.3833, -0.53333, 78},
    {"ALPHARD", "SA VE NE", "ALFA  HYDRA", 141.89583, 1.229167, -8.667, -0.4333, 192},
    {"REGULUS", "JU MA", "ALFA  LEO", 152.0917, 1.32917, 11.9667, -0.48333, 85},
    {"PHEKDA", "MA UR NE", "GAMMA URS.MAJ", 178.454167, 1.3125, 53.7, -0.55, 163},
    {"ALIOTH", "MA", "EPSI  URS.MAJ", 193.5083, 1.1, 55.950, -0.55, 408},
    {"MIZAR", "MA", "ZETA  URS.MAJ", 200.98333, 1.008333, 54.93333, -0.51666, 88},
    {"DENEBOLA", "UR VE SA", "BETA  LEO", 177.2667, 1.275, 14.5667, -0.5667, 43},
    {"BENETNASCH", "MA UR SA", "ETA   URS.MAJ", 206.8833, 0.9833, 49.3167, -0.5, 815},
    {"VINDEMIATRIX", "SA ME", "EPSY. VIRGO", 195.5458, 1.2458, 10.9667, -0.5333, 90},
    {"ALGORAB", "MA SA", "DELTA CORVO", 187.4625, 1.29167, -16.51667, -0.55, 181},
    {"SPICA", "VE MA", "ALFA  VIRGO", 201.2958, 1.3167, -11.15, -0.5167, 220},
    {"ARCTURUS", "JU MA", "ALFA  BOOTES", 213.9167, 1.1417, 19.1833, -0.5167, 36},
    {"GEMMA", "VE ME", "ALFA  CORONA", 233.67083, 1.05833, 26.7167, -0.3333, 76},
    {"SÜDL.WAAGSCH.", "MA SA", "ALFA  LIBRA", 222.72083, 1.38333, -16.05, -0.4167, 67},
    {"NÖRDL.WAAGS.", "ME JU", "BETA  LIBRA", 229.25, 1.34583, -9.3833, -0.3637, 0},
    {"UNUK", "MA SA", "ALFA  SERPEN.", 236.07083, 1.233, 6.4166, -0.31667, 71},
    {"BETA CENTAURI", "VE JU", "BETA  CENTAU.", 210.95833, 1.766666, -60.366666, -0.4833333, 204},
    {"BUNGULA", "VE JU", "ALFA  CENTAU.", 219.9, 1.7, -60.8333, -0.4167, 4},
    {"AKRAB", "MA SA", "BETA  SKORPIO", 241.35833, 1.45417, -19.8, -0.2667, 815},
    {"ANTARES", "MA JU SA", "ALFA  SCORPIO", 247.3542, 1.5333, -26.4333, -0.2167, 400},
    {"RAS ALGETHI", "MA VE", "ALF(1)HERCUL.", 258.6625, 1.1467, 14.3833, -0.1167, 500},
    {"RAS ALHAGUE", "SA VE", "ALFA  OPHIUC.", 263.7333, 1.15833, 12.567, 0.0667, 58},
    {"ETTANIN", "MA JU SA", "GAMMA DRACON.", 269.15, 0.579167, 51.4833, 0.0167, 220},
    {"GALAKT.ZENTR.", "SO MC PL", "", 265.65417, 1, -28.95, 1, 26000},
    {"APEX", "SO AC PL", "", 271.21, 1, 30, 1, 0},
    {"BOGEN SAGITT.", "JU MA", "LAMBD SAGITT.", 276.99167, 1.54167, -25.4333, 0.05, 71},
    {"VEGA", "VE ME NE", "ALFA  LYRA", 279.2333, 0.84583, 38.7833, 0.1, 26},
    {"ATAIR", "ME JU", "ALFA  AQUILA", 297.6958, 1.2208, 8.8667, 0.2667, 16},
    {"FOMALHAUT", "ME VE NE", "BETA  PISC.A", 344.4125, 1.379167, -29.6167, 0.53333, 23},
    {"DENEB", "ME VE", "ALFA  CYGNUS", 310.3583, 0.8542, 45.2667, 0.35, 1500},
    {"ACHERNAR", "JU MA UR", "ALFA  ERID.", 24.425, 0.929167, -57.25, 0.5, 142},
    {"MARKAB", "ME MA", "ALFA  PEGASUS", 346.19167, 1.025, 15.2, 0.533333, 109},
    {"SCHEAT", "SA", "BETA  PEGASUS", 345.94583, 1.2125, 28.0833, 0.55, 217},
};

// the clusters and the galactic points precess from a fixed epoch
// instead of moving linearly
bool precessed_entry(const char* name, double& jda) {
  if (std::strncmp(name, "GAL", 3) == 0 || std::strncmp(name, "APE", 3) == 0) {
    //RR 31.12.1949
    jda = kJdB1950;
    return true;
  }
  if (std::strncmp(name, "PLE", 3) == 0 || std::strncmp(name, "PRA", 3) == 0 || std::strncmp(name, "HYA", 3) == 0) {
    //RR 1.7.1988
    jda = 2447344.2481;
    return true;
  }
  return false;
}

}  // namespace

// ported from HORCOM stella and stelk
std::vector<StarRow> fixed_stars(const Chart& chart, double orb) {
  std::vector<StarRow> out;
  const double ekls = chart.smo.ekls;
  const double l = chart.b[body::kSun].el;
  const double tja = chart.ta.tropical_year_days;
  for (const StarData& s : kStars) {
    StarRow row;
    row.name = s.name;
    row.quality = s.quality;
    row.astro = s.astro;
    row.lightyears = s.lightyears;
    double ar = s.ar * kDegToRad;
    double de = s.de * kDegToRad;
    double jda = 0.0;
    if (precessed_entry(s.name, jda)) {
      double ar1 = 0.0;
      double de1 = 0.0;
      precess_newcomb(chart.ta.jd, tja, jda, ar, de, ar1, de1);
      ar = ar1;
      de = de1;
    } else {
      // linear motion from the catalogue epoch two thousand
      const double ts = chart.ta.t1 - 1.0;
      ar += ts * s.dar * kDegToRad;
      de += ts * s.dde * kDegToRad;
    }
    // annual aberration against the chart's sun
    const double a = kAberration * (std::sin(l) * std::sin(ar) + std::cos(l) * std::cos(ar) * std::cos(ekls));
    const double df = kAberration * (std::sin(de) * std::cos(ar) * std::sin(l) +
                                     std::cos(l) * (std::sin(ekls) * std::cos(de) - std::cos(ekls) * std::sin(de) * std::sin(ar)));
    ar -= a / std::cos(de);
    de -= df;
    const Ecliptic ec = equatorial_to_ecliptic(ar, de, ekls);
    row.la = ec.lon;
    row.br = ec.lat;
    row.ar = ar;
    row.de = de;
    row.aspects = point_aspects(chart, row.la, orb);
    out.push_back(std::move(row));
  }
  return out;
}

// ported from HORCOM stelk
std::vector<std::pair<int, char>> point_aspects(const Chart& chart, double la, double orb) {
  std::vector<std::pair<int, char>> out;
  // the aspect windows of stelk, two degrees times the orb for the
  // conjunction, halved, quartered and thirded down the row
  const double h = 2.0 * kDegToRad * orb;
  for (int k = 0; k < body::kSlotCount; ++k) {
    if (k == body::kNodeDesc) {
      continue;
    }
    const BodyState& b = chart.b[static_cast<std::size_t>(k)];
    if (!b.present || !b.valid) {
      continue;
    }
    const double p = b.el;
    const double ca = norm_rad(la - p);
    const double cb = norm_rad(la + kPi - p);
    const double cc = norm_rad(la + kPi / 2.0 - p);
    const double cd = norm_rad(la + 3.0 * kPi / 2.0 - p);
    const double ce = norm_rad(la + 2.0 * kPi / 3.0 - p);
    const double cf = norm_rad(la + 4.0 * kPi / 3.0 - p);
    if (ca > 0.0 && (ca < h || ca > kTwoPi - h)) {
      out.emplace_back(k, 'K');
    }
    if (cb > 0.0 && cb < h / 2.0) {
      out.emplace_back(k, 'O');
    }
    if ((cc > 0.0 && cc < h / 4.0) || (cd > 0.0 && cd < h / 4.0)) {
      out.emplace_back(k, 'Q');
    }
    if ((ce > 0.0 && ce < h / 3.0) || (cf > 0.0 && cf < h / 3.0)) {
      out.emplace_back(k, 'T');
    }
  }
  return out;
}

}  // namespace horcom
