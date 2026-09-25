// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/collection.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

#include "horcom/core/constants.hpp"
#include "horcom/data/encoding.hpp"

namespace horcom {

namespace {

// the calendar flags of the remark, his jul$
constexpr std::string_view kJulianFlag = "(JULIAN.)";
constexpr std::string_view kGregorianFlag = "(GREGOR.)";
// LSET goo$ = "NICHT GENANNT !" and LEFT$(goo$,17) of horcom_aaf3
constexpr std::string_view kNoPlace = "NICHT GENANNT !";
constexpr std::size_t kExportPlaceLength = 17;
// the DAT text fields as byte counts
constexpr auto kNameBytes = static_cast<std::size_t>(kDatNameLength);
constexpr auto kPlaceBytes = static_cast<std::size_t>(kDatPlaceLength);

std::string trimmed(std::string_view v) {
  std::size_t a = 0;
  std::size_t b = v.size();
  while (a < b && (v[a] == ' ' || v[a] == '\t')) {
    ++a;
  }
  while (b > a && (v[b - 1] == ' ' || v[b - 1] == '\t')) {
    --b;
  }
  return std::string(v.substr(a, b - a));
}

// UPPER$ over the Windows 1252 bytes of a text
std::string upper_1252(std::string bytes) {
  for (char& ch : bytes) {
    const auto c = static_cast<unsigned char>(ch);
    unsigned char u = c;
    if (c >= 'a' && c <= 'z') {
      u = static_cast<unsigned char>(c - 0x20);
    } else if (c >= 0xE0 && c <= 0xFE && c != 0xF7) {
      // à to þ onto À to Þ, the division sign has no capital
      u = static_cast<unsigned char>(c - 0x20);
    } else if (c == 0x9A || c == 0x9C || c == 0x9E) {
      // š œ ž onto Š Œ Ž
      u = static_cast<unsigned char>(c - 0x10);
    } else if (c == 0xFF) {
      // ÿ onto Ÿ
      u = 0x9F;
    }
    ch = static_cast<char>(u);
  }
  return bytes;
}

bool starts_with_case_blind(std::string_view text, std::string_view head) {
  if (text.size() < head.size()) {
    return false;
  }
  for (std::size_t i = 0; i < head.size(); ++i) {
    if (std::toupper(static_cast<unsigned char>(text[i])) != std::toupper(static_cast<unsigned char>(head[i]))) {
      return false;
    }
  }
  return true;
}

// LSET x$ into a field of 25 blanks
std::string padded(std::string s, std::size_t length) {
  if (s.size() < length) {
    s.resize(length, ' ');
  }
  return s;
}

// the given name without the star of an empty field
std::string given_plain(const AafRecord& r) {
  std::string g = trimmed(r.given);
  if (g == "*") {
    g.clear();
  }
  return g;
}

// the given name as his AAF line held it, the star for an empty field
std::string given_or_star(const AafRecord& r) {
  const std::string g = given_plain(r);
  return g.empty() ? std::string("*") : g;
}

}  // namespace

std::string record_name(const AafRecord& r) {
  return trimmed(trimmed(r.surname) + " " + given_plain(r));
}

std::string dat_field(std::string_view utf8, std::size_t length) {
  std::string bytes = upper_1252(utf8_to_cp1252(utf8));
  if (bytes.size() > length) {
    bytes.resize(length);
  }
  return cp1252_to_utf8(bytes);
}

// ported from aaf_horcom2
ChartRecord chart_record_from_aaf(const AafRecord& r) {
  const CalendarDate ut = calendar_date(aaf_moment_jd_ut(r), r.calendar);
  ChartRecord c;
  c.day = ut.day;
  c.month = ut.month;
  c.year = ut.year;
  c.hour = ut.hour;
  c.minute = ut.minute;
  c.lon = r.longitude();
  c.lat = r.latitude();
  // LSET naa$ = UPPER$(LEFT$(TRIM$(aaf$(1) + " " + aaf$(2)),25))
  c.name = dat_field(record_name(r), kNameBytes);
  // LSET goo$ = UPPER$(TRIM$(aaf$(6) + g$)), the nation behind a blank
  const std::string nation = trimmed(r.country);
  const bool has_nation = !nation.empty() && nation != "*";
  c.place = dat_field(trimmed(trimmed(r.place) + (has_nation ? " " + nation : std::string())), kPlaceBytes);
  // the calendar flag is stored in front of BEMERKG.
  const std::string_view flag = r.calendar == Calendar::kJulian      ? kJulianFlag
                                : r.calendar == Calendar::kGregorian ? kGregorianFlag
                                                                     : std::string_view();
  c.remark = (!flag.empty() && r.comment.find(flag) == std::string::npos)
                 ? std::string(flag) + " " + r.comment
                 : r.comment;
  return c;
}

// ported from horcom_aaf3, la& = INSTR(naa$," ") splits the name
AafRecord aaf_from_chart_record(const ChartRecord& c) {
  AafRecord a;
  const std::string name = trimmed(c.name);
  const std::size_t blank = name.find(' ');
  if (blank == std::string::npos) {
    // his aaf$(2) = "*", the loader reads the star as an empty field
    a.surname = name;
  } else {
    a.surname = name.substr(0, blank);
    a.given = trimmed(std::string_view(name).substr(blank + 1));
  }
  a.day = c.day;
  a.month = c.month;
  a.year = c.year;
  // the DAT clock is UT, the minute field carries the seconds. His STR$
  // could round to a sixtieth second, the port carries it
  constexpr long long kPerMinute = 60;
  constexpr auto kPerHour = static_cast<long long>(kSecondsPerHour);
  long long seconds = std::llround((c.hour * 60.0 + c.minute) * 60.0);
  if (seconds >= kSecondsPerDay) {
    seconds = kSecondsPerDay - 1;
  }
  a.hour = static_cast<int>(seconds / kPerHour);
  a.minute = static_cast<int>((seconds / kPerMinute) % kPerMinute);
  a.second = static_cast<int>(seconds % kPerMinute);
  a.place = c.place;
  a.comment = c.remark;
  a.calendar = c.calendar();
  a.set_latitude(c.lat);
  a.set_longitude(c.lon);
  a.zone = kUtZoneText;
  return a;
}

AafRecord aaf_export_record(const ChartRecord& c) {
  AafRecord a = aaf_from_chart_record(c);
  std::string place = trimmed(c.place);
  if (place.empty()) {
    place = kNoPlace;
  }
  std::string bytes = utf8_to_cp1252(place);
  if (bytes.size() > kExportPlaceLength) {
    bytes.resize(kExportPlaceLength);
  }
  if (const std::size_t star = bytes.find('*'); star != std::string::npos) {
    bytes.resize(star);
  }
  a.place = trimmed(cp1252_to_utf8(bytes));
  std::string remark = c.remark;
  if (starts_with_case_blind(remark, kJulianFlag) || starts_with_case_blind(remark, kGregorianFlag)) {
    remark = remark.substr(kJulianFlag.size());
  }
  a.comment = trimmed(remark);
  // st$(4) = "#ZNAM:GMT"
  a.zone_name = "GMT";
  return a;
}

std::optional<std::size_t> aaf_ident(const std::vector<AafRecord>& aaf, std::string_view dat_name) {
  // LSET e$ = na_aaf$
  const std::string e = padded(dat_field(trimmed(dat_name), kNameBytes), kNameBytes);
  for (std::size_t i = 0; i < aaf.size(); ++i) {
    // the name as aaf_horcom2 stores it, and his f$ with the star that
    // older files of the port carry
    const std::string plain = padded(dat_field(record_name(aaf[i]), kNameBytes), kNameBytes);
    const std::string star =
        padded(dat_field(trimmed(aaf[i].surname) + " " + given_or_star(aaf[i]), kNameBytes), kNameBytes);
    if (plain == e || star == e) {
      return i;
    }
  }
  for (std::size_t i = 0; i < aaf.size(); ++i) {
    // ELSE IF INSTR(e$,n1$) > 0 && LEN(n2$) = 1
    const std::string n1 = dat_field(trimmed(aaf[i].surname));
    const std::string n2 = dat_field(given_or_star(aaf[i]));
    if (!n1.empty() && e.find(n1) != std::string::npos && utf8_to_cp1252(n2).size() == 1) {
      return i;
    }
  }
  return std::nullopt;
}

std::optional<std::size_t> aaf_same_name(const std::vector<AafRecord>& aaf, const AafRecord& r) {
  const std::string surname = dat_field(trimmed(r.surname));
  const std::string given = dat_field(given_plain(r));
  std::optional<std::size_t> hit;
  for (std::size_t i = 0; i < aaf.size(); ++i) {
    if (dat_field(trimmed(aaf[i].surname)) == surname && dat_field(given_plain(aaf[i])) == given) {
      hit = i;
    }
  }
  // an empty name would sit inside every line, his INSTR asks for both
  if (hit || surname.empty() || given.empty()) {
    return hit;
  }
  // e$ = UPPER$(ed$(0) + "  " + ed$(1)), ae$ = UPPER$(MID$(a$,6,LEN(e$)))
  constexpr std::size_t kTagLength = 5;
  const std::size_t head = utf8_to_cp1252(surname + "  " + given).size();
  for (std::size_t i = 0; i < aaf.size(); ++i) {
    const std::string line = utf8_to_cp1252(format_aaf({aaf[i]}));
    const std::size_t eol = std::min(line.find('\r'), line.size());
    const std::string ae = dat_field(cp1252_to_utf8(line.substr(kTagLength, std::min(head, eol - kTagLength))));
    if (ae.find(surname) != std::string::npos && ae.find(given) != std::string::npos) {
      hit = i;
    }
  }
  return hit;
}

// ported from the IF taa&(j&) > 0 && moo&(j&) > 0 of a200dat and the
// coordinate test of a2f_tr_dat
bool chain_keeps(const AafRecord& r) {
  const bool placed = !(r.longitude() == 0.0 && r.latitude() == 0.0);
  return r.day > 0 && r.month > 0 && placed;
}

bool write_dat_from_aaf(const std::filesystem::path& dat, const std::vector<AafRecord>& aaf) {
  std::vector<ChartRecord> out;
  out.reserve(aaf.size());
  for (const AafRecord& a : aaf) {
    out.push_back(chart_record_from_aaf(a));
  }
  return write_chart_file(dat, out);
}

}  // namespace horcom
