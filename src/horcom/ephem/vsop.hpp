// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <filesystem>
#include <vector>

// The truncated VSOP87 series engine for Mercury through Neptune with the
// Earth standing in for the Sun. Ported from the original HORCOM procedures
// initpterm, readpterm and plposhi. Robert Rettig documents the provenance
// himself, an abridged VSOP87 of the Bureau des Longitudes made accessible
// by Jean Meeus, whose book he cites as his source.
//
// The binary tables are planets.ndx and planets.dat. The ndx holds pairs of
// 16 bit little endian integers, data record offset and term count, in
// blocks of 18 records per planet, three coordinates L, B, R times six
// powers of time. The dat holds triples of 64 bit doubles A, B, C of
// A * cos(B + C * t). Record numbers count from zero, block zero is an
// unused duplicate of the Earth, blocks one to eight are Mercury, Venus,
// Earth, Mars, Jupiter, Saturn, Uranus, Neptune.
namespace horcom {

class VsopTables {
 public:
  /// Loads the term tables for planets 1..8 like the original startup
  /// loop over readpterm.
  ///
  /// @param ndx_path the planets.ndx index file
  /// @param dat_path the planets.dat term file
  /// @return the loaded tables
  /// @note throws std::runtime_error when a file cannot be read
  static VsopTables load(const std::filesystem::path& ndx_path, const std::filesystem::path& dat_path);

  struct Result {
    double l = 0.0;   // heliocentric longitude, radians, normalised
    double b = 0.0;   // heliocentric latitude, radians
    double r = 0.0;   // radius, AU
    double lt = 0.0;  // rates per day, the original helt, hebt, hert
    double bt = 0.0;
    double rt = 0.0;
  };

  /// The original plposhi for one planet.
  ///
  /// @param planet     1 Mercury through 8 Neptune, 3 is the Earth
  /// @param t11        Julian centuries from J2000
  /// @param with_rates mirrors the lin! switch that skips the derivative
  /// @return heliocentric longitude, latitude, radius and their rates
  [[nodiscard]] Result evaluate(int planet, double t11, bool with_rates = true) const;

  /// @return number of loaded series terms
  [[nodiscard]] int term_count() const noexcept { return static_cast<int>(a_.size()); }

 private:
  struct Series {
    int offset = 0;  // 0-based index into the term arrays
    int count = 0;
  };
  std::array<std::array<std::array<Series, 6>, 3>, 9> index_{};  // [planet][coord][power]
  std::vector<double> a_;
  std::vector<double> b_;
  std::vector<double> c_;
};

}  // namespace horcom
