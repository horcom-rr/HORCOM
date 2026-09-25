// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QAbstractButton>
#include <QAction>
#include <QDateTime>
#include <QEventLoop>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>

#include <cmath>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "probe.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a synthetic birth east of Greenwich, no real person
AafRecord walker() {
  AafRecord r;
  r.surname = "TESTFALL";
  r.day = 11;
  r.month = 8;
  r.year = 1999;
  r.hour = 11;
  r.minute = 8;
  r.zone = kUtZoneText;
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 19;
  return r;
}

void press(QDialog* d, int key) {
  QKeyEvent e(QEvent::KeyPress, key, Qt::NoModifier);
  QApplication::sendEvent(d, &e);
}

// the labels of the stepping screen by their content
struct Screen {
  QString diff;
  QString mid;
  QString asp;
  QString dir;
};

Screen read_screen(QDialog* d) {
  Screen s;
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    const QString t = l->text();
    if (t.startsWith("Zeit-Diff")) {
      s.diff = t;
    } else if (t.contains("HALBSUMMEN - ZÄHLER")) {
      s.mid = t;
    } else if (t.contains("ASPEKTE - ZÄHLER")) {
      s.asp = t;
    } else if (t.startsWith("Vorwärts") || t.startsWith("Rückwärts")) {
      s.dir = t;
    }
  }
  return s;
}

// the chain up to the stepping screen, six hours forward with counters
void open_walk(DialogDriver& drive) {
  drive.then(DialogDriver::click("Zähler EINSCHALTEN mit AUTOMATISCHEM"))
      .then(DialogDriver::click("NORMALE AUSGABE"))
      .then(DialogDriver::click("STUNDEN"))
      .then(DialogDriver::fill({"6"}, "OK"))
      .then(DialogDriver::click("VOR"))
      .then([](QDialog* d) { press(d, Qt::Key_Space); });  // his eingalp sheet
}

}  // namespace

TEST_CASE("ZEIT-WANDERN steps by the chosen unit and steers with his keys") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  const double jd0 = MainWindowProbe::panel_jd(*w);
  Screen first;
  Screen doubled;
  Screen back;
  Screen last;
  bool mid_visible = false;
  bool asp_visible = false;
  DialogDriver drive;
  open_walk(drive);
  drive.then([&](QDialog* d) {
         // the first step runs as the screen opens
         QApplication::processEvents();
         first = read_screen(d);
         for (const QLabel* l : d->findChildren<QLabel*>()) {
           if (l->text().contains("HALBSUMMEN - ZÄHLER")) {
             mid_visible = l->isVisibleTo(d);
           }
           if (l->text().contains("ASPEKTE - ZÄHLER")) {
             asp_visible = l->isVisibleTo(d);
           }
         }
         // his + = Verdoppelung, the next step goes twelve hours
         press(d, Qt::Key_Plus);
         doubled = read_screen(d);
         // his R = Rückwärts
         press(d, Qt::Key_R);
         back = read_screen(d);
         press(d, Qt::Key_Escape);
         last = read_screen(d);
       })
      .then(DialogDriver::click(" NEIN = ENDE "))
      .then(DialogDriver::click(" NEIN "));  // VARIIERTE ZEIT in RADIX ÜBERNEHMEN ?
  MainWindowProbe::time_wander(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(first.diff.endsWith("+(   0 D  6 H  0 M )"));
  CHECK(first.dir == "Vorwärts !");
  CHECK(mid_visible);
  CHECK(asp_visible);
  CHECK(first.mid.contains(" HALBSUMMEN - ZÄHLER :    1 VORGÄNGE"));
  CHECK(first.asp.contains("      1 VORGÄNGE"));
  CHECK(doubled.diff.endsWith("+(   0 D 18 H  0 M )"));
  CHECK(back.diff.endsWith("+(   0 D  6 H  0 M )"));
  CHECK(back.dir == "Rückwärts!");
  CHECK(back.mid.contains(":    3 VORGÄNGE"));
  // ESC shows the counters once more without a new chart
  CHECK(last.mid.contains(":    3 VORGÄNGE"));
  // NEIN leaves the radix at its own time
  CHECK(MainWindowProbe::panel_jd(*w) == doctest::Approx(jd0).epsilon(1e-12));
}

