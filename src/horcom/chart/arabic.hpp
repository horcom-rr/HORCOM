// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <vector>

#include "horcom/chart/chart.hpp"

// The arabic parts of the original arabt screen. Every part follows
// the one formula base plus first minus second, on a day birth, and
// swaps the two moving terms on a night birth, the tradition's rule.
// Thirty seven parts ship built in, from the Glückspunkt to the point
// of death, several contributed by named HORCOM users, and the user's
// own definitions load from the two ARABTEI files beside the data.
namespace horcom {

/// The formula question of the original, traditional asks the chart
/// whether the sun stood above the horizon.
enum class ArabicFormula {
  kTraditional = 1,
  kAlwaysDay = 2,
  kAlwaysNight = 3,
};

/// One computed part.
struct ArabicPart {
  std::string name;
  /// the formula as text, base plus first minus second
  std::string formula;
  /// the contributor note of his table
  std::string remark;
  /// ecliptic longitude in radians
  double la = 0.0;
  /// one of the user's own points
  bool own = false;
  /// false for an own point whose terms name no body, cusp, ruler or
  /// degree, his old dispatch could store such records
  bool valid = true;
};

/// One term of an own point, his c4 kind and c3 value.
struct OwnArabicTerm {
  /// 0 a body slot, 12 a cusp, 13 the ruler of a house, 14 a whole
  /// ecliptic degree
  int kind = 0;
  int value = 0;
};

/// One own point of ARABTEI1.INT with its three terms of ARABTEI2.INT.
struct OwnArabicPoint {
  std::string name;
  std::string remark;
  /// base, first and second term
  std::array<OwnArabicTerm, 3> terms{};
};

/// The most own points his arrays held.
inline constexpr int kMaxOwnArabic = 36;

/// Reads the own points, both files must exist like his f1! && f2!.
///
/// @param dir the folder of the two files
/// @return the points in index order
[[nodiscard]] std::vector<OwnArabicPoint> read_own_arabic(const std::filesystem::path& dir);

/// Appends one name record to ARABTEI1.INT, 39 bytes, the index in four
/// places, the name in 21 and the remark in 14.
///
/// @param dir    the folder of the file
/// @param index  the point's number, counted from zero
/// @param name   the name, cut to 21 characters
/// @param remark the remark, cut to 14 characters
/// @return false when the file cannot be written
bool append_own_arabic_name(const std::filesystem::path& dir, int index, const std::string& name, const std::string& remark);

/// Appends one term record to ARABTEI2.INT, 16 bytes of four fields.
///
/// @param dir   the folder of the file
/// @param index the point's number, counted from zero
/// @param term  1 for the base, 2 and 3 for the moving terms
/// @param t     the term, its kind and value
/// @return false when the file cannot be written
bool append_own_arabic_term(const std::filesystem::path& dir, int index, int term, const OwnArabicTerm& t);

/// Deletes both files.
///
/// @param dir the folder of the two files
void delete_own_arabic(const std::filesystem::path& dir);

/// Whether a term names something his table can resolve.
///
/// @param t the term
/// @return false for a kind or value outside his table
[[nodiscard]] bool own_term_valid(const OwnArabicTerm& t);

/// Computes the built in parts and the user's own over a chart, the
/// original arabt table.
///
/// @param chart the computed chart, houses included
/// @param af    the formula mode
/// @param own_dir the folder holding ARABTEI1.INT and ARABTEI2.INT,
///               empty skips the own points
/// @param classic_rulers his alt!, the house rulers Hv read the old
///               rulers of Scorpio, Aquarius and Pisces
/// @return the 37 parts in his screen order, the own points replacing
///         the first rows like his arabte
[[nodiscard]] std::vector<ArabicPart> arabic_parts(const Chart& chart, ArabicFormula af, const std::filesystem::path& own_dir = {},
                                                   bool classic_rulers = false);

}  // namespace horcom
