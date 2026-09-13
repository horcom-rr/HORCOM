// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "statist_dialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

constexpr const char* kObjectName[14] = {"Sonne", "Mond",   "Merkur", "Venus", "Mars", "Jupiter", "Saturn",
                                         "Uranus", "Neptun", "Pluto",  "Knoten", "Südknoten", "AC", "MC"};

constexpr const char* kSignTag[12] = {"AR", "TA", "GM", "CN", "LE", "VI", "LI", "SC", "SG", "CP", "AQ", "PS"};

}  // namespace

StatistDialog::StatistDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle(tr("Statistik"));
  auto* v = new QVBoxLayout(this);
  auto* top = new QHBoxLayout();
  object_ = new QComboBox(this);
  for (int i = 0; i < 14; ++i) {
    object_->addItem(kObjectName[i], i + 1);
  }
  count_ = new QLabel(this);
  top->addWidget(new QLabel(tr("Objekt"), this));
  top->addWidget(object_);
  top->addWidget(count_, 1);
  table_ = new QTableWidget(0, 3, this);
  table_->setHorizontalHeaderLabels({tr("Name"), tr("Datum"), tr("Ort")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->verticalHeader()->setDefaultSectionSize(20);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  distribution_ = new QLabel(this);
  distribution_->setObjectName("aspectsLine");
  distribution_->setWordWrap(true);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  v->addLayout(top);
  v->addWidget(table_, 1);
  v->addWidget(distribution_);
  v->addWidget(buttons);

  connect(object_, &QComboBox::currentIndexChanged, this, [this](int) { refresh_distribution(); });
  connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { accept_row(row); });
  connect(buttons, &QDialogButtonBox::accepted, this, [this]() { accept_row(table_->currentRow()); });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  resize(680, 560);
}

bool StatistDialog::load(const QString& sta) {
  const auto set = load_statistics(std::filesystem::path(sta.toStdWString()));
  if (!set) {
    return false;
  }
  set_ = *set;
  table_->setRowCount(0);
  for (std::size_t i = 0; i < set_.records.size(); ++i) {
    const StatRecord& r = set_.records[i];
    const int row = table_->rowCount();
    table_->insertRow(row);
    auto* item = new QTableWidgetItem(QString::fromStdString(r.name));
    item->setData(Qt::UserRole, static_cast<qulonglong>(i));
    table_->setItem(row, 0, item);
    table_->setItem(row, 1, new QTableWidgetItem(QString::asprintf("%02d.%02d.%d", r.day, r.month, r.year)));
    table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(r.place)));
  }
  count_->setText(tr("%1 Datensätze").arg(set_.records.size()));
  if (table_->rowCount() > 0) {
    table_->selectRow(0);
  }
  table_->resizeColumnToContents(0);
  table_->resizeColumnToContents(1);
  refresh_distribution();
  return true;
}

void StatistDialog::refresh_distribution() {
  const int object = object_->currentData().toInt();
  std::array<int, 12> bins{};
  int have = 0;
  for (const StatRecord& r : set_.records) {
    double el = 0.0;
    if (object == 13) {
      el = r.ac;
    } else if (object == 14) {
      el = r.mc;
    } else {
      el = r.el[static_cast<std::size_t>(object)];
    }
    if (el == 0.0) {
      continue;
    }
    const int sign = static_cast<int>(norm_deg(el * kRadToDeg) / kDegPerSign) % 12;
    ++bins[static_cast<std::size_t>(sign)];
    ++have;
  }
  QString text = tr("<span style='color:#D4A94A'>ZEICHEN</span>&nbsp; ");
  for (int i = 0; i < 12; ++i) {
    if (i > 0) {
      text += "  ";
    }
    text += QString("%1 %2").arg(kSignTag[i]).arg(bins[static_cast<std::size_t>(i)]);
  }
  text += tr("&nbsp; (%1 belegt)").arg(have);
  distribution_->setText(text);
}

void StatistDialog::accept_row(int row) {
  if (row < 0 || row >= table_->rowCount()) {
    return;
  }
  const auto idx = table_->item(row, 0)->data(Qt::UserRole).toULongLong();
  chosen_ = set_.records[static_cast<std::size_t>(idx)];
  accept();
}

}  // namespace horcom
