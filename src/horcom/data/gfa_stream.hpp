// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The GFA BASIC WRITE and INPUT text form. WRITE emits comma separated
// values with strings in quotes and CRLF line ends, INPUT reads the same
// stream while IGNORING line boundaries, which the KONSTA file exploits,
// its writer and reader split the fields across lines differently. A
// faithful loader therefore tokenises the whole file first.
namespace horcom {

/// Reads a WRITE style stream token by token.
class GfaReader {
 public:
  /// @param text the whole file content
  explicit GfaReader(std::string_view text);

  /// @return the next token as raw text, quotes removed, or nothing
  [[nodiscard]] std::optional<std::string> next();

  /// @return the next token as double, missing tokens read as zero
  [[nodiscard]] double next_number();

  /// @return the next token as int
  [[nodiscard]] int next_int() { return static_cast<int>(next_number()); }

  /// @return the next token as bool, the original stores -1 and 0
  [[nodiscard]] bool next_bool() { return next_number() != 0.0; }

  /// @return the next token as string
  [[nodiscard]] std::string next_string();

  /// @return count of tokens still unread
  [[nodiscard]] std::size_t remaining() const { return tokens_.size() - pos_; }

 private:
  std::vector<std::string> tokens_;
  std::size_t pos_ = 0;
};

/// Builds a WRITE style line from already formatted tokens.
///
/// @param fields tokens, strings must arrive pre quoted
/// @return the joined line without the line end
[[nodiscard]] std::string gfa_write_line(const std::vector<std::string>& fields);

/// Formats a number the way GFA WRITE prints it, integers plain,
/// floats in their shortest round trip form.
[[nodiscard]] std::string gfa_number(double v);

/// Quotes a string for WRITE.
[[nodiscard]] std::string gfa_quoted(std::string_view s);

/// Formats a bool as the original -1 or 0.
[[nodiscard]] inline std::string gfa_bool(bool b) { return b ? "-1" : "0"; }

}  // namespace horcom
