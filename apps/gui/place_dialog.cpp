// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "place_dialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <string>

#include "horcom/data/record_order.hpp"

namespace horcom {

namespace {

// more rows than this stay behind the filter so the table remains snappy
constexpr int kMaxRows = 400;

// the zone picker files keep the zone in the last five name characters,
// the display drops them
QString display_name(const PlaceRecord& r, bool has_zone) {
  QString n = QString::fromStdString(r.name);
  if (has_zone) {
    n.chop(qMin(qsizetype(5), n.size()));
  }
  return n.trimmed();
}

}  // namespace

PlaceDialog::PlaceDialog(std::filesystem::path places_dir, const std::filesystem::path& nima_table,
                         QWidget* parent)
    : QDialog(parent), dir_(std::move(places_dir)) {
  if (const auto n = load_nima_countries(nima_table)) {
    nima_ = *n;
  }
  setWindowTitle(tr("Ort suchen"));
  auto* v = new QVBoxLayout(this);
  auto* top = new QHBoxLayout();
  files_ = new QComboBox(this);
  browse_ = new QPushButton(tr("Andere Datei…"), this);
  top->addWidget(files_, 1);
  top->addWidget(browse_);
  filter_ = new QLineEdit(this);
  filter_->setPlaceholderText(tr("Suchen…"));
  filter_->setClearButtonEnabled(true);
  table_ = new QTableWidget(0, 4, this);
  table_->setHorizontalHeaderLabels({tr("Ort"), tr("Länge"), tr("Breite"), tr("Zone (h östl.)")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->verticalHeader()->setDefaultSectionSize(20);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  count_ = new QLabel(this);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  v->addLayout(top);
  v->addWidget(filter_);
  v->addWidget(table_, 1);
  v->addWidget(count_);
  v->addWidget(buttons);

  connect(files_, &QComboBox::currentIndexChanged, this, [this](int) { load_current_file(); });
  connect(browse_, &QPushButton::clicked, this, &PlaceDialog::browse);
  connect(filter_, &QLineEdit::textChanged, this, [this](const QString&) { refresh(); });
  connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
    // his second click inside the double click time ends the marking
    if (max_pick_ > 0) {
      accept_marks();
    } else {
      accept_row(row);
    }
  });
  connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
    if (max_pick_ > 0) {
      accept_marks();
      return;
    }
    accept_row(table_->currentRow());
  });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  resize(580, 540);

  scan_directory();
  filter_->setFocus();
}

void PlaceDialog::scan_directory() {
  const QDir dir(QString::fromStdWString(dir_.wstring()));
  const QStringList names = dir.entryList({"*.int", "*.INT"}, QDir::Files, QDir::Name);
  const QSignalBlocker block(files_);
  files_->clear();
  for (const QString& n : names) {
    // the big files spell CC_A_K, the NIMA table names the country
    QString label = n;
    if (n.size() >= 6 && n[2] == '_' && n[4] == '_') {
      const std::string country = nima_country_name(nima_, n.left(2).toStdString());
      if (!country.empty()) {
        label = QString("%1   %2").arg(n, QString::fromStdString(country));
      }
    }
    files_->addItem(label, dir.absoluteFilePath(n));
  }
  load_current_file();
}

void PlaceDialog::load_current_file() {
  records_.clear();
  file_index_.clear();
  marks_.clear();
  const QString path = files_->currentData().toString();
  if (!path.isEmpty()) {
    if (auto r = read_place_file(std::filesystem::path(path.toStdWString()))) {
      // the original sorts the loaded file with QSORT before the listbox
      std::vector<std::size_t> order(r->size());
      for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = i;
      }
      // n$ = UPPER$(goo$) sorted WITH vg|, the umlauts among their vowels
      std::vector<std::string> key(r->size());
      for (std::size_t i = 0; i < key.size(); ++i) {
        key[i] = collation_key((*r)[i].name);
      }
      std::stable_sort(order.begin(), order.end(), [&key](std::size_t a, std::size_t b) { return key[a] < key[b]; });
      for (const std::size_t i : order) {
        records_.push_back((*r)[i]);
        file_index_.push_back(i);
      }
    }
  }
  refresh();
}

void PlaceDialog::lock_file(const std::filesystem::path& file) {
  const QString path = QString::fromStdWString(file.wstring());
  const QSignalBlocker block(files_);
  int at = -1;
  for (int i = 0; i < files_->count(); ++i) {
    if (QFileInfo(files_->itemData(i).toString()) == QFileInfo(path)) {
      at = i;
    }
  }
  if (at < 0) {
    files_->addItem(QFileInfo(path).fileName(), path);
    at = files_->count() - 1;
  }
  files_->setCurrentIndex(at);
  files_->setEnabled(false);
  browse_->setEnabled(false);
  load_current_file();
}

