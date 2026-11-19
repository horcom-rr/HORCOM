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
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QStyleOptionButton>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/statist.hpp"
#include "horcom/data/statist_eval.hpp"
#include "probe.hpp"
#include "robert_input.hpp"
#include "theme.hpp"

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

const char* const kNames[3] = {"ANNA MUSTER", "BERND BEISPIEL", "CARLA DEMO"};

// a longitude in radians as degrees within the circle
double deg_of(double rad) {
  return norm_rad(rad) * kRadToDeg;
}

// the record whose value stands more than six degrees from every other,
// the window of two degrees around it holds that record alone. The size
// of the list when no record stands apart
std::size_t lone_record(const std::vector<double>& deg) {
  for (std::size_t i = 0; i < deg.size(); ++i) {
    bool apart = true;
    for (std::size_t k = 0; k < deg.size(); ++k) {
      const double gap = std::fabs(deg[i] - deg[k]);
      apart = apart && (k == i || std::min(gap, kDegPerCircle - gap) > 6.0);
    }
    if (apart) {
      return i;
    }
  }
  return deg.size();
}

// one condition at LAGE Bei GRAD with an orb of two degrees, the steps
// of the object first, and the texts of its list
QStringList at_degree(MainWindow& w, const std::filesystem::path& dir, double degree,
                      const std::vector<DialogDriver::Step>& object) {
  QStringList page;
  DialogDriver drive;
  drive.then(DialogDriver::click("AUSWERTUNG STARTEN")).then(pick_file(dir / "statist7" / "DEMO.STA"));
  for (const DialogDriver::Step& s : object) {
    drive.then(s);
  }
  drive.then(DialogDriver::click("Bei GRAD..ORBIS.."))
      .then(DialogDriver::fill({QString::number(degree, 'f', 1)}, "OK"))
      .then(DialogDriver::click_exact("2"))  // ORBIS in GRAD ?
      .then(DialogDriver::click("AUSGABE-LISTE"))
      .then(read_list(page))
      .then(DialogDriver::click("JA"));
  MainWindowProbe::statistics_hub(w);
  INFO(drive.titles().join(" | ").toStdString());
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  return page;
}

// the list holds the one record and neither of the others
void check_only(const QStringList& page, std::size_t record) {
  const QString all = page.join("|");
  for (std::size_t i = 0; i < 3; ++i) {
    INFO(std::string(kNames[i]));
    CHECK(all.contains(kNames[i]) == (i == record));
  }
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

TEST_CASE("the aspect counter stands where his window of anzeigen_asp_zaehl stood") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  QRect box;
  QSize view;
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
        .then(DialogDriver::click("NORMALE AUSGABE"))
        .then([&box, &view](QDialog* d) {
          view = d->size();
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            if (l->text().contains("ASPEKTE - ZÄHLER")) {
              box = l->geometry();
            }
          }
          press(Qt::Key_Space)(d);
        })
        .then(press(Qt::Key_Escape))
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
  }
  REQUIRE(box.isValid());
  // his OPENW #12,@xk(5),@yk(300) on the 480 lines of his screen, the
  // tester found the box sunk to the bottom edge too low
  const long y = std::lround(view.height() * 300.0 / 480.0);
  CHECK(box.top() <= y);
  // and no higher than the sheet's edge pushes it up
  CHECK(box.top() >= std::min<long>(y, view.height() - box.height() - box.left()) - 1);
  CHECK(box.bottom() < view.height());
  CHECK(box.left() < view.width() / 20);
}