TEST_CASE("ZEIT-WANDERN takes the varied time into the radix on JA") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  const double jd0 = MainWindowProbe::panel_jd(*w);
  DialogDriver drive;
  open_walk(drive);
  drive.then([](QDialog* d) {
         QApplication::processEvents();
         press(d, Qt::Key_Escape);
       })
      .then(DialogDriver::click(" NEIN = ENDE "))
      .then(DialogDriver::click("JA"));
  MainWindowProbe::time_wander(*w);
  CHECK(drive.pending() == 0);
  CHECK(MainWindowProbe::panel_jd(*w) == doctest::Approx(jd0 + 0.25).epsilon(1e-9));
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  CHECK(MainWindowProbe::slot(*w, 0)->hour == 17);
  CHECK(MainWindowProbe::slot(*w, 0)->minute == 8);
}

TEST_CASE("ZEIT-WANDERN goes on with a new unit and resets the counters on request") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  const double jd0 = MainWindowProbe::panel_jd(*w);
  QString zeroed;
  QStringList titles;
  DialogDriver drive;
  open_walk(drive);
  drive.then([](QDialog* d) {
         QApplication::processEvents();
         press(d, Qt::Key_Escape);
       })
      .then(DialogDriver::click("Weiter"))
      .then(DialogDriver::click("JA"))  // Die ZÄHLER RÜCKSETZEN ?
      .then([&zeroed](QDialog* d) {
        if (auto* box = qobject_cast<QMessageBox*>(d)) {
          zeroed = box->text();
        }
        d->accept();
      })
      .then([&titles](QDialog* d) {  // ZEIT-EINHEIT ? again
        titles << d->windowTitle();
        d->reject();
      })
      .then(DialogDriver::click(" NEIN "));
  MainWindowProbe::time_wander(*w);
  CHECK(drive.pending() == 0);
  CHECK(zeroed == "ZÄHLER auf NULL !");
  CHECK(titles == QStringList{"AUSWAHL"});
  CHECK(MainWindowProbe::panel_jd(*w) == doctest::Approx(jd0).epsilon(1e-12));
}

TEST_CASE("the session switch-off skips the counter box on the next walk") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  QStringList first_titles;
  DialogDriver drive;
  drive.then(DialogDriver::click("Zähler DAUERND AUSSCHALTEN"))
      .then(DialogDriver::click("NORMALE AUSGABE"))
      .then([](QDialog* d) { d->reject(); });  // ZEIT-EINHEIT ?
  MainWindowProbe::time_wander(*w);
  // the second walk opens straight with the unit
  drive.then([&first_titles](QDialog* d) {
    const auto* l = d->findChild<QLabel*>();
    first_titles << (l != nullptr ? l->text() : QString());
    for (const QLabel* x : d->findChildren<QLabel*>()) {
      first_titles << x->text();
    }
    d->reject();
  });
  MainWindowProbe::time_wander(*w);
  CHECK(drive.pending() == 0);
  CHECK(first_titles.join(QChar(0x0A)).contains("ZEIT-EINHEIT ?"));
}

namespace {

// the chain up to the place walk, two degrees east and no latitude shift
void open_place_walk(DialogDriver& drive, const QString& lon, const QString& lat, const QString& lat_dir = QString()) {
  drive.then(DialogDriver::fill({lon}, "OK")).then(DialogDriver::click("ÖSTLICH")).then(DialogDriver::fill({lat}, "OK"));
  if (!lat_dir.isEmpty()) {
    drive.then(DialogDriver::click(lat_dir));
  }
  drive.then([](QDialog* d) { press(d, Qt::Key_Space); });  // his ortgalp sheet
}

QString diff_of(QDialog* d) {
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    if (l->text().startsWith("LÄNGEN-Differenz")) {
      return l->text();
    }
  }
  return {};
}

