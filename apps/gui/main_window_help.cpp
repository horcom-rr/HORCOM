// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// His help and function keys. F1 on the main screen or the right mouse
// button opens the Erste Hilfe list, F1 in an output the ERLÄUTERUNG of
// the menu the output came from, the other keys of his legend work in
// the outputs as well. The Alt letters serve the main screen and the
// outputs wherever no button mnemonic competes with them.

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QTextStream>
#include <QVBoxLayout>

#include "choice_dialog.hpp"
#include "kommen_dialog.hpp"
#include "main_window.hpp"
#include "robert_input.hpp"
#include "theme.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

// the property that names the ERLÄUTERUNG of a menu entry
constexpr const char* kStemProperty = "kommStem";

// the window of his F2, a plain chart until ESC, F2 or ALT + A, the event
// filter of the main window serves the two keys
class ChartBetween final : public QDialog {
 public:
  using QDialog::QDialog;
};

}  // namespace

QWidget* MainWindow::front_window() {
  QWidget* modal = QApplication::activeModalWidget();
  return modal != nullptr ? modal : this;
}

// his wart_erl, the menu an entry belongs to decides the text, a few
// entries of AUSWERTUNG, DIVERSES and EPHEMERIDE point into their own.
// The captions match the entries as the menu tree carries them
void MainWindow::tag_help_stems(const std::vector<std::pair<QMenu*, QString>>& menus) {
  const std::vector<std::pair<QString, QString>> own{
      {tr("STATISTIK G/H…"), "kommstat"},
      {tr("MÜNCHNER RHYTHMENLEHRE…"), "komm6"},
      {tr("GRAD-DATUM-LISTE…"), "komm6"},
      {tr("DYNAMOGRAMM…"), "komm7"},
      {tr("SEKUNDÄR-DIREKTION…"), "komm7"},
      {tr("SONNE (MOND)-BOGEN-DIREKTION…"), "komm7"},
      {tr("PRIMÄR-DIREKTION ( E.C.KÜHR )…"), "komm7"},
      {tr("SYMB. DIREKTION: ÄQUATORIAL…"), "komm7"},
      {tr("SYMB. DIREKTION: EKLIPT. G/H…"), "komm7"},
      {tr("LINEAR-GRAPHIK…"), "komm7"},
      {tr("TRANSITE…"), "komm7"},
      {tr("MUNDAN-ASPEKTE…"), "komm7"},
      {tr("HÄUSER-SYSTEM…"), "komm8"},
      {tr("HÄUSER-TABELLE…"), "komm8"},
  };
  for (const auto& [menu, stem] : menus) {
    for (QAction* a : menu->actions()) {
      QString s = stem;
      for (const auto& [text, own_stem] : own) {
        if (a->text() == text) {
          s = own_stem;
        }
      }
      a->setProperty(kStemProperty, s);
    }
    connect(menu, &QMenu::triggered, this, [this](QAction* a) {
      const QString s = a->property(kStemProperty).toString();
      if (!s.isEmpty()) {
        help_stem_ = s;
      }
    });
  }
}

// ported from wart_erl, the ERLÄUTERUNG of the running output
void MainWindow::context_help() {
  QWidget* over = front_window();
  // a box that names its own ERLÄUTERUNG, the HARDCOPY box reads his ae2
  const QString own = over->property("horcomHelp").toString();
  KommenDialog dialog(data_dir_ / "kommen", own.isEmpty() ? help_stem_ : own, english_edition(), over);
  dialog.exec();
}

