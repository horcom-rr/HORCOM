// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <functional>
#include <string>

#include "horcom/render/wheel.hpp"

namespace horcom {

/// Resolves a glyph to an embedded raster image, his symbbmp sprites.
/// Takes the glyph text and the requested colour, returns a data URI
/// ready for an SVG image element, or empty to keep the font glyph.
/// The renderer itself stays free of the sprite assets, the GUI hands
/// its tinted sprite cache in through this hook.
using GlyphImageResolver = std::function<std::string(const std::string&, Rgb)>;

/// Renders a display list as a standalone SVG document.
///
/// @param dl      the wheel primitives on the virtual canvas
/// @param sprites optional resolver embedding his symbol sprites, the
///                default keeps the font glyphs
/// @return the SVG text, viewBox 0 0 640 480 like the original screen
[[nodiscard]] std::string to_svg(const DisplayList& dl, const GlyphImageResolver& sprites = {});

}  // namespace horcom
