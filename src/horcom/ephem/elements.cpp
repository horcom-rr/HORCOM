// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/ephem/elements.hpp"

#include <stdexcept>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

//RR Elemente nach MEEUS
Orbit mean_elements(int body, const TimeArguments& t) {
  const double t11 = t.t11;
  const double t21 = t.t21;
  const double t31 = t.t31;
  const double pu = kDegToRad;
  Orbit o;
  switch (body) {
    case 1:  //RR TE
      o.mel = norm_rad(pu * (100.466449 + 36000.7698231 * t11 + 0.00030368 * t21 + 2.1e-08 * t31));
      o.a = 1.000001018;
      o.e = 0.01670862 - 0.000042037 * t11 - 1.236e-07 * t21 + 4.0e-11 * t31;
      o.i = 0.0;
      o.o = 0.0;
      o.p = pu * (102.937348 + 1.7195269 * t11 + 0.00045962 * t21 + 4.99e-07 * t31);
      break;
    case 3:  //RR ME
      o.mel = norm_rad(pu * (252.250906 + 149474.0722491 * t11 + 0.00030397 * t21 - 1.8e-08 * t31));
      o.a = 0.38709831;
      o.e = 0.20563175 + 0.000020406 * t11 - 2.84e-08 * t21 - 1.7e-10 * t31;
      o.i = pu * (7.004986 + 0.0018215 * t11 - 0.00001809 * t21 + 5.3e-08 * t31);
      o.o = pu * (48.330893 + 1.186189 * t11 + 0.00017587 * t21 + 2.11e-07 * t31);
      o.p = pu * (77.456119 + 1.5564775 * t11 + 0.00029589 * t21 + 5.6e-08 * t31);
      break;
    case 4:  //RR VE
      o.mel = norm_rad(pu * (181.979801 + 58519.2130302 * t11 + 0.0003106 * t21 + 1.5e-08 * t31));
      o.a = 0.72332982;
      o.e = 0.00677188 - 0.000047766 * t11 + 9.75e-08 * t21 + 4.4e-10 * t31;
      o.i = pu * (3.394662 + 0.0010037 * t11 - 8.8e-07 * t21 - 7.0e-09 * t31);
      o.o = pu * (76.67992 + 0.901119 * t11 + 0.00040665 * t21 - 8.0e-08 * t31);
      o.p = pu * (131.563707 + 1.4022188 * t11 - 0.00107337 * t21 - 5.315e-06 * t31);
      break;
    case 5:  //RR MA
      o.mel = norm_rad(pu * (355.433275 + 19141.6964746 * t11 + 0.00031097 * t21 + 1.5e-08 * t31));
      o.a = 1.523679342;
      o.e = 0.09340062 + 0.000090483 * t11 - 8.06e-08 * t21 - 3.5e-10 * t31;
      o.i = pu * (1.849726 - 0.000601 * t11 + 0.00001276 * t21 - 6.0e-09 * t31);
      o.o = pu * (49.558093 + 0.7720923 * t11 + 0.00001605 * t21 + 2.325e-06 * t31);
      o.p = pu * (336.060234 + 1.8410331 * t11 + 0.00013515 * t21 + 3.18e-07 * t31);
      break;
    case 6:  //RR JU
      o.mel = norm_rad(pu * (34.351484 + 3036.3027889 * t11 + 0.00022374 * t21 + 2.5e-08 * t31));
      o.a = 5.202603191 + 1.913e-07 * t11;
      o.e = 0.04849485 + 0.000163244 * t11 - 4.719e-07 * t21 - 1.97e-09 * t31;
      o.i = pu * (1.30327 - 0.0054966 * t11 + 4.65e-06 * t21 - 4.0e-09 * t31);
      o.o = pu * (100.464441 + 1.020955 * t11 + 0.00040117 * t21 + 5.69e-07 * t31);
      o.p = pu * (14.331309 + 1.6126668 * t11 + 0.00103127 * t21 - 4.569e-06 * t31);
      break;
    case 7:  //RR SA
      o.mel = norm_rad(pu * (50.077471 + 1223.5110141 * t11 + 0.00051952 * t21 - 3.0e-09 * t31));
      o.a = 9.554909596 - 2.1389e-06 * t11;
      o.e = 0.05550862 - 0.000346818 * t11 - 6.456e-07 * t21 + 3.38e-09 * t31;
      o.i = pu * (2.488878 - 0.0037363 * t11 - 0.00001516 * t21 + 8.9e-08 * t31);
      o.o = pu * (113.665524 + 0.8770979 * t11 - 0.00012067 * t21 - 2.38e-06 * t31);
      o.p = pu * (93.056787 + 1.9637694 * t11 + 0.00083757 * t21 + 4.899e-06 * t31);
      break;
    case 8:  //RR UR
      o.mel = norm_rad(pu * (314.055005 + 429.8640561 * t11 + 0.00030434 * t21 + 2.6e-08 * t31));
      o.a = 19.218446062 - 3.72e-08 * t11 + 9.8e-10 * t21;
      o.e = 0.0462959 - 0.000027337 * t11 + 7.9e-08 * t21 + 2.5e-10 * t31;
      o.i = pu * (0.773196 + 0.0007744 * t11 + 0.00003749 * t21 - 9.2e-08 * t31);
      o.o = pu * (74.005947 + 0.5211258 * t11 + 0.00133982 * t21 + 0.000018516 * t31);
      o.p = pu * (173.005159 + 1.4863784 * t11 + 0.0002145 * t21 + 4.33e-07 * t31);
      break;
    case 9:  //RR NE
      o.mel = norm_rad(pu * (304.348665 + 219.8833092 * t11 + 0.00030926 * t21 + 1.8e-08 * t31));
      o.a = 30.110386869 - 1.663e-07 * t11 + 6.9e-10 * t21;
      o.e = 0.00898809 + 6.408e-06 * t11 - 8.0e-10 * t21 - 5.0e-11 * t31;
      o.i = pu * (1.769952 - 0.0093082 * t11 - 7.08e-06 * t21 + 2.8e-08 * t31);
      o.o = pu * (131.784057 + 1.1022057 * t11 + 0.00026006 * t21 - 6.36e-07 * t31);
      o.p = pu * (48.123691 + 1.4262677 * t11 + 0.00037918 * t21 - 3.0e-09 * t31);
      break;
    default:
      throw std::invalid_argument("mean_elements handles bodies 1 and 3 through 9");
  }
  o.man = norm_rad(o.mel - o.p);
  return o;
}