// ported from erste_hilfe, his list of ersthilf.int with the way back and
// the short manual. His HINWEISE lesen button read HINWEIS5, which holds
// only notes on the historical Windows installation and is not shipped
void MainWindow::erste_hilfe() {
  const std::filesystem::path dir = data_dir_ / "kommen";
  const std::filesystem::path en = dir / "ersthilf_en.txt";
  const std::filesystem::path file = english_edition() && std::filesystem::exists(en) ? en : dir / "ersthilf.txt";
  QStringList lines;
  QFile in(QString::fromStdWString(file.wstring()));
  if (in.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream ts(&in);
    ts.setEncoding(QStringConverter::Utf8);
    while (!ts.atEnd()) {
      lines << ts.readLine();
    }
  } else {
    //RR HILFE-Datei fehlt !
    QMessageBox::information(this, "HORCOM", tr("HILFE-Datei fehlt !"));
    return;
  }
  for (;;) {
    QDialog dialog(this);
    //RR Erste Hilfe und Einführung
    dialog.setWindowTitle(tr("Erste Hilfe und Einführung"));
    auto* v = new QVBoxLayout(&dialog);
    auto* list = new QListWidget(&dialog);
    list->setFont(theme::mono_font());
    for (const QString& line : lines) {
      list->addItem(" " + line);
    }
    v->addWidget(list, 1);
    auto* row = new QHBoxLayout();
    // his mer$ = "&Zurück zum HAUPT - MENÜ"
    auto* back = new QPushButton(tr("&Zurück zum HAUPT - MENÜ"), &dialog);
    back->setDefault(true);
    //RR KURZ-ANLEITUNG lesen
    auto* manual = new QPushButton(tr("KURZ-ANLEITUNG lesen"), &dialog);
    row->addWidget(back);
    row->addStretch(1);
    row->addWidget(manual);
    v->addLayout(row);
    bool read_manual = false;
    connect(back, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(manual, &QPushButton::clicked, &dialog, [&dialog, &read_manual]() {
      read_manual = true;
      dialog.accept();
    });
    dialog.resize(760, 640);
    dialog.exec();
    if (!read_manual) {
      return;
    }
    // his @ae12, then back into the list
    KommenDialog kommen(dir, "kurzanl5", english_edition(), this);
    kommen.exec();
  }
}

// ported from aendlist, the three texts behind ÄNDERUNGEN / HINWEISE /
// KURZANL. His HINWEISE für HORCOM7P mit WINDOWS read HINWEIS5, which is
// not shipped, the box offers the other two
void MainWindow::anmerkungen() {
  // his alertbox(1," ANMERKUNGEN zu HORCOM7P ",...,"ÄNDERUNGEN seit 1996",...,"KURZ-ANLEITUNG",ac$)
  const int ae = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr(" ANMERKUNGEN zu HORCOM ")},
                                   {tr("ÄNDERUNGEN seit 1996"), tr("KURZ-ANLEITUNG"), tr("ABBRUCH")}, 0);
  if (ae < 0 || ae == 2) {
    return;
  }
  KommenDialog dialog(data_dir_ / "kommen", ae == 0 ? "aendlist" : "kurzanl5", english_edition(), this);
  dialog.exec();
}

