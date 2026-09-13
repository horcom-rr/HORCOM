// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string>

#include "horcom/render/wheel.hpp"

namespace horcom {

/// Renders a display list as a standalone SVG document.
///
/// @param dl the wheel primitives on the virtual canvas
/// @return the SVG text, viewBox 0 0 640 480 like the original screen
[[nodiscard]] std::string to_svg(const DisplayList& dl);

}  // namespace horcom
