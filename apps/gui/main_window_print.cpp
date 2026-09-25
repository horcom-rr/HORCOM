// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// The printer side of the main window, the DRUCKER-OPTION, the HARDCOPY
// after an output, the DRUCKER-GRAPHIK choice before one, the F9
// DOPPEL-AUSDRUCK of two pictures on one page and the LETZTES BILD.

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QScopedValueRollback>
#include <QVBoxLayout>

#include <filesystem>
#include <memory>

#include "choice_dialog.hpp"
#include "main_window.hpp"
#include "painter.hpp"
#include "print_pages.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

// ported from titlew, the hints behind the caption of a screen that waits
// in wart, by the muuu& number of its entry
QString wart_hint(int item) {
  // his w$ = "| WEITER mit " + lt$ + " !" with lt$ = "Leertaste"
  const QString weiter = QStringLiteral(" ") + MainWindow::tr("| WEITER mit Leertaste !");
  // his e$
  const QString ende = MainWindow::tr(" | ENDE mit 'ESC' !");
  // his CASE 1 TO 34 of the first two menus and CASE 84 of the clock
  if (item <= 34 || item == menu_item::kClock) {
    return {};
  }
  switch (item) {
    // his moda& = 1 over the charts, the right button picks the planets
    case menu_item::kChartGraphic:
    case menu_item::kAspektarium:
    case menu_item::kMidpointGraphic:
    case menu_item::kReturns:
    case menu_item::kDayChart:
      return MainWindow::tr(" | EINZELNE Planeten HERVORHEBEN : RECHTE Maustaste") + ende;
    // his CASE 42,43,55,67,69 TO 75,89,90,97 and CASE 37,38,44
    case menu_item::kPlanetCoordinates:
    case menu_item::kExtraCoordinates:
    case menu_item::kDegreeList:
    case menu_item::kFixedStars:
    case menu_item::kArabicParts:
    case menu_item::kRhythm:
    case menu_item::kSecondary:
    case menu_item::kArcDirection:
    case menu_item::kPrimary:
    case menu_item::kSymbolicEquatorial:
    case menu_item::kSymbolicEcliptic:
    case menu_item::kTransits:
    case menu_item::kMundane:
    case menu_item::kRiseSet:
    case menu_item::kEclipses:
    case menu_item::kPlaceWander:
      return weiter + ende;
    // his CASE 36,49,52,59,63,66,68,81,101, the defaults, the
    // explanations and the double wheel
    case 36:
    case 49:
    case 52:
    case menu_item::kDoubleWheel:
    case 63:
    case 66:
    case 68:
    case 81:
    case 101:
      return {};
    default:
      return weiter;
  }
}

// his PUT of a page or a picture, a window of the given size over the
// main window until the caller lets it go
std::unique_ptr<QDialog> picture_view(QWidget* parent, const QString& title, const DisplayList& page,
                                      const QPixmap& shot) {
  auto view = std::make_unique<QDialog>(parent);
  view->setWindowTitle(title);
  auto* v = new QVBoxLayout(view.get());
  v->setContentsMargins(0, 0, 0, 0);
  if (!page.items.empty()) {
    auto* canvas = new WheelWidget(view.get());
    canvas->set_plain_list(page);
    v->addWidget(canvas);
  } else {
    auto* label = new QLabel(view.get());
    label->setAlignment(Qt::AlignCenter);
    label->setPixmap(shot);
    v->addWidget(label);
  }
  view->resize(parent->size());
  view->show();
  return view;
}

}  // namespace

// ported from pr_enabel, the entry of the EIN-AUSG. menu
void MainWindow::printer_option_entry() {
  konsta_.prenbl = konsta_.prenbl == 0 ? 1 : 0;
  // his a$ = "DRUCKER-OPTION für HORCOM :    " with "EIN ! " or "AUS ! "
  QMessageBox::information(this, "HORCOM",
                           konsta_.prenbl != 0 ? tr("DRUCKER-OPTION für HORCOM :    EIN ! ")
                                               : tr("DRUCKER-OPTION für HORCOM :    AUS ! "));
  // his @param_sp
  persist_konsta();
}

