// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QAbstractButton>
#include <QApplication>
#include <QIcon>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QTimer>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/statist_eval.hpp"
#include "probe.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a synthetic evening birth in the north, no real person
AafRecord evening_birth(int hour = 20) {
  AafRecord r;
  r.surname = "WIZARDCASE";
  r.day = 3;
  r.month = 2;
  r.year = 1984;
  r.hour = hour;
  r.zone = "00hE00:00";
  r.lat_deg = 53;
  r.lat_min = 33;
  r.lon_deg = 10;
  r.lon_min = 0;
  return r;
}

// a step pressing one key on the box, R and PgUp are his zurueck!
DialogDriver::Step press(Qt::Key key) {
  return [key](QDialog* d) {
    QKeyEvent e(QEvent::KeyPress, key, Qt::NoModifier, key == Qt::Key_R ? QStringLiteral("r") : QString());
    QApplication::sendEvent(d, &e);
  };
}

// clicks a bar of the ASPEKT-LINIEN screen, rows of 17 from y 30 on a
// canvas of 640 by 431
void click_line(QDialog* d, int row) {
  auto* list = d->findChild<QWidget*>("lineStyleList");
  REQUIRE(list != nullptr);
  const QPointF at(100.0 * list->width() / 640.0, (30.0 + (row - 1) * 17.0 + 8.0) * list->height() / 431.0);
  QMouseEvent e(QEvent::MouseButtonPress, at, list->mapToGlobal(at), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  QApplication::sendEvent(list, &e);
}

// answers the POPUP menus of a screen, picks the entry with the caption
// or closes the menu
struct PopupAnswer {
  QTimer timer;
  QStringList items;
  QString pick;
  int seen = 0;
  explicit PopupAnswer(QString choose = {}) : pick(std::move(choose)) {
    QObject::connect(&timer, &QTimer::timeout, [this]() {
      auto* m = qobject_cast<QMenu*>(QApplication::activePopupWidget());
      if (m == nullptr) {
        return;
      }
      ++seen;
      items.clear();
      QAction* chosen = nullptr;
      for (QAction* a : m->actions()) {
        items << a->text();
        if (!pick.isEmpty() && a->text().contains(pick)) {
          chosen = a;
        }
      }
      if (chosen == nullptr) {
        m->close();
        return;
      }
      m->setActiveAction(chosen);
      QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
      QApplication::sendEvent(m, &enter);
    });
    timer.start(20);
  }
};

QString labels(QDialog* d) {
  QString all;
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    all += l->text() + "|";
  }
  return all;
}

}  // namespace

TEST_CASE("VORGABEN HOROSKOP walks on from the chosen topic and R steps back") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  k.begz = 1;
  MainWindowProbe::alt_rulers(*w) = false;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("BEGINN des Horoskops"))  // GEWÜNSCHTES THEMA ANKLICKEN !
        .then(DialogDriver::click("0 WIDDER"))                 // BEGINN HOROSKOP ?
        .then(press(Qt::Key_R))                               // ZUORDNUNG ZEICHEN-HERRSCHER ?
        .then(DialogDriver::click("MC"))                      // BEGINN HOROSKOP ? again
        .then(DialogDriver::click("ALT"))                     // ZUORDNUNG ZEICHEN-HERRSCHER ?
        .then(DialogDriver::click("EXIT"));                   // FARBEN im HOROSKOP-RING ?
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    CHECK(drive.titles().at(1) == drive.titles().at(3));
  }
  CHECK(k.begz == 2);
  CHECK(MainWindowProbe::alt_rulers(*w));
  const WheelOptions o = MainWindowProbe::wheel_options(*w);
  CHECK(o.begin == 2);
  // horbeg puts the MC on the left of the wheel
  CHECK(MainWindowProbe::wheel(*w).items.size() > 0);
}

TEST_CASE("ORBES BESTIMMEN takes his equally probable orbs and the factor") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  AspectSettings& a = MainWindowProbe::aspect_settings(*w);
  a.equal_probability = false;
  a.orb = 1.0;
  a.orbe.fill(0.0);
  QString first_field;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ORBES BESTIMMEN"))
        .then(DialogDriver::click("NEU SELBST"))                    // ORBES der ASPEKTE EINZELN VORGEBEN ?
        .then(DialogDriver::click("GLEICH WAHRSCHEINLICHE"))         // ORBES der GRUND-ASPEKTE EINGEBEN !
        .then([&first_field](QDialog* d) {                           // the same table with his presets
          first_field = d->findChildren<QLineEdit*>().at(0)->text().trimmed();
          DialogDriver::click("Weiter")(d);
        })
        .then(DialogDriver::click("NEIN"))                            // GEWICHTUNG der PLANETEN-ORBES ?
        .then(DialogDriver::click("150"))                             // FAKTOR vor ORBIS in PROZENTEN
        .then(DialogDriver::click("EXIT"));                           // GRADE und RÜCKLÄUFIGKEITEN ?
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(first_field == "5.40");
  CHECK(a.equal_probability);
  CHECK(a.orb == doctest::Approx(1.5));
  CHECK(a.orbe[1] * kRadToDeg == doctest::Approx(5.4));
  CHECK(a.orbe[2] * kRadToDeg == doctest::Approx(6.8));
  CHECK(a.orbe[11] * kRadToDeg == doctest::Approx(0.6));
  CHECK(a.orbe[13] * kRadToDeg == doctest::Approx(2.0));
  CHECK(a.orbe[14] * kRadToDeg == doctest::Approx(1.0));
  // kon_dsp writes the table as his orb$ strings
  CHECK(MainWindowProbe::konsta(*w).orb_text[1] == " 5.40");
}

