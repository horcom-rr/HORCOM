// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QAbstractButton>
#include <QKeyEvent>
#include <QLabel>
#include <QTableWidget>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <memory>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/chart/arabic.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/stars.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/wheel.hpp"
#include "probe.hpp"
#include "robert_text.hpp"
#include "table_fit.hpp"
#include "wheel_widget.hpp"
#include "zodiac_cells.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a synthetic morning birth east of Greenwich, no real person
AafRecord morning_birth() {
  AafRecord r;
  r.surname = "TESTFALL";
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

void press_later(QDialog* d, int key) {
  QTimer::singleShot(0, d, [d, key]() {
    QKeyEvent e(QEvent::KeyPress, key, Qt::NoModifier);
    QApplication::sendEvent(d, &e);
  });
}

QString cell(const QTableWidget* t, int row, int col) {
  const QTableWidgetItem* item = t->item(row, col);
  return item != nullptr ? item->text() : QString();
}

}  // namespace

TEST_CASE("PLANETEN-KOORDINATEN draws his ko_ta rows with both nodes") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const ChartSettings s = MainWindowProbe::settings(*w);
  QStringList heads;
  QString head_text;
  int rows = 0;
  QString mean_vel;
  QString mean_a;
  QString true_a;
  QString sun_a;
  QString mean_mark;
  QString true_lon;
  QString true_mark;
  QString true_ra;
  QString sun_ra;
  QString sun_percent;
  QString sun_au;
  QString sun_apsides;
  QString mars_node;
  QString ag_mean_lon;
  QString ag_true_lon;
  QString ag_mean_mark;
  QString ag_true_mark;
  QString gl_tag;
  QString gl_lon;
  QColor mean_node_back;
  QColor true_node_back;
  QColor mean_ag_back;
  QColor true_ag_back;
  QString question;
  {
    DialogDriver drive;
    drive
        .then([&](QDialog* d) {
          const auto* t = d->findChild<QTableWidget*>();
          REQUIRE(t != nullptr);
          for (int c = 0; c < t->columnCount(); ++c) {
            heads << t->horizontalHeaderItem(c)->text();
          }
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            head_text += l->text() + "|";
          }
          rows = t->rowCount();
          mean_vel = cell(t, 10, 3);
          mean_a = cell(t, 10, 4);
          true_a = cell(t, 11, 4);
          sun_a = cell(t, 0, 4);
          // his textrc of Mittel and Wahr in the Breite column of the nodes
          mean_mark = cell(t, 10, 2);
          true_lon = cell(t, 11, 1);
          true_mark = cell(t, 11, 2);
          true_ra = cell(t, 11, 7);
          sun_ra = cell(t, 0, 7);
          sun_percent = cell(t, 0, 5);
          sun_au = cell(t, 0, 6);
          sun_apsides = cell(t, 0, 11);
          mars_node = cell(t, 4, 9);
          // the Black Moon moved up from ZUSATZ, its box in the Rekt. column
          ag_mean_lon = cell(t, 12, 1);
          ag_true_lon = cell(t, 13, 1);
          ag_mean_mark = cell(t, 12, 7);
          ag_true_mark = cell(t, 13, 7);
          gl_tag = cell(t, 14, 0);
          gl_lon = cell(t, 14, 1);
          mean_node_back = t->item(10, 2)->background().color();
          true_node_back = t->item(11, 2)->background().color();
          mean_ag_back = t->item(12, 7)->background().color();
          true_ag_back = t->item(13, 7)->background().color();
          press_later(d, Qt::Key_Space);
        })
        .then([&question](QDialog* d) {
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            question += l->text();
          }
          // the table stays after NEIN, ESC closes it
          auto* table = qobject_cast<QDialog*>(d->parentWidget());
          DialogDriver::click(" NEIN ")(d);
          if (table != nullptr) {
            QTimer::singleShot(0, table, [table]() { table->reject(); });
          }
        });
    MainWindowProbe::coordinate_table(*w, false);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // the distance in percent and in AU side by side, the tester's wish in
  // place of the ENTFERNUNGSWERTE question
  CHECK(heads == QStringList{"Pl", "Ekl. Länge", "Breite", "Vel.'", "A", "Entf.%", "Entf.AE", "Rekt.°", "Dekl.°",
                             "Knot.ND", "Knot.SD", "Apsiden", "Pl"});
  // SO to PL, the node and the Black Moon twice, Mittel and Wahr, and the
  // Glückspunkt last
  CHECK(rows == 15);
  // his mean node runs -0.00092422029 per day, -3.18 arc minutes
  CHECK(mean_vel.trimmed() == "-3.18");
  // the Mittel row has no change of motion, his IF without a zero case
  // printed "-" there, the true node carries its sign
  CHECK(mean_a.isEmpty());
  CHECK((true_a == "+" || true_a == "-"));
  // the Sun loses speed from January to July, his chain had no sign for it
  CHECK(sun_a == "-");
  CHECK(mean_mark == "Mittel");
  CHECK(true_mark == "Wahr");
  // the true node in the minutes format
  CHECK_FALSE(true_lon.contains('"'));
  CHECK(true_lon.contains('\''));
  CHECK_FALSE(true_ra.isEmpty());
  // the right ascension with three decimals
  CHECK(sun_ra.trimmed().section('.', 1).size() == 3);
  CHECK(sun_percent.trimmed().endsWith('%'));
  CHECK(sun_au.trimmed().toDouble() == doctest::Approx(1.0).epsilon(0.02));
  // the apsides stand in two lines
  CHECK(sun_apsides.contains(QChar(0x0A)));
  CHECK_FALSE(mars_node.isEmpty());
  // the mean apogee in seconds, the true one in minutes
  CHECK(ag_mean_lon.contains('"'));
  CHECK_FALSE(ag_true_lon.isEmpty());
  CHECK_FALSE(ag_true_lon.contains('"'));
  CHECK(ag_mean_mark == "Mittel");
  CHECK(ag_true_mark == "Wahr");
  CHECK(gl_tag == "GL");
  CHECK_FALSE(gl_lon.isEmpty());
  // the yellow box marks only the form the chart works with
  const QColor yellow(0xFF, 0xFF, 0x00);
  CHECK((s.true_node ? true_node_back : mean_node_back) == yellow);
  CHECK((s.true_node ? mean_node_back : true_node_back) != yellow);
  CHECK((s.true_apogee ? true_ag_back : mean_ag_back) == yellow);
  CHECK((s.true_apogee ? mean_ag_back : true_ag_back) != yellow);
  CHECK(head_text.contains("Planeten-Koordinaten"));
  CHECK(head_text.contains("MOND-Apsiden : "));
  CHECK(question.contains("ZEIT VARIIEREN ?"));
}