// his comp!, comb! and dppel!, a paired chart is on screen
bool MainWindow::pair_view() const {
  const bool composite = composite_action_ != nullptr && composite_action_->isChecked() && partner_chart_;
  const bool double_wheel = partner_chart_ && ((compare_action_ != nullptr && compare_action_->isChecked()) ||
                                               (dial_action_ != nullptr && dial_action_->isChecked()));
  return composite || double_wheel || !combin_name1_.empty();
}

// the gate of start_hardc
bool MainWindow::hardcopy_due(int item) const {
  // his IF prenbl& && ... && hardcop! = 0 && mehrf! = 0, then his excluded entries
  return konsta_.prenbl != 0 && !hardcopy_running_ && !double_active_ && hardcopy_item(item, pair_view());
}

// ported from hardc_kompl, start_hardc and hardcopy. The output keeps
// its picture while the box asks, JA prints it on half a page or on the
// full page and the output waits for a key before it goes on
void MainWindow::hardcopy_offer(QWidget* output, int item) {
  if (output == nullptr || !hardcopy_due(item)) {
    return;
  }
  hardcopy_running_ = true;
  if (ask_hardcopy(output)) {
    const bool half = konsta_.halbs != 0;
    QPrinter printer(QPrinter::HighResolution);
    if (!prepare_printer(output, printer, PrintPage::kHardcopy, half)) {
      printer_failed(output);
    } else {
      QPainter p(&printer);
      if (!p.isActive()) {
        printer_failed(output);
      } else {
        // his SWITCH halbs&, CASE 1 at gdxp& / 20,gdyp& / 90, CASE 0 at gdyp& / 90,gdxp& / 90 times 1.4
        const QRectF target = half ? robert_page_rect(printer, page_margin::kLeft, page_margin::kTop, 1.0)
                                   : robert_page_rect(printer, page_margin::kFull, page_margin::kFull,
                                                      page_margin::kFullFactor);
        if (output == this) {
          paint_robert_page(p, classic_export_list(), target);
        } else {
          paint_output(p, output, target);
        }
        p.end();
      }
    }
    // his _WIN$(WIN(WIN())) = _WIN$(WIN(WIN())) + "  |  WEITER mit LEERTASTE", KEYGET k%
    const QString title = output->windowTitle();
    output->setWindowTitle(title + tr("  |  WEITER mit LEERTASTE"));
    (void)wait_key(output == this ? wheel_ : output);
    output->setWindowTitle(title);
  }
  hardcopy_running_ = false;
}

// ported from wart with hardc_kompl at its end, the chart on the main
// window stays until a key, every key but ESC offers the HARDCOPY. The
// title carries the hints of his titlew while the chart waits and the
// function keys serve the output like his wart_gem
void MainWindow::wart(int item, bool more_follows) {
  // his menu screen gave way to the chart, the shell keeps its menu in
  // view. Without the DRUCKER-OPTION a wait before nothing but the
  // HARDCOPY only swallowed the next click on the menu, so the chart
  // simply stays like after every other output
  if (!more_follows && konsta_.prenbl == 0) {
    remember_picture(this);
    return;
  }
  const QString title = windowTitle();
  setWindowTitle(title + wart_hint(item));
  int key = 0;
  {
    const QScopedValueRollback<int> waiting(wart_item_, item);
    key = wait_key(wheel_);
  }
  setWindowTitle(title);
  // his scget, the chart screen stays as the last picture
  remember_picture(this);
  // his IF wartfg! = 0 AND expr! = 0 : @hardc_kompl, MOUSEK = 0
  if (key != 0 && key != Qt::Key_Escape) {
    hardcopy_offer(this, item);
  }
}

