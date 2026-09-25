// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "direction_list_dialog.hpp"

#include <algorithm>
#include <cmath>

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "a18_rows.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/time/calendar.hpp"

namespace horcom {

namespace {

constexpr int kColWhen = 0;
constexpr int kColDirected = 1;
constexpr int kColAngle = 2;
constexpr int kColTarget = 3;
constexpr int kColKind = 4;

// a point of a direction, a body, a cardinal point, a cusp or a midpoint
QString point_tag(int slot, int cusp, int partner) {
  if (cusp > 0) {
    return QString("H%1").arg(cusp);
  }
  if (partner > 0) {
    return a18::slot_tag(slot) + "/" + a18::slot_tag(partner);
  }
  if (body::cardinal(slot)) {
    static constexpr const char* kCardinal[4] = {"0°AR", "0°CN", "0°LI", "0°CP"};
    return QString::fromUtf8(kCardinal[slot - body::kAriesPoint]);
  }
  return a18::slot_tag(slot);
}

}  // namespace

DirectionListDialog::DirectionListDialog(std::vector<DirectionHit> hits, const Chart& radix, DirectionDisplay display,
                                         QWidget* parent)
    : QDialog(parent), display_(std::move(display)) {
  // the walk lists by pairs, his table showed them in that order
  rows_.reserve(hits.size());
  for (std::size_t i = 0; i < hits.size(); ++i) {
    const DirectionHit& h = hits[i];
    Row r;
    r.order = i;
    r.years = h.years;
    r.jd = radix.jd_ut + h.years * radix.ta.tropical_year_days;
    r.arc_deg = h.arc_deg;
    const double angle = h.multiple * display_.base_angle_deg;
    r.angle_deg = angle > 180.0 ? kDegPerCircle - angle : angle;
    r.directed = h.directed_cusp > 0 ? 0 : h.directed;
    r.directed_text = point_tag(h.directed, h.directed_cusp, 0);
    r.target = h.target_cusp > 0 || h.target2 > 0 ? 0 : h.target;
    r.target_text = point_tag(h.target, h.target_cusp, h.target2);
    // drg$ = "D" oder "K"
    r.kind = h.converse ? "K" : "D";
    rows_.push_back(r);
  }
  build();
}

DirectionListDialog::DirectionListDialog(std::vector<DirectedEvent> events, const Chart& radix, DirectionDisplay display,
                                         QWidget* parent)
    : QDialog(parent), display_(std::move(display)) {
  rows_.reserve(events.size());
  for (std::size_t i = 0; i < events.size(); ++i) {
    const DirectedEvent& e = events[i];
    Row r;
    r.order = i;
    r.jd = e.jd_life_ut;
    r.years = (e.jd_life_ut - radix.jd_ut) / radix.ta.tropical_year_days;
    r.angle_deg = e.event.angle_deg > 180.0 ? kDegPerCircle - e.event.angle_deg : e.event.angle_deg;
    r.directed = e.event.transiting;
    r.directed_text = a18::slot_tag(e.event.transiting) + (e.event.retrograde ? " R" : "");
    r.target = e.event.cusp > 0 || e.event.radix2 > 0 ? 0 : e.event.radix;
    r.target_text = point_tag(e.event.radix, e.event.cusp, e.event.radix2);
    // the progressed body reaches its radix point directly
    r.kind = "D";
    rows_.push_back(r);
  }
  build();
}

void DirectionListDialog::build() {
  setWindowTitle(display_.heading.isEmpty() ? QString("HORCOM") : display_.heading.front().trimmed());
  auto* v = new QVBoxLayout(this);
  for (const QString& line : display_.heading) {
    auto* l = new QLabel(line, this);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    v->addWidget(l);
  }
  table_ = new QTableWidget(0, 5, this);
  // LJ MO, BOGEN or the date, then SIG PRO or PL1 PL2 and ASP
  QString when = tr("Datum");
  if (display_.format == DirectionFormat::kAge) {
    when = tr("LJ  MO");
  } else if (display_.format == DirectionFormat::kArc || display_.format == DirectionFormat::kArcDecimal) {
    when = tr("BOGEN");
  }
  table_->setHorizontalHeaderLabels({when, tr("Gerichtet"), tr("Winkel"), tr("Ziel"), tr("Art")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  a18::style_table(table_, display_.small_symbols);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  count_ = new QLabel(this);
  auto* sort = new QPushButton(tr("LISTE SORTIEREN…"), this);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
  auto* bottom = new QHBoxLayout();
  bottom->addWidget(count_, 1);
  bottom->addWidget(sort);
  v->addWidget(table_, 1);
  if (!display_.footer.isEmpty()) {
    v->addWidget(new QLabel(display_.footer, this));
  }
  v->addLayout(bottom);
  v->addWidget(buttons);
  connect(sort, &QPushButton::clicked, this, [this]() {
    const int mode = a18::ask_sort(this, display_.base_angle_deg);
    if (mode >= 0) {
      sort_rows(mode);
    }
  });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  fill();
  resize(620, 600);
}

void DirectionListDialog::fill() {
  table_->setRowCount(0);
  for (const Row& r : rows_) {
    const int row = table_->rowCount();
    table_->insertRow(row);
    QString when;
    switch (display_.format) {
      case DirectionFormat::kAge: {
        // a175, completed years and the month with one decimal
        const double years = std::max(0.0, r.years);
        const int lj = static_cast<int>(std::floor(years));
        when = QString::asprintf("%3d %4.1f", lj, kMonthsPerYear * (years - lj));
        break;
      }
      case DirectionFormat::kArc: {
        const int total = static_cast<int>(std::lround(r.arc_deg * kArcsecPerDeg));
        when = QString::asprintf("%3d°%02d'%02d\"", total / 3600, (total / 60) % 60, total % 60);
        break;
      }
      case DirectionFormat::kArcDecimal:
        when = QString::asprintf("%8.4f°", r.arc_deg);
        break;
      case DirectionFormat::kDate: {
        const CalendarDate d = calendar_date(r.jd, display_.calendar);
        when = QString::asprintf("%02d.%02d.%04d", d.day, d.month, d.year);
        break;
      }
    }
    table_->setItem(row, kColWhen, new QTableWidgetItem(when));
    //RR EINZELNE PLANETEN ROT MARKIEREN
    auto* directed = new QTableWidgetItem(r.directed_text);
    a18::dress_factor(directed, r.directed, true, display_.plinv, display_.marked);
    table_->setItem(row, kColDirected, directed);
    auto* angle = new QTableWidgetItem(QString::asprintf("%3.0f", r.angle_deg));
    a18::colour_aspect(angle, r.angle_deg, display_.plinv);
    table_->setItem(row, kColAngle, angle);
    auto* target = new QTableWidgetItem(r.target_text);
    a18::dress_factor(target, r.target, false, display_.plinv, display_.marked);
    table_->setItem(row, kColTarget, target);
    table_->setItem(row, kColKind, new QTableWidgetItem(r.kind));
  }
  count_->setText(tr("%1 Auslösungen").arg(rows_.size()));
  table_->resizeColumnsToContents();
}

// ported from the sort question of dirend
void DirectionListDialog::sort_rows(int mode) {
  a18::sort_rows(rows_, mode, &Row::years);
  fill();
}

// ported from dirend, the question comes before the first page and ESC
// keeps the order
int DirectionListDialog::open_sorted() {
  if (!rows_.empty()) {
    const int mode = a18::ask_sort(parentWidget(), display_.base_angle_deg);
    if (mode >= 0) {
      sort_rows(mode);
    }
  }
  return exec();
}

int DirectionListDialog::row_count() const {
  return table_->rowCount();
}

QStringList DirectionListDialog::row_texts(int row) const {
  QStringList out;
  for (int c = 0; c < table_->columnCount(); ++c) {
    const QTableWidgetItem* item = table_->item(row, c);
    out << (item != nullptr ? item->text() : QString());
  }
  return out;
}

}  // namespace horcom
