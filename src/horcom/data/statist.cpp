// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/statist.hpp"

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "horcom/data/encoding.hpp"
#include "horcom/data/gfa_stream.hpp"

namespace horcom {

namespace {

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

// LSET into a fixed field, upper cased like the writer, 1252 bytes
std::string lset_upper(const std::string& utf8, std::size_t len) {
  std::string s = utf8_to_cp1252(utf8);
  for (char& c : s) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
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

}  // namespace

// ported from stat2_teil
std::filesystem::path sth_path(const std::filesystem::path& sta) {
  std::string stem = sta.stem().string();
  if (stem.size() > 8) {
    stem.resize(8);
  }
  std::filesystem::path out = sta;
  out.replace_filename(stem + "1.STH");
  return out;
}

std::optional<StatSet> load_statistics(const std::filesystem::path& sta) {
  StatSet set;
  // the parameters first, the slot table rules the extras
  std::filesystem::path par = sta;
  par.replace_extension(".PAR");
  const auto par_text = read_all(par);
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
    for (int i = 1; i <= 22; ++i) {
      p.nk[static_cast<std::size_t>(i)] = r.next_int();
    }
  }

  const auto sta_bytes = read_all(sta);
  if (!sta_bytes || sta_bytes->size() % kStaRecordBytes != 0) {
    return std::nullopt;
  }
  const auto sth_bytes = read_all(sth_path(sta));

  const std::size_t count = sta_bytes->size() / kStaRecordBytes;
  set.records.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    const std::string_view rec(sta_bytes->data() + i * kStaRecordBytes, kStaRecordBytes);
    StatRecord r;
    r.name = cp1252_to_utf8(trimmed(rec.substr(0, 25)));
    r.place = cp1252_to_utf8(trimmed(rec.substr(25, 20)));
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
    if (sth_bytes && sth_bytes->size() >= (i + 1) * kSthRecordBytes) {
      const auto* q = reinterpret_cast<const unsigned char*>(sth_bytes->data()) + i * kSthRecordBytes;
      for (int k = 17; k <= 22; ++k, q += 4) {
        const int slot = set.params.nk[static_cast<std::size_t>(k)];
        if (slot > 0 && slot <= 40) {
          r.el[static_cast<std::size_t>(slot)] = read_long(q) / kStatAngleScale;
        }
      }
    }
    set.records.push_back(std::move(r));
  }
  return set;
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
    for (int i = 1; i <= 22; ++i) {
      out += gfa_number(set.params.nk[static_cast<std::size_t>(i)]);
      out += "\r\n";
    }
    std::filesystem::path par = sta;
    par.replace_extension(".PAR");
    std::ofstream f(par, std::ios::binary);
    if (!f) {
      return false;
    }
    f << out;
  }

  std::string sta_out;
  std::string sth_out;
  for (const StatRecord& r : set.records) {
    std::string rec = lset_upper(r.name, 25) + lset_upper(r.place, 20);
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
    for (int k = 17; k <= 22; ++k) {
      const int slot = set.params.nk[static_cast<std::size_t>(k)];
      write_long(sth_out, slot > 0 && slot <= 40 ? r.el[static_cast<std::size_t>(slot)] : 0.0);
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

}  // namespace horcom
