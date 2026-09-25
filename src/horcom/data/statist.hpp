// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "horcom/chart/bodies.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/chart/settings.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/konsta.hpp"

// The statistics store of the original, STATIST7 with one dataset as
// three files. The .STA carries 210 byte records, name, place, a packed
// clock, packed coordinates and 34 int32 angles in radians times ten
// million. The .STH twin adds six more extras per record. The original
// named it after the base cut to seven characters plus a one, so two
// datasets sharing their first seven letters shared one twin. The port
// keeps that name while it is free and gives the whole base plus a one
// to a dataset whose cut name another one holds. The .PAR carries the
// calculation parameters, without it the extra slots mean nothing.
namespace horcom {

/// the record sizes of the original FIELD layouts
inline constexpr std::size_t kStaRecordBytes = 210;
inline constexpr std::size_t kSthRecordBytes = 24;
/// the angle scale asn%, radians times ten million
inline constexpr double kStatAngleScale = 10000000.0;
/// the name and place fields of the .STA records, his na$ and go$
inline constexpr std::size_t kStatNameWidth = 25;
inline constexpr std::size_t kStatPlaceWidth = 20;
/// the extra bodies of his nk&(1) to nk&(22)
inline constexpr int kStatExtraCount = 22;
/// the .STA carries the extras 1 to 16, the .STH the ones after
inline constexpr int kStaExtraLast = 16;
/// the list slots of the four stored cusps H2, H3, H5 and H6 in the
/// statistics menus, his pl$(15) to pl$(18)
inline constexpr int kStatCuspSlotFirst = 15;
inline constexpr int kStatCuspSlotLast = 18;
/// the house of each stored cusp slot, H2, H3, H5 and H6
inline constexpr std::array<int, 4> kStatCuspHouse = {2, 3, 5, 6};

/// @param slot a list slot of the statistics menus
/// @return true for the four stored cusps H2, H3, H5 and H6
[[nodiscard]] constexpr bool stat_cusp_slot(int slot) {
  return slot >= kStatCuspSlotFirst && slot <= kStatCuspSlotLast;
}

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
  /// nk&(1..22) as the file carries it, index 0 unused. Robert numbers
  /// the chosen extras compactly, the k-th chosen one gets 18 + its rank,
  /// so only nk > 0 says whether extra k is present. Its data lives in
  /// the fixed body slot extra_slot(k)
  std::array<int, kStatExtraCount + 1> nk{};
};

/// The fixed body slot of extra body k in the standard layout.
///
/// @param k the extra index 1 to 22, the position in the nk table
/// @return 18 + k, the slot the engine and the tables use
[[nodiscard]] constexpr int extra_slot(int k) { return 18 + k; }

/// Robert's compact nk numbering for a set of chosen extras, the k-th
/// chosen extra gets 18 + its rank like the selection loop of plgen.
///
/// @param chosen per extra index 1 to 22, true when present
/// @return the nk table as his .PAR and KONSTA files carry it
[[nodiscard]] std::array<int, kStatExtraCount + 1> compact_nk(const std::array<bool, kStatExtraCount + 1>& chosen);

/// One chart of a dataset, positions already mapped onto body slots.
struct StatRecord {
  std::string name;   ///< upper case like the writer, at most 25 characters
  std::string place;  ///< upper case, at most 20 characters
  int day = 0;
  int month = 0;
  int year = 0;
  int hour = 0;
  double minute = 0.0;
  double lon = 0.0;  ///< decimal degrees east, two decimals in the file
  double lat = 0.0;  ///< decimal degrees north, two decimals in the file
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

  /// @return true for a record written in the heliocentric mode, the
  ///         Sun, AC, MC and house 2 all zero
  /// @note His stat2 took AC, MC and house 2 at zero alone, so a file
  ///       computed without houses and angles, house systems 9 and 10,
  ///       read as heliocentric and switched the program to HELIO. Only
  ///       the heliocentric writer of stat1 empties the Sun as well
  [[nodiscard]] bool heliocentric() const { return el[body::kSun] == 0.0 && ac == 0.0 && mc == 0.0 && h2 == 0.0; }
};

