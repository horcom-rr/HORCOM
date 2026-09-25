// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QAbstractButton>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <filesystem>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/statist.hpp"
#include "probe.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a working folder with SPEZIAL and three invented charts, no real people,
// more invented ones and one beyond the polar circle on demand
std::filesystem::path statist_folder(int more = 0, bool polar = false) {
  const std::filesystem::path dir = std::filesystem::temp_directory_path() / "horcom_statist_test";
  std::error_code ec;
  std::filesystem::remove_all(dir, ec);
  std::filesystem::create_directories(dir / "spezial");
  std::vector<ChartRecord> records;
  const char* names[3] = {"ANNA MUSTER", "BERND BEISPIEL", "CARLA DEMO"};
  for (int i = 0; i < 3; ++i) {
    ChartRecord r;
    r.name = names[i];
    r.place = "EICHENAU";
    r.day = 5 + i;
    r.month = 4 + 3 * i;
    r.year = 1960 + 10 * i;
    r.hour = 12.0;
    r.minute = 30.0;
    r.lon = 10.0;
    r.lat = 50.0;
    records.push_back(r);
  }
  for (int i = 0; i < more; ++i) {
    ChartRecord r;
    r.name = QString::asprintf("PERSON %03d", i).toStdString();
    r.place = "EICHENAU";
    r.day = 1 + i % 28;
    r.month = 1 + i % 12;
    r.year = 1950 + i % 50;
    r.hour = 6.0;
    r.lon = 10.0;
    r.lat = 50.0;
    records.push_back(r);
  }
  if (polar) {
    ChartRecord r;
    r.name = "DORA NORDKAP";
    r.place = "NORDKAP";
    r.day = 1;
    r.month = 7;
    r.year = 1980;
    r.hour = 12.0;
    r.lon = 25.8;
    r.lat = 71.2;
    records.push_back(r);
  }
  REQUIRE(write_chart_file(dir / "spezial" / "DEMO.DAT", records));
  return dir;
}

// a left click on a row of the list, his zeilklick
DialogDriver::Step click_row(int row) {
  return [row](QDialog* d) {
    auto* c = d->findChild<WheelWidget*>("statList");
    REQUIRE(c != nullptr);
    const QPointF at = c->from_canvas(QPointF(300.0, 45.0 + 16.0 * row));
    QTimer::singleShot(0, d, [c, at]() {
      QMouseEvent press(QEvent::MouseButtonPress, at, c->mapToGlobal(at), Qt::LeftButton, Qt::LeftButton,
                        Qt::NoModifier);
      QApplication::sendEvent(c, &press);
    });
  };
}

// the Qt file dialog of FILESELECT, the name typed in
DialogDriver::Step pick_file(const std::filesystem::path& path) {
  return [path](QDialog* d) {
    auto* fd = qobject_cast<QFileDialog*>(d);
    if (fd == nullptr) {
      d->reject();
      return;
    }
    const QFileInfo info(QString::fromStdWString(path.wstring()));
    fd->setDirectory(info.absolutePath());
    if (auto* name = fd->findChild<QLineEdit*>("fileNameEdit")) {
      name->setText(info.fileName());
    } else {
      fd->selectFile(info.absoluteFilePath());
    }
    static_cast<QDialog*>(fd)->accept();
  };
}

QString box_text(QDialog* d) {
  QString all = d->windowTitle() + "|";
  for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
    all += b->text() + "|";
  }
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    all += l->text() + "|";
  }
  if (auto* m = qobject_cast<QMessageBox*>(d)) {
    all += m->text();
  }
  return all;
}

// a key into a window that waits for one like his @stop, the texts of
// the window, the yellow counter windows among them, read first
DialogDriver::Step press(Qt::Key key, QString* seen = nullptr) {
  return [key, seen](QDialog* d) {
    if (seen != nullptr) {
      *seen = box_text(d);
    }
    QTimer::singleShot(0, d, [d, key]() {
      QKeyEvent k(QEvent::KeyPress, key, Qt::NoModifier);
      QApplication::sendEvent(d, &k);
    });
  };
}

