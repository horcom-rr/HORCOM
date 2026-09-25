// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <string_view>

// The body slot numbering of the original element arrays, kept identical
// so every formula and file format reads literally. Slots 19 and up hold
// the extra bodies in the program's standard layout, reachable through the
// nk table of the settings.
namespace horcom::body {

inline constexpr int kFixpunkt = 0;
inline constexpr int kSun = 1;
inline constexpr int kMoon = 2;   // the Earth in heliocentric mode
inline constexpr int kMercury = 3;
inline constexpr int kVenus = 4;
inline constexpr int kMars = 5;
inline constexpr int kJupiter = 6;
inline constexpr int kSaturn = 7;
inline constexpr int kUranus = 8;
inline constexpr int kNeptune = 9;
inline constexpr int kPluto = 10;
inline constexpr int kNodeAsc = 11;   // DR, Drachenkopf
inline constexpr int kNodeDesc = 12;  // DS, Drachenschwanz
inline constexpr int kAscendant = 13;
inline constexpr int kMc = 14;
// the cardinal points 0 Aries, 0 Cancer, 0 Libra and 0 Capricorn of his
// a921 on the slots between the axes and the extra bodies
inline constexpr int kAriesPoint = 15;
inline constexpr int kCapricornPoint = 18;
// the extra body slots of the standard layout nk(i) = 18 + i
inline constexpr int kApogee = 19;
inline constexpr int kChiron = 20;
inline constexpr int kTranspluto = 21;
inline constexpr int kFortune = 22;
inline constexpr int kCeres = 23;
inline constexpr int kPallas = 24;
inline constexpr int kJuno = 25;
inline constexpr int kVesta = 26;
// the eight Hamburger Faktoren, the Uranian school after Witte and
// Sieggrün, slots follow the ps$ table of the original
inline constexpr int kCupido = 27;
inline constexpr int kHades = 28;
inline constexpr int kZeus = 29;
inline constexpr int kKronos = 30;
inline constexpr int kApollon = 31;
inline constexpr int kAdmetos = 32;
inline constexpr int kVulkanus = 33;
inline constexpr int kPoseidon = 34;
inline constexpr int kQuaoar = 35;
inline constexpr int kHalley = 36;
inline constexpr int kPholus = 37;
inline constexpr int kDamokles = 38;
inline constexpr int kNessus = 39;
inline constexpr int kXena = 40;
inline constexpr int kSlotCount = 41;

/// The two letter tags of the original ps$ table, index 0 through 40.
/// They key his sprite drawings, all text output uses kName instead.
inline constexpr std::string_view kTag[kSlotCount] = {
    "te", "so", "mo", "me", "ve", "ma", "ju", "sa", "ur", "ne", "pl",
    "dr", "ds", "ac", "mc", "ar", "cn", "li", "cp", "ag", "ch", "tp",
    "gl", "ce", "pa", "jn", "vs", "cu", "ha", "ze", "kr", "ap", "ad",
    "vu", "po", "qu", "hl", "ph", "da", "ns", "xe"};

/// The uppercase display names of the original pl$ table, ported from
/// HORCOM plnm. Slot 0 is the user defined Fixpunkt, his FP.
inline constexpr std::string_view kName[kSlotCount] = {
    "FP", "SO", "MO", "ME", "VE", "MA", "JU", "SA", "UR", "NE", "PL",
    "DR", "DS", "AC", "MC", "AR", "CN", "LI", "CP", "AG", "CH", "TP",
    "GL", "CE", "PA", "JN", "VS", "CU", "HA", "ZE", "KR", "AP", "AD",
    "VU", "PO", "QU", "HL", "PH", "DA", "NS", "XE"};

//RR pl$(2) = "TE", the moon slot carries the earth in the hrg mode
inline constexpr std::string_view kEarthName = "TE";

/// @param slot a body slot
/// @return true for the four cardinal points of a921
[[nodiscard]] constexpr bool cardinal(int slot) {
  return slot >= kAriesPoint && slot <= kCapricornPoint;
}

/// @param slot a body slot 19 and up in the standard layout
/// @return the ephemeris file stem for eph based bodies, empty otherwise
[[nodiscard]] constexpr std::string_view eph_name(int slot) {
  switch (slot) {
    case 20: return "chiron";
    case 23: return "ceres";
    case 24: return "pallas";
    case 25: return "juno";
    case 26: return "vesta";
    case 35: return "quaoar";
    case 36: return "halley";
    case 37: return "pholus";
    case 38: return "damokles";
    case 39: return "nessus";
    case 40: return "xena";
    default: return {};
  }
}

}  // namespace horcom::body
