// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "print_pages.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDialog>
#include <QFont>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <algorithm>

#include "choice_dialog.hpp"
#include "painter.hpp"
#include "theme.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

struct PrintPages {
  Q_DECLARE_TR_FUNCTIONS(PrintPages)
};

// his 0.8 * 640 * dpixp& / 72, the screen of kCanvasWidth by 459 units
// at 0.8 points each
constexpr double kPointsPerUnit = 0.8;
constexpr double kPointsPerInch = 72.0;
constexpr double kScreenHeight = 459.0;
// the a11 page of the DRUCKER-GRAPHIK DIN A4, ydr& = 980
constexpr double kA4Height = 980.0;
constexpr double kA4PointsPerUnit = 0.75;
constexpr double kA4LeftDiv = 14.0;
constexpr double kA4TopDiv = 80.0;
// the property that names the menu entry of an output window
constexpr const char* kOutputProperty = "horcomOutput";
// his drad2 CASE 2, the credit twelve units under the picture in size 10
constexpr double kCreditGap = 12.0;
constexpr double kCreditSize = 10.0;
// his print_font, th& = gdyp& / 60 and tw& = gdxp& / 90
constexpr double kListLinesPerPage = 60.0;
constexpr double kListColumns = 90.0;
// his FOR i& = ib& TO MIN(ib& + 52,ne&), 53 rows a page
constexpr int kListRowsPerPage = 53;
// his x% = gdxp& / 20
constexpr double kListMarginDiv = 20.0;

QString& print_file() {
  static QString path;
  return path;
}

// the fixed pitch face every system carries, Liberation Mono stands in
// where Courier New is missing
QFont list_font() {
  QFont f = theme::mono_font();
  f.setFixedPitch(true);
  f.setBold(true);
  return f;
}

// the HARDCOPY box, NEIN keeps the focus, PgUp and PgDn answer NEIN
class HardcopyBox final : public QDialog {
 public:
  explicit HardcopyBox(QWidget* parent) : QDialog(parent) {}

 protected:
  void keyPressEvent(QKeyEvent* e) override {
    if (e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown) {
      reject();
      return;
    }
    QDialog::keyPressEvent(e);
  }
};

}  // namespace

bool hardcopy_item(int item, bool pair_view) {
  // his CASE 46 TO 49,52,60,63,66,68,76,79,81,85,86,87,88,91 TO 101
  // with IF NOT (comp! OR comb! OR dppel!)
  const bool excluded = (item >= 46 && item <= 49) || item == 52 || item == 60 || item == 63 || item == 66 ||
                        item == 68 || item == 76 || item == 79 || item == 81 || (item >= 85 && item <= 88) ||
                        (item >= 91 && item <= 101);
  return !excluded || pair_view;
}

// ported from druck_einr_anz
bool prepare_printer(QWidget* parent, QPrinter& printer, PrintPage page, bool half_page) {
  // his @fanz(" DRUCKER BEREIT ? ")
  QMessageBox::information(parent, QStringLiteral("HORCOM"), PrintPages::tr(" DRUCKER BEREIT ? "));
  const bool landscape = (page == PrintPage::kHardcopy && !half_page) || page == PrintPage::kLinearA4;
  const QString c = landscape ? PrintPages::tr(" QUERFORMAT ( bzw. 'LANDSCAPE' ) ")
                              : PrintPages::tr(" HOCHFORMAT ( bzw. 'PORTRÄT' ) ");
  const QString d = PrintPages::tr(" oder BENUTZERDEF. HALBSEITE ");
  QString b;
  QString e;
  switch (page) {
    case PrintPage::kHardcopy:
      b = half_page ? "DIN A5" + d + c : "DIN A4" + c;
      e = half_page ? PrintPages::tr("HARDCOPY - FORMAT : DIN A5") : PrintPages::tr("HARDCOPY - FORMAT : DIN A4");
      break;
    case PrintPage::kGraphicA5:
      b = "DIN A4" + d + c;
      break;
    case PrintPage::kGraphicA4:
    case PrintPage::kLinearA4:
    case PrintPage::kDouble:
      b = "DIN A4" + c;
      break;
    case PrintPage::kList:
      b = c;
      break;
  }
  // his alertbox(1,e$,a$,b$,"EINSTELLEN und WARTEN bis DRUCKER ARBEITET !",1,wr$),
  // ESC falls through like his single answer
  (void)ChoiceDialog::ask(parent, PrintPages::tr(" HINWEIS "),
                          {e, PrintPages::tr("Mit 'EINRICHTEN' ( bzw. 'SETUP' ) "), b,
                           PrintPages::tr("EINSTELLEN und WARTEN bis DRUCKER ARBEITET !")},
                          {PrintPages::tr("Weiter")}, 0);
  printer.setPageSize(QPageSize(QPageSize::A4));
  printer.setPageOrientation(landscape ? QPageLayout::Landscape : QPageLayout::Portrait);
  if (!print_file().isEmpty()) {
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(print_file());
    return true;
  }
  // his DLG PRINT WIN(),0,dr&
  QPrintDialog dialog(&printer, parent);
  return dialog.exec() == QDialog::Accepted;
}

