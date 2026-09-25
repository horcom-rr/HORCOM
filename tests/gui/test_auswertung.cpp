// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QAbstractButton>
#include <QComboBox>
#include <QDateEdit>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QSpinBox>
#include <QTableWidget>
#include <QTimer>
#include <cmath>
#include <filesystem>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/progressions.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/place_file.hpp"
#include "probe.hpp"
#include "wheel_widget.hpp"

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

void click_button(QDialog* d, const QString& caption) {
  for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
    if (b->text().contains(caption)) {
      b->click();
      return;
    }
  }
}

}  // namespace

TEST_CASE("the day chart lands on the chosen day and steps on a day") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const double day0 = julian_day({24, 9, 2026, 0, 0.0});
  double first = 0.0;
  DialogDriver drive;
  drive.then(DialogDriver::click("JA"))                               // EREIGNIS-Ort = GEBURTS-Ort ?
      .then(DialogDriver::fill({"", "24", "9", "2026"}, "OK"))        // DATUM EINGEBEN !
      .then([&w, &first](QDialog* d) {                                // WEITERES TAGES-HOROSKOP ?
        first = MainWindowProbe::panel_jd(*w);
        click_button(d, "NÄCHSTER TAG");
      })
      .then(DialogDriver::click("( = ENDE )"));
  MainWindowProbe::day_chart(*w);
  INFO(drive.titles().join(" | ").toStdString());
  CHECK(drive.unexpected() == 0);
  // a noon seed once pushed a morning birth onto the next day
  CHECK(first >= day0);
  CHECK(first < day0 + 1.0);
  const double second = MainWindowProbe::panel_jd(*w);
  CHECK(second - first == doctest::Approx(1.0).epsilon(0.01));
}

TEST_CASE("the progression reads the event at the birth clock and names both dates") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const Chart radix = MainWindowProbe::chart(*w);
  DialogDriver drive;
  drive.then(DialogDriver::click("JA"))
      .then(DialogDriver::fill({"", "1", "6", "2000"}, "OK"))
      .then(DialogDriver::click("UT = RADIX-UT"))
      .then(DialogDriver::click("ENDE = Daten"));
  MainWindowProbe::progression(*w);
  CHECK(drive.unexpected() == 0);
  const SearchContext ctx = MainWindowProbe::context(*w);
  const double event = event_at_radix_clock(radix, julian_day({1, 6, 2000, 0, 0.0}), true);
  const ProgressedMoment m = progressed_moment(radix, event, ProgressionMode::kRadixClock, ctx);
  REQUIRE(m.ok);
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - m.jd_ut) * 86400.0 < 1.0);
  const ClassicSheetText t = MainWindowProbe::sheet(*w);
  CHECK(t.pair_moment1 == "Ereig: 01.06.2000");
  CHECK(t.pair_moment2 == "RADIX: 10.05.1970");
}

TEST_CASE("the lunar steps on from the lunar on screen") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const SearchContext ctx = MainWindowProbe::context(*w);
  const double natal_moon = MainWindowProbe::chart(*w).b[body::kMoon].el;
  double from_date = 0.0;
  DialogDriver drive;
  drive.then([&w, &from_date](QDialog* d) {
    // the date lunar first, then one step into the future
    click_button(d, "Nächstes Lunar");
    from_date = MainWindowProbe::panel_jd(*w);
    click_button(d, "Nächstes Lunar");
    click_button(d, "OK");
  });
  MainWindowProbe::lunar(*w);
  CHECK(drive.unexpected() == 0);
  const double stepped = MainWindowProbe::panel_jd(*w);
  // one lunation later, never a jump back to the first lunar of the life
  CHECK(stepped - from_date == doctest::Approx(27.32).epsilon(0.02));
  const BodyLongitude moon = body_longitude(stepped, body::kMoon, ctx);
  double d = std::abs(norm_rad(moon.el) - norm_rad(natal_moon));
  if (d > kPi) {
    d = kTwoPi - d;
  }
  CHECK(d * kRadToDeg * 3600.0 < 30.0);
}

