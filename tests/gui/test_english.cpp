// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QTableWidget>
#include <QTimer>
#include <QTranslator>
#include <filesystem>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/data/chart_file.hpp"
#include "kommen_dialog.hpp"
#include "print_pages.hpp"
#include "probe.hpp"
#include "record_mask_dialog.hpp"
#include "wheel_widget.hpp"
#include "zeitzon_dialog.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

#ifdef HORCOM_TEST_QM

namespace {

// the English edition of the shell for one test, removed afterwards
struct English {
  QTranslator translator;
  bool loaded = false;
  English() {
    loaded = translator.load(QStringLiteral(HORCOM_TEST_QM));
    if (loaded) {
      QApplication::installTranslator(&translator);
    }
    qApp->setProperty(kEnglishEditionProperty, true);
  }
  ~English() {
    if (loaded) {
      QApplication::removeTranslator(&translator);
    }
    qApp->setProperty(kEnglishEditionProperty, false);
  }
};

// every button caption and label of a box, for the language checks
QString box_text(QDialog* d) {
  QString all = d->windowTitle() + "|";
  for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
    all += b->text() + "|";
  }
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    all += l->text() + "|";
  }
  if (auto* box = qobject_cast<QMessageBox*>(d)) {
    all += box->text();
  }
  return all;
}

}  // namespace

