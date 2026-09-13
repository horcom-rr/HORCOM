// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>
#include <string>
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
[[nodiscard]] PlaceRecord decode_place_record(std::string_view bytes);

/// Encodes a record into the original byte layout.
[[nodiscard]] std::string encode_place_record(const PlaceRecord& r);

/// Reads a whole place file.
///
/// @param path a SPEZ_ORT style .INT file
/// @return all records, or std::nullopt on read errors or a size that is
///         no multiple of the record size
[[nodiscard]] std::optional<std::vector<PlaceRecord>> read_place_file(const std::filesystem::path& path);

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
