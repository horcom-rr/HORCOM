// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>

#include "horcom/chart/bodies.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// The statistics store of the original, STATIST7 with one dataset as
// three files. The .STA carries 210 byte records, name, place, a packed
// clock, packed coordinates and 34 int32 angles in radians times ten
// million. The .STH twin adds six more extras per record, its name is
// the base cut to eight characters plus a one. The .PAR carries the
// calculation parameters, without it the extra slots mean nothing.
namespace horcom {

/// the record sizes of the original FIELD layouts
inline constexpr std::size_t kStaRecordBytes = 210;
inline constexpr std::size_t kSthRecordBytes = 24;
/// the angle scale asn%, radians times ten million
inline constexpr double kStatAngleScale = 10000000.0;

/// The calculation parameters of a dataset, the .PAR file.
struct StatParams {
  int haw = 1;
  std::string haus = "Placidus";
  int appa = 1;
  std::string appa_name;
  int gen = 2;
  std::string gena;
  bool apogw = false;
  bool moknw = false;
  double par = 0.0;
  std::array<int, 23> nk{};  // nk&(1..22), index 0 unused
};

/// One chart of a dataset, positions already mapped onto body slots.
struct StatRecord {
  std::string name;   // uppercased like the writer
  std::string place;  // uppercased
  int day = 0;
  int month = 0;
  int year = 0;
  int hour = 0;
  double minute = 0.0;
  double lon = 0.0;  // decimal degrees, two decimals in the file
  double lat = 0.0;
  /// body slot positions in radians, slots 1 to 12 always, extras per
  /// the .PAR nk table, zero where the file holds nothing
  std::array<double, body::kSlotCount> el{};
  /// the six stored cusps, AC, MC and houses 2, 3, 5, 6
  double ac = 0.0;
  double mc = 0.0;
  double h2 = 0.0;
  double h3 = 0.0;
  double h5 = 0.0;
  double h6 = 0.0;

  /// @return true when AC, MC and house 2 are all zero, the original's
  ///         heliocentric file detection
  [[nodiscard]] bool heliocentric() const { return ac == 0.0 && mc == 0.0 && h2 == 0.0; }
};

/// A loaded dataset.
struct StatSet {
  StatParams params;
  std::vector<StatRecord> records;
};

/// The .STH twin of a .STA path, base name cut to eight characters plus
/// a one, like stat2_teil builds it.
[[nodiscard]] std::filesystem::path sth_path(const std::filesystem::path& sta);

/// Loads a dataset from its .STA path, the .PAR beside it is required,
/// the .STH twin is read when present.
///
/// @param sta the <NAME>.STA file
/// @return the dataset, or std::nullopt when a file is unreadable or torn
[[nodiscard]] std::optional<StatSet> load_statistics(const std::filesystem::path& sta);

/// Writes a dataset as .STA, .STH and .PAR with the original packing.
///
/// @param sta the target <NAME>.STA path
/// @param set the dataset
/// @return true when all three files were written
bool save_statistics(const std::filesystem::path& sta, const StatSet& set);

}  // namespace horcom
