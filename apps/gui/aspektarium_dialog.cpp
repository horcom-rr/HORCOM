// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "aspektarium_dialog.hpp"

#include <QVBoxLayout>

#include "wheel_widget.hpp"

namespace horcom {

AspektariumDialog::AspektariumDialog(DisplayList sheet, QWidget* parent) : QDialog(parent) {
  setWindowTitle(tr("ASPEKTARIUM"));
  auto* v = new QVBoxLayout(this);
  v->setContentsMargins(0, 0, 0, 0);
  canvas_ = new WheelWidget(this);
  v->addWidget(canvas_);
  connect(canvas_, &WheelWidget::right_clicked, this, &AspektariumDialog::right_clicked);
  set_sheet(std::move(sheet));
  resize(1100, 800);
}

void AspektariumDialog::set_sheet(DisplayList sheet) {
  canvas_->set_plain_list(std::move(sheet));
}

}  // namespace horcom
