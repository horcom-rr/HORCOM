// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "ingress_dialog.hpp"

#include <QApplication>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the bodies of his ingress menu in slot order
constexpr const char* kBodyName[10] = {"Sonne",  "Mond",   "Merkur", "Venus",  "Mars",
                                       "Jupiter", "Saturn", "Uranus", "Neptun", "Pluto"};

constexpr const char* kSignName[12] = {"AR", "TA", "GM", "CN", "LE", "VI",
                                       "LI", "SC", "SG", "CP", "AQ", "PS"};

}  // namespace

IngressDialog::IngressDialog(SearchContext ctx, QWidget* parent) : QDialog(parent), ctx_(std::move(ctx)) {
  setWindowTitle(tr("Ingresse"));
  auto* v = new QVBoxLayout(this);
  auto* top = new QHBoxLayout();
  body_ = new QComboBox(this);
  for (int slot = 1; slot <= 10; ++slot) {
    body_->addItem(kBodyName[slot - 1], slot);
  }
  when_ = new QDateEdit(QDate::currentDate(), this);
  when_->setCalendarPopup(true);
  when_->setDisplayFormat("dd.MM.yyyy");
  auto* run = new QPushButton(tr("Rechnen"), this);
  top->addWidget(body_);
  top->addWidget(new QLabel(tr("um"), this));
  top->addWidget(when_);
  top->addWidget(run, 1);
  table_ = new QTableWidget(0, 3, this);
  table_->setHorizontalHeaderLabels({tr("Zeichen"), tr("Datum"), tr("Zeit (UT)")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->verticalHeader()->setDefaultSectionSize(20);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  v->addLayout(top);
  v->addWidget(table_, 1);
  v->addWidget(buttons);

  connect(run, &QPushButton::clicked, this, [this]() { run_scan(); });
  connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { accept_row(row); });
  connect(buttons, &QDialogButtonBox::accepted, this, [this]() { accept_row(table_->currentRow()); });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  resize(420, 420);
}

void IngressDialog::run_scan() {
  const QDate d = when_->date();
  const double jd = julian_day({d.day(), d.month(), d.year(), 12, 0.0}, ctx_.settings.calendar);
  QApplication::setOverrideCursor(Qt::WaitCursor);
  table_data_ = sign_ingresses(jd, body_->currentData().toInt(), ctx_);
  QApplication::restoreOverrideCursor();
  table_->setRowCount(0);
  for (int t = 0; t < 12; ++t) {
    const LongitudeCrossing& hit = table_data_[static_cast<std::size_t>(t)];
    const int row = table_->rowCount();
    table_->insertRow(row);
    auto* sign = new QTableWidgetItem(QString(kSignName[t]));
    sign->setData(Qt::UserRole, t);
    table_->setItem(row, 0, sign);
    if (!hit.ok) {
      table_->setItem(row, 1, new QTableWidgetItem(QString::fromUtf8("—")));
      continue;
    }
    const CalendarDate c = calendar_date(hit.jd_ut, ctx_.settings.calendar);
    int seconds = static_cast<int>((c.hour * 60.0 + c.minute) * 60.0 + 0.5);
    if (seconds >= kSecondsPerDay) {
      seconds = kSecondsPerDay - 1;
    }
    table_->setItem(row, 1, new QTableWidgetItem(QString::asprintf("%02d.%02d.%04d", c.day, c.month, c.year)));
    table_->setItem(row, 2, new QTableWidgetItem(QString::asprintf("%02d:%02d", seconds / 3600, (seconds / 60) % 60)));
  }
  if (table_->rowCount() > 0) {
    table_->selectRow(0);
  }
}

void IngressDialog::accept_row(int row) {
  if (row < 0 || row >= table_->rowCount()) {
    return;
  }
  const int idx = table_->item(row, 0)->data(Qt::UserRole).toInt();
  const LongitudeCrossing& hit = table_data_[static_cast<std::size_t>(idx)];
  if (!hit.ok) {
    return;
  }
  chosen_jd_ = hit.jd_ut;
  accept();
}

}  // namespace horcom
