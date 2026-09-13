// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "kommen_dialog.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace horcom {

KommenDialog::KommenDialog(const std::filesystem::path& dir, QWidget* parent) : QDialog(parent) {
  //RR TEXT-DATEI LESEN
  setWindowTitle(tr("Text-Datei lesen"));
  entries_ = kommen_entries(dir);
  auto* v = new QVBoxLayout(this);
  auto* split = new QHBoxLayout();
  list_ = new QListWidget(this);
  list_->setFixedWidth(240);
  for (const KommenEntry& e : entries_) {
    list_->addItem(QString::fromUtf8(e.title.c_str()));
  }
  text_ = new QPlainTextEdit(this);
  text_->setReadOnly(true);
  //RR FIXEDSYS, his reading box was fixed width
  QFont mono("Cascadia Mono");
  mono.setStyleHint(QFont::Monospace);
  text_->setFont(mono);
  text_->setLineWrapMode(QPlainTextEdit::NoWrap);
  split->addWidget(list_);
  split->addWidget(text_, 1);
  v->addLayout(split, 1);
  auto* bottom = new QHBoxLayout();
  find_ = new QLineEdit(this);
  find_->setPlaceholderText(tr("Suchbegriff"));
  //RR SUCHEN
  auto* find_button = new QPushButton(tr("Suchen"), this);
  bottom->addWidget(find_, 1);
  bottom->addWidget(find_button);
  v->addLayout(bottom);

  if (entries_.empty()) {
    text_->setPlainText(
        tr("Keine Kommentar-Texte gefunden.\n\n"
           "Die Original-Texte von Robert Rettig gehören nicht zum\n"
           "Repository. Wer sie besitzt, legt die Dateien des Ordners\n"
           "KOMMEN7P (KOMM1.TXT bis KOMM9.TXT, KOMMSTAT.TXT,\n"
           "AAF_KOMM.TXT, dazu AENDLIST.TXT, HINWEIS5.TXT und\n"
           "KURZANL5.TXT) in den Ordner:\n\n    %1")
            .arg(QString::fromStdWString((dir).wstring())));
  }
  connect(list_, &QListWidget::currentRowChanged, this, &KommenDialog::show_entry);
  connect(find_button, &QPushButton::clicked, this, &KommenDialog::search);
  connect(find_, &QLineEdit::returnPressed, this, &KommenDialog::search);
  if (!entries_.empty()) {
    list_->setCurrentRow(0);
  }
  resize(920, 640);
}

void KommenDialog::show_entry(int row) {
  if (row < 0 || row >= static_cast<int>(entries_.size())) {
    return;
  }
  const auto text = read_kommen(entries_[static_cast<std::size_t>(row)].path);
  text_->setPlainText(text ? QString::fromUtf8(text->c_str()) : tr("Datei fehlt !"));
}

void KommenDialog::search() {
  const QString needle = find_->text();
  if (needle.isEmpty()) {
    return;
  }
  if (!text_->find(needle)) {
    // wrap around once like a reading loop
    text_->moveCursor(QTextCursor::Start);
    text_->find(needle);
  }
}

}  // namespace horcom
