// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The places database of the original, SPEZ_ORT *.INT and the BIGFILES
// derived from the NIMA gazetteer,
//RR Liste der geographischen country-codes nach NIMA
// Fixed 36 byte records, longitude and latitude as right aligned decimal
// degrees, twenty bytes of place name. The zone picker files EUROPA.INT
// and WELT.INT carry the time zone difference in the last five name
// bytes, stored as the correction that leads from zone time to UT, MEZ
// appears as minus one.
namespace horcom {

/// the record size of the original FIELD layout
inline constexpr std::size_t kPlaceRecordBytes = 36;

/// One place, name already UTF-8.
struct PlaceRecord {
  double lon = 0.0;  // decimal degrees, east positive
  double lat = 0.0;  // decimal degrees, north positive
  std::string name;  // often Name / CC

  /// Zone difference of the picker files.
  ///
  /// @return the value of the last five name characters, the correction
  ///         added to zone time to reach UT, or nothing when the field
  ///         holds no number
  [[nodiscard]] std::optional<double> zone_to_ut() const;
};

/// Decodes one 36 byte record.
///
/// @param bytes exactly kPlaceRecordBytes raw bytes
/// @return the place with a trimmed UTF-8 name
[[nodiscard]] PlaceRecord decode_place_record(std::string_view bytes);

/// Encodes a record into the original byte layout.
///
/// @param r the place
/// @return kPlaceRecordBytes bytes, the coordinates right aligned, the
///         name cut to twenty bytes of Windows 1252
[[nodiscard]] std::string encode_place_record(const PlaceRecord& r);

/// Reads a whole place file.
///
/// @param path a SPEZ_ORT style .INT file
/// @return all records, or std::nullopt on read errors or a size that is
///         no multiple of the record size
[[nodiscard]] std::optional<std::vector<PlaceRecord>> read_place_file(const std::filesystem::path& path);

/// Writes a whole place file in the original 36 byte layout. The old file
/// is replaced in one step, a failed write leaves it untouched.
///
/// @param path    a SPEZ_ORT style .INT file
/// @param records the places
/// @return true on success
bool write_place_file(const std::filesystem::path& path, const std::vector<PlaceRecord>& records);

/// Appends one place to a place file, ported from the EINTRAGEN branch
/// of a2ort, the file is read, the record added and the file rewritten,
/// a missing file is created. An existing file the reader refuses, too
/// short or no whole number of records, stays untouched.
///
/// @param path   the .INT file
/// @param record the place to add
/// @return true on success, false when the file could not be read or
///         written
bool append_place(const std::filesystem::path& path, const PlaceRecord& record);

/// The Datei TRIMMEN pass over a place file, ported from a2f_tr_ort.
/// A place without a printable name falls out, and one whose longitude
/// and latitude are both zero. His pass tested the longitude alone and
/// dropped every place on the Greenwich meridian, the port asks both
/// coordinates like his Daten-Datei trim.
///
/// @param places edited in place, order kept
void trim_places(std::vector<PlaceRecord>& places);

/// The Datensätze LÖSCHEN pass over a place file, ported from
/// a2f_tr_ort in delete mode. The doomed places fall out and the trim
/// rules run alongside like his single pass.
///
/// @param places edited in place, order kept
/// @param doomed indices into places before the pass
void delete_places(std::vector<PlaceRecord>& places, const std::vector<std::size_t>& doomed);

/// Writes the preferred place file ORT.EXT like ortp, the coordinates
/// as integer micro degrees in eight bytes.
///
/// @param path  the ORT.EXT file
/// @param place the place
/// @return true on success
bool write_preferred_place(const std::filesystem::path& path, const PlaceRecord& place);

/// Reads the single record preferred place file ORT.EXT.
///
/// The writer of the original stores integer micro degrees while older
/// files hold plain decimals, the reader accepts both exactly like the
/// original branch on the missing decimal point.
///
/// @param path the ORT.EXT file
/// @return the place, or std::nullopt
[[nodiscard]] std::optional<PlaceRecord> read_preferred_place(const std::filesystem::path& path);

}  // namespace horcom
