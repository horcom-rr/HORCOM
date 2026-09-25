// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "choice_dialog.hpp"

#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace horcom {

namespace {

// the width of the box with short captions and its side margins
constexpr int kBoxWidth = 460;
constexpr int kSideMargin = 18;

}  // namespace

ChoiceDialog::ChoiceDialog(const QString& title, const QStringList& info, const QStringList& buttons,
                           int default_index, QWidget* parent)
    : QDialog(parent) {
  setWindowTitle(title);
  auto* v = new QVBoxLayout(this);
  v->setContentsMargins(kSideMargin, 14, kSideMargin, 14);
  v->setSpacing(10);
  for (const QString& line : info) {
    auto* label = new QLabel(line, this);
    label->setAlignment(Qt::AlignHCenter);
    v->addWidget(label);
  }
  if (!info.isEmpty()) {
    v->addSpacing(6);
  }
  for (int i = 0; i < buttons.size(); ++i) {
    auto* b = new QPushButton(buttons.at(i), this);
    b->setMinimumHeight(34);
    b->setDefault(i == default_index);
    b->setAutoDefault(true);
    connect(b, &QPushButton::clicked, this, [this, i]() {
      choice_ = i;
      accept();
    });
    v->addWidget(b);
  }
  // the box keeps its 460 and grows with a longer caption, the layout
  // minimum rules so no row is ever cut, however small the screen
  v->addStrut(kBoxWidth - 2 * kSideMargin);
  v->setSizeConstraint(QLayout::SetMinimumSize);
}

int ChoiceDialog::ask(QWidget* parent, const QString& title, const QStringList& info,
                      const QStringList& buttons, int default_index) {
  ChoiceDialog d(title, info, buttons, default_index, parent);
  return d.run({});
}

void ChoiceDialog::keyPressEvent(QKeyEvent* e) {
  // his ex& 33, 82 and 114, PgUp and R in both cases
  if (back_ && (e->key() == Qt::Key_R || e->key() == Qt::Key_PageUp)) {
    choice_ = kBack;
    reject();
    return;
  }
  QDialog::keyPressEvent(e);
}

int ChoiceDialog::run(const std::vector<int>& disabled) {
  const QList<QPushButton*> rows = findChildren<QPushButton*>(Qt::FindDirectChildrenOnly);
  for (const int i : disabled) {
    if (i >= 0 && i < rows.size()) {
      rows[i]->setEnabled(false);
    }
  }
  exec();
  return choice_;
}

int ChoiceDialog::ask_step(QWidget* parent, const QString& title, const QStringList& info, const QStringList& buttons,
                           int default_index, const std::vector<int>& disabled) {
  ChoiceDialog d(title, info, buttons, default_index, parent);
  d.back_ = true;
  return d.run(disabled);
}

int ChoiceDialog::ask_with_disabled(QWidget* parent, const QString& title, const QStringList& info,
                                    const QStringList& buttons, int default_index, const std::vector<int>& disabled) {
  ChoiceDialog d(title, info, buttons, default_index, parent);
  return d.run(disabled);
}

}  // namespace horcom