// the texts of the list canvas
QStringList list_texts(QDialog* d) {
  QStringList texts;
  if (auto* c = d->findChild<WheelWidget*>("statList")) {
    for (const Primitive& p : c->display_list().items) {
      if (p.kind == Primitive::Kind::kText) {
        texts << QString::fromStdString(p.text);
      }
    }
  }
  return texts;
}

// the texts of the list page and ESC with his BEENDEN ? JA after it
DialogDriver::Step read_list(QStringList& texts) {
  return [&texts](QDialog* d) {
    if (auto* c = d->findChild<WheelWidget*>("statList")) {
      for (const Primitive& p : c->display_list().items) {
        if (p.kind == Primitive::Kind::kText) {
          texts << QString::fromStdString(p.text);
        }
      }
    }
    QTimer::singleShot(0, d, [d]() {
      QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
      QApplication::sendEvent(d, &esc);
    });
  };
}

void build_dataset(MainWindow& w, const std::filesystem::path& dir) {
  // slist! = 0, the GROß list
  MainWindowProbe::konsta(w).slist = false;
  DialogDriver drive;
  drive.then(DialogDriver::click("AUSWERTEFÄHIGE DATEI ERSTELLEN ?"))
      .then(DialogDriver::click("JA"))  // PARAMETER RICHTIG GESETZT ?
      .then(pick_file(dir / "spezial" / "DEMO.DAT"));
  MainWindowProbe::statistics_hub(w);
  CHECK(drive.pending() == 0);
}

}  // namespace