// ported from dr_fehl
void printer_failed(QWidget* parent) {
  // his @fanz("DRUCKER ABBRUCH")
  QMessageBox::information(parent, QStringLiteral("HORCOM"), PrintPages::tr("DRUCKER ABBRUCH"));
}

QRectF robert_page_rect(const QPrinter& printer, double left_div, double top_div, double factor) {
  const QRectF page = printer.pageRect(QPrinter::DevicePixel);
  const double px = printer.resolution() / kPointsPerInch;
  return {page.width() / left_div, page.height() / top_div, factor * kPointsPerUnit * kCanvasWidth * px,
          factor * kPointsPerUnit * kScreenHeight * px};
}

QRectF robert_a4_rect(const QPrinter& printer) {
  // his VIEWPORT gdxp& / 14,gdyp& / 80,xdr&,ydr&,0.75 * xdr& * dpixp& / 72,0.75 * ydr& * dpiyp& / 72
  const QRectF page = printer.pageRect(QPrinter::DevicePixel);
  const double px = printer.resolution() / kPointsPerInch;
  return {page.width() / kA4LeftDiv, page.height() / kA4TopDiv, kA4PointsPerUnit * kCanvasWidth * px,
          kA4PointsPerUnit * kA4Height * px};
}

void mark_output(QWidget* output, int item) {
  output->setProperty(kOutputProperty, item);
}

int output_item(const QWidget* w) {
  return w != nullptr ? w->property(kOutputProperty).toInt() : 0;
}

void paint_robert_page(QPainter& p, const DisplayList& page, const QRectF& target) {
  DisplayList stamped = page;
  stamp_credit(stamped, print_stamp());
  paint_fitted(p, stamped, target);
}

DisplayList output_page(QWidget* output) {
  if (output == nullptr) {
    return {};
  }
  for (const WheelWidget* canvas : output->findChildren<WheelWidget*>()) {
    // a closing output hides its window, the canvas itself stays shown
    if (!canvas->isHidden() && !canvas->display_list().items.empty()) {
      return canvas->display_list();
    }
  }
  return {};
}

// ported from hardcopy, the picture of the output window stretched onto
// the page with the credit of drad2 under it
void paint_output(QPainter& p, QWidget* output, const QRectF& target) {
  const DisplayList page = output_page(output);
  if (!page.items.empty()) {
    paint_robert_page(p, page, target);
    return;
  }
  const QPixmap shot = output->grab();
  if (shot.isNull()) {
    return;
  }
  const double s = std::min(target.width() / shot.width(), target.height() / shot.height());
  const QRectF picture(target.x(), target.y(), shot.width() * s, shot.height() * s);
  p.drawPixmap(picture, shot, QRectF(shot.rect()));
  const double unit = target.width() / kCanvasWidth;
  QFont f = p.font();
  f.setPixelSize(std::max(1, static_cast<int>(kCreditSize * unit)));
  p.setFont(f);
  p.setPen(Qt::black);
  p.drawText(QRectF(target.x(), picture.bottom(), target.width(), 2.0 * kCreditGap * unit), Qt::AlignHCenter | Qt::AlignVCenter,
             QString::fromUtf8(kCreditLine.data(), static_cast<int>(kCreditLine.size())) + " " +
                 QString::fromStdString(print_stamp()));
}

