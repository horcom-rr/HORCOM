// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// UHR, his uhr with acmcl. The running clock of the chosen place, as an
// output of its own or taken over as a record that ticks in its slot,
// with the status strip of UHR, AC, MC and sidereal time underneath.

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

#include "choice_dialog.hpp"
#include "horcom/core/constants.hpp"
#include "main_window.hpp"
#include "record_mask_dialog.hpp"
#include "robert_text.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

// his TIMER - t2% > 15000, the clock screen redraws every fifteen seconds
constexpr int kClockScreenMs = 15000;
// his TIMER - tv% > 5000, the taken over record ticks every five seconds
constexpr int kClockRecordMs = 5000;

// the whole seconds of a clock moment, the last second of the day at most
// like the panel clock
int clock_seconds(const CalendarDate& d) {
  return std::min(static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5), kSecondsPerDay - 1);
}

}  // namespace

// the moment of the running clock, UT from the system
CalendarDate MainWindow::clock_now() {
  const QDateTime now = QDateTime::currentDateTimeUtc();
  return {now.date().day(), now.date().month(), now.date().year(), static_cast<double>(now.time().hour()),
          now.time().minute() + now.time().second() / 60.0};
}

// ported from uhr_par, the clock place with the moment of now
ChartInput MainWindow::clock_input() const {
  ChartInput in = record_input(uhr_place_);
  in.date_ut = clock_now();
  return in;
}

// ported from uhr_kon_l, the clock forgets its place and record
void MainWindow::clock_off() {
  uhr_on_ = false;
  uhr_place_set_ = false;
  if (uhr_slot_ >= 0) {
    uhr_slot_ = -1;
    recompute();
  }
  statusBar()->hide();
  clock_menu_update();
}

// his gouhr$ = go$ with gluhr and gguhr. His zeitzon box turned the system
// clock into UT, the port reads UT from the system itself
void MainWindow::set_clock_place(const AafRecord& r) {
  uhr_place_ = r;
  uhr_place_.surname = "UHR";
  uhr_place_.given.clear();
  uhr_place_.zone = kUtZoneText;
  uhr_place_set_ = true;
}

// his a2113 with bem$ = "WIRD ALLE 15 SEK AKTUALISIERT !"
AafRecord MainWindow::clock_record(const CalendarDate& d) const {
  AafRecord rec = uhr_place_;
  const int seconds = clock_seconds(d);
  rec.day = d.day;
  rec.month = d.month;
  rec.year = d.year;
  rec.hour = seconds / 3600;
  rec.minute = (seconds / 60) % 60;
  rec.second = seconds % 60;
  rec.zone = kUtZoneText;
  rec.comment = "WIRD ALLE 15 SEK AKTUALISIERT !";
  return rec;
}

// his men2 entry of UHR, " *  => DATENSATZ 'UHR' G/H" and grey while the
// clock record ticks, zeuhr > 0
void MainWindow::clock_menu_update() {
  if (clock_action_ == nullptr) {
    return;
  }
  // the menu's own caption waits in a property while the record ticks
  constexpr const char* kPlainCaption = "plainCaption";
  const bool record = uhr_slot_ >= 0;
  if (!clock_action_->property(kPlainCaption).isValid()) {
    clock_action_->setProperty(kPlainCaption, clock_action_->text());
  }
  const QString caption = record ? tr("=> DATENSATZ 'UHR' G/H…") : clock_action_->property(kPlainCaption).toString();
  if (clock_action_->text() != caption) {
    clock_action_->setText(caption);
  }
  clock_action_->setEnabled(!record);
}

// ported from acmcl, the strip under the main screen
void MainWindow::clock_strip_update() {
  if (!uhr_on_ || clock_strip_ == nullptr) {
    return;
  }
  const ChartInput in = clock_input();
  const Chart c = compute_chart(in, current_settings(), vsop_, eph_);
  if (!c.ok) {
    return;
  }
  const int seconds = clock_seconds(in.date_ut);
  // his "UHR: " + h + "h " + m + "m " + s + "s UT" with " AC: ", " MC: " and " STZ: " + homise$
  clock_strip_->setText(QString::asprintf("UHR: %2dh %2dm %2ds UT", seconds / 3600, (seconds / 60) % 60, seconds % 60) +
                        "   |   " + tr(" AC: %1 ").arg(zodiac(c.b[body::kAscendant].el)) + "  |  " +
                        tr(" MC: %1 ").arg(zodiac(c.b[body::kMc].el)) + "  |  " +
                        tr(" STZ: %1 ").arg(homise_text(c.armc_deg, 0)));
  clock_strip_->show();
  statusBar()->show();
}

