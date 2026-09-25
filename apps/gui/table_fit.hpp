// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QAbstractItemModel>
#include <QEvent>
#include <QFontInfo>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QPointer>
#include <QScrollBar>
#include <QStyle>
#include <QTableWidget>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <vector>

namespace horcom {

/// Makes a table ask for the room of all its columns, so the window
/// holding it grows until every column shows whole. The width a column
/// needs is the larger of its cells and its head, a stretched last
/// column included.
///
/// @param t the table, its cells filled
inline void fit_columns(QTableWidget* t) {
  int width = 2 * t->frameWidth() + t->verticalScrollBar()->sizeHint().width();
  if (t->verticalHeader()->isVisible()) {
    width += t->verticalHeader()->sizeHint().width();
  }
  // the view's measure through the base, where it is public
  const auto* view = static_cast<const QAbstractItemView*>(t);
  for (int c = 0; c < t->columnCount(); ++c) {
    if (!t->isColumnHidden(c)) {
      width += std::max(view->sizeHintForColumn(c), t->horizontalHeader()->sectionSizeHint(c));
    }
  }
  t->setMinimumWidth(width);
}

/// Grows the text of a table with its window like his screens, which
/// stretched one sheet over the whole display. The text never falls
/// below the size of the theme, a table larger than its window scrolls
/// as before. Labels handed to follow() grow by the same factor.
class TableZoom final : public QObject {
 public:
  /// Installs the zoom on a table, it lives as the table's child.
  ///
  /// @param table      the table at the text size of the theme
  /// @param fixed_rows true when the rows keep a set height, false when
  ///                   they follow their contents
  TableZoom(QTableWidget* table, bool fixed_rows) : QObject(table), table_(table), fixed_rows_(fixed_rows) {
    table->ensurePolished();
    base_px_ = QFontInfo(table->font()).pixelSize();
    head_px_ = QFontInfo(table->horizontalHeader()->font()).pixelSize();
    base_row_ = table->verticalHeader()->defaultSectionSize();
    base_icon_ = table->iconSize().isValid()
                     ? table->iconSize().width()
                     : table->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, table);
    timer_.setSingleShot(true);
    timer_.setInterval(kSettleMs);
    connect(&timer_, &QTimer::timeout, this, &TableZoom::apply);
    table->installEventFilter(this);
    // a walk refills the cells and their room changes with them
    QAbstractItemModel* model = table->model();
    connect(model, &QAbstractItemModel::dataChanged, this, [this]() { timer_.start(); });
    connect(model, &QAbstractItemModel::rowsInserted, this, [this]() { timer_.start(); });
    connect(model, &QAbstractItemModel::modelReset, this, [this]() { timer_.start(); });
  }

  /// Lets a label grow with the table.
  ///
  /// @param label a label of the same window
  void follow(QLabel* label) {
    label->ensurePolished();
    followers_.push_back({label, QFontInfo(label->font()).pixelSize()});
  }

  /// @return the factor the text stands at, one for the theme size
  [[nodiscard]] double scale() const { return scale_; }

  /// Fits the text to the window now instead of after the resize
  /// settled, the tests and the first show use it.
  void apply() {
    if (table_ == nullptr || !table_->isVisible()) {
      return;
    }
    // the state reached stands while it fits and the window kept its size,
    // the resizes of its own labels start no new search
    const QSize window = table_->window()->size();
    const double ratio = fit_ratio();
    if (ratio >= 1.0 && (window == searched_ || ratio < 1.0 + kSlack || scale_ >= kMaxScale)) {
      return;
    }
    searched_ = window;
    // the labels that grow along take room from the table, so the fit is
    // searched between a factor that fits and one that overflows
    double lo = 1.0;
    double hi = kMaxScale;
    set_scale(hi);
    if (fit_ratio() >= 1.0) {
      return;
    }
    for (int i = 0; i < kSearchSteps; ++i) {
      const double mid = (lo + hi) / 2.0;
      set_scale(mid);
      if (fit_ratio() >= 1.0) {
        lo = mid;
      } else {
        hi = mid;
      }
    }
    set_scale(lo);
  }

 protected:
  bool eventFilter(QObject* watched, QEvent* e) override {
    if (watched == table_ && (e->type() == QEvent::Resize || e->type() == QEvent::Show)) {
      timer_.start();
    }
    return false;
  }

 private:
  // the resizes of a dragged window settle before the text follows
  static constexpr int kSettleMs = 60;
  // his sheet doubled on a large screen reads well, more only blurs rows
  static constexpr double kMaxScale = 2.0;
  // the share of room the fitted text may leave unused
  static constexpr double kSlack = 0.04;
  // halvings of the search, the factor lands within a hundredth
  static constexpr int kSearchSteps = 7;

  struct Follower {
    QPointer<QLabel> label;
    int base_px = 0;
  };

  // the room the table offers against the room its text needs, below one
  // when it overflows
  [[nodiscard]] double fit_ratio() const {
    const QRect room = table_->contentsRect();
    const auto* view = static_cast<const QAbstractItemView*>(table_);
    int width = 0;
    if (table_->verticalHeader()->isVisible()) {
      width += table_->verticalHeader()->sizeHint().width();
    }
    for (int c = 0; c < table_->columnCount(); ++c) {
      if (!table_->isColumnHidden(c)) {
        int w = view->sizeHintForColumn(c);
        if (table_->horizontalHeader()->isVisible()) {
          w = std::max(w, table_->horizontalHeader()->sectionSizeHint(c));
        }
        width += w;
      }
    }
    int height = table_->horizontalHeader()->isVisible() ? table_->horizontalHeader()->sizeHint().height() : 0;
    for (int r = 0; r < table_->rowCount(); ++r) {
      height += table_->rowHeight(r);
    }
    if (width <= 0 || height <= 0) {
      return 1.0;
    }
    return std::min(static_cast<double>(room.width()) / width, static_cast<double>(room.height()) / height);
  }

  void set_scale(double f) {
    scale_ = f;
    const auto px = [f](int base) { return std::max(base, static_cast<int>(std::lround(base * f))); };
    table_->setStyleSheet(QStringLiteral("QTableWidget { font-size: %1px; } QHeaderView::section { font-size: %2px; }")
                              .arg(px(base_px_))
                              .arg(px(head_px_)));
    table_->setIconSize(QSize(px(base_icon_), px(base_icon_)));
    for (const Follower& fl : followers_) {
      if (fl.label != nullptr) {
        fl.label->setStyleSheet(QStringLiteral("QLabel { font-size: %1px; }").arg(px(fl.base_px)));
      }
    }
    if (fixed_rows_) {
      table_->verticalHeader()->setDefaultSectionSize(px(base_row_));
    } else {
      table_->resizeRowsToContents();
    }
    table_->resizeColumnsToContents();
    // the labels take their new height before the table is measured
    if (QLayout* l = table_->window()->layout()) {
      l->activate();
    }
  }

  QPointer<QTableWidget> table_;
  bool fixed_rows_ = false;
  int base_px_ = 0;
  int head_px_ = 0;
  int base_row_ = 0;
  int base_icon_ = 0;
  double scale_ = 1.0;
  QSize searched_;
  std::vector<Follower> followers_;
  QTimer timer_;
};

}  // namespace horcom
