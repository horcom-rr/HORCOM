// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <array>

#include "horcom/time/calendar.hpp"

// The chart settings of the original, one field per global switch. The
// defaults mirror the program's startup values.
namespace horcom {

/// House system selector, the original haw& with his menu order.
enum class HouseSystem {
  kPlacidus = 1,
  kTopocentric = 2,
  kKochGoh = 3,
  kRegiomontanus = 4,
  kCampanus = 5,
  kEqualAsc = 6,      //RR ÄQUAL EKLIPTIKAL ab AC
  kEqualVehlow = 7,
  kAcMcOnly = 8,      //RR KEINE Häuser, NUR AC und MC
  kNone = 9,
};

/// Apparent position mode, the original appa&.
enum class ApparentMode {
  kLightTime = 1,             // App.1
  kLightTimeAberration = 2,   // App.2
  kTrue = 3,                  // no correction
};

/// One chart's calculation switches.
struct ChartSettings {
  HouseSystem houses = HouseSystem::kPlacidus;
  ApparentMode apparent = ApparentMode::kLightTime;
  /// the original par = 1, Robert Rettig's topocentric parallax
  bool topocentric_parallax = false;
  /// the original apogw!, true instead of mean Black Moon
  bool true_apogee = false;
  /// the original moknw!, true instead of mean lunar node
  bool true_node = false;
  /// the original stzw& = 1, apparent instead of mean sidereal time
  bool apparent_sidereal = true;
  /// the original hrg!, heliocentric mode
  bool heliocentric = false;
  /// the original klpl!, extra bodies enabled
  bool extra_bodies = false;
  /// the original nk&(1..22) slot table, index 0 unused, 0 means off.
  /// Index meaning, 1 apogee AG, 2 Chiron, 3 Transpluto, 4 Part of
  /// Fortune, 5..8 Ceres Pallas Juno Vesta, 9..16 the Hamburg factors,
  /// 17 Quaoar, 18 Halley, 19 Pholus, 20 Damokles, 21 Nessus, 22 Xena
  std::array<int, 23> nk{};
  Calendar calendar = Calendar::kAuto;

  /// Fills the slot table with the program's standard layout nk(i) = 18 + i
  /// and switches the extra bodies on.
  void enable_standard_extras() {
    extra_bodies = true;
    for (int i = 1; i <= 22; ++i) {
      nk[static_cast<std::size_t>(i)] = 18 + i;
    }
  }

  /// @return the highest active body slot, the original np&
  [[nodiscard]] int body_count() const {
    if (!extra_bodies) {
      return 12;
    }
    int np = 18;
    for (int i = 1; i <= 22; ++i) {
      if (nk[static_cast<std::size_t>(i)] > 0) {
        ++np;
      }
    }
    return np;
  }
};

}  // namespace horcom