TEST_CASE("STATISTIK writes his dataset of a DAT into STATIST7") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  QString hub;
  {
    DialogDriver drive;
    drive.then([&hub](QDialog* d) {
      hub = box_text(d);
      DialogDriver::click("AUSWERTEFÄHIGE DATEI ERSTELLEN ?")(d);
    })
        .then(DialogDriver::click("JA"))
        .then(pick_file(dir / "spezial" / "DEMO.DAT"));
    MainWindowProbe::statistics_hub(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // ue$(0) = "     * WAS " + wol$ + " TUN ? *      "
  CHECK(hub.contains("* WAS Wollen Sie  TUN ? *"));
  CHECK(hub.contains("VORGABEN STATISTIK ÄNDERN"));
  const std::filesystem::path sta = dir / "statist7" / "DEMO.STA";
  REQUIRE(std::filesystem::exists(sta));
  CHECK(std::filesystem::exists(dir / "statist7" / "DEMO.PAR"));
  CHECK(std::filesystem::exists(sth_path(sta)));
  const auto set = load_statistics(sta);
  REQUIRE(set.has_value());
  REQUIRE(set->records.size() == 3);
  CHECK(set->records[0].name == "ANNA MUSTER");
  // par = 1 with the parallax, 2 without, gena$ the label the program
  // runs with
  CHECK((set->params.par == 1.0 || set->params.par == 2.0));
  CHECK(set->params.gena == MainWindowProbe::konsta(*w).gena);
  CHECK(set->params.gen == MainWindowProbe::konsta(*w).gen);
  // a second run asks before it writes over the dataset
  QString again;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTEFÄHIGE DATEI ERSTELLEN ?"))
        .then(DialogDriver::click("JA"))
        .then(pick_file(dir / "spezial" / "DEMO.DAT"))
        .then([&again](QDialog* d) {
          again = box_text(d);
          DialogDriver::click("ABBRUCH")(d);
        });
    MainWindowProbe::statistics_hub(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(again.contains("EXISTIERT SCHON !"));
  CHECK(again.contains("NEU ANLEGEN ?"));
}

TEST_CASE("AUSWERTUNG STARTEN lists every record OHNE EINSCHRÄNKUNG by longitude") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  QString objects;
  QString where;
  QStringList page;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then([&objects](QDialog* d) {  // 1. OBJEKT WÄHLEN !
          objects = box_text(d);
          DialogDriver::click("PLANET / HÄUSERSPITZE")(d);
        })
        .then(DialogDriver::pick_row(0))  // SONNE
        .then([&where](QDialog* d) {      // Wo soll das OBJEKT GESUCHT werden ?
          where = box_text(d);
          DialogDriver::click("OHNE EINSCHRÄNKUNG")(d);
        })
        .then(read_list(page))
        .then(DialogDriver::click("JA"));  // 'STATISTIK' BEENDEN ?
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(objects.contains(" 1. OBJEKT WÄHLEN !"));
  CHECK(objects.contains("ALPHABETISCHE Liste"));
  CHECK(where.contains("Wo soll das OBJEKT GESUCHT werden ?"));
  CHECK(where.contains("SUCHE OBJEKT : SO"));
  const QString all = page.join("|");
  CHECK(all.contains("ANNA MUSTER"));
  CHECK(all.contains("CARLA DEMO"));
  CHECK(all.contains("Datum     Zeit(UT)    AC"));
  CHECK(all.contains("Länge SO"));
  CHECK(all.contains("Im Bereich 0..360°"));
  CHECK(all.contains("Total ="));
  CHECK(all.contains("* Blättern: Leertaste | Zurück mit 'R'|Weitere Beding: 'W'|ENDE: Mit 'ESC' *"));
  // the Sun in April, July and October, the list climbs with the longitude
  CHECK(all.indexOf("ANNA MUSTER") < all.indexOf("BERND BEISPIEL"));
  CHECK(all.indexOf("BERND BEISPIEL") < all.indexOf("CARLA DEMO"));
}

TEST_CASE("UND INKLUSIV chains two NAME conditions and inverts the complete rows") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  QString join;
  QStringList page;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("NAME    ( BUCHSTABENFOLGE )"))
        .then(DialogDriver::fill({" "}, "OK"))
        .then([&join](QDialog* d) {  // * WEITERE BEDINGUNG ? *
          join = box_text(d);
          DialogDriver::click("UND  INKLUSIV")(d);
        })
        .then(DialogDriver::click("NAME    ( BUCHSTABENFOLGE )"))
        .then(DialogDriver::fill({"anna"}, "OK"))
        .then(DialogDriver::click("AUSGABE-LISTE"))
        .then(read_list(page))
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(join.contains("* WEITERE BEDINGUNG ? *"));
  CHECK(join.contains("AUSGABE-LISTE"));
  const QString all = page.join("|");
  // three records met the blank, ANNA both conditions
  CHECK(all.contains("1u2"));
  CHECK(all.contains("Mehrere Bedingungen !"));
  CHECK(all.contains("UND-Bedingung"));
}

TEST_CASE("a clicked row takes the record over as RADIX with his remark") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  QString row_box;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("PLANET / HÄUSERSPITZE"))
        .then(DialogDriver::pick_row(0))
        .then(DialogDriver::click("OHNE EINSCHRÄNKUNG"))
        .then([](QDialog* d) {  // the list, a click on the first row
          auto* c = d->findChild<WheelWidget*>("statList");
          REQUIRE(c != nullptr);
          const QPointF at = c->from_canvas(QPointF(300.0, 45.0));
          QTimer::singleShot(0, d, [c, at]() {
            QMouseEvent press(QEvent::MouseButtonPress, at, c->mapToGlobal(at), Qt::LeftButton, Qt::LeftButton,
                              Qt::NoModifier);
            QApplication::sendEvent(c, &press);
          });
        })
        .then([&row_box](QDialog* d) {  // HOROSKOP nur ANSCHAUEN ?
          row_box = box_text(d);
          // NICHT  ÜBERNEHMEN holds the caption too, the exact button
          for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            if (b->text() == QStringLiteral(" ÜBERNEHMEN")) {
              b->click();
              return;
            }
          }
          d->reject();
        })
        .then([](QDialog* d) {  // the chart until a key
          QTimer::singleShot(0, d, [d]() {
            QKeyEvent key(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
            QApplication::sendEvent(d, &key);
          });
        })
        .then([](QDialog* d) {  // the list again
          QTimer::singleShot(0, d, [d]() {
            QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            QApplication::sendEvent(d, &esc);
          });
        })
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
  }
  CHECK(row_box.contains("HOROSKOP nur ANSCHAUEN ?"));
  CHECK(row_box.contains("NICHT  ÜBERNEHMEN"));
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  CHECK(MainWindowProbe::slot(*w, 0)->surname == "ANNA MUSTER");
  CHECK(MainWindowProbe::slot(*w, 0)->comment == "DATEN AUS 'STATISTIK'-DATEI ÜBERNOMMEN");
}

