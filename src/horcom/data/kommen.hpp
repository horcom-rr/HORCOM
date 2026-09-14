// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// The commentary texts of the KOMMEN7P folder, Robert Rettig's own
// German essays that the original showed in its TEXT-DATEI LESEN box.
// The repository carries them as markdown editions in the data folder,
// his original text files are still read when a folder holds them.
namespace horcom {

/// One entry of the komm_les menu whose file is present.
struct KommenEntry {
  /// the original menu index f
  int index = 0;
  /// the erle title of the ae entry points
  std::string title;
  std::filesystem::path path;
};

/// Lists the commentary files found under a folder, the komm_les table.
/// KOMM1 to KOMM9 with their Erläuterung titles, KOMMSTAT and AAF_KOMM,
/// and the three root texts AENDLIST, HINWEIS5 and KURZANL5. For every
/// entry a shipped markdown edition is preferred over the original
/// text file of the same topic.
///
/// @param dir the local kommen folder
/// @return the entries in menu order, missing files skipped
[[nodiscard]] std::vector<KommenEntry> kommen_entries(const std::filesystem::path& dir);

/// Reads one commentary with the lese_text rules. Lines carrying a
/// tilde or 256 characters and more are dropped, a line starting with
/// a dash ends the text, the bytes decode from his Windows 1252.
///
/// @param file the text file
/// @return the text with newline joined lines, nullopt when unreadable
[[nodiscard]] std::optional<std::string> read_kommen(const std::filesystem::path& file);

}  // namespace horcom
