#include "horcom/time/calendar.hpp"

#include <cmath>

namespace horcom {

namespace {

// GFA FIX, truncation toward zero
double fix(double x) noexcept {
  return std::trunc(x);
}

// GFA FRAC, x - FIX(x), sign follows x
double frac(double x) noexcept {
  return x - std::trunc(x);
}

}  // namespace

double julian_day(const CalendarDate& d, Calendar cal) {
  double y = 0.0;
  double m = 0.0;
  if (d.month > 2) {
    y = d.year;
    m = d.month;
  } else {
    y = d.year - 1;
    m = d.month + 12;
  }

  // The original encodes the date as ja + mo/100 + ta/10000 and compares
  // against 1582.1015, the first Gregorian day 1582-10-15.
  const double datecode = d.year + d.month / 100.0 + d.day / 10000.0;

  double b = 0.0;
  bool gregorian = false;
  if (datecode < 1582.1015) {
    gregorian = false;
  } else if (cal == Calendar::kJulian) {
    gregorian = false;
  } else {
    gregorian = true;
  }
  if (datecode < 1582.1015 && cal == Calendar::kGregorian) {
    gregorian = true;
  }
  if (gregorian) {
    const double a = fix(y / 100.0);
    b = 2.0 - a + fix(a / 4.0);
  }

  double jd = 0.0;
  if (y < 0.0) {
    jd = fix(365.25 * y - 0.75) + fix(30.6001 * (m + 1.0)) + d.day + d.hour / 24.0 + d.minute / 1440.0 + 1720994.5 + b;
  } else {
    jd = fix(365.25 * y) + fix(30.6001 * (m + 1.0)) + d.day + d.hour / 24.0 + d.minute / 1440.0 + 1720994.5 + b;
  }
  return jd;
}

CalendarDate calendar_date(double jd, Calendar cal) {
  const double z = fix(jd + 0.5);
  const double f = frac(jd + 0.5);

  double a = 0.0;
  if (z < 2299161.0 || cal == Calendar::kJulian) {
    a = z;
  } else {
    const double aa = fix((z - 1867216.25) / 36524.25);
    a = z + 1.0 + aa - fix(aa / 4.0);
  }
  if (z < 2299161.0 && cal == Calendar::kGregorian) {
    const double aa = fix((z - 1867216.25) / 36524.25);
    a = z + 1.0 + aa - fix(aa / 4.0);
  }

  const double b = a + 1524.0;
  const double c = fix((b - 122.1) / 365.25);
  const double dd = fix(365.25 * c);
  const double e = fix((b - dd) / 30.6001);
  const double g = b - dd - fix(30.6001 * e) + f;

  CalendarDate out;
  out.day = static_cast<int>(fix(g));
  out.hour = fix(frac(g) * 24.0);
  // the kk guard here is commented out in the original and stays out
  out.minute = 60.0 * frac(frac(g) * 24.0);
  if (e < 13.5) {
    out.month = static_cast<int>(e - 1.0);
  }
  if (e > 13.5) {
    out.month = static_cast<int>(e - 13.0);
  }
  if (out.month > 2) {
    out.year = static_cast<int>(c - 4716.0);
  }
  if (out.month < 2 || out.month == 2) {
    out.year = static_cast<int>(c - 4715.0);
  }
  return out;
}

TimeArguments time_arguments(double jd) {
  TimeArguments t;
  t.jd = jd;
  t.t1 = (jd - 2415020.0) / 36525.0;
  t.t2 = t.t1 * t.t1;
  t.t3 = t.t2 * t.t1;
  t.t4 = t.t3 * t.t1;
  t.t11 = t.t1 - 1.0;
  t.t21 = t.t11 * t.t11;
  t.t31 = t.t21 * t.t11;
  t.t41 = t.t31 * t.t11;
  t.tropical_year_days = 365.24219879 - 6.14e-06 * t.t1;
  t.mean_obliquity_rad = 0.40931974745 - 0.0002271109689 * t.t1 - 2.86234e-08 * t.t2 + 8.779e-09 * t.t3;
  return t;
}

}  // namespace horcom
