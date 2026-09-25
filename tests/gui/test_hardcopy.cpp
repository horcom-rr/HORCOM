// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPrinter>
#include <QRegularExpression>
#include <QTimer>
#include <filesystem>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "event_filter.hpp"
#include "print_pages.hpp"
#include "probe.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a synthetic morning birth east of Greenwich, no real person
AafRecord print_birth() {
  AafRecord r;
  r.surname = "DRUCKFALL";
  r.day = 10;
  r.month = 5;
  r.year = 1970;
  r.hour = 7;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 35;
  return r;
}

// every print of a test lands in its own PDF instead of the system dialog
struct PrintFile {
  QString path;
  explicit PrintFile(const char* name) {
    path = QString::fromStdWString((std::filesystem::temp_directory_path() / name).wstring());
    QFile::remove(path);
    set_print_file(path);
  }
  ~PrintFile() {
    set_print_file(QString());
    // the capture rig keeps the pages for a look
    if (!qEnvironmentVariableIsSet("HORCOM_KEEP_PRINTS")) {
      QFile::remove(path);
    }
  }
  [[nodiscard]] int pages() const {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
      return 0;
    }
    const QByteArray pdf = f.readAll();
    return static_cast<int>(QString::fromLatin1(pdf).count(QRegularExpression("/Type\\s*/Page[^s]")));
  }
};

// every line and button of a box, for the wording checks
QString box_words(QDialog* d) {
  QString all = d->windowTitle() + "|";
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    all += l->text() + "|";
  }
  for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
    all += b->text() + "|";
  }
  if (auto* box = qobject_cast<QMessageBox*>(d)) {
    all += box->text();
  }
  return all;
}

void send_key(QWidget* to, int key) {
  QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier, key == Qt::Key_Space ? " " : QString());
  QApplication::sendEvent(to, &press);
}

}  // namespace

TEST_CASE("start_hardc offers the HARDCOPY only outside his excluded entries") {
  // the converters, the defaults and the explanations get none
  CHECK_FALSE(hardcopy_item(46, false));
  CHECK_FALSE(hardcopy_item(52, false));
  CHECK_FALSE(hardcopy_item(menu_item::kPlaceWander, false));
  CHECK_FALSE(hardcopy_item(101, false));
  // unless a composite, combin or double chart is up
  CHECK(hardcopy_item(52, true));
  CHECK(hardcopy_item(menu_item::kFixedStars, false));
  CHECK(hardcopy_item(menu_item::kReturns, false));
  CHECK(hardcopy_item(menu_item::kEclipses, false));
}

