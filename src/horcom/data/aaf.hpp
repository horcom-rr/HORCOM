// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
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

/// An angle as whole degrees, minutes and seconds.
struct Dms {
  int deg = 0;
  int min = 0;
  int sec = 0;
};

/// Splits the absolute value of an angle into degrees, minutes and
/// rounded seconds. The rounding carries into the minutes and degrees,
/// his horcom_aaf3 could write a sixtieth second.
///
/// @param degrees decimal degrees, the sign is dropped
/// @return the three fields
[[nodiscard]] Dms split_dms(double degrees);

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

  /// Stores a latitude in the degree, minute, second and N or S fields.
  ///
  /// @param degrees decimal degrees, south negative
  void set_latitude(double degrees);

  /// Stores a longitude in the degree, minute, second and E or W fields.
  ///
  /// @param degrees decimal degrees, west negative
  void set_longitude(double degrees);
};

/// Composes the zone field from hours east of Greenwich, the shape
/// 01hE00:00 with the letter carrying the side, like zeitzon builds it.
///
/// @param hours_east the zone in hours, east positive
/// @return the verbatim zone string
[[nodiscard]] std::string aaf_zone(double hours_east);

/// Reads the zone field back into hours east of Greenwich, ported from
/// the ZZD branch of aaf_horcom2.
///
/// @param zone the verbatim zone string like 05hE30:00 or 5E
/// @return hours east, west negative, zero without a side letter
[[nodiscard]] double aaf_zone_hours(std::string_view zone);

/// The summer time shift of the dst code, ported from the korr_sommz
/// table of aaf_horcom2.
///
/// @param dst the code, 1 and w one hour, 2 two hours, h half an hour
/// @return hours to add to the zone, zero for 0, *, m, L and unknown codes
[[nodiscard]] double aaf_dst_hours(std::string_view dst);

/// The moment a record stands for, ported from aaf_horcom2. The julian
/// date outranks the clock fields, else the clock of the record's
/// calendar runs back over the zone and the summer time.
///
/// @param r the record
/// @return Julian day UT
[[nodiscard]] double aaf_moment_jd_ut(const AafRecord& r);

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
///
/// @param path the file
/// @return all records in order, or std::nullopt when the file cannot be
///         read
[[nodiscard]] std::optional<std::vector<AafRecord>> read_aaf(const std::filesystem::path& path);

/// Formats records exactly like the original make_aaf, the A93 and B93
/// lines with star placeholders, the julian date as STR$(jd,13,5), and
/// only the filled optional tags.
///
/// @param records the records in file order
/// @return the text with CR LF line ends, still UTF-8
[[nodiscard]] std::string format_aaf(const std::vector<AafRecord>& records);

/// Writes an .AAF file in Windows 1252 like the original. The old file
/// is replaced in one step, a failed write leaves it untouched.
///
/// @param path    the file
/// @param records the records in file order
/// @return true on success
bool write_aaf(const std::filesystem::path& path, const std::vector<AafRecord>& records);

/// The AAF twin of a chart collection, ported from bilde_aaffile$.
///
/// The original swapped SPEZIAL for AAFDATEN under its fixed root. The
/// port keeps that swap when the collection lives in a SPEZIAL folder
/// with an AAFDATEN folder beside it and otherwise places the twin next
/// to the collection, same base name, AAF extension. Folder and file
/// are found regardless of case, a new twin gets the capital extension.
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
