// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <string>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/render/wheel.hpp"

// The ASPEKTARIUM of aspar as a drawn sheet. The triangular matrix of
// 26 by 20 cells carries the exact separation of every matched pair
// above the diagonal and the named aspect below it, the hit count of
// every body on the hatched diagonal, the divisor table and his legend
// on the right, the planet weights under it and the aspect histogram
// inset when the matrix leaves room.
namespace horcom {

/// The texts of the sheet, assembled by the caller, the legend defaults
/// are his German wording.
struct AspektariumText {
  std::string title;  ///< horgt$ + "es Aspektarium  | " + sol$
  std::string name;   ///< na$, split at the first space like nam
  std::string date_label = "Datum:";
  std::string date;   ///< datum3$
  std::array<std::string, 6> heads = {"Tei-", "ler", "Win-", "kel", "Or-", "bis"};
  std::string factor1 = "Orbis-";
  std::string factor2 = "Faktor = ";
  std::array<std::string, 6> legend = {"Diagramm :",      "Unten:auf volle", "Grade gerundet",
                                       "Darüber Teiler", "Oberhalb Diago.", "Istwerte"};
  std::string name_label = "Name:";
  std::string weights1 = "Planeten-";
  std::string weights2 = "Gewichtung:";
  std::string extras = "Zusatz-Planeten:";
};

/// Everything the sheet reads.
struct AspektariumInput {
  /// the chart whose pairs the matrix shows, a fixed point on slot zero
  /// opens a column of its own
  const Chart* chart = nullptr;
  /// its settings, the hrg mode and the extras decide the columns
  ChartSettings settings;
  /// the scan with the divisors of the MAXIMALER Teiler box and his
  /// asp_wahl filter
  const AspectResult* aspects = nullptr;
  /// orb factor, the equal probability table and the counted divisors
  AspectSettings orbs;
  /// the planet weights the bottom row prints, unfiltered
  std::array<int, body::kSlotCount> weights{};
  /// the Planeten-Auswahl, 1 draws red like plan_col
  std::array<int, body::kSlotCount> emphasis{};
  /// the asphist bars of the inset
  std::array<Rgb, 17> hist_colors = profile_dress().hist_colors;
  /// how the inset bars are filled, WEIß leaves them empty
  RingFill hist_fill = RingFill::kShaded;
  /// his ryt!, the Rhythmenlehre mode leaves the histogram inset away
  bool rhythm = false;
  /// the node pair stamped inverted like plein2 under moknw!
  bool invert_nodes = false;
  /// the Black Moon stamped inverted like plein2 under apogw!
  bool invert_apogee = false;
};

/// the columns of his sheet, 19 at most, five extras fit beside MC
inline constexpr int kAspektariumColumns = 19;

/// Builds the sheet on a 640 by 460 canvas.
///
/// @param in   chart, scan, orbs and dress
/// @param text the texts around the matrix
/// @return the drawing
[[nodiscard]] DisplayList build_aspektarium(const AspektariumInput& in, const AspektariumText& text);

}  // namespace horcom