TEST_CASE("the NEW ENTRY flow runs in the English edition") {
  English en;
  REQUIRE(en.loaded);
  const auto dir = std::filesystem::temp_directory_path() / "horcom_english_entry";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  const QString file = QString::fromStdWString((dir / "ENGLISH.DAT").wstring());
  auto w = MainWindowProbe::make();
  QStringList seen;
  {
    DialogDriver drive;
    drive
        .then([&seen](QDialog* d) {
          seen << box_text(d);
          const QList<QLineEdit*> edits = d->findChildren<QLineEdit*>();
          const QStringList values = {"ENGLISH CASE", "TESTPLACE", "E", "11", "34", "0", "N", "48", "8", "0",
                                      "",             "7",         "7", "1970", "12", "30", "0", ""};
          for (int i = 0; i < values.size() && i < edits.size(); ++i) {
            edits[i]->setText(values[i]);
          }
          DialogDriver::click("OK")(d);
        })
        .then([&seen](QDialog* d) {
          seen << box_text(d);
          DialogDriver::click("SAVE")(d);
        })
        .then([file](QDialog* d) {
          auto* fd = qobject_cast<QFileDialog*>(d);
          REQUIRE(fd != nullptr);
          const QFileInfo info(file);
          fd->setDirectory(info.absolutePath());
          if (auto* name = fd->findChild<QLineEdit*>("fileNameEdit")) {
            name->setText(info.fileName());
          }
          static_cast<QDialog*>(fd)->accept();
        })
        .then([&seen](QDialog* d) {
          seen << box_text(d);
          d->accept();
        });
    MainWindowProbe::new_entry(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    REQUIRE(!drive.titles().isEmpty());
    CHECK(drive.titles().front() == "NEW ENTRY of RECORDS  :   |  RADIX NO.1");
  }
  REQUIRE(seen.size() == 3);
  CHECK(seen[0].contains("CET and OTHER ZONE TIMES"));
  CHECK(seen[0].contains("LOCAL TIME (HISTORICAL CHARTS)"));
  CHECK(seen[0].contains("READ TIME DETERMINATIONS"));
  CHECK(seen[0].contains("YYYY"));
  CHECK(seen[1].contains("SAVE the record ?"));
  CHECK(seen[1].contains("ONLY TAKE OVER as a record"));
  CHECK(seen[2].contains("NEW DATA FILE ENGLISH.DAT !"));
  const auto records = read_chart_file(std::filesystem::path(file.toStdWString()));
  REQUIRE(records.has_value());
  CHECK(records->size() == 1);
  std::filesystem::remove_all(dir);
}

TEST_CASE("the place menu and the zone box speak English") {
  English en;
  REQUIRE(en.loaded);
  const auto dir = std::filesystem::temp_directory_path() / "horcom_english_places";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  RecordMaskDialog mask(AafRecord{}, "NEW ENTRY", RecordMaskDialog::Mode::kEntry, dir);
  QString menu;
  {
    DialogDriver drive;
    drive.then([&menu](QDialog* d) {
      menu = box_text(d);
      DialogDriver::click("UNDO")(d);
    });
    QFocusEvent ev(QEvent::FocusIn, Qt::TabFocusReason);
    QApplication::sendEvent(mask.findChildren<QLineEdit*>()[1], &ev);
    QApplication::processEvents();
    CHECK(drive.pending() == 0);
  }
  CHECK(menu.startsWith("ENTER !|"));
  CHECK(menu.contains(" PREFERRED PLACE  EMPTY ! ENTER A NEW ONE !"));
  CHECK(menu.contains("PLACE FILES : FETCH - ENTER - DELETE"));
  ZeitzonDialog zone(HORCOM_TEST_DATA_DIR);
  const QString text = box_text(&zone);
  CHECK(text.contains("SINGLE SUMMER TIME = DST"));
  CHECK(text.contains("EUROPE :"));
  std::filesystem::remove_all(dir);
}

TEST_CASE("RESET EVERYTHING asks its question in English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  bool restarted = false;
  MainWindowProbe::on_restart(*w, [&restarted]() { restarted = true; });
  QString text;
  {
    DialogDriver drive;
    drive.then([&text](QDialog* d) {
      text = box_text(d);
      d->reject();
    });
    MainWindowProbe::full_reset(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(text.startsWith("RESET EVERYTHING|"));
  CHECK(text.contains("RESET HORCOM to the STATE of its FIRST START ?"));
  CHECK(text.contains("Records, places and own files are kept."));
  CHECK(text.contains("RESET and RESTART"));
  // the box closed without an answer changes nothing
  CHECK_FALSE(restarted);
}

TEST_CASE("the TRANSITS questions and the object chooser speak English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  QString first;
  {
    DialogDriver drive;
    drive.then([&first](QDialog* d) {
      first = box_text(d);
      d->reject();
    });
    MainWindowProbe::transit_list(*w);
  }
  CHECK(first.contains("CHOOSE the BASE ASPECT!"));
  CHECK(first.contains("360° = 0° = CONJUNCTION"));
}

TEST_CASE("the MUNICH RHYTHM THEORY chain and its table speak English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  AafRecord r;
  r.surname = "ENGLISH CASE";
  r.day = 10;
  r.month = 5;
  r.year = 1970;
  r.hour = 7;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 35;
  MainWindowProbe::apply(*w, r);
  QStringList boxes;
  QString heading;
  QString art;
  {
    DialogDriver drive;
    const auto answer = [&boxes](const QString& caption) {
      return [&boxes, caption](QDialog* d) {
        boxes << box_text(d);
        DialogDriver::click(caption)(d);
      };
    };
    drive.then(answer("TRIGGER TABLE"))
        .then(answer("LEFT"))
        .then(answer("DATE"))
        .then(answer("ONE"))
        .then(answer("YEAR"))
        .then(answer("SEVEN"))
        .then(answer("NO SPECIAL POINT"))
        .then([&](QDialog* d) {
          heading = box_text(d);
          const auto* t = d->findChild<QTableWidget*>();
          for (int i = 0; t != nullptr && i < t->rowCount(); ++i) {
            if (t->item(i, 0) != nullptr) {
              art += t->item(i, 0)->text() + "|";
            }
          }
          d->reject();
        });
    MainWindowProbe::rhythm(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const QString all = boxes.join(QChar(0x0A));
  CHECK(all.contains("EVALUATION MODE"));
  CHECK(all.contains("MARK the SENSE of DIRECTION !"));
  CHECK(all.contains("Triggers after W.DÖBEREINER"));
  CHECK(all.contains(" Period PER HOUSE ? "));
  CHECK(all.contains("SPECIAL POINT ( FIXED POINT ) CHOOSE ?"));
  CHECK_FALSE(all.contains("AUSWERTE"));
  CHECK_FALSE(all.contains("RICHTUNGSSINN"));
  CHECK(heading.contains("Trigger after W.DÖBEREINER |Direction:LEFT  |Period: 7 Years"));
  CHECK(art.contains("PHASE 1 = H 1"));
}

TEST_CASE("the CHANGE CHART DEFAULTS wizard speaks English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  AafRecord r;
  r.surname = "ENGLISH CASE";
  r.day = 3;
  r.month = 2;
  r.year = 1984;
  r.hour = 20;
  r.zone = "00hE00:00";
  r.lat_deg = 53;
  r.lat_min = 33;
  r.lon_deg = 10;
  r.lon_min = 0;
  MainWindowProbe::apply(*w, r);
  QStringList boxes;
  {
    DialogDriver drive;
    const auto answer = [&boxes](const QString& caption) {
      return [&boxes, caption](QDialog* d) {
        boxes << box_text(d);
        DialogDriver::click(caption)(d);
      };
    };
    drive.then(answer("BEGINNING of the chart"))
        .then(answer("0 ARIES"))
        .then(answer("OLD"))
        .then(answer("EXIT"));
    MainWindowProbe::vorgaben_horoskop(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const QString all = boxes.join(QChar(0x0A));
  CHECK(all.contains("CLICK the WANTED TOPIC"));
  CHECK(all.contains("BEGINNING of the CHART ?"));
  CHECK(all.contains("ASSIGNMENT of SIGN RULERS ?"));
  CHECK(all.contains("COLOURS in the CHART RING and HISTOGRAMS ?"));
  CHECK_FALSE(all.contains("BEGINN HOROSKOP"));
  CHECK_FALSE(all.contains("HERRSCHER"));
  CHECK(MainWindowProbe::konsta(*w).begz == 3);
  CHECK(MainWindowProbe::alt_rulers(*w));
}

TEST_CASE("the COMPOSITE session speaks English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  AafRecord r;
  r.surname = "ENGLISH CASE";
  r.day = 3;
  r.month = 2;
  r.year = 1984;
  r.hour = 20;
  r.zone = "00hE00:00";
  r.lat_deg = 53;
  r.lat_min = 33;
  r.lon_deg = 10;
  MainWindowProbe::put_slot(*w, 0, r);
  r.surname = "SECOND CASE";
  r.year = 1990;
  MainWindowProbe::put_slot(*w, 1, r);
  QStringList boxes;
  {
    DialogDriver drive;
    const auto answer = [&boxes](const QString& caption) {
      return [&boxes, caption](QDialog* d) {
        boxes << box_text(d);
        DialogDriver::click(caption)(d);
      };
    };
    drive.then(answer("after ROBERT HAND"))
        .then(answer("SATZ1"))
        .then(answer("SATZ2"))
        .then([&boxes](QDialog* d) {
          boxes << box_text(d);
          DialogDriver::fill({"", "RESIDENCE", "E", "13", "24", "0", "N", "52", "31", "0"}, "OK")(d);
        });
    MainWindowProbe::composite_session(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const QString all = boxes.join(QChar(0x0A));
  CHECK(all.contains("COMPOSITE CHART MODE ?"));
  CHECK(all.contains("WHICH HOUSE SYSTEM ?"));
  CHECK(all.contains("CLICK 2 RECORDS ONE AFTER THE OTHER !"));
  CHECK(all.contains("Activate the record for PARTNER 2 !"));
  CHECK(all.contains("COMPOSITE NO.1"));
  CHECK_FALSE(all.contains("HÄUSER-SYSTEM"));
  CHECK_FALSE(all.contains("aktivieren"));
  CHECK(MainWindowProbe::composite_on(*w));
  CHECK(MainWindowProbe::sheet(*w).place == "RESIDENCE");
}

TEST_CASE("the COMBINE session speaks English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  AafRecord r;
  r.surname = "FIRST CASE";
  r.day = 3;
  r.month = 2;
  r.year = 1984;
  r.hour = 20;
  r.zone = "00hE00:00";
  r.lat_deg = 53;
  r.lon_deg = 10;
  for (int i = 0; i < 3; ++i) {
    r.year = 1984 + i;
    MainWindowProbe::put_slot(*w, i, r);
  }
  QStringList boxes;
  {
    DialogDriver drive;
    const auto answer = [&boxes](const QString& caption) {
      return [&boxes, caption](QDialog* d) {
        boxes << box_text(d);
        DialogDriver::click(caption)(d);
      };
    };
    drive.then(answer("SATZ1")).then(answer("SATZ2")).then(answer("OUTPUT"));
    MainWindowProbe::combin_chart(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const QString all = boxes.join(QChar(0x0A));
  CHECK(all.contains("CLICK THE RECORDS ( AT LEAST 2, AT MOST 5 ) ONE AFTER THE OTHER !"));
  CHECK(all.contains("FETCH record 3 ?"));
  CHECK(all.contains("OUTPUT ?"));
  CHECK_FALSE(all.contains("HOLEN"));
  CHECK(MainWindowProbe::sheet(*w).place == "COMBIN-ORT");
}

TEST_CASE("the STATISTICS hub speaks English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  const std::filesystem::path dir = std::filesystem::temp_directory_path() / "horcom_statist_english";
  std::error_code ec;
  std::filesystem::remove_all(dir, ec);
  std::filesystem::create_directories(dir);
  MainWindowProbe::set_data_dir(*w, dir);
  QString hub;
  {
    DialogDriver drive;
    drive.then([&hub](QDialog* d) {
      hub = box_text(d);
      DialogDriver::click("CANCEL")(d);
    });
    MainWindowProbe::statistics_hub(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(hub.contains("* WHAT DO YOU WANT TO DO ? *"));
  CHECK(hub.contains("CREATE an EVALUABLE FILE ?"));
  CHECK(hub.contains("START the EVALUATION"));
  CHECK_FALSE(hub.contains("AUSWERTUNG"));
}

namespace {

// a synthetic morning birth east of Greenwich, no real person
AafRecord english_birth() {
  AafRecord r;
  r.surname = "ENGLISH CASE";
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

}  // namespace

TEST_CASE("the TERRAR and its table speak English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, english_birth());
  MainWindowProbe::helio(*w, true);
  QStringList boxes;
  {
    DialogDriver drive;
    MainWindow* main = w.get();
    drive
        .then([main, &boxes](QDialog* d) {
          boxes << box_text(d);
          QTimer::singleShot(400, main, [main]() {
            QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
            QApplication::sendEvent(main, &e);
          });
          DialogDriver::fill({"2020"}, "OK")(d);
        })
        .then([&boxes](QDialog* d) {
          boxes << box_text(d);
          DialogDriver::click("Continue ( = END )")(d);
        });
    MainWindowProbe::solar(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const QString all = boxes.join(QChar(0x0A));
  CHECK(all.contains("ENTER the WANTED CALENDAR YEAR 'YYYY' !"));
  CHECK(all.contains("SEARCH a FURTHER SOLAR ?"));
  CHECK_FALSE(all.contains("KALENDER"));
  CHECK(MainWindowProbe::banner_record(*w).endsWith(".TERRAR"));
  QStringList texts;
  {
    DialogDriver drive;
    drive.then(DialogDriver::fill({"2000"}, "OK")).then([&texts](QDialog* d) {
      if (const auto* canvas = d->findChild<WheelWidget*>()) {
        for (const Primitive& p : canvas->display_list().items) {
          if (p.kind == Primitive::Kind::kText) {
            texts << QString::fromStdString(p.text);
          }
        }
      }
      d->reject();
    });
    MainWindowProbe::return_list(*w, false);
    CHECK(drive.pending() == 0);
  }
  const QString sheet = texts.join("|");
  CHECK(sheet.contains("TERRAR - moments for"));
  CHECK(sheet.contains(" Heliocentric "));
  CHECK(sheet.contains("Time in UT = GMT"));
  CHECK_FALSE(sheet.contains("Zeitpunkte"));
}

TEST_CASE("the PLANETAR search and its passage menu speak English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, english_birth());
  QStringList boxes;
  {
    DialogDriver drive;
    const auto press = [](QDialog* d, const QString& caption) {
      for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
        if (b->text().contains(caption)) {
          b->click();
          return;
        }
      }
    };
    drive
        .then([&boxes, press](QDialog* d) {
          const QList<QComboBox*> combos = d->findChildren<QComboBox*>();
          REQUIRE(combos.size() >= 2);
          combos[0]->setCurrentIndex(combos[0]->findText("MARSAR"));
          boxes << box_text(d) + combos[1]->currentText();
          press(d, "OK");
        })
        .then([&boxes](QDialog* d) {
          boxes << box_text(d);
          DialogDriver::fill({"", "1", "1", "2000"}, "OK")(d);
        })
        .then([&boxes, press](QDialog* d) {
          boxes << box_text(d);
          press(d, " / DIRECT ");
        })
        .then([&boxes, press](QDialog* d) {
          boxes << box_text(d);
          press(d, "TAKE OVER the TIME VALUE");
        });
    MainWindowProbe::planetar(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const QString all = boxes.join(QChar(0x0A));
  CHECK(all.contains("SEARCH with :"));
  CHECK(all.contains("DATE"));
  CHECK(all.contains("SEARCH DATE ?  PERIOD MA="));
  CHECK(all.contains("TAKE OVER the TIME or SEARCH FURTHER POINTS ?"));
  CHECK(all.contains("SEARCH the NEXT DIRECT MARSAR in the PAST ?"));
  CHECK(all.contains("VIEW the CHART ?"));
  CHECK_FALSE(all.contains("SUCHEN"));
  CHECK_FALSE(all.contains("ÜBERNEHMEN"));
  CHECK(MainWindowProbe::banner_record(*w) == "15.MARSAR");
}

TEST_CASE("the printer boxes speak English") {
  English en;
  REQUIRE(en.loaded);
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, english_birth());
  MainWindowProbe::konsta(*w).prenbl = 1;
  const QString pdf = QString::fromStdWString((std::filesystem::temp_directory_path() / "horcom_english_print.pdf").wstring());
  set_print_file(pdf);
  QStringList boxes;
  {
    DialogDriver drive;
    const auto answer = [&boxes](const QString& caption) {
      return [&boxes, caption](QDialog* d) {
        boxes << box_text(d);
        DialogDriver::click(caption)(d);
      };
    };
    drive.then(answer("PRINTER GRAPHIC  DIN A5 ?"))
        .then([&boxes](QDialog* d) {
          boxes << box_text(d);
          d->accept();
        })
        .then(answer("Continue"));
    MainWindowProbe::horoskop_graphik(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  set_print_file(QString());
  QFile::remove(pdf);
  const QString all = boxes.join(QChar(0x0A));
  CHECK(all.contains("OUTPUT on the SCREEN or as a PRINTER GRAPHIC ?"));
  CHECK(all.contains("SCREEN ( possibly HARDCOPY )?"));
  CHECK(all.contains(" PRINTER READY ? "));
  CHECK(all.contains("SET UP and WAIT until the PRINTER WORKS !"));
  CHECK_FALSE(all.contains("DRUCKER"));
}

#endif