TEST_CASE("ZUSATZ-PLANETEN-KOORDINATEN lists every extra body with its name") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  int rows = 0;
  QStringList names;
  QStringList longitudes;
  QString cupido_lat;
  QString chiron_ra;
  int table_height = 0;
  int rows_height = 0;
  {
    DialogDriver drive;
    drive.then([&](QDialog* d) {
      const auto* t = d->findChild<QTableWidget*>();
      REQUIRE(t != nullptr);
      rows = t->rowCount();
      for (int r = 0; r < rows; ++r) {
        names << cell(t, r, 8);
        longitudes << cell(t, r, 1);
        rows_height += t->rowHeight(r);
      }
      chiron_ra = cell(t, 0, 6);
      cupido_lat = cell(t, 12, 2);
      table_height = t->viewport()->height();
      d->reject();
    });
    MainWindowProbe::coordinate_table(*w, true);
    CHECK(drive.pending() == 0);
  }
  // the tester's order, the real bodies first, the invented ones last,
  // the Black Moon and the Glückspunkt stand on the first sheet
  CHECK(rows == 20);
  CHECK(names.value(0) == "Chiron            CH");
  CHECK(names.value(1) == "Quaoar            QU");
  CHECK(names.value(2) == "Xena              XE");
  CHECK(names.value(3) == "Ceres             CE");
  CHECK(names.value(7) == "Pholus            PH");
  CHECK(names.value(10) == "Komet Halley      HL");
  CHECK(names.value(11) == "Transpluto = Isis TP");
  CHECK(names.value(12) == "Cupido            CU");
  CHECK(names.value(19) == "Poseidon          PO");
  // QU, XE and PO carry their values, the tester found them missing
  for (const int r : {1, 2, 19}) {
    CHECK_FALSE(longitudes.value(r).isEmpty());
  }
  CHECK_FALSE(chiron_ra.isEmpty());
  // the Hamburg points carry no latitude
  CHECK(cupido_lat.isEmpty());
  // every row stands in the window, none hides below its edge
  CHECK(table_height >= rows_height);
}

TEST_CASE("ZEIT VARIIEREN walks the table and keeps the varied moment") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const double jd0 = MainWindowProbe::panel_jd(*w);
  double stepped = 0.0;
  {
    DialogDriver drive;
    drive.then([](QDialog* d) { press_later(d, Qt::Key_Space); })
        .then(DialogDriver::click("JA"))       // ZEIT VARIIEREN ?
        .then(DialogDriver::click("TAGE"))     // ZEIT-EINHEIT ?
        .then(DialogDriver::fill({"1"}, "OK")) // TAGE als BELIEBIGE ZAHL
        .then(DialogDriver::click("VOR"))      // RICHTUNG ?
        // the eingalp sheet, the first step follows it
        .then([](QDialog* d) { d->accept(); })
        .then([&w, &stepped](QDialog* d) {
          // the table again with the first step done, ESC ends the walk
          stepped = MainWindowProbe::panel_jd(*w);
          press_later(d, Qt::Key_Escape);
        })
        .then(DialogDriver::click("NEIN = ENDE"))
        .then(DialogDriver::click("JA"))  // VARIIERTE ZEIT in RADIX ÜBERNEHMEN ?
        .then([](QDialog* d) { d->reject(); });
    MainWindowProbe::coordinate_table(*w, false);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(stepped - jd0 == doctest::Approx(1.0).epsilon(1e-6));
  // JA keeps the varied moment in the record
  CHECK(MainWindowProbe::panel_jd(*w) - jd0 == doctest::Approx(1.0).epsilon(1e-6));
}

