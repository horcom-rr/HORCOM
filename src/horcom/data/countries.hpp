// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The country tables of the original. INTERN landnima.int carries the
//RR Liste der geographischen country-codes nach NIMA
// that names the two letter prefixes of the big place files, and INTERN
// laender.int carries the German auto style abbreviations of the record
// mask.
namespace horcom {

/// One NIMA gazetteer country, the line shape XX = NAME.
struct NimaCountry {
  std::string code;
  std::string name;
};

/// One German country abbreviation, up to three letters and the name.
struct GermanCountry {
  std::string abbrev;
  std::string name;
};

/// Loads the NIMA code table.
///
/// @param path the landnima.int file
/// @return every code line of the file, or std::nullopt when unreadable
/// @note the original scan of ort_name_discr starts behind its first
///       array slot and never matches Aruba, the rewrite deliberately
///       searches every entry
[[nodiscard]] std::optional<std::vector<NimaCountry>> load_nima_countries(const std::filesystem::path& path);

/// Finds the country name for a two letter code like the original
/// discriminator, case blind, the last match wins.
///
/// @param table the loaded code table
/// @param code the two letter prefix of a place file name
/// @return the country name, or an empty string
[[nodiscard]] std::string nima_country_name(const std::vector<NimaCountry>& table, std::string_view code);

/// Loads the German abbreviation table behind its header row.
///
/// @param path the laender.int file
/// @return the entries, or std::nullopt when unreadable
[[nodiscard]] std::optional<std::vector<GermanCountry>> load_german_countries(const std::filesystem::path& path);

}  // namespace horcom