QString capital_of(QDialog* d) {
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    if (l->text().startsWith(" Ca.")) {
      return l->text();
    }
  }
  return {};
}

}  // namespace

TEST_CASE("ORT-WANDERN walks the place at a fixed moment with his keys") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  const double jd0 = MainWindowProbe::panel_jd(*w);
  const double lon0 = MainWindowProbe::lon(*w);
  QString start;
  QString doubled;
  QString turned;
  DialogDriver drive;
  open_place_walk(drive, "2", "0");
  drive.then([&](QDialog* d) {
         QApplication::processEvents();
         start = diff_of(d);
         // his + doubles both shifts and steps
         press(d, Qt::Key_Plus);
         doubled = diff_of(d);
         // his L turns the longitude round
         press(d, Qt::Key_L);
         turned = diff_of(d);
         press(d, Qt::Key_Escape);
       })
      .then(DialogDriver::click(" NEIN = ENDE "));
  MainWindowProbe::place_wander(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(start == QString::fromUtf8("LÄNGEN-Differenz\n    0.000°\nBREITEN-Differenz\n    0.000°"));
  CHECK(doubled.contains("    4.000°\nBREITEN"));
  CHECK(turned.contains("    0.000°\nBREITEN"));
  // back at the start nothing is asked and nothing changes
  CHECK(MainWindowProbe::lon(*w) == doctest::Approx(lon0));
  CHECK(MainWindowProbe::panel_jd(*w) == doctest::Approx(jd0).epsilon(1e-12));
}

TEST_CASE("ORT-WANDERN finds the nearest metropolis on F10 and takes its place on JA") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  const double jd0 = MainWindowProbe::panel_jd(*w);
  QString box;
  QString snapped;
  QString after;
  DialogDriver drive;
  open_place_walk(drive, "1", "0");
  drive.then([&](QDialog* d) {
         QApplication::processEvents();
         press(d, Qt::Key_F10);
         box = capital_of(d);
         // ENTER snaps to the metropolis with his message
         QTimer::singleShot(0, d, [d]() { press(d, Qt::Key_Return); });
       })
      .then([&snapped](QDialog* d) {
        if (auto* m = qobject_cast<QMessageBox*>(d)) {
          snapped = m->text();
        }
        d->accept();
      })
      .then([&after](QDialog* d) {
        // back on the walk, Space closes the box and redraws there
        press(d, Qt::Key_Space);
        after = diff_of(d);
        press(d, Qt::Key_Escape);
      })
      .then(DialogDriver::click(" NEIN = ENDE "))
      .then(DialogDriver::click("JA"));
  MainWindowProbe::place_wander(*w);
  CHECK(drive.pending() == 0);
  // 11°19' E 48°10' N lies some 19 km west and 2 km north of Munich
  CHECK(box.startsWith(" Ca.  19 km westlich  \n Ca.   2 km nördlich  \n Munich/GERMANY"));
  CHECK(snapped == "Im Folgenden weiter ab den Koordinaten von Munich/GERMANY !");
  CHECK(after.contains("    0.250°"));
  // JA keeps the place of Munich at the same moment
  CHECK(MainWindowProbe::lon(*w) == doctest::Approx(11.5667).epsilon(1e-4));
  CHECK(MainWindowProbe::panel_jd(*w) == doctest::Approx(jd0).epsilon(1e-12));
}

