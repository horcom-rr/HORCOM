// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/aaf.hpp"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string_view>

#include "horcom/data/encoding.hpp"

namespace horcom {

namespace {

// legacy files arrive as Windows 1252 which is never valid multi byte
// UTF-8, so a validity scan decides the decoding once per file
bool looks_like_utf8(std::string_view text) {
  std::size_t i = 0;
  while (i < text.size()) {
    const auto b = static_cast<unsigned char>(text[i]);
    std::size_t follow = 0;
    if (b < 0x80) {
      follow = 0;
    } else if ((b & 0xE0) == 0xC0) {
      follow = 1;
    } else if ((b & 0xF0) == 0xE0) {
      follow = 2;
    } else if ((b & 0xF8) == 0xF0) {
      follow = 3;
    } else {
      return false;
    }
    for (std::size_t k = 1; k <= follow; ++k) {
      if (i + k >= text.size() || (static_cast<unsigned char>(text[i + k]) & 0xC0) != 0x80) {
        return false;
      }
    }
    i += follow + 1;
  }
  return true;
}

// the original tag_elim$, removes HTML tags from accidentally saved pages
std::string strip_html(std::string_view line) {
  std::string out;
  bool in_tag = false;
  for (const char c : line) {
    if (c == '<') {
      in_tag = true;
    } else if (c == '>') {
      in_tag = false;
    } else if (!in_tag) {
      out += c;
    }
  }
  return out;
}

std::string trimmed(std::string_view v) {
  std::size_t a = 0;
  std::size_t b = v.size();
  while (a < b && (v[a] == ' ' || v[a] == '\t')) {
    ++a;
  }
  while (b > a && (v[b - 1] == ' ' || v[b - 1] == '\t' || v[b - 1] == '\r')) {
    --b;
  }
  return std::string(v.substr(a, b - a));
}

// a star means no value throughout the format
std::string star(std::string_view v) {
  const std::string t = trimmed(v);
  return t == "*" ? std::string() : t;
}

std::vector<std::string> split(std::string_view v, char sep) {
  std::vector<std::string> out;
  std::string cur;
  for (const char c : v) {
    if (c == sep) {
      out.push_back(cur);
      cur.clear();
    } else {
      cur += c;
    }
  }
  out.push_back(cur);
  return out;
}

// the date field dd.mm.yyyy with an optional j or g calendar suffix
void parse_date(std::string_view v, AafRecord& r) {
  const std::vector<std::string> p = split(v, '.');
  if (p.size() < 3) {
    return;
  }
  r.day = std::atoi(p[0].c_str());
  r.month = std::atoi(p[1].c_str());
  std::string y = trimmed(p[2]);
  if (!y.empty()) {
    const char last = static_cast<char>(std::tolower(static_cast<unsigned char>(y.back())));
    if (last == 'j') {
      r.calendar = Calendar::kJulian;
      y.pop_back();
    } else if (last == 'g') {
      r.calendar = Calendar::kGregorian;
      y.pop_back();
    }
  }
  r.year = std::atoi(y.c_str());
}

// the clock accepts HHhMM:SS and HH:MM:SS like the original separator
// scan over colon and the letter h
void parse_time(std::string_view v, AafRecord& r) {
  int parts[3] = {0, 0, 0};
  int idx = 0;
  std::string cur;
  for (const char c : v) {
    if (c == ':' || c == 'h' || c == 'H') {
      if (idx < 3) {
        parts[idx] = std::atoi(cur.c_str());
      }
      ++idx;
      cur.clear();
    } else {
      cur += c;
    }
  }
  if (idx < 3) {
    parts[idx] = std::atoi(cur.c_str());
  }
  r.hour = parts[0];
  r.minute = parts[1];
  r.second = parts[2];
}

// coordinates like 47N38:00 and 007E40:00
void parse_coord(std::string_view v, int& deg, char& hemi, int& min, int& sec, std::string_view letters) {
  std::string d;
  std::size_t i = 0;
  while (i < v.size() && letters.find(static_cast<char>(std::toupper(static_cast<unsigned char>(v[i])))) == std::string_view::npos) {
    d += v[i];
    ++i;
  }
  deg = std::atoi(d.c_str());
  if (i < v.size()) {
    hemi = static_cast<char>(std::toupper(static_cast<unsigned char>(v[i])));
    ++i;
  }
  std::string m;
  while (i < v.size() && v[i] != ':') {
    m += v[i];
    ++i;
  }
  min = std::atoi(m.c_str());
  if (i < v.size()) {
    sec = std::atoi(std::string(v.substr(i + 1)).c_str());
  }
}

void parse_a93(std::string_view content, AafRecord& r) {
  const std::vector<std::string> f = split(content, ',');
  if (f.size() > 0) {
    r.surname = star(f[0]);
  }
  if (f.size() > 1) {
    r.given = star(f[1]);
  }
  if (f.size() > 2) {
    r.sex = star(f[2]);
  }
  if (f.size() > 3) {
    parse_date(f[3], r);
  }
  if (f.size() > 4) {
    parse_time(star(f[4]), r);
  }
  if (f.size() > 5) {
    r.place = star(f[5]);
  }
  if (f.size() > 6) {
    r.country = star(f[6]);
  }
}

void parse_b93(std::string_view content, AafRecord& r) {
  const std::vector<std::string> f = split(content, ',');
  if (f.size() > 0) {
    r.jd = std::strtod(f[0].c_str(), nullptr);
  }
  if (f.size() > 1 && !star(f[1]).empty()) {
    parse_coord(trimmed(f[1]), r.lat_deg, r.lat_ns, r.lat_min, r.lat_sec, "NS");
  }
  if (f.size() > 2 && !star(f[2]).empty()) {
    parse_coord(trimmed(f[2]), r.lon_deg, r.lon_ew, r.lon_min, r.lon_sec, "EW");
  }
  if (f.size() > 3) {
    r.zone = star(f[3]);
  }
  if (f.size() > 4) {
    r.dst = star(f[4]);
  }
}

}  // namespace

// ported from the zone string composition in zeitzon
std::string aaf_zone(double hours_east) {
  const double za = std::abs(hours_east);
  const int hh = static_cast<int>(za);
  const double rem = (za - hh) * 60.0;
  int mm = static_cast<int>(rem);
  int ss = static_cast<int>((rem - mm) * 60.0 + 0.5);
  if (ss >= 60) {
    ss -= 60;
    ++mm;
  }
  const char side = hours_east < 0.0 ? 'W' : 'E';
  char out[16];
  std::snprintf(out, sizeof(out), "%02dh%c%02d:%02d", hh, side, mm, ss);
  return out;
}

double AafRecord::latitude() const {
  const double v = lat_deg + lat_min / 60.0 + lat_sec / 3600.0;
  return lat_ns == 'S' ? -v : v;
}

double AafRecord::longitude() const {
  const double v = lon_deg + lon_min / 60.0 + lon_sec / 3600.0;
  return lon_ew == 'W' ? -v : v;
}

std::vector<AafRecord> parse_aaf(std::string_view text) {
  const bool utf8_in = looks_like_utf8(text);
  const auto decode = [utf8_in](const std::string& s) { return utf8_in ? s : cp1252_to_utf8(s); };
  std::vector<AafRecord> out;
  AafRecord cur;
  bool open = false;
  bool in_comment = false;
  std::size_t start = 0;
  while (start <= text.size()) {
    const std::size_t end = text.find('\n', start);
    const std::string_view raw = text.substr(start, (end == std::string_view::npos ? text.size() : end) - start);
    start = (end == std::string_view::npos) ? text.size() + 1 : end + 1;

    // the tilde is the original's universal ignore marker
    if (raw.find('~') != std::string_view::npos) {
      continue;
    }
    const std::string line = trimmed(strip_html(raw));
    if (line.empty()) {
      continue;
    }
    if (line[0] != '#') {
      // a plain line after the comment continues the comment
      if (open && in_comment) {
        if (!cur.comment.empty()) {
          cur.comment += ' ';
        }
        cur.comment += decode(line);
      }
      continue;
    }
    in_comment = false;
    const std::size_t colon = line.find(':');
    if (colon == std::string::npos) {
      continue;
    }
    const std::string tag = line.substr(0, colon);
    const std::string content = decode(line.substr(colon + 1));
    if (tag == "#A93") {
      if (open) {
        out.push_back(cur);
      }
      cur = AafRecord{};
      open = true;
      parse_a93(content, cur);
    } else if (!open) {
      continue;
    } else if (tag == "#B93") {
      parse_b93(content, cur);
    } else if (tag == "#COM") {
      cur.comment = star(content);
      in_comment = true;
    } else if (tag == "#VIA") {
      cur.via = star(content);
    } else if (tag == "#SRC") {
      cur.source = star(content);
    } else if (tag == "#GZQ") {
      cur.quality = star(content);
    } else if (tag == "#ZNAM") {
      cur.zone_name = star(content);
    } else if (tag == "#CWORD") {
      cur.catchword = star(content);
    } else if (tag == "#ATTRB") {
      cur.attributes = star(content);
    }
  }
  if (open) {
    out.push_back(cur);
  }
  return out;
}

std::optional<std::vector<AafRecord>> read_aaf(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return std::nullopt;
  }
  std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return parse_aaf(text);
}

