// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

// The row dressing both a18 result tables share, the transit and
// mundane list and the direction list. Ported from the table parts of
// a181, a181tx and the sort question of dirend.

#include <QColor>
#include <QCoreApplication>
#include <QHeaderView>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <algorithm>
#include <cmath>
#include <string_view>
#include <vector>

#include "choice_dialog.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/calendar.hpp"
#include "theme.hpp"
#include "zodiac_cells.hpp"

namespace horcom::a18 {

/// The red of his marked planets, hard aspects and STATION rows.
inline const QColor kTableRed{0xE8, 0x00, 0x00};
/// The red of a marked planet on the inverted black ground.
inline const QColor kInvertedRed{0xFF, 0x40, 0x40};
/// The green of his harmonic aspects.
inline const QColor kTableGreen{0x00, 0xA0, 0x00};

/// @param slot a body slot
/// @return the two letter tag of the body, SO for the Sun
[[nodiscard]] inline QString slot_tag(int slot) {
  const std::string_view v = body::kName[static_cast<std::size_t>(slot)];
  return QString::fromUtf8(v.data(), static_cast<qsizetype>(v.size()));
}

/// His inverted symbols of the heavy planets, MA SA UR NE PL.
///
/// @param slot a body slot
/// @return true for Mars and Saturn to Pluto
[[nodiscard]] inline bool heavy(int slot) {
  return slot == body::kMars || (slot >= body::kSaturn && slot <= body::kPluto);
}

/// Dresses the cell of a factor, his plinv inversion and the red mark
/// of EINZELNE PLANETEN ROT MARKIEREN.
///
/// @param item     the cell
/// @param slot     the factor, zero for a cusp or a midpoint
/// @param markable true where the red mark may land, the running side
/// @param plinv    his plinv, 1 or 3 invert
/// @param marked   the marked bodies
inline void dress_factor(QTableWidgetItem* item, int slot, bool markable, int plinv, const std::vector<int>& marked) {
  const bool invert = plinv == 1 || plinv == 3;
  const bool red = markable && slot > 0 && std::find(marked.begin(), marked.end(), slot) != marked.end();
  QColor ink = theme::ink_now();
  if (invert && heavy(slot)) {
    item->setBackground(Qt::black);
    ink = red ? kInvertedRed : QColor(Qt::white);
    item->setForeground(ink);
  } else if (red) {
    ink = kTableRed;
    item->setForeground(ink);
  }
  // a181 drew the sprite of the body, it stands before the tag like in
  // the coordinate tables
  set_body_sprite(item, slot, ink);
}

/// The 25.07.03 change of his tables, hard aspects red and harmonic ones
/// green when plinv asks for colour.
///
/// @param item      the angle cell
/// @param angle_deg the folded aspect angle
/// @param plinv     his plinv, 2 or 3 colour
inline void colour_aspect(QTableWidgetItem* item, double angle_deg, int plinv) {
  if (plinv != 2 && plinv != 3) {
    return;
  }
  const int w5 = static_cast<int>(std::lround(angle_deg));
  if (w5 % 90 == 0) {
    item->setForeground(kTableRed);
  } else if (w5 == 60 || w5 == 120) {
    item->setForeground(kTableGreen);
  }
}

/// The table face of VORGABEN DIREKTIONEN, KLEIN or GROß symbols. His
/// a18liv drew rows of 14 or 19 pixels, the cells follow in points.
///
/// @param table the result table
/// @param small his klsyt, the small symbols
inline void style_table(QTableWidget* table, bool small) {
  table->setFont(theme::mono_font(small ? 9 : 11));
  table->verticalHeader()->setDefaultSectionSize(small ? 16 : 21);
}

/// Seconds of the day of a calendar moment, held below midnight so a
/// rounded clock never reads 24.
///
/// @param d the moment
/// @return 0 to 86399
[[nodiscard]] inline int clock_seconds(const CalendarDate& d) {
  const int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
  return std::min(seconds, kSecondsPerDay - 1);
}

/// The sort question of dirend, LISTE SORTIEREN ?
///
/// @param parent         the list the box opens over
/// @param base_angle_deg the w4d of the run, at 360 no aspect sort
/// @return 0 by time, 1 by aspects, 2 keeps the order, -1 after ESC
[[nodiscard]] inline int ask_sort(QWidget* parent, double base_angle_deg) {
  const QString keep = QCoreApplication::translate("horcom::TransitListDialog", "REIHENFOLGE BELASSEN");
  if (base_angle_deg > 0.0 && base_angle_deg < kDegPerCircle) {
    return ChoiceDialog::ask(parent, "HORCOM",
                             {QCoreApplication::translate("horcom::TransitListDialog", "LISTE SORTIEREN ?")},
                             {QCoreApplication::translate("horcom::TransitListDialog", "ZEITLICH SORTIEREN"),
                              QCoreApplication::translate("horcom::TransitListDialog", "Nach ASPEKTEN SORTIEREN"), keep});
  }
  const int b = ChoiceDialog::ask(parent, "HORCOM",
                                  {QCoreApplication::translate("horcom::TransitListDialog", "LISTE ZEITLICH SORTIEREN ?")},
                                  {QCoreApplication::translate("horcom::TransitListDialog", "SORTIEREN"), keep});
  return b == 1 ? 2 : b;
}

/// Orders the rows like the answers of LISTE SORTIEREN. His aspect key
/// was the angle, a conjunction kept its time key and so sorted behind
/// every aspect.
///
/// @param rows the rows, each with angle_deg and order
/// @param mode 0 by time, 1 by aspects, 2 keeps the order of the sweep
/// @param time the member that orders by time
template <class Row>
void sort_rows(std::vector<Row>& rows, int mode, double Row::*time) {
  if (mode == 0) {
    std::stable_sort(rows.begin(), rows.end(), [time](const Row& a, const Row& b) { return a.*time < b.*time; });
  } else if (mode == 1) {
    std::stable_sort(rows.begin(), rows.end(), [time](const Row& a, const Row& b) {
      const bool ca = std::lround(a.angle_deg) == 0;
      const bool cb = std::lround(b.angle_deg) == 0;
      if (ca != cb) {
        return cb;
      }
      if (!ca && std::lround(a.angle_deg) != std::lround(b.angle_deg)) {
        return a.angle_deg < b.angle_deg;
      }
      return a.*time < b.*time;
    });
  } else {
    std::stable_sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) { return a.order < b.order; });
  }
}

}  // namespace horcom::a18
