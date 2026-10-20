// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>

class QTableWidget;

// The result tables as files a spreadsheet opens, a modern addition the
// tester asked for in his tenth batch to filter the transit lists in
// Excel. The original printed its lists only.
namespace horcom {

/// The text of a table as Excel reads a CSV file on a German system, a
/// semicolon between the columns, the byte order mark of UTF-8 first so
/// the umlauts and the degree signs survive, CRLF line ends.
///
/// @param lines the heading lines above the table, an empty row follows
///        them
/// @param table the table, its column heads and its rows in their shown
///        order
/// @return the bytes of the file
[[nodiscard]] QByteArray excel_csv(const QStringList& lines, const QTableWidget& table);

/// Writes excel_csv into a file.
///
/// @param path  the file, replaced whole
/// @param lines the heading lines above the table
/// @param table the table
/// @return true when the file was written
bool write_excel_csv(const QString& path, const QStringList& lines, const QTableWidget& table);

}  // namespace horcom
