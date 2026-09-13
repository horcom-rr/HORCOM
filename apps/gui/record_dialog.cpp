// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "record_dialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

namespace horcom {

namespace {

// the summer time codes of the original sommerzeit table, the single
// character goes into the AAF field
struct DstCode {
  const char* code;
  const char* label;
};

//RR 0 = Standardzeit
//RR 1 = Einfache Sommerzeit
//RR 2 = Doppelte Sommerzeit
//RR w = Kriegszeiten  = '1'
//RR h = Halbe    Sommerzeit
//RR m = Bestimmter Zeitmeridian
//RR L = Ortszeit
//RR * = Ortszeit oder ohne Belang
constexpr DstCode kDstCodes[] = {
    {"0", "0 = Standardzeit"},
    {"1", "1 = Einfache Sommerzeit"},
    {"2", "2 = Doppelte Sommerzeit"},
    {"w", "w = Kriegszeiten = '1'"},
    {"h", "h = Halbe Sommerzeit"},
    {"m", "m = Bestimmter Zeitmeridian"},
    {"L", "L = Ortszeit"},
    {"*", "* = Ortszeit oder ohne Belang"},
};

}  // namespace

RecordDialog::RecordDialog(AafRecord record, const std::vector<GermanCountry>& countries, QWidget* parent)
    : QDialog(parent), base_(std::move(record)) {
  setWindowTitle(tr("Datensatz"));
  auto* v = new QVBoxLayout(this);
  auto* form = new QFormLayout();
  surname_ = new QLineEdit(QString::fromStdString(base_.surname), this);
  given_ = new QLineEdit(QString::fromStdString(base_.given), this);
  sex_ = new QComboBox(this);
  sex_->addItems({"", "m", "w"});
  sex_->setCurrentText(QString::fromStdString(base_.sex));
  place_ = new QLineEdit(QString::fromStdString(base_.place), this);
  country_ = new QComboBox(this);
  country_->addItem("", QString());
  for (const GermanCountry& c : countries) {
    country_->addItem(QString("%1   %2").arg(QString::fromStdString(c.abbrev), -4).arg(QString::fromStdString(c.name)),
                      QString::fromStdString(c.abbrev));
  }
  const int have = country_->findData(QString::fromStdString(base_.country));
  if (have >= 0) {
    country_->setCurrentIndex(have);
  }
  dst_ = new QComboBox(this);
  for (const DstCode& c : kDstCodes) {
    dst_->addItem(c.label, QString(c.code));
  }
  const int dst_have = dst_->findData(QString::fromStdString(base_.dst.empty() ? "*" : base_.dst));
  dst_->setCurrentIndex(dst_have >= 0 ? dst_have : dst_->count() - 1);
  comment_ = new QLineEdit(QString::fromStdString(base_.comment), this);
  form->addRow(tr("Name"), surname_);
  form->addRow(tr("Vorname"), given_);
  form->addRow(tr("Geschlecht"), sex_);
  form->addRow(tr("Ort"), place_);
  form->addRow(tr("Land"), country_);
  form->addRow(tr("Sommerzeit"), dst_);
  form->addRow(tr("Bemerkung"), comment_);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(buttons);
  resize(460, 0);
  surname_->setFocus();
}

AafRecord RecordDialog::record() const {
  AafRecord r = base_;
  r.surname = surname_->text().trimmed().toStdString();
  r.given = given_->text().trimmed().toStdString();
  r.sex = sex_->currentText().toStdString();
  r.place = place_->text().trimmed().toStdString();
  r.country = country_->currentData().toString().toStdString();
  r.dst = dst_->currentData().toString().toStdString();
  r.comment = comment_->text().trimmed().toStdString();
  return r;
}

}  // namespace horcom