TEST_CASE("the lunar list starts at the BEGINN-DATUM") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const double start = julian_day({1, 1, 2020, 0, 0.0});
  std::vector<double> dates;
  DialogDriver drive;
  // IF par = 1 : @ort_wahl(0,0), his profile computes topocentric
  drive.then(DialogDriver::click("JA")).then(DialogDriver::fill({"", "1", "1", "2020"}, "OK")).then([&dates](QDialog* d) {
    // sol_lun_tabelle, the moments of his three columns
    for (const QVariant& v : d->property("moments").toList()) {
      dates.push_back(v.toDouble());
    }
    d->reject();
  });
  MainWindowProbe::return_list(*w, true);
  CHECK(drive.unexpected() == 0);
  REQUIRE(dates.size() == 84);
  // SUB jd,29 and ADD jd,33, the first lunar is the last one before four
  // days past the start, the search adds his half step of one day
  CHECK(dates.front() <= start + 5.0);
  CHECK(dates.front() > start + 5.0 - 27.6);
  // his a16_1 put the birthday 10 May over the entered day and month, the
  // list then opened in May 2020
  CHECK(dates.front() < julian_day({1, 2, 2020, 0, 0.0}));
  for (std::size_t i = 1; i < dates.size(); ++i) {
    CHECK(dates[i] - dates[i - 1] == doctest::Approx(27.32).epsilon(0.02));
  }
}

namespace {

// the texts of a plain canvas in a dialog
QStringList canvas_texts(QDialog* d) {
  QStringList out;
  if (const auto* w = d->findChild<WheelWidget*>()) {
    for (const Primitive& p : w->display_list().items) {
      if (p.kind == Primitive::Kind::kText) {
        out << QString::fromStdString(p.text);
      }
    }
  }
  return out;
}

void press_key(QDialog* d, int key) {
  QKeyEvent e(QEvent::KeyPress, key, Qt::NoModifier);
  QApplication::sendEvent(d, &e);
}

}  // namespace

