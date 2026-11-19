// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QAbstractButton>
#include <QApplication>
#include <QDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTimer>
#include <QWidget>
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

  /// A step that clicks the button whose whole caption is the given one.
  /// @note A part would also hit the longer captions around it, 1 inside
  ///       10 or HALBSUMME inside his ASPEKT row
  static Step click_exact(const QString& caption) {
    return [caption](QDialog* d) {
      for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
        if (b->text().remove('&').trimmed() == caption) {
          b->click();
          return;
        }
      }
      d->reject();
    };
  }

  /// A step that activates the list row whose trimmed text is the caption.
  static Step pick_text(const QString& caption) {
    return [caption](QDialog* d) {
      if (QListWidget* list = d->findChild<QListWidget*>()) {
        for (int i = 0; i < list->count(); ++i) {
          if (list->item(i)->text().trimmed() == caption) {
            list->setCurrentRow(i);
            emit list->itemActivated(list->currentItem());
            return;
          }
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

/// Presses the space bar on the target once a wait of the program stands.
/// @param target the window that takes the key, it owns the watch
/// @note The bar of wait_key lives only while its loop runs, a key sent
///       before it would be lost and a slow runner may draw longer than
///       any fixed delay. The bar sits on the main window or on an output
///       window of its own, so every window is searched
inline void key_at_wait(QWidget& target) {
  auto* poll = new QTimer(&target);
  QObject::connect(poll, &QTimer::timeout, &target, [&target, poll]() {
    for (const QWidget* window : QApplication::topLevelWidgets()) {
      if (window->findChild<QLabel*>("waitBar") != nullptr) {
        poll->stop();
        poll->deleteLater();
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(&target, &key);
        return;
      }
    }
  });
  poll->start(15);
}

}  // namespace horcom::test
