// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "horcom/ephem/eph_file.hpp"

namespace horcom {

/// Opens and caches the .eph files of a data directory.
class Ephemerides {
 public:
  /// @param dir directory holding pluto.eph and friends
  explicit Ephemerides(std::filesystem::path dir) : dir_(std::move(dir)) {}

  /// Fetches one body's ephemeris, opening it on first use.
  ///
  /// @param name file stem like pluto or chiron
  /// @return the reader, or nullptr when the file is missing
  [[nodiscard]] const EphFile* get(std::string_view name) const {
    auto it = cache_.find(name);
    if (it == cache_.end()) {
      it = cache_.emplace(std::string(name), EphFile::open(dir_ / (std::string(name) + ".eph"))).first;
    }
    return it->second.has_value() ? &*it->second : nullptr;
  }

 private:
  std::filesystem::path dir_;
  mutable std::map<std::string, std::optional<EphFile>, std::less<>> cache_;
};

}  // namespace horcom