//RR TP
Orbit transpluto_elements(const TimeArguments& t) {
  Orbit o;
  o.a = 77.755;
  o.e = 0.3;
  o.i = 0.0;
  o.p = 0.00076575972 + 0.024365 * t.t1;
  o.o = 0.0;
  o.man = norm_rad(1.1659863 + 0.91638372749 * t.t1);
  return o;
}

Orbit uranian_elements(int k, const TimeArguments& t) {
  Orbit o;
  switch (k) {
    case 9:  //RR CUPIDO
      o.a = 41.182704557;
      o.man = norm_rad(1.835881356 + 2.40180025 * t.t1);
      break;
    case 10:
      o.a = 50.5809095;
      o.man = norm_rad(5.886203695 + 1.770997 * t.t1);
      break;
    case 11:
      o.a = 59.30660263;
      o.man = norm_rad(1.818503639 + 1.40007598 * t.t1);
      break;
    case 12:
      o.a = 65.06865729;
      o.man = norm_rad(0.313584087 + 1.22144827 * t.t1);
      break;
    case 13:
      o.a = 70.3169545;
      o.man = norm_rad(2.4093538 + 1.08996027 * t.t1);
      break;
    case 14:
      o.a = 73.61284229;
      o.man = norm_rad(6.1314576 + 1.0192027 * t.t1);
      break;
    case 15:
      o.a = 77.247746;
      o.man = norm_rad(0.97532577 + 0.94982 * t.t1);
      break;
    case 16:
      o.a = 83.66791295;
      o.man = norm_rad(2.88913441 + 0.8453682 * t.t1);
      break;
    default:
      throw std::invalid_argument("uranian_elements handles slots 9 through 16");
  }
  return o;
}

}  // namespace horcom
