// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QElapsedTimer>
#include <QFontInfo>
#include <QFontMetricsF>
#include <QHeaderView>
#include <QImage>
#include <QKeyEvent>
#include <QListWidget>
#include <QPainter>
#include <QScrollBar>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include "choice_dialog.hpp"
#include "dialog_driver.hpp"
#include "doctest.h"
#include "layout_check.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/items.hpp"
#include "horcom/render/stat_sheet.hpp"
#include "painter.hpp"
#include "probe.hpp"
#include "table_fit.hpp"
#include "theme.hpp"

using namespace horcom;
using horcom::test::check_view;
using horcom::test::DialogDriver;

// Every screen shows its whole content, the texts of a drawn sheet stay
// on the paper and in their columns, the table cells, labels and buttons
// are as wide as their texts.
namespace {

// a synthetic evening birth, no real person
AafRecord sample_birth() {
  AafRecord r;
  r.surname = "TESTFALL";
  r.given = "MAX";
  r.place = "EICHENAU";
  r.day = 13;
  r.month = 10;
  r.year = 1992;
  r.hour = 3;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 19;
  return r;
}

void key(QWidget* d, int k) {
  QKeyEvent e(QEvent::KeyPress, k, Qt::NoModifier);
  QApplication::sendEvent(d, &e);
}

// a step that checks the dialog it lands on and closes it
DialogDriver::Step check_and_close(const QString& name, QStringList* issues) {
  return [name, issues](QDialog* d) {
    *issues += check_view(d, name);
    d->reject();
  };
}

// the measurements need real faces, a platform without fonts skips
bool measurable() {
  if (!horcom::test::fonts_available()) {
    MESSAGE("no sheet face installed on this platform, the layout checks skip");
    return false;
  }
  return true;
}

// the views that fit themselves after they open. The table zoom fits a
// moment after the resizes settled and the scroll bars follow one event
// later, a busy runner delays both past any fixed wait, so the check
// waits for the fitted state up to a deadline
constexpr int kSettleTimeoutMs = 10000;

template <typename Done>
void settle(Done done) {
  QElapsedTimer t;
  t.start();
  while (!done() && t.elapsed() < kSettleTimeoutMs) {
    QApplication::processEvents();
  }
}

void report(const QStringList& issues) {
  for (const QString& i : issues) {
    MESSAGE(i.toStdString());
  }
  CHECK(issues.isEmpty());
}

}  // namespace

TEST_CASE("layout: the main window holds its coordinate and cusp tables") {
  if (!measurable()) {
    return;
  }
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample_birth());
  QStringList issues;
  for (const QSize s : {QSize(1280, 800), QSize(1600, 900), QSize(1920, 1080)}) {
    w->resize(s);
    w->show();
    QApplication::processEvents();
    issues += check_view(w.get(), QString("main_%1x%2").arg(s.width()).arg(s.height()));
  }
  w->hide();
  report(issues);
}

TEST_CASE("layout: PLANETEN-KOORDINATEN and ZUSATZ-PLANETEN-KOORDINATEN") {
  if (!measurable()) {
    return;
  }
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample_birth());
  QStringList issues;
  {
    DialogDriver drive;
    drive
        .then([&issues](QDialog* d) {
          issues += check_view(d, "coord");
          QTimer::singleShot(0, d, [d]() { key(d, Qt::Key_Space); });
        })
        .then(DialogDriver::click(" NEIN "))
        // back in front after NEIN the table closes as a step of its own, a
        // queued close raced the driver, which then took the table returning
        // to the front for a new dialog
        .then([](QDialog* d) { d->reject(); });
    MainWindowProbe::coordinate_table(*w, false);
  }
  {
    DialogDriver drive;
    drive.then(check_and_close("coord_extra", &issues));
    MainWindowProbe::coordinate_table(*w, true);
  }
  report(issues);
}

TEST_CASE("layout: the coordinate screen grows its text with the window like his full screen sheet") {
  if (!measurable()) {
    return;
  }
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample_birth());
  w->resize(1920, 1080);
  int base_px = 0;
  int grown_px = 0;
  bool scrolls = true;
  QStringList issues;
  {
    DialogDriver drive;
    drive.then([&](QDialog* d) {
      auto* table = d->findChild<QTableWidget*>();
      if (table != nullptr) {
        base_px = QFontInfo(QApplication::font(table)).pixelSize();
        const auto text_px = [table]() { return QFontInfo(table->font()).pixelSize(); };
        const auto scrolling = [table]() {
          return table->verticalScrollBar()->isVisible() || table->horizontalScrollBar()->isVisible();
        };
        settle([&]() { return text_px() > base_px && !scrolling(); });
        grown_px = text_px();
        scrolls = scrolling();
        issues += check_view(d, "coord_zoomed");
      }
      d->reject();
    });
    MainWindowProbe::coordinate_table(*w, false);
  }
  // his screen filled the display, the text grows and still fits whole
  CHECK(grown_px > base_px);
  CHECK_FALSE(scrolls);
  report(issues);
}