TEST_CASE("ORT-WANDERN stops where Placidus fails and asks the shifts again") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  MainWindowProbe::houses(*w, 1);
  QString problem;
  DialogDriver drive;
  // a zero longitude shift asks no direction, the latitude one does
  drive.then(DialogDriver::fill({"0"}, "OK"))
      .then(DialogDriver::fill({"45"}, "OK"))
      .then(DialogDriver::click("NÖRDLICH"))
      .then([](QDialog* d) { press(d, Qt::Key_Space); })
      .then([](QDialog* d) {
        QApplication::processEvents();
        QTimer::singleShot(0, d, [d]() { press(d, Qt::Key_X); });
      })
      .then([&problem](QDialog* d) {
        if (auto* m = qobject_cast<QMessageBox*>(d)) {
          problem = m->text();
        }
        d->accept();
      })
      .then([](QDialog* d) { d->reject(); });  // GRAD LÄNGEN-VERSCHIEBUNG again
  MainWindowProbe::place_wander(*w);
  CHECK(drive.pending() == 0);
  CHECK(problem == "Placidus UNGÜLTIG !");
}

TEST_CASE("UHR asks his system time box, takes the clock place and runs its own screen") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  QString alert_title;
  QStringList alert_buttons;
  bool place_locked = false;
  QString note;
  QString view_title;
  DialogDriver drive;
  drive
      .then([&](QDialog* d) {
        alert_title = d->windowTitle();
        for (const QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
          alert_buttons << b->text();
        }
        DialogDriver::click("RICHTIG ? = OK")(d);
      })
      .then([&place_locked](QDialog* d) {
        const QList<QLineEdit*> edits = d->findChildren<QLineEdit*>();
        // his eingabe(0,-1,1,10), the place and its coordinates only
        place_locked = edits.size() >= 12 && edits[9]->isEnabled() && !edits[11]->isEnabled();
        DialogDriver::click("OK")(d);
      })
      .then(DialogDriver::click(" NEIN "))  // LAUFENDE UHR als DATENSATZ ÜBERNEHMEN ?
      .then([&note](QDialog* d) {
        if (auto* m = qobject_cast<QMessageBox*>(d)) {
          note = m->text();
        }
        d->accept();
      })
      .then([&view_title](QDialog* d) {
        view_title = d->windowTitle();
        d->reject();
      });
  MainWindowProbe::uhr(*w);
  CHECK(drive.pending() == 0);
  CHECK(alert_title == "ENTSCHEIDUNG !");
  CHECK(alert_buttons.size() == 2);
  CHECK(place_locked);
  CHECK(note == "Solange UHR SICHTBAR wird HOROSKOP ALLE 15 SEK NACHGEZEICHNET !");
  CHECK(view_title == "UHR LAUFEND | ENDE mit ESC-TASTE");
  CHECK(MainWindowProbe::uhr_on(*w));
  CHECK(MainWindowProbe::clock_strip(*w).startsWith("UHR: "));
  CHECK(MainWindowProbe::clock_strip(*w).contains(" AC: "));
  CHECK(MainWindowProbe::clock_strip(*w).contains(" STZ: "));
  CHECK(MainWindowProbe::uhr_slot(*w) == -1);
  // the second call offers to delete the clock
  drive.then(DialogDriver::click("ABBRUCH = UHR LÖSCHEN !"));
  MainWindowProbe::uhr(*w);
  CHECK(drive.pending() == 0);
  CHECK_FALSE(MainWindowProbe::uhr_on(*w));
}

TEST_CASE("UHR taken over as a record ticks in its slot") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  DialogDriver drive;
  drive.then(DialogDriver::click("RICHTIG ? = OK")).then(DialogDriver::click("OK")).then(DialogDriver::click(" JA "));
  MainWindowProbe::uhr(*w);
  CHECK(drive.pending() == 0);
  REQUIRE(MainWindowProbe::uhr_slot(*w) == 1);
  const auto rec = MainWindowProbe::slot(*w, 1);
  REQUIRE(rec.has_value());
  CHECK(rec->surname == "UHR");
  CHECK(rec->comment == "WIRD ALLE 15 SEK AKTUALISIERT !");
  // the wheel shows the moment of now
  const QDateTime now = QDateTime::currentDateTimeUtc();
  const double jd_now = julian_day({now.date().day(), now.date().month(), now.date().year(),
                                    static_cast<double>(now.time().hour()), now.time().minute() + now.time().second() / 60.0});
  CHECK(std::abs(MainWindowProbe::chart(*w).jd_ut - jd_now) < 60.0 / 86400.0);
}