// ported from zeige_horoskop, the chart in between the outputs. His list
// of the outputs decides. The chart graphic of a derived chart and the
// returns and day charts show the radix at once, the other outputs of
// the list ask whether the derived chart or the radix shows, every other
// output ignores the key
void MainWindow::chart_between(int item) {
  if (!last_chart_ || dynamic_cast<ChartBetween*>(QApplication::activeModalWidget()) != nullptr) {
    return;
  }
  bool ask = false;
  switch (item) {
    case menu_item::kChartGraphic:
      // his CASE 53, the radix on screen needs no second look
      if (!active_is_solar_) {
        return;
      }
      break;
    case menu_item::kReturns:
    case menu_item::kDayChart:
      break;
    case menu_item::kPlanetCoordinates:
    case menu_item::kExtraCoordinates:
    case menu_item::kDegreeList:
    case menu_item::kFixedStars:
    case menu_item::kArabicParts:
    case menu_item::kAspektarium:
    case menu_item::kMidpointGraphic:
    case menu_item::kMulti:
    case menu_item::kRhythm:
    case menu_item::kSecondary:
    case menu_item::kArcDirection:
    case menu_item::kPrimary:
    case menu_item::kSymbolicEquatorial:
    case menu_item::kSymbolicEcliptic:
    case menu_item::kTransits:
    case menu_item::kMundane:
    case menu_item::kHouseTable:
      ask = active_is_solar_ && active_solar_ >= 0;
      break;
    default:
      return;
  }
  QWidget* over = front_window();
  QString label = QStringLiteral("RADIX");
  bool radix = true;
  if (ask) {
    label = solar_labels_[static_cast<std::size_t>(active_solar_)];
    // his alertbox(2,t$ + " oder RADIX zeigen ?",...,t$,"RADIX",ac$)
    const int rr = ChoiceDialog::ask(over, tr("AUSWAHL"), {tr("%1 oder RADIX zeigen ?").arg(label)},
                                     {label, tr("RADIX"), tr("ABBRUCH")}, 0);
    if (rr < 0 || rr == 2) {
      return;
    }
    radix = rr == 1;
  }
  ChartBetween box(over);
  const QString name = record_label_.trimmed();
  // his TITLEW #13," Nur zwischendurch das " + t$ + " anzeigen : " + na$ + " |   ZURÜCK mit 'ESC' !"
  box.setWindowTitle(tr(" Nur zwischendurch das %1 anzeigen : %2 |   ZURÜCK mit 'ESC' !")
                         .arg(radix ? QStringLiteral("RADIX") : label, name));
  auto* v = new QVBoxLayout(&box);
  v->setContentsMargins(0, 0, 0, 0);
  auto* wheel = new WheelWidget(&box);
  v->addWidget(wheel);
  if (radix && active_is_solar_) {
    const ChartSettings s = current_settings();
    const Chart c = radix_chart();
    const AspectResult a = scan_aspects(c, s, shown_aspect_settings());
    DisplayList dl = build_wheel(c, s, a, radix_wheel_options(c, s));
    add_corner_text(dl, sheet_text_for(record_, radix_input(), &c, s, record_.calendar), 8.0, kScreenSheetWidth / 2.0,
                    kScreenSheetWidth - 8.0);
    wheel->set_display_list(std::move(dl));
  } else {
    wheel->set_display_list(wheel_->display_list());
  }
  box.resize(760, 760);
  box.exec();
}

// ported from calc_in, the calculator of the system
void MainWindow::start_calculator() {
#ifdef Q_OS_WIN
  const QStringList programs{QStringLiteral("calc.exe")};
#else
  const QStringList programs{QStringLiteral("gnome-calculator"), QStringLiteral("kcalc"), QStringLiteral("galculator"),
                             QStringLiteral("mate-calc"), QStringLiteral("xcalc")};
#endif
  for (const QString& p : programs) {
    const QString found = QStandardPaths::findExecutable(p);
    if (!found.isEmpty() && QProcess::startDetached(found, {})) {
      return;
    }
  }
  // his @fanz(wind$ + "\CALC.EXE  FEHLT !")
  QMessageBox::information(this, "HORCOM", tr("RECHNER ( CALCULATOR ) FEHLT !"));
}

// ported from geohelio and a93, the switch with his message
void MainWindow::toggle_helio() {
  helio_->setChecked(!helio_->isChecked());
  // his fanz(he$ + " 'EIN' !") or " 'AUS' !"
  QMessageBox::information(this, "HORCOM",
                           helio_->isChecked() ? tr("Heliozentrisch 'EIN' !") : tr("Heliozentrisch 'AUS' !"));
}

// his F7 took the output into MSPAINT through the clipboard scget had
// filled, to be saved as a GIF. The port puts the window in front on the
// clipboard the same way and saves it as a PNG picture, by default in the
// BILDER folder his LETZTES BILD keeps
void MainWindow::save_output_picture() {
  QWidget* front = front_window();
  const QPixmap shot = front->grab();
  QApplication::clipboard()->setPixmap(shot);
  const std::filesystem::path dir = data_dir_ / "bilder";
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  const QString path =
      QFileDialog::getSaveFileName(front, tr("AUSGABE als BILD SPEICHERN"),
                                   QString::fromStdWString((dir / "horcom.png").wstring()), tr("PNG-Bild (*.png)"));
  if (path.isEmpty()) {
    return;
  }
  if (!shot.save(path, "PNG")) {
    QMessageBox::warning(front, "HORCOM", tr("Das Bild ließ sich nicht speichern."));
  }
}