TEST_CASE("layout: a table that grows with its window keeps its heads a step above the cells") {
  if (!measurable()) {
    return;
  }
  const QString sheet_before = qApp->styleSheet();
  theme::apply(theme::kTextScaleNormal, false);
  QString zoom_sheet;
  {
    // a plain window, the sweep captures dialogs and this one is small on
    // purpose
    QWidget d;
    auto* v = new QVBoxLayout(&d);
    auto* t = new QTableWidget(40, 6, &d);
    t->setHorizontalHeaderLabels({"Pl", "Ekl. Länge", "Breite", "Vel.'", "A", "Entf.AE"});
    v->addWidget(t);
    auto* zoom = new TableZoom(t, false);
    // a window too small for the table, the zoom settles at the theme size
    d.resize(240, 160);
    d.show();
    QApplication::processEvents();
    zoom->apply();
    zoom_sheet = t->styleSheet();
  }
  qApp->setStyleSheet(sheet_before);
  // the head size had been read from the font of the header widget, the
  // 14 pixels of the QWidget rule, and the table's own sheet then held the
  // heads below the size of the theme
  CHECK(zoom_sheet.contains(QString("QHeaderView::section { font-size: %1px; }").arg(theme::kTableHeadPx)));
  CHECK(zoom_sheet.contains(QString("QTableWidget { font-size: %1px; }").arg(theme::kTableCellPx)));
  CHECK(zoom_sheet.contains(QString("QHeaderView::section:vertical { font-size: %1px; }").arg(theme::kTableCellPx)));
}

TEST_CASE("layout: rows and lists grow with a larger text size") {
  if (!measurable()) {
    return;
  }
  const QString sheet_before = qApp->styleSheet();
  theme::apply(150, false);
  int row = 0;
  int row_need = 0;
  int list_min = 0;
  double list_need = 0.0;
  {
    // a plain window, the sweep captures dialogs
    QWidget host;
    auto* v = new QVBoxLayout(&host);
    auto* t = new QTableWidget(3, 2, &host);
    // a row of fixed pixels like his a18liv and arabtex. The header here
    // would raise it to its own minimum, the dialogs hide their row heads
    // and kept the pixels
    t->verticalHeader()->setMinimumSectionSize(1);
    t->verticalHeader()->setDefaultSectionSize(20);
    for (int r = 0; r < t->rowCount(); ++r) {
      t->setItem(r, 0, new QTableWidgetItem("UNGENANNT"));
    }
    v->addWidget(t);
    auto* l = new QListWidget(&host);
    l->addItem(" CHIRON         a=13.61  e=0.38  i=6.94°  T=50 Jahre ");
    v->addWidget(l);
    fit_rows(t);
    fit_list(l);
    row = t->verticalHeader()->defaultSectionSize();
    row_need = static_cast<const QAbstractItemView*>(t)->sizeHintForRow(0) - (t->showGrid() ? 1 : 0);
    list_min = l->minimumWidth();
    list_need = QFontMetricsF(l->font()).horizontalAdvance(l->item(0)->text()) + kListTextRoom + 2 * l->frameWidth();
  }
  qApp->setStyleSheet(sheet_before);
  // the text of the larger size had run over the rows and the list
  CHECK(row > 20);
  CHECK(row >= row_need);
  CHECK(list_min >= list_need);
}

TEST_CASE("layout: GRAD-LISTE with and without the degree panel") {
  if (!measurable()) {
    return;
  }
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, sample_birth());
  QStringList issues;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click(" JA "))  // ZWISCHEN - HÄUSER HINZUNEHMEN ?
        .then([&issues](QDialog* d) {
          issues += check_view(d, "grad_page1");
          QTimer::singleShot(0, d, [d]() {
            for (int i = 0; i < 12 && QApplication::activeModalWidget() == d; ++i) {
              key(d, Qt::Key_Space);
            }
          });
        })
        .then([&issues](QDialog* d) {
          auto* view = qobject_cast<QDialog*>(d->parentWidget());
          DialogDriver::click(" JA ")(d);  // Nach Länge SORTIEREN ?
          if (view != nullptr) {
            QTimer::singleShot(0, view, [view, &issues]() {
              issues += check_view(view, "grad_sorted");
              view->reject();
            });
          }
        });
    MainWindowProbe::degree_list(*w);
  }
  report(issues);
}

TEST_CASE("layout: the ASPEKTARIUM sheet") {
  if (!measurable()) {
    return;
  }
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample_birth());
  MainWindowProbe::aspect_settings(*w).equal_probability = false;
  QStringList issues;
  {
    DialogDriver drive;
    drive.then([&issues](QDialog* d) {
           issues += check_view(d, "aspektarium_ask");
           DialogDriver::click(" 16 ")(d);
         })
        .then(check_and_close("aspektarium", &issues));
    MainWindowProbe::open_aspektarium(*w);
  }
  report(issues);
}

