// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "aaf_mask_dialog.hpp"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "kommen_dialog.hpp"

namespace horcom {

namespace {

QLineEdit* box(QWidget* parent, const QString& text, int chars, int max_value) {
  auto* e = new QLineEdit(text, parent);
  e->setAlignment(Qt::AlignRight);
  e->setMaxLength(chars);
  e->setFixedWidth(24 + 12 * chars);
  if (max_value > 0) {
    e->setValidator(new QIntValidator(0, max_value, e));
  }
  return e;
}

}  // namespace

AafMaskDialog::AafMaskDialog(AafRecord record, std::filesystem::path kommen, QWidget* parent)
    : QDialog(parent), base_(std::move(record)), kommen_(std::move(kommen)) {
  //RR AAF - Eingabe - und Anzeige - Box | Erläuterung mit F1
  setWindowTitle(tr("AAF- Eingabe- und Anzeige- Box   (Erläuterung mit F1)"));
  auto* v = new QVBoxLayout(this);

  //RR Standarddaten AAF-A
  auto* a = new QGroupBox(tr("Standarddaten AAF-A"), this);
  auto* ag = new QGridLayout(a);
  ag->addWidget(new QLabel(tr("Nachname"), a), 0, 0);
  surname_ = new QLineEdit(QString::fromStdString(base_.surname), a);
  ag->addWidget(surname_, 0, 1, 1, 3);
  ag->addWidget(new QLabel(tr("Vorname"), a), 0, 4);
  given_ = new QLineEdit(QString::fromStdString(base_.given), a);
  ag->addWidget(given_, 0, 5, 1, 3);
  ag->addWidget(new QLabel(tr("Horoskopart / Geschlecht"), a), 1, 0);
  sex_ = box(a, QString::fromStdString(base_.sex), 2, 0);
  ag->addWidget(sex_, 1, 1);
  ag->addWidget(new QLabel(tr("Datum  TT MM JJJJ"), a), 2, 0);
  auto* datum = new QHBoxLayout();
  day_ = box(a, QString::number(base_.day), 2, 31);
  month_ = box(a, QString::number(base_.month), 2, 12);
  year_ = box(a, QString::number(base_.year > 0 ? base_.year : 1 - base_.year), 5, 99999);
  datum->addWidget(day_);
  datum->addWidget(month_);
  datum->addWidget(year_);
  datum->addWidget(new QLabel(tr("v.Chr. 'V'"), a));
  bc_ = box(a, base_.year > 0 ? QString() : QStringLiteral("V"), 1, 0);
  datum->addWidget(bc_);
  datum->addStretch(1);
  ag->addLayout(datum, 2, 1, 1, 7);
  ag->addWidget(new QLabel(tr("Zeit  hh mm ss"), a), 3, 0);
  auto* zeit = new QHBoxLayout();
  hour_ = box(a, QString::number(base_.hour), 2, 23);
  minute_ = box(a, QString::number(base_.minute), 2, 59);
  second_ = box(a, QString::number(base_.second), 2, 59);
  zeit->addWidget(hour_);
  zeit->addWidget(minute_);
  zeit->addWidget(second_);
  zeit->addStretch(1);
  ag->addLayout(zeit, 3, 1, 1, 7);
  ag->addWidget(new QLabel(tr("Ortsname"), a), 4, 0);
  place_ = new QLineEdit(QString::fromStdString(base_.place), a);
  ag->addWidget(place_, 4, 1, 1, 5);
  ag->addWidget(new QLabel(tr("Land z.B. 'D'"), a), 4, 6);
  country_ = box(a, QString::fromStdString(base_.country), 4, 0);
  ag->addWidget(country_, 4, 7);
  v->addWidget(a);

  //RR Standarddaten AAF-B
  auto* b = new QGroupBox(tr("Standarddaten AAF-B"), this);
  auto* bg = new QGridLayout(b);
  bg->addWidget(new QLabel(tr("Breite  N/S grd min sek"), b), 0, 0);
  auto* breite = new QHBoxLayout();
  lat_ns_ = box(b, QChar(base_.lat_ns), 1, 0);
  lat_deg_ = box(b, QString::number(base_.lat_deg), 2, 89);
  lat_min_ = box(b, QString::number(base_.lat_min), 2, 59);
  lat_sec_ = box(b, QString::number(base_.lat_sec), 2, 59);
  for (QLineEdit* e : {lat_ns_, lat_deg_, lat_min_, lat_sec_}) {
    breite->addWidget(e);
  }
  breite->addStretch(1);
  bg->addLayout(breite, 0, 1);
  bg->addWidget(new QLabel(tr("Länge  E/W grd min sek"), b), 1, 0);
  auto* laenge = new QHBoxLayout();
  lon_ew_ = box(b, QChar(base_.lon_ew), 1, 0);
  lon_deg_ = box(b, QString::number(base_.lon_deg), 3, 180);
  lon_min_ = box(b, QString::number(base_.lon_min), 2, 59);
  lon_sec_ = box(b, QString::number(base_.lon_sec), 2, 59);
  for (QLineEdit* e : {lon_ew_, lon_deg_, lon_min_, lon_sec_}) {
    laenge->addWidget(e);
  }
  laenge->addStretch(1);
  bg->addLayout(laenge, 1, 1);
  bg->addWidget(new QLabel(tr("Zone (ZZD)"), b), 2, 0);
  auto* zonrow = new QHBoxLayout();
  //RR the AAF zone string, like 01hE00:00, east leads with its letter
  zone_ = new QLineEdit(QString::fromStdString(base_.zone), b);
  zone_->setFixedWidth(140);
  zonrow->addWidget(zone_);
  zonrow->addWidget(new QLabel(tr("Sommerzeit"), b));
  dst_ = box(b, QString::fromStdString(base_.dst), 1, 0);
  zonrow->addWidget(dst_);
  zonrow->addStretch(1);
  bg->addLayout(zonrow, 2, 1);
  v->addWidget(b);

  //RR Zusatzdaten AAF-C
  auto* c = new QGroupBox(tr("Zusatzdaten AAF-C"), this);
  auto* cg = new QGridLayout(c);
  cg->addWidget(new QLabel(tr("COM (alle Zeichen außer '#')"), c), 0, 0, 1, 2);
  com_ = new QPlainTextEdit(QString::fromStdString(base_.comment), c);
  com_->setFixedHeight(60);
  cg->addWidget(com_, 1, 0, 1, 2);
  const auto add_line = [&](int row, const QString& label, const std::string& value) {
    cg->addWidget(new QLabel(label, c), row, 0);
    auto* e = new QLineEdit(QString::fromStdString(value), c);
    cg->addWidget(e, row, 1);
    return e;
  };
  via_ = add_line(2, "VIA", base_.via);
  src_ = add_line(3, "SRC", base_.source);
  gzq_ = add_line(4, "GZQ", base_.quality);
  znam_ = add_line(5, "ZNAM", base_.zone_name);
  cword_ = add_line(6, "CWORD", base_.catchword);
  attrb_ = add_line(7, "ATTRB", base_.attributes);
  v->addWidget(c);

  auto* buttons = new QDialogButtonBox(this);
  //RR AAF-Help, the F1 text of the box
  auto* help = buttons->addButton(tr("AAF-Help"), QDialogButtonBox::HelpRole);
  connect(help, &QPushButton::clicked, this, [this]() {
    KommenDialog dialog(kommen_, "aaf_komm", false, this);
    dialog.exec();
  });
  auto* ok = buttons->addButton(tr("OK = Speichern"), QDialogButtonBox::AcceptRole);
  ok->setDefault(true);
  buttons->addButton(tr("EXIT"), QDialogButtonBox::RejectRole);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  v->addWidget(buttons);
  resize(900, 0);
  surname_->setFocus();
}

void AafMaskDialog::keyPressEvent(QKeyEvent* event) {
  //RR VK_F1, the AAF help
  if (event->key() == Qt::Key_F1) {
    KommenDialog dialog(kommen_, "aaf_komm", false, this);
    dialog.exec();
    return;
  }
  QDialog::keyPressEvent(event);
}

AafRecord AafMaskDialog::record() const {
  AafRecord r = base_;
  r.surname = surname_->text().trimmed().toStdString();
  r.given = given_->text().trimmed().toStdString();
  r.sex = sex_->text().trimmed().toStdString();
  r.place = place_->text().trimmed().toStdString();
  r.country = country_->text().trimmed().toUpper().toStdString();
  r.day = day_->text().toInt();
  r.month = month_->text().toInt();
  const int year = year_->text().toInt();
  //RR 'V' macht aus dem historischen Jahr die astronomische Zählung
  r.year = bc_->text().trimmed().toUpper().startsWith('V') ? 1 - year : year;
  r.hour = hour_->text().toInt();
  r.minute = minute_->text().toInt();
  r.second = second_->text().toInt();
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
  r.lat_ns = letter(lat_ns_, base_.lat_ns, 'N', 'S');
  r.lat_deg = lat_deg_->text().toInt();
  r.lat_min = lat_min_->text().toInt();
  r.lat_sec = lat_sec_->text().toInt();
  r.lon_ew = letter(lon_ew_, base_.lon_ew, 'E', 'W');
  r.lon_deg = lon_deg_->text().toInt();
  r.lon_min = lon_min_->text().toInt();
  r.lon_sec = lon_sec_->text().toInt();
  r.zone = zone_->text().trimmed().toStdString();
  r.dst = dst_->text().trimmed().toStdString();
  r.comment = com_->toPlainText().trimmed().toStdString();
  r.via = via_->text().trimmed().toStdString();
  r.source = src_->text().trimmed().toStdString();
  r.quality = gzq_->text().trimmed().toStdString();
  r.zone_name = znam_->text().trimmed().toStdString();
  r.catchword = cword_->text().trimmed().toStdString();
  r.attributes = attrb_->text().trimmed().toStdString();
  // edits to the clock invalidate a stored julian date, the fields rule
  if (r.day != base_.day || r.month != base_.month || r.year != base_.year || r.hour != base_.hour ||
      r.minute != base_.minute || r.second != base_.second) {
    r.jd = 0.0;
  }
  return r;
}

}  // namespace horcom
