// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/settings.hpp"

// The settings file of the original, INTERN KONSTA7P.INT, written by
// kon_dsp and read by kon_dhol as one GFA token stream. Field names keep
// the original variable names so the schema stays literally comparable.
namespace horcom {

/// Every field of the settings file in stream order.
struct Konsta {
  int haw = 1;
  std::string haus = "Placidus";
  int appa = 1;
  std::string appa_name = "App.1";
  int gen = 2;
  std::string gena = "Maximal";
  bool apogw = false;
  bool moknw = false;
  double orb = 1.0;
  bool klsy = false;
  bool klsyt = false;
  double sext = -1.0;
  double pziff = 1.0;
  bool voll = false;
  int nasp = 12;
  bool bdsp = true;
  bool ryt = false;
  double par = 2.0;
  double fza = 0.0;
  int begz = 1;
  std::string begz_name;
  int hard = 2;
  int lfm = 0;
  int bres = 1;
  int brep = 1;
  bool zwhd = false;
  bool kard = false;
  int horm = 1;
  bool orbe_on = false;   // the original orbe!
  bool stats = false;
  bool slist = false;
  bool plusl = false;
  int col_dial = 0;
  int col_backg = 0;
  bool farbs = true;
  bool linie = true;
  int plinv = 0;
  int tabstop = 0;
  int prenbl = 1;
  int halbs = 1;
  bool zeichen = true;
  bool comp_mstz = false;
  bool comp_hand = true;
  bool gitter = false;
  bool selbst_cl_st = false;
  bool eigfarb = false;
  bool farbp = false;
  bool weiss = false;
  int elem = 1;
  bool gebherr_dop = true;
  bool haus1_dop = false;
  double jdgross = 2370832.0;  //RR CHAUVIN f.AQU.
  int zal_grossj = 330;
  int entf = 1;
  int stzw = 1;  //RR wahre Sternzeit
  int erase_ = 0;
  int fixpunkt = 2;
  std::string fixpunkt_name;
  std::string fixpunkt_rh;
  bool lpktg = false;
  int anzweg = 0;
  int halbs_dir = 0;
  int nursymb = 0;
  bool lin_inv = false;
  std::array<int, body::kSlotCount> or_weight{};        // or&(0..40)
  std::array<std::string, 15> orb_text{}; // orb$(0..14), present when orbe_on
  std::array<int, 23> nk{};               // nk&(1..22), index 0 unused
  std::array<int, 20> aspli_flag{};       // aspli|(1..19)
  std::array<int, 20> aspli_col{};        // aspli%(1..19)
  std::array<int, 20> aspst{};            // aspst|(1..19)
  std::array<int, 5> cols{};              // cols%(1..4)
  std::array<int, 16> pn{};               // pn&(1..15)

  Konsta() { or_weight.fill(100); }

  /// Maps the file onto the pipeline settings like kon_dhol does,
  /// including the nasp 16 to 12 rule of the equal probability mode.
  [[nodiscard]] ChartSettings chart_settings() const;

  /// Maps the orb configuration, weights and the orbe table converted to
  /// radians with the original pu times absolute value.
  [[nodiscard]] AspectSettings aspect_settings() const;
};

/// His final working profile, every value taken from the settings file
/// Robert Rettig ran himself. Parallax on, true node and true apogee,
/// the equal probability orb mode with his own orb table and weights.
///
/// @return the profile as a Konsta, ready for the settings mappers
[[nodiscard]] Konsta robert_profile();

/// Loads a KONSTA stream from file.
///
/// @param path the INTERN KONSTA7P.INT style file
/// @return the settings, or std::nullopt when the file cannot be read,
///         mirroring the original's FORMAT-ÄNDERUNG catch
[[nodiscard]] std::optional<Konsta> load_konsta(const std::filesystem::path& path);

/// Parses a KONSTA stream from memory.
///
/// @param text the whole stream, Windows 1252 bytes
/// @return the settings read in stream order
[[nodiscard]] Konsta parse_konsta(std::string_view text);

/// Writes the stream exactly like kon_dsp, same line grouping, strings
/// quoted, booleans as -1 and 0, the orb table only in equal probability
/// mode.
///
/// @param k the settings
/// @return the stream text
[[nodiscard]] std::string format_konsta(const Konsta& k);

/// Saves the stream to file, the old file is replaced in one step.
///
/// @param path the INTERN KONSTA7P.INT style file
/// @param k    the settings
/// @return true on success
bool save_konsta(const std::filesystem::path& path, const Konsta& k);

}  // namespace horcom
