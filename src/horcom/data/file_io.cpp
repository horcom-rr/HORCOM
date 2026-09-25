// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/file_io.hpp"

#include <cctype>
#include <fstream>
#include <iterator>
#include <system_error>

namespace horcom {

namespace {

// beside the destination, so the final rename never crosses a volume
constexpr const char* kScratchSuffix = ".horcom-new";

// UTF-8 names compared with the ASCII letters folded, a narrow string
// of a wide Windows name could throw on characters outside the code page
bool same_name_case_blind(const std::u8string& a, const std::u8string& b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t i = 0; i < a.size(); ++i) {
    const auto fold = [](char8_t c) {
      return static_cast<char8_t>(std::tolower(static_cast<unsigned char>(c)));
    };
    if (fold(a[i]) != fold(b[i])) {
      return false;
    }
  }
  return true;
}

}  // namespace

std::optional<std::string> read_file_bytes(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return std::nullopt;
  }
  return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// the shape of his scratch file with NAME uu$ AS datru$
bool replace_file(const std::filesystem::path& path, std::string_view bytes) {
  std::filesystem::path scratch = path;
  scratch += kScratchSuffix;
  std::error_code ec;
  {
    std::ofstream f(scratch, std::ios::binary | std::ios::trunc);
    if (!f) {
      return false;
    }
    f.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    f.flush();
    if (!f) {
      f.close();
      std::filesystem::remove(scratch, ec);
      return false;
    }
  }
  // rename replaces an existing destination on POSIX and on Windows
  std::filesystem::rename(scratch, path, ec);
  if (ec) {
    std::error_code ignored;
    std::filesystem::remove(scratch, ignored);
    return false;
  }
  return true;
}

std::filesystem::path find_case_blind(const std::filesystem::path& dir, const std::filesystem::path& name) {
  const std::filesystem::path exact = dir / name;
  std::error_code ec;
  if (std::filesystem::exists(exact, ec)) {
    return exact;
  }
  const std::u8string wanted = name.filename().u8string();
  for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
    if (same_name_case_blind(entry.path().filename().u8string(), wanted)) {
      return entry.path();
    }
  }
  return exact;
}

}  // namespace horcom