TEST_CASE("orbis_asp refuses an overlapping orb and shows the entry again") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  AspectSettings& a = MainWindowProbe::aspect_settings(*w);
  const double quintil_before = a.orbe[5];
  QString alert;
  QString again;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ORBES BESTIMMEN"))
        .then(DialogDriver::click("NEU SELBST"))
        .then([](QDialog* d) {
          // 20 degrees on the quintil reach into the sextil
          d->findChildren<QLineEdit*>().at(4)->setText("20");
          DialogDriver::click("Weiter")(d);
        })
        .then([&alert](QDialog* d) {  // !! ACHTUNG !!
          alert = d->windowTitle() + "|" + labels(d);
          DialogDriver::click("NOCHMAL")(d);
        })
        .then([&again](QDialog* d) {
          again = d->findChildren<QLineEdit*>().at(4)->text().trimmed();
          DialogDriver::click("EXIT")(d);
        });
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(alert.contains("ACHTUNG"));
  CHECK(alert.contains(QString::fromUtf8("ÜBERDECKENDER ORBIS")));
  CHECK(alert.contains("BEI NR. 5"));
  CHECK(again == "20.00");
  CHECK(a.orbe[5] == quintil_before);
}

TEST_CASE("the ASPEKT-LINIEN screen switches his lines and asks for Teiler 12") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  k.selbst_cl_st = true;
  k.aspli_flag[5] = 5;
  k.aspli_flag[2] = 2;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ASPEKTE bzw."))
        .then(DialogDriver::click("1....4"))     // AUSWERTUNGEN im HOROSKOP-FORMULAR ?
        .then(DialogDriver::click("NEIN"))       // HALBSUMMENLISTE HINZUNEHMEN ?
        .then([](QDialog* d) {                   // ASPEKT - LINIEN
          QTimer::singleShot(0, d, [d]() {
            click_line(d, 22);  // HORCOM - STANDARD WIEDERHERSTELLEN
            click_line(d, 5);   // the quintil needs the Teiler 12
          });
        })
        .then(DialogDriver::click("OK"))         // TEILER auf 12 FESTGELEGT !
        .then([](QDialog* d) {                   // the screen again
          QTimer::singleShot(0, d, [d]() {
            click_line(d, 5);   // his standard quintil off
            click_line(d, 23);  // WAHLENDE = WEITER = SPEICHERN
          });
        })
        .then(DialogDriver::click("BLAU"));      // FARBE FESTLEGEN bei DOPPELKREIS
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::aspect_settings(*w).divisors == 12);
  CHECK_FALSE(k.selbst_cl_st);
  CHECK(k.aspli_flag[5] == 0);
  CHECK(MainWindowProbe::outer_color(*w) == 3);
  const WheelOptions o = MainWindowProbe::wheel_options(*w);
  CHECK_FALSE(o.chords[5].on);
  CHECK(o.chords[2].on);
  CHECK(o.chords[2].color == 0xFF0000);
  CHECK(o.chords[2].style == Primitive::Style::kSolid);
}

TEST_CASE("OHNE ASPEKT - LINIEN takes the chords off the wheel") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ASPEKTE bzw."))
        .then(DialogDriver::click("OHNE ASPEKT"))
        .then(DialogDriver::click("JA"))          // HALBSUMMENLISTE HINZUNEHMEN ?
        .then(DialogDriver::click("SCHWARZ"));    // the outer colour follows
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::aspect_settings(*w).divisors == 1);
  CHECK(MainWindowProbe::konsta(*w).voll);
  CHECK_FALSE(MainWindowProbe::wheel_options(*w).aspect_lines);
  CHECK(MainWindowProbe::outer_color(*w) == 2);
}

TEST_CASE("the colour modes of avh dress the ring and the histograms") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("FARBEN bzw. SCHRAFFUR"))
        .then(DialogDriver::click("FARBIG PUR"))          // FARBEN im HOROSKOP-RING u.HISTOGRAMMEN ?
        .then(DialogDriver::click("FARBEN NICHT"))        // FARBEN für HOROSKOP - RING FESTLEGEN !
        .then(DialogDriver::click("EXIT"));               // HISTOGRAMME für ELEMENTE
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(k.farbp);
  CHECK(k.eigfarb);
  CHECK_FALSE(k.farbs);
  WheelOptions o = MainWindowProbe::wheel_options(*w);
  CHECK(o.ring_fill == RingFill::kSolid);
  CHECK(o.hist_fill == RingFill::kSolid);
  // his own cols%, fire red and water blue
  CHECK(o.ring_colors[1] == 0xFF0000);
  CHECK(o.ring_colors[4] == 0x0000FF);
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("FARBEN bzw. SCHRAFFUR"))
        .then(DialogDriver::click("ZEICHEN-SYMBOLE: FARBIG    HISTOGRAMME:     W"))
        .then(DialogDriver::click("EXIT"));
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(k.weiss);
  CHECK(k.nursymb == 1);
  o = MainWindowProbe::wheel_options(*w);
  CHECK(o.ring_fill == RingFill::kWhite);
  CHECK(o.hist_fill == RingFill::kWhite);
  CHECK(o.colored_signs);
}