TEST_CASE("NEUE DATENSÄTZE UPDATEN puts a saved record into the dataset of its file") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  MainWindowProbe::konsta(*w).stats = true;
  MainWindowProbe::bind(*w, QString::fromStdWString((dir / "spezial" / "DEMO.DAT").wstring()));
  AafRecord r;
  r.surname = "DORA";
  r.given = "NEU";
  r.day = 3;
  r.month = 3;
  r.year = 1985;
  r.hour = 8;
  r.zone = "00hE00:00";
  r.lat_deg = 50;
  r.lon_deg = 10;
  MainWindowProbe::apply(*w, r);
  QString ask;
  QString done;
  {
    DialogDriver drive;
    drive.then([](QDialog* d) { d->accept(); })  // DATEI ... SÄTZE
        .then([&ask](QDialog* d) {
          ask = box_text(d);
          DialogDriver::click("JA")(d);
        })
        .then([&done](QDialog* d) {
          done = box_text(d);
          d->accept();
        });
    MainWindowProbe::save(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
  }
  CHECK(ask.contains("Datensatz in STATISTIK-DATEI"));
  CHECK(ask.contains("ÜBERNEHMEN ?"));
  CHECK(done.contains("= 4 SÄTZE"));
  const auto set = load_statistics(dir / "statist7" / "DEMO.STA");
  REQUIRE(set.has_value());
  CHECK(set->records.size() == 4);
  CHECK(set->records.back().name == "DORA NEU");
}

TEST_CASE("ABBRUCH in the EINZELNE PLANETEN box still shows the chart and keeps the list") {
  // his einzel_plan_wahl1 answered ABBRUCH with @rer alone, kotab_sta
  // went on to a11_1, the port ended the whole STATISTIK there
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  QString chart;
  QStringList page;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("PLANET / HÄUSERSPITZE"))
        .then(DialogDriver::pick_row(0))
        .then(DialogDriver::click("OHNE EINSCHRÄNKUNG"))
        .then(click_row(0))
        .then(DialogDriver::click("NICHT  ÜBERNEHMEN"))
        .then(DialogDriver::click("EINZEL-Betrachtung"))
        .then(DialogDriver::click("ABBRUCH"))  // EINZELNE PlANETEN ?
        .then(press(Qt::Key_Space, &chart))
        .then(read_list(page))
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(chart.startsWith("ANNA MUSTER|"));
  // the counters run for the single view, one record so far
  CHECK(chart.contains("   1 DATENSÄTZE"));
  CHECK(page.join("|").contains("ANNA MUSTER"));
}

