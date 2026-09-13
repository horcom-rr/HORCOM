// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "transit_list_dialog.hpp"

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

QString slot_tag(int slot) {
  return QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                           static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size()));
}

QString moment_text(double jd_ut, QString& time_out) {
  const CalendarDate d = calendar_date(jd_ut);
  int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
  if (seconds >= kSecondsPerDay) {
    seconds = kSecondsPerDay - 1;
  }
  time_out = QString::asprintf("%02d:%02d", seconds / 3600, (seconds / 60) % 60);
  return QString::asprintf("%02d.%02d.%04d", d.day, d.month, d.year);
}

}  // namespace

TransitListDialog::TransitListDialog(Chart radix, SearchContext ctx, QWidget* parent)
    : QDialog(parent), radix_(std::move(radix)), ctx_(std::move(ctx)) {
  setWindowTitle(tr("Transit-Liste"));
  auto* v = new QVBoxLayout(this);
  auto* top = new QHBoxLayout();
  from_ = new QDateEdit(QDate::currentDate(), this);
  from_->setCalendarPopup(true);
  from_->setDisplayFormat("dd.MM.yyyy");
  to_ = new QDateEdit(QDate::currentDate().addMonths(3), this);
  to_->setCalendarPopup(true);
  to_->setDisplayFormat("dd.MM.yyyy");
  angle_ = new QComboBox(this);
  for (const int a : {30, 45, 60, 90, 180}) {
    angle_->addItem(QString("%1°").arg(a), a);
  }
  auto* run = new QPushButton(tr("Rechnen"), this);
  top->addWidget(new QLabel(tr("Von"), this));
  top->addWidget(from_);
  top->addWidget(new QLabel(tr("Bis"), this));
  top->addWidget(to_);
  top->addWidget(new QLabel(tr("Winkel"), this));
  top->addWidget(angle_);
  top->addWidget(run, 1);
  table_ = new QTableWidget(0, 5, this);
  table_->setHorizontalHeaderLabels({tr("Datum"), tr("Zeit (UT)"), tr("Transit"), tr("Winkel"), tr("Radix")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->verticalHeader()->setDefaultSectionSize(20);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  count_ = new QLabel(this);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  v->addLayout(top);
  v->addWidget(table_, 1);
  v->addWidget(count_);
  v->addWidget(buttons);

  connect(run, &QPushButton::clicked, this, [this]() { run_scan(); });
  connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { accept_row(row); });
  connect(buttons, &QDialogButtonBox::accepted, this, [this]() { accept_row(table_->currentRow()); });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  resize(640, 560);
}

void TransitListDialog::preset(const QDate& from, const QDate& to) {
  from_->setDate(from);
  to_->setDate(to);
}

void TransitListDialog::run_scan() {
  TransitScan scan;
  const QDate f = from_->date();
  const QDate t = to_->date();
  scan.jd_from_ut = julian_day({f.day(), f.month(), f.year(), 0, 0.0}, ctx_.settings.calendar);
  scan.jd_to_ut = julian_day({t.day(), t.month(), t.year(), 24, 0.0}, ctx_.settings.calendar);
  scan.base_angle_deg = angle_->currentData().toDouble();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  events_ = scan_transits(radix_, scan, ctx_);
  QApplication::restoreOverrideCursor();
  table_->setRowCount(0);
  for (std::size_t i = 0; i < events_.size(); ++i) {
    const TransitEvent& e = events_[i];
    const int row = table_->rowCount();
    table_->insertRow(row);
    QString time;
    const QString date = moment_text(e.jd_ut, time);
    auto* item = new QTableWidgetItem(date);
    item->setData(Qt::UserRole, static_cast<qulonglong>(i));
    table_->setItem(row, 0, item);
    table_->setItem(row, 1, new QTableWidgetItem(time));
    QString running = slot_tag(e.transiting);
    if (e.retrograde) {
      running += " R";
    }
    table_->setItem(row, 2, new QTableWidgetItem(running));
    QString angle = QString::asprintf("%g°", e.angle_deg);
    if (e.station_touch) {
      //RR stationär
      angle += " S";
    }
    table_->setItem(row, 3, new QTableWidgetItem(angle));
    table_->setItem(row, 4, new QTableWidgetItem(slot_tag(e.radix)));
  }
  count_->setText(tr("%1 Ereignisse").arg(events_.size()));
  if (table_->rowCount() > 0) {
    table_->selectRow(0);
  }
}

void TransitListDialog::accept_row(int row) {
  if (row < 0 || row >= table_->rowCount()) {
    return;
  }
  const auto idx = table_->item(row, 0)->data(Qt::UserRole).toULongLong();
  chosen_jd_ = events_[static_cast<std::size_t>(idx)].jd_ut;
  accept();
}

}  // namespace horcom