TEST_CASE("the sign list takes one click like his ausw_obj_e") {
  // his handlemessage set select& on the first click and ausw_obj_e ended
  // there, the tester had to double click in the port
  std::optional<int> picked;
  {
    DialogDriver drive;
    drive.then([](QDialog* d) {
      auto* list = d->findChild<QListWidget*>();
      REQUIRE(list != nullptr);
      const QPoint at = list->visualItemRect(list->item(4)).center();
      QTimer::singleShot(0, d, [list, at]() {
        QMouseEvent press(QEvent::MouseButtonPress, at, list->viewport()->mapToGlobal(at), Qt::LeftButton, Qt::LeftButton,
                          Qt::NoModifier);
        QApplication::sendEvent(list->viewport(), &press);
        QMouseEvent release(QEvent::MouseButtonRelease, at, list->viewport()->mapToGlobal(at), Qt::LeftButton,
                            Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(list->viewport(), &release);
      });
    });
    picked = pick_sign(nullptr);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // row 4 is the LÖWE
  CHECK(picked == std::optional<int>(4));
}

TEST_CASE("the keys of his ZIFFERN-EINGABE show their digits in the dress of the program") {
  // the style sheet pads its buttons, the narrow keys lost their digits and
  // the tester saw empty boxes for the ORBIS and the HAUS NR.
  const QString sheet_before = qApp->styleSheet();
  theme::apply(theme::kTextScaleNormal, false);
  int keys = 0;
  bool readable = true;
  std::optional<int> house;
  {
    DialogDriver drive;
    drive.then([&keys, &readable](QDialog* d) {
      for (QPushButton* b : d->findChildren<QPushButton*>()) {
        QStyleOptionButton opt;
        opt.initFrom(b);
        opt.text = b->text();
        const QRect room = b->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, b);
        readable = readable && room.width() >= b->fontMetrics().horizontalAdvance(b->text().trimmed());
        ++keys;
      }
      DialogDriver::click("12")(d);
    });
    house = ask_digit(nullptr, "HERR von HAUS NR.?", 1, 12);
    CHECK(drive.pending() == 0);
  }
  qApp->setStyleSheet(sheet_before);
  CHECK(keys == 12);
  CHECK(readable);
  CHECK(house == std::optional<int>(12));
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
        .then(press(Qt::Key_Space, &whole))  // the clicked one again with the sums
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
  // the windows stand over the clicked chart, his kota0 had left them over
  // the last chart of the list, CARLA DEMO, one the reader never chose
  CHECK(whole.startsWith("ANNA MUSTER|"));
  CHECK(whole.section('|', 1) == single.section('|', 1));
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

TEST_CASE("HALBSUMME searches the nearer midpoint of its two factors") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  const auto set = load_statistics(dir / "statist7" / "DEMO.STA");
  REQUIRE(set.has_value());
  std::vector<double> deg;
  for (const StatRecord& r : set->records) {
    const double so = r.el[body::kSun];
    const double mo = r.el[body::kMoon];
    // the midpoint on the shorter arc between the two
    const double mid = (so + mo) / 2.0;
    deg.push_back(deg_of(std::fabs(so - mo) > kPi ? mid + kPi : mid));
  }
  const std::size_t i = lone_record(deg);
  REQUIRE(i < deg.size());
  const QStringList page = at_degree(*w, dir, deg[i],
                                     {DialogDriver::click_exact("HALBSUMME"), DialogDriver::pick_text("SONNE"),
                                      DialogDriver::pick_text("MOND")});
  check_only(page, i);
  // p$ = "HALBSUMME " + pl$(m&(1)) + "-" + pl$(m&(2))
  CHECK(page.join("|").contains("HALBSUMME SO-MO"));
}

TEST_CASE("SPIEGELPUNKT mirrors its factor at the chosen axis") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  const auto set = load_statistics(dir / "statist7" / "DEMO.STA");
  REQUIRE(set.has_value());
  // AR-LI turns w into 360° - w, CN-CP into 180° - w
  const std::pair<QString, double> axes[2] = {{"AR-LI", kTwoPi}, {"CN-CP", kPi}};
  for (const auto& [axis, turn] : axes) {
    INFO(axis.toStdString());
    std::vector<double> deg;
    for (const StatRecord& r : set->records) {
      deg.push_back(deg_of(turn - r.el[body::kSun]));
    }
    const std::size_t i = lone_record(deg);
    REQUIRE(i < deg.size());
    const QStringList page = at_degree(*w, dir, deg[i],
                                       {DialogDriver::click("SPIEGELPUNKT"), DialogDriver::click_exact(axis),
                                        DialogDriver::pick_text("SONNE")});
    check_only(page, i);
    CHECK(page.join("|").contains("SPIEGELP.SO /" + axis));
  }
}