TEST_CASE("AUSWERTUNG des ZÄHLERS counts every listed record once") {
  // his kota0 showed the last chart again without a recount, the port
  // counted it twice. Three single views in mode 2 must sum up to the
  // same windows as the one pass of mode 4
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  const auto start = [&](DialogDriver& drive) {
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("PLANET / HÄUSERSPITZE"))
        .then(DialogDriver::pick_row(0))
        .then(DialogDriver::click("OHNE EINSCHRÄNKUNG"));
  };
  QString single;
  {
    DialogDriver drive;
    start(drive);
    drive.then(click_row(0))
        .then(DialogDriver::click("NICHT  ÜBERNEHMEN"))
        .then(DialogDriver::click("EINZEL-Betrachtung"))
        .then(DialogDriver::click("NORMALE AUSGABE"))
        .then(press(Qt::Key_Space))
        .then(click_row(1))
        .then(DialogDriver::click("NICHT  ÜBERNEHMEN"))
        .then(press(Qt::Key_Space))
        .then(click_row(2))
        .then(DialogDriver::click("NICHT  ÜBERNEHMEN"))
        .then(press(Qt::Key_Space, &single))
        .then(press(Qt::Key_Escape))
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
  }
  QString first;
  QString whole;
  {
    DialogDriver drive;
    start(drive);
    drive.then(click_row(0))
        .then(DialogDriver::click("NICHT  ÜBERNEHMEN"))
        .then(DialogDriver::click("AUSWERTUNG des ZÄHLERS"))
        .then(press(Qt::Key_Space, &first))  // the clicked chart first
        .then(press(Qt::Key_Space, &whole))  // the last one with the sums
        .then(DialogDriver::click("JA"))     // WEITERGEHEN und ZÄHLER LÖSCHEN ?
        .then(DialogDriver::click("Beenden"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(single.contains("   3 DATENSÄTZE"));
  // his kotab_sta drew the clicked chart plain before the list counted
  CHECK(first.startsWith("ANNA MUSTER|"));
  CHECK_FALSE(first.contains("DATENSÄTZE"));
  CHECK(whole.startsWith("CARLA DEMO|"));
  CHECK(whole == single);
}

TEST_CASE("a list without a match shows no page") {
  // his WHILE kl& < zdm& never ran, lend started the next evaluation
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  QString niete;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("NAME    ( BUCHSTABENFOLGE )"))
        .then(DialogDriver::fill({"XYZ"}, "OK"))
        .then(DialogDriver::click("AUSGABE-LISTE"))
        .then([&niete](QDialog* d) {
          niete = box_text(d);
          d->accept();
        })
        .then(DialogDriver::click("ABBRUCH"));  // 1. OBJEKT WÄHLEN ! of the next run
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    for (const QString& t : drive.titles()) {
      CHECK_FALSE(t.startsWith("STATISTIK |"));
    }
  }
  CHECK(niete.contains("ERFÜLLT ALLE BEDINGUNGEN"));
}

TEST_CASE("S jumps six pages like his footer says") {
  // his kl& + 120 at the end of a page and the next page after it, the
  // port went five pages on
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder(147);
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  QStringList first;
  QStringList after;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("NAME    ( BUCHSTABENFOLGE )"))
        .then(DialogDriver::fill({" "}, "OK"))
        .then(DialogDriver::click("AUSGABE-LISTE"))
        .then([&first, &after](QDialog* d) {
          first = list_texts(d);
          QKeyEvent s(QEvent::KeyPress, Qt::Key_S, Qt::NoModifier, "s");
          QApplication::sendEvent(d, &s);
          after = list_texts(d);
          QTimer::singleShot(0, d, [d]() {
            QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            QApplication::sendEvent(d, &esc);
          });
        })
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
  }
  // 150 rows fill seven pages, the second footer stands past 145
  CHECK(first.contains("  1"));
  CHECK(first.join("|").contains("6 Bildschirme Vorwärts : 'S'"));
  CHECK(after.contains("  7"));
  CHECK_FALSE(after.contains("  6"));
}

TEST_CASE("a dataset without houses and angles stays geocentric") {
  // his stat2 took AC, MC and house 2 at zero for a heliocentric file,
  // house system 9 switched the program to HELIO
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  MainWindowProbe::houses(*w, static_cast<int>(HouseSystem::kNone));
  build_dataset(*w, dir);
  const auto set = load_statistics(dir / "statist7" / "DEMO.STA");
  REQUIRE(set.has_value());
  CHECK(set->records[0].ac == 0.0);
  CHECK_FALSE(stat_heliocentric(*set));
  QString objects;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then([&objects](QDialog* d) {
          objects = box_text(d);
          DialogDriver::click("ABBRUCH")(d);
        });
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(objects.contains("OBJEKT WÄHLEN"));
  CHECK_FALSE(objects.contains("HELIOZENTRISCH"));
  CHECK_FALSE(MainWindowProbe::settings(*w).heliocentric);
}

