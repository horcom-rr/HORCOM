// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/statist.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "horcom/core/constants.hpp"
#include "horcom/data/encoding.hpp"
#include "horcom/data/gfa_stream.hpp"

namespace horcom {

namespace {

// the characters of the stem his LEFT$(daa$,8) kept after the backslash
constexpr std::size_t kLegacySthStem = 7;
// the clock of a record keeps hundredths of a minute
constexpr long long kSecondsPerMinute = 60;

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

std::int32_t read_long(const unsigned char* p) {
  return static_cast<std::int32_t>(static_cast<std::uint32_t>(p[0]) |
                                   (static_cast<std::uint32_t>(p[1]) << 8) |
                                   (static_cast<std::uint32_t>(p[2]) << 16) |
                                   (static_cast<std::uint32_t>(p[3]) << 24));
}

void write_long(std::string& out, double el_rad) {
  const auto v = static_cast<std::int32_t>(std::llround(el_rad * kStatAngleScale));
  const auto u = static_cast<std::uint32_t>(v);
  out += static_cast<char>(u & 0xFF);
  out += static_cast<char>((u >> 8) & 0xFF);
  out += static_cast<char>((u >> 16) & 0xFF);
  out += static_cast<char>((u >> 24) & 0xFF);
}

// UPPER$ on one Windows 1252 byte, the letters with accents included,
// the sharp s has no capital there
char upper_1252(char c) {
  const auto u = static_cast<unsigned char>(c);
  if ((u >= 'a' && u <= 'z') || (u >= 0xE0 && u <= 0xFE && u != 0xF7)) {
    return static_cast<char>(u - 0x20);
  }
  switch (u) {
    case 0x9A: return static_cast<char>(0x8A);  // š
    case 0x9C: return static_cast<char>(0x8C);  // œ
    case 0x9E: return static_cast<char>(0x8E);  // ž
    case 0xFF: return static_cast<char>(0x9F);  // ÿ
    default: return c;
  }
}

// the field bytes of LEFT$(TRIM$(UPPER$(text)),len), Windows 1252
std::string upper_field(const std::string& utf8, std::size_t len) {
  std::string s = utf8_to_cp1252(trimmed(utf8));
  std::transform(s.begin(), s.end(), s.begin(), upper_1252);
  if (s.size() > len) {
    s.resize(len);
  }
  return s;
}

// LSET into a fixed field, upper cased like the writer
std::string lset_upper(const std::string& utf8, std::size_t len) {
  std::string s = upper_field(utf8, len);
  s.resize(len, ' ');
  return s;
}

std::optional<std::string> read_all(const std::filesystem::path& p) {
  std::ifstream f(p, std::ios::binary);
  if (!f) {
    return std::nullopt;
  }
  return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// the lower case of the ASCII letters, the key his case blind names
// compare by
std::u8string ascii_lower(std::u8string s) {
  for (char8_t& c : s) {
    if (c >= u8'A' && c <= u8'Z') {
      c = static_cast<char8_t>(c - u8'A' + u8'a');
    }
  }
  return s;
}

// the first n characters of a UTF-8 name, a character never splits
std::u8string first_chars(const std::u8string& s, std::size_t n) {
  std::size_t i = 0;
  for (std::size_t chars = 0; i < s.size() && chars < n; ++chars) {
    ++i;
    while (i < s.size() && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) {
      ++i;
    }
  }
  return s.substr(0, i);
}

// the base name of a .STA with a suffix, as a path of that folder
std::filesystem::path beside(const std::filesystem::path& sta, const std::u8string& name) {
  return find_ignoring_case(sta.parent_path(), std::filesystem::path(name));
}

// the twin of stat2_teil. His daa$ carried the leading backslash, so
// LEFT$(daa$,8) keeps seven letters of the name and the 1 completes a
// DOS name of eight
std::u8string legacy_sth_name(const std::filesystem::path& sta) {
  return first_chars(sta.stem().u8string(), kLegacySthStem) + u8"1.STH";
}

// the twin of the whole base name
std::u8string full_sth_name(const std::filesystem::path& sta) {
  return sta.stem().u8string() + u8"1.STH";
}

// another .STA of the folder whose cut twin meets this one's
bool legacy_sth_shared(const std::filesystem::path& sta) {
  const std::u8string mine = ascii_lower(legacy_sth_name(sta));
  const std::u8string own = ascii_lower(sta.filename().u8string());
  std::error_code ec;
  for (const auto& e : std::filesystem::directory_iterator(sta.parent_path(), ec)) {
    if (ascii_lower(e.path().filename().u8string()) == own || ascii_lower(e.path().extension().u8string()) != u8".sta") {
      continue;
    }
    if (ascii_lower(legacy_sth_name(e.path())) == mine) {
      return true;
    }
  }
  return false;
}

}  // namespace

std::filesystem::path find_ignoring_case(const std::filesystem::path& dir, const std::filesystem::path& name) {
  const std::filesystem::path exact = dir / name;
  std::error_code ec;
  if (std::filesystem::exists(exact, ec)) {
    return exact;
  }
  const std::u8string key = ascii_lower(name.u8string());
  for (const auto& e : std::filesystem::directory_iterator(dir, ec)) {
    if (ascii_lower(e.path().filename().u8string()) == key) {
      return e.path();
    }
  }
  return exact;
}

std::filesystem::path par_path(const std::filesystem::path& sta) {
  return beside(sta, sta.stem().u8string() + u8".PAR");
}

// ported from stat2_teil. Two datasets sharing their first seven letters
// shared his twin, the second one takes the whole base name
std::filesystem::path sth_path(const std::filesystem::path& sta) {
  const std::filesystem::path full = beside(sta, full_sth_name(sta));
  std::error_code ec;
  if (std::filesystem::exists(full, ec) || legacy_sth_shared(sta)) {
    return full;
  }
  return legacy_sth_path(sta);
}

std::filesystem::path legacy_sth_path(const std::filesystem::path& sta) {
  return beside(sta, legacy_sth_name(sta));
}

std::optional<std::filesystem::path> existing_sth_path(const std::filesystem::path& sta) {
  std::error_code ec;
  for (const std::filesystem::path& p : {beside(sta, full_sth_name(sta)), legacy_sth_path(sta)}) {
    if (std::filesystem::exists(p, ec)) {
      return p;
    }
  }
  return std::nullopt;
}

std::optional<StatSet> load_statistics(const std::filesystem::path& sta) {
  StatSet set;
  // the parameters first, the slot table rules the extras
  const auto par_text = read_all(par_path(sta));
  if (!par_text) {
    return std::nullopt;
  }
  {
    GfaReader r(*par_text);
    StatParams& p = set.params;
    p.haw = r.next_int();
    p.haus = cp1252_to_utf8(r.next_string());
    p.appa = r.next_int();
    p.appa_name = cp1252_to_utf8(r.next_string());
    p.gen = r.next_int();
    p.gena = cp1252_to_utf8(r.next_string());
    p.apogw = r.next_bool();
    p.moknw = r.next_bool();
    p.par = r.next_number();
    for (int i = 1; i <= kStatExtraCount; ++i) {
      p.nk[static_cast<std::size_t>(i)] = r.next_int();
    }
  }

  const auto sta_bytes = read_all(sta);
  if (!sta_bytes || sta_bytes->size() % kStaRecordBytes != 0) {
    return std::nullopt;
  }
  const std::optional<std::filesystem::path> sth = existing_sth_path(sta);
  const auto sth_bytes = sth ? read_all(*sth) : std::nullopt;

  const std::size_t count = sta_bytes->size() / kStaRecordBytes;
  set.records.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    const std::string_view rec(sta_bytes->data() + i * kStaRecordBytes, kStaRecordBytes);
    StatRecord r;
    r.name = cp1252_to_utf8(trimmed(rec.substr(0, kStatNameWidth)));
    r.place = cp1252_to_utf8(trimmed(rec.substr(kStatNameWidth, kStatPlaceWidth)));
    const std::string_view da = rec.substr(45, 16);
    r.day = static_cast<int>(val(da.substr(0, 2)));
    r.month = static_cast<int>(val(da.substr(2, 2)));
    r.year = static_cast<int>(val(da.substr(4, 5)));
    r.hour = static_cast<int>(val(da.substr(9, 2)));
    r.minute = val(da.substr(11, 5));
    const std::string_view gf = rec.substr(61, 13);
    r.lon = val(gf.substr(0, 7));
    r.lat = val(gf.substr(7, 6));
    const auto* p = reinterpret_cast<const unsigned char*>(rec.data()) + 74;
    for (int col = 8; col <= 41; ++col, p += 4) {
      const double v = read_long(p) / kStatAngleScale;
      switch (col) {
        case 20: r.ac = v; break;
        case 21: r.mc = v; break;
        case 22: r.h2 = v; break;
        case 23: r.h3 = v; break;
        case 24: r.h5 = v; break;
        case 25: r.h6 = v; break;
        default: r.el[static_cast<std::size_t>(col - 7)] = v; break;
      }
    }
    // field k carries extra k whatever its compact number, an unchosen
    // extra wrote el(0) there and means nothing
    for (int k = 1; k <= kStaExtraLast; ++k) {
      if (set.params.nk[static_cast<std::size_t>(k)] <= 0) {
        r.el[static_cast<std::size_t>(extra_slot(k))] = 0.0;
      }
    }
    if (sth_bytes && sth_bytes->size() >= (i + 1) * kSthRecordBytes) {
      const auto* q = reinterpret_cast<const unsigned char*>(sth_bytes->data()) + i * kSthRecordBytes;
      for (int k = kStaExtraLast + 1; k <= kStatExtraCount; ++k, q += 4) {
        if (set.params.nk[static_cast<std::size_t>(k)] > 0) {
          r.el[static_cast<std::size_t>(extra_slot(k))] = read_long(q) / kStatAngleScale;
        }
      }
    }
    set.records.push_back(std::move(r));
  }
  return set;
}

// ported from the selection loop of plgen, nk&(i&) = 18 + z&
std::array<int, kStatExtraCount + 1> compact_nk(const std::array<bool, kStatExtraCount + 1>& chosen) {
  std::array<int, kStatExtraCount + 1> nk{};
  int z = 0;
  for (int i = 1; i <= kStatExtraCount; ++i) {
    if (chosen[static_cast<std::size_t>(i)]) {
      ++z;
      nk[static_cast<std::size_t>(i)] = 18 + z;
    }
  }
  return nk;
}

bool save_statistics(const std::filesystem::path& sta, const StatSet& set) {
  // the parameters like the writer at stat1, the gena keeps its comma
  {
    const auto q = [](const std::string& utf8) { return gfa_quoted(utf8_to_cp1252(utf8)); };
    std::string out;
    out += gfa_write_line({gfa_number(set.params.haw), q(set.params.haus), gfa_number(set.params.appa),
                           q(set.params.appa_name), gfa_number(set.params.gen), q(set.params.gena),
                           gfa_bool(set.params.apogw), gfa_bool(set.params.moknw), gfa_number(set.params.par)});
    out += "\r\n";
    std::array<bool, kStatExtraCount + 1> chosen{};
    for (int i = 1; i <= kStatExtraCount; ++i) {
      chosen[static_cast<std::size_t>(i)] = set.params.nk[static_cast<std::size_t>(i)] > 0;
    }
    const std::array<int, kStatExtraCount + 1> nk = compact_nk(chosen);
    for (int i = 1; i <= kStatExtraCount; ++i) {
      out += gfa_number(nk[static_cast<std::size_t>(i)]);
      out += "\r\n";
    }
    std::ofstream f(par_path(sta), std::ios::binary);
    if (!f) {
      return false;
    }
    f << out;
  }

  std::string sta_out;
  std::string sth_out;
  for (const StatRecord& r : set.records) {
    std::string rec = lset_upper(r.name, kStatNameWidth) + lset_upper(r.place, kStatPlaceWidth);
    char da[24];
    std::snprintf(da, sizeof(da), "%2d%2d%5d%2d%5.2f", r.day, r.month, r.year, r.hour, r.minute);
    rec.append(da, 16);
    char gf[16];
    std::snprintf(gf, sizeof(gf), "%7.2f%6.2f", r.lon, r.lat);
    rec.append(gf, 13);
    for (int col = 8; col <= 41; ++col) {
      double v = 0.0;
      switch (col) {
        case 20: v = r.ac; break;
        case 21: v = r.mc; break;
        case 22: v = r.h2; break;
        case 23: v = r.h3; break;
        case 24: v = r.h5; break;
        case 25: v = r.h6; break;
        default: v = r.el[static_cast<std::size_t>(col - 7)]; break;
      }
      write_long(rec, v);
    }
    sta_out += rec;
    for (int k = kStaExtraLast + 1; k <= kStatExtraCount; ++k) {
      const bool chosen = set.params.nk[static_cast<std::size_t>(k)] > 0;
      write_long(sth_out, chosen ? r.el[static_cast<std::size_t>(extra_slot(k))] : 0.0);
    }
  }
  {
    std::ofstream f(sta, std::ios::binary);
    if (!f) {
      return false;
    }
    f.write(sta_out.data(), static_cast<std::streamsize>(sta_out.size()));
  }
  {
    std::ofstream f(sth_path(sta), std::ios::binary);
    if (!f) {
      return false;
    }
    f.write(sth_out.data(), static_cast<std::streamsize>(sth_out.size()));
  }
  return true;
}

// ported from stat3. His KILL of the .STH named the cut twin, deleting
// one of two datasets that shared their first seven letters took the
// extras of the other
bool remove_statistics(const std::filesystem::path& sta) {
  std::error_code ec;
  const std::filesystem::path par = par_path(sta);
  const std::filesystem::path full = beside(sta, full_sth_name(sta));
  const std::filesystem::path legacy = legacy_sth_path(sta);
  const bool shared = legacy_sth_shared(sta);
  std::filesystem::remove(sta, ec);
  const bool gone = !std::filesystem::exists(sta, ec);
  std::filesystem::remove(par, ec);
  std::filesystem::remove(full, ec);
  if (!shared) {
    std::filesystem::remove(legacy, ec);
  }
  return gone;
}

bool stat_heliocentric(const StatSet& set) {
  return std::any_of(set.records.begin(), set.records.end(), [](const StatRecord& r) { return r.heliocentric(); });
}

StatParams stat_params(const ChartSettings& s, const Konsta& k) {
  StatParams p;
  p.haw = static_cast<int>(s.houses);
  // his appa$ reads App.1, App.2 or Wahr
  p.appa = static_cast<int>(s.apparent);
  p.appa_name = s.apparent == ApparentMode::kLightTime              ? "App.1"
                : s.apparent == ApparentMode::kLightTimeAberration ? "App.2"
                                                                     : "Wahr";
  // WRITE #24,...,gen&,gena$, the labels the program runs with
  p.gen = k.gen;
  p.gena = k.gena;
  p.apogw = s.true_apogee;
  p.moknw = s.true_node;
  // par = 1 with the parallax, 2 without
  //RR ohne Par.
  p.par = s.topocentric_parallax ? 1.0 : 2.0;
  p.nk = s.nk;
  return p;
}

// ported from stat2parl
ChartSettings stat_chart_settings(Konsta k, const StatParams& p, bool helio) {
  k.haw = p.haw;
  k.haus = p.haus;
  // a stray appa& keeps the one of the profile, the engine knows three
  if (p.appa >= static_cast<int>(ApparentMode::kLightTime) && p.appa <= static_cast<int>(ApparentMode::kTrue)) {
    k.appa = p.appa;
    k.appa_name = p.appa_name;
  }
  k.gen = p.gen;
  k.gena = p.gena;
  k.apogw = p.apogw;
  k.moknw = p.moknw;
  k.par = p.par;
  k.nk = p.nk;
  ChartSettings s = k.chart_settings();
  s.heliocentric = helio;
  return s;
}

// ported from stat1_0 with stat1_1 and stat1_2
StatRecord stat_record(const ChartRecord& r, const Chart& c, bool helio) {
  StatRecord rec;
  // LSET na$ = LEFT$(TRIM$(UPPER$(naa$)),25)
  rec.name = cp1252_to_utf8(upper_field(r.name, kStatNameWidth));
  rec.place = cp1252_to_utf8(upper_field(r.place, kStatPlaceWidth));
  rec.day = r.day;
  rec.month = r.month;
  rec.year = r.year;
  rec.hour = static_cast<int>(r.hour);
  rec.minute = r.minute;
  rec.lon = r.lon;
  rec.lat = r.lat;
  for (int slot = 1; slot < body::kSlotCount; ++slot) {
    const BodyState& b = c.b[static_cast<std::size_t>(slot)];
    if (b.present && b.valid) {
      rec.el[static_cast<std::size_t>(slot)] = b.el;
    }
  }
  // the helio file writes el(1) = 0 and f(1..13) = 0
  if (!helio && c.houses.ok) {
    rec.ac = c.houses.cusp[1];
    rec.mc = c.houses.cusp[10];
    rec.h2 = c.houses.cusp[2];
    rec.h3 = c.houses.cusp[3];
    rec.h5 = c.houses.cusp[5];
    rec.h6 = c.houses.cusp[6];
  }
  return rec;
}

ChartInput stat_input(const StatRecord& r) {
  ChartInput in;
  in.date_ut = {r.day, r.month, r.year, static_cast<double>(r.hour), r.minute};
  in.lon_deg_east = r.lon;
  in.lat_deg = r.lat;
  return in;
}

AafRecord stat_aaf_record(const StatRecord& r) {
  AafRecord a;
  a.surname = r.name;
  a.place = r.place;
  a.day = r.day;
  a.month = r.month;
  a.year = r.year;
  a.hour = r.hour;
  // the hundredths of a minute as whole seconds, a rounded sixtieth
  // carries into the minute
  const long long seconds = std::llround(r.minute * static_cast<double>(kSecondsPerMinute));
  a.minute = static_cast<int>(seconds / kSecondsPerMinute);
  a.second = static_cast<int>(seconds % kSecondsPerMinute);
  a.zone = kUtZoneText;
  const Dms lat = split_dms(r.lat);
  a.lat_deg = lat.deg;
  a.lat_min = lat.min;
  a.lat_sec = lat.sec;
  a.lat_ns = r.lat < 0.0 ? 'S' : 'N';
  const Dms lon = split_dms(r.lon);
  a.lon_deg = lon.deg;
  a.lon_min = lon.min;
  a.lon_sec = lon.sec;
  a.lon_ew = r.lon < 0.0 ? 'W' : 'E';
  return a;
}

}  // namespace horcom
