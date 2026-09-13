// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

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
};

/// Computes the built in parts and the user's own over a chart, the
/// original arabt table.
///
/// @param chart the computed chart, houses included
/// @param af    the formula mode
/// @param own_dir the folder holding ARABTEI1.INT and ARABTEI2.INT,
///               empty skips the own points
/// @return the parts in his screen order
[[nodiscard]] std::vector<ArabicPart> arabic_parts(const Chart& chart, ArabicFormula af, const std::filesystem::path& own_dir = {});

}  // namespace horcom