TEST_CASE("GRAD-LISTE G/H pages his list and sorts it on request") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  QStringList first_page;
  QStringList sorted_page;
  QString question;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click(" JA "))  // ZWISCHEN - HÄUSER HINZUNEHMEN ?
        .then([&](QDialog* d) {
          const auto* c = d->findChild<WheelWidget*>();
          REQUIRE(c != nullptr);
          for (const Primitive& p : c->display_list().items) {
            if (p.kind == Primitive::Kind::kText) {
              first_page << QString::fromStdString(p.text);
            }
          }
          // Space walks the pages, after the last one his question comes
          QTimer::singleShot(0, d, [d]() {
            for (int i = 0; i < 12 && QApplication::activeModalWidget() == d; ++i) {
              QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
              QApplication::sendEvent(d, &e);
            }
          });
        })
        .then([&question, &sorted_page](QDialog* d) {
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            question += l->text() + "|";
          }
          auto* view = qobject_cast<QDialog*>(d->parentWidget());
          DialogDriver::click(" JA ")(d);
          if (view != nullptr) {
            QTimer::singleShot(0, view, [view, &sorted_page]() {
              const auto* c = view->findChild<WheelWidget*>();
              for (const Primitive& p : c->display_list().items) {
                if (p.kind == Primitive::Kind::kText) {
                  sorted_page << QString::fromStdString(p.text);
                }
              }
              view->reject();
            });
          }
        });
    MainWindowProbe::degree_list(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  REQUIRE(!first_page.isEmpty());
  CHECK(first_page.join("|").contains("Grad-Liste |TESTFALL"));
  CHECK(first_page.contains(" 1"));
  CHECK(std::any_of(first_page.begin(), first_page.end(), [](const QString& t) { return t.endsWith(" H2"); }));
  CHECK(question.contains("Nach Länge SORTIEREN ?"));
  CHECK(sorted_page.contains("Gesamt -"));
}

TEST_CASE("ARABISCHE TEILE mark a conjunction with his yellow box like the fixed stars") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  const std::filesystem::path dir = std::filesystem::temp_directory_path() / "horcom_arabic_conjunction";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  MainWindowProbe::set_data_dir(*w, dir);
  const QString conjunction = QString::fromUtf8(aspect_glyph(star_aspect_family(kStarConjunction)));
  int rows = 0;
  int conjunctions = 0;
  int agree = 0;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("TRADITIONELL"))
        .then(DialogDriver::click("TABELLEN-AUSGABE"))
        .then([&](QDialog* d) {
          const auto* t = d->findChild<QTableWidget*>();
          REQUIRE(t != nullptr);
          for (int r = 0; r < t->rowCount(); ++r) {
            const QTableWidgetItem* asp = t->item(r, 3);
            if (asp == nullptr) {
              continue;
            }
            ++rows;
            const bool has = asp->text().contains(conjunction);
            const bool yellow = asp->background().style() != Qt::NoBrush && asp->background().color() == QColor(0xFF, 0xFF, 0x00);
            conjunctions += has ? 1 : 0;
            agree += has == yellow ? 1 : 0;
          }
          d->reject();
        });
    MainWindowProbe::arabic_table(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  std::filesystem::remove_all(dir);
  // the yellow box stands exactly on the rows with a conjunction
  REQUIRE(rows > 30);
  CHECK(conjunctions > 0);
  CHECK(agree == rows);
}