TEST_CASE("layout: the DYNAMOGRAMM arcs and graph") {
  if (!measurable()) {
    return;
  }
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, sample_birth());
  w->resize(1280, 800);
  QStringList issues;
  int pass = 0;
  const auto arcs = [&issues, &pass](QDialog* d) {
    key(d, Qt::Key_Space);
    issues += check_view(d, QString("dynamo_arcs%1").arg(++pass));
    key(d, Qt::Key_Space);
  };
  {
    DialogDriver drive;
    drive.then(DialogDriver::fill({"20"}, "OK"))
        .then(DialogDriver::click(" JA "))    // NEGATIVE ( REGRESSIVE ) ZEIT-RICHTUNG
        .then(DialogDriver::click("JA"))      // MOND LAUFEND
        .then(DialogDriver::click("JA"))      // EINZEL - BÖGEN BEOBACHTEN ?
        .then(DialogDriver::click("COSINUS-BÖGEN"))
        .then(arcs)
        .then(arcs)
        .then(arcs)
        .then(arcs)
        .then(check_and_close("dynamo_graph", &issues));
    MainWindowProbe::dynamogram_view(*w);
  }
  report(issues);
}

TEST_CASE("layout: a text on his FONT WIDTH steps exactly its cells at any scale") {
  if (!measurable()) {
    return;
  }
  // sixty capitals at his size 13, seven units each, drawn at an odd scale
  // where a rounded pixel size would drift the face
  constexpr double kScale = 1.7;
  const Primitive p = screen_text(10.0, 40.0, 13.0, std::string(60, 'M'));
  const double cell = text_advance(p);
  CHECK(cell == doctest::Approx(7.0));
  CHECK(text_box(p, kScale).width() == doctest::Approx(60.0 * cell));
  QImage img(static_cast<int>(700 * kScale), static_cast<int>(60 * kScale), QImage::Format_ARGB32);
  img.fill(Qt::white);
  {
    QPainter painter(&img);
    painter.scale(kScale, kScale);
    DisplayList dl;
    dl.width = 700.0;
    dl.height = 60.0;
    dl.items.push_back(p);
    paint_display_list(painter, dl);
  }
  // the ink ends within the last cell, not a cell further or shorter
  int right = 0;
  for (int x = 0; x < img.width(); ++x) {
    for (int y = 0; y < img.height(); ++y) {
      if (qGray(img.pixel(x, y)) < 128) {
        right = x;
        break;
      }
    }
  }
  const double end = (10.0 + 60.0 * cell) * kScale;
  CHECK(right <= end + 1.0);
  CHECK(right >= end - cell * kScale);
}

TEST_CASE("layout: the STATISTIK list keeps two digit hours clear of its rule") {
  if (!measurable()) {
    return;
  }
  // invented records, no real people
  std::vector<StatSheetRow> rows;
  for (int i = 0; i < 24; ++i) {
    StatSheetRow r;
    r.slot = 1;
    r.value = (10.0 + 14.0 * i) * kDegToRad;
    r.label = "PERSON " + std::to_string(100 + i);
    r.moment = QString::asprintf("%2d.%2d.%5d  %2d h %2d'", 1 + i, 1 + i % 12, 1950 + i, 10 + i % 14, 30).toStdString();
    r.has_angle = true;
    r.angle = (5.0 + 13.0 * i) * kDegToRad;
    rows.push_back(r);
  }
  StatSheetText text;
  text.heads = "Datum     Zeit(UT)    AC";
  text.file = " Datei : DEMO.STA ";
  text.object_line = "Länge SO";
  text.window_line = "In ZEICHEN";
  text.total = 24;
  text.partial_count = 24;
  text.footer = "* Blättern: Leertaste | Zurück mit 'R'|Weitere Beding: 'W'|ENDE: Mit 'ESC' *";
  const DisplayList dl = build_stat_page(rows, text);
  QStringList issues;
  for (const double scale : {1.0, 1.6, 2.2}) {
    issues += horcom::test::sheet_issues(dl, scale);
  }
  report(issues);
}

TEST_CASE("layout: his choice box grows with a caption wider than its 460") {
  if (!measurable()) {
    return;
  }
  const QString wide = QString("SET the COLOUR of the OUTER SYMBOLS of the DOUBLE WHEEL and of the RETROGRADE MARK");
  ChoiceDialog box("CLICK the WANTED TOPIC !", {wide}, {QString("EXIT"), wide}, 0);
  box.show();
  QApplication::processEvents();
  CHECK(box.width() > 460);
  report(horcom::test::widget_issues(&box));
  box.hide();
}