namespace {

// holds a walk open longer than the settle time of the panel history
void linger(int ms) {
  QEventLoop loop;
  QTimer::singleShot(ms, &loop, &QEventLoop::quit);
  loop.exec();
}

// the UHR take over up to the ticking record
void take_clock_over(MainWindow& w) {
  DialogDriver drive;
  drive.then(DialogDriver::click("RICHTIG ? = OK")).then(DialogDriver::click("OK")).then(DialogDriver::click(" JA "));
  MainWindowProbe::uhr(w);
  REQUIRE(drive.pending() == 0);
  REQUIRE(MainWindowProbe::uhr_slot(w) >= 0);
}

}  // namespace

TEST_CASE("ZEIT-WANDERN keeps its steps out of the history and the slot") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  linger(1000);
  const auto slot = MainWindowProbe::slot(*w, 0);
  // the count as the walk begins, after the boxes before it
  std::size_t steps = 0;
  std::size_t during = 0;
  std::size_t before_end = 0;
  DialogDriver drive;
  drive.then(DialogDriver::click("Zähler EINSCHALTEN mit AUTOMATISCHEM"))
      .then(DialogDriver::click("NORMALE AUSGABE"))
      .then([&](QDialog* d) {
        steps = MainWindowProbe::history_steps(*w);
        DialogDriver::click("STUNDEN")(d);
      })
      .then(DialogDriver::fill({"6"}, "OK"))
      .then(DialogDriver::click("VOR"))
      .then([](QDialog* d) { press(d, Qt::Key_Space); });
  drive.then([&](QDialog* d) {
         QApplication::processEvents();
         press(d, Qt::Key_Plus);
         // longer than the settle time of the history, every step used to
         // land in the Zurück list and in the active slot
         linger(1000);
         during = MainWindowProbe::history_steps(*w);
         press(d, Qt::Key_Escape);
       })
      .then(DialogDriver::click(" NEIN = ENDE "))
      .then([&](QDialog* d) {
        before_end = MainWindowProbe::history_steps(*w);
        DialogDriver::click(" NEIN ")(d);
      });
  MainWindowProbe::time_wander(*w);
  CHECK(drive.pending() == 0);
  CHECK(during == steps);
  CHECK(before_end == steps);
  CHECK(MainWindowProbe::history_steps(*w) == steps);
  const auto after = MainWindowProbe::slot(*w, 0);
  REQUIRE(after.has_value() == slot.has_value());
  if (after) {
    CHECK(after->hour == slot->hour);
    CHECK(after->minute == slot->minute);
  }
}

TEST_CASE("ZEIT-WANDERN asks for the counter reset only while the counters run") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  QString next;
  DialogDriver drive;
  drive.then(DialogDriver::click("Zähler VORÜBERGEHEND AUSSCHALTEN"))
      .then(DialogDriver::click("NORMALE AUSGABE"))
      .then(DialogDriver::click("STUNDEN"))
      .then(DialogDriver::fill({"6"}, "OK"))
      .then(DialogDriver::click("VOR"))
      .then([](QDialog* d) { press(d, Qt::Key_Space); })
      .then([](QDialog* d) {
        QApplication::processEvents();
        press(d, Qt::Key_Escape);
      })
      .then(DialogDriver::click("Weiter"))
      .then([&next](QDialog* d) {  // ZEIT-EINHEIT ? at once, his halbs_ruecksetz stays silent
        for (const QLabel* l : d->findChildren<QLabel*>()) {
          next += l->text() + "|";
        }
        d->reject();
      })
      .then(DialogDriver::click(" NEIN "));
  MainWindowProbe::time_wander(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(next.contains("ZEIT-EINHEIT ?"));
  CHECK_FALSE(next.contains("RÜCKSETZEN"));
}

