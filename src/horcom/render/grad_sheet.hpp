// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string>
#include <vector>

#include "horcom/chart/degree_list.hpp"
#include "horcom/render/wheel.hpp"

// The pages of the GRAD-LISTE, his grlinit frame with five columns of
// 41 rows, and in the sorted mode the Gesamt-Verteilung of listscal
// right of them.
namespace horcom {

/// The entries of one page, five columns of 41 rows.
inline constexpr int kGradPerPage = 205;

/// The frame texts of grlinit.
struct GradSheetText {
  /// "Grad-Liste |" with name, date and chart label
  std::string title;
  /// his horgt$, the frame of the positions
  std::string frame;
  /// his gena2$, the ephemeris mode
  std::string ephem;
  /// the two heading lines of the panel, the caller translates them
  std::string panel_head = "Gesamt -";
  std::string panel_sub = "Verteilung :";
};

/// Draws one page of the list.
///
/// @param entries the whole list in the order shown
/// @param page    the page from zero
/// @param text    the frame texts
/// @param panel   the sorted mode draws the distribution of every entry
/// @param unit    the pixels per entry of the panel, grad_unit
/// @return the sheet on the classic 640 by 480 canvas
[[nodiscard]] DisplayList build_grad_sheet(const std::vector<GradEntry>& entries, int page, const GradSheetText& text,
                                           bool panel, int unit);

}  // namespace horcom
