// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "transit_list_dialog.hpp"

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
#include "horcom/chart/signs.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the fixed columns of both tables, the mundane one adds its positions
constexpr int kColDate = 0;
constexpr int kColTime = 1;
constexpr int kColFirst = 2;
constexpr int kColAngle = 3;
constexpr int kColSecond = 4;
constexpr int kColPositions = 5;

// the radix point of a transit, a body, a midpoint, a cusp or a cardinal point
QString target_tag(const TransitEvent& e) {
  if (e.cusp > 0) {
    return QString("H%1").arg(e.cusp);
  }
  if (e.radix2 > 0) {
    return a18::slot_tag(e.radix) + "/" + a18::slot_tag(e.radix2);
  }
  return a18::slot_tag(e.radix);
}

// the gz3$ form of grze, degree, sign tag and minute
QString degree_sign_minute(double lon) {
  const int per_circle = static_cast<int>(kDegPerCircle * kArcminPerDeg);
  const int per_sign = static_cast<int>(kDegPerSign * kArcminPerDeg);
  int total = static_cast<int>(std::lround(norm_deg(lon * kRadToDeg) * kArcminPerDeg));
  if (total >= per_circle) {
    total = 0;
  }
  const int sign = total / per_sign;
  const int in_sign = total - sign * per_sign;
  return QString::asprintf("%2d%s%02d", in_sign / 60, kSignTag[sign], in_sign % 60);
}

// the time resolution classes of a181tx1 and a181_discr
struct TimeClass {
  bool hours = false;
  bool minutes = false;
};

// a181_discr, the class of a point the running body meets
TimeClass point_class(int slot, const ChartSettings& s) {
  if ((slot >= body::kSun && slot <= body::kMars) || (slot >= body::kAscendant && slot <= body::kCapricornPoint)) {
    return {true, true};
  }
  if ((slot >= body::kJupiter && slot <= body::kNeptune) || slot == body::kFortune) {
    return {true, false};
  }
  if (slot == body::kNodeAsc || slot == body::kNodeDesc) {
    return {true, !s.true_node};
  }
  if (slot == body::kApogee) {
    return {true, !s.true_apogee};
  }
  if (slot == body::kPluto || slot == body::kChiron || (slot >= body::kCeres && slot <= body::kVesta) ||
      slot >= body::kQuaoar) {
    return {true, false};
  }
  return {};
}

// the SELECT t& of a181tx1, the class of the running body, a body near
// its station keeps to hours
TimeClass running_class(int slot, const ChartSettings& s, double speed) {
  const bool slow = std::abs(speed) < kStationDailyMotion;
  if ((slot >= body::kSun && slot <= body::kMars) || slot == body::kFortune || slot == body::kAscendant ||
      slot == body::kMc) {
    return {true, !slow};
  }
  if (slot >= body::kJupiter && slot <= body::kNeptune) {
    return {true, false};
  }
  if (slot == body::kNodeAsc || slot == body::kNodeDesc) {
    return {true, !s.true_node && !slow};
  }
  if (slot == body::kApogee) {
    return {true, !s.true_apogee && !slow};
  }
  if (slot == body::kPluto || slot == body::kChiron || (slot >= body::kCeres && slot <= body::kVesta) ||
      slot >= body::kQuaoar) {
    return {true, false};
  }
  return {};
}

// the date and the clock of a moment at the resolution the row earned,
// in the calendar of the run
void moment_texts(double jd_ut, bool hours, bool minutes, Calendar cal, QString& date, QString& time) {
  double jd = jd_ut;
  if (minutes) {
    jd = std::floor(jd_ut * kMinutesPerDay + 0.5) / kMinutesPerDay;
  } else if (hours) {
    jd = std::floor(jd_ut * kHoursPerDay + 0.5) / kHoursPerDay;
  }
  const CalendarDate d = calendar_date(jd, cal);
  const int seconds = a18::clock_seconds(d);
  date = QString::asprintf("%02d.%02d.%04d", d.day, d.month, d.year);
  if (minutes) {
    time = QString::asprintf("%02d:%02d", seconds / 3600, (seconds / 60) % 60);
  } else if (hours) {
    time = QString::asprintf("%02d h", seconds / 3600);
  } else {
    time.clear();
  }
}

}  // namespace

TransitListDialog::TransitListDialog(std::vector<TransitEvent> events, A18Display display, QWidget* parent)
    : QDialog(parent), display_(std::move(display)) {
  rows_.reserve(events.size());
  for (std::size_t i = 0; i < events.size(); ++i) {
    const TransitEvent& e = events[i];
    Row r;
    r.jd_ut = e.jd_ut;
    r.order = i;
    r.first = e.transiting;
    r.second = e.cusp > 0 || e.radix2 > 0 ? 0 : e.radix;
    r.second_text = target_tag(e);
    r.angle_deg = e.angle_deg > 180.0 ? kDegPerCircle - e.angle_deg : e.angle_deg;
    r.retrograde = e.retrograde;
    r.station = e.station_touch;
    // tras, the running class decides and the point needs any class
    const TimeClass run = running_class(e.transiting, display_.settings, e.speed);
    const TimeClass point = point_class(e.cusp > 0 ? body::kAscendant : e.radix, display_.settings);
    r.hours = run.hours;
    r.minutes = run.minutes && (point.hours || point.minutes);
    rows_.push_back(r);
  }
  build(false);
}