// ported from druck_graph_ein, the screen or a DRUCKER-GRAPHIK before
// the output, asked only while the DRUCKER-OPTION is on
int MainWindow::ask_graphic_output(int item, bool a4) {
  if (konsta_.prenbl == 0) {
    return kOutputScreen;
  }
  //RR AUSGABE auf BILDSCHIRM oder als DRUCKER-GRAPHIK ?
  QStringList info{tr("AUSGABE auf BILDSCHIRM oder als DRUCKER-GRAPHIK ?")};
  QStringList buttons{tr("BILDSCHIRM ( evtl. HARDCOPY )?"), tr("DRUCKER-GRAPHIK  DIN A5 ?")};
  if (a4) {
    // his IF muuu& = 53 : e$ = "( DRUCKER-OPTION UMSCHALTEN mit F8 aus MENÜ oder AUSGABEN ! )"
    info << (item == menu_item::kChartGraphic ? tr("( DRUCKER-OPTION UMSCHALTEN mit F8 aus MENÜ oder AUSGABEN ! )")
                                              : QString());
    buttons << tr("DRUCKER-GRAPHIK  DIN A4 ?");
  }
  const int r = ChoiceDialog::ask(this, tr("AUSWAHL"), info, buttons, 0);
  // his moda& = r&, no answer draws nothing
  return r < 0 ? kOutputNone : r + 1;
}

// the DRUCKER-GRAPHIK branches of a11, a12, a13 and a14, the page at
// 0.8 points per screen unit or the a11 page of DIN A4
bool MainWindow::print_graphic(const DisplayList& page, bool a4, double left_div) {
  if (page.items.empty()) {
    return false;
  }
  QPrinter printer(QPrinter::HighResolution);
  if (!prepare_printer(this, printer, a4 ? PrintPage::kGraphicA4 : PrintPage::kGraphicA5, konsta_.halbs != 0)) {
    printer_failed(this);
    return false;
  }
  QPainter p(&printer);
  if (!p.isActive()) {
    printer_failed(this);
    return false;
  }
  paint_robert_page(p, page,
                    a4 ? robert_a4_rect(printer) : robert_page_rect(printer, left_div, page_margin::kTop, 1.0));
  return p.end();
}

// the chart outputs of the menu with his druck_graph_ein around them,
// the chart already stands on the main window
void MainWindow::chart_output(int item, bool a4) {
  const int moda = ask_graphic_output(item, a4);
  if (moda == kOutputA5 || moda == kOutputA4) {
    const double left = item == menu_item::kChartGraphic ? page_margin::kLeftChart : page_margin::kLeft;
    (void)print_graphic(moda == kOutputA4 ? a4_export_list() : classic_export_list(), moda == kOutputA4, left);
    return;
  }
  if (moda != kOutputScreen) {
    return;
  }
  // his scget at the end of men3, the chart is the last picture whether
  // or not the DRUCKER-OPTION holds it for a key
  remember_picture(this);
  if (konsta_.prenbl != 0) {
    wart(item);
  }
}

// ported from druck_horm, the last MULTI or HARMONICS picture as a
// DRUCKER-GRAPHIK, false after ABBRUCH
bool MainWindow::multi_print_offer() {
  if (konsta_.prenbl == 0) {
    return true;
  }
  // his alertbox(2,"Letztes Bild als","DRUCKER-GRAPHIK in DIN A5 ausgeben ?","","",2,"JA"," NEIN ",ac$)
  const int r = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("Letztes Bild als"), tr("DRUCKER-GRAPHIK in DIN A5 ausgeben ?")},
                                  {tr("JA"), tr(" NEIN "), tr("ABBRUCH")}, 1);
  if (r == 0) {
    (void)print_graphic(classic_export_list(), false, page_margin::kLeft);
  }
  return r != 2;
}

