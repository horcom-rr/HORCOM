// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "record_list_dialog.hpp"

#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QTextDocument>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the search buttons of ausw_datei only appeared on long lists
constexpr int kSearchThreshold = 58;

// one coordinate like grmise(gd,1), degrees in three, tenth minutes in
// five columns
QString coordinate(double value, char pos, char neg) {
  const char hemi = value < 0.0 ? neg : pos;
  const double g = std::abs(value) + kEps;
  const int deg = static_cast<int>(g);
  const double min = 60.0 * (g - deg);
  return QString::asprintf("%3d\xC2\xB0%5.1f' %c", deg, min, hemi);
}

}  // namespace

// ported from the lt$ line of a2111
QString record_list_line(const AafRecord& r) {
  QString name = QString::fromStdString(r.surname);
  const QString given = QString::fromStdString(r.given).trimmed();
  if (!given.isEmpty()) {
    name = (name.trimmed() + " " + given);
  }
  name = name.toUpper();
  name = name.leftJustified(25, ' ', true);
  const double mii = r.minute + r.second / 60.0;
  QString date;
  if (r.year > 0) {
    date = QString::asprintf("%2d.%2d.%5d     ", r.day, r.month, r.year);
  } else {
    //RR vC
    date = QString::asprintf("%2d.%2d.%5d vC  ", r.day, r.month, 1 + std::abs(r.year));
  }
  return name + " " + date + QString::asprintf("%2d H %4.1f MIN ", r.hour, mii) +
         coordinate(r.longitude(), 'E', 'W') + "  " + coordinate(r.latitude(), 'N', 'S') + "    " +
         QString::fromStdString(r.place).toUpper();
}