TEST_CASE("ARABISCHE TEILE take the fixed formula or turn it by day and night") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  const auto set = load_statistics(dir / "statist7" / "DEMO.STA");
  REQUIRE(set.has_value());
  {
    // IMMER : FAKTOR 1 + FAKTOR 2 - FAKTOR 3
    std::vector<double> deg;
    for (const StatRecord& r : set->records) {
      deg.push_back(deg_of(r.ac + r.el[body::kMoon] - r.el[body::kSun]));
    }
    const std::size_t i = lone_record(deg);
    REQUIRE(i < deg.size());
    const QStringList page =
        at_degree(*w, dir, deg[i],
                  {DialogDriver::click("ARABISCHE TEILE"), DialogDriver::click("IMMER F1 + F2 - F3"),
                   DialogDriver::pick_text("ASZENDENT"), DialogDriver::pick_text("MOND"),
                   DialogDriver::pick_text("SONNE")});
    check_only(page, i);
    CHECK(page.join("|").contains("ARABT. AC+MO-SO"));
  }
  {
    // TRADITIONELL with his GLÜCKSPUNKT example, F1 + F3 - F2 when the
    // Sun stands above the horizon, from the descendant on to the
    // ascendant, F1 + F2 - F3 below it
    std::vector<double> deg;
    for (const StatRecord& r : set->records) {
      const double so = r.el[body::kSun];
      const double mo = r.el[body::kMoon];
      const bool day = norm_rad(so - r.ac) > kPi;
      deg.push_back(deg_of(day ? r.ac + mo - so : r.ac + so - mo));
    }
    const std::size_t i = lone_record(deg);
    REQUIRE(i < deg.size());
    const QStringList page =
        at_degree(*w, dir, deg[i],
                  {DialogDriver::click("ARABISCHE TEILE"), DialogDriver::click("TRADITIONELL"),
                   DialogDriver::pick_text("ASZENDENT"), DialogDriver::pick_text("SONNE"),
                   DialogDriver::pick_text("MOND")});
    check_only(page, i);
    CHECK(page.join("|").contains("ARABT. TRADIT. AC/SO/MO"));
  }
}

TEST_CASE("HERRSCHER von HAUS searches the ruler of the sign on the cusp") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  const auto set = load_statistics(dir / "statist7" / "DEMO.STA");
  REQUIRE(set.has_value());
  StatQuery q;
  q.object = StatObject::kHouseRuler;
  q.a.house = 1;
  q.rulers = MainWindowProbe::rulers(*w);
  std::vector<double> mask;
  const StatEvalResult ruled = evaluate_statistics(*set, q, MainWindowProbe::aspect_settings(*w), mask);
  REQUIRE(ruled.matches.size() == 3);
  // the evaluator itself is pinned in test_statist_eval.cpp, here it
  // gives each record the place of its ruler
  std::vector<double> deg(3);
  for (const StatMatch& m : ruled.matches) {
    deg[static_cast<std::size_t>(m.record)] = deg_of(m.value);
  }
  const std::size_t i = lone_record(deg);
  REQUIRE(i < deg.size());
  const QStringList page =
      at_degree(*w, dir, deg[i], {DialogDriver::click("HERRSCHER von HAUS"), DialogDriver::click_exact("1")});
  check_only(page, i);
  CHECK(page.join("|").contains("Herr v. Haus 1"));
}