TEST_CASE("ARABISCHE TEILE defines an own point and puts it on the first row") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  const std::filesystem::path dir = std::filesystem::temp_directory_path() / "horcom_arabic_gui";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  MainWindowProbe::set_data_dir(*w, dir);
  QString first_name;
  QString first_formula;
  QStringList sorted_degrees;
  QString echo;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("TRADITIONELL"))
        .then(DialogDriver::click("NEU DEFINIEREN"))
        .then(DialogDriver::fill({"TESTPUNKT"}, "OK"))
        .then(DialogDriver::fill({"BEM"}, "OK"))
        .then(DialogDriver::pick_row(12))  // ASZENDENT
        .then(DialogDriver::pick_row(0))   // SONNE
        .then([&echo](QDialog* d) {
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            echo += l->text() + "|";
          }
          DialogDriver::pick_row(20)(d);   // HAUS NR.
        })
        .then(DialogDriver::click(" 4"))   // SPITZE HAUS NR.?
        .then(DialogDriver::click("NEIN")) // WeiterEN PUNKT definierEN ?
        .then(DialogDriver::click("TRADITIONELL"))
        .then(DialogDriver::click("TABELLEN-AUSGABE"))
        .then([&](QDialog* d) {
          const auto* t = d->findChild<QTableWidget*>();
          REQUIRE(t != nullptr);
          first_name = t->item(0, 0)->text();
          first_formula = t->item(0, 1)->text();
          QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
          QApplication::sendEvent(d, &e);
          for (int r = 0; r < t->rowCount(); ++r) {
            if (t->item(r, 2) != nullptr) {
              sorted_degrees << t->item(r, 2)->text();
            }
          }
          d->reject();
        });
    MainWindowProbe::arabic_table(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // his glanz echoes every term
  CHECK(echo.contains("AC = 1.Glied"));
  CHECK(echo.contains("SO = 2.Glied"));
  CHECK(first_name == "TESTPUNKT");
  CHECK(first_formula.startsWith("AC + "));
  CHECK(first_formula.contains("H4"));
  // Space sorted the rows by degree, 37 rows with longitudes
  CHECK(sorted_degrees.size() >= 35);
  const auto own = read_own_arabic(dir);
  REQUIRE(own.size() == 1);
  CHECK(own[0].terms[2].kind == 12);
  CHECK(own[0].terms[2].value == 4);
  std::filesystem::remove_all(dir);
}

TEST_CASE("INGRESSE draws his Sun year and pages a year on") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const bool parallax = MainWindowProbe::settings(*w).topocentric_parallax;
  QString title_first;
  QString title_next;
  QString aries;
  QString first_sign;
  QString window_title;
  int rows = 0;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("SONNE"));
    if (parallax) {
      drive.then(DialogDriver::click("OK"));  // Wegen PARALLAXE bitte Ereignis-Ort eingeben !
    }
    drive.then(DialogDriver::fill({"1993"}, "OK"))
        .then([&](QDialog* d) {
          const auto* t = d->findChild<QTableWidget*>();
          REQUIRE(t != nullptr);
          rows = t->rowCount();
          window_title = d->windowTitle();
          // the sign column the tester wanted at the far left
          first_sign = t->item(0, 0)->text();
          aries = t->item(0, 1)->text() + "|" + t->item(0, 2)->text();
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            if (l->text().contains("Ingresse")) {
              title_first = l->text();
            }
          }
          QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
          QApplication::sendEvent(d, &e);
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            if (l->text().contains("Ingresse")) {
              title_next = l->text();
            }
          }
          d->reject();
        });
    MainWindowProbe::ingress_table(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(rows == 12);
  CHECK(window_title == "INGRESSE PLANETEN-MC-AC");
  CHECK(first_sign.endsWith("WIDDER"));
  CHECK(title_first == " Ingresse der SONNE im Kalenderjahr 1993");
  CHECK(title_next == " Ingresse der SONNE im Kalenderjahr 1994");
  // the equinox of 1993 fell on March 20
  CHECK(aries.startsWith("20. 3.1993"));
  CHECK(aries.contains("h "));
}

TEST_CASE("INGRESSE of the MC fill three day columns and page three days") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  QString first_day;
  QString third_day;
  QString after_step;
  QString mc_length;
  int columns = 0;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("MC"))
        .then(DialogDriver::click("OK"))  // the record mask, date and place
        .then([&](QDialog* d) {
          const auto* t = d->findChild<QTableWidget*>();
          REQUIRE(t != nullptr);
          columns = t->columnCount();
          // the sign stands first, the three days and the Länge follow
          first_day = t->item(0, 1) != nullptr ? t->item(0, 1)->text() : QString();
          third_day = t->item(0, 3) != nullptr ? t->item(0, 3)->text() : QString();
          mc_length = t->item(0, 4) != nullptr ? t->item(0, 4)->text() : QString();
          QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
          QApplication::sendEvent(d, &e);
          after_step = t->item(0, 1) != nullptr ? t->item(0, 1)->text() : QString();
          d->reject();
        });
    MainWindowProbe::ingress_table(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(columns == 5);
  INFO(first_day.toStdString());
  // his datum$ with the two digit year, three days D to D+2
  CHECK(first_day.startsWith("10. 5.70"));
  CHECK(third_day.startsWith("12. 5.70"));
  CHECK(after_step.startsWith("13. 5.70"));
  // his el(14) = f(10) at the hit with the degree mark of grzemise, the
  // computed MC stands within a tenth of a second of 0 AR
  CHECK(mc_length.startsWith(QString::fromUtf8(" 0\xC2\xB0 ")));
  CHECK(mc_length.endsWith(" 00'00\""));
}

