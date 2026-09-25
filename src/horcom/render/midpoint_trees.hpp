// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <string>
#include <vector>

#include "horcom/chart/aspects.hpp"
#include "horcom/render/wheel.hpp"

// The HALBSUMMEN-GRAPHIK of aspar2, eleven trees in each of two bands,
// every tree a point with its degree and sign on top, a trunk with nine
// branches and on each branch the pair whose midpoint falls on the
// point, the level lettered D, Q, H or V between them.
namespace horcom {

/// the trees of one page, his i1& runs to 22 before the key wait
inline constexpr int kTreesPerPage = 22;

/// The texts around the trees, assembled by the caller so the drawing
/// stays free of record and locale handling.
struct MidpointTreeText {
  std::string sol;    ///< his sol$, RADIX or the derived chart
  std::string frame;  ///< horgt$, Geozentrisch and the like
  std::string name;   ///< the Name: line
  std::string date;   ///< the date part, "Datum : 3. 2.1984"
  std::string clock;  ///< the second header line with UT, Länge, Breite
  std::string ephem;  ///< gena2$, the ephemeris footer
};

/// The sprites the tree graphic stamps inverted like plein2.
struct TreeGlyphs {
  bool invert_nodes = false;   ///< the node pair, his moknw!
  bool invert_apogee = false;  ///< the Black Moon, his apogw!
};

/// The key of his sort, eight times the longitude reduced to a circle.
///
/// @param lon ecliptic longitude in radians
/// @return FN nb(8 * lon) in radians
[[nodiscard]] double tree_dial(double lon);

/// Orders the trees by tree_dial like his SSORT over ce%(t&), the
/// planet pictures of the 45 degree dial stand side by side then.
///
/// @param trees the trees in slot order
/// @return the same trees in ascending dial order
[[nodiscard]] std::vector<MidpointTree> sort_trees_by_dial(std::vector<MidpointTree> trees);

/// The red rule of the sorted view, a tree whose dial value lies within
/// the larger of the two orb weights at level eight of another point.
///
/// @param trees the trees as shown
/// @param orbs  orb configuration, his org(t&,8), the cusp trees weigh
///              as tree_orb_settings sets them
/// @return one flag per tree
[[nodiscard]] std::vector<bool> tree_dial_close(const std::vector<MidpointTree>& trees, const AspectSettings& orbs);

/// Builds one page of the tree graphic on the 640 by 480 sheet.
///
/// @param trees    the trees in display order
/// @param page     zero based, 22 trees a page
/// @param text     header and footer lines
/// @param sorted   the sorted view prints each dial value under its tree
/// @param close    the red flags of tree_dial_close, used when sorted
/// @param emphasis the Planeten-Auswahl, 1 draws a body red like plan_col
/// @param glyphs   the sprites stamped inverted
/// @return the drawing
[[nodiscard]] DisplayList build_midpoint_trees(const std::vector<MidpointTree>& trees, int page, const MidpointTreeText& text,
                                               bool sorted, const std::vector<bool>& close,
                                               const std::array<int, body::kSlotCount>& emphasis,
                                               const TreeGlyphs& glyphs = {});

}  // namespace horcom