RecordListDialog::RecordListDialog(const std::vector<AafRecord>& records, const std::vector<std::size_t>& order,
                                   const QString& file_label, int max_pick, Mode mode, QWidget* parent)
    : QDialog(parent), max_pick_(max_pick), mode_(mode) {
  switch (mode_) {
    case Mode::kFetch:
      //RR Bis zu 5 Datensätze wählen,dann 'WAHLENDE'
      setWindowTitle(tr("Bis zu %1 Datensätze wählen, dann 'WAHL - ENDE' !  Datei: %2").arg(max_pick_).arg(file_label));
      break;
    case Mode::kDelete:
      //RR Bis zu 10 zu LÖSCHENDE DATENSÄTZE markieren !
      setWindowTitle(tr("Bis zu %1 zu LÖSCHENDE DATENSÄTZE markieren !  Datei: %2").arg(max_pick_).arg(file_label));
      break;
    case Mode::kSingle:
      setWindowTitle(tr("Datensatz wählen !  Datei: %1").arg(file_label));
      break;
  }
  auto* v = new QVBoxLayout(this);
  //RR the column header of his chooser listbox
  auto* head = new QLabel(QStringLiteral("  NAME                      DATUM          ZEIT (UT)     LÄNGE"
                                         "         BREITE        ORTS-NAME"),
                          this);
  v->addWidget(head);
  list_ = new QListWidget(this);
  list_->setSelectionMode(mode_ == Mode::kSingle ? QAbstractItemView::SingleSelection
                                                 : QAbstractItemView::MultiSelection);
  for (const std::size_t i : order) {
    auto* item = new QListWidgetItem(" " + record_list_line(records[i]), list_);
    item->setData(Qt::UserRole, static_cast<qulonglong>(i));
  }
  if (list_->count() > 0) {
    if (mode_ == Mode::kSingle) {
      list_->setCurrentRow(0);
      picked_ = {list_->item(0)->data(Qt::UserRole).toULongLong()};
    } else {
      // in multi-selection mode setCurrentRow would also select row 0,
      // and the first click on it would then toggle the selection off,
      // making row 0 look inert. Keep the first row as the current one
      // for keyboard navigation but leave every row unselected on entry
      list_->setCurrentItem(list_->item(0), QItemSelectionModel::NoUpdate);
    }
  }
  v->addWidget(list_, 1);
  auto* row = new QHBoxLayout();
  auto* done = new QPushButton(tr("WAHL - ENDE"), this);
  done->setDefault(true);
  auto* exit = new QPushButton(tr("EXIT"), this);
  //RR &DRUCKEN, the list goes to the printer as his datei_pr did
  auto* print = new QPushButton(tr("DRUCKEN"), this);
  row->addWidget(exit);
  row->addWidget(done);
  row->addWidget(print);
  connect(print, &QPushButton::clicked, this, [this, head]() {
    QPrinter printer;
    QPrintDialog ask(&printer, this);
    if (ask.exec() != QDialog::Accepted) {
      return;
    }
    QString body = head->text() + "\n\n";
    for (int i = 0; i < list_->count(); ++i) {
      body += list_->item(i)->text() + "\n";
    }
    QTextDocument doc;
    QFont mono("Courier New");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(8);
    doc.setDefaultFont(mono);
    doc.setPlainText(windowTitle() + "\n\n" + body);
    doc.print(&printer);
  });
  if (static_cast<int>(records.size()) > kSearchThreshold) {
    // one field covers his Wortanfang and Allgemein search buttons,
    // typing jumps to the word start, Enter scans on through the rows
    search_ = new QLineEdit(this);
    search_->setPlaceholderText(tr("Suchen"));
    search_->setClearButtonEnabled(true);
    connect(search_, &QLineEdit::textEdited, this, &RecordListDialog::search);
    connect(search_, &QLineEdit::returnPressed, this, [this]() {
      const QString needle = search_->text().trimmed().toUpper();
      if (needle.isEmpty()) {
        return;
      }
      //RR Allgemein suchen
      const int start = list_->currentRow() + 1;
      for (int off = 0; off < list_->count(); ++off) {
        const int i = (start + off) % list_->count();
        if (list_->item(i)->text().contains(needle)) {
          list_->setCurrentRow(i);
          return;
        }
      }
    });
    row->addSpacing(16);
    row->addWidget(search_, 1);
  } else {
    row->addStretch(1);
  }
  v->addLayout(row);
  connect(list_, &QListWidget::itemClicked, this, &RecordListDialog::toggle);
  connect(list_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
    const std::size_t i = item->data(Qt::UserRole).toULongLong();
    if (std::find(picked_.begin(), picked_.end(), i) == picked_.end()) {
      picked_.push_back(i);
    }
    accept();
  });
  connect(done, &QPushButton::clicked, this, [this]() {
    if (mode_ == Mode::kSingle && list_->currentItem() != nullptr) {
      picked_ = {list_->currentItem()->data(Qt::UserRole).toULongLong()};
    }
    if (picked_.empty()) {
      // the original stayed in the box until something was chosen
      return;
    }
    accept();
  });
  connect(exit, &QPushButton::clicked, this, &QDialog::reject);
  resize(1020, 560);
}

void RecordListDialog::toggle(QListWidgetItem* item) {
  const std::size_t i = item->data(Qt::UserRole).toULongLong();
  if (mode_ == Mode::kSingle) {
    picked_ = {i};
    return;
  }
  const auto have = std::find(picked_.begin(), picked_.end(), i);
  if (item->isSelected() && have == picked_.end()) {
    if (static_cast<int>(picked_.size()) >= max_pick_) {
      item->setSelected(false);
      return;
    }
    picked_.push_back(i);
  } else if (!item->isSelected() && have != picked_.end()) {
    picked_.erase(have);
  }
}

// the Wortanfang jump of his listbox typing
void RecordListDialog::search(const QString& text) {
  const QString needle = text.trimmed().toUpper();
  if (needle.isEmpty()) {
    return;
  }
  for (int i = 0; i < list_->count(); ++i) {
    // rows carry one lead blank before the name
    if (list_->item(i)->text().mid(1).startsWith(needle)) {
      list_->setCurrentRow(i);
      return;
    }
  }
}

}  // namespace horcom
