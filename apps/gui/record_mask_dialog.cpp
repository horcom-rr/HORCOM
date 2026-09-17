// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "record_mask_dialog.hpp"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <cstdlib>

#include "aaf_mask_dialog.hpp"

namespace horcom {

namespace {

QLineEdit* small_box(QWidget* parent, const QString& text, int chars, int max_value) {
  auto* e = new QLineEdit(text, parent);
  e->setAlignment(Qt::AlignRight);
  e->setMaxLength(chars);
  e->setFixedWidth(26 + 12 * chars);
  if (max_value > 0) {
    e->setValidator(new QIntValidator(0, max_value, e));
  }
  return e;
}

}  // namespace

RecordMaskDialog::RecordMaskDialog(AafRecord record, const QString& title, Mode mode,
                                   std::filesystem::path kommen, QWidget* parent)
    : QDialog(parent), base_(std::move(record)), kommen_(std::move(kommen)) {
  //RR EINGABE- und ANZEIGE-BOX
  setWindowTitle(title);
  auto* v = new QVBoxLayout(this);
  v->setContentsMargins(16, 14, 16, 14);
  v->setSpacing(14);

  joined_name_ = (QString::fromStdString(base_.surname).trimmed() + " " +
                  QString::fromStdString(base_.given).trimmed())
                     .trimmed();
  auto* row1 = new QHBoxLayout();
  row1->addWidget(new QLabel(tr("NAME,VORNAME"), this));
  name_ = new QLineEdit(joined_name_, this);
  row1->addWidget(name_, 3);
  row1->addSpacing(10);
  row1->addWidget(new QLabel(tr("ORT"), this));
  place_ = new QLineEdit(QString::fromStdString(base_.place), this);
  row1->addWidget(place_, 2);
  v->addLayout(row1);

  auto* row2 = new QHBoxLayout();
  row2->addWidget(new QLabel(tr("LÄNGE O/W"), this));
  lon_ew_ = small_box(this, QChar(base_.lon_ew), 1, 0);
  row2->addWidget(lon_ew_);
  row2->addWidget(new QLabel(QStringLiteral("°"), this));
  lon_deg_ = small_box(this, QString::number(base_.lon_deg), 3, 180);
  row2->addWidget(lon_deg_);
  row2->addWidget(new QLabel(QStringLiteral("'"), this));
  lon_min_ = small_box(this, QString::number(base_.lon_min), 2, 59);
  row2->addWidget(lon_min_);
  row2->addWidget(new QLabel(QStringLiteral("''"), this));
  lon_sec_ = small_box(this, QString::number(base_.lon_sec), 2, 59);
  row2->addWidget(lon_sec_);
  row2->addStretch(1);
  row2->addWidget(new QLabel(tr("BREITE N/S"), this));
  lat_ns_ = small_box(this, QChar(base_.lat_ns), 1, 0);
  row2->addWidget(lat_ns_);
  row2->addWidget(new QLabel(QStringLiteral("°"), this));
  lat_deg_ = small_box(this, QString::number(base_.lat_deg), 2, 89);
  row2->addWidget(lat_deg_);
  row2->addWidget(new QLabel(QStringLiteral("'"), this));
  lat_min_ = small_box(this, QString::number(base_.lat_min), 2, 59);
  row2->addWidget(lat_min_);
  row2->addWidget(new QLabel(QStringLiteral("''"), this));
  lat_sec_ = small_box(this, QString::number(base_.lat_sec), 2, 59);
  row2->addWidget(lat_sec_);
  v->addLayout(row2);

  auto* row3 = new QHBoxLayout();
  row3->addWidget(new QLabel(tr("WENN V.CHR.,  'V' EINGEBEN"), this));
  bc_ = small_box(this, base_.year > 0 ? QString() : QStringLiteral("V"), 1, 0);
  row3->addWidget(bc_);
  row3->addStretch(1);
  row3->addWidget(new QLabel(tr("DATUM :  TT"), this));
  day_ = small_box(this, QString::number(base_.day), 2, 31);
  row3->addWidget(day_);
  row3->addWidget(new QLabel(QStringLiteral("MM"), this));
  month_ = small_box(this, QString::number(base_.month), 2, 12);
  row3->addWidget(month_);
  row3->addWidget(new QLabel(QStringLiteral("JJJJ"), this));
  //RR WENN V.CHR., 'V' EINGEBEN, the year field then counts historically
  year_ = small_box(this, QString::number(base_.year > 0 ? base_.year : 1 - base_.year), 5, 99999);
  row3->addWidget(year_);
  v->addLayout(row3);

  auto* row4 = new QHBoxLayout();
  // a stored zone means the clock is a local one like his UHRZEIT label
  const bool zoned = !base_.zone.empty() && base_.zone != "00hE00:00" && std::atof(base_.zone.c_str()) != 0.0;
  row4->addWidget(new QLabel(zoned ? tr("UHRZEIT : hh") : tr("WZ = GMT = UT : hh"), this));
  hour_ = small_box(this, QString::number(base_.hour), 2, 23);
  row4->addWidget(hour_);
  row4->addWidget(new QLabel(QStringLiteral("mm"), this));
  minute_ = small_box(this, QString::number(base_.minute), 2, 59);
  row4->addWidget(minute_);
  row4->addWidget(new QLabel(QStringLiteral("ss"), this));
  second_ = small_box(this, QString::number(base_.second), 2, 59);
  row4->addWidget(second_);
  row4->addStretch(2);
  v->addLayout(row4);

  auto* row5 = new QHBoxLayout();
  row5->addStretch(1);
  row5->addWidget(new QLabel(tr("BEMERKUNG:"), this));
  comment_ = new QLineEdit(QString::fromStdString(base_.comment), this);
  comment_->setMaxLength(51);
  row5->addWidget(comment_, 3);
  auto* ok = new QPushButton(tr("OK"), this);
  ok->setDefault(true);
  connect(ok, &QPushButton::clicked, this, &QDialog::accept);
  row5->addWidget(ok);
  v->addLayout(row5);

  auto* row6 = new QHBoxLayout();
  //RR AAF - Format, the button that switches into the richer AAF box
  if (!kommen_.empty()) {
    auto* aaf = new QPushButton(tr("AAF-Format"), this);
    connect(aaf, &QPushButton::clicked, this, &RecordMaskDialog::open_aaf_box);
    row6->addWidget(aaf);
  }
  row6->addStretch(1);
  if (mode == Mode::kEntry) {
    auto* cancel = new QPushButton(tr("ABBRUCH"), this);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    row6->addWidget(cancel);
  }
  if (!kommen_.empty() || mode == Mode::kEntry) {
    v->addLayout(row6);
  }
  resize(760, 0);
  name_->setFocus();
}

// ported from the AAF-Format button of eing_box, it carries the current
// fields into the richer AAF box, an accept there fills this box and
// closes it so the fetch or entry flow takes the AAF edited record
void RecordMaskDialog::open_aaf_box() {
  AafMaskDialog dialog(record(), kommen_, this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  aaf_result_ = dialog.record();
  accept();
}

AafRecord RecordMaskDialog::record() const {
  // the AAF box, when used, supplied the whole record already
  if (aaf_result_) {
    return *aaf_result_;
  }
  AafRecord r = base_;
  const QString name = name_->text().trimmed();
  if (name != joined_name_) {
    // an edited name lands whole in the first field like the 25 byte
    // DAT name did, the split into surname and given name is gone
    r.surname = name.toStdString();
    r.given.clear();
  }
  r.place = place_->text().trimmed().toStdString();
  const auto letter = [](const QLineEdit* e, char fallback, char a, char b) {
    const QString t = e->text().trimmed().toUpper();
    if (t.startsWith(QChar(a))) {
      return a;
    }
    if (t.startsWith(QChar(b))) {
      return b;
    }
    return fallback;
  };
  r.lon_ew = letter(lon_ew_, base_.lon_ew, 'E', 'W');
  r.lon_deg = lon_deg_->text().toInt();
  r.lon_min = lon_min_->text().toInt();
  r.lon_sec = lon_sec_->text().toInt();
  r.lat_ns = letter(lat_ns_, base_.lat_ns, 'N', 'S');
  r.lat_deg = lat_deg_->text().toInt();
  r.lat_min = lat_min_->text().toInt();
  r.lat_sec = lat_sec_->text().toInt();
  r.day = day_->text().toInt();
  r.month = month_->text().toInt();
  const int year = year_->text().toInt();
  //RR 'V' macht aus dem historischen Jahr die astronomische Zählung
  r.year = bc_->text().trimmed().toUpper().startsWith('V') ? 1 - year : year;
  r.hour = hour_->text().toInt();
  r.minute = minute_->text().toInt();
  r.second = second_->text().toInt();
  r.comment = comment_->text().trimmed().toStdString();
  // edits invalidate a stored julian date, the clock fields rule again
  if (r.day != base_.day || r.month != base_.month || r.year != base_.year || r.hour != base_.hour ||
      r.minute != base_.minute || r.second != base_.second) {
    r.jd = 0.0;
  }
  return r;
}

}  // namespace horcom
