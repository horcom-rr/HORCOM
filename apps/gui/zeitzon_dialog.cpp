// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "zeitzon_dialog.hpp"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include "horcom/data/kommen.hpp"
#include "horcom/data/place_file.hpp"
#include "zone_dialog.hpp"

namespace horcom {

namespace {

// his lists show the twenty name bytes with the zone in the last five
void fill_places(QListWidget* list, const std::filesystem::path& file) {
  const auto places = read_place_file(file);
  if (!places) {
    return;
  }
  for (const PlaceRecord& p : *places) {
    auto* item = new QListWidgetItem(QString::fromStdString(p.name), list);
    if (const auto z = p.zone_to_ut()) {
      item->setData(Qt::UserRole, *z);
    }
  }
}

}  // namespace

// ported from zeitzon, the dialog AUSWAHL der ZEIT - ZONE
ZeitzonDialog::ZeitzonDialog(std::filesystem::path data_dir, QWidget* parent)
    : QDialog(parent), data_dir_(std::move(data_dir)) {
  //RR AUSWAHL der ZEIT - ZONE | Evtl. direkt unten NAHELIEGENDEN ORT anklicken oder in ZEIT-ZONEN-LISTE anklicken !
  setWindowTitle(tr("AUSWAHL der ZEIT - ZONE | Evtl. direkt unten NAHELIEGENDEN ORT anklicken oder in "
                    "ZEIT-ZONEN-LISTE anklicken !"));
  auto* grid = new QGridLayout(this);
  grid->setHorizontalSpacing(14);

  auto* left = new QVBoxLayout();
  //RR ZEIT-ZONEN-DIFF. (h): Z.B. -1 h mit MEZ
  left->addWidget(new QLabel(tr("ZEIT-ZONEN-DIFF. (h): Z.B. -1 h mit MEZ"), this));
  field_ = new QLineEdit(this);
  field_->setMaxLength(8);
  field_->setMaximumWidth(90);
  left->addWidget(field_);
  //RR EINFACHE SOMMERZEIT = DSZ, DOPPELTE SOMMERZEIT = DDSZ
  dsz_ = new QCheckBox(tr("EINFACHE SOMMERZEIT = DSZ"), this);
  ddsz_ = new QCheckBox(tr("DOPPELTE SOMMERZEIT = DDSZ"), this);
  left->addWidget(dsz_);
  left->addWidget(ddsz_);
  left->addSpacing(10);
  auto* texts = new QPushButton(tr("ZEIT-BESTIMMUNGEN für EUROPA lesen"), this);
  auto* catalogue = new QPushButton(tr("ZEIT-ZONE in LISTE (WELT) anklicken"), this);
  left->addWidget(texts);
  left->addWidget(catalogue);
  left->addStretch(1);
  auto* ok = new QPushButton(tr("OK"), this);
  ok->setDefault(true);
  left->addWidget(ok, 0, Qt::AlignLeft);
  grid->addLayout(left, 0, 0, 3, 1);

  //RR Evtl. unten über NAHELIEGENDEN ORT die ZEIT-ZONE anklicken !
  grid->addWidget(new QLabel(tr("Evtl. unten über NAHELIEGENDEN ORT die ZEIT-ZONE anklicken !"), this), 0, 1, 1, 2);
  grid->addWidget(new QLabel(tr("EUROPA :"), this), 1, 1);
  grid->addWidget(new QLabel(tr("WELT   :"), this), 1, 2);
  europa_ = new QListWidget(this);
  welt_ = new QListWidget(this);
  QFont mono(QStringLiteral("Courier New"));
  mono.setStyleHint(QFont::Monospace);
  for (QListWidget* list : {europa_, welt_}) {
    list->setFont(mono);
    list->setMinimumWidth(200);
    list->setMinimumHeight(260);
  }
  fill_places(europa_, data_dir_ / "places" / "europa.int");
  fill_places(welt_, data_dir_ / "places" / "welt.int");
  grid->addWidget(europa_, 2, 1);
  grid->addWidget(welt_, 2, 2);

  // a place click takes its zone and clears the summer boxes like his
  // SETCHECK 104 and 105 to zero
  for (QListWidget* list : {europa_, welt_}) {
    connect(list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
      const QVariant z = item->data(Qt::UserRole);
      if (z.isValid()) {
        set_base(z.toDouble(), true);
      }
    });
  }
  // the summer boxes exclude each other and ride on the chosen zone. His
  // boxes wrote -2 and -3 flat, right only for MEZ, the port adds one or
  // two hours to whatever zone the field holds
  connect(dsz_, &QCheckBox::toggled, this, [this](bool on) {
    if (on) {
      const QSignalBlocker b(ddsz_);
      ddsz_->setChecked(false);
    }
    show_field();
  });
  connect(ddsz_, &QCheckBox::toggled, this, [this](bool on) {
    if (on) {
      const QSignalBlocker b(dsz_);
      dsz_->setChecked(false);
    }
    show_field();
  });
  // a typed difference is the whole value, summer included
  connect(field_, &QLineEdit::textEdited, this, [this](const QString& t) {
    bool ok_value = false;
    const double v = t.trimmed().toDouble(&ok_value);
    if (ok_value) {
      base_ = v;
      const QSignalBlocker b1(dsz_);
      const QSignalBlocker b2(ddsz_);
      dsz_->setChecked(false);
      ddsz_->setChecked(false);
    }
  });
  connect(texts, &QPushButton::clicked, this, [this]() { read_time_determinations(this, data_dir_); });
  // his zeitzon_nam_horc, a row without a number gives zero
  connect(catalogue, &QPushButton::clicked, this, [this]() {
    ZoneDialog dialog(data_dir_ / "zonnamen.int", this);
    if (!dialog.loaded()) {
      //RR ZEITZONEN-Datei fehlt !
      QMessageBox::warning(this, "HORCOM", tr("ZEITZONEN-Datei fehlt !"));
      return;
    }
    if (dialog.exec() == QDialog::Accepted) {
      set_base(dialog.chosen().to_ut_hours.value_or(0.0), false);
    }
  });
  connect(ok, &QPushButton::clicked, this, &QDialog::accept);
  set_base(-1.0, true);
  resize(900, 0);
}

