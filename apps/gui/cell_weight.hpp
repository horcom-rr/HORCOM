// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QFont>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <initializer_list>

// The bold cells of the lists. The rows of a table stand in the plain
// weight so the bold column heads read as heads, the values a reader
// looks for, names, longitudes, aspects and dates, stand bold again as
// the tester asked in his eighth batch.
namespace horcom {

/// The bold weight alone. A cell font resolves against the font of its
/// view, so the family and the size keep following the style sheet, the
/// text scale and the table zoom.
///
/// @return a font that sets nothing but the weight
[[nodiscard]] inline QFont bold_weight() {
  QFont f;
  f.setBold(true);
  return f;
}

/// Sets one cell bold.
///
/// @param item the cell, a null cell is left alone
inline void bold_cell(QTableWidgetItem* item) {
  if (item != nullptr) {
    item->setFont(bold_weight());
  }
}

/// Sets the cells of some columns of a filled table bold.
///
/// @param table   the table, its rows filled
/// @param columns the columns whose cells turn bold
inline void bold_columns(QTableWidget* table, std::initializer_list<int> columns) {
  for (int r = 0; r < table->rowCount(); ++r) {
    for (const int c : columns) {
      bold_cell(table->item(r, c));
    }
  }
}

/// Sets every cell of a filled table bold.
///
/// @param table the table, its rows filled
inline void bold_all(QTableWidget* table) {
  for (int r = 0; r < table->rowCount(); ++r) {
    for (int c = 0; c < table->columnCount(); ++c) {
      bold_cell(table->item(r, c));
    }
  }
}

/// Sets every row of a filled list bold.
///
/// @param list the list, its rows filled
inline void bold_all(QListWidget* list) {
  for (int r = 0; r < list->count(); ++r) {
    list->item(r)->setFont(bold_weight());
  }
}

}  // namespace horcom