namespace {

// the keys that left his wart, Tab, Return, Space, PgUp, PgDn, R, V, plus
// and minus, the statistics pages also W, S and H
bool is_wart_key(int key, int item) {
  switch (key) {
    case Qt::Key_Tab:
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_R:
    case Qt::Key_V:
    case Qt::Key_Plus:
    case Qt::Key_Minus:
      return true;
    case Qt::Key_W:
    case Qt::Key_S:
    case Qt::Key_H:
      return item == menu_item::kStatistics;
    default:
      return false;
  }
}

// the function key an Alt letter of his legend stands for, zero for
// letters outside it. ALT + H belongs to the main screen only, his
// outputs keep F6 for themselves
int alt_function_key(int key, bool main_screen) {
  switch (key) {
    case Qt::Key_E:
      return Qt::Key_F1;
    case Qt::Key_A:
      return Qt::Key_F2;
    case Qt::Key_C:
      return Qt::Key_F3;
    case Qt::Key_R:
      return Qt::Key_F5;
    case Qt::Key_H:
      return main_screen ? Qt::Key_F6 : 0;
    case Qt::Key_F:
      return Qt::Key_F7;
    case Qt::Key_D:
    case Qt::Key_Z:
      return Qt::Key_F8;
    case Qt::Key_M:
      return Qt::Key_F9;
    default:
      return 0;
  }
}

}  // namespace

// ported from druck_enbl_alt, the switch his param_sp keeps
void MainWindow::toggle_printer_option() {
  konsta_.prenbl = konsta_.prenbl != 0 ? 0 : 1;
  // his d$ = "DRUCKER-OPTION " with "AUS !" or "EIN !"
  QMessageBox::information(front_window(), "HORCOM",
                           konsta_.prenbl != 0 ? tr("DRUCKER-OPTION EIN !") : tr("DRUCKER-OPTION AUS !"));
  persist_konsta();
}