TEST_CASE("a record beyond the polar circle is counted out with a notice") {
  // Placidus refuses the Nordkap, his stat1 had no guard, the port
  // dropped the record without a word
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder(0, true);
  MainWindowProbe::set_data_dir(*w, dir);
  MainWindowProbe::houses(*w, static_cast<int>(HouseSystem::kPlacidus));
  MainWindowProbe::konsta(*w).slist = false;
  QString notice;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTEFÄHIGE DATEI ERSTELLEN ?"))
        .then(DialogDriver::click("JA"))
        .then(pick_file(dir / "spezial" / "DEMO.DAT"))
        .then([&notice](QDialog* d) {
          notice = box_text(d);
          d->accept();
        });
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
  }
  CHECK(notice.contains("1 DATENSÄTZE NICHT AUFGENOMMEN"));
  const auto set = load_statistics(dir / "statist7" / "DEMO.STA");
  REQUIRE(set.has_value());
  CHECK(set->records.size() == 3);
}

TEST_CASE("SO MO AC draws its sign bars and OHNE EINSCHRÄNKUNG keeps no info box") {
  // inf_box2 skipped only ASPEKT and NAME, inf_box4 stayed shut under
  // suc& = 5
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  QStringList bars;
  QStringList plain;
  QStringList boxed;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("PLANET / HÄUSERSPITZE"))
        .then([](QDialog* d) {  // SO / MO / AC, the row after HERR v. HAUS
          QListWidget* list = d->findChild<QListWidget*>();
          REQUIRE(list != nullptr);
          for (int i = 0; i < list->count(); ++i) {
            if (list->item(i)->text().contains("SO / MO / AC")) {
              list->setCurrentRow(i);
              emit list->itemActivated(list->currentItem());
              return;
            }
          }
          d->reject();
        })
        .then(DialogDriver::click("Bei GRAD : NENNWERT"))
        .then(DialogDriver::fill({"15"}, "OK"))
        .then(DialogDriver::click("10"))  // ORBIS in GRAD ?
        .then(DialogDriver::click("AUSGABE-LISTE"))
        .then(read_list(bars))
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
  }
  CHECK(bars.contains("SO..AC "));
  CHECK(bars.contains("Zeichen"));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("PLANET / HÄUSERSPITZE"))
        .then(DialogDriver::pick_row(0))
        .then(DialogDriver::click("OHNE EINSCHRÄNKUNG"))
        .then([&plain](QDialog* d) {
          auto* c = d->findChild<WheelWidget*>("statList");
          REQUIRE(c != nullptr);
          const QPointF at = c->from_canvas(QPointF(300.0, 45.0));
          QMouseEvent right(QEvent::MouseButtonPress, at, c->mapToGlobal(at), Qt::RightButton, Qt::RightButton,
                            Qt::NoModifier);
          QApplication::sendEvent(c, &right);
          plain = list_texts(d);
          QTimer::singleShot(0, d, [d]() {
            QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            QApplication::sendEvent(d, &esc);
          });
        })
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK_FALSE(plain.join("|").contains("wurden durchgesucht"));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("NAME    ( BUCHSTABENFOLGE )"))
        .then(DialogDriver::fill({"ANNA"}, "OK"))
        .then(DialogDriver::click("AUSGABE-LISTE"))
        .then([&boxed](QDialog* d) {
          auto* c = d->findChild<WheelWidget*>("statList");
          REQUIRE(c != nullptr);
          const QPointF at = c->from_canvas(QPointF(300.0, 45.0));
          QMouseEvent right(QEvent::MouseButtonPress, at, c->mapToGlobal(at), Qt::RightButton, Qt::RightButton,
                            Qt::NoModifier);
          QApplication::sendEvent(c, &right);
          boxed = list_texts(d);
          QTimer::singleShot(0, d, [d]() {
            QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            QApplication::sendEvent(d, &esc);
          });
        })
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(boxed.join("|").contains("   3 DATENSÄTZE wurden durchgesucht"));
}