// ported from mehrf_1, F9 or ALT + M in an output. The first press
// explains the double print and keeps the picture, the second keeps the
// second one, the third offers the print
void MainWindow::double_capture(QWidget* output) {
  const DisplayList page = output == this ? classic_export_list() : output_page(output);
  if (!double_active_) {
    // his m2$ read "ausdruckenm indem", the port prints the word he meant
    const int re = ChoiceDialog::ask(
        output, tr("ENTSCHEIDUNG !"),
        {tr("DOPPEL - DARSTELLUNG"), tr("Mittels der FUNKTIONS-TASTE  F9  ( oder 'ALT + M' ) Können Sie"),
         tr("2 Auswertungen auf einer DINA4-Seite ausdrucken, indem Sie diese mit F9 SPEICHERN,"),
         tr("dann aus dem Haupt-Menü wieder F9 drücken und ausdrucken.")},
        {tr("VORLIEGENDES BILD in DOPPEL-SPEICHER"), tr("ABBRUCH")}, 0);
    if (re != 0) {
      return;
    }
    double_active_ = true;
    double_buffer_.clear();
    double_buffer_.push_back(page.items.empty() ? DisplayList{} : page);
    double_pictures_.clear();
    double_pictures_.push_back(page.items.empty() ? output->grab() : QPixmap());
    return;
  }
  if (double_buffer_.size() >= 2) {
    // his IF wmehr& = 5 : @mehrf_aus_1
    double_offer();
    return;
  }
  double_buffer_.push_back(page.items.empty() ? DisplayList{} : page);
  double_pictures_.push_back(page.items.empty() ? output->grab() : QPixmap());
  // his @fanz("VORLIEGENDES " + STR$(wmehr& - 2) + ". BILD WURDE GESPEICHERT !")
  QMessageBox::information(output, "HORCOM", tr("VORLIEGENDES 2. BILD WURDE GESPEICHERT !"));
}

// ported from mehrf_aus and mehrf_aus_1, F9 where no output stands
void MainWindow::double_offer() {
  if (!double_active_) {
    //RR Kein DOPPELBILD vorhanden !
    QMessageBox::information(this, "HORCOM", tr("Kein DOPPELBILD vorhanden !"));
    return;
  }
  if (double_buffer_.size() < 2) {
    //RR Erst ein Bild gespeichert !
    QMessageBox::information(this, "HORCOM", tr("Erst ein Bild gespeichert !"));
    return;
  }
  // his m2$ = STR$(wmbanz&) + " EINZELBILDER VORHANDEN !"
  const int re = ChoiceDialog::ask(front_window(), tr("ENTSCHEIDUNG !"),
                                   {tr("2 EINZELBILDER VORHANDEN !"), tr("Beim AUSDRUCK werden ZWEI BILDER"),
                                    tr("auf GANZSEITE ( DIN A4 HOCHFORMAT ) GEDRUCKT !")},
                                   {tr("WEITER = DRUCKEN"), tr("ABBRUCH"), tr("DOPPELBILD - SPEICHER LÖSCHEN")}, 0);
  if (re == 0) {
    double_print_run();
  } else if (re == 2) {
    // his @mehrf_a_end
    double_clear();
  }
}

void MainWindow::double_clear() {
  double_buffer_.clear();
  double_pictures_.clear();
  double_active_ = false;
}

