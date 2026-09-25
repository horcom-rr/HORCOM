// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "record_list_dialog.hpp"

#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPrinter>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

#include "choice_dialog.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/collection.hpp"
#include "print_pages.hpp"

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
  const QString name = QString::fromStdString(record_name(r)).toUpper().leftJustified(kDatNameLength, ' ', true);
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
    : QDialog(parent), records_(records), max_pick_(max_pick), mode_(mode) {
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
  // the column header of his chooser listbox
  auto* head = new QLabel(tr("  NAME                      DATUM          ZEIT (UT)     LÄNGE"
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
  buttons_ = row;
  auto* done = new QPushButton(tr("WAHL - ENDE"), this);
  done_ = done;
  done->setDefault(true);
  auto* exit = new QPushButton(tr("EXIT"), this);
  // &DRUCKEN, the list goes to the printer as his datei_pr did
  auto* print = new QPushButton(tr("DRUCKEN"), this);
  print_ = print;
  row->addWidget(exit);
  row->addWidget(done);
  row->addWidget(print);
  connect(print, &QPushButton::clicked, this, [this]() {
    QPrinter printer(QPrinter::HighResolution);
    // list_druck! = -1, druck& = @druck_einr_anz
    if (!prepare_printer(this, printer, PrintPage::kList, true)) {
      return;
    }
    // datei_pr, the rows in their current order, then the list closes
    QStringList rows;
    for (int i = 0; i < list_->count(); ++i) {
      rows << list_->item(i)->text();
    }
    if (print_text_rows(printer, rows)) {
      reject();
    }
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
  connect(list_, &QListWidget::itemDoubleClicked, this, &RecordListDialog::double_click);
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
  connect(exit, &QPushButton::clicked, this, &RecordListDialog::exit_list);
  resize(1020, 560);
}

// his second click on a marked row inside the double click time, GOTO
// obda2 ends the choice with the marks made. A click past the limit
// never adds a record, and the browser takes none
void RecordListDialog::double_click(QListWidgetItem* item) {
  if (browsing_) {
    return;
  }
  const std::size_t i = item->data(Qt::UserRole).toULongLong();
  if (mode_ == Mode::kSingle) {
    picked_ = {i};
  } else if (std::find(picked_.begin(), picked_.end(), i) == picked_.end()) {
    if (static_cast<int>(picked_.size()) < max_pick_) {
      picked_.push_back(i);
      item->setSelected(true);
    } else {
      item->setSelected(false);
    }
  }
  if (!picked_.empty()) {
    accept();
  }
}

// CASE 102, EXIT leaves the list, the browser first asks whether to go
// back to the choice like his Zurück in EINGABE ?
void RecordListDialog::exit_list() {
  if (!browsing_) {
    reject();
    return;
  }
  //RR Zurück in EINGABE ? ( Mit Zugang zu allen Programmen )
  const int al = ChoiceDialog::ask(this, "HORCOM", {tr("Zurück in EINGABE ?"), tr("( Mit Zugang zu allen Programmen )")},
                                   {tr("NEIN = EXIT"), tr(" JA ")}, 0);
  if (al != 1) {
    reject();
    return;
  }
  // CLR muster!, GOTO ast, the list takes records again
  browsing_ = false;
  marked_row_ = -1;
  done_->setEnabled(true);
  print_->setEnabled(true);
  list_->clearSelection();
  list_->setSelectionMode(mode_ == Mode::kSingle ? QAbstractItemView::SingleSelection
                                                 : QAbstractItemView::MultiSelection);
}

// CASE 27 of his key loop, ESC asks before it leaves the EIN-AUSGABE
void RecordListDialog::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    //RR EIN-AUSGABE verlassen ?
    const int al = ChoiceDialog::ask(this, "HORCOM", {tr("EIN-AUSGABE verlassen ?")}, {tr("NEIN"), tr("JA")}, 0);
    if (al == 1) {
      reject();
    }
    return;
  }
  QDialog::keyPressEvent(event);
}

void RecordListDialog::toggle(QListWidgetItem* item) {
  marked_row_ = list_->row(item);
  // IF NOT(muster! OR ...), the browser only marks the row
  if (browsing_) {
    return;
  }
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

void RecordListDialog::set_preview(std::function<void(const AafRecord&)> preview) {
  preview_ = std::move(preview);
  //RR NUR HOROSKOP ZEIGEN
  auto* show = new QPushButton(tr("NUR HOROSKOP ZEIGEN"), this);
  buttons_->addWidget(show);
  connect(show, &QPushButton::clicked, this, &RecordListDialog::show_marked);
}

// ported from CASE 107 of ausw_datei
void RecordListDialog::show_marked() {
  if (marked_row_ < 0 || marked_row_ >= list_->count()) {
    //RR Datensatz MARKIEREN mit EINFACH-KLICK !
    QMessageBox::information(this, "HORCOM", tr("Datensatz MARKIEREN mit EINFACH-KLICK !"));
    return;
  }
  if (!browsing_) {
    //RR Hiermit können Sie ALLE HOROSKOPE einer Datei durchsehen|OHNE diese in die EINGABE zu übernehmen !
    const int al = ChoiceDialog::ask(this, "HORCOM",
                                     {tr("Hiermit können Sie ALLE HOROSKOPE einer Datei durchsehen"),
                                      tr("OHNE diese in die EINGABE zu übernehmen !")},
                                     {tr(" OK = WEITER "), tr("IN EINGABE GEHEN")});
    if (al != 0) {
      marked_row_ = -1;
      return;
    }
    // his muster!, WAHL - ENDE and DRUCKEN go grey, the list is a browser
    browsing_ = true;
    done_->setEnabled(false);
    print_->setEnabled(false);
    picked_.clear();
    list_->clearSelection();
    list_->setSelectionMode(QAbstractItemView::SingleSelection);
    list_->setCurrentRow(marked_row_);
  }
  const std::size_t i = list_->item(marked_row_)->data(Qt::UserRole).toULongLong();
  if (preview_ && i < records_.size()) {
    preview_(records_[i]);
  }
}

}  // namespace horcom
