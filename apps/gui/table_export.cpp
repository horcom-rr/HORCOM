// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "table_export.hpp"

#include <QSaveFile>
#include <QTableWidget>

namespace horcom {

namespace {

// the separator of Excel on a German system, where the comma is the
// decimal mark
constexpr QChar kSeparator = u';';

// a field in quotes where it holds the separator, a quote or a line
// break, the quotes inside doubled
QString csv_field(QString text) {
  text = text.trimmed();
  if (text.contains(kSeparator) || text.contains(u'"') || text.contains(u'\n') || text.contains(u'\r')) {
    text.replace(u'"', QStringLiteral("\"\""));
    return u'"' + text + u'"';
  }
  return text;
}

}  // namespace

QByteArray excel_csv(const QStringList& lines, const QTableWidget& table) {
  QStringList rows;
  for (const QString& line : lines) {
    rows << csv_field(line);
  }
  if (!lines.isEmpty()) {
    rows << QString();
  }
  QStringList heads;
  for (int c = 0; c < table.columnCount(); ++c) {
    const QTableWidgetItem* head = table.horizontalHeaderItem(c);
    heads << csv_field(head != nullptr ? head->text() : QString());
  }
  rows << heads.join(kSeparator);
  for (int r = 0; r < table.rowCount(); ++r) {
    QStringList cells;
    for (int c = 0; c < table.columnCount(); ++c) {
      const QTableWidgetItem* item = table.item(r, c);
      cells << csv_field(item != nullptr ? item->text() : QString());
    }
    rows << cells.join(kSeparator);
  }
  return QByteArray("\xEF\xBB\xBF") + rows.join(QStringLiteral("\r\n")).toUtf8() + "\r\n";
}

bool write_excel_csv(const QString& path, const QStringList& lines, const QTableWidget& table) {
  QSaveFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }
  file.write(excel_csv(lines, table));
  return file.commit();
}

}  // namespace horcom