// ported from uhr
void MainWindow::uhr() {
  const QString now = QDateTime::currentDateTime().toString("HH:mm:ss");
  // his s$ = "SYSTEM-UHRZEIT " with mer$, "ABBRUCH = UHR LÖSCHEN !" once a place is set
  QStringList buttons{tr("SYSTEM-UHRZEIT   %1  RICHTIG ? = OK ").arg(now),
                      tr("Über 'SYSTEMSTEUERUNG' neu EINSTELLEN ? = &Zurück zum HAUPT - MENÜ")};
  if (uhr_place_set_) {
    buttons << tr("ABBRUCH = UHR LÖSCHEN !");
  }
  const int r = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("Wenn UHRZEIT GROB FALSCH ist"), tr("HORCOM NEU STARTEN !")},
                                  buttons, 0);
  // ESC leaves a running clock alone, his two other answers clear it
  if (r < 0) {
    return;
  }
  if (r != 0) {
    clock_off();
    return;
  }
  if (!uhr_place_set_) {
    // his et$(1) = "UHR" with @eingabe(0,-1,1,10), the place of the clock
    AafRecord seed = panel_record();
    seed.surname = "UHR";
    seed.given.clear();
    RecordMaskDialog mask(seed, tr("EREIGNIS-ORT  EINGEBEN !  ->  EVENTL. TAB - TASTE !"), RecordMaskDialog::Mode::kShow,
                          data_dir_, this);
    mask.limit_fields(10);
    if (mask.exec() != QDialog::Accepted) {
      return;
    }
    set_clock_place(mask.record());
  }
  uhr_on_ = true;
  clock_strip_update();
  // his IF z < 5 && zeuhr = 0 && uhraktuell! = 0
  if (uhr_slot_ < 0 && active_slot_ < 4) {
    const int re = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"),
                                     {tr(" LAUFENDE UHR als DATENSATZ ÜBERNEHMEN ?"), tr("Wird NICHT empfohlen !")},
                                     {tr(" NEIN "), tr(" JA ")}, 0);
    if (re == 1) {
      const int slot = claim_radix_slot();
      if (slot < 0) {
        return;
      }
      uhr_slot_ = slot;
      set_slot(slot, clock_record(clock_now()), true);
      clock_menu_update();
      return;
    }
  }
  // his loop ends at once while zeuhr > 0, the clock chart drawn once
  if (uhr_slot_ >= 0) {
    set_slot(uhr_slot_, clock_record(clock_now()), true);
    return;
  }
  // his output loop, the clock chart until ESC
  QDialog view(this);
  view.setWindowTitle(tr("UHR LAUFEND | ENDE mit ESC-TASTE"));
  auto* v = new QVBoxLayout(&view);
  v->setContentsMargins(0, 0, 0, 0);
  auto* wheel = new WheelWidget(&view);
  v->addWidget(wheel);
  const auto draw = [&]() {
    const ChartInput in = clock_input();
    // his @textc(amh& - 17,bmh& + 6,13," UHR ")
    if (auto dl = sheet_wheel(uhr_place_, in, current_settings(), " UHR ")) {
      wheel->set_display_list(std::move(*dl));
    }
  };
  draw();
  QTimer timer;
  connect(&timer, &QTimer::timeout, &view, draw);
  timer.start(kClockScreenMs);
  // his IF anz! = 0, the note on the first screen
  QTimer::singleShot(0, &view, [&view]() {
    QMessageBox::information(&view, tr(" Information "),
                             tr("Solange UHR SICHTBAR wird HOROSKOP ALLE 15 SEK NACHGEZEICHNET !"));
  });
  view.resize(size());
  view.exec();
}

// his main loop ran acmcl every second and moved the clock record every
// five, never while an output stood in front. A tick inside a modal box
// would redraw the clock over KORREKTUR, ZEIT-WANDERN or any result
void MainWindow::clock_tick() {
  if (QApplication::activeModalWidget() != nullptr) {
    return;
  }
  clock_menu_update();
  clock_strip_update();
  if (uhr_slot_ < 0) {
    return;
  }
  if (clock_record_clock_.isValid() && clock_record_clock_.elapsed() < kClockRecordMs) {
    return;
  }
  clock_record_clock_.restart();
  if (active_slot_ == uhr_slot_ && !active_is_solar_) {
    recompute();
  }
}

}  // namespace horcom