TEST_CASE("his page geometry puts 640 by 459 units at 0.8 points onto the paper") {
  PrintFile file("horcom_geometry.pdf");
  QPrinter printer(QPrinter::HighResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setOutputFileName(file.path);
  printer.setPageSize(QPageSize(QPageSize::A4));
  printer.setPageOrientation(QPageLayout::Portrait);
  const double px = printer.resolution() / 72.0;
  const QRectF page = printer.pageRect(QPrinter::DevicePixel);
  const QRectF half = robert_page_rect(printer, page_margin::kLeft, page_margin::kTop, 1.0);
  CHECK(half.x() == doctest::Approx(page.width() / 20.0));
  CHECK(half.y() == doctest::Approx(page.height() / 90.0));
  // 0.8 * 640 points, about 180.6 mm, and 0.8 * 459 points
  CHECK(half.width() == doctest::Approx(512.0 * px));
  CHECK(half.height() == doctest::Approx(367.2 * px));
  const QRectF full = robert_page_rect(printer, page_margin::kFull, page_margin::kFull, page_margin::kFullFactor);
  CHECK(full.width() == doctest::Approx(1.4 * 512.0 * px));
  // the a11 DIN A4 page, 640 by 980 units at 0.75 points
  const QRectF a4 = robert_a4_rect(printer);
  CHECK(a4.x() == doctest::Approx(page.width() / 14.0));
  CHECK(a4.height() == doctest::Approx(735.0 * px));
}

TEST_CASE("the text lists print 53 rows a page like datei_pr") {
  PrintFile file("horcom_rows.pdf");
  QPrinter printer(QPrinter::HighResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setOutputFileName(file.path);
  QStringList rows;
  for (int i = 0; i < 120; ++i) {
    rows << QString("ZEILE %1").arg(i + 1);
  }
  REQUIRE(print_text_rows(printer, rows));
  CHECK(file.pages() == 3);

  // his ninetieth of the width for short rows, a row of his record list
  // with some 112 characters narrows the pitch until it fits the margins
  CHECK(list_pitch(900.0, 40) == doctest::Approx(10.0));
  const double pitch = list_pitch(900.0, 112);
  CHECK(pitch < 10.0);
  CHECK(112.0 * pitch <= 900.0 - 2.0 * 900.0 / 20.0 + 1e-9);

  // his reader rows wrap at the word before the 80th column
  const QStringList wrapped = wrap_text_rows(QString(30, 'A') + " " + QString(30, 'B') + " " + QString(30, 'C') + "\nZWEI", kTextColumns);
  REQUIRE(wrapped.size() == 3);
  CHECK(wrapped[0] == QString(30, 'A') + " " + QString(30, 'B'));
  CHECK(wrapped[1] == QString(30, 'C'));
  CHECK(wrapped[2] == "ZWEI");
}

TEST_CASE("DRUCKER-OPTION EIN / AUS switches his prenbl and the overview shows it") {
  auto w = MainWindowProbe::make();
  QString said;
  {
    DialogDriver drive;
    drive.then([&said](QDialog* d) {
      said = box_words(d);
      d->accept();
    });
    MainWindowProbe::printer_option_entry(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(MainWindowProbe::konsta(*w).prenbl == 1);
  CHECK(said.contains("DRUCKER-OPTION für HORCOM :    EIN ! "));
  QString overview;
  {
    DialogDriver drive;
    drive.then([&overview](QDialog* d) {
      overview = box_words(d);
      d->reject();
    });
    MainWindowProbe::vorgaben_overview(*w);
  }
  CHECK(overview.contains("Drucker-Option EIN"));
  CHECK(overview.contains("Hardcopy : DIN A5"));
}

TEST_CASE("a page key on an output offers his HARDCOPY and prints the page") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, print_birth());
  MainWindowProbe::konsta(*w).prenbl = 1;
  MainWindowProbe::konsta(*w).halbs = 1;
  PrintFile file("horcom_hardcopy.pdf");
  QStringList seen;
  QString waiting;
  {
    DialogDriver drive;
    // EREIGNIS-Ort = GEBURTS-Ort ?, his profile computes topocentric
    drive.then(DialogDriver::click("JA"))
        .then(DialogDriver::fill({"2000"}, "OK"))
        .then([&seen](QDialog* d) {  // AUSGABE auf BILDSCHIRM oder als DRUCKER-GRAPHIK ?
          seen << box_words(d);
          DialogDriver::click("BILDSCHIRM")(d);
        })
        .then([](QDialog* d) {  // the SOLAR table, Space leaves it
          QTimer::singleShot(30, d, [d]() {
            send_key(QApplication::focusWidget() != nullptr ? QApplication::focusWidget() : d, Qt::Key_Space);
          });
        })
        .then([&seen](QDialog* d) {  //  HARDCOPY ?
          seen << box_words(d);
          DialogDriver::click("JA")(d);
        })
        .then([&seen](QDialog* d) {  //  DRUCKER BEREIT ?
          seen << box_words(d);
          d->accept();
        })
        .then([&seen](QDialog* d) {  // his setup box
          seen << box_words(d);
          DialogDriver::click("Weiter")(d);
        })
        .then([&waiting](QDialog* d) {  // the table again, KEYGET before the key goes on
          waiting = d->windowTitle();
          QTimer::singleShot(0, d, [d]() { send_key(d, Qt::Key_Space); });
        });
    MainWindowProbe::return_list(*w, false);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const QString all = seen.join("\n");
  CHECK(all.contains("AUSGABE auf BILDSCHIRM oder als DRUCKER-GRAPHIK ?"));
  CHECK(all.contains(" HARDCOPY ?"));
  CHECK(all.contains(" DRUCKER BEREIT ? "));
  CHECK(all.contains("HARDCOPY - FORMAT : DIN A5"));
  CHECK(all.contains("DIN A5 oder BENUTZERDEF. HALBSEITE  HOCHFORMAT ( bzw. 'PORTRÄT' ) "));
  CHECK(all.contains("EINSTELLEN und WARTEN bis DRUCKER ARBEITET !"));
  CHECK(waiting.endsWith("  |  WEITER mit LEERTASTE"));
  CHECK(file.pages() == 1);
}

TEST_CASE("HOROSKOP-GRAPHIK asks BILDSCHIRM or DRUCKER-GRAPHIK while the option is on") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, print_birth());
  MainWindowProbe::konsta(*w).prenbl = 1;
  PrintFile file("horcom_graphic.pdf");
  QStringList seen;
  {
    DialogDriver drive;
    drive
        .then([&seen](QDialog* d) {
          seen << box_words(d);
          DialogDriver::click("DIN A5")(d);
        })
        .then([](QDialog* d) { d->accept(); })
        .then([&seen](QDialog* d) {
          seen << box_words(d);
          DialogDriver::click("Weiter")(d);
        });
    MainWindowProbe::horoskop_graphik(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  REQUIRE(seen.size() == 2);
  CHECK(seen[0].contains("BILDSCHIRM ( evtl. HARDCOPY )?"));
  CHECK(seen[0].contains("DRUCKER-GRAPHIK  DIN A4 ?"));
  CHECK(seen[0].contains("( DRUCKER-OPTION UMSCHALTEN mit F8 aus MENÜ oder AUSGABEN ! )"));
  CHECK(seen[1].contains("DIN A4 oder BENUTZERDEF. HALBSEITE  HOCHFORMAT ( bzw. 'PORTRÄT' ) "));
  CHECK(file.pages() == 1);

  // with the option off the chart just stands, no box at all
  MainWindowProbe::konsta(*w).prenbl = 0;
  MainWindowProbe::konsta(*w).voll = false;
  DialogDriver quiet;
  MainWindowProbe::horoskop_graphik(*w);
  CHECK(quiet.unexpected() == 0);
}

TEST_CASE("F9 keeps two pictures and prints them stacked on one page") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, print_birth());
  PrintFile file("horcom_double.pdf");
  QString first;
  QString second;
  {
    DialogDriver drive;
    drive.then([&first](QDialog* d) {
      first = box_words(d);
      DialogDriver::click("VORLIEGENDES BILD in DOPPEL-SPEICHER")(d);
    });
    MainWindowProbe::double_capture(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(first.contains("DOPPEL - DARSTELLUNG"));
  CHECK(MainWindowProbe::double_active(*w));
  CHECK(MainWindowProbe::double_count(*w) == 1);
  {
    DialogDriver drive;
    drive.then([&second](QDialog* d) {
      second = box_words(d);
      d->accept();
    });
    MainWindowProbe::double_capture(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(second.contains("VORLIEGENDES 2. BILD WURDE GESPEICHERT !"));
  CHECK(MainWindowProbe::double_count(*w) == 2);
  QString offer;
  {
    DialogDriver drive;
    MainWindow* main = w.get();
    drive
        .then([&offer](QDialog* d) {
          offer = box_words(d);
          DialogDriver::click("WEITER = DRUCKEN")(d);
        })
        .then([](QDialog* d) { d->accept(); })
        .then([main](QDialog* d) {
          // his wart over the preview after the print
          QTimer::singleShot(200, main, [main]() { send_key(main, Qt::Key_Space); });
          DialogDriver::click("Weiter")(d);
        })
        .then(DialogDriver::click("LÖSCHEN"));
    MainWindowProbe::double_capture(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(offer.contains("2 EINZELBILDER VORHANDEN !"));
  CHECK(offer.contains("auf GANZSEITE ( DIN A4 HOCHFORMAT ) GEDRUCKT !"));
  CHECK(file.pages() == 1);
  CHECK_FALSE(MainWindowProbe::double_active(*w));
  CHECK(MainWindowProbe::double_count(*w) == 0);
}

TEST_CASE("LETZTES BILD shows the last output and keeps it in the BILDER folder") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, print_birth());
  const std::filesystem::path dir = std::filesystem::temp_directory_path() / "horcom_bilder_test";
  std::error_code ec;
  std::filesystem::remove_all(dir, ec);
  std::filesystem::create_directories(dir);
  MainWindowProbe::set_data_dir(*w, dir);
  MainWindow* main = w.get();
  const auto key_later = [main]() {
    QTimer::singleShot(250, main, [main]() { send_key(main, Qt::Key_Space); });
  };
  QString none;
  {
    DialogDriver drive;
    drive.then([&none](QDialog* d) {
      none = box_words(d);
      d->accept();
    });
    MainWindowProbe::last_picture(*w);
  }
  CHECK(none.contains("Noch kein Ausgabe - Bildschirm vorhanden !"));
  {
    // a SOLAR table leaves its page behind when it goes
    DialogDriver drive;
    drive.then(DialogDriver::click("JA")).then(DialogDriver::fill({"2000"}, "OK")).then([](QDialog* d) { d->reject(); });
    MainWindowProbe::return_list(*w, false);
    CHECK(drive.pending() == 0);
  }
  const auto name_step = [](const QString& file) {
    return [file](QDialog* d) {
      if (auto* name = d->findChild<QLineEdit*>("fileNameEdit")) {
        name->setText(file);
      }
      d->accept();
    };
  };
  QString box;
  {
    DialogDriver drive;
    drive
        .then([&box](QDialog* d) {
          box = box_words(d);
          DialogDriver::click(" SPEICHERN ")(d);
        })
        .then(name_step(QString::fromStdWString((dir / "bilder" / "TABELLE.png").wstring())));
    key_later();
    MainWindowProbe::last_picture(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(box.contains("Bild in DATEI SPEICHERN ?"));
  CHECK_FALSE(box.contains("LADEN"));
  REQUIRE(std::filesystem::exists(dir / "bilder" / "TABELLE.png"));
  QString second;
  QString gone;
  {
    DialogDriver drive;
    drive
        .then([&second](QDialog* d) {
          second = box_words(d);
          DialogDriver::click("Bild LÖSCHEN")(d);
        })
        // his scre3 shows the picture while it goes, no key waits
        .then(name_step(QString::fromStdWString((dir / "bilder" / "TABELLE.png").wstring())))
        .then([&gone](QDialog* d) {
          gone = box_words(d);
          d->accept();
        })
        .then(DialogDriver::click("ABBRUCH"));
    key_later();
    MainWindowProbe::last_picture(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(second.contains("Bild aus DATEI LADEN ?"));
  CHECK(second.contains("LADEN"));
  CHECK(gone.contains("TABELLE.png gelöscht !"));
  CHECK_FALSE(std::filesystem::exists(dir / "bilder" / "TABELLE.png"));
  std::filesystem::remove_all(dir, ec);
}

namespace {

// a fresh BILDER world in the temp folder, gone after the test
struct PictureDir {
  std::filesystem::path dir;
  explicit PictureDir(const char* name) : dir(std::filesystem::temp_directory_path() / name) {
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir / "bilder");
  }
  ~PictureDir() {
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
  }
};

// the entry of a top menu by its caption
QAction* menu_action(QMainWindow& w, const QString& menu, const QString& entry) {
  for (QAction* top : w.menuBar()->actions()) {
    if (top->text() == menu && top->menu() != nullptr) {
      for (QAction* a : top->menu()->actions()) {
        if (a->text() == entry) {
          return a;
        }
      }
    }
  }
  return nullptr;
}

}  // namespace

TEST_CASE("HOROSKOP-GRAPHIK leaves the last picture with the DRUCKER-OPTION off") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, print_birth());
  MainWindowProbe::konsta(*w).voll = false;
  PictureDir pics("horcom_bilder_chart");
  MainWindowProbe::set_data_dir(*w, pics.dir);
  MainWindowProbe::horoskop_graphik(*w);
  MainWindow* main = w.get();
  QTimer::singleShot(250, main, [main]() { send_key(main, Qt::Key_Space); });
  QString box;
  DialogDriver drive;
  drive.then([&box](QDialog* d) {
    box = box_words(d);
    DialogDriver::click("ABBRUCH")(d);
  });
  MainWindowProbe::last_picture(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  // his scget ran in men3, the chart is the picture to keep
  CHECK(box.contains("Bild in DATEI SPEICHERN ?"));
  CHECK_FALSE(box.contains("Noch kein Ausgabe"));
}

TEST_CASE("LETZTES BILD offers the stored pictures without a last output") {
  auto w = MainWindowProbe::make();
  PictureDir pics("horcom_bilder_stored");
  QPixmap one(8, 8);
  one.fill(Qt::red);
  REQUIRE(one.save(QString::fromStdWString((pics.dir / "bilder" / "ALT.png").wstring()), "PNG"));
  MainWindowProbe::set_data_dir(*w, pics.dir);
  QString none;
  QString box;
  DialogDriver drive;
  drive
      .then([&none](QDialog* d) {
        none = box_words(d);
        d->accept();
      })
      .then([&box](QDialog* d) {
        box = box_words(d);
        DialogDriver::click("ABBRUCH")(d);
      });
  MainWindowProbe::last_picture(*w);
  CHECK(drive.pending() == 0);
  CHECK(none.contains("Noch kein Ausgabe - Bildschirm vorhanden !"));
  // his scre1 goes on to the box while BILDER holds pictures, only the
  // SPEICHERN row needs a picture on hand
  CHECK(box.contains("LADEN"));
  CHECK(box.contains("Bild LÖSCHEN"));
  CHECK_FALSE(box.contains(" SPEICHERN "));
}

TEST_CASE("LETZTES BILD refuses once after STATISTIK like his scrout!") {
  auto w = MainWindowProbe::make();
  PictureDir pics("horcom_bilder_scrout");
  MainWindowProbe::set_data_dir(*w, pics.dir);
  QAction* statistik = menu_action(*w, "&EPHEMERIDE", "STATISTIK G/H…");
  REQUIRE(statistik != nullptr);
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ABBRUCH"));
    statistik->trigger();
    CHECK(drive.pending() == 0);
  }
  CHECK(MainWindowProbe::scrout(*w));
  QString refused;
  {
    DialogDriver drive;
    drive.then([&refused](QDialog* d) {
      refused = box_words(d);
      d->accept();
    });
    MainWindowProbe::last_picture(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // his @fanz of the locked screen routine
  CHECK(refused.contains("Nicht für diese Anwendung !"));
  CHECK_FALSE(MainWindowProbe::scrout(*w));
}

TEST_CASE("the chart of the main window waits with his hints and no shortcut runs meanwhile") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, print_birth());
  MainWindowProbe::konsta(*w).prenbl = 1;
  MainWindowProbe::konsta(*w).voll = false;
  MainWindow* main = w.get();
  const QString plain_title = main->windowTitle();
  QString waiting;
  bool shortcut_held = false;
  QString help;
  // acts once the title shows that the chart waits
  QTimer watch;
  watch.setInterval(20);
  QObject::connect(&watch, &QTimer::timeout, main, [main, &watch, &plain_title, &waiting, &shortcut_held]() {
    if (main->windowTitle() == plain_title || QApplication::activeModalWidget() != nullptr) {
      return;
    }
    watch.stop();
    waiting = main->windowTitle();
    // a menu shortcut arrives as a plain key while the chart waits
    QKeyEvent over(QEvent::ShortcutOverride, Qt::Key_O, Qt::ControlModifier);
    QApplication::sendEvent(main, &over);
    shortcut_held = over.isAccepted();
    // F1 opens the ERLÄUTERUNG of wart_erl, not the Erste Hilfe
    send_key(main, Qt::Key_F1);
    // ESC ends the wait without the HARDCOPY
    send_key(main, Qt::Key_Escape);
  });
  DialogDriver drive;
  drive
      .then([&watch](QDialog* d) {
        // AUSGABE auf BILDSCHIRM oder als DRUCKER-GRAPHIK ?, then the wart
        watch.start();
        DialogDriver::click("BILDSCHIRM")(d);
      })
      .then([&help](QDialog* d) {
        help = d->windowTitle();
        d->reject();
      });
  MainWindowProbe::horoskop_graphik(*w);
  INFO(drive.titles().join(" | ").toStdString());
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  // his titlew for the chart graphic, the right button and ESC named
  CHECK(waiting == plain_title + " | EINZELNE Planeten HERVORHEBEN : RECHTE Maustaste | ENDE mit 'ESC' !");
  CHECK(shortcut_held);
  CHECK(help == "Text-Datei lesen");
  CHECK(main->windowTitle() == plain_title);
}

TEST_CASE("an output the replayed key opens offers its own HARDCOPY, keypad keys count") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, print_birth());
  MainWindowProbe::konsta(*w).prenbl = 1;
  // an output with a focusable canvas, the keys reach the dialog from it
  const auto focus_host = [](QDialog& d) {
    auto* canvas = new QWidget(&d);
    canvas->setFocusPolicy(Qt::StrongFocus);
    canvas->setFocus();
  };
  QDialog outer(w.get());
  mark_output(&outer, menu_item::kFixedStars);
  focus_host(outer);
  bool inner_ran = false;
  // the Enter of the outer output opens a second output, his a nested
  // screen of the same kind, its Space closes it again
  LambdaFilter opener([&outer, &inner_ran, &focus_host](QEvent* e) {
    if (e->type() != QEvent::KeyPress || static_cast<QKeyEvent*>(e)->key() != Qt::Key_Enter) {
      return false;
    }
    QDialog inner(&outer);
    mark_output(&inner, menu_item::kFixedStars);
    focus_host(inner);
    LambdaFilter closer(LambdaFilter::keys([&inner](int key) {
      if (key == Qt::Key_Space) {
        inner.accept();
        return true;
      }
      return false;
    }));
    inner.installEventFilter(&closer);
    // a lost key never leaves the test hanging
    QTimer::singleShot(5000, &inner, [&inner]() { inner.reject(); });
    inner_ran = true;
    inner.exec();
    outer.accept();
    return true;
  });
  outer.installEventFilter(&opener);
  QTimer::singleShot(10000, &outer, [&outer]() { outer.reject(); });
  const auto key_to = [](int key, Qt::KeyboardModifiers mods) {
    return [key, mods](QDialog* d) {
      QTimer::singleShot(0, d, [d, key, mods]() {
        QWidget* to = QApplication::focusWidget() != nullptr ? QApplication::focusWidget() : d;
        QKeyEvent press(QEvent::KeyPress, key, mods);
        QApplication::sendEvent(to, &press);
      });
    };
  };
  int hardcopies = 0;
  const auto no_hardcopy = [&hardcopies](QDialog* d) {
    ++hardcopies;
    DialogDriver::click("NEIN")(d);
  };
  DialogDriver drive;
  drive.then(key_to(Qt::Key_Enter, Qt::KeypadModifier))
      .then(no_hardcopy)
      .then(key_to(Qt::Key_Space, Qt::NoModifier))
      .then(no_hardcopy);
  outer.exec();
  INFO(drive.titles().join(" | ").toStdString());
  CHECK(inner_ran);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(hardcopies == 2);
}