TEST_CASE("ASPEKT lists the records whose factors stand in aspect") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  const auto set = load_statistics(dir / "statist7" / "DEMO.STA");
  REQUIRE(set.has_value());
  const AspectSettings& orbs = MainWindowProbe::aspect_settings(*w);
  const auto records_of = [&](const StatQuery& q) {
    std::vector<double> mask;
    std::vector<bool> hit(set->records.size());
    for (const StatMatch& m : evaluate_statistics(*set, q, orbs, mask).matches) {
      hit[static_cast<std::size_t>(m.record)] = true;
    }
    return hit;
  };
  const auto run = [&](const std::vector<DialogDriver::Step>& steps) {
    QStringList page;
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN")).then(pick_file(dir / "statist7" / "DEMO.STA"));
    for (const DialogDriver::Step& s : steps) {
      drive.then(s);
    }
    drive.then(DialogDriver::click("AUSGABE-LISTE")).then(read_list(page)).then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    return page.join("|");
  };

  // every aspect up to the divisor 8 between SO and MO, the orbs of the
  // orb table
  StatQuery all;
  all.object = StatObject::kAspect;
  all.a.body = body::kSun;
  all.b.body = body::kMoon;
  all.asp_low = 1;
  all.asp_high = kStatMaxRangeDivisor;
  const std::vector<bool> want = records_of(all);
  REQUIRE(std::find(want.begin(), want.end(), true) != want.end());
  const QString listed =
      run({DialogDriver::click("ASPEKT (mit"), DialogDriver::pick_text("SONNE"),
           DialogDriver::click("mit EINZELNEM FAKTOR"), DialogDriver::pick_text("MOND"),
           DialogDriver::click("ALLE ASPEKTE bis TEILER"), DialogDriver::click_exact("8")});
  for (std::size_t i = 0; i < 3; ++i) {
    INFO(std::string(kNames[i]));
    CHECK(listed.contains(kNames[i]) == want[i]);
  }
  CHECK(listed.contains("ASPEKT SO/MO bis TEILER 8"));

  // MA in one aspect to the midpoint of SO and MO, at the first divisor
  // that tells the records apart, with the orb of its row
  StatQuery mid;
  mid.object = StatObject::kMidpointAspect;
  mid.a.body = body::kMars;
  mid.b.body = body::kSun;
  mid.c.body = body::kMoon;
  int divisor = 0;
  std::vector<bool> some;
  for (int n = 1; n <= kStatMaxSingleDivisor && divisor == 0; ++n) {
    mid.asp_low = n;
    mid.asp_high = n;
    mid.asp_orb = orbs.orbe[static_cast<std::size_t>(n)];
    some = records_of(mid);
    const auto count = std::count(some.begin(), some.end(), true);
    if (count > 0 && count < 3) {
      divisor = n;
    }
  }
  REQUIRE(divisor > 0);
  const QString single =
      run({DialogDriver::click("ASPEKT (mit"), DialogDriver::pick_text("MARS"), DialogDriver::click("mit HALBSUMME"),
           DialogDriver::pick_text("SONNE"), DialogDriver::pick_text("MOND"), DialogDriver::click("EINZEL-ASPEKT"),
           DialogDriver::click_exact(QString::number(divisor))});
  for (std::size_t i = 0; i < 3; ++i) {
    INFO(std::string(kNames[i]));
    CHECK(single.contains(kNames[i]) == some[i]);
  }
  CHECK(single.contains("ASPEKT MA/ HS. SO-MO"));
}

