// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/chart_file.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>

#include "horcom/data/encoding.hpp"
#include "horcom/data/file_io.hpp"

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
constexpr FieldSpec kName{32, kDatNameLength};
constexpr FieldSpec kPlace{57, kDatPlaceLength};
constexpr FieldSpec kRemark{77, kDatRemarkLength};

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
  // room for the widest %g of any double, GCC cannot bound the field length
  char buf[320];
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

// IF UPPER$(LEFT$(bem$,9)) = "(JULIAN.)" of a2113 and horcom_aaf3
Calendar ChartRecord::calendar() const {
  std::string head = remark.substr(0, std::min<std::size_t>(remark.size(), 9));
  for (char& c : head) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  if (head == "(JULIAN.)") {
    return Calendar::kJulian;
  }
  if (head == "(GREGOR.)") {
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
  const auto bytes = read_file_bytes(path);
  if (!bytes || bytes->size() % kChartRecordBytes != 0) {
    return std::nullopt;
  }
  std::vector<ChartRecord> out;
  out.reserve(bytes->size() / kChartRecordBytes);
  for (std::size_t off = 0; off < bytes->size(); off += kChartRecordBytes) {
    out.push_back(decode_chart_record(std::string_view(*bytes).substr(off, kChartRecordBytes)));
  }
  return out;
}

// his passes wrote a scratch file and swapped it in with NAME, a failed
// write leaves the old collection untouched
bool write_chart_file(const std::filesystem::path& path, const std::vector<ChartRecord>& records) {
  std::string bytes;
  bytes.reserve(records.size() * kChartRecordBytes);
  for (const ChartRecord& r : records) {
    bytes += encode_chart_record(r);
  }
  return replace_file(path, bytes);
}

// ported from a2f_tr_dat, the trim and the delete branch share one
// pass over the file like the original t! switch
void delete_records(std::vector<ChartRecord>& records, const std::vector<std::size_t>& doomed) {
  std::vector<ChartRecord> kept;
  kept.reserve(records.size());
  for (std::size_t i = 0; i < records.size(); ++i) {
    ChartRecord& r = records[i];
    // records without any place coordinates never survive the pass
    if (r.lon == 0.0 && r.lat == 0.0) {
      continue;
    }
    if (std::find(doomed.begin(), doomed.end(), i) != doomed.end()) {
      continue;
    }
    // the original kept a record only while its day field held a value
    if (r.day <= 0) {
      continue;
    }
    const std::size_t first = r.name.find_first_not_of(' ');
    if (first != std::string::npos && first > 0) {
      r.name.erase(0, first);
    }
    kept.push_back(std::move(r));
  }
  records = std::move(kept);
}

// ported from Datei TRIMMEN
void trim_records(std::vector<ChartRecord>& records) {
  delete_records(records, {});
}

namespace {

// case blind trimmed name equality, see remove_records_by_name
bool same_name(const std::string& a, const std::string& b) {
  const auto trim = [](const std::string& s) {
    const std::size_t x = s.find_first_not_of(' ');
    if (x == std::string::npos) {
      return std::string();
    }
    return s.substr(x, s.find_last_not_of(' ') - x + 1);
  };
  const std::string ta = trim(a);
  const std::string tb = trim(b);
  if (ta.size() != tb.size()) {
    return false;
  }
  for (std::size_t i = 0; i < ta.size(); ++i) {
    if (std::toupper(static_cast<unsigned char>(ta[i])) != std::toupper(static_cast<unsigned char>(tb[i]))) {
      return false;
    }
  }
  return true;
}

}  // namespace

// ported from a22ueberschrb
std::size_t remove_records_by_name(std::vector<ChartRecord>& records, const std::string& name) {
  std::vector<ChartRecord> kept;
  kept.reserve(records.size());
  std::size_t removed = 0;
  for (ChartRecord& r : records) {
    if (same_name(r.name, name)) {
      ++removed;
      continue;
    }
    kept.push_back(std::move(r));
  }
  records = std::move(kept);
  return removed;
}

// ported from Datei MINIMIEREN
void minimize_records(std::vector<ChartRecord>& records) {
  std::vector<ChartRecord> kept;
  kept.reserve(records.size());
  for (ChartRecord& r : records) {
    bool duplicate = false;
    for (const ChartRecord& k : kept) {
      //RR mehrfach vorhandene Datensätze mit gleichem Namen und gleicher Geburtszeit
      if (k.name == r.name && k.day == r.day && k.month == r.month && k.year == r.year && k.hour == r.hour &&
          k.minute == r.minute) {
        duplicate = true;
        break;
      }
    }
    if (!duplicate) {
      kept.push_back(std::move(r));
    }
  }
  records = std::move(kept);
}

}  // namespace horcom