TransitListDialog::TransitListDialog(std::vector<MundaneAspect> aspects, A18Display display, QWidget* parent)
    : QDialog(parent), display_(std::move(display)) {
  rows_.reserve(aspects.size());
  for (std::size_t i = 0; i < aspects.size(); ++i) {
    const MundaneAspect& m = aspects[i];
    Row r;
    r.jd_ut = m.jd_ut;
    r.order = i;
    r.first = m.first;
    r.second = m.second;
    r.second_text = a18::slot_tag(m.second);
    r.angle_deg = m.angle_deg;
    r.positions = degree_sign_minute(m.first_lon) + "  " + degree_sign_minute(m.second_lon);
    // mund1 hands a181 no speed, so the second body counts as stationary
    const TimeClass u = point_class(m.first, display_.settings);
    const TimeClass t = running_class(m.second, display_.settings, 0.0);
    r.minutes = (t.minutes && (u.minutes || u.hours)) || (u.minutes && (t.minutes || t.hours));
    r.hours = r.minutes || (t.hours && u.hours) || (!t.hours && u.minutes) || (!u.hours && t.minutes);
    rows_.push_back(r);
  }
  build(true);
}

void TransitListDialog::build(bool mundane) {
  mundane_ = mundane;
  //RR TRANSITE, Ekliptikale Mundan-Aspekte
  setWindowTitle(display_.heading.isEmpty() ? QString("HORCOM") : display_.heading.front().trimmed());
  auto* v = new QVBoxLayout(this);
  for (const QString& line : display_.heading) {
    auto* l = new QLabel(line, this);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    v->addWidget(l);
  }
  table_ = new QTableWidget(0, mundane ? 6 : 5, this);
  QStringList heads{tr("Datum"), tr("Zeit (UT)"), tr("Laufend"), tr("Winkel"), mundane ? tr("Laufend") : tr("Radix")};
  if (mundane) {
    heads << tr("Positionen");
  }
  table_->setHorizontalHeaderLabels(heads);
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  a18::style_table(table_, display_.small_symbols);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  count_ = new QLabel(this);
  auto* sort = new QPushButton(tr("LISTE SORTIEREN…"), this);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
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
  connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { accept_row(row); });
  connect(buttons, &QDialogButtonBox::accepted, this, [this]() { accept_row(table_->currentRow()); });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  fill();
  resize(mundane ? 700 : 620, 580);
}

void TransitListDialog::fill() {
  table_->setRowCount(0);
  for (std::size_t i = 0; i < rows_.size(); ++i) {
    const Row& r = rows_[i];
    const int row = table_->rowCount();
    table_->insertRow(row);
    QString date;
    QString time;
    moment_texts(r.jd_ut, r.hours, r.minutes, display_.settings.calendar, date, time);
    auto* date_item = new QTableWidgetItem(date);
    date_item->setData(Qt::UserRole, static_cast<qulonglong>(i));
    table_->setItem(row, kColDate, date_item);
    auto* time_item = new QTableWidgetItem(r.station ? QString("STATION") : time);
    if (r.station) {
      time_item->setForeground(a18::kTableRed);
    }
    table_->setItem(row, kColTime, time_item);
    // only running bodies wear the red mark, both tables invert
    auto* first = new QTableWidgetItem(a18::slot_tag(r.first) + (r.retrograde ? " R" : ""));
    a18::dress_factor(first, r.first, true, display_.plinv, display_.marked);
    table_->setItem(row, kColFirst, first);
    auto* angle = new QTableWidgetItem(QString::asprintf("%3.0f", r.angle_deg));
    a18::colour_aspect(angle, r.angle_deg, display_.plinv);
    table_->setItem(row, kColAngle, angle);
    auto* second = new QTableWidgetItem(r.second_text);
    a18::dress_factor(second, r.second, mundane_, display_.plinv, display_.marked);
    table_->setItem(row, kColSecond, second);
    if (mundane_) {
      table_->setItem(row, kColPositions, new QTableWidgetItem(r.positions));
    }
  }
  count_->setText(tr("%1 Auslösungen").arg(rows_.size()));
  table_->resizeColumnsToContents();
  if (table_->rowCount() > 0) {
    table_->selectRow(0);
  }
}

// ported from the sort question of dirend
void TransitListDialog::sort_rows(int mode) {
  a18::sort_rows(rows_, mode, &Row::jd_ut);
  fill();
}

// ported from dirend, the question comes before the first page and ESC
// keeps the order
int TransitListDialog::open_sorted() {
  if (!rows_.empty()) {
    const int mode = a18::ask_sort(parentWidget(), display_.base_angle_deg);
    if (mode >= 0) {
      sort_rows(mode);
    }
  }
  return exec();
}

int TransitListDialog::row_count() const {
  return table_->rowCount();
}

QStringList TransitListDialog::row_texts(int row) const {
  QStringList out;
  for (int c = 0; c < table_->columnCount(); ++c) {
    const QTableWidgetItem* item = table_->item(row, c);
    out << (item != nullptr ? item->text() : QString());
  }
  return out;
}

void TransitListDialog::accept_row(int row) {
  if (row < 0 || row >= table_->rowCount()) {
    return;
  }
  const auto idx = table_->item(row, kColDate)->data(Qt::UserRole).toULongLong();
  chosen_jd_ = rows_[static_cast<std::size_t>(idx)].jd_ut;
  accept();
}

}  // namespace horcom
