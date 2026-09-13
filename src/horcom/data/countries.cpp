// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/countries.hpp"

#include <cctype>
#include <fstream>

#include "horcom/data/encoding.hpp"

namespace horcom {

namespace {

constexpr std::size_t kAbbrevCols = 3;

std::string trimmed(std::string_view v) {
  std::size_t a = 0;
  std::size_t b = v.size();
  while (a < b && (v[a] == ' ' || v[a] == '\r')) {
    ++a;
  }
  while (b > a && (v[b - 1] == ' ' || v[b - 1] == '\r')) {
    --b;
  }
  return std::string(v.substr(a, b - a));
}

char upper(char c) {
  return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

}  // namespace

// ported from land_List
std::optional<std::vector<NimaCountry>> load_nima_countries(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    //RR LAENDER-Datei ( NIMA ) fehlt !
    return std::nullopt;
  }
  std::vector<NimaCountry> out;
  std::string line;
  while (std::getline(f, line)) {
    const std::size_t sep = line.find(" = ");
    if (sep == std::string::npos) {
      continue;
    }
    NimaCountry c;
    c.code = trimmed(std::string_view(line).substr(0, sep));
    c.name = cp1252_to_utf8(trimmed(std::string_view(line).substr(sep + 3)));
    out.push_back(std::move(c));
  }
  return out;
}

// ported from ort_name_discr$
std::string nima_country_name(const std::vector<NimaCountry>& table, std::string_view code) {
  std::string prefix;
  for (const char c : code) {
    prefix += upper(c);
  }
  std::string found;
  for (const NimaCountry& c : table) {
    if (c.code == prefix) {
      found = c.name;
    }
  }
  return found;
}

// ported from laender
std::optional<std::vector<GermanCountry>> load_german_countries(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return std::nullopt;
  }
  std::vector<GermanCountry> out;
  std::string line;
  bool header = true;
  while (std::getline(f, line)) {
    if (header) {
      header = false;
      continue;
    }
    if (trimmed(line).empty()) {
      continue;
    }
    GermanCountry c;
    c.abbrev = cp1252_to_utf8(trimmed(std::string_view(line).substr(0, std::min(line.size(), kAbbrevCols))));
    if (line.size() > kAbbrevCols) {
      c.name = cp1252_to_utf8(trimmed(std::string_view(line).substr(kAbbrevCols)));
    }
    out.push_back(std::move(c));
  }
  return out;
}

}  // namespace horcom