TEST_CASE("DYNAMOGRAMM walks his question chain, the single arcs and the intervals") {
  auto w = MainWindowProbe::make();
  AafRecord r;
  r.surname = "TESTFALL";
  r.day = 13;
  r.month = 10;
  r.year = 1992;
  r.hour = 3;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 19;
  MainWindowProbe::apply(*w, r);
  QStringList arc_texts;
  QStringList radix_pass;
  QStringList mutual_pass;
  QStringList graph_texts;
  QStringList second_graph;
  QStringList interval;
  QString arcs_title;
  QStringList pass_titles;
  // one screen per pass of his analysis, Space shows every arc of the
  // pass at once and the second Space goes on
  const auto pass = [&pass_titles](QStringList* texts) {
    return [&pass_titles, texts](QDialog* d) {
      pass_titles << d->windowTitle();
      press_key(d, Qt::Key_Space);
      if (texts != nullptr) {
        *texts = canvas_texts(d);
      }
      press_key(d, Qt::Key_Space);
    };
  };
  DialogDriver drive;
  drive.then(DialogDriver::fill({"20"}, "OK"))
      .then(DialogDriver::click(" NEIN "))  // NEGATIVE ( REGRESSIVE ) ZEIT-RICHTUNG
      .then(DialogDriver::click(" NEIN "))  // MOND LAUFEND
      .then(DialogDriver::click("JA"))      // EINZEL - BÖGEN BEOBACHTEN ?
      .then(DialogDriver::click("COSINUS-BÖGEN"))
      .then(pass(&radix_pass))   // asp_analy_rad
      .then(pass(&mutual_pass))  // asp_analy_mund
      .then([&](QDialog* d) {
        graph_texts = canvas_texts(d);
        press_key(d, Qt::Key_Space);
      })
      .then([&interval](QDialog* d) {
        for (const QLabel* l : d->findChildren<QLabel*>()) {
          interval << l->text();
        }
        DialogDriver::click("NÄCHSTES")(d);
      })
      .then(pass(nullptr))
      .then(pass(nullptr))
      .then([&second_graph](QDialog* d) {
        second_graph = canvas_texts(d);
        d->reject();
      });
  MainWindowProbe::dynamogram_view(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  REQUIRE(pass_titles.size() == 4);
  arcs_title = pass_titles.front();
  arc_texts = radix_pass + mutual_pass;
  CHECK(arcs_title == " DYNAMOGRAMM nach KRAFFT-GOERNER | WEITER mit LEERTASTE");
  CHECK(arc_texts.join(QChar(0x0A)).contains("Beginn - LEBENS-Jahr = 20"));
  CHECK(arc_texts.contains("LEBENS-Jahre"));
  // each pass keeps to its own arcs
  CHECK_FALSE(radix_pass.join(QChar(0x0A)).contains("untereinander"));
  CHECK_FALSE(mutual_pass.join(QChar(0x0A)).contains("RADIX-Faktoren"));
  const QString all = graph_texts.join(QChar(0x0A));
  CHECK(all.contains("DYNAMOGRAMM nach K.E.KRAFFT / F.G.GOERNER"));
  // his textzent centres TRIM$(tex$), the trailing blanks of datum3$ go
  CHECK(all.contains("TESTFALL |  |Geb.- Datum: 13.10.1992"));
  CHECK(all.contains(" 1 Existentielle Situation "));
  CHECK(all.contains(" 2 Grundstimmung "));
  CHECK(all.contains(" Resultierende Energie "));
  CHECK(all.contains(" MITTEL über 50 Tage ( = Jahre ) = "));
  // twenty years after the October 1992 birth the window opens in 2012
  CHECK(graph_texts.contains("2013"));
  CHECK(interval.join(QChar(0x0A)).contains("NÄCHSTES INTERVALL ?"));
  CHECK(interval.join(QChar(0x0A)).contains("TESTFALL 13.10. 1992 | "));
  // NÄCHSTES moves the window four years on
  CHECK(second_graph.contains("2017"));
}

TEST_CASE("TERRAR takes the place of SOLAR under hrg! and lists his table") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  MainWindowProbe::helio(*w, true);
  QString menu_label;
  {
    DialogDriver drive;
    MainWindow* main = w.get();
    drive.then([main](QDialog* d) {  // GEWÜNSCHTES KALENDER-JAHR, then his wart
          QTimer::singleShot(400, main, [main]() {
            QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
            QApplication::sendEvent(main, &e);
          });
          DialogDriver::fill({"2020"}, "OK")(d);
        })
        .then([&menu_label](QDialog* d) {  // WEITERES SOLAR SUCHEN ?
          menu_label = d->windowTitle();
          DialogDriver::click("Weiter ( = ENDE )")(d);
        });
    MainWindowProbe::solar(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
  }
  // the Earth stands where it stood at the birth, heliocentric
  const Chart& now = MainWindowProbe::chart(*w);
  CHECK(now.b[body::kMoon].present);
  CHECK(MainWindowProbe::banner_record(*w).endsWith(".TERRAR"));
  QStringList texts;
  std::vector<double> moments;
  {
    DialogDriver drive;
    drive.then(DialogDriver::fill({"2000"}, "OK")).then([&](QDialog* d) {
      texts = canvas_texts(d);
      for (const QVariant& v : d->property("moments").toList()) {
        moments.push_back(v.toDouble());
      }
      d->reject();
    });
    MainWindowProbe::return_list(*w, false);
    CHECK(drive.pending() == 0);
  }
  REQUIRE(moments.size() == 84);
  const QString all = texts.join("|");
  CHECK(all.contains(" TERRAR - Zeitpunkte für"));
  CHECK(all.contains(" Heliozentrische "));
  CHECK(all.contains("Zeit in UT = GMT"));
  // one terrar a year
  for (std::size_t i = 1; i < moments.size(); ++i) {
    CHECK(moments[i] - moments[i - 1] == doctest::Approx(365.25).epsilon(0.01));
  }
}

TEST_CASE("PLANETAR searches by DATUM by default and walks back over the passages") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const Chart radix = MainWindowProbe::chart(*w);
  QString menu;
  {
    DialogDriver drive;
    drive
        .then([](QDialog* d) {  // his body list and SUCHEN mit, DATUM preset
          const QList<QComboBox*> combos = d->findChildren<QComboBox*>();
          REQUIRE(combos.size() >= 2);
          combos[0]->setCurrentIndex(combos[0]->findText("MARSAR"));
          CHECK(combos[1]->currentText() == "DATUM");
          click_button(d, "OK");
        })
        .then(DialogDriver::fill({"", "1", "1", "2000"}, "OK"))  // SUCH-DATUM ?  UMLAUFZEIT
        .then([&menu](QDialog* d) {  // ZEIT ÜBERNEHMEN oder WEITERE PUNKTE SUCHEN ?
          menu = d->windowTitle();
          for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            menu += "|" + b->text();
          }
          click_button(d, "LÄUFIG");
        })
        .then([](QDialog* d) { click_button(d, "ZEIT-WERT ÜBERNEHMEN"); });
    MainWindowProbe::planetar(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(menu.contains("ZEIT ÜBERNEHMEN oder WEITERE PUNKTE SUCHEN ?"));
  CHECK(menu.contains("NÄCHSTES DIREKTLÄUFIGES MARSAR in VERGANGENHEIT SUCHEN ?"));
  // Mars stands on its radix place again, the count of the DATUM before
  // the first of January 2000, Mars returns every 687 days
  const Chart& now = MainWindowProbe::chart(*w);
  double d = norm_rad(now.b[body::kMars].el - radix.b[body::kMars].el);
  d = std::min(d, kTwoPi - d);
  CHECK(d * kRadToDeg < 0.05);
  CHECK(MainWindowProbe::banner_record(*w) == "15.MARSAR");
}

namespace {

// a synthetic preferred place far west of the birth, answered by the
// VORZUGSORT button of his ort_wahl box
constexpr double kFarLon = -74.0;
constexpr double kFarLat = 40.7;

std::filesystem::path far_place_dir(const char* tag) {
  const std::filesystem::path dir = std::filesystem::temp_directory_path() / tag;
  std::filesystem::create_directories(dir);
  REQUIRE(write_preferred_place(dir / "ort.ext", {kFarLon, kFarLat, "FERNORT"}));
  return dir;
}

double arcsec_apart(double a, double b) {
  double d = std::abs(norm_rad(a) - norm_rad(b));
  if (d > kPi) {
    d = kTwoPi - d;
  }
  return d * kRadToDeg * 3600.0;
}

// the birth moment and its search context at the birth place
SearchContext radix_context(const MainWindow& w) {
  SearchContext ctx = MainWindowProbe::context(w);
  ctx.base = MainWindowProbe::radix_input(w);
  return ctx;
}

SearchContext at_far_place(SearchContext ctx) {
  ctx.base.lon_deg_east = kFarLon;
  ctx.base.lat_deg = kFarLat;
  return ctx;
}

// answers his wart with Space once the main window waits for the key
void release_wart(MainWindow* main) {
  auto* poll = new QTimer(main);
  QObject::connect(poll, &QTimer::timeout, main, [main, poll]() {
    if (MainWindowProbe::wart_item(*main) != 0) {
      poll->stop();
      poll->deleteLater();
      QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
      QApplication::sendEvent(main, &e);
    }
  });
  poll->start(20);
}

}  // namespace

TEST_CASE("SOLAR searches at the event place and steps in one SOLAR row") {
  auto w = MainWindowProbe::make();
  AafRecord r = morning_birth();
  r.place = "TESTORT";
  MainWindowProbe::apply(*w, r);
  const std::filesystem::path dir = far_place_dir("horcom_solar_place");
  MainWindowProbe::set_data_dir(*w, dir);
  REQUIRE(MainWindowProbe::settings(*w).topocentric_parallax);
  const SearchContext birth_ctx = radix_context(*w);
  const double birth = julian_day(birth_ctx.base.date_ut);
  const double natal = body_longitude(birth, body::kSun, birth_ctx).el;
  const LongitudeCrossing far = solar_return(birth_ctx.base.date_ut, natal, 2002, at_far_place(birth_ctx));
  const LongitudeCrossing home = solar_return(birth_ctx.base.date_ut, natal, 2002, birth_ctx);
  REQUIRE(far.ok);
  REQUIRE(home.ok);
  {
    DialogDriver drive;
    drive
        .then([](QDialog* d) {
          // the nested ort_wahl box runs from the dialog's own loop
          QTimer::singleShot(0, d, [d]() {
            click_button(d, "Ort wählen");
            d->findChild<QSpinBox*>()->setValue(2000);
            click_button(d, "Nächstes Jahr");
            click_button(d, "Nächstes Jahr");
            click_button(d, "OK");
          });
        })
        .then(DialogDriver::click("VORZUGSORT"));
    MainWindowProbe::solar(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // his solnummer, the years since the birth before the SOLAR
  CHECK(MainWindowProbe::banner_record(*w) == "32.SOLAR");
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - far.jd_ut) * 86400.0 < 2.0);
  // the topocentric Sun of the birth place crosses at another moment
  if (std::abs(far.jd_ut - home.jd_ut) * 86400.0 > 4.0) {
    CHECK(std::abs(MainWindowProbe::panel_jd(*w) - home.jd_ut) * 86400.0 > 2.0);
  }
  CHECK(MainWindowProbe::lon(*w) == doctest::Approx(kFarLon));
  // the radix keeps its place, the button once renamed the birth record
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  CHECK(MainWindowProbe::slot(*w, 0)->place == "TESTORT");
  CHECK(MainWindowProbe::radix_input(*w).lon_deg_east == doctest::Approx(11.0 + 35.0 / 60.0));
  // sol$(2,ze), the three solars went into the row of the radix number
  CHECK(MainWindowProbe::active_solar(*w) == 0);
  CHECK(MainWindowProbe::solar_slot(*w, 0).has_value());
  for (int i = 1; i < 5; ++i) {
    CHECK_FALSE(MainWindowProbe::solar_slot(*w, i).has_value());
  }
  std::filesystem::remove_all(dir);
}

TEST_CASE("the LUNAR list reads the radix Moon of the birth place") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const std::filesystem::path dir = far_place_dir("horcom_lunar_list_place");
  MainWindowProbe::set_data_dir(*w, dir);
  std::vector<double> dates;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("VORZUGSORT"))
        .then(DialogDriver::fill({"", "1", "1", "2020"}, "OK"))
        .then([&dates](QDialog* d) {
          for (const QVariant& v : d->property("moments").toList()) {
            dates.push_back(v.toDouble());
          }
          d->reject();
        });
    MainWindowProbe::return_list(*w, true);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  REQUIRE(dates.size() >= 3);
  const SearchContext birth_ctx = radix_context(*w);
  const SearchContext far_ctx = at_far_place(birth_ctx);
  const double birth = julian_day(birth_ctx.base.date_ut);
  const double natal_home = body_longitude(birth, body::kMoon, birth_ctx).el;
  const double natal_far = body_longitude(birth, body::kMoon, far_ctx).el;
  // the parallax moves the Moon of the far place well apart
  CHECK(arcsec_apart(natal_home, natal_far) > 60.0);
  // pz = plz(1,ze,2), the Moon seen at the event place comes back to the
  // radix Moon of the birth place
  for (std::size_t i = 0; i < 3; ++i) {
    CHECK(arcsec_apart(body_longitude(dates[i], body::kMoon, far_ctx).el, natal_home) < 30.0);
  }
  std::filesystem::remove_all(dir);
}