std::string format_aaf(const std::vector<AafRecord>& records) {
  std::ostringstream s;
  const auto field = [](const std::string& v) { return v.empty() ? std::string("*") : v; };
  for (const AafRecord& r : records) {
    char date[24];
    const char* suffix = (r.calendar == Calendar::kJulian) ? "j" : (r.calendar == Calendar::kGregorian) ? "g" : "";
    std::snprintf(date, sizeof(date), "%02d.%02d.%d%s", r.day, r.month, r.year, suffix);
    char clock[16];
    std::snprintf(clock, sizeof(clock), "%02dh%02d:%02d", r.hour, r.minute, r.second);
    s << "#A93:" << field(r.surname) << ',' << field(r.given) << ',' << field(r.sex) << ',' << date << ','
      << clock << ',' << field(r.place) << ',' << field(r.country) << "\r\n";
    //RR STR$(VAL(ed$(11)),13,5)
    char jd[24];
    std::snprintf(jd, sizeof(jd), "%13.5f", r.jd);
    char lat[16];
    std::snprintf(lat, sizeof(lat), "%02d%c%02d:%02d", r.lat_deg, r.lat_ns, r.lat_min, r.lat_sec);
    char lon[16];
    std::snprintf(lon, sizeof(lon), "%03d%c%02d:%02d", r.lon_deg, r.lon_ew, r.lon_min, r.lon_sec);
    s << "#B93:" << jd << ',' << lat << ',' << lon << ',' << field(r.zone) << ',' << field(r.dst) << "\r\n";
    const auto tag = [&](const char* name, const std::string& v) {
      if (!v.empty()) {
        s << name << v << "\r\n";
      }
    };
    tag("#COM:", r.comment);
    tag("#VIA:", r.via);
    tag("#SRC:", r.source);
    tag("#GZQ:", r.quality);
    tag("#ZNAM:", r.zone_name);
    tag("#CWORD:", r.catchword);
    tag("#ATTRB:", r.attributes);
  }
  return s.str();
}