TEST_CASE("GRADE, KLEIN-SYMBOLE and the histogram points reach the wheel") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  k.pn[3] = 1;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("GRÖßE der SYMBOLE"))
        .then(DialogDriver::click("KLEIN"))                    // KLEIN-SYMBOLE in HOROSKOPEN ?
        .then(DialogDriver::click("EXIT"));                     // ORBES der ASPEKTE
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
  }
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("GRADE und RÜCKLÄUFIGKEIT"))
        .then(DialogDriver::click("NUR die GRADE"))
        .then(DialogDriver::click("EXIT"));                     // BEGINN HOROSKOP ?
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
  }
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("HISTOGRAMM der ELEMENTE"))
        .then(DialogDriver::click("Nur für ZEICHEN"))
        .then(DialogDriver::click("JA"))                        // GEWICHTUNG für HISTOGRAMM ?
        .then([](QDialog* d) {                                  // PUNKTE-WERT 0....9 EINGEBEN !
          d->findChildren<QLineEdit*>().at(2)->setText("7");
          DialogDriver::click("WEITER")(d);
        })
        .then(DialogDriver::click("EXIT"));                     // AUSWERTUNGEN im HOROSKOP-FORMULAR ?
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(k.klsy);
  CHECK(k.pziff == 2.0);
  CHECK(k.elem == 1);
  CHECK(k.pn[3] == 7);
  const WheelOptions o = MainWindowProbe::wheel_options(*w);
  CHECK(o.small_symbols);
  CHECK(o.degree_numbers);
  CHECK_FALSE(o.retro_marks);
}

TEST_CASE("PgUp on the points form steps back to the colours") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  QStringList seen;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("HISTOGRAMM der ELEMENTE"))
        .then(DialogDriver::click("Nur für ZEICHEN"))
        .then(DialogDriver::click("JA"))
        .then([](QDialog* d) {
          QKeyEvent e(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier);
          QApplication::sendEvent(d, &e);
        })
        .then(DialogDriver::click("EXIT"));  // FARBEN im HOROSKOP-RING u.HISTOGRAMMEN ?
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    seen = drive.titles();
  }
  REQUIRE(seen.size() == 5);
  CHECK(seen.at(4).contains("FARBEN im HOROSKOP-RING"));
}

TEST_CASE("the old rulers of alt! reach the birth ruler of the wheel") {
  auto w = MainWindowProbe::make();
  // find an evening whose ascendant stands in Scorpio, Aquarius or Pisces
  int hour = -1;
  for (int h = 0; h < 24 && hour < 0; ++h) {
    MainWindowProbe::apply(*w, evening_birth(h));
    const int sign = static_cast<int>(MainWindowProbe::chart(*w).houses.cusp[1] / (kPi / 6.0));
    if (sign == 7 || sign == 10 || sign == 11) {
      hour = h;
    }
  }
  REQUIRE(hour >= 0);
  const double ac = MainWindowProbe::chart(*w).houses.cusp[1];
  MainWindowProbe::alt_rulers(*w) = false;
  CHECK(MainWindowProbe::wheel_options(*w).ruler_slot == sign_ruler(ac, false));
  MainWindowProbe::alt_rulers(*w) = true;
  const int classic = sign_ruler(ac, true);
  CHECK(classic != sign_ruler(ac, false));
  CHECK(MainWindowProbe::wheel_options(*w).ruler_slot == classic);
}

TEST_CASE("the KOMPAKT-AUSWERTUNG lists the midpoints and bes2 names the chart") {
  auto w = MainWindowProbe::make();
  Konsta& k = MainWindowProbe::konsta(*w);
  k.voll = true;
  MainWindowProbe::apply(*w, evening_birth());
  const QString with = MainWindowProbe::summary(*w);
  CHECK(with.contains("HALBSUM."));
  CHECK(with.contains("DIREKT:"));
  CHECK(with.contains("HALBQU:"));
  CHECK(with.contains(QRegularExpression("[A-Z]{2}=[A-Z]{2}-[A-Z]{2}")));
  k.voll = false;
  MainWindowProbe::apply(*w, evening_birth());
  CHECK_FALSE(MainWindowProbe::summary(*w).contains("HALBSUM."));
  // his sol$ stands in the centre of the radix wheel
  bool radix = false;
  for (const Primitive& p : MainWindowProbe::wheel(*w).items) {
    radix = radix || (p.kind == Primitive::Kind::kText && p.text == "RADIX");
  }
  CHECK(radix);
}

TEST_CASE("the HALBSUMMEN-GRAPHIK pages its trees and asks for the dial sort") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  QStringList titles;
  QString question;
  {
    DialogDriver drive;
    drive
        .then([&titles](QDialog* d) {  // the only page of the trees
          titles << d->windowTitle();
          QTimer::singleShot(0, d, [d]() {
            QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
            QApplication::sendEvent(d, &space);
          });
        })
        .then([&question](QDialog* d) {  // SORTIEREN nach KRITERIUM :
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            question += l->text() + "|";
          }
          DialogDriver::click("JA")(d);
        })
        .then([&titles](QDialog* d) {  // the sorted trees
          titles << d->windowTitle();
          QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
          QApplication::sendEvent(d, &esc);
        });
    MainWindowProbe::midpoint_tree(*w);
    CHECK(drive.unexpected() == 0);
  }
  REQUIRE(titles.size() == 2);
  CHECK(titles[0].contains("HALBSUMMEN-GRAPHIK"));
  CHECK(titles[0].contains("WEITER mit LEERTASTE"));
  CHECK(titles[1] == "HALBSUMMEN-GRAPHIK");
  CHECK(question.contains("SORTIEREN nach KRITERIUM"));
  CHECK(question.contains("PLANETENBILDER"));
}

