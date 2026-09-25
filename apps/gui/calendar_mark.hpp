// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QString>

#include "horcom/time/calendar.hpp"

namespace horcom {

/// His jul$(od,ze), the calendar mark of a record.
///
/// @param cal the calendar the record was entered in
/// @return "(JULIAN.)" or "(GREGOR.)" for a forced calendar, empty for
///         the automatic switch
[[nodiscard]] inline QString jul_mark(Calendar cal) {
  if (cal == Calendar::kJulian) {
    return QStringLiteral("(JULIAN.)");
  }
  return cal == Calendar::kGregorian ? QStringLiteral("(GREGOR.)") : QString();
}

}  // namespace horcom
