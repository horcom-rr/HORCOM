// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QPixmap>
#include <QPointer>
#include <QTimer>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QStyle>
#include <QTableWidget>
#include <QTextStream>
#include <QWidget>
#include <algorithm>
#include <cmath>

#include "painter.hpp"
#include "wheel_widget.hpp"

// Finds what does not fit on a screen, the texts of a drawn sheet that
// leave the paper or run into each other or a column line, and the table
// cells, headers, labels and buttons whose text is wider than the room
// the layout gives them. With HORCOM_SHOTS set every checked view is
// also saved there as a PNG with its findings beside it.
namespace horcom::test {

namespace layout_detail {

// the share of the text height two middles may lie apart in one row
constexpr double kRowShare = 0.5;
// the ink of a line stands in the middle of its cell, this share of the
// height above and below the middle
constexpr double kInkBand = 0.3;
// how many primitives back the ground of a text may lie
constexpr std::size_t kGroundLookBack = 4;
// a rule this share of a cell from its edge touches the neighbour
constexpr double kEdgeShare = 0.15;
// canvas units a text may reach past its neighbour or the paper edge
constexpr double kTolerance = 0.5;

// the box the ink of a text covers, the leading and trailing blanks off
inline QRectF ink_box(const Primitive& item, double scale) {
  const QString text = QString::fromStdString(item.text);
  const qsizetype lead = text.size() - QString(text).remove(QRegularExpression("^ +")).size();
  const QString trimmed = text.trimmed();
  if (trimmed.isEmpty()) {
    return {};
  }
  const QRectF full = text_box(item, scale);
  Primitive part = item;
  part.align_left = true;
  part.align_right = false;
  part.x1 = 0.0;
  part.text = text.left(lead).toStdString();
  const double lead_w = lead > 0 ? text_box(part, scale).width() : 0.0;
  part.text = trimmed.toStdString();
  const double ink_w = text_box(part, scale).width();
  if (item.vertical) {
    return {full.x(), full.bottom() - lead_w - ink_w, full.width(), ink_w};
  }
  return {full.x() + lead_w, full.y(), ink_w, full.height()};
}

inline QString where(const Primitive& p) {
  return QString("\"%1\" at %2,%3").arg(QString::fromStdString(p.text)).arg(p.x1, 0, 'f', 0).arg(p.y1, 0, 'f', 0);
}

// two texts share a row when their middles lie closer than half the
// smaller height, his stacked two line heads stand a line apart
inline bool same_row(const QRectF& a, const QRectF& b) {
  return std::abs(a.center().y() - b.center().y()) < kRowShare * std::min(a.height(), b.height());
}

}  // namespace layout_detail

/// Whether the platform sets real faces, the measurements mean nothing
/// on a platform without fonts like the offscreen one of Windows.
///
/// @return true when the sheet face or its metric twin is installed
inline bool fonts_available() {
  return QFontDatabase::hasFamily(QStringLiteral("Courier New")) ||
         QFontDatabase::hasFamily(QStringLiteral("Liberation Mono"));
}

/// The fitting problems of a drawn sheet at one scale.
///
/// @param dl    the sheet
/// @param scale device pixels per canvas unit the texts are set at
/// @return one line per problem, empty when everything fits
inline QStringList sheet_issues(const DisplayList& dl, double scale) {
  using namespace layout_detail;
  QStringList out;
  const QRectF paper(-kTolerance, -kTolerance, dl.width + 2.0 * kTolerance, dl.height + 2.0 * kTolerance);
  // the texts in drawing order, each marked when it stands on an opaque
  // ground of its own that covers whatever it overlaps
  struct Placed {
    QRectF box;
    const Primitive* p;
    bool grounded;
    std::size_t at;
  };
  std::vector<Placed> texts;
  std::vector<std::pair<const Primitive*, std::size_t>> columns;
  std::size_t at = 0;
  for (const Primitive& p : dl.items) {
    switch (p.kind) {
      case Primitive::Kind::kText: {
        const QRectF box = ink_box(p, scale);
        if (box.isEmpty()) {
          break;
        }
        if (!paper.contains(box)) {
          out << QString("text leaves the paper: %1, box %2..%3 x %4..%5")
                     .arg(where(p))
                     .arg(box.left(), 0, 'f', 1)
                     .arg(box.right(), 0, 'f', 1)
                     .arg(box.top(), 0, 'f', 1)
                     .arg(box.bottom(), 0, 'f', 1);
        }
        // his OPAQUE texts and white patches, a box laid just before the
        // text that covers the band its ink stands in, shadow lines of a
        // boxed text may come between
        const QRectF band(box.left(), box.center().y() - kInkBand * box.height(), box.width(),
                          2.0 * kInkBand * box.height());
        std::size_t ground = 0;
        for (std::size_t back = 1; back <= kGroundLookBack && back <= at; ++back) {
          const Primitive& g = dl.items[at - back];
          if (g.kind == Primitive::Kind::kRect &&
              QRectF(g.x1 - g.r1, g.y1 - g.r2, 2.0 * g.r1, 2.0 * g.r2)
                  .adjusted(-kTolerance, -kTolerance, kTolerance, kTolerance)
                  .contains(band)) {
            ground = at - back + 1;
            break;
          }
        }
        texts.push_back({box, &p, ground > 0, ground > 0 ? ground - 1 : at});
        break;
      }
      case Primitive::Kind::kLine: {
        if (!paper.contains(QPointF(p.x1, p.y1)) || !paper.contains(QPointF(p.x2, p.y2))) {
          out << QString("line leaves the paper: %1,%2 to %3,%4").arg(p.x1).arg(p.y1).arg(p.x2).arg(p.y2);
        }
        if (std::abs(p.x1 - p.x2) < 0.01 && std::abs(p.y1 - p.y2) > 4.0) {
          columns.emplace_back(&p, at);
        }
        break;
      }
      case Primitive::Kind::kCircle:
      case Primitive::Kind::kSector: {
        const double r = std::max(p.r1, p.r2);
        if (!paper.contains(QRectF(p.x1 - r, p.y1 - r, 2.0 * r, 2.0 * r))) {
          out << QString("circle leaves the paper at %1,%2 r %3").arg(p.x1).arg(p.y1).arg(r);
        }
        break;
      }
      case Primitive::Kind::kGlyph: {
        const double h = p.size * 0.55;
        if (!paper.contains(QRectF(p.x1 - h, p.y1 - h, 2.0 * h, 2.0 * h))) {
          out << QString("glyph leaves the paper: %1").arg(where(p));
        }
        break;
      }
      case Primitive::Kind::kRect:
        if (!paper.contains(QRectF(p.x1 - p.r1, p.y1 - p.r2, 2.0 * p.r1, 2.0 * p.r2))) {
          out << QString("box leaves the paper at %1,%2").arg(p.x1).arg(p.y1);
        }
        break;
      case Primitive::Kind::kDot: break;
    }
    ++at;
  }
  for (std::size_t i = 0; i < texts.size(); ++i) {
    const QRectF& a = texts[i].box;
    for (std::size_t j = i + 1; j < texts.size(); ++j) {
      const QRectF& b = texts[j].box;
      // a later text on its own ground covers the earlier one cleanly
      // like his OPAQUE output, only a mix of two inks is a fault
      if (texts[j].grounded || !a.intersects(b) || (!texts[i].p->vertical && !texts[j].p->vertical && !same_row(a, b))) {
        continue;
      }
      // the same text twice on the same spot sets the same ink
      if (texts[i].p->text == texts[j].p->text && a == b) {
        continue;
      }
      const QRectF shared = a.intersected(b);
      if (shared.width() > kTolerance && shared.height() > kTolerance) {
        out << QString("texts overlap by %1: %2 and %3").arg(shared.width(), 0, 'f', 1).arg(where(*texts[i].p),
                                                                                            where(*texts[j].p));
      }
    }
    // a column rule through a line of text, the upward texts run along
    // the rules on purpose
    if (texts[i].p->vertical) {
      continue;
    }
    for (const auto& [c, line_at] : columns) {
      // a ground laid after the rule wipes it under the text
      if (texts[i].grounded && line_at < texts[i].at) {
        continue;
      }
      const double top = std::min(c->y1, c->y2);
      const double bottom = std::max(c->y1, c->y2);
      // a rule in a blank between the fields of a fixed pitch line
      const QString s = QString::fromStdString(texts[i].p->text);
      const QRectF full = text_box(*texts[i].p, scale);
      if (!s.isEmpty() && full.width() > 0.0) {
        const double rel = (c->x1 - full.left()) / (full.width() / static_cast<double>(s.size()));
        const auto idx = static_cast<qsizetype>(std::floor(rel));
        const auto blank = [&s](qsizetype k) { return k < 0 || k >= s.size() || s.at(k) == ' '; };
        if (blank(idx) && (rel - static_cast<double>(idx) > kEdgeShare || blank(idx - 1)) &&
            (static_cast<double>(idx + 1) - rel > kEdgeShare || blank(idx + 1))) {
          continue;
        }
      }
      if (c->x1 > a.left() + kTolerance && c->x1 < a.right() - kTolerance && bottom > a.center().y() &&
          top < a.center().y()) {
        out << QString("column line at x %1 crosses %2").arg(c->x1).arg(where(*texts[i].p));
      }
    }
  }
  return out;
}

/// The fitting problems of a widget tree as it is laid out now.
///
/// @param root the window or dialog
/// @return one line per problem, empty when everything fits
inline QStringList widget_issues(QWidget* root) {
  QStringList out;
  for (QTableWidget* t : root->findChildren<QTableWidget*>()) {
    if (!t->isVisibleTo(root)) {
      continue;
    }
    const QString name = t->objectName().isEmpty() ? QStringLiteral("table") : t->objectName();
    const auto* view = static_cast<const QAbstractItemView*>(t);
    for (int c = 0; c < t->columnCount(); ++c) {
      if (t->isColumnHidden(c)) {
        continue;
      }
      const int room = t->columnWidth(c);
      if (const QTableWidgetItem* h = t->horizontalHeaderItem(c); h != nullptr && t->horizontalHeader()->isVisible()) {
        const int need = t->horizontalHeader()->sectionSizeHint(c);
        if (need > room) {
          out << QString("%1 header %2 \"%3\" needs %4 px, the column has %5").arg(name).arg(c).arg(h->text()).arg(
                     need).arg(room);
        }
      }
      // the delegate measures the cells, the sprites of the zodiac
      // cells included, the widest text names the column
      const int need = view->sizeHintForColumn(c) - (t->showGrid() ? 1 : 0);
      if (need > room) {
        QString widest;
        for (int r = 0; r < t->rowCount(); ++r) {
          if (const QTableWidgetItem* item = t->item(r, c); item != nullptr && item->text().size() > widest.size()) {
            widest = item->text();
          }
        }
        out << QString("%1 column %2 \"%3\" needs %4 px, it has %5").arg(name).arg(c).arg(widest.replace('\n', " / ")).arg(
                   need).arg(room);
      }
    }
    // the delegate measures the rows too, the two line cells included
    for (int r = 0; r < t->rowCount(); ++r) {
      if (t->isRowHidden(r)) {
        continue;
      }
      const int need = view->sizeHintForRow(r) - (t->showGrid() ? 1 : 0);
      if (need > t->rowHeight(r)) {
        QString tallest;
        for (int c = 0; c < t->columnCount(); ++c) {
          if (const QTableWidgetItem* item = t->item(r, c); item != nullptr && item->text().count('\n') >= tallest.count('\n')) {
            tallest = item->text();
          }
        }
        out << QString("%1 row %2 \"%3\" needs %4 px height, it has %5").arg(name).arg(r).arg(tallest.replace('\n', " / ")).arg(
                   need).arg(t->rowHeight(r));
      }
    }
    // a result dialog shows its table whole, the docks of the main
    // window scroll their further columns on purpose but close flush
    if (t->horizontalScrollBar()->isVisible()) {
      if (qobject_cast<QDialog*>(t->window()) != nullptr) {
        out << QString("%1 scrolls sideways, %2 px of columns in %3 px").arg(name).arg(t->horizontalHeader()->length()).arg(
                   t->viewport()->width());
      } else if (t->horizontalScrollBar()->value() == 0) {
        const int edge = t->viewport()->width();
        for (int c = 0; c < t->columnCount(); ++c) {
          const int x = t->columnViewportPosition(c);
          if (!t->isColumnHidden(c) && x < edge && x + t->columnWidth(c) > edge + 1) {
            out << QString("%1 column %2 is cut at the edge, %3 of %4 px shown").arg(name).arg(c).arg(edge - x).arg(
                       t->columnWidth(c));
          }
        }
      }
    }
  }
  for (QLabel* l : root->findChildren<QLabel*>()) {
    if (!l->isVisibleTo(root) || l->text().isEmpty() || l->pixmap().isNull() == false) {
      continue;
    }
    if (l->wordWrap()) {
      if (l->heightForWidth(l->width()) > l->height() + 1) {
        out << QString("label \"%1\" needs %2 px height, it has %3").arg(l->text().left(60)).arg(
                   l->heightForWidth(l->width())).arg(l->height());
      }
    } else if (l->minimumSizeHint().width() > l->width() + 1) {
      out << QString("label \"%1\" needs %2 px, it has %3").arg(l->text().left(60)).arg(l->minimumSizeHint().width()).arg(
                 l->width());
    }
  }
  for (QAbstractButton* b : root->findChildren<QAbstractButton*>()) {
    if (!b->isVisibleTo(root) || b->text().isEmpty()) {
      continue;
    }
    // a push button's hint carries the minimum width of the style, a
    // button is cut only when its caption with the margins is wider too
    int need = b->sizeHint().width();
    if (qobject_cast<QPushButton*>(b) != nullptr) {
      QString caption = b->text();
      caption.remove('&');
      need = std::min(need, static_cast<int>(std::ceil(QFontMetricsF(b->font()).horizontalAdvance(caption))) +
                                2 * b->style()->pixelMetric(QStyle::PM_ButtonMargin, nullptr, b));
    }
    if (need > b->width() + 1) {
      out << QString("button \"%1\" needs %2 px, it has %3").arg(b->text()).arg(need).arg(b->width());
    }
  }
  for (QComboBox* c : root->findChildren<QComboBox*>()) {
    if (!c->isVisibleTo(root)) {
      continue;
    }
    if (c->minimumSizeHint().width() > c->width() + 1 &&
        QFontMetricsF(c->font()).horizontalAdvance(c->currentText()) + 30 > c->width()) {
      out << QString("combo \"%1\" needs %2 px, it has %3").arg(c->currentText()).arg(c->minimumSizeHint().width()).arg(
                 c->width());
    }
  }
  for (QListWidget* l : root->findChildren<QListWidget*>()) {
    if (!l->isVisibleTo(root)) {
      continue;
    }
    const QFontMetricsF fm(l->font());
    for (int i = 0; i < l->count(); ++i) {
      const double need = fm.horizontalAdvance(l->item(i)->text()) + 12;
      if (need > l->viewport()->width() + 0.5) {
        out << QString("list row \"%1\" needs %2 px, the list shows %3").arg(l->item(i)->text()).arg(std::ceil(need)).arg(
                   l->viewport()->width());
        break;
      }
    }
  }
  return out;
}

/// The problems of the drawn sheets inside a window, at the scale they
/// are shown and at the plain one of a small window.
///
/// @param root the window holding sheet canvases
/// @return one line per problem
inline QStringList canvas_issues(QWidget* root) {
  QStringList out;
  QList<WheelWidget*> canvases = root->findChildren<WheelWidget*>();
  if (auto* self = qobject_cast<WheelWidget*>(root)) {
    canvases << self;
  }
  for (WheelWidget* w : canvases) {
    const DisplayList& dl = w->shown_list();
    if (dl.items.empty() || !w->isVisibleTo(root)) {
      continue;
    }
    // the paper margin of the widget, see WheelWidget
    const double shown = std::min((w->width() - 28.0) / dl.width, (w->height() - 28.0) / dl.height);
    for (const double scale : {shown, 1.0}) {
      for (const QString& s : sheet_issues(dl, scale)) {
        const QString line = QString("[scale %1] %2").arg(scale, 0, 'f', 2).arg(s);
        if (!out.contains(line)) {
          out << line;
        }
      }
    }
  }
  return out;
}

/// Checks a view and saves its capture.
///
/// @param w    the window or dialog
/// @param name the file stem of the capture
/// @return the problems of its widgets and drawn sheets
inline QStringList check_view(QWidget* w, const QString& name) {
  // the layouts a late setText asked for, then a paint, the wheel
  // canvases lay out their sheet in it
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  const QPixmap shot = w->grab();
  QStringList issues = widget_issues(w) + canvas_issues(w);
  const QString dir = qEnvironmentVariable("HORCOM_SHOTS");
  if (!dir.isEmpty()) {
    QDir().mkpath(dir);
    shot.save(dir + "/" + name + ".png");
    QFile f(dir + "/" + name + ".txt");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream s(&f);
      s << w->windowTitle() << "  " << w->width() << "x" << w->height() << "\n";
      for (const QString& i : issues) {
        s << i << "\n";
      }
    }
  }
  return issues;
}