/// A loaded dataset.
struct StatSet {
  StatParams params;
  std::vector<StatRecord> records;
};

/// A file in a folder matched without regard to ASCII case, the way his
/// DOS and Windows names resolved on every file system.
///
/// @param dir  the folder
/// @param name the file name
/// @return the existing file of that name in any case, else dir / name
[[nodiscard]] std::filesystem::path find_ignoring_case(const std::filesystem::path& dir,
                                                       const std::filesystem::path& name);

/// The .PAR beside a .STA.
///
/// @param sta the <NAME>.STA file
/// @return the existing <NAME>.PAR in any case, else the upper case name
[[nodiscard]] std::filesystem::path par_path(const std::filesystem::path& sta);

/// The .STH twin a dataset writes. His name of the base cut to seven
/// characters plus a one, unless another dataset of the folder shares it
/// or a twin of the whole base name exists, then the whole base name
/// plus a one.
///
/// @param sta the <NAME>.STA file
/// @return the twin, an existing file of that name in any case
[[nodiscard]] std::filesystem::path sth_path(const std::filesystem::path& sta);

/// The .STH twin of the original, the base cut to seven characters plus
/// a one like stat2_teil builds it.
///
/// @param sta the <NAME>.STA file
/// @return the existing twin of that name in any case, else the upper
///         case name
[[nodiscard]] std::filesystem::path legacy_sth_path(const std::filesystem::path& sta);

/// The .STH a dataset reads, the whole base name first, the cut name of
/// the original else.
///
/// @param sta the <NAME>.STA file
/// @return the twin on disk, nothing when neither exists
[[nodiscard]] std::optional<std::filesystem::path> existing_sth_path(const std::filesystem::path& sta);

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

/// Deletes a dataset, its .STA, .PAR and .STH. The cut .STH name of the
/// original goes only when no other dataset of the folder shares it.
///
/// @param sta the <NAME>.STA file
/// @return true when the .STA is gone
bool remove_statistics(const std::filesystem::path& sta);

/// @param set a loaded dataset
/// @return true when a record of it was written in the heliocentric mode,
///         his hrge!
[[nodiscard]] bool stat_heliocentric(const StatSet& set);

/// The .PAR parameters of a dataset about to be computed, ported from the
/// WRITE of stat1 with his runtime labels.
///
/// @param s the settings the records are computed with
/// @param k the profile, it carries his gen& and gena$
/// @return the parameters, haus stays for the first computed chart
[[nodiscard]] StatParams stat_params(const ChartSettings& s, const Konsta& k);

/// The settings a dataset was computed with, ported from stat2parl. The
/// .PAR fields replace those of the profile, the rest stays.
///
/// @param k     the running profile
/// @param p     the parameters of the dataset
/// @param helio true for a heliocentric dataset
/// @return the settings for its charts
[[nodiscard]] ChartSettings stat_chart_settings(Konsta k, const StatParams& p, bool helio);

/// One dataset record from a stored chart and its computation, ported
/// from stat1_0 with stat1_1 and stat1_2.
///
/// @param r     the chart record of the .DAT
/// @param c     the chart computed with the dataset settings
/// @param helio true writes no cusps, his helio file zeroes f(1..13)
/// @return the record, name and place upper case and cut to their fields
[[nodiscard]] StatRecord stat_record(const ChartRecord& r, const Chart& c, bool helio);

/// @param r a dataset record
/// @return its moment and place, the clock of the .STA is UT
[[nodiscard]] ChartInput stat_input(const StatRecord& r);

/// A dataset record as a RADIX record, the clock stays UT.
///
/// @param r a dataset record
/// @return the record with zone 00hE00:00 and whole seconds
[[nodiscard]] AafRecord stat_aaf_record(const StatRecord& r);

}  // namespace horcom
