// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/place_file.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "horcom/core/constants.hpp"
#include "horcom/data/encoding.hpp"

namespace horcom {

namespace {

constexpr std::size_t kLonLen = 8;
constexpr std::size_t kLatLen = 8;
constexpr std::size_t kNameLen = 20;

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

std::string rset(std::string_view s, std::size_t len) {
  std::string out(len, ' ');
  const std::string_view use = s.size() > len ? s.substr(0, len) : s;
  for (std::size_t i = 0; i < use.size(); ++i) {
    out[len - use.size() + i] = use[i];
  }
  return out;
}

}  // namespace

std::optional<double> PlaceRecord::zone_to_ut() const {
  if (name.size() < 1) {
    return std::nullopt;
  }
  const std::size_t take = name.size() < 5 ? name.size() : 5;
  const std::string tail = name.substr(name.size() - take);
  bool has_digit = false;
  for (const char c : tail) {
    if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
      has_digit = true;
    }
  }
  if (!has_digit) {
    return std::nullopt;
  }
  return val(tail);
}

PlaceRecord decode_place_record(std::string_view bytes) {
  PlaceRecord r;
  r.lon = val(bytes.substr(0, kLonLen));
  r.lat = val(bytes.substr(kLonLen, kLatLen));
  r.name = cp1252_to_utf8(trimmed(bytes.substr(kLonLen + kLatLen, kNameLen)));
  return r;
}

std::string encode_place_record(const PlaceRecord& r) {
  char lon[16];
  char lat[16];
  std::snprintf(lon, sizeof(lon), "%.*g", 7, r.lon);
  std::snprintf(lat, sizeof(lat), "%.*g", 7, r.lat);
  std::string rec = rset(lon, kLonLen) + rset(lat, kLatLen);
  const std::string name = utf8_to_cp1252(r.name);
  for (std::size_t i = 0; i < kNameLen; ++i) {
    rec += (i < name.size()) ? name[i] : ' ';
  }
  return rec;
}

std::optional<std::vector<PlaceRecord>> read_place_file(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return std::nullopt;
  }
  std::string bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  if (bytes.size() % kPlaceRecordBytes != 0) {
    return std::nullopt;
  }
  std::vector<PlaceRecord> out;
  out.reserve(bytes.size() / kPlaceRecordBytes);
  for (std::size_t off = 0; off < bytes.size(); off += kPlaceRecordBytes) {
    out.push_back(decode_place_record(std::string_view(bytes).substr(off, kPlaceRecordBytes)));
  }
  return out;
}

std::optional<PlaceRecord> read_preferred_place(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return std::nullopt;
  }
  std::string bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  if (bytes.size() < kPlaceRecordBytes) {
    return std::nullopt;
  }
  PlaceRecord r;
  const std::string lon = trimmed(std::string_view(bytes).substr(0, kLonLen));
  const std::string lat = trimmed(std::string_view(bytes).substr(kLonLen, kLatLen));
  // the original branch, no decimal point means integer micro degrees
  if (lon.find('.') == std::string::npos) {
    r.lon = kEps + val(lon) / 1000000.0;
  } else {
    r.lon = kEps + val(lon);
  }
  if (lat.find('.') == std::string::npos) {
    r.lat = kEps + val(lat) / 1000000.0;
  } else {
    r.lat = kEps + val(lat);
  }
  r.name = cp1252_to_utf8(trimmed(std::string_view(bytes).substr(kLonLen + kLatLen, kNameLen)));
  return r;
}

}  // namespace horcom