TEST_CASE("the right mouse on the chart opens einzel_plan_wahl") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  MainWindowProbe::emphasis(*w)[body::kMars] = 1;
  QString box;
  {
    DialogDriver drive;
    drive.then([&box](QDialog* d) {
      box = d->windowTitle();
      for (const QLabel* l : d->findChildren<QLabel*>()) {
        box += "|" + l->text();
      }
      DialogDriver::click("NORMALE AUSGABE")(d);
    });
    QWidget* wheel = MainWindowProbe::wheel_widget(*w);
    const QPointF at(wheel->width() / 2.0, wheel->height() / 2.0);
    QMouseEvent press(QEvent::MouseButtonPress, at, wheel->mapToGlobal(at), Qt::RightButton, Qt::RightButton,
                      Qt::NoModifier);
    QApplication::sendEvent(wheel, &press);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(box.contains("EINZELNE PlANETEN"));
  //RR NORMALE AUSGABE clears plan_col!
  CHECK(MainWindowProbe::emphasis(*w)[body::kMars] == 0);
}

TEST_CASE("the ASPEKTARIUM asks his MAXIMALER Teiler box first") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  AspectSettings& a = MainWindowProbe::aspect_settings(*w);
  a.equal_probability = false;
  a.divisors = 8;
  QStringList buttons;
  QString info;
  QString sheet_title;
  {
    DialogDriver drive;
    drive
        .then([&](QDialog* d) {
          for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            buttons << b->text();
          }
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            info += l->text() + "|";
          }
          DialogDriver::click(" 16 ")(d);
        })
        .then([&](QDialog* d) {
          sheet_title = d->windowTitle();
          d->reject();
        });
    MainWindowProbe::open_aspektarium(*w);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(buttons.size() == 3);
  CHECK(info.contains(QString::fromUtf8("ORBES nach HORCOM- Zählung !")));
  CHECK(info.contains("MAXIMALER Teiler ?"));
  CHECK(sheet_title == "ASPEKTARIUM");
  // nasp& comes back after the sheet
  CHECK(a.divisors == 8);
  // the equal probability orbs offer no sixteen
  a.equal_probability = true;
  buttons.clear();
  {
    DialogDriver drive;
    drive.then([&](QDialog* d) {
      for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
        buttons << b->text();
      }
      d->reject();
    });
    MainWindowProbe::open_aspektarium(*w);
  }
  CHECK(buttons.size() == 2);
}

namespace {

// a key on the main window some time after the boxes closed, his wart
void key_later(MainWindow& w, int ms) {
  QTimer::singleShot(ms, &w, [&w]() {
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
    QApplication::sendEvent(&w, &e);
  });
}

}  // namespace

TEST_CASE("DOPPEL-KREIS asks MODUS, INNEN and AUSSEN and draws his a12 sheet") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::put_slot(*w, 0, evening_birth(20));
  AafRecord other = evening_birth(8);
  other.surname = "PARTNERCASE";
  other.year = 1990;
  MainWindowProbe::put_slot(*w, 1, other);
  QStringList titles;
  QString inner_box;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click(" 90"))
        .then([&](QDialog* d) {  // Datensatz für INNEN-Kreis aktivieren !
          inner_box = labels(d);
          DialogDriver::click("SATZ2")(d);
        })
        .then(DialogDriver::click("SATZ1"));  // AUSSEN-Kreis
    MainWindowProbe::double_wheel_session(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    titles = drive.titles();
  }
  CHECK(titles.at(0).contains("MODUS"));
  CHECK(inner_box.contains("INNEN-Kreis"));
  CHECK(inner_box.contains("90"));
  CHECK(MainWindowProbe::active_slot(*w) == 1);
  CHECK(MainWindowProbe::compare_on(*w));
  CHECK(MainWindowProbe::dial_on(*w));
  CHECK(MainWindowProbe::full_sheet(*w));
  bool inner = false;
  bool outer = false;
  bool column = false;
  for (const Primitive& p : MainWindowProbe::wheel(*w).items) {
    if (p.kind != Primitive::Kind::kText) {
      continue;
    }
    inner = inner || p.text == "INNEN-Kreis";
    outer = outer || p.text == "AUSSEN-Kreis";
    column = column || p.text.rfind("SO ", 0) == 0;
  }
  CHECK(inner);
  CHECK(outer);
  CHECK(column);
}

TEST_CASE("MULTIPLE DIREKTIONEN runs his session from MODUS to BEENDEN") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  QString date_box;
  QString weit;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("MULTI 1"))
        .then([&](QDialog* d) {  // Ereignis - DATUM eingeben !
          date_box = d->windowTitle() + "|" + labels(d);
          DialogDriver::fill({"", "3", "2", "2020"}, "OK")(d);
        })
        .then([&w](QDialog* d) {  // haus_ber
          key_later(*w, 200);
          DialogDriver::click("Wie PLANETEN")(d);
        })
        .then([&w](QDialog* d) {  // GEBURTSZEIT VARIIEREN ?
          key_later(*w, 200);
          DialogDriver::click("NEIN")(d);
        })
        .then([&weit](QDialog* d) {  // Weiteres DATUM untersuchen ?
          weit = labels(d);
          DialogDriver::click("BEENDEN")(d);
        });
    MainWindowProbe::multi_session(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(date_box.contains("Ereignis - DATUM"));
  CHECK(date_box.contains("LEHRIEDER"));
  CHECK(weit.contains("Weiteres DATUM"));
  CHECK(MainWindowProbe::multi_on(*w));
  bool radix = false;
  bool multi = false;
  for (const Primitive& p : MainWindowProbe::wheel(*w).items) {
    radix = radix || (p.kind == Primitive::Kind::kText && p.text == "RADIX :");
    multi = multi || (p.kind == Primitive::Kind::kText && p.text == "MULTI 1:");
  }
  CHECK(radix);
  CHECK(multi);
}