TEST_CASE("a click on the SOLAR list takes the row under the pointer") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  std::vector<double> moments;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("JA"))
        .then(DialogDriver::fill({"2000"}, "OK"))
        .then([&moments](QDialog* d) {
          for (const QVariant& v : d->property("moments").toList()) {
            moments.push_back(v.toDouble());
          }
          auto* canvas = d->findChild<WheelWidget*>();
          REQUIRE(canvas != nullptr);
          // the third row stands centred on 46 + 3 * 14 less half its height
          const QPointF at = canvas->from_canvas({100.0, 80.0});
          QMouseEvent press(QEvent::MouseButtonPress, at, canvas->mapToGlobal(at), Qt::LeftButton, Qt::LeftButton,
                            Qt::NoModifier);
          QApplication::sendEvent(canvas, &press);
        });
    MainWindowProbe::return_list(*w, false);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  REQUIRE(moments.size() >= 4);
  // the old reading took the row below, the 2003 solar
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - moments[2]) * 86400.0 < 2.0);
  CHECK(MainWindowProbe::banner_record(*w) == "32.SOLAR");
}

TEST_CASE("LUNAR searches by number like his SUCHEN mit box") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const double birth = julian_day(MainWindowProbe::radix_input(*w).date_ut);
  const auto by_number = [](int n) {
    return [n](QDialog* d) {
      const QList<QComboBox*> combos = d->findChildren<QComboBox*>();
      REQUIRE(!combos.isEmpty());
      CHECK(combos[0]->currentText() == "DATUM");
      combos[0]->setCurrentIndex(combos[0]->findText("NR. ZUKUNFT"));
      d->findChild<QSpinBox*>()->setValue(n);
      click_button(d, "OK");
    };
  };
  {
    DialogDriver drive;
    drive.then(by_number(0));
    MainWindowProbe::lunar(*w);
    CHECK(drive.unexpected() == 0);
  }
  //RR NR. 0 ENTSPRICHT der RADIX
  CHECK(MainWindowProbe::banner_record(*w) == "0.LUNAR");
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - birth) * 86400.0 < 60.0);
  {
    DialogDriver drive;
    drive.then(by_number(1));
    MainWindowProbe::lunar(*w);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::banner_record(*w) == "1.LUNAR");
  CHECK(MainWindowProbe::panel_jd(*w) - birth == doctest::Approx(27.32).epsilon(0.02));
}