// ported from mehrf_a. The preview shows both pictures side by side at
// half size, the page stacks them on DIN A4 portrait at the size of his
// half page HARDCOPY
void MainWindow::double_print_run() {
  QDialog preview(this);
  // his TITLEW #7,"DOPPELBILD AUSDRUCKEN" and _WIN$(WIN(7)) = "WEITER mit LEERTASTE !"
  preview.setWindowTitle(tr("DOPPELBILD AUSDRUCKEN") + "  |  " + tr("WEITER mit LEERTASTE !"));
  auto* row = new QHBoxLayout(&preview);
  for (std::size_t i = 0; i < double_buffer_.size(); ++i) {
    if (!double_buffer_[i].items.empty()) {
      auto* canvas = new WheelWidget(&preview);
      canvas->set_plain_list(double_buffer_[i]);
      row->addWidget(canvas);
    } else {
      auto* label = new QLabel(&preview);
      label->setPixmap(double_pictures_[i].scaled(size() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation));
      row->addWidget(label);
    }
  }
  preview.resize(size());
  preview.show();
  QApplication::processEvents();
  const auto paint_one = [this](QPainter& p, std::size_t i, const QRectF& target) {
    if (!double_buffer_[i].items.empty()) {
      paint_robert_page(p, double_buffer_[i], target);
      return;
    }
    const QPixmap& shot = double_pictures_[i];
    const double s = std::min(target.width() / shot.width(), target.height() / shot.height());
    p.drawPixmap(QRectF(target.x(), target.y(), shot.width() * s, shot.height() * s), shot, QRectF(shot.rect()));
  };
  QPrinter printer(QPrinter::HighResolution);
  if (!prepare_printer(&preview, printer, PrintPage::kDouble, true)) {
    // his @dr_fehl, then @mehrf_a_ex clears the double memory
    printer_failed(&preview);
    double_clear();
    return;
  }
  {
    QPainter p(&printer);
    if (!p.isActive()) {
      printer_failed(&preview);
      double_clear();
      return;
    }
    // his o& = gdxp& / 20 and q& = gdyp& / 28, the second picture at 2 * q& + hh&
    const QRectF first = robert_page_rect(printer, page_margin::kLeft, page_margin::kDoubleTop, 1.0);
    const QRectF second(first.x(), 2.0 * first.y() + first.height(), first.width(), first.height());
    paint_one(p, 0, first);
    paint_one(p, 1, second);
    p.end();
  }
  // his asc& = @wart
  (void)wait_key(&preview);
  preview.hide();
  // his alertbox(1,m$ + " ?",...,"LÖSCHEN","BEHALTEN") with m$ = "DOPPEL-BILD LÖSCHEN "
  const int rl = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("DOPPEL-BILD LÖSCHEN  ?")}, {tr("LÖSCHEN"), tr("BEHALTEN")}, 0);
  if (rl == 0) {
    double_clear();
  }
}

// ported from scget, the output leaves its page behind for his
// LETZTES BILD
void MainWindow::remember_picture(QWidget* output) {
  last_title_ = output->windowTitle();
  if (output == this) {
    last_page_ = classic_export_list();
    last_shot_ = QPixmap();
    return;
  }
  last_page_ = output_page(output);
  last_shot_ = last_page_.items.empty() ? output->grab() : QPixmap();
}

void MainWindow::show_picture(const QString& title, const DisplayList& page, const QPixmap& shot) {
  const std::unique_ptr<QDialog> view = picture_view(this, title, page, shot);
  // his KEYGET k%
  (void)wait_key(view.get());
}