TEST_CASE("zeitwim walks the birth time under the MULTI sheet and gives it back") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  const double jd0 = MainWindowProbe::panel_jd(*w);
  double walked = 0.0;
  QStringList sheet;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("MULTI 1"))
        .then(DialogDriver::fill({"", "3", "2", "2020"}, "OK"))
        .then([&w](QDialog* d) {
          key_later(*w, 200);
          DialogDriver::click("Wie PLANETEN")(d);
        })
        .then(DialogDriver::click("JA"))            // GEBURTSZEIT VARIIEREN ?
        .then(DialogDriver::click("MINUTEN"))       // ZEIT-EINHEIT ?
        .then(DialogDriver::fill({"30"}, "OK"))     // MINUTEN als BELIEBIGE ZAHL
        .then(DialogDriver::click("VOR"))           // RICHTUNG ?
        .then([&sheet](QDialog* d) {                // the key legend
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            sheet << l->text();
          }
          QTimer::singleShot(0, d, [d]() {
            QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
            QApplication::sendEvent(d, &e);
          });
        })
        .then([&](QDialog* d) {                     // the walk, one step, then ESC
          QTimer::singleShot(0, d, [&, d]() {
            QKeyEvent v(QEvent::KeyPress, Qt::Key_V, Qt::NoModifier, "v");
            QApplication::sendEvent(d, &v);
            walked = MainWindowProbe::panel_jd(*w);
            QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            QApplication::sendEvent(d, &esc);
          });
        })
        .then(DialogDriver::click("NEIN = ENDE"))   // Weiter Mit NEUER ZEITEINHEIT ?
        .then([&w](QDialog* d) {                    // VARIIERTE ZEIT in RADIX ÜBERNEHMEN ?
          key_later(*w, 200);
          DialogDriver::click("NEIN")(d);
        })
        .then(DialogDriver::click("BEENDEN"));
    MainWindowProbe::multi_session(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(sheet.join("|").contains("S  =  1 Sekunde"));
  // one step of thirty minutes, then the radix came back
  CHECK((walked - jd0) * 1440.0 == doctest::Approx(30.0).epsilon(0.01));
  CHECK(MainWindowProbe::panel_jd(*w) == doctest::Approx(jd0));
}

TEST_CASE("COMPOSIT asks his MODUS, both SATZ clicks and the ROBERT HAND residence") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::put_slot(*w, 0, evening_birth(20));
  AafRecord other = evening_birth(8);
  other.surname = "PARTNERCASE";
  other.year = 1990;
  MainWindowProbe::put_slot(*w, 1, other);
  Konsta& k = MainWindowProbe::konsta(*w);
  k.comp_mstz = true;
  k.comp_hand = false;
  QStringList modes;
  QString first_box;
  QString mask_title;
  {
    DialogDriver drive;
    drive
        .then([&](QDialog* d) {  // COMPOSIT-HOROSKOP MODUS ?
          for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            modes << b->text();
          }
          modes << labels(d);
          DialogDriver::click("ROBERT HAND")(d);
        })
        .then([&](QDialog* d) {  // Datensatz für PARTNER 1 aktivieren !
          first_box = labels(d);
          DialogDriver::click("SATZ1")(d);
        })
        .then(DialogDriver::click("SATZ2"))  // PARTNER 2
        .then([&](QDialog* d) {              // his eingabe(0,-1,1,10)
          mask_title = d->windowTitle();
          DialogDriver::fill({"", "TESTRESIDENZ", "E", "13", "24", "0", "N", "52", "31", "0"}, "OK")(d);
        });
    MainWindowProbe::composite_session(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // the three house systems, the one of his profile offered first
  CHECK(modes.join("|").contains("HÄUSER-SYSTEM mit MITTLERER STZ,Länge,Breite ab MC -HALBSUMME"));
  CHECK(modes.join("|").contains("HÄUSER-SYSTEM : SCHEMATISCHE HALBSUMMEN ab MC -HALBSUMME"));
  CHECK(modes.join("|").contains("COMPOSIT-HOROSKOP MODUS ?"));
  CHECK(first_box.contains("2 DATENSÄTZE NACHEINANDER ANKLICKEN !"));
  CHECK(first_box.contains("PARTNER 1"));
  CHECK(mask_title == "EREIGNIS-ORT  EINGEBEN !  ->  EVENTL. TAB - TASTE !  |  COMPOSIT NR.1");
  // his param_sp keeps the choice
  CHECK(k.comp_hand);
  CHECK_FALSE(k.comp_mstz);
  CHECK(MainWindowProbe::composite_on(*w));
  CHECK(MainWindowProbe::active_slot(*w) == 0);
  REQUIRE(MainWindowProbe::residence(*w).has_value());
  CHECK(*MainWindowProbe::residence(*w) == "TESTRESIDENZ");
  // bes2_comp in the centre, H1 on the first cusp and the AC midpoint on
  // its own axis
  bool label = false;
  bool method = false;
  bool h1 = false;
  bool ac = false;
  for (const Primitive& p : MainWindowProbe::wheel(*w).items) {
    if (p.kind != Primitive::Kind::kText) {
      continue;
    }
    label = label || p.text == "COMPOSIT";
    method = method || p.text == "N.ROB. HAND";
    h1 = h1 || p.text == "H1";
    ac = ac || p.text == "AC";
  }
  CHECK(label);
  CHECK(method);
  CHECK(h1);
  CHECK(ac);
  // bes10 without Vel., the residence in the place corner
  CHECK(MainWindowProbe::body_column_hidden(*w, 3));
  CHECK_FALSE(MainWindowProbe::body_column_hidden(*w, 0));
  const ClassicSheetText sheet = MainWindowProbe::sheet(*w);
  CHECK(sheet.place == "TESTRESIDENZ");
  CHECK(sheet.lat.find("52") != std::string::npos);
  CHECK(sheet.longitudes_only);
  CHECK(sheet.h1_axis);
  CHECK(sheet.pair_note.empty());
  // HOROSKOP - GRAPHIK drops the pair, the plain chart has its columns
  MainWindowProbe::reset_views(*w);
  CHECK_FALSE(MainWindowProbe::composite_on(*w));
  CHECK_FALSE(MainWindowProbe::compare_on(*w));
  CHECK_FALSE(MainWindowProbe::body_column_hidden(*w, 3));
  // the COMPOSIT row of DOPPEL-DATEN brings the pair back with its
  // residence and no question
  {
    DialogDriver drive;
    MainWindowProbe::recall_double(*w, 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::composite_on(*w));
  REQUIRE(MainWindowProbe::residence(*w).has_value());
  CHECK(*MainWindowProbe::residence(*w) == "TESTRESIDENZ");
}

TEST_CASE("COMPOSIT under the equal houses skips the MODUS box and draws the schematic sheet") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::put_slot(*w, 0, evening_birth(20));
  AafRecord other = evening_birth(8);
  other.surname = "PARTNERCASE";
  MainWindowProbe::put_slot(*w, 1, other);
  MainWindowProbe::houses(*w, 6);
  Konsta& k = MainWindowProbe::konsta(*w);
  k.comp_hand = true;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("SATZ1")).then(DialogDriver::click("SATZ2"));
    MainWindowProbe::composite_session(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR IF haw& = 6 OR haw& = 7 : CLR comp_hand!,comp_mstz!
  CHECK_FALSE(k.comp_hand);
  CHECK_FALSE(k.comp_mstz);
  CHECK(MainWindowProbe::composite_on(*w));
  bool schematic = false;
  bool h1 = false;
  for (const Primitive& p : MainWindowProbe::wheel(*w).items) {
    schematic = schematic || (p.kind == Primitive::Kind::kText && p.text == "Schematisch");
    h1 = h1 || (p.kind == Primitive::Kind::kText && p.text == "H1");
  }
  CHECK(schematic);
  CHECK_FALSE(h1);
  // no residence, no place of its own
  const ClassicSheetText sheet = MainWindowProbe::sheet(*w);
  CHECK(sheet.place.empty());
  CHECK(sheet.lon.empty());
  CHECK_FALSE(sheet.h1_axis);
}

