// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/kommen.hpp"

#include <fstream>

#include "horcom/data/encoding.hpp"

namespace horcom {

namespace {

// the titles of the ae entry points, erl$ plus the topic
struct MenuRow {
  int index;
  const char* title;
  const char* file;
};

constexpr MenuRow kMenu[] = {
    {1, "Einführender Kommentar", "KOMM1.TXT"},
    {2, "Erläuterung Ein-Ausgabe", "KOMM2.TXT"},
    {3, "Erläuterung Ephemeride", "KOMM3.TXT"},
    {4, "Erläuterung Horoskope", "KOMM4.TXT"},
    {5, "Erläuterung Solar,Septar...", "KOMM5.TXT"},
    {6, "Erläuterung M.R.", "KOMM6.TXT"},
    {7, "Erläuterung Direktionen", "KOMM7.TXT"},
    {8, "Erläuterung Häuser", "KOMM8.TXT"},
    {9, "Erläuterung Diverses", "KOMM9.TXT"},
    {10, "Änderungsliste", "AENDLIST.TXT"},
    {11, "Hinweise", "HINWEIS5.TXT"},
    {12, "Kurzanleitung", "KURZANL5.TXT"},
    {13, "Erläuterung Statistik", "KOMMSTAT.TXT"},
    {14, "Erläuterung AAF-Ein-Ausgabe", "AAF_KOMM.TXT"},
};

}  // namespace

// ported from HORCOM komm_les and the ae menu titles
std::vector<KommenEntry> kommen_entries(const std::filesystem::path& dir) {
  std::vector<KommenEntry> out;
  std::error_code ec;
  for (const MenuRow& row : kMenu) {
    const std::filesystem::path p = dir / row.file;
    if (std::filesystem::exists(p, ec)) {
      out.push_back({row.index, row.title, p});
    }
  }
  return out;
}

// ported from HORCOM lese_text
std::optional<std::string> read_kommen(const std::filesystem::path& file) {
  std::ifstream in(file, std::ios::binary);
  if (!in) {
    return std::nullopt;
  }
  std::string out;
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    //RR nicht bei Zeitbest, the commentary handle always stops here
    if (!line.empty() && line.front() == '-') {
      break;
    }
    if (line.size() < 256 && line.find('~') == std::string::npos) {
      out += cp1252_to_utf8(line);
      out += '\n';
    }
  }
  return out;
}

}  // namespace horcom
