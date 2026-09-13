// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/ephem/vsop.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>

#include "horcom/core/angle.hpp"

namespace horcom {

namespace {

std::vector<char> read_all(const std::filesystem::path& p) {
  std::ifstream f(p, std::ios::binary);
  if (!f) {
    throw std::runtime_error("cannot open " + p.string());
  }
  std::vector<char> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return bytes;
}

std::uint16_t u16_at(const std::vector<char>& bytes, std::size_t off) {
  std::uint16_t v = 0;
  std::memcpy(&v, bytes.data() + off, 2);
  return v;
}

double f64_at(const std::vector<char>& bytes, std::size_t off) {
  double v = 0;
  std::memcpy(&v, bytes.data() + off, 8);
  return v;
}

}  // namespace

VsopTables VsopTables::load(const std::filesystem::path& ndx_path, const std::filesystem::path& dat_path) {
  const std::vector<char> ndx = read_all(ndx_path);
  const std::vector<char> dat = read_all(dat_path);
  if (ndx.size() < 9 * 18 * 4 || dat.size() % 24 != 0) {
    throw std::runtime_error("planetary term tables have unexpected size");
  }

  VsopTables t;
  // cterm& in the original starts at 1 into 1-based arrays, here the term
  // vectors are 0-based and the offsets are rebased accordingly
  for (int pl = 1; pl <= 8; ++pl) {
    // 'Indices ermitteln
    int rec = pl * 18;
    Series raw[3][6];
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 6; ++j) {
        const std::size_t off = static_cast<std::size_t>(rec) * 4;
        raw[i][j].offset = u16_at(ndx, off);
        raw[i][j].count = u16_at(ndx, off + 2);
        ++rec;
      }
    }
    const int datoffset = raw[0][0].offset;
    int cursor = datoffset;
    const int rebase = static_cast<int>(t.a_.size()) - datoffset;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 6; ++j) {
        for (int k = 0; k < raw[i][j].count; ++k) {
          const std::size_t off = static_cast<std::size_t>(cursor) * 24;
          if (off + 24 > dat.size()) {
            throw std::runtime_error("planetary term table truncated");
          }
          t.a_.push_back(f64_at(dat, off));
          t.b_.push_back(f64_at(dat, off + 8));
          t.c_.push_back(f64_at(dat, off + 16));
          ++cursor;
        }
        t.index_[static_cast<std::size_t>(pl)][static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = {
            raw[i][j].offset + rebase, raw[i][j].count};
      }
    }
  }
  return t;
}

VsopTables::Result VsopTables::evaluate(int planet, double t11, bool with_rates) const {
  // the original works in Julian millennia, t = 1E-15 + t11 / 10
  const double t = 1e-15 + t11 / 10.0;
  double ko[3] = {0.0, 0.0, 0.0};
  double vel[3] = {0.0, 0.0, 0.0};
  for (int i = 0; i < 3; ++i) {
    double tn = 1.0;
    for (int j = 0; j < 6; ++j) {
      double x = 0.0;
      double v = 0.0;
      const Series s = index_[static_cast<std::size_t>(planet)][static_cast<std::size_t>(i)][static_cast<std::size_t>(j)];
      for (int k = 0; k < s.count; ++k) {
        const std::size_t of = static_cast<std::size_t>(s.offset + k);
        const double a = a_[of];
        const double c = c_[of];
        const double arg = norm_rad(b_[of] + t * c);
        x += a * std::cos(arg);
        if (with_rates) {
          v += -a * c * std::sin(arg);
        }
      }
      ko[i] += x * tn;
      if (with_rates) {
        vel[i] += x * j * tn / t + v * tn;
      }
      tn *= t;
    }
  }
  Result out;
  out.l = norm_rad(ko[0]);  //RR HEL.LÄ
  out.b = ko[1];            //RR HEL.BR
  out.r = ko[2];            //RR RADVEK
  out.lt = vel[0] / kDaysPerMillennium;  //RR HEL.GESCHW
  out.bt = vel[1] / kDaysPerMillennium;
  out.rt = vel[2] / kDaysPerMillennium;
  return out;
}

}  // namespace horcom
