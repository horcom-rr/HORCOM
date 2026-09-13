// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string>
#include <string_view>

// Character encoding of the legacy files. Every text field of the
// original formats is Windows 1252, verified byte level in the legacy
// analysis. The port normalises to UTF-8 on read and converts back on
// write, unmappable characters become a question mark.
namespace horcom {

/// Decodes a Windows 1252 byte string to UTF-8.
///
/// @param in raw bytes from a legacy file
/// @return the UTF-8 text
[[nodiscard]] std::string cp1252_to_utf8(std::string_view in);

/// Encodes UTF-8 text as Windows 1252 bytes.
///
/// @param in UTF-8 text
/// @return the byte string, unmappable code points as question marks
[[nodiscard]] std::string utf8_to_cp1252(std::string_view in);

}  // namespace horcom
