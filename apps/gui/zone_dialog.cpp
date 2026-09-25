// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "zone_dialog.hpp"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QTableWidget>
#include <QVBoxLayout>

#include "table_fit.hpp"

namespace horcom {

ZoneDialog::ZoneDialog(const std::filesystem::path& catalogue, QWidget* parent) : QDialog(parent) {
  if (const auto z = load_zone_names(catalogue)) {
    entries_ = *z;
  }
  // his dialog title credited the source of the table, that stays
  setWindowTitle(tr("Zeit-Zonen (P.D. via B. Mahl)"));
  auto* v = new QVBoxLayout(this);
  filter_ = new QLineEdit(this);
  filter_->setPlaceholderText(tr("Suchen…"));
  filter_->setClearButtonEnabled(true);
  table_ = new QTableWidget(0, 3, this);
  table_->setHorizontalHeaderLabels({tr("Name der Zeit-Zone"), tr("Abk."), tr("Zone (h östl.)")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->verticalHeader()->setDefaultSectionSize(20);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  v->addWidget(filter_);
  v->addWidget(table_, 1);
  v->addWidget(buttons);

  connect(filter_, &QLineEdit::textChanged, this, [this](const QString&) { refresh(); });
  connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { accept_row(row); });
  connect(buttons, &QDialogButtonBox::accepted, this, [this]() { accept_row(table_->currentRow()); });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  resize(620, 520);

  refresh();
  filter_->setFocus();
}

void ZoneDialog::refresh() {
  const QString needle = filter_->text().trimmed();
  table_->setRowCount(0);
  for (std::size_t i = 0; i < entries_.size(); ++i) {
    const ZoneEntry& z = entries_[i];
    const QString name = QString::fromStdString(z.name);
    const QString abbrev = QString::fromStdString(z.abbrev);
    if (!needle.isEmpty() && !name.contains(needle, Qt::CaseInsensitive) &&
        !abbrev.contains(needle, Qt::CaseInsensitive)) {
      continue;
    }
    const int row = table_->rowCount();
    table_->insertRow(row);
    auto* item = new QTableWidgetItem(name);
    item->setData(Qt::UserRole, static_cast<qulonglong>(i));
    table_->setItem(row, 0, item);
    table_->setItem(row, 1, new QTableWidgetItem(abbrev));
    // the catalogue stores the step from zone time to UT, the column
    // shows hours east like the input panel
    QString east;
    if (z.to_ut_hours) {
      east = QString::asprintf("%+.4g", -*z.to_ut_hours);
    } else if (z.is_local_time()) {
      east = tr("aus Länge");
    }
    table_->setItem(row, 2, new QTableWidgetItem(east));
  }
  if (table_->rowCount() > 0) {
    table_->selectRow(0);
  }
  table_->resizeColumnToContents(0);
  table_->resizeColumnToContents(1);
  fit_columns(table_);
}

void ZoneDialog::accept_row(int row) {
  if (row < 0 || row >= table_->rowCount()) {
    return;
  }
  const auto idx = table_->item(row, 0)->data(Qt::UserRole).toULongLong();
  chosen_ = entries_[static_cast<std::size_t>(idx)];
  accept();
}

}  // namespace horcom