double list_pitch(double page_width, qsizetype longest) {
  const double x0 = page_width / kListMarginDiv;
  const double columns = std::max<double>(1.0, static_cast<double>(longest));
  return std::min(page_width / kListColumns, (page_width - 2.0 * x0) / columns);
}

// ported from datei_pr and lese_text_pr. His font forced a ninetieth of
// the width onto each character and clipped the long rows, the port
// narrows the pitch until the longest row fits
bool print_text_rows(QPrinter& printer, const QStringList& rows) {
  QPainter p(&printer);
  if (!p.isActive()) {
    return false;
  }
  const QRectF page = printer.pageRect(QPrinter::DevicePixel);
  const double th = page.height() / kListLinesPerPage;
  const double x0 = page.width() / kListMarginDiv;
  qsizetype longest = 0;
  for (const QString& r : rows) {
    longest = std::max(longest, r.size());
  }
  const double tw = list_pitch(page.width(), longest);
  QFont f = list_font();
  f.setPixelSize(std::max(1, static_cast<int>(th)));
  const double natural = QFontMetricsF(f, &printer).horizontalAdvance(QLatin1Char('M'));
  if (natural > 0.0) {
    f.setStretch(std::clamp(static_cast<int>(100.0 * tw / natural), 1, 4000));
  }
  p.setFont(f);
  p.setPen(Qt::black);
  int sheet = 1;
  for (qsizetype first = 0; first < rows.size(); first += kListRowsPerPage) {
    if (first > 0 && !printer.newPage()) {
      return false;
    }
    // his TEXT x%,y% - th&,SPACE$(38) + "-" + STR$(bl&) + "-"
    p.drawText(QRectF(x0, 0.0, page.width() - x0, th), Qt::AlignLeft | Qt::AlignTop,
               QString(38, QLatin1Char(' ')) + "-" + QString::number(sheet) + "-");
    const qsizetype last = std::min(rows.size(), first + kListRowsPerPage);
    for (qsizetype i = first; i < last; ++i) {
      const double y = static_cast<double>(i - first + 1) * th;
      p.drawText(QRectF(x0, y, page.width() - x0, th), Qt::AlignLeft | Qt::AlignTop, rows[i]);
    }
    ++sheet;
  }
  return p.end();
}

QStringList wrap_text_rows(const QString& text, int columns) {
  QStringList rows;
  for (const QString& paragraph : text.split(QLatin1Char('\n'))) {
    QString row;
    for (const QString& word : paragraph.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
      if (!row.isEmpty() && row.size() + 1 + word.size() > columns) {
        rows << row;
        row.clear();
      }
      row += (row.isEmpty() ? QString() : QStringLiteral(" ")) + word;
    }
    rows << row;
  }
  return rows;
}

std::string print_stamp() {
  // his datumakt$ = DATE$ and tim$ = LEFT$(TIME$,5)
  return QDateTime::currentDateTime().toString(QStringLiteral("dd.MM.yyyy HH:mm")).toStdString();
}

void set_print_file(const QString& path) {
  print_file() = path;
}

// ported from start_hardc, the box at the lower right of the output
bool ask_hardcopy(QWidget* output) {
  HardcopyBox box(output);
  // his DIALOG #diha&,x&,y&,@xk(120),@yk(52)," HARDCOPY ?"
  box.setWindowTitle(PrintPages::tr(" HARDCOPY ?"));
  // F1 in the box opens his ERLÄUTERUNG 2
  box.setProperty("horcomHelp", QStringLiteral("komm2"));
  auto* row = new QHBoxLayout(&box);
  auto* yes = new QPushButton(PrintPages::tr("JA"), &box);
  auto* no = new QPushButton(PrintPages::tr("NEIN"), &box);
  row->addWidget(yes);
  row->addWidget(no);
  QObject::connect(yes, &QPushButton::clicked, &box, &QDialog::accept);
  QObject::connect(no, &QPushButton::clicked, &box, &QDialog::reject);
  // his ~SetFocus(DLGITEM(diha&,102)), NEIN holds the focus
  no->setDefault(true);
  no->setFocus();
  box.adjustSize();
  if (output != nullptr) {
    const QPoint corner = output->mapToGlobal(QPoint(output->width(), output->height()));
    box.move(corner - QPoint(box.width() + 8, box.height() + 8));
  }
  return box.exec() == QDialog::Accepted;
}

}  // namespace horcom