// his main loop, wart and wart_gem. The function keys reach every window,
// the Alt letters the main screen and the outputs, ESC on the main screen
// asks to quit
bool MainWindow::eventFilter(QObject* watched, QEvent* e) {
  // an output window of his menu leaves its page behind when it goes
  if (e->type() == QEvent::Hide && watched->isWidgetType()) {
    auto* w = static_cast<QWidget*>(watched);
    if (w->isWindow() && output_item(w) != 0) {
      remember_picture(w);
    }
  }
  if (e->type() != QEvent::KeyPress || QApplication::activePopupWidget() != nullptr) {
    return QMainWindow::eventFilter(watched, e);
  }
  const auto* k = static_cast<QKeyEvent*>(e);
  QWidget* modal = QApplication::activeModalWidget();
  // the keypad keys count like the main block, his wart read both
  const Qt::KeyboardModifiers mods = k->modifiers() & ~Qt::KeyboardModifiers(Qt::KeypadModifier);
  int key = k->key();
  // the chart in between closes on F2 or ALT + A like zeige_horoskop, ESC
  // stays with the box
  if (auto* between = dynamic_cast<ChartBetween*>(modal); between != nullptr) {
    if ((key == Qt::Key_F2 && mods == Qt::NoModifier) || (key == Qt::Key_A && mods == Qt::AltModifier)) {
      between->reject();
      return true;
    }
    return QMainWindow::eventFilter(watched, e);
  }
  // an output window of his menu, the wart keys offer the HARDCOPY before
  // the output acts on them, at the widget the key reaches first. The
  // boxes of the HARDCOPY change the filter list of the application while
  // this key is under way, so the key stops here and a copy of it goes on
  // to the output once the HARDCOPY is done. Only that copy passes the
  // gate, an output it opens offers its own HARDCOPY again
  const int item = output_item(modal);
  if (item != 0 && e != replayed_key_ && mods == Qt::NoModifier && !k->isAutoRepeat() && is_wart_key(key, item) &&
      hardcopy_due(item)) {
    QWidget* first = QApplication::focusWidget() != nullptr ? QApplication::focusWidget() : modal;
    if (watched == first) {
      QPointer<QObject> target = watched;
      QKeyEvent copy(k->type(), k->key(), k->modifiers(), k->text(), k->isAutoRepeat(), k->count());
      hardcopy_offer(modal, item);
      if (target) {
        const QScopedValueRollback<const QEvent*> replay(replayed_key_, &copy);
        QApplication::sendEvent(target, &copy);
      }
      return true;
    }
  }
  // the main window waits in wart over its chart, his wart_gem serves the
  // same keys there as in the outputs
  const bool main_wart = modal == nullptr && wart_item_ != 0;
  const bool main_screen = modal == nullptr && (isActiveWindow() || main_wart);
  if (mods == Qt::AltModifier) {
    if (item == 0 && !main_screen) {
      return QMainWindow::eventFilter(watched, e);
    }
    // his main loop read ALT + E as the EINFÜHRUNG, ae1
    if (key == Qt::Key_E && main_screen && !main_wart) {
      KommenDialog dialog(data_dir_ / "kommen", QStringLiteral("komm1"), english_edition(), this);
      dialog.exec();
      return true;
    }
    key = alt_function_key(key, main_screen && (!main_wart || wart_item_ == menu_item::kChartGraphic));
    if (key == 0) {
      return QMainWindow::eventFilter(watched, e);
    }
  } else if (mods != Qt::NoModifier) {
    return QMainWindow::eventFilter(watched, e);
  }
  // the reading dialogs keep their own keys
  if (dynamic_cast<KommenDialog*>(modal) != nullptr) {
    return QMainWindow::eventFilter(watched, e);
  }
  switch (key) {
    case Qt::Key_F1:
      if (modal != nullptr || main_wart) {
        context_help();
      } else if (isActiveWindow()) {
        erste_hilfe();
      } else {
        return QMainWindow::eventFilter(watched, e);
      }
      return true;
    case Qt::Key_F2:
      // his legend, F2 only in the outputs, not on the main screen
      if (modal == nullptr && !main_wart) {
        return QMainWindow::eventFilter(watched, e);
      }
      chart_between(modal != nullptr ? item : wart_item_);
      return true;
    case Qt::Key_F3:
      start_calculator();
      return true;
    case Qt::Key_F5:
      angle_converter(front_window());
      return true;
    case Qt::Key_F6:
      // his outputs keep F6 for themselves, the preview turns helio there,
      // a waiting chart only turns on the HOROSKOP - GRAPHIK like men3
      if (!main_screen || (main_wart && wart_item_ != menu_item::kChartGraphic)) {
        return QMainWindow::eventFilter(watched, e);
      }
      toggle_helio();
      return true;
    case Qt::Key_F7:
      save_output_picture();
      return true;
    case Qt::Key_F8:
      toggle_printer_option();
      return true;
    case Qt::Key_F9:
      // his mehrf_1 in the outputs, mehrf_aus in the main menu
      if (item != 0) {
        double_capture(modal);
        return true;
      }
      if (!main_screen) {
        return QMainWindow::eventFilter(watched, e);
      }
      // the main window is his chart screen too, a chart on it is kept
      if (last_chart_ && !(double_active_ && double_buffer_.size() >= 2)) {
        double_capture(this);
      } else {
        double_offer();
      }
      return true;
    case Qt::Key_Escape:
      if (!main_screen) {
        return QMainWindow::eventFilter(watched, e);
      }
      desktop_quit();
      return true;
    default:
      return QMainWindow::eventFilter(watched, e);
  }
}

// his MENU(4) = 2 on the passive main screen opens the Erste Hilfe. The
// wheel and the result docks keep the right button to themselves, a
// chart waiting in wart has no Erste Hilfe on that button either
void MainWindow::contextMenuEvent(QContextMenuEvent* event) {
  if (wart_item_ != 0) {
    event->ignore();
    return;
  }
  event->accept();
  erste_hilfe();
}

}  // namespace horcom