void ZeitzonDialog::set_base(double zzd, bool clear_summer) {
  base_ = zzd;
  if (clear_summer) {
    const QSignalBlocker b1(dsz_);
    const QSignalBlocker b2(ddsz_);
    dsz_->setChecked(false);
    ddsz_->setChecked(false);
  }
  show_field();
}

void ZeitzonDialog::show_field() {
  field_->setText(QString::number(base_ - summer()));
}

double ZeitzonDialog::zzd() const {
  bool ok = false;
  const double v = field_->text().trimmed().toDouble(&ok);
  return ok ? v : base_ - summer();
}

int ZeitzonDialog::summer() const {
  if (ddsz_->isChecked()) {
    return 2;
  }
  return dsz_->isChecked() ? 1 : 0;
}

// ported from the ZEITBEST branch of eingabe and zeitzon, FILESELECT
// ZEITBEST\*.TXT into lese_text
void read_time_determinations(QWidget* parent, const std::filesystem::path& data_dir) {
  const QString dir = QString::fromStdWString((data_dir / "zeitbest").wstring());
  const QString path = QFileDialog::getOpenFileName(parent, QObject::tr("ZEITBESTIMMUNGEN LESEN"), dir,
                                                    QObject::tr("Zeitbestimmungen (*.TXT *.txt)"));
  if (path.isEmpty()) {
    return;
  }
  const auto text = read_zeitbest(std::filesystem::path(path.toStdWString()));
  if (!text) {
    QMessageBox::warning(parent, "HORCOM", QObject::tr("Die Datei ließ sich nicht lesen."));
    return;
  }
  QDialog box(parent);
  // TEXT-DATEI LESEN | erle$, his caption with the file name
  box.setWindowTitle(QObject::tr("TEXT-DATEI LESEN | %1").arg(QFileInfo(path).fileName()));
  auto* v = new QVBoxLayout(&box);
  auto* view = new QPlainTextEdit(QString::fromStdString(*text), &box);
  view->setReadOnly(true);
  view->setLineWrapMode(QPlainTextEdit::NoWrap);
  QFont mono(QStringLiteral("Courier New"));
  mono.setStyleHint(QFont::Monospace);
  view->setFont(mono);
  v->addWidget(view);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &box);
  QObject::connect(buttons, &QDialogButtonBox::accepted, &box, &QDialog::accept);
  v->addWidget(buttons);
  box.resize(760, 560);
  box.exec();
}

}  // namespace horcom