TEST_CASE("LUNAR by date names the lunar it found") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const SearchContext ctx = radix_context(*w);
  const double birth = julian_day(ctx.base.date_ut);
  const double natal = body_longitude(birth, body::kMoon, ctx).el;
  const LongitudeCrossing a = lunar_return(julian_day({1, 3, 2020, 0, 0.0}), natal, ctx);
  REQUIRE(a.ok);
  const LongitudeCrossing b = lunar_return(a.jd_ut + 29.0, natal, ctx);
  REQUIRE(b.ok);
  // a search date two to three days before the next lunar, at 0h UT
  const double day0 = std::floor(b.jd_ut - 2.0 - 0.5) + 0.5;
  const CalendarDate d = calendar_date(day0);
  const int k = static_cast<int>(std::lround((a.jd_ut - birth) / kTropicalMonthDays));
  {
    DialogDriver drive;
    drive.then([&d](QDialog* dlg) {
      dlg->findChild<QDateEdit*>()->setDate(QDate(d.year, d.month, d.day));
      click_button(dlg, "OK");
    });
    MainWindowProbe::lunar(*w);
    CHECK(drive.unexpected() == 0);
  }
  // the lunar before the date
  CHECK(std::abs(MainWindowProbe::panel_jd(*w) - a.jd_ut) * 86400.0 < 5.0);
  CHECK(MainWindowProbe::banner_record(*w) == QString("%1.LUNAR").arg(k));
  // his solnummer counted from the search date and named the next lunar
  CHECK(static_cast<int>(std::trunc((5.0 + day0 - birth) / kTropicalMonthDays)) == k + 1);
}