TEST_CASE("COMBIN takes up to five SATZ clicks with his HOLEN box between") {
  auto w = MainWindowProbe::make();
  AafRecord one = evening_birth(20);
  one.surname = "ERSTERFALL";
  AafRecord two = evening_birth(8);
  two.surname = "ZWEITERFALL";
  two.year = 1990;
  two.lon_deg = 12;
  AafRecord three = evening_birth(14);
  three.surname = "DRITTERFALL";
  three.year = 1996;
  three.lon_deg = 14;
  MainWindowProbe::put_slot(*w, 0, one);
  MainWindowProbe::put_slot(*w, 1, two);
  MainWindowProbe::put_slot(*w, 2, three);
  QString second_pick;
  QString holen;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("SATZ1"))
        .then([&](QDialog* d) {
          for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            second_pick += b->text() + "|";
          }
          DialogDriver::click("SATZ3")(d);
        })
        .then([&](QDialog* d) {  // 3. Datensatz HOLEN ?
          holen = labels(d);
          DialogDriver::click("HOLEN")(d);
        })
        .then(DialogDriver::click("SATZ2"));
    MainWindowProbe::combin_chart(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR sol$ = "*" + sol$ on the slots clicked so far
  CHECK(second_pick.contains("SATZ1: *RADIX  ERSTERFALL"));
  CHECK(second_pick.contains("SATZ2: RADIX  ZWEITERFALL"));
  CHECK(holen.contains("3. Datensatz HOLEN ?"));
  CHECK(holen.contains("AUSGABE ?"));
  // the mean of the three places, his COMBIN-ORT
  CHECK(MainWindowProbe::lon(*w) == doctest::Approx(12.0));
  const ClassicSheetText sheet = MainWindowProbe::sheet(*w);
  CHECK(sheet.place == "COMBIN-ORT");
  CHECK(sheet.pair_name1 == "SÄTZE: 1,3,2");
  REQUIRE(sheet.pair_list.size() == 3);
  CHECK(sheet.pair_list[1] == "DRITTERFALL");
  CHECK(sheet.pair_note.rfind("COMBIN-UT: ", 0) == 0);
  CHECK(sheet.stz.empty());
  // bes2 names the chart COMBIN
  bool centre = false;
  for (const Primitive& p : MainWindowProbe::wheel(*w).items) {
    centre = centre || (p.kind == Primitive::Kind::kText && p.text == "COMBIN");
  }
  CHECK(centre);
  // the slots keep their own places
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  CHECK(MainWindowProbe::slot(*w, 0)->lon_deg == 10);
  // the DOPPEL-DATEN row brings all three back without a question
  MainWindowProbe::apply(*w, one);
  CHECK(MainWindowProbe::sheet(*w).place != "COMBIN-ORT");
  {
    DialogDriver drive;
    MainWindowProbe::recall_double(*w, 1);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::sheet(*w).pair_name1 == "SÄTZE: 1,3,2");
  CHECK(MainWindowProbe::lon(*w) == doctest::Approx(12.0));
}