TEST_CASE("ODER joins two conditions and UND EXKLUSIV lists only the complete rows") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  const auto two_names = [&](const QString& first, const QString& join, const QString& second, QString& box) {
    QStringList page;
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTUNG STARTEN"))
        .then(pick_file(dir / "statist7" / "DEMO.STA"))
        .then(DialogDriver::click("NAME    ( BUCHSTABENFOLGE )"))
        .then(DialogDriver::fill({first}, "OK"))
        .then(DialogDriver::click_exact(join))
        .then(DialogDriver::click("NAME    ( BUCHSTABENFOLGE )"))
        .then(DialogDriver::fill({second}, "OK"))
        .then([&box](QDialog* d) {  // * WEITERE BEDINGUNG ? *
          box = box_text(d);
          DialogDriver::click("AUSGABE-LISTE")(d);
        })
        .then(read_list(page))
        .then(DialogDriver::click("JA"));
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    return page.join("|");
  };
  QString box;
  const QString either = two_names("ANNA", "ODER", "CARLA", box);
  CHECK(either.contains("ANNA MUSTER"));
  CHECK(either.contains("CARLA DEMO"));
  CHECK_FALSE(either.contains("BERND BEISPIEL"));
  // e$ = " " + STR$(anzb&,2) + " OBJEKTE SCHON GEWÄHLT ! "
  CHECK(box.contains("  2 OBJEKTE SCHON GEWÄHLT ! "));
  // three records meet the blank, ANNA both conditions, the exclusive
  // list keeps her alone
  const QString both = two_names(" ", "UND  EXKLUSIV", "anna", box);
  CHECK(both.contains("ANNA MUSTER"));
  CHECK_FALSE(both.contains("BERND BEISPIEL"));
  CHECK_FALSE(both.contains("CARLA DEMO"));
  CHECK(both.contains("UND-Bedingung"));
}

TEST_CASE("VORGABEN STATISTIK set the update and the list size, LÖSCHEN asks first") {
  auto w = MainWindowProbe::make();
  const auto dir = statist_folder();
  MainWindowProbe::set_data_dir(*w, dir);
  build_dataset(*w, dir);
  MainWindowProbe::konsta(*w).stats = false;
  MainWindowProbe::konsta(*w).slist = false;
  QString update;
  QString size;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("VORGABEN STATISTIK ÄNDERN"))
        .then([&update](QDialog* d) {
          update = box_text(d);
          DialogDriver::click_exact("JA")(d);
        })
        .then([&size](QDialog* d) {
          size = box_text(d);
          DialogDriver::click_exact("KLEIN")(d);
        });
    MainWindowProbe::statistics_hub(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(update.contains("NEUE DATENSÄTZE UPDATEN ?"));
  CHECK(size.contains("MEHR DETAILS in KLEIN-SCHRIFT ?"));
  CHECK(MainWindowProbe::konsta(*w).stats);
  CHECK(MainWindowProbe::konsta(*w).slist);
  {
    // his GOTO stend skipped kon_dsp, the first answer goes with it
    DialogDriver drive;
    drive.then(DialogDriver::click("VORGABEN STATISTIK ÄNDERN"))
        .then(DialogDriver::click_exact("NEIN"))
        .then(DialogDriver::click("Zurück zum HAUPT - MENÜ"));
    MainWindowProbe::statistics_hub(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::konsta(*w).stats);
  CHECK(MainWindowProbe::konsta(*w).slist);

  // NEIN stands first and keeps the dataset, JA removes it
  const std::filesystem::path sta = dir / "statist7" / "DEMO.STA";
  QString ask;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTEFÄHIGE DATEI LÖSCHEN ?"))
        .then(pick_file(sta))
        .then([&ask](QDialog* d) {
          ask = box_text(d);
          DialogDriver::click_exact("NEIN")(d);
        });
    MainWindowProbe::statistics_hub(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(ask.contains("WIRKLICH LÖSCHEN ?"));
  CHECK(std::filesystem::exists(sta));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSWERTEFÄHIGE DATEI LÖSCHEN ?"))
        .then(pick_file(sta))
        .then(DialogDriver::click_exact("JA"));
    MainWindowProbe::statistics_hub(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK_FALSE(std::filesystem::exists(sta));
  CHECK_FALSE(std::filesystem::exists(dir / "statist7" / "DEMO.PAR"));
}
