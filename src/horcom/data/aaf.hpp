// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "horcom/time/calendar.hpp"

// The AAF exchange format, the richest of the original's file formats
// and the port's archive format going forward. Line oriented text with
// tagged records, ported from the original procedures make_aaf and
// make_aaf_eing_horc. Robert Rettig's own warning about the format's
// weak point travels with the loader,
//RR Im AAF-Format sind Kommata wichtige Trennzeichen ! Wenn an der
//RR falschen Stelle ein Komma "entsteht",z.B. weil statt eines Umlauts
//RR ein Komma "übersetzt" wird,kann der betr. Datensatz nicht mehr
//RR korrekt interpretiert werden.
namespace horcom {

/// One AAF record, text already UTF-8, empty strings for the * fields.
struct AafRecord {
  std::string surname;
  std::string given;
  std::string sex;         // m or w
  int day = 0;
  int month = 0;
  int year = 0;
  Calendar calendar = Calendar::kAuto;  // a j or g suffix on the year
  int hour = 0;
  int minute = 0;
  int second = 0;          // civil clock time, not UT
  std::string place;
  std::string country;
  double jd = 0.0;         // has PRIORITY over date and time when above zero
  int lat_deg = 0;
  int lat_min = 0;
  int lat_sec = 0;
  char lat_ns = 'N';
  int lon_deg = 0;
  int lon_min = 0;
  int lon_sec = 0;
  char lon_ew = 'E';
  std::string zone;        // like 01hE00:00, verbatim
  std::string dst;         // the code 0 1 2 w h m L or *
  std::string comment;
  std::string via;
  std::string source;
  std::string quality;     // the GZQ Rodden grade
  std::string zone_name;
  std::string catchword;
  std::string attributes;

  /// @return latitude in decimal degrees, south negative
  [[nodiscard]] double latitude() const;

  /// @return longitude in decimal degrees, west negative
  [[nodiscard]] double longitude() const;
};

/// Composes the zone field from hours east of Greenwich, the shape
/// 01hE00:00 with the letter carrying the side, like zeitzon builds it.
///
/// @param hours_east the zone in hours, east positive
/// @return the verbatim zone string
[[nodiscard]] std::string aaf_zone(double hours_east);

/// Parses AAF text.
///
/// Lines containing a tilde are dropped like the original's universal
/// ignore marker, HTML tags are stripped like tag_elim so saved web pages
/// still parse, and plain lines after a comment continue the comment.
///
/// @param text the whole file, Windows 1252 or UTF-8 bytes
/// @return all records in order
[[nodiscard]] std::vector<AafRecord> parse_aaf(std::string_view text);

/// Reads an .AAF file.
[[nodiscard]] std::optional<std::vector<AafRecord>> read_aaf(const std::filesystem::path& path);

/// Formats records exactly like the original make_aaf, the A93 and B93
/// lines with star placeholders, the julian date as STR$(jd,13,5), and
/// only the filled optional tags.
[[nodiscard]] std::string format_aaf(const std::vector<AafRecord>& records);

/// Writes an .AAF file in Windows 1252 like the original.
bool write_aaf(const std::filesystem::path& path, const std::vector<AafRecord>& records);

/// The AAF twin of a chart collection, ported from bilde_aaffile$.
///
/// The original swapped SPEZIAL for AAFDATEN under its fixed root. The
/// port keeps that swap when the collection lives in a SPEZIAL folder
/// with an AAFDATEN folder beside it and otherwise places the twin next
/// to the collection, same base name, AAF extension.
///
/// @param dat_path the .DAT collection
/// @return where its .AAF twin lives or would live
[[nodiscard]] std::filesystem::path aaf_twin_path(const std::filesystem::path& dat_path);

/// The chart collection twin of an AAF file, ported from bilde_horcfile$.
///
/// @param aaf_path the .AAF file
/// @return where its .DAT twin lives or would live
[[nodiscard]] std::filesystem::path dat_twin_path(const std::filesystem::path& aaf_path);

}  // namespace horcom
