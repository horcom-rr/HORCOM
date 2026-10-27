// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <cstdint>

// His BEEP. The walks, the stepping coordinate sheets and the arc passes
// signal every step, the Windows alert sound behind QApplication::beep
// was loud and restarted itself under a key held down.
namespace horcom::tone {

/// The settings key of SIGNALTON EIN / AUS in ANSICHT.
inline constexpr const char* kEnabledKey = "view/signalTone";

/// Rings his BEEP as a short soft ping.
///
/// @note Silent while SIGNALTON is off and while the ping before still
///       sounds, so a key held down rings an even tick instead of cutting
///       each ping short. The headless test platform counts the ping
///       without a sound.
void ring();

/// Tells whether SIGNALTON is on.
///
/// @return true while SIGNALTON is on, the default
[[nodiscard]] bool enabled();

/// Switches SIGNALTON and keeps the choice in the settings.
///
/// @param on true rings the signal again
void set_enabled(bool on);

/// Counts the pings, the tests read the throttle by it.
///
/// @return how many pings rang since the program started
[[nodiscard]] std::uint64_t rung();

}  // namespace horcom::tone