/// Checks every dialog and message box of a run the moment it shows,
/// installed on the application when HORCOM_SHOTS names a folder, so the
/// whole suite sweeps every screen its flows reach.
class LayoutSweep final : public QObject {
 public:
  using QObject::QObject;

  bool eventFilter(QObject* o, QEvent* e) override {
    if (e->type() == QEvent::Show) {
      // Qt's own file dialog stands in for the native one of the
      // platform, its look in box is none of the program's layout
      if (auto* d = qobject_cast<QDialog*>(o); d != nullptr && d->isWindow() && qobject_cast<QFileDialog*>(d) == nullptr) {
        // the filter runs before the box's own show handler, a message
        // box sizes itself there, the check waits one turn of the loop
        const QPointer<QDialog> box(d);
        const int n = ++count_;
        QTimer::singleShot(0, this, [box, n]() {
          if (box.isNull() || !box->isVisible()) {
            return;
          }
          QString title = box->windowTitle().trimmed();
          title.replace(QRegularExpression("[^A-Za-z0-9]+"), "_");
          check_view(box, QString("sweep_%1_%2").arg(n, 4, 10, QChar('0')).arg(title.left(40)));
        });
      }
    }
    return false;
  }

 private:
  int count_ = 0;
};

}  // namespace horcom::test
