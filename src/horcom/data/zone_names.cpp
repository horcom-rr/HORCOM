// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/zone_names.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>

#include "horcom/data/encoding.hpp"

namespace horcom {

namespace {

constexpr std::size_t kNameCols = 46;
constexpr std::size_t kOffsetCols = 10;

std::string trimmed(std::string_view v) {
  std::size_t a = 0;
  std::size_t b = v.size();
  while (a < b && v[a] == ' ') {
    ++a;
  }
  while (b > a && v[b - 1] == ' ') {
    --b;
  }
  return std::string(v.substr(a, b - a));
}

double val(std::string_view v) {
  return std::strtod(std::string(v).c_str(), nullptr);
}

}  // namespace

// ported from zeitzon_nam_horc
ZoneEntry parse_zone_line(std::string_view line) {
  while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
    line.remove_suffix(1);
  }
  ZoneEntry e;
  e.name = cp1252_to_utf8(trimmed(line.substr(0, std::min(line.size(), kNameCols))));
  if (line.size() > kNameCols) {
    const std::size_t abbrev_end =
        line.size() > kNameCols + kOffsetCols ? line.size() - kOffsetCols : line.size();
    e.abbrev = cp1252_to_utf8(trimmed(line.substr(kNameCols, abbrev_end - kNameCols)));
  }
  if (line.size() >= kOffsetCols) {
    const std::string_view zd = line.substr(line.size() - kOffsetCols);
    bool has_digit = false;
    for (const char c : zd) {
      if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
        has_digit = true;
      }
    }
    if (has_digit) {
      //RR "-11 h 23 m"
      const double h = val(zd.substr(0, 3));
      const double min = val(zd.substr(6, 2));
      double zzd = std::abs(h) + min / 60.0;
      if (zd.front() == '-') {
        zzd = -zzd;
      }
      e.to_ut_hours = zzd;
    }
  }
  return e;
}

std::optional<std::vector<ZoneEntry>> load_zone_names(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    //RR ZEITZONEN-Datei fehlt !
    return std::nullopt;
  }
  std::vector<ZoneEntry> out;
  std::string line;
  int row = 0;
  // the original loads rows 0 to 176 only, row 0 is the header
  while (row < kZoneCatalogueRows && std::getline(f, line)) {
    if (row > 0) {
      out.push_back(parse_zone_line(line));
    }
    ++row;
  }
  return out;
}

}  // namespace horcom
