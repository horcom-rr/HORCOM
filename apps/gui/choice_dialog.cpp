// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "choice_dialog.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace horcom {

ChoiceDialog::ChoiceDialog(const QString& title, const QStringList& info, const QStringList& buttons,
                           int default_index, QWidget* parent)
    : QDialog(parent) {
  setWindowTitle(title);
  auto* v = new QVBoxLayout(this);
  v->setContentsMargins(18, 14, 18, 14);
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
  setMinimumWidth(460);
}

int ChoiceDialog::ask(QWidget* parent, const QString& title, const QStringList& info,
                      const QStringList& buttons, int default_index) {
  ChoiceDialog d(title, info, buttons, default_index, parent);
  d.exec();
  return d.choice();
}

}  // namespace horcom