// ported from screen, LETZTES BILD ZEIGEN / bzw.SPEICHERN. The last output
// shows again until a key, then his box keeps it in the BILDER folder,
// loads one from there or deletes one. His BMP of the screen resolution
// with FORMAT.BLD becomes a PNG that any screen reads
void MainWindow::last_picture() {
  // his scrout!, STATISTIK and ZEIT-WANDERN lock the routine once
  if (scrout_) {
    scrout_ = false;
    //RR Nicht für diese Anwendung !
    QMessageBox::information(this, "HORCOM", tr("Nicht für diese Anwendung !"));
    return;
  }
  const bool shown = !last_page_.items.empty() || !last_shot_.isNull();
  if (shown) {
    show_picture(last_title_, last_page_, last_shot_);
  } else {
    //RR Noch kein Ausgabe - Bildschirm vorhanden !
    QMessageBox::information(this, "HORCOM", tr("Noch kein Ausgabe - Bildschirm vorhanden !"));
  }
  const QString dir = QString::fromStdWString((data_dir_ / "bilder").wstring());
  const auto pictures = [&dir]() { return QDir(dir).entryList({"*.png", "*.PNG"}, QDir::Files); };
  const auto pick = [this, &dir](const QString& title) {
    return QFileDialog::getOpenFileName(this, title, dir, tr("Bilder (*.png)"));
  };
  enum class Answer { kSave, kLoad, kDelete, kCancel };
  for (;;) {
    // his scre1, bilder! = EXIST(hrc$ + "\BILDER\*.BMP")
    const bool stored = !pictures().isEmpty();
    if (!stored && !shown) {
      return;
    }
    // the stored pictures stay reachable without a last output, only a
    // picture on hand can be saved
    QStringList info;
    QStringList buttons;
    std::vector<Answer> answers;
    if (shown) {
      info << tr("Bild in DATEI SPEICHERN ?");
      buttons << tr(" SPEICHERN ");
      answers.push_back(Answer::kSave);
    }
    if (stored) {
      info << tr("Bild aus DATEI LADEN ?") << tr("Bild LÖSCHEN")
           << tr("Gelegentliches LÖSCHEN im Ordner 'BILDER' NICHT VERGESSEN !");
      buttons << tr("LADEN") << tr("Bild LÖSCHEN");
      answers.push_back(Answer::kLoad);
      answers.push_back(Answer::kDelete);
    }
    buttons << tr("ABBRUCH");
    answers.push_back(Answer::kCancel);
    const int w = ChoiceDialog::ask(this, tr("AUSWAHL"), info, buttons, buttons.size() - 1);
    const Answer answer = w >= 0 && w < static_cast<int>(answers.size()) ? answers[static_cast<std::size_t>(w)]
                                                                         : Answer::kCancel;
    if (answer == Answer::kSave) {
      // his IF FGATTR(hrc$ + "\BILDER") < 0 : MKDIR hrc$ + "\BILDER"
      std::error_code ec;
      std::filesystem::create_directories(data_dir_ / "bilder", ec);
      for (;;) {
        QString name = QFileDialog::getSaveFileName(this, tr("Bild in DATEI SPEICHERN ?"), dir, tr("Bilder (*.png)"),
                                                    nullptr, QFileDialog::DontConfirmOverwrite);
        if (name.isEmpty()) {
          return;
        }
        if (!name.endsWith(".png", Qt::CaseInsensitive)) {
          name += ".png";
        }
        if (QFileInfo::exists(name)) {
          //RR Name existiert schon !
          QMessageBox::information(this, "HORCOM", tr("Name existiert schon !"));
          continue;
        }
        QPixmap image = last_shot_;
        if (!last_page_.items.empty()) {
          // his screen of 640 by 480 at twice the size for a sharp picture
          image = QPixmap(static_cast<int>(2.0 * last_page_.width), static_cast<int>(2.0 * last_page_.height));
          image.fill(Qt::white);
          QPainter p(&image);
          p.setRenderHint(QPainter::Antialiasing, true);
          paint_fitted(p, last_page_, QRectF(image.rect()));
        }
        if (!image.save(name, "PNG")) {
          QMessageBox::warning(this, "HORCOM", tr("Das Bild ließ sich nicht speichern."));
        }
        return;
      }
    }
    if (answer == Answer::kLoad) {
      const QString name = pick(tr("Bild aus DATEI LADEN ?"));
      if (!name.isEmpty()) {
        // his @bildld(0,im$), then @scget, the loaded picture is the last one
        last_page_ = DisplayList{};
        last_shot_ = QPixmap(name);
        last_title_ = QFileInfo(name).fileName();
        show_picture(last_title_, last_page_, last_shot_);
      }
      return;
    }
    if (answer == Answer::kDelete) {
      // his scre3, the picture shows while it goes and the next one is
      // asked while any remain, no key waits in between
      for (;;) {
        const QString name = pick(tr("Bild LÖSCHEN"));
        if (name.isEmpty()) {
          return;
        }
        const QString file = QFileInfo(name).fileName();
        const std::unique_ptr<QDialog> view = picture_view(this, file, DisplayList{}, QPixmap(name));
        QFile::remove(name);
        // his @fanz(im$ + " gelöscht !")
        QMessageBox::information(this, "HORCOM", file + tr(" gelöscht !"));
        if (pictures().isEmpty()) {
          break;
        }
      }
      continue;
    }
    return;
  }
}

}  // namespace horcom