TEST_CASE("COMBIN of two outputs at once when no further RADIX slot is filled") {
  auto w = MainWindowProbe::make();
  AafRecord one = evening_birth(20);
  one.surname = "ERSTERFALL";
  AafRecord two = evening_birth(8);
  two.surname = "ZWEITERFALL";
  two.lon_deg = 12;
  MainWindowProbe::put_slot(*w, 0, one);
  MainWindowProbe::put_slot(*w, 1, two);
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("SATZ1")).then(DialogDriver::click("SATZ2"));
    MainWindowProbe::combin_chart(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const ClassicSheetText sheet = MainWindowProbe::sheet(*w);
  CHECK(sheet.pair_name1 == "1: ERSTERFALL");
  CHECK(sheet.pair_name2 == "2: ZWEITERFALL");
  CHECK(sheet.pair_list.empty());
  CHECK(MainWindowProbe::lon(*w) == doctest::Approx(11.0));
  CHECK(MainWindowProbe::double_action(*w, 1)->text().startsWith("COMBIN: ERSTERFALL-ZWEITERFAL"));
}

TEST_CASE("ERGEBNIS als RADIX takes the COMBIN with its mean moment and place") {
  auto w = MainWindowProbe::make();
  AafRecord one = evening_birth(20);
  one.surname = "ERSTERFALL";
  AafRecord two = evening_birth(8);
  two.surname = "ZWEITERFALL";
  two.lon_deg = 12;
  MainWindowProbe::put_slot(*w, 0, one);
  MainWindowProbe::put_slot(*w, 1, two);
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("SATZ1")).then(DialogDriver::click("SATZ2"));
    MainWindowProbe::combin_chart(*w);
  }
  const double mean_jd = MainWindowProbe::panel_jd(*w);
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("WEITER")).then(DialogDriver::click("OK"));
    MainWindowProbe::result_as_radix(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR na$(0,2) = LEFT$(n1,10) + "-" + LEFT$(n2,10), sol$ = "COMBIN ALS RADIX"
  REQUIRE(MainWindowProbe::slot(*w, 2).has_value());
  CHECK(MainWindowProbe::slot(*w, 2)->surname == "ERSTERFALL-ZWEITERFAL");
  CHECK(MainWindowProbe::slot(*w, 2)->place == "COMBIN-ORT");
  CHECK(MainWindowProbe::slot_text(*w, 2) == "SATZ3: COMBIN ALS RADIX  ERSTERFALL-ZWEITERFAL");
  CHECK(MainWindowProbe::active_slot(*w) == 2);
  CHECK(MainWindowProbe::panel_jd(*w) == doctest::Approx(mean_jd).epsilon(1e-9));
  CHECK(MainWindowProbe::lon(*w) == doctest::Approx(11.0));
  // the promoted chart is a radix now, the combin rows are gone
  CHECK(MainWindowProbe::sheet(*w).pair_note.empty());
}

TEST_CASE("halbs_zaehl_gr writes the midpoint counts as a line under the trees, no popup") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  const QString line = MainWindowProbe::count_line(*w, {30, 15, 8, 2}, true);
  CHECK(line.startsWith("Anzahl Halbsummen :"));
  CHECK(line.contains("Direkt 30"));
  CHECK(line.contains("Viertelquadrat 2"));
  CHECK(line.contains("ZUSAMMEN 55"));
  CHECK(line.contains("ORBIS-FAKTOR = 100%"));
  // without the fourth level the line leaves it out
  CHECK_FALSE(MainWindowProbe::count_line(*w, {30, 15, 8, 0}, false).contains("Viertelquadrat"));
  QString shown;
  int popups = 0;
  {
    PopupAnswer popup;
    DialogDriver drive;
    drive.then([&shown](QDialog* d) {
      if (const auto* l = d->findChild<QLabel*>("midpointCounts")) {
        shown = l->text();
      }
      d->reject();
    });
    MainWindowProbe::midpoint_tree(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    popups = popup.seen;
  }
  // the graphic opens without anything to click away
  CHECK(popups == 0);
  CHECK(shown.startsWith("Anzahl Halbsummen :"));
}

TEST_CASE("R steps back from the GEWICHTUNG and the HALBSUMMENLISTE boxes like his zurueck!") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  MainWindowProbe::aspect_settings(*w).equal_probability = false;
  QStringList seen;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ORBES BESTIMMEN"))
        .then(DialogDriver::click("HORCOM - Z"))  // ORBES der ASPEKTE EINZELN VORGEBEN ?
        .then(press(Qt::Key_R))                   // GEWICHTUNG der PLANETEN-ORBES ÄNDERN ?
        .then(DialogDriver::click("EXIT"));       // orbis_pla sent him back to avh4
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    seen = drive.titles();
  }
  REQUIRE(seen.size() == 4);
  CHECK(seen.at(1) == seen.at(3));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ASPEKTE bzw."))
        .then(DialogDriver::click("1...12"))  // AUSWERTUNGEN im HOROSKOP-FORMULAR ?
        .then(press(Qt::Key_R))               // HALBSUMMENLISTE HINZUNEHMEN ?
        .then(DialogDriver::click("EXIT"));   // avh11, the histograms
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    seen = drive.titles();
  }
  REQUIRE(seen.size() == 4);
  CHECK(seen.at(3) == "AUSWAHL");
}

TEST_CASE("Enter on the ASPEKT-LINIEN screen ends VORGABEN HOROSKOP like his CASE 13,32") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  k.selbst_cl_st = false;
  const auto flags = k.aspli_flag;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ASPEKTE bzw."))
        .then(DialogDriver::click("1...12"))
        .then(DialogDriver::click("NEIN"))  // HALBSUMMENLISTE HINZUNEHMEN ?
        .then([](QDialog* d) {              // ASPEKT - LINIEN
          QTimer::singleShot(0, d, [d]() {
            auto* list = d->findChild<QWidget*>("lineStyleList");
            QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
            QApplication::sendEvent(list, &down);
            QApplication::sendEvent(list, &down);
            QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            QApplication::sendEvent(list, &enter);
          });
        });
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    // the FARBE FESTLEGEN box of avh14 never comes
    CHECK(drive.unexpected() == 0);
  }
  // the walked row stays as it was
  CHECK(k.aspli_flag == flags);
}

