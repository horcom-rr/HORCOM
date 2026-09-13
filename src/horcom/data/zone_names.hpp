// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// The zone name catalogue of the original, INTERN zonnamen.int, behind
// the dialog "Zeit-Zonen (P.D. Via B.MAHL)". Fixed column text, the name
// to column 46, the abbreviation from column 47, the last ten characters
// the difference that leads from zone time to UT, so MEZ reads minus
// one. The LMT and LTT rows carry no number, local time follows from the
// longitude instead.
namespace horcom {

/// the rows the original loads, the header plus 176 entries
inline constexpr int kZoneCatalogueRows = 177;

/// One named zone of the catalogue.
struct ZoneEntry {
  std::string name;    // UTF-8
  std::string abbrev;  // GMT, AMT, LMT and so on
  /// the correction added to zone time to reach UT, nothing when the
  /// field is blank
  std::optional<double> to_ut_hours;

  /// @return true for the local time rows, the original's column check
  [[nodiscard]] bool is_local_time() const { return abbrev == "LMT" || abbrev == "LTT"; }
};

/// Parses one catalogue line with the original's column arithmetic.
[[nodiscard]] ZoneEntry parse_zone_line(std::string_view line);

/// Loads the catalogue.
///
/// @param path the zonnamen.int file
/// @return the entries without the header row, or std::nullopt when the
///         file cannot be read
[[nodiscard]] std::optional<std::vector<ZoneEntry>> load_zone_names(const std::filesystem::path& path);

}  // namespace horcom