// ported from ausw_datei with loe$, the marking list of LÖSCHEN
void PlaceDialog::set_delete_mode(int max_pick) {
  max_pick_ = max_pick;
  //RR Bis zu 10 zu LÖSCHENDE DATENSÄTZE markieren !
  setWindowTitle(tr("Bis zu %1 zu LÖSCHENDE DATENSÄTZE markieren !  Datei: %2")
                     .arg(max_pick)
                     .arg(files_->currentText()));
  table_->setSelectionMode(QAbstractItemView::MultiSelection);
  table_->clearSelection();
  marks_.clear();
  // his dsr&(1..10) held ten marks, a further click is refused. The marks
  // belong to the places, a new filter shows them again
  connect(table_->selectionModel(), &QItemSelectionModel::selectionChanged, this,
          [this](const QItemSelection& added, const QItemSelection& removed) {
            if (refreshing_) {
              return;
            }
            for (const QModelIndex& ix : removed.indexes()) {
              if (ix.column() == 0 && table_->item(ix.row(), 0) != nullptr) {
                marks_.erase(table_->item(ix.row(), 0)->data(Qt::UserRole).toULongLong());
              }
            }
            for (const QModelIndex& ix : added.indexes()) {
              if (ix.column() != 0 || table_->item(ix.row(), 0) == nullptr) {
                continue;
              }
              const std::size_t idx = table_->item(ix.row(), 0)->data(Qt::UserRole).toULongLong();
              if (marks_.count(idx) == 0 && static_cast<int>(marks_.size()) >= max_pick_) {
                const QSignalBlocker b(table_->selectionModel());
                table_->selectionModel()->select(ix, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
                continue;
              }
              marks_.insert(idx);
            }
          });
  if (auto* box = findChild<QDialogButtonBox*>()) {
    box->button(QDialogButtonBox::Ok)->setText(tr("WAHL - ENDE"));
  }
}

void PlaceDialog::browse() {
  const QString path = QFileDialog::getOpenFileName(this, tr("Ortsdatei öffnen"), QString(),
                                                    tr("Ortsdateien (*.int *.INT)"));
  if (path.isEmpty()) {
    return;
  }
  const QSignalBlocker block(files_);
  files_->addItem(QFileInfo(path).fileName(), path);
  files_->setCurrentIndex(files_->count() - 1);
  load_current_file();
}

void PlaceDialog::refresh() {
  const QString needle = filter_->text().trimmed();
  refreshing_ = true;
  table_->setRowCount(0);
  int shown = 0;
  int hits = 0;
  for (std::size_t i = 0; i < records_.size(); ++i) {
    const PlaceRecord& r = records_[i];
    const auto zone = r.zone_to_ut();
    const QString name = display_name(r, zone.has_value());
    if (!needle.isEmpty() && !name.contains(needle, Qt::CaseInsensitive)) {
      continue;
    }
    ++hits;
    if (shown >= kMaxRows) {
      continue;
    }
    const int row = table_->rowCount();
    table_->insertRow(row);
    auto* item = new QTableWidgetItem(name);
    item->setData(Qt::UserRole, static_cast<qulonglong>(i));
    table_->setItem(row, 0, item);
    table_->setItem(row, 1, new QTableWidgetItem(QString::asprintf("%9.4f", r.lon)));
    table_->setItem(row, 2, new QTableWidgetItem(QString::asprintf("%8.4f", r.lat)));
    // the file stores the step from zone time to UT, the column shows
    // hours east like the input panel
    table_->setItem(row, 3, new QTableWidgetItem(zone ? QString::asprintf("%+g", -*zone) : QString()));
    if (max_pick_ > 0 && marks_.count(i) > 0) {
      table_->selectionModel()->select(table_->model()->index(row, 0),
                                       QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
    ++shown;
  }
  refreshing_ = false;
  if (hits > shown) {
    count_->setText(tr("%1 Treffer, die ersten %2 angezeigt").arg(hits).arg(shown));
  } else {
    count_->setText(tr("%1 Treffer").arg(hits));
  }
  if (table_->rowCount() > 0 && max_pick_ == 0) {
    table_->selectRow(0);
  }
  table_->resizeColumnToContents(0);
}

QString PlaceDialog::chosen_name() const {
  return display_name(chosen_, chosen_.zone_to_ut().has_value());
}

// WAHL - ENDE of the LÖSCHEN list, the marks in file order
void PlaceDialog::accept_marks() {
  marked_.clear();
  for (const std::size_t idx : marks_) {
    marked_.push_back(file_index_[idx]);
  }
  std::sort(marked_.begin(), marked_.end());
  accept();
}

void PlaceDialog::accept_row(int row) {
  if (row < 0 || row >= table_->rowCount()) {
    return;
  }
  const auto idx = table_->item(row, 0)->data(Qt::UserRole).toULongLong();
  chosen_ = records_[static_cast<std::size_t>(idx)];
  accept();
}

}  // namespace horcom