TEST_CASE("INGRESSE of the MOND page on from the PISCES ingress where plant left jd") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const bool parallax = MainWindowProbe::settings(*w).topocentric_parallax;
  const double jd = MainWindowProbe::panel_jd(*w);
  QString title_first;
  QString title_next;
  QString title_back;
  const auto title_of = [](QDialog* d) {
    for (const QLabel* l : d->findChildren<QLabel*>()) {
      if (l->text().contains("Ingresse")) {
        return l->text();
      }
    }
    return QString();
  };
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("MOND"));
    if (parallax) {
      drive.then(DialogDriver::click("OK"));  // Wegen PARALLAXE bitte Ereignis-Ort eingeben !
    }
    drive.then(DialogDriver::click("OK"))  // SUCH-DATUM ( MONAT ) EINGEBEN !
        .then([&](QDialog* d) {
          title_first = title_of(d);
          QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
          QApplication::sendEvent(d, &space);
          title_next = title_of(d);
          QKeyEvent back(QEvent::KeyPress, Qt::Key_R, Qt::NoModifier);
          QApplication::sendEvent(d, &back);
          title_back = title_of(d);
          d->reject();
        });
    MainWindowProbe::ingress_table(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  SearchContext ctx = MainWindowProbe::context(*w);
  ctx.settings.heliocentric = false;
  const Calendar cal = ctx.settings.calendar;
  const auto pisces = [&ctx](double at) { return sign_ingresses(at, body::kMoon, ctx)[11]; };
  const LongitudeCrossing first = pisces(jd);
  REQUIRE(first.ok);
  const QString head = " Ingresse des MONDES bis ca. dem Datum : ";
  CHECK(title_first == head + datum3_text(calendar_date(jd, cal)));
  // his ADD jd,29 counts from the PISCES ingress, the port added the
  // month to the search date before
  const double next = first.jd_ut + 29.0;
  CHECK(title_next == head + datum3_text(calendar_date(next, cal)));
  if (datum3_text(calendar_date(first.jd_ut, cal)) != datum3_text(calendar_date(jd, cal))) {
    CHECK(title_next != head + datum3_text(calendar_date(jd + 29.0, cal)));
  }
  // and SUB jd,28 from the PISCES ingress of the page shown
  const LongitudeCrossing second = pisces(next);
  REQUIRE(second.ok);
  CHECK(title_back == head + datum3_text(calendar_date(second.jd_ut - 28.0, cal)));
}

TEST_CASE("INGRESSE of the tester's bodies page one circuit of the zodiac") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const bool parallax = MainWindowProbe::settings(*w).topocentric_parallax;
  const double jd = MainWindowProbe::panel_jd(*w);
  QString title_next;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("MARS"));
    if (parallax) {
      drive.then(DialogDriver::click("OK"));
    }
    drive.then(DialogDriver::click("OK")).then([&](QDialog* d) {
      QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
      QApplication::sendEvent(d, &space);
      for (const QLabel* l : d->findChildren<QLabel*>()) {
        if (l->text().contains("Ingresse")) {
          title_next = l->text();
        }
      }
      d->reject();
    });
    MainWindowProbe::ingress_table(*w);
    CHECK(drive.pending() == 0);
  }
  const double tja = time_arguments(jd).tropical_year_days;
  const double circuit = body_period_days(body::kMars, tja);
  CHECK(circuit > tja);
  CHECK(title_next == " Ingresse von MARS bis ca. dem Datum : " +
                          datum3_text(calendar_date(jd + circuit, MainWindowProbe::settings(*w).calendar)));
}

TEST_CASE("PLANETEN-KOORDINATEN stands beyond the polar circle and names the inverted planets") {
  auto w = MainWindowProbe::make();
  AafRecord r = morning_birth();
  r.lat_deg = 70;
  MainWindowProbe::apply(*w, r);
  // his a60 leaves only the house routine on maxbreit, ko_ta still lists
  // every body, the sheet takes the AC and MC of the wheel
  MainWindowProbe::konsta(*w).plinv = 3;
  QString sun;
  QString node_vel;
  QString notes;
  {
    DialogDriver drive;
    drive.then([&](QDialog* d) {
      const auto* t = d->findChild<QTableWidget*>();
      REQUIRE(t != nullptr);
      sun = cell(t, 0, 1);
      node_vel = cell(t, 11, 3);
      for (const QLabel* l : d->findChildren<QLabel*>()) {
        notes += l->text() + "|";
      }
      d->reject();
    });
    MainWindowProbe::coordinate_table(*w, false);
    CHECK(drive.pending() == 0);
  }
  CHECK_FALSE(sun.trimmed().isEmpty());
  // the true node of the chart, not one differentiated around an unset epoch
  CHECK(node_vel.trimmed().toDouble() != 0.0);
  CHECK(std::abs(node_vel.trimmed().toDouble()) < 60.0);
  // the two lines under plinv answer where the inverse symbols come from
  CHECK(notes.contains("Invertiert : MA,SA,UR,NE,PL"));
  CHECK(notes.contains("KENNZEICHNUNG in VORGABEN DIREKTIONEN"));
}