TEST_CASE("the single arc screen closes every arc on the axis") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  Dynamogram d;
  DynamogramArc sun;
  sun.pl = body::kSun;
  sun.rd = body::kMars;
  sun.i1 = kDynamogramWindowStart + 100;
  sun.i2 = kDynamogramWindowStart + 110;
  sun.bog.assign(11, 20.0);
  DynamogramArc mars = sun;
  mars.pl = body::kMars;
  mars.rd = body::kSun;
  mars.radix = false;
  d.arcs = {sun, mars};
  const auto arc_lines = [](const DisplayList& dl, Rgb c) {
    std::vector<Primitive> out;
    for (const Primitive& p : dl.items) {
      if (p.kind == Primitive::Kind::kLine && p.color == c && p.x1 >= 140.0 && p.x1 < 440.0 && p.y1 < 219.0) {
        out.push_back(p);
      }
    }
    return out;
  };
  // FOR i& = i1& TO i2&, eleven samples draw eleven segments, the last
  // one falls back to the empty sample behind the arc
  const DisplayList first = MainWindowProbe::dynamo_arc_screen(*w, d, 0, 0, 20);
  const std::vector<Primitive> green = arc_lines(first, 0x009C00);
  REQUIRE(green.size() == 11);
  CHECK(green.back().y2 == doctest::Approx(220.0));
  CHECK(arc_lines(first, 0xFF0000).empty());
  // the second pass shows its own arc alone
  const DisplayList second = MainWindowProbe::dynamo_arc_screen(*w, d, 1, 1, 20);
  CHECK(arc_lines(second, 0x009C00).empty());
  CHECK(arc_lines(second, 0xFF0000).size() == 11);
}

