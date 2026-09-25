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

/// Decodes the hand typed texts of his archive. They mix the German
/// letters of the Atari ST character set, typed on his first machine,
/// with Windows 1252 letters added later. The Atari letters sit in 0x80
/// to 0x9F where Windows 1252 has only punctuation, so both decode side
/// by side.
///
/// @param in raw bytes from a legacy text
/// @return the UTF-8 text
[[nodiscard]] std::string atari_cp1252_to_utf8(std::string_view in);

/// Tells UTF-8 from the legacy encodings. Windows 1252 and Atari bytes
/// never form valid multi byte UTF-8 in German text.
///
/// @param text the bytes of a file or line
/// @return true when every byte sequence is valid UTF-8
[[nodiscard]] bool looks_like_utf8(std::string_view text);

/// Encodes UTF-8 text as Windows 1252 bytes.
///
/// @param in UTF-8 text
/// @return the byte string, unmappable code points as question marks
[[nodiscard]] std::string utf8_to_cp1252(std::string_view in);

}  // namespace horcom
