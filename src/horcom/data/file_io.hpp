// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

// Whole file reads and whole file replacements for the collection
// formats. The original rewrote its files through a scratch file like
// QAYWSXED.DAT and swapped it in with NAME at the end, the port keeps
// that shape so a crash never leaves a half written collection behind.
namespace horcom {

/// Reads a whole file.
///
/// @param path the file
/// @return its bytes, or std::nullopt when it cannot be opened
[[nodiscard]] std::optional<std::string> read_file_bytes(const std::filesystem::path& path);

/// Replaces a file in one step. The bytes go into a scratch file beside
/// the destination, which then takes the destination's name.
///
/// @param path  the destination, created when missing
/// @param bytes the new content
/// @return true when the destination holds the new bytes, false leaves
///         the old file untouched
bool replace_file(const std::filesystem::path& path, std::string_view bytes);

/// Finds an entry of a folder by name regardless of case. Windows file
/// names are case blind while Linux keeps the case, the original wrote
/// its twins in capitals and a copied archive may use either.
///
/// @param dir  the folder
/// @param name the wanted entry name
/// @return the existing entry, or dir / name when none matches
[[nodiscard]] std::filesystem::path find_case_blind(const std::filesystem::path& dir,
                                                    const std::filesystem::path& name);

}  // namespace horcom