TEST_CASE("PERSONARE asks the place after the planet and closes with his box") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const SearchContext ctx = radix_context(*w);
  const double birth = julian_day(ctx.base.date_ut);
  const double natal_moon = body_longitude(birth, body::kMoon, ctx).el;
  MainWindow* main = w.get();
  double first = 0.0;
  QStringList box;
  {
    DialogDriver drive;
    drive
        .then([](QDialog* d) {
          auto* bodies = d->findChild<QComboBox*>();
          REQUIRE(bodies != nullptr);
          bodies->setCurrentIndex(bodies->findText("MOND-PERS"));
          click_button(d, "OK");
        })
        .then([main](QDialog* d) {  // EREIGNIS-Ort = GEBURTS-Ort ?, then his wart
          release_wart(main);
          DialogDriver::click("JA")(d);
        })
        .then([&, main](QDialog* d) {  // WEITERES PERSONAR ERSTELLEN ?
          first = MainWindowProbe::panel_jd(*main);
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            box << l->text();
          }
          for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            box << b->text();
          }
          release_wart(main);
          DialogDriver::click("EIN JAHR VORWÄRTS")(d);
        })
        .then(DialogDriver::click("NEIN"));
    MainWindowProbe::personar(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const QString all = box.join(QChar(0x0A));
  CHECK(all.contains("WEITERES PERSONAR ERSTELLEN ?"));
  CHECK(all.contains("EIN JAHR ZURÜCK ( Für GRENZFÄLLE ! )"));
  CHECK(MainWindowProbe::banner_record(*w) == "MOND-PERS");
  // the Sun of the first year of life over the radix Moon
  CHECK(first - birth > 0.0);
  CHECK(first - birth < 366.0);
  CHECK(arcsec_apart(body_longitude(first, body::kSun, ctx).el, natal_moon) < 30.0);
  // EIN JAHR VORWÄRTS, jdz = jd(1,ze) + 2 * tja, one crossing later
  const double second = MainWindowProbe::panel_jd(*w);
  CHECK(second - first == doctest::Approx(365.25).epsilon(0.01));
  CHECK(arcsec_apart(body_longitude(second, body::kSun, ctx).el, natal_moon) < 30.0);
}

TEST_CASE("TAGES-HOROSKOP switches the heliocentric mode off like CLR hrg!") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  MainWindowProbe::helio(*w, true);
  const double day0 = julian_day({24, 9, 2026, 0, 0.0});
  DialogDriver drive;
  drive.then(DialogDriver::click("JA"))
      .then(DialogDriver::fill({"", "24", "9", "2026"}, "OK"))
      .then(DialogDriver::click("( = ENDE )"));
  MainWindowProbe::day_chart(*w);
  CHECK(drive.unexpected() == 0);
  CHECK_FALSE(MainWindowProbe::settings(*w).heliocentric);
  // the true solar time needs the Sun, the moment lands on the day
  const double jd = MainWindowProbe::panel_jd(*w);
  CHECK(jd >= day0);
  CHECK(jd < day0 + 1.0);
  CHECK(MainWindowProbe::banner_record(*w) == "TAG-HOR");
}
