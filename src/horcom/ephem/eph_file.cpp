// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/ephem/eph_file.hpp"

#include <cmath>
#include <cstring>
#include <fstream>

#include "horcom/core/angle.hpp"
#include "horcom/ephem/interpolate.hpp"
#include "horcom/ephem/precession.hpp"

namespace horcom {

namespace {

constexpr EphBodyInfo kBodies[] = {
    {"pluto", 2.1748e9 / 52.0, EphFrame::kEquatorialJ2000},
    {"chiron", 2.1748e9 / 20.0, EphFrame::kEclipticB1950},
    {"ceres", 2.1748e9 / 5.0, EphFrame::kEclipticJ2000},
    {"pallas", 2.1748e9 / 5.0, EphFrame::kEclipticJ2000},
    {"juno", 2.1748e9 / 5.0, EphFrame::kEclipticJ2000},
    {"vesta", 2.1748e9 / 5.0, EphFrame::kEclipticJ2000},
    {"quaoar", 2.1748e9 / 60.0, EphFrame::kEclipticJ2000},
    // the original's duplicated CASE precesses Halley from B1950 even
    // though the file is written for J2000, preserved knowingly until a
    // golden test decides the fix
    {"halley", 2.1748e9 / 40.0, EphFrame::kEclipticB1950},
    {"pholus", 2.1748e9 / 40.0, EphFrame::kEclipticJ2000},
    {"damokles", 2.1748e9 / 40.0, EphFrame::kEclipticJ2000},
    {"nessus", 2.1748e9 / 40.0, EphFrame::kEclipticJ2000},
    {"xena", 2.1748e9 / 100.0, EphFrame::kEclipticJ2000},
};

}  // namespace

const EphBodyInfo* eph_body(std::string_view name) {
  for (const EphBodyInfo& b : kBodies) {
    if (b.filename == name) {
      return &b;
    }
  }
  return nullptr;
}

std::optional<EphFile> EphFile::open(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return std::nullopt;
  }
  std::vector<char> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  if (bytes.size() < 16 * 8 || bytes.size() % 16 != 0) {
    return std::nullopt;
  }
  EphFile e;
  e.raw_.resize(bytes.size() / 4);
  std::memcpy(e.raw_.data(), bytes.data(), bytes.size());
  // record zero is the writer's unwritten dummy, records one and two give
  // the newest epoch and the step
  e.jda_ = e.raw_[1 * 4];
  const double jd2 = e.raw_[2 * 4];
  e.djd_ = e.jda_ - jd2;  //RR Intervall-Länge
  e.upper_ = e.jda_ - 3.0 * e.djd_ + 0.5;  //RR oben !
  const std::size_t last = e.record_count() - 1;
  e.lower_ = e.raw_[last * 4] + 3.0 * e.djd_ - 0.5;
  return e;
}

EphFile::Sample EphFile::evaluate(double jd, double fplanet, EphFrame frame) const {
  Sample out;
  if (jd < lower_ || jd > upper_) {
    return out;
  }
  out.in_range = true;
  //RR FIX(jd) !!!
  const double jdsuch = jd - 0.5;
  //RR Datensatzindex von oben ( jda ) gerechnet )
  const auto ind = static_cast<std::size_t>(std::trunc((jda_ - jdsuch) / std::abs(djd_)));
  const double jd3 = raw_[(ind + 2) * 4];
  const double jdaeq = (frame == EphFrame::kEclipticB1950) ? kJdB1950 : kJdJ2000;

  // ys index one is the oldest of the five samples like the original loop
  double ys[3][6];
  for (int j = 5; j >= 1; --j) {
    const std::size_t rec = ind + 5 - static_cast<std::size_t>(j);
    std::array<double, 3> x = {raw_[rec * 4 + 1] / fplanet, raw_[rec * 4 + 2] / fplanet, raw_[rec * 4 + 3] / fplanet};
    if (frame == EphFrame::kEquatorialJ2000) {
      precess_equatorial(x, jd, jdaeq);
    } else {
      precess_ecliptic(x, jd, jdaeq);
    }
    ys[0][j] = x[0];
    ys[1][j] = x[1];
    ys[2][j] = x[2];
  }
  //RR Geschwind bez. auf Intervallmitte
  double vs[3][5];
  for (int j = 1; j <= 4; ++j) {
    for (int i = 0; i < 3; ++i) {
      vs[i][j] = (ys[i][j + 1] - ys[i][j]) / djd_;
    }
  }
  const double jdip = (jdsuch - jd3) / djd_;
  //RR Lage und Beschleunigung
  const double x1 = ipol(ys[0][1], ys[0][2], ys[0][3], ys[0][4], ys[0][5], jdip);
  const double x2 = ipol(ys[1][1], ys[1][2], ys[1][3], ys[1][4], ys[1][5], jdip);
  const double x3 = ipol(ys[2][1], ys[2][2], ys[2][3], ys[2][4], ys[2][5], jdip);
  //RR Geschwindigkeit
  const double yip = jdip - djd_ / 2.0;
  const double v1 = ipol3(vs[0][2], vs[0][3], vs[0][4], yip);
  const double v2 = ipol3(vs[1][2], vs[1][3], vs[1][4], yip);
  const double v3 = ipol3(vs[2][2], vs[2][3], vs[2][4], yip);

  out.xyz = {x1, x2, x3};
  const double rd = std::sqrt(x1 * x1 + x2 * x2 + x3 * x3);
  out.lon = atn(x2, x1);
  out.lat = std::asin(x3 / rd);
  out.r = rd;
  //RR Differenzieren von hel(f&)
  out.lont = std::abs((v2 * x1 - x2 * v1) / (x1 * x1 + x2 * x2));
  const double sq = 1.0 / std::sqrt(1.0 - (x3 / rd) * (x3 / rd));
  //RR Ableitung Radius
  out.rt = std::abs((x1 * v1 + x2 * v2 + x3 * v3) / rd);
  //RR Ableitung Breite
  out.latt = std::abs(sq * (rd * v3 - x3 * out.rt) / (rd * rd));
  return out;
}

}  // namespace horcom