TEST_CASE("ZEIT-WANDERN does not step on a lone modifier key") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  Screen first;
  Screen shifted;
  DialogDriver drive;
  open_walk(drive);
  drive.then([&](QDialog* d) {
         QApplication::processEvents();
         first = read_screen(d);
         // his INKEY$ saw no character for Shift or Ctrl alone
         press(d, Qt::Key_Shift);
         press(d, Qt::Key_Control);
         shifted = read_screen(d);
         press(d, Qt::Key_Escape);
       })
      .then(DialogDriver::click(" NEIN = ENDE "))
      .then(DialogDriver::click(" NEIN "));
  MainWindowProbe::time_wander(*w);
  CHECK(drive.pending() == 0);
  CHECK(first.diff.endsWith("+(   0 D  6 H  0 M )"));
  CHECK(shifted.diff == first.diff);
}

TEST_CASE("UHR ESC on the first box leaves a running clock record alone") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  take_clock_over(*w);
  const int slot = MainWindowProbe::uhr_slot(*w);
  DialogDriver drive;
  drive.then([](QDialog* d) { d->reject(); });
  MainWindowProbe::uhr(*w);
  CHECK(drive.pending() == 0);
  CHECK(MainWindowProbe::uhr_on(*w));
  CHECK(MainWindowProbe::uhr_slot(*w) == slot);
}

TEST_CASE("UHR with a clock record greys its entry like men2 and draws the clock once") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  QAction* entry = MainWindowProbe::clock_action(*w);
  REQUIRE(entry != nullptr);
  const QString plain = entry->text();
  CHECK(entry->isEnabled());
  take_clock_over(*w);
  // his " *  => DATENSATZ 'UHR' G/H" grey while zeuhr > 0
  CHECK(entry->text() == "=> DATENSATZ 'UHR' G/H…");
  CHECK_FALSE(entry->isEnabled());
  // a call anyway shows the clock chart once and asks nothing more
  DialogDriver drive;
  drive.then(DialogDriver::click("RICHTIG ? = OK"));
  MainWindowProbe::uhr(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK(MainWindowProbe::active_slot(*w) == MainWindowProbe::uhr_slot(*w));
  const QDateTime now = QDateTime::currentDateTimeUtc();
  const double jd_now = julian_day({now.date().day(), now.date().month(), now.date().year(),
                                    static_cast<double>(now.time().hour()), now.time().minute() + now.time().second() / 60.0});
  CHECK(std::abs(MainWindowProbe::chart(*w).jd_ut - jd_now) < 60.0 / kSecondsPerDay);
  // his uhr_kon_l brings the plain entry back
  MainWindowProbe::clear_clock(*w);
  CHECK(entry->text() == plain);
  CHECK(entry->isEnabled());
}

TEST_CASE("UHR ticks its record only while no output stands in front") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, walker());
  take_clock_over(*w);
  const QTime marker(1, 2, 3);
  const auto mark = [&w, &marker]() {
    const QSignalBlocker block(MainWindowProbe::time(*w));
    MainWindowProbe::time(*w)->setTime(marker);
  };
  mark();
  MainWindowProbe::clock_gate_open(*w);
  QTime in_front;
  {
    QDialog output;
    QTimer::singleShot(0, &output, [&]() {
      MainWindowProbe::clock_tick(*w);
      in_front = MainWindowProbe::time(*w)->time();
      output.reject();
    });
    output.exec();
  }
  // his main loop alone moved zeuhr, the panel kept what the output showed
  CHECK(in_front == marker);
  // back at the main screen the tick moves the record and his
  // mainkont_dat_zeit shows its moment in the panel
  MainWindowProbe::clock_gate_open(*w);
  MainWindowProbe::clock_tick(*w);
  CHECK(MainWindowProbe::time(*w)->time() != marker);
}
