// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "direction_list_dialog.hpp"

#include <cmath>

#include <QApplication>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/calendar.hpp"

namespace horcom {

// the 25.07.03 change, hard aspects red and harmonic ones green
inline void color_aspect_item(QTableWidgetItem* item, double angle_deg) {
  const double a = std::fmod(std::abs(angle_deg), 360.0);
  const auto near = [a](double w) { return std::abs(a - w) < 0.5; };
  if (near(0.0) || near(90.0) || near(180.0) || near(270.0) || near(360.0)) {
    item->setForeground(QColor(0xE8, 0x5D, 0x4E));
  } else if (near(60.0) || near(120.0) || near(240.0) || near(300.0)) {
    item->setForeground(QColor(0x3F, 0xB6, 0x50));
  }
}


namespace {

// the extras of the direction runs live on slots 15 to 18
QString slot_tag(int slot, DirectionExtras extras) {
  if (slot >= 15 && slot <= 18) {
    if (extras == DirectionExtras::kCusps) {
      static constexpr const char* kCusp[4] = {"H2", "H3", "H5", "H6"};
      return kCusp[slot - 15];
    }
    static constexpr const char* kCardinal[4] = {"0°AR", "0°CN", "0°LI", "0°CP"};
    return QString::fromUtf8(kCardinal[slot - 15]);
  }
  return QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                           static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size()));
}

}  // namespace

