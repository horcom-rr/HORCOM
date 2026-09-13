// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QRectF>

#include "horcom/render/wheel.hpp"

class QPainter;

namespace horcom {

/// Paints a display list onto an already transformed painter, the one
/// code path behind the screen widget, the printer and the PDF export
/// that the original resolved through its moda branches.
///
/// @param p  painter whose coordinate system is the virtual canvas
/// @param dl the list to draw
void paint_display_list(QPainter& p, const DisplayList& dl);

/// Fits a display list into a target rectangle, aspect preserved and
/// centred, used by the print and PDF paths over a white page.
///
/// @param p      painter in device coordinates
/// @param dl     the list to draw
/// @param target the page area to fill
void paint_fitted(QPainter& p, const DisplayList& dl, const QRectF& target);

}  // namespace horcom
