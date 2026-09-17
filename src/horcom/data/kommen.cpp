// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/kommen.hpp"

#include <fstream>

#include "horcom/data/encoding.hpp"

namespace horcom {

namespace {

// the titles of the ae entry points, erl$ plus the topic, each with
// the shipped markdown edition and his original text file name
struct MenuRow {
  int index;
  const char* title;
  const char* md;
  const char* file;
};

constexpr MenuRow kMenu[] = {
    {1, "Einführender Kommentar", "komm1.md", "KOMM1.TXT"},
    {2, "Erläuterung Ein-Ausgabe", "komm2.md", "KOMM2.TXT"},
    {3, "Erläuterung Ephemeride", "komm3.md", "KOMM3.TXT"},
    {4, "Erläuterung Horoskope", "komm4.md", "KOMM4.TXT"},
    {5, "Erläuterung Solar,Septar...", "komm5.md", "KOMM5.TXT"},
    {6, "Erläuterung M.R.", "komm6.md", "KOMM6.TXT"},
    {7, "Erläuterung Direktionen", "komm7.md", "KOMM7.TXT"},
    {8, "Erläuterung Häuser", "komm8.md", "KOMM8.TXT"},
    {9, "Erläuterung Diverses", "komm9.md", "KOMM9.TXT"},
    {10, "Änderungsliste", "aendlist.md", "AENDLIST.TXT"},
    {11, "Hinweise", "hinweis5.md", "HINWEIS5.TXT"},
    {12, "Kurzanleitung", "kurzanl5.md", "KURZANL5.TXT"},
    {13, "Erläuterung Statistik", "kommstat.md", "KOMMSTAT.TXT"},
    {14, "Erläuterung AAF-Ein-Ausgabe", "aaf_komm.md", "AAF_KOMM.TXT"},
};

}  // namespace

// ported from HORCOM komm_les and the ae menu titles
std::vector<KommenEntry> kommen_entries(const std::filesystem::path& dir, bool english) {
  std::vector<KommenEntry> out;
  std::error_code ec;
  for (const MenuRow& row : kMenu) {
    // the English edition wins in the English shell, then his markdown
    // edition, then his original text file
    if (english) {
      std::string en = row.md;
      const std::size_t dot = en.rfind(".md");
      if (dot != std::string::npos) {
        en.insert(dot, "_en");
        const std::filesystem::path enp = dir / en;
        if (std::filesystem::exists(enp, ec)) {
          out.push_back({row.index, row.title, enp});
          continue;
        }
      }
    }
    const std::filesystem::path md = dir / row.md;
    if (std::filesystem::exists(md, ec)) {
      out.push_back({row.index, row.title, md});
      continue;
    }
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