DirectionListDialog::DirectionListDialog(Chart radix, SearchContext ctx, QWidget* parent)
    : QDialog(parent), radix_(std::move(radix)), ctx_(std::move(ctx)) {
  setWindowTitle(tr("Direktionen-Auswertung"));
  auto* v = new QVBoxLayout(this);
  auto* top = new QHBoxLayout();
  method_ = new QComboBox(this);
  method_->addItem(tr("Symbolisch ekliptikal"), 0);
  method_->addItem(tr("Symbolisch äquatorial (AR)"), 1);
  method_->addItem(tr("Symbolisch mundan"), 2);
  method_->addItem(QString::fromUtf8("Primär (E.C. Kühr)"), 3);
  method_->addItem(QString::fromUtf8("Sekundär-Direktion"), 4);
  method_->addItem(tr("Sonnenbogen-Direktion"), 5);
  method_->addItem(tr("Mondbogen-Direktion"), 6);
  top->addWidget(new QLabel(tr("Methode"), this));
  top->addWidget(method_, 1);
  v->addLayout(top);

  auto* row = new QHBoxLayout();
  from_years_ = new QDoubleSpinBox(this);
  from_years_->setRange(0.0, 180.0);
  from_years_->setDecimals(1);
  to_years_ = new QDoubleSpinBox(this);
  to_years_->setRange(0.0, 180.0);
  to_years_->setDecimals(1);
  to_years_->setValue(90.0);
  from_date_ = new QDateEdit(QDate::currentDate(), this);
  from_date_->setCalendarPopup(true);
  from_date_->setDisplayFormat("dd.MM.yyyy");
  to_date_ = new QDateEdit(QDate::currentDate().addYears(1), this);
  to_date_->setCalendarPopup(true);
  to_date_->setDisplayFormat("dd.MM.yyyy");
  angle_ = new QComboBox(this);
  for (const int a : {30, 45, 60, 90, 180, 360}) {
    angle_->addItem(QString("%1°").arg(a), a);
  }
  key_ = new QDoubleSpinBox(this);
  key_->setRange(0.1, 10.0);
  key_->setDecimals(2);
  //RR Schlüssel, ein Grad je Jahr
  key_->setValue(1.0);
  extras_ = new QComboBox(this);
  extras_->addItem(tr("Ohne Zusatzpunkte"), 0);
  extras_->addItem(QString::fromUtf8("Zwischenhäuser"), 1);
  extras_->addItem(tr("Kardinalpunkte"), 2);
  row->addWidget(new QLabel(tr("Alter von"), this));
  row->addWidget(from_years_);
  row->addWidget(new QLabel(tr("bis"), this));
  row->addWidget(to_years_);
  row->addWidget(from_date_);
  row->addWidget(to_date_);
  row->addWidget(new QLabel(tr("Winkel"), this));
  row->addWidget(angle_);
  row->addWidget(new QLabel(QString::fromUtf8("Schlüssel"), this));
  row->addWidget(key_);
  row->addWidget(extras_);
  auto* run = new QPushButton(tr("Rechnen"), this);
  row->addWidget(run);
  v->addLayout(row);

  table_ = new QTableWidget(0, 6, this);
  table_->setHorizontalHeaderLabels({tr("Alter"), tr("Datum"), tr("Gerichtet"), tr("Winkel"), tr("Ziel"), tr("Art")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->verticalHeader()->setDefaultSectionSize(20);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  count_ = new QLabel(this);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
  v->addWidget(table_, 1);
  v->addWidget(count_);
  v->addWidget(buttons);

  connect(method_, &QComboBox::currentIndexChanged, this, [this](int) { update_fields(); });
  connect(run, &QPushButton::clicked, this, [this]() { run_scan(); });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  update_fields();
  resize(860, 600);
}

void DirectionListDialog::update_fields() {
  const bool dated = method_->currentData().toInt() >= 4;
  from_years_->setVisible(!dated);
  to_years_->setVisible(!dated);
  key_->setVisible(!dated);
  extras_->setVisible(!dated);
  from_date_->setVisible(dated);
  to_date_->setVisible(dated);
}

void DirectionListDialog::run_scan() {
  const int m = method_->currentData().toInt();
  table_->setRowCount(0);
  QApplication::setOverrideCursor(Qt::WaitCursor);
  if (m <= 3) {
    DirectionRange range;
    range.from_years = from_years_->value();
    range.to_years = to_years_->value();
    range.base_angle_deg = angle_->currentData().toDouble();
    range.key = key_->value();
    range.extras = static_cast<DirectionExtras>(extras_->currentData().toInt());
    static constexpr DirectionMethod kMethod[4] = {DirectionMethod::kSymbolicEcliptic, DirectionMethod::kSymbolicEquatorial,
                                                   DirectionMethod::kSymbolicMundane, DirectionMethod::kPrimary};
    std::vector<DirectionHit> hits = direction_hits(radix_, kMethod[m], range, ctx_.base.lat_deg);
    std::sort(hits.begin(), hits.end(), [](const DirectionHit& a, const DirectionHit& b) { return a.years < b.years; });
    for (const DirectionHit& h : hits) {
      const int row = table_->rowCount();
      table_->insertRow(row);
      table_->setItem(row, 0, new QTableWidgetItem(QString::asprintf("%7.2f", h.years)));
      // the age lands on a calendar day through the year length
      const double jd = radix_.jd_ut + h.years * radix_.ta.tropical_year_days;
      const CalendarDate d = calendar_date(jd, ctx_.settings.calendar);
      table_->setItem(row, 1, new QTableWidgetItem(QString::asprintf("%02d.%02d.%04d", d.day, d.month, d.year)));
      table_->setItem(row, 2, new QTableWidgetItem(slot_tag(h.directed, range.extras)));
      auto* angle_item = new QTableWidgetItem(QString::asprintf("%g°", h.multiple * range.base_angle_deg));
      color_aspect_item(angle_item, h.multiple * range.base_angle_deg);
      table_->setItem(row, 3, angle_item);
      table_->setItem(row, 4, new QTableWidgetItem(slot_tag(h.target, range.extras)));
      //RR D / K
      table_->setItem(row, 5, new QTableWidgetItem(h.converse ? "K" : "D"));
    }
    count_->setText(tr("%1 Direktionen").arg(table_->rowCount()));
  } else {
    const QDate f = from_date_->date();
    const QDate t = to_date_->date();
    const double jd_from = julian_day({f.day(), f.month(), f.year(), 0, 0.0}, ctx_.settings.calendar);
    const double jd_to = julian_day({t.day(), t.month(), t.year(), 24, 0.0}, ctx_.settings.calendar);
    const double base = angle_->currentData().toDouble();
    std::vector<DirectedEvent> events;
    if (m == 4) {
      events = secondary_direction_events(radix_, jd_from, jd_to, base, ctx_);
    } else {
      events = arc_direction_events(radix_, m == 6, jd_from, jd_to, base, ctx_);
    }
    for (const DirectedEvent& e : events) {
      const int row = table_->rowCount();
      table_->insertRow(row);
      const double years = (e.jd_life_ut - radix_.jd_ut) / radix_.ta.tropical_year_days;
      table_->setItem(row, 0, new QTableWidgetItem(QString::asprintf("%7.2f", years)));
      const CalendarDate d = calendar_date(e.jd_life_ut, ctx_.settings.calendar);
      table_->setItem(row, 1, new QTableWidgetItem(QString::asprintf("%02d.%02d.%04d", d.day, d.month, d.year)));
      QString moving = slot_tag(e.event.transiting, DirectionExtras::kNone);
      if (e.event.retrograde && m == 4) {
        moving += " R";
      }
      table_->setItem(row, 2, new QTableWidgetItem(moving));
      auto* ev_angle = new QTableWidgetItem(QString::asprintf("%g°", e.event.angle_deg));
      color_aspect_item(ev_angle, e.event.angle_deg);
      table_->setItem(row, 3, ev_angle);
      table_->setItem(row, 4, new QTableWidgetItem(slot_tag(e.event.radix, DirectionExtras::kNone)));
      table_->setItem(row, 5, new QTableWidgetItem("D"));
    }
    count_->setText(tr("%1 Ereignisse").arg(table_->rowCount()));
  }
  QApplication::restoreOverrideCursor();
}

}  // namespace horcom