bool write_aaf(const std::filesystem::path& path, const std::vector<AafRecord>& records) {
  std::ofstream f(path, std::ios::binary | std::ios::trunc);
  if (!f) {
    return false;
  }
  const std::string text = utf8_to_cp1252(format_aaf(records));
  f.write(text.data(), static_cast<std::streamsize>(text.size()));
  return static_cast<bool>(f);
}

namespace {

bool folder_is(const std::filesystem::path& dir, std::string_view name) {
  std::string d = dir.filename().string();
  if (d.size() != name.size()) {
    return false;
  }
  for (std::size_t i = 0; i < d.size(); ++i) {
    if (std::toupper(static_cast<unsigned char>(d[i])) != name[i]) {
      return false;
    }
  }
  return true;
}

// the basename swap of bilde_aaffile$ and bilde_horcfile$, folder pair
// SPEZIAL and AAFDATEN when his tree is present, siblings otherwise
std::filesystem::path twin(const std::filesystem::path& path, std::string_view own_folder,
                           std::string_view twin_folder, const char* extension) {
  const std::filesystem::path dir = path.parent_path();
  std::filesystem::path stem = path.stem();
  stem += extension;
  if (folder_is(dir, own_folder)) {
    const std::filesystem::path other = dir.parent_path() / twin_folder;
    std::error_code ec;
    if (std::filesystem::is_directory(other, ec)) {
      return other / stem;
    }
  }
  return dir / stem;
}

}  // namespace

std::filesystem::path aaf_twin_path(const std::filesystem::path& dat_path) {
  return twin(dat_path, "SPEZIAL", "AAFDATEN", ".AAF");
}

std::filesystem::path dat_twin_path(const std::filesystem::path& aaf_path) {
  return twin(aaf_path, "AAFDATEN", "SPEZIAL", ".DAT");
}

}  // namespace horcom
