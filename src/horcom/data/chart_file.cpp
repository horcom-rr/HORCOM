// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/chart_file.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "horcom/data/encoding.hpp"

namespace horcom {

namespace {

// field layout of the original FIELD #20 statement, offset and length
struct FieldSpec {
  std::size_t off;
  std::size_t len;
};
constexpr FieldSpec kDay{0, 2};
constexpr FieldSpec kMonth{2, 2};
constexpr FieldSpec kYear{4, 5};
constexpr FieldSpec kHour{9, 2};
constexpr FieldSpec kMinute{11, 5};
constexpr FieldSpec kLon{16, 8};
constexpr FieldSpec kLat{24, 8};
constexpr FieldSpec kName{32, 25};
constexpr FieldSpec kPlace{57, 20};
constexpr FieldSpec kRemark{77, 51};

std::string_view slice(std::string_view bytes, FieldSpec f) {
  return bytes.substr(f.off, f.len);
}

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

// GFA VAL, leading spaces allowed, stops at the first unusable character
double val(std::string_view v) {
  return std::strtod(std::string(v).c_str(), nullptr);
}

// numbers as the original RSET STR$(x) writes them, compact and right
// aligned in the field
void rset_number(std::string& rec, FieldSpec f, double v, bool integral) {
  char buf[32];
  if (integral) {
    std::snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(v));
  } else {
    std::snprintf(buf, sizeof(buf), "%.*g", static_cast<int>(f.len) - 1, v);
  }
  std::string s = buf;
  if (s.size() > f.len) {
    s = s.substr(0, f.len);
  }
  const std::size_t pad = f.len - s.size();
  for (std::size_t i = 0; i < pad; ++i) {
    rec[f.off + i] = ' ';
  }
  for (std::size_t i = 0; i < s.size(); ++i) {
    rec[f.off + pad + i] = s[i];
  }
}

// text as LSET writes it, left aligned, space padded, truncated
void lset_text(std::string& rec, FieldSpec f, const std::string& utf8) {
  const std::string bytes = utf8_to_cp1252(utf8);
  for (std::size_t i = 0; i < f.len; ++i) {
    rec[f.off + i] = (i < bytes.size()) ? bytes[i] : ' ';
  }
}

}  // namespace

Calendar ChartRecord::calendar() const {
  if (remark.rfind("(JULIAN.)", 0) == 0) {
    return Calendar::kJulian;
  }
  if (remark.rfind("(GREGOR.)", 0) == 0) {
    return Calendar::kGregorian;
  }
  return Calendar::kAuto;
}

ChartRecord decode_chart_record(std::string_view bytes) {
  ChartRecord r;
  r.day = static_cast<int>(val(slice(bytes, kDay)));
  r.month = static_cast<int>(val(slice(bytes, kMonth)));
  r.year = static_cast<int>(val(slice(bytes, kYear)));
  r.hour = val(slice(bytes, kHour));
  r.minute = val(slice(bytes, kMinute));
  r.lon = val(slice(bytes, kLon));
  r.lat = val(slice(bytes, kLat));
  r.name = cp1252_to_utf8(trimmed(slice(bytes, kName)));
  r.place = cp1252_to_utf8(trimmed(slice(bytes, kPlace)));
  r.remark = cp1252_to_utf8(trimmed(slice(bytes, kRemark)));
  return r;
}

std::string encode_chart_record(const ChartRecord& r) {
  std::string rec(kChartRecordBytes, ' ');
  rset_number(rec, kDay, r.day, true);
  rset_number(rec, kMonth, r.month, true);
  rset_number(rec, kYear, r.year, true);
  rset_number(rec, kHour, r.hour, true);
  rset_number(rec, kMinute, r.minute, false);
  rset_number(rec, kLon, r.lon, false);
  rset_number(rec, kLat, r.lat, false);
  lset_text(rec, kName, r.name);
  lset_text(rec, kPlace, r.place);
  lset_text(rec, kRemark, r.remark);
  return rec;
}

std::optional<std::vector<ChartRecord>> read_chart_file(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return std::nullopt;
  }
  std::string bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  if (bytes.size() % kChartRecordBytes != 0) {
    return std::nullopt;
  }
  std::vector<ChartRecord> out;
  out.reserve(bytes.size() / kChartRecordBytes);
  for (std::size_t off = 0; off < bytes.size(); off += kChartRecordBytes) {
    out.push_back(decode_chart_record(std::string_view(bytes).substr(off, kChartRecordBytes)));
  }
  return out;
}

bool write_chart_file(const std::filesystem::path& path, const std::vector<ChartRecord>& records) {
  std::ofstream f(path, std::ios::binary | std::ios::trunc);
  if (!f) {
    return false;
  }
  for (const ChartRecord& r : records) {
    const std::string rec = encode_chart_record(r);
    f.write(rec.data(), static_cast<std::streamsize>(rec.size()));
  }
  return static_cast<bool>(f);
}

}  // namespace horcom
