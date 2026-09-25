// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

// Reader for Robert Rettig's own ephemeris files. He produced them with a
// Runge Kutta started Stoermer integration to the twelfth difference,
// following the method of Astronomical Papers Vol. XII, and shipped them
// with the program. Ported from the original HORCOM procedure
// ephem_auswert.
//
// A file is a sequence of 16 byte records, four little endian signed 32
// bit integers, the integer Julian day and the heliocentric rectangular
// coordinates scaled by a per body factor. Records run from the newest
// date DOWNWARD, the true epoch of a record is its integer plus 0.5, and
// record zero is an unwritten dummy because the original writer counts
// records from one while the runtime addresses them from zero.
namespace horcom {

/// Reference frame of the stored vectors.
enum class EphFrame {
  kEquatorialJ2000,  // pluto.eph
  kEclipticJ2000,    // asteroids, centaurs, quaoar, xena
  kEclipticB1950,    // chiron.eph and halley.eph
};

/// Signed rates of a heliocentric state, per day.
struct SphericalRates {
  double lont = 0.0;  // longitude, the original helt
  double latt = 0.0;  // latitude, hebt
  double rt = 0.0;    // radius, hert
};

/// The derivatives of longitude, latitude and radius of a cartesian
/// state, ported from the helt, hert and hebt lines of ephem_auswert.
///
/// @param x position vector in AU
/// @param v velocity vector in AU per day, same frame
/// @return the rates with their sign
/// @note the original wrapped all three in ABS, which lost the sign of a
///       falling radius, a southward latitude and Halley's retrograde
///       longitude, the geocentric speed and the R marker went wrong
[[nodiscard]] SphericalRates spherical_rates(const std::array<double, 3>& x, const std::array<double, 3>& v);

/// Static description of one ephemeris body.
struct EphBodyInfo {
  std::string_view filename;
  double fplanet;   // scale, 2.1748E9 divided by the body's maximum radius
  EphFrame frame;
};

/// The body table of the original's SELECT with his scale factors.
///
/// @param name file stem like pluto, chiron, ceres, xena
/// @return the description, or nullptr for an unknown name
[[nodiscard]] const EphBodyInfo* eph_body(std::string_view name);

/// One of his ephemeris files, read whole and interpolated on demand.
class EphFile {
 public:
  /// Opens and slurps an ephemeris file.
  ///
  /// @param path the .eph file
  /// @return the reader, or std::nullopt when the file is missing or short
  [[nodiscard]] static std::optional<EphFile> open(const std::filesystem::path& path);

  /// Result of an evaluation at one epoch.
  struct Sample {
    bool in_range = false;
    double lon = 0.0;  // longitude in the file frame precessed to date,
                       // right ascension for the equatorial frame
    double lat = 0.0;  // latitude, declination for the equatorial frame
    double r = 0.0;    // heliocentric distance, AU
    double lont = 0.0; // signed rates per day, see spherical_rates
    double latt = 0.0;
    double rt = 0.0;
    std::array<double, 3> xyz{};  // interpolated cartesian vector of date
    std::array<double, 3> vxyz{}; // interpolated velocity, AU per day
  };

  /// Interpolates the body position for an epoch.
  ///
  /// Five samples around the epoch are individually precessed to date,
  /// combined with the original's Newton Stirling formula, velocities come
  /// from first differences at interval midpoints, interpolated around the
  /// midpoint half a step after the centre sample.
  ///
  /// @param jd      epoch in Ephemeris Time
  /// @param fplanet scale factor of the body
  /// @param frame   stored reference frame
  /// @return the sample, in_range false outside the usable span
  [[nodiscard]] Sample evaluate(double jd, double fplanet, EphFrame frame) const;

  /// @return integer Julian day of the newest record
  [[nodiscard]] double jd_newest() const noexcept { return jda_; }
  /// @return step between records in days, positive
  [[nodiscard]] double step() const noexcept { return djd_; }
  /// @return upper usable bound, the original jdplaneta
  [[nodiscard]] double upper_bound() const noexcept { return upper_; }
  /// @return lower usable bound, the original jdplanete
  [[nodiscard]] double lower_bound() const noexcept { return lower_; }
  /// @return number of 16 byte records including the dummy record zero
  [[nodiscard]] std::size_t record_count() const noexcept { return raw_.size() / 4; }

 private:
  std::vector<std::int32_t> raw_;
  double jda_ = 0.0;
  double djd_ = 0.0;
  double upper_ = 0.0;
  double lower_ = 0.0;
};

}  // namespace horcom