TEST_CASE("INGRESSE of XENA page up to the last day of its ephemeris") {
  auto w = MainWindowProbe::make();
  // the table reckons XENA also where the panel leaves it out
  MainWindowProbe::preset_extras(*w, false, false, false);
  MainWindowProbe::apply(*w, morning_birth());
  const bool parallax = MainWindowProbe::settings(*w).topocentric_parallax;
  QString title_next;
  QStringList years_next;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("XENA"));
    if (parallax) {
      drive.then(DialogDriver::click("OK"));
    }
    drive.then(DialogDriver::fill({"", "1", "1", "2026"}, "OK")).then([&](QDialog* d) {
      const auto* t = d->findChild<QTableWidget*>();
      REQUIRE(t != nullptr);
      QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
      QApplication::sendEvent(d, &space);
      for (const QLabel* l : d->findChildren<QLabel*>()) {
        if (l->text().contains("Ingresse")) {
          title_next = l->text();
        }
      }
      // his datum3$ pads the date on the right
      for (int r = 0; r < t->rowCount(); ++r) {
        if (t->item(r, 1) != nullptr) {
          years_next << t->item(r, 1)->text().trimmed().right(4);
        }
      }
      d->reject();
    });
    MainWindowProbe::ingress_table(*w);
    CHECK(drive.pending() == 0);
  }
  INFO(title_next.toStdString() << " | " << years_next.join(",").toStdString());
  // his XENA file ends in 2201, a whole circuit of 557 years on the page
  // stops there instead of turning empty, the tester saw the dates end in
  // the 19th century
  CHECK(title_next.trimmed().right(4).toInt() >= 2200);
  CHECK(std::any_of(years_next.begin(), years_next.end(), [](const QString& y) { return y.toInt() > 2026; }));
  // the coming entries of his file, TAURUS, GEMINI and CANCER
  for (const char* year : {"2048", "2148", "2193"}) {
    CHECK(years_next.contains(year));
  }
}

TEST_CASE("the zodiac cells carry a rounded unit into the next sign like grze_0") {
  // 29 PS 59'59.6" rounds into Aries, the old cell printed 30 PS 00'00"
  const double late = norm_rad(-0.4 * kArcsecToRad);
  const std::unique_ptr<QTableWidgetItem> item(zodiac_item(late));
  CHECK(item->text().startsWith(" 0 "));
  CHECK(item->text().endsWith(" 00'00\""));
  CHECK(item->data(kSignRole).toInt() == 1);
  // the minutes form of the coordinate screen with his degree mark
  const std::unique_ptr<QTableWidgetItem> minutes(zodiac_item(norm_rad(-20.0 * kArcsecToRad), false, true));
  CHECK(minutes->text().startsWith(QString::fromUtf8(" 0\xC2\xB0 ")));
  CHECK(minutes->text().endsWith(" 00'"));
  CHECK(minutes->data(kSignRole).toInt() == 1);
  // the text forms of grze
  CHECK(zodiac_text(late, ZodiacForm::kGz2) == QString::fromUtf8(" 0\xC2\xB0" "AR  0' 0\""));
  const double leo = (135.0 + 30.0 / 60.0 + 20.0 / 3600.0) * kDegToRad;
  CHECK(zodiac_text(leo, ZodiacForm::kGz0) == QString::fromUtf8("15\xC2\xB0LE30'"));
  CHECK(zodiac_text(leo, ZodiacForm::kGz1) == QString::fromUtf8("15\xC2\xB0 LE 30'"));
  CHECK(zodiac_text(leo, ZodiacForm::kGz2) == QString::fromUtf8("15\xC2\xB0LE 30'20\""));
  CHECK(zodiac_text(leo, ZodiacForm::kGz8) == "15 LE 30");
  // his grze carried only the seconds and rounded the minutes with CINT
  // afterwards, 15 LE 59'40" read "15° LE 60'" in gz1$. The minute now
  // carries into the degree
  const double late_minute = (135.0 + 59.0 / 60.0 + 40.0 / 3600.0) * kDegToRad;
  CHECK(zodiac_text(late_minute, ZodiacForm::kGz1) == QString::fromUtf8("16\xC2\xB0 LE  0'"));
  CHECK(zodiac_text(late_minute, ZodiacForm::kGz1) != QString::fromUtf8("15\xC2\xB0 LE 60'"));
}