TEST_CASE("hor_farb dresses every element row in its colour and the screens box says it stands idle") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  k.cols = {0, 255, 65280, 16776960, 16711680};
  std::vector<QRgb> swatches;
  QStringList seen;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("FARBEN bzw. SCHRAFFUR"))
        .then(DialogDriver::click("SCHRAFFIERT EIGENE FARBEN"))  // FARBEN im HOROSKOP-RING ?
        .then([&swatches](QDialog* d) {                           // FARBEN für HOROSKOP - RING FESTLEGEN !
          for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            if (!b->icon().isNull()) {
              const QImage img = b->icon().pixmap(QSize(60, 16)).toImage();
              swatches.push_back(img.pixel(img.width() / 2, img.height() / 2) & 0xFFFFFF);
            }
          }
          press(Qt::Key_R)(d);
        })
        .then(DialogDriver::click("EXIT"));  // R steps back to ZUORDNUNG ZEICHEN-HERRSCHER
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    seen = drive.titles();
  }
  REQUIRE(swatches.size() == 4);
  // his hatch of deffi as it rendered, fire salmon and water blue
  CHECK(swatches[0] == ring_fill_color(1, 0xFF0000, RingFill::kShaded));
  CHECK(swatches[3] == ring_fill_color(4, 0x0000FF, RingFill::kShaded));
  CHECK(seen.at(3) == "AUSWAHL");
  QString screens;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("Langsame BILDSCHIRME"))
        .then([&screens](QDialog* d) {
          screens = labels(d);
          DialogDriver::click("EXIT")(d);
        });
    MainWindowProbe::vorgaben_horoskop(*w);
    CHECK(drive.pending() == 0);
  }
  CHECK(screens.contains("ohne Wirkung"));
}

TEST_CASE("PageDown pages the HALBSUMMEN-GRAPHIK, his virtual key 34") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::preset_extras(*w, true, true, true);
  MainWindowProbe::apply(*w, evening_birth());
  PopupAnswer popup;
  QString first;
  QString second;
  {
    DialogDriver drive;
    drive.then([&](QDialog* d) {
      QTimer::singleShot(0, d, [&, d]() {
        first = d->windowTitle();
        QKeyEvent next(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier);
        QApplication::sendEvent(d, &next);
        second = d->windowTitle();
        QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QApplication::sendEvent(d, &esc);
      });
    });
    MainWindowProbe::midpoint_tree(*w);
    CHECK(drive.unexpected() == 0);
  }
  // more than 22 trees, the first page does not ask for the sort
  CHECK(first == "HALBSUMMEN-GRAPHIK");
  CHECK(second.contains("WEITER mit LEERTASTE"));
}

TEST_CASE("MULTI and HARMONICS run geocentric and refuse the order zero") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  MainWindowProbe::helio(*w, true);
  QStringList seen;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("HARMONICS"))
        .then(DialogDriver::fill({"0"}, "OK"))  // ORDNUNGS-ZAHL der HARMONIC, zero
        .then(DialogDriver::fill({"3"}, "OK"))  // asked again
        .then([&w](QDialog* d) {                // haus_ber
          key_later(*w, 200);
          DialogDriver::click("Wie PLANETEN")(d);
        })
        .then([&w](QDialog* d) {  // GEBURTSZEIT VARIIEREN ?
          key_later(*w, 200);
          DialogDriver::click("NEIN")(d);
        })
        .then(DialogDriver::click("BEENDEN"));  // Weiteres HARMONIC untersuchen ?
    MainWindowProbe::multi_session(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    seen = drive.titles();
  }
  CHECK(seen.at(1) == seen.at(2));
  CHECK(MainWindowProbe::harmonic_order(*w) == doctest::Approx(3.0));
  CHECK(MainWindowProbe::harmonic_on(*w));
  // his harm cleared hrg! for the sheet, the Sun heads the radix column
  // and the panel keeps its heliocentric switch
  bool sun_row = false;
  bool harmonic = false;
  for (const Primitive& p : MainWindowProbe::wheel(*w).items) {
    sun_row = sun_row || (p.kind == Primitive::Kind::kText && p.text.rfind("SO ", 0) == 0);
    harmonic = harmonic || (p.kind == Primitive::Kind::kText && p.text == "3.HARMONIC:");
  }
  CHECK(sun_row);
  CHECK(harmonic);
  CHECK(MainWindowProbe::settings(*w).heliocentric);
}

TEST_CASE("an empty DOPPEL-KREIS row runs a12 and COMPOSIT casts geocentric") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, evening_birth());
  QString title;
  {
    DialogDriver drive;
    drive.then([&title](QDialog* d) {
      title = d->windowTitle();
      d->reject();
    });
    MainWindowProbe::recall_double(*w, 2);
    CHECK(drive.pending() == 0);
  }
  // his MODUS box, not a record chooser
  CHECK(title.contains("MODUS"));
  // a13 runs CLR hrg! before its first box
  MainWindowProbe::helio(*w, true);
  {
    DialogDriver drive;
    drive.then([](QDialog* d) { d->reject(); });
    MainWindowProbe::composite_session(*w);
  }
  CHECK_FALSE(MainWindowProbe::settings(*w).heliocentric);
}
