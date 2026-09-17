// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "kommen_dialog.hpp"

#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace horcom {

KommenDialog::KommenDialog(const std::filesystem::path& dir, const QString& preselect, bool english,
                           QWidget* parent)
    : QDialog(parent), preselect_(preselect), english_(english) {
  //RR TEXT-DATEI LESEN
  setWindowTitle(tr("Text-Datei lesen"));
  auto* v = new QVBoxLayout(this);
  auto* split = new QHBoxLayout();
  list_ = new QListWidget(this);
  list_->setFixedWidth(240);
  text_ = new QTextBrowser(this);
  text_->setOpenExternalLinks(true);
  split->addWidget(list_);
  split->addWidget(text_, 1);
  v->addLayout(split, 1);
  // the note names the edition on show, his German original or the
  // English translation that preserves his wording
  auto* note = new QLabel(
      english_ ? tr("English editions of Robert Rettig's original texts, his wording preserved in translation.")
               : tr("Die Original-Texte von Robert Rettig, in seinem deutschen Wortlaut."),
      this);
  note->setWordWrap(true);
  v->addWidget(note);
  auto* bottom = new QHBoxLayout();
  find_ = new QLineEdit(this);
  find_->setPlaceholderText(tr("Suchbegriff"));
  //RR SUCHEN
  auto* find_button = new QPushButton(tr("Suchen"), this);
  bottom->addWidget(find_, 1);
  bottom->addWidget(find_button);
  // whoever keeps the original texts elsewhere points the reader there
  auto* pick = new QPushButton(tr("Ordner wählen…"), this);
  bottom->addWidget(pick);
  v->addLayout(bottom);

  connect(list_, &QListWidget::currentRowChanged, this, &KommenDialog::show_entry);
  connect(find_button, &QPushButton::clicked, this, &KommenDialog::search);
  connect(find_, &QLineEdit::returnPressed, this, &KommenDialog::search);
  connect(pick, &QPushButton::clicked, this, [this]() {
    const QString chosen = QFileDialog::getExistingDirectory(this, tr("KOMMEN7P-Ordner wählen"));
    if (!chosen.isEmpty()) {
      reload(std::filesystem::path(chosen.toStdWString()));
    }
  });
  reload(dir);
  resize(920, 640);
}

void KommenDialog::reload(const std::filesystem::path& dir) {
  entries_ = kommen_entries(dir, english_);
  list_->clear();
  // the list shows his menu titles, the English shell shows them in
  // English while the German menu titles stay his own words
  static const std::pair<const char*, const char*> kTitleEn[] = {
      {"Einführender Kommentar", "Introductory Commentary"},
      {"Erläuterung Ein-Ausgabe", "Input and Output"},
      {"Erläuterung Ephemeride", "Ephemeris"},
      {"Erläuterung Horoskope", "Chart Types"},
      {"Erläuterung Solar,Septar...", "Solar, Septar and more"},
      {"Erläuterung M.R.", "Münchner Rhythmenlehre"},
      {"Erläuterung Direktionen", "Directions"},
      {"Erläuterung Häuser", "Houses"},
      {"Erläuterung Diverses", "Miscellaneous"},
      {"Änderungsliste", "Change list"},
      {"Hinweise", "Notes"},
      {"Kurzanleitung", "Short Manual"},
      {"Erläuterung Statistik", "Statistics"},
      {"Erläuterung AAF-Ein-Ausgabe", "AAF Input and Output"},
  };
  for (const KommenEntry& e : entries_) {
    QString title = QString::fromUtf8(e.title.c_str());
    if (english_) {
      for (const auto& [de, en] : kTitleEn) {
        if (e.title == de) {
          title = QString::fromUtf8(en);
          break;
        }
      }
    }
    list_->addItem(title);
  }
  if (entries_.empty()) {
    text_->setPlainText(
        tr("Keine Kommentar-Texte gefunden.\n\n"
           "Die Original-Texte von Robert Rettig liegen normalerweise\n"
           "im Daten-Ordner des Programms. Die Dateien des Ordners\n"
           "KOMMEN7P (KOMM1.TXT bis KOMM9.TXT, KOMMSTAT.TXT,\n"
           "AAF_KOMM.TXT, dazu AENDLIST.TXT, HINWEIS5.TXT und\n"
           "KURZANL5.TXT) gehören in den Ordner:\n\n    %1\n\n"
           "Alternativ unten einen KOMMEN7P-Ordner direkt auswählen.")
            .arg(QString::fromStdWString(dir.wstring())));
  } else {
    int row = 0;
    if (!preselect_.isEmpty()) {
      // the ERLÄUTERUNG entries of his menus land on their own text
      for (std::size_t i = 0; i < entries_.size(); ++i) {
        const QString stem = QString::fromStdWString(entries_[i].path.stem().wstring());
        // the English edition wears an _en suffix, match either stem
        if (stem.compare(preselect_, Qt::CaseInsensitive) == 0 ||
            stem.compare(preselect_ + "_en", Qt::CaseInsensitive) == 0) {
          row = static_cast<int>(i);
          break;
        }
      }
    }
    list_->setCurrentRow(row);
  }
}

void KommenDialog::show_entry(int row) {
  if (row < 0 || row >= static_cast<int>(entries_.size())) {
    return;
  }
  const std::filesystem::path& path = entries_[static_cast<std::size_t>(row)].path;
  if (path.extension() == ".md") {
    // the shipped markdown edition of his text
    QFile file(QString::fromStdWString(path.wstring()));
    if (file.open(QIODevice::ReadOnly)) {
      text_->setFont(font());
      text_->setLineWrapMode(QTextEdit::WidgetWidth);
      text_->setMarkdown(QString::fromUtf8(file.readAll()));
      return;
    }
    text_->setPlainText(tr("Datei fehlt !"));
    return;
  }
  //RR FIXEDSYS, his reading box was fixed width
  QFont mono("Courier New");
  mono.setStyleHint(QFont::Monospace);
  mono.setBold(true);
  text_->setFont(mono);
  text_->setLineWrapMode(QTextEdit::NoWrap);
  const auto text = read_kommen(path);
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