TEST_CASE("PLANETEN-KOORDINATEN shows his haust box of the cusps") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  QString box;
  {
    DialogDriver drive;
    drive.then([&box](QDialog* d) {
      const auto* l = d->findChild<QLabel*>("cuspBox");
      REQUIRE(l != nullptr);
      box = l->isVisibleTo(d) ? l->text() : QString();
      d->reject();
    });
    MainWindowProbe::coordinate_table(*w, false);
    CHECK(drive.pending() == 0);
  }
  const QStringList lines = box.split(QChar(0x0A));
  REQUIRE(lines.size() == 8);
  CHECK(lines[0] == QString::fromUtf8("Häuserspitzen"));
  CHECK(lines[1].startsWith("("));
  const Chart& c = MainWindowProbe::chart(*w);
  // bes111 skips from H3 to the MC, the opposite cusps stay out
  CHECK(lines[2] == " AC:" + zodiac_text(c.houses.cusp[1], ZodiacForm::kGz2));
  CHECK(lines[3] == "H 2 :" + zodiac_text(c.houses.cusp[2], ZodiacForm::kGz2));
  CHECK(lines[4] == "H 3 :" + zodiac_text(c.houses.cusp[3], ZodiacForm::kGz2));
  CHECK(lines[5] == " MC:" + zodiac_text(c.houses.cusp[10], ZodiacForm::kGz2));
  CHECK(lines[6] == "H11 :" + zodiac_text(c.houses.cusp[11], ZodiacForm::kGz2));
  CHECK(lines[7] == "H12 :" + zodiac_text(c.houses.cusp[12], ZodiacForm::kGz2));
}

TEST_CASE("ZEIT VARIIEREN JA writes the varied moment into the RADIX slot of the same SATZ") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const double jd0 = MainWindowProbe::panel_jd(*w);
  // a SOLAR slot holds the panel like after a return chart
  MainWindowProbe::store_solar(*w, "SOLAR 1970");
  MainWindowProbe::activate_solar(*w, 0);
  {
    DialogDriver drive;
    drive.then([](QDialog* d) { press_later(d, Qt::Key_Space); })
        .then(DialogDriver::click("JA"))        // ZEIT VARIIEREN ?
        .then(DialogDriver::click("TAGE"))      // ZEIT-EINHEIT ?
        .then(DialogDriver::fill({"1"}, "OK"))  // TAGE als BELIEBIGE ZAHL
        .then(DialogDriver::click("VOR"))       // RICHTUNG ?
        .then([](QDialog* d) { d->accept(); })  // the eingalp sheet
        .then([](QDialog* d) { press_later(d, Qt::Key_Escape); })
        .then(DialogDriver::click("NEIN = ENDE"))
        .then(DialogDriver::click("JA"))  // VARIIERTE ZEIT in RADIX ÜBERNEHMEN ?
        .then([](QDialog* d) { d->reject(); });
    MainWindowProbe::coordinate_table(*w, false);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // his od = 1 before @merk, the RADIX SATZ1 takes the moment and holds
  // the panel, the SOLAR slot keeps its own
  CHECK(MainWindowProbe::active_slot(*w) == 0);
  const std::optional<AafRecord> radix = MainWindowProbe::slot(*w, 0);
  REQUIRE(radix.has_value());
  CHECK(radix->jd - jd0 == doctest::Approx(1.0).epsilon(1e-6));
  const std::optional<AafRecord> solar = MainWindowProbe::solar_slot(*w, 0);
  REQUIRE(solar.has_value());
  CHECK(solar->jd == doctest::Approx(jd0).epsilon(1e-9));
  CHECK(MainWindowProbe::panel_jd(*w) - jd0 == doctest::Approx(1.0).epsilon(1e-6));
}

