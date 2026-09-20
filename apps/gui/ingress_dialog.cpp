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

// the bodies of his ingress menu in slot order plus the tester's extras
struct BodyEntry {
  const char* name;
  int slot;
  int min_year;
};

// CH orbit varies over centuries (Chiron crosses Saturn), so its earliest
// reliable ingress lands around 1800. NE-PL-QU-XE are steady enough back
// to 1500 which is what the tester asked for. AC and MC vary with the
// nutation and the earth's obliquity, both good back to 1500 too.
constexpr BodyEntry kBodies[] = {
    {"Sonne",   1,  1},
    {"Mond",    2,  1},
    {"Merkur",  3,  1},
    {"Venus",   4,  1},
    {"Mars",    5,  1},
    {"Jupiter", 6,  1},
    {"Saturn",  7,  1},
    {"Uranus",  8,  1500},
    {"Neptun",  9,  1500},
    {"Pluto",  10,  1500},
    {"CH",     20,  1800},
    {"QU",     35,  1500},
    {"XE",     40,  1500},
    {"AC",     13,  1},
    {"MC",     14,  1},
};

constexpr const char* kSignName[12] = {"AR", "TA", "GM", "CN", "LE", "VI",
                                       "LI", "SC", "SG", "CP", "AQ", "PS"};

}  // namespace

IngressDialog::IngressDialog(SearchContext ctx, QWidget* parent) : QDialog(parent), ctx_(std::move(ctx)) {
  setWindowTitle(tr("Ingresse"));
  auto* v = new QVBoxLayout(this);
  auto* top = new QHBoxLayout();
  body_ = new QComboBox(this);
  for (const BodyEntry& e : kBodies) {
    body_->addItem(e.name, e.slot);
  }
  when_ = new QDateEdit(QDate::currentDate(), this);
  when_->setCalendarPopup(true);
  when_->setDisplayFormat("dd.MM.yyyy");
  when_->setMinimumDate(QDate(1500, 1, 1));
  auto* run = new QPushButton(tr("Rechnen"), this);
  top->addWidget(body_);
  top->addWidget(new QLabel(tr("um"), this));
  top->addWidget(when_);
  top->addWidget(run, 1);
  //RR the min date follows the body's reliable range, CH starts around 1800
  const auto sync_min_date = [this]() {
    const int slot = body_->currentData().toInt();
    int min_year = 1;
    for (const BodyEntry& e : kBodies) {
      if (e.slot == slot) {
        min_year = e.min_year;
        break;
      }
    }
    when_->setMinimumDate(QDate(min_year, 1, 1));
  };
  connect(body_, qOverload<int>(&QComboBox::currentIndexChanged), this, sync_min_date);
  sync_min_date();
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
  const int slot = body_->currentData().toInt();
  table_data_ = slot >= 13 ? angle_ingresses(jd, slot, ctx_) : sign_ingresses(jd, slot, ctx_);
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
