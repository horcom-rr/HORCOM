// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <utility>
#include <vector>

class QLineEdit;

// The EINGABE-MODUS of VORGABEN EIN-AUSGABE ÄNDERN. With AUTOMATISCH
// WEITERSCHALTEN a field typed to its full length hands the focus to the
// next field and selects its text, like his gettext did. A field entered
// with the mouse keeps the focus, his mousestop!.
namespace horcom {

/// Switches the automatic advance for every mask opened from now on.
///
/// @param on true for AUTOMATISCH WEITERSCHALTEN, his tabstop& = 0
void set_auto_advance(bool on);

/// @return true while AUTOMATISCH WEITERSCHALTEN is chosen
[[nodiscard]] bool auto_advance();

/// Chains the fields of a mask in entry order when the automatic advance
/// is on, otherwise leaves them alone.
///
/// @param fields each field with the length that completes it, his
///               eingl| of the field, the last field only receives focus
void chain_fields(const std::vector<std::pair<QLineEdit*, int>>& fields);

}  // namespace horcom