TEST_CASE("FIX-STERN-POSITIONEN runs his stella in Apparent 2 with the stelt line") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  MainWindowProbe::konsta(*w).appa = 1;
  // a Fixpunkt on ALDEBARAN, his aa& = 0 lets stelk match it
  const std::vector<StarRow> plain = fixed_stars(MainWindowProbe::chart(*w), 1.0);
  const auto aldebaran = std::find_if(plain.begin(), plain.end(), [](const StarRow& r) { return r.name == "ALDEBARAN"; });
  REQUIRE(aldebaran != plain.end());
  MainWindowProbe::fixpunkt(*w) = aldebaran->la;
  QString title;
  QStringList heads;
  QString footer;
  QString aldebaran_aspects;
  QColor marked_back;
  QColor marked_ink;
  QColor conjunction_back;
  {
    DialogDriver drive;
    drive.then([&](QDialog* d) {
      title = d->windowTitle();
      const auto* t = d->findChild<QTableWidget*>();
      REQUIRE(t != nullptr);
      for (int c = 0; c < t->columnCount(); ++c) {
        heads << t->horizontalHeaderItem(c)->text();
      }
      for (int r = 0; r < t->rowCount(); ++r) {
        if (cell(t, r, 0) == "ALDEBARAN") {
          aldebaran_aspects = cell(t, r, 2);
          marked_back = t->item(r, 0)->background().color();
          marked_ink = t->item(r, 0)->foreground().color();
          conjunction_back = t->item(r, 2)->background().color();
        }
      }
      if (const auto* l = d->findChild<QLabel*>("stelt")) {
        footer = l->text();
      }
      d->reject();
    });
    MainWindowProbe::fixed_star_table(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(title == "FIX-STERN-POSITIONEN");
  CHECK(heads == QStringList{"Stern - Name", "Ekl.Länge", "Aspekte", "Qualität", "Astron.Name", "Breite", "Rekt.", "Dekl.",
                             "D/LJ"});
  // his appa& = 2 whatever the profile says
  CHECK(footer.contains("|Eph.:App.2,"));
  CHECK(footer.startsWith("TESTFALL"));
  // sol$(od,ze) names the kind of chart, the name stands only once
  CHECK(footer.contains("|RADIX|Eph.:"));
  CHECK(footer.count("TESTFALL") == 1);
  // the Fixpunkt conjunction with his sprite, not the letter K
  // stelk puts the sprite first and the tag behind it
  CHECK(aldebaran_aspects.startsWith(QString::fromUtf8(aspect_glyph(star_aspect_family(kStarConjunction))) + "FP"));
  // his deftextcol(3), red on cyan
  CHECK(marked_back == QColor(0x00, 0xFF, 0xFF));
  CHECK(marked_ink == QColor(0xFF, 0x00, 0x00));
  // the conjunction wears his yellow box, the tester's wish
  CHECK(conjunction_back == QColor(0xFF, 0xFF, 0x00));
}

TEST_CASE("VORGABEN EPHEMERIDE runs his ave as a chain") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  MainWindowProbe::fixpunkt(*w) = -1.0;
  MainWindowProbe::konsta(*w).appa = 1;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("MODUS DER"))
        .then(DialogDriver::click("WAHR = GEOMETRISCHE POSITION"))
        // INC as&, the MONDKNOTEN box follows without the topic list
        .then(DialogDriver::click("EXIT"));
    MainWindowProbe::vorgaben_ephemeride(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // his appa$ = "Wahr", the port wrote "Wahre"
  CHECK(MainWindowProbe::konsta(*w).appa == 3);
  CHECK(MainWindowProbe::konsta(*w).appa_name == "Wahr");
  QStringList kept_rows;
  QStringList topics;
  {
    DialogDriver drive;
    drive
        .then([&topics](QDialog* d) {
          for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            topics << b->text();
          }
          DialogDriver::click("FIXPUNKT als 'PLANET'")(d);
        })
        .then(DialogDriver::click("NEU DEFINIEREN"))
        .then(DialogDriver::pick_row(4))                     // LÖWE
        .then(DialogDriver::fill({"12", "30", "0"}, "OK"));  // his input_grmise_zod, the last topic ends the chain
    MainWindowProbe::vorgaben_ephemeride(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // the tables show both distances, the question of his last topic is gone
  CHECK_FALSE(topics.join("|").contains("ENTFERNUNGSWERTE"));
  CHECK(MainWindowProbe::fixpunkt(*w) * kRadToDeg == doctest::Approx(132.5));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("FIXPUNKT als 'PLANET'")).then([&kept_rows](QDialog* d) {
      for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
        kept_rows << b->text();
      }
      DialogDriver::click("BEIBEHALTEN")(d);
    });
    MainWindowProbe::vorgaben_ephemeride(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // his fixp_def offers the defined point by its gz0$
  CHECK(kept_rows.contains(QString::fromUtf8("FIXPUNKT 12\xC2\xB0LE30' BEIBEHALTEN")));
  CHECK(kept_rows.contains(QString::fromUtf8("FIXPUNKT 12\xC2\xB0LE30' LÖSCHEN")));
  CHECK(MainWindowProbe::fixpunkt(*w) * kRadToDeg == doctest::Approx(132.5));
}

TEST_CASE("the table zoom lets its table close with the focus in it") {
  QWidget window;
  auto* table = new QTableWidget(3, 2, &window);
  new TableZoom(table, true);
  table->setFocus();
  window.show();
  QApplication::processEvents();
  // the destructor clears the focus through the zoom's filter while the
  // table is only a QWidget any more, the sanitizer build checks that the
  // filter never treats it as a table then
  REQUIRE(table->hasFocus());
  delete table;
  CHECK(window.findChild<QTableWidget*>() == nullptr);
}
