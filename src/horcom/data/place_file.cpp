// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/place_file.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <system_error>

#include "horcom/core/constants.hpp"
#include "horcom/data/encoding.hpp"
#include "horcom/data/file_io.hpp"

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

// ported from zeitzon with his FUNCTION VAL, which keeps only the signs,
// the comma, the dot and the digits of the last five name bytes, so a
// long name reaching into the zone bytes still reads. A name cut with a
// dot left ".-1" there and GFA's VAL read that as zero, the port takes
// the trailing signed number instead
std::optional<double> PlaceRecord::zone_to_ut() const {
  const std::size_t take = name.size() < 5 ? name.size() : 5;
  std::string kept;
  for (const char c : name.substr(name.size() - take)) {
    // CASE 43,44,45,46,48 TO 57
    if (c == '+' || c == ',' || c == '-' || c == '.' || std::isdigit(static_cast<unsigned char>(c)) != 0) {
      kept += c;
    }
  }
  std::size_t start = kept.size();
  while (start > 0 && (std::isdigit(static_cast<unsigned char>(kept[start - 1])) != 0 || kept[start - 1] == '.')) {
    --start;
  }
  const std::string number = kept.substr(start);
  if (number.find_first_of("0123456789") == std::string::npos) {
    return std::nullopt;
  }
  const double value = val(number);
  return start > 0 && kept[start - 1] == '-' ? -value : value;
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
  const auto bytes = read_file_bytes(path);
  if (!bytes || bytes->size() % kPlaceRecordBytes != 0) {
    return std::nullopt;
  }
  std::vector<PlaceRecord> out;
  out.reserve(bytes->size() / kPlaceRecordBytes);
  for (std::size_t off = 0; off < bytes->size(); off += kPlaceRecordBytes) {
    out.push_back(decode_place_record(std::string_view(*bytes).substr(off, kPlaceRecordBytes)));
  }
  return out;
}

bool write_place_file(const std::filesystem::path& path, const std::vector<PlaceRecord>& records) {
  std::string bytes;
  bytes.reserve(records.size() * kPlaceRecordBytes);
  for (const PlaceRecord& r : records) {
    bytes += encode_place_record(r);
  }
  return replace_file(path, bytes);
}

// ported from the EINTRAGEN branch of a2ort. His PUT added one record to
// the open file, a file the reader refuses is never rewritten with the
// new place alone
bool append_place(const std::filesystem::path& path, const PlaceRecord& record) {
  std::vector<PlaceRecord> records;
  std::error_code ec;
  if (std::filesystem::exists(path, ec)) {
    const auto existing = read_place_file(path);
    if (!existing) {
      return false;
    }
    records = *existing;
  }
  records.push_back(record);
  return write_place_file(path, records);
}

std::optional<PlaceRecord> read_preferred_place(const std::filesystem::path& path) {
  const auto file = read_file_bytes(path);
  if (!file || file->size() < kPlaceRecordBytes) {
    return std::nullopt;
  }
  const std::string& bytes = *file;
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

void trim_places(std::vector<PlaceRecord>& places) {
  delete_places(places, {});
}

// ported from a2f_tr_ort
void delete_places(std::vector<PlaceRecord>& places, const std::vector<std::size_t>& doomed) {
  std::vector<PlaceRecord> kept;
  kept.reserve(places.size());
  for (std::size_t i = 0; i < places.size(); ++i) {
    if (std::find(doomed.begin(), doomed.end(), i) != doomed.end()) {
      continue;
    }
    const PlaceRecord& p = places[i];
    const std::string name = trimmed(p.name);
    // IF ABS(VAL(ggl$)) > kk && ASC(goo$) > 31
    const bool placed = std::abs(p.lon) > kEps || std::abs(p.lat) > kEps;
    if (placed && !name.empty() && static_cast<unsigned char>(name.front()) > 31) {
      kept.push_back(p);
    }
  }
  places = std::move(kept);
}

// ported from ortp
bool write_preferred_place(const std::filesystem::path& path, const PlaceRecord& place) {
  // his STR$(CINT(gl * 1000000),8,0) overflows the eight bytes west of
  // ten degrees, a decimal then fills the field, the reader takes both
  const auto field = [](double deg) {
    const long long micro = std::llround(deg * 1000000.0);
    std::string s = std::to_string(micro);
    if (s.size() > kLonLen) {
      char buf[32];
      std::snprintf(buf, sizeof(buf), "%.*f", 4, deg);
      s = buf;
      while (s.size() > kLonLen && s.back() != '.') {
        s.pop_back();
      }
    }
    return rset(s, kLonLen);
  };
  std::string rec = field(place.lon) + field(place.lat);
  const std::string name = utf8_to_cp1252(place.name);
  for (std::size_t i = 0; i < kNameLen; ++i) {
    rec += (i < name.size()) ? name[i] : ' ';
  }
  return replace_file(path, rec);
}

}  // namespace horcom
