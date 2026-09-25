// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QAbstractButton>
#include <QApplication>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QTimer>
#include <deque>
#include <functional>

// Answers the modal boxes of a flow in turn, the way a user clicks
// through his alert chains. Every step runs once on the next modal
// dialog that shows up, a flow that opens more boxes than scripted
// gets them rejected so no test ever hangs.
namespace horcom::test {

class DialogDriver {
 public:
  using Step = std::function<void(QDialog*)>;

  DialogDriver() {
    timer_.setInterval(15);
    QObject::connect(&timer_, &QTimer::timeout, [this]() { poll(); });
    timer_.start();
  }

  /// Queues one answer for the next dialog.
  DialogDriver& then(Step s) {
    steps_.push_back(std::move(s));
    return *this;
  }

  /// @return steps not consumed yet
  [[nodiscard]] std::size_t pending() const { return steps_.size(); }
  /// @return dialogs that appeared beyond the script
  [[nodiscard]] int unexpected() const { return unexpected_; }
  /// @return the window titles of the dialogs in the order they came
  [[nodiscard]] const QStringList& titles() const { return titles_; }

  /// A step that clicks the button whose text contains the caption.
  static Step click(const QString& caption) {
    return [caption](QDialog* d) {
      for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
        if (b->text().contains(caption)) {
          b->click();
          return;
        }
      }
      d->reject();
    };
  }

  /// A step that fills the line edits in order and clicks the caption.
  static Step fill(const QStringList& values, const QString& caption) {
    return [values, caption](QDialog* d) {
      const QList<QLineEdit*> edits = d->findChildren<QLineEdit*>();
      for (int i = 0; i < values.size() && i < edits.size(); ++i) {
        edits[i]->setText(values[i]);
      }
      click(caption)(d);
    };
  }

  /// A step that activates one row of the dialog's list.
  static Step pick_row(int row) {
    return [row](QDialog* d) {
      QListWidget* list = d->findChild<QListWidget*>();
      if (list == nullptr) {
        d->reject();
        return;
      }
      list->setCurrentRow(row);
      emit list->itemActivated(list->currentItem());
    };
  }

 private:
  void poll() {
    auto* d = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if (d == nullptr || d == last_) {
      return;
    }
    last_ = d;
    // a box built on the stack may reuse the address of the one before
    QObject::connect(d, &QDialog::finished, &timer_, [this]() { last_ = nullptr; });
    QObject::connect(d, &QObject::destroyed, &timer_, [this]() { last_ = nullptr; });
    titles_ << d->windowTitle();
    if (steps_.empty()) {
      ++unexpected_;
      d->reject();
      return;
    }
    Step s = std::move(steps_.front());
    steps_.pop_front();
    s(d);
  }

  QTimer timer_;
  std::deque<Step> steps_;
  QDialog* last_ = nullptr;
  int unexpected_ = 0;
  QStringList titles_;
};

}  // namespace horcom::test
