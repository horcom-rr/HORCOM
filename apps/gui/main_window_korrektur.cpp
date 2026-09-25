// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// The KORREKTUR world of the DIVERSES menu, the birth time rectification
// of korr and the primary directed axes session of prima.

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QScopeGuard>

#include "a18_rows.hpp"
#include "banner.hpp"
#include "choice_dialog.hpp"
#include "horcom/chart/directions.hpp"
#include "horcom/chart/rectification.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/pair_sheet.hpp"
#include "main_window.hpp"
#include "place_dialog.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

// his prima turned the summed sidereal time variation into birth time
// with dst = 0.0027379 days a degree, a day over the year length, 3.94
// clock minutes a degree against his own dialog text of four. One degree
// of sidereal time passes in this many solar days
constexpr double kDaysPerSiderealDegree = 1.0 / (kDegPerCircle * kSolarToSiderealRate);

// the clock minutes his boxes quote for a degree of sidereal time
constexpr double kClockMinutesPerDegree = 4.0;

// the PLACIDUS row of the house system box, his haw& = 1
constexpr int kPlacidusRow = 0;

// primhorg writes the centre in size 10 and the edges in size 13, the
// corner rows of the screen sheet keep their own 11
constexpr double kCentreText = 10.0;
constexpr double kEdgeText = 13.0;
constexpr double kCornerText = 11.0;
// the margin of the corner blocks and the pair rows of the sheet, stacked
// upward from their base line
constexpr double kSheetEdge = 8.0;
constexpr double kFootBase = 466.0;
constexpr double kFootStep = 12.0;
// the black ground of an inverse line reaches this far beyond the ink
constexpr double kInverseMargin = 1.0;

}  // namespace

// ported from plre, the notice for the systems two to seven
void MainWindow::placidus_notice() {
  const HouseSystem sys = current_settings().houses;
  if (sys >= HouseSystem::kTopocentric && sys <= HouseSystem::kEqualVehlow) {
    ChoiceDialog::ask(this, tr("!! ACHTUNG !!"),
                      {tr("PLACIDUS - HÄUSER ERFORDERLICH !"), tr("ES WIRD VORÜBERGEHEND UMGESCHALTET !")},
                      {tr("WEITER")});
  }
}

// korr keeps haw_merk& for korrend and clears hrg! for good, prima's own
// h! only hands the cleared value back. The history holds still so the
// session counts as one step
std::function<void()> MainWindow::korrektur_frame(bool always_placidus) {
  std::function<void()> release = hold_history();
  const int houses_before = houses_->currentIndex();
  // his CLR hrg!, the searches and the directed axes run geocentric
  helio_->setChecked(false);
  //RR Nur PLACIDUS !
  const HouseSystem sys = current_settings().houses;
  placidus_notice();
  // his plre switches the systems two to seven, prima's haw& = 1 all
  if (always_placidus || (sys >= HouseSystem::kTopocentric && sys <= HouseSystem::kEqualVehlow)) {
    const QSignalBlocker block(houses_);
    houses_->setCurrentIndex(kPlacidusRow);
  }
  recompute();
  return [this, houses_before, release = std::move(release)]() {
    {
      const QSignalBlocker block(houses_);
      houses_->setCurrentIndex(houses_before);
    }
    release();
    recompute();
  };
}

// ported from korr
void MainWindow::correction() {
  if (!last_chart_) {
    return;
  }
  const auto frame = qScopeGuard(korrektur_frame(false));
  for (;;) {
    // his ue$(0) = " *  Womit " + wol$ + k$ + " ?  * "
    const int es = ChoiceDialog::ask(this, tr(" *  Womit Wollen Sie Korrigieren ?  * "), {},
                                     {tr("STERNZEIT"), "MC", "AC", tr("ZWISCHEN-HÄUSERN"), tr("SONNE"), tr("MOND"),
                                      tr("* PRIMÄR DIRIGIERTE ACHSEN *"), tr("ABBRUCH")});
    CorrectionRequest q;
    std::optional<double> value;
    switch (es) {
      case 0:
        q.target = CorrectionTarget::kSiderealTime;
        // his korr0 with zeiteing
        value = ask_clock(this, tr("STERNZEIT Eingeben !"));
        break;
      case 1:
        q.target = CorrectionTarget::kMc;
        value = ask_zodiac_position(this, tr("Ekliptikale Länge DES MC Eingeben"));
        break;
      case 2:
        q.target = CorrectionTarget::kAc;
        value = ask_zodiac_position(this, tr("Ekliptikale Länge  des AC Eingeben !"));
        break;
      case 3: {
        q.target = CorrectionTarget::kCusp;
        const std::optional<double> nr = ask_number(this, tr("ZAHLEN-Eingabe !"), tr("HÄUSERSPITZE NR. ?"), 1, 12, 2, 0);
        if (!nr) {
          return;
        }
        q.cusp = static_cast<int>(*nr);
        // his CASE 1,10,7,4 : GOTO korre, the angles have their own rows
        if (q.cusp == 1 || q.cusp == 4 || q.cusp == 7 || q.cusp == 10) {
          continue;
        }
        value = ask_zodiac_position(this, tr("Ekliptikale Länge Der Spitze Haus %1 Eingeben !").arg(q.cusp, 2));
        break;
      }
      case 4:
        q.target = CorrectionTarget::kSun;
        value = ask_zodiac_position(this, tr("Ekliptikale Länge DER SONNE Eingeben"));
        break;
      case 5:
        q.target = CorrectionTarget::kMoon;
        value = ask_zodiac_position(this, tr("Ekliptikale Länge DES MONDES Eingeben"));
        break;
      case 6:
        // his @prima inside the frame of korr
        primary_axes_session();
        return;
      default:
        return;
    }
    if (!value) {
      return;
    }
    q.value = *value;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const CorrectionResult res = correct_birth_time(*last_chart_, q, make_context());
    QApplication::restoreOverrideCursor();
    if (!res.ok) {
      if (res.span_too_large) {
        QMessageBox::warning(this, "HORCOM", tr("Spanne zu groß , Neu versuchen ! "));
      } else {
        banner_->set_record(tr("Korrektur nicht gefunden"));
      }
      return;
    }
    // the corrected radix keeps the clock the birth time was read on
    apply_moment(res.jd_ut, tr("KORRIGIERT"), false, true);
    if (q.target == CorrectionTarget::kSun || q.target == CorrectionTarget::kMoon) {
      // his @a91 after the light branches, the coordinates of the moment
      coordinate_table(false);
    } else {
      // his korr21 ends with hausa, on the Placidus cusps of plre
      house_table();
    }
    return;
  }
}

// ported from prima, the directed axes shown over the radix or the event
// positions while the sidereal time is varied, the sums can move the
// radix. One question per box like his alert chain, the wheel behind
// the boxes shows the directed axes all along
void MainWindow::primary_axes_session() {
  if (!last_chart_) {
    return;
  }
  // his h! = hrg! with hrg! = 0, haw_merk& with plre and haw& = 1
  const auto frame = qScopeGuard(korrektur_frame(true));
  // his da$ = " " + das$ + TRIM$(na$(1,ze)) + "  " + datum3$
  const auto data_line = [this]() {
    return tr(" Datensatz %1").arg(record_label_.trimmed()) + "  " + datum3_text(panel_day());
  };
  // his m$ = mend1$(muuu&) + " beenden"
  const QString finish = tr("KORREKTUR beenden");
  const auto leave = [this]() {
    const QSignalBlocker block(directions_action_);
    directions_action_->setChecked(false);
    dir_sums_ = AxesSums{};
    recompute();
    banner_->set_record(record_label_.trimmed());
  };
  // one run over the radix of the moment, a take over starts a new run
  for (;;) {
    const Chart radix = *last_chart_;
    // his CLR sum,dif,varaus!,va&
    double sum = 0.0;
    double dif = 0.0;
    int va = 0;
    bool varaus = false;
    const int b = ChoiceDialog::ask(this, data_line(), {tr("Ereignis-ORT = Geburts-ORT ?")},
                                    {tr(" NEIN "), tr("JA"), finish}, 1);
    double lon = lon_->value();
    double lat = lat_->value();
    if (b == 0) {
      PlaceDialog pick(data_dir_ / "places", data_dir_ / "landnima.int", this);
      if (pick.exec() != QDialog::Accepted) {
        leave();
        return;
      }
      lon = pick.chosen().lon;
      lat = pick.chosen().lat;
    } else if (b != 1) {
      leave();
      return;
    }
    // his a37dat(200," ","DATUM EINGEBEN !"), the blank zeitv$ adds the
    // clock row whose time juld reads as UT. It opens on today at the
    // radix clock
    CalendarDate start = calendar_date(julian_day(today_date()), current_settings().calendar);
    const CalendarDate birth_ut = calendar_date(radix.jd_ut, current_settings().calendar);
    start.hour = birth_ut.hour;
    start.minute = birth_ut.minute;
    const std::optional<CalendarDate> day = ask_date(this, tr("DATUM EINGEBEN !"), start, " ");
    if (!day) {
      leave();
      return;
    }
    const double jd2 = julian_day(*day, current_settings().calendar);
    bool new_data = false;
    bool take_over = false;
    while (!new_data && !take_over) {
      // his "Mit " + rd$ + " - " + pe$ + " ?" and "oder " + ere$ + " - " + pe$ + " ?"
      const int cpl = ChoiceDialog::ask(this, data_line(), {tr("Mit RADIX - Planeten ?"), tr("oder Ereignis - Planeten ?")},
                                        {tr("RADIX"), tr("Ereignis"), finish}, dir_event_planets_ ? 1 : 0);
      if (cpl != 0 && cpl != 1) {
        leave();
        return;
      }
      const QString chosen = cpl == 0 ? tr("RADIX - Planeten") : tr("Ereignis - Planeten");
      const int ce = ChoiceDialog::ask(this, data_line(),
                                       {tr("RICHTUNG der ACHSEN - DREHUNG ?"), tr("%1 GEWÄHLT !").arg(chosen)},
                                       {tr("DIREKT   ( + )"), tr("KONVERS  ( - )"), finish});
      if (ce != 0 && ce != 1) {
        leave();
        return;
      }
      // his brm = arm + djd * 360, the variation starts fresh
      dir_jd_ = jd2;
      dir_converse_ = ce == 1;
      dir_event_planets_ = cpl == 1;
      dir_lon_ = lon;
      dir_lat_ = lat;
      dir_vary_ = 0.0;
      dif = 0.0;
      bool same_again = false;
      while (!same_again && !new_data && !take_over) {
        // his prima2, the wheel of the directed axes, primhorg gets dif,
        // sum and va& with it
        {
          const QSignalBlocker block(directions_action_);
          directions_action_->setChecked(true);
        }
        dir_sums_ = AxesSums{dif, sum, va};
        claim_wheel();
        recompute();
        if (!varaus) {
          const bool varied = dif != 0.0 || sum != 0.0;
          const QString ub = varied ? tr("VARIATION BEENDEN und VARIIERTE STZ evtl. ÜBERNEHMEN") : finish;
          const int d = ChoiceDialog::ask(this, tr("PROBEWEISE STERNZEIT VARIIEREN ?"),
                                          {tr(" 1° STZ entspr. 4 Zeitminuten "), tr("Die einzelnen STZ - VARIATIONEN"),
                                           tr("Können SUMMIERT werden !")},
                                          {tr("STZ VARIIEREN"), tr("STZ NICHT VARIIEREN"),
                                           tr("VARIATIONS - MÖGLICHKEIT AUSSCHALTEN"), ub},
                                          dif != 0.0 ? 0 : 1);
          if (d == 0) {
            const std::optional<double> v =
                ask_number(this, tr(" 1° STZ entspricht 4 Zeitminuten "),
                           tr("STZ - VARIATION in GRAD ( MIT VORZEICHEN ! ) eingeben !"), -360.0, 360.0, 0.0, 3);
            if (!v) {
              leave();
              return;
            }
            dif = *v;
            const int s = ChoiceDialog::ask(this, tr("STERNZEIT - VARIATION AUFSUMMIEREN ( SPEICHERN ) ?"), {},
                                            {tr(" SUMMIEREN "),
                                             sum != 0.0 ? tr(" NICHT WEITER SUMMIEREN ") : tr(" NICHT SUMMIEREN "),
                                             finish},
                                            sum != 0.0 ? 0 : 1);
            if (s != 0 && s != 1) {
              leave();
              return;
            }
            // his INC va& with sum = sum + dif and ADD brm,dif * 360 / tja
            ++va;
            if (s == 0) {
              sum += dif;
            }
            dir_vary_ += dif;
          } else if (d == 2) {
            // his CLR dif,sum,va& with varaus! = TRUE
            dif = 0.0;
            sum = 0.0;
            va = 0;
            varaus = true;
          } else if (d != 1) {
            if (varied) {
              break;
            }
            leave();
            return;
          }
        }
        // his prima3, the running sums and where to go on
        QStringList info;
        if (dif != 0.0) {
          // his brm + dif - arm counted the last variation twice, brm had
          // taken it in already. The total turn is the arc of the wheel
          const double turn = direct_axes(radix, lon, lat, jd2, dir_converse_, dir_vary_, HouseSystem::kPlacidus).arc_deg;
          info << tr("BETRAG der LETZTEN VARIATION der STZ :")
               << tr("%1°  ENTSPRICHT %2 ZEITMINUTEN").arg(dif, 8, 'f', 3).arg(dif * kClockMinutesPerDegree, 7, 'f', 2)
               << tr("BISHERIGE GESAMT-DREHUNG der ARMC-ACHSE ( = der STZ ) : %1°").arg(turn, 8, 'f', 3);
        }
        const bool can_vary_on = !varaus && (dif != 0.0 || sum != 0.0);
        QStringList buttons{tr("NEUER TEST mit GLEICHEM Ort und Zeit"), tr("NEUER TEST mit NEUEN DATEN")};
        if (can_vary_on) {
          buttons << tr("WEITERE VARIATION oder TEST BEENDEN");
        }
        buttons << finish;
        const int g = ChoiceDialog::ask(this, data_line(), info, buttons, dif != 0.0 && can_vary_on ? 2 : 0);
        if (g < 0) {
          leave();
          return;
        }
        if (g == 0) {
          same_again = true;
        } else if (g == 1) {
          new_data = true;
        } else if (can_vary_on && g == 2) {
          continue;
        } else if (g == buttons.size() - 1) {
          leave();
          return;
        } else {
          break;
        }
      }
      if (same_again || new_data) {
        continue;
      }
      // his primend, the sums may move the radix
      if (varaus || (sum == 0.0 && dif == 0.0)) {
        leave();
        return;
      }
      // his jd1, the radix moment a take over moves on
      double jd1 = radix.jd_ut;
      for (;;) {
        QStringList lines;
        if (sum != 0.0) {
          lines << tr("Gesamt - SUMME = %1° = %2 Zeitminuten").arg(sum, 5, 'f', 1).arg(kClockMinutesPerDegree * sum, 7, 'f', 2)
                << tr("Mittlere SUMME = %1° = %2 Zeitminuten")
                       .arg(sum / va, 5, 'f', 1)
                       .arg(kClockMinutesPerDegree * sum / va, 7, 'f', 2);
        } else {
          lines << tr("STZ - ÄNDERUNG = %1° = %2 Zeitminuten").arg(dif, 8, 'f', 3).arg(dif * kClockMinutesPerDegree, 7, 'f', 2);
        }
        const int su = ChoiceDialog::ask(this, tr("NEUE TESTS mit %1 ?").arg(data_line()), lines,
                                         {tr("NEUE TESTS und STZ-ÄNDERUNGEN LÖSCHEN"),
                                          tr("SUMME der VARIATIONEN in RADIX ÜBERNEHMEN ?"),
                                          tr("MITTLERE VARIATION in RADIX ÜBERNEHMEN ?"), tr("TESTS BEENDEN")},
                                         1);
        if (su == 0) {
          new_data = true;
          break;
        }
        if (su != 1 && su != 2) {
          leave();
          return;
        }
        const bool mean = su == 2;
        // his jd = jd1 + sum * dst, the mean divides by va&
        const double degrees = mean ? (va > 0 ? sum / va : 0.0) : sum;
        {
          const QSignalBlocker block(directions_action_);
          directions_action_->setChecked(false);
        }
        jd1 += degrees * kDaysPerSiderealDegree;
        apply_moment(jd1, tr("KORRIGIERT"), false, true);
        // the mean offers his m$ as a third way out
        QStringList ways{tr("JA = WEITERE TESTS "), tr(" TESTS BEENDEN ")};
        if (mean) {
          ways << finish;
        }
        const int u = ChoiceDialog::ask(this, data_line(), {tr("NEUE TESTS mit dem KORRIGIERTEN RADIX durchführen ? ")}, ways);
        if (u == 0) {
          take_over = true;
          break;
        }
        // his CASE 2 : GOTO primend after the mean, the sums box again
        if (mean && u == 1) {
          continue;
        }
        banner_->set_record(tr("KORRIGIERT"));
        return;
      }
    }
  }
}

// ported from primhorg. The turn and the event moment stand in the centre,
// the direction k$ over the wheel and the directed sidereal time top right
// where the radix corners keep their mode line and STZ otherwise. The
// variations and the planets e$ stack at the lower left, at his
// coordinates under the wheel they would leave the screen sheet
void MainWindow::show_directed_axes(DisplayList dl, const DirectedAxes& d) {
  constexpr double amh = kWheelCenterX;
  constexpr double bmh = kWheelCenterY;
  // "STZ-DIFF=" + STR$(b - arm,9,3) + "°"
  add_sheet_text(dl, amh - 68.0, bmh - 10.0, (QString::asprintf("STZ-DIFF=%9.3f", d.arc_deg) + QChar(0xB0)).toStdString(),
                 kCentreText);
  // ere$ + ":" with d$ and z$, the event moment in UT
  const CalendarDate ev = calendar_date(dir_jd_, current_settings().calendar);
  add_sheet_text(dl, amh - 25.0, bmh + 4.0, (tr("Ereignis") + ":").toStdString(), kCentreText);
  add_sheet_text(dl, amh - 35.0, bmh + 14.0, datum3_text(ev).toStdString(), kCentreText);
  add_sheet_text(dl, amh - 35.0, bmh + 24.0,
                 homise_text(kDegPerHour * a18::clock_seconds(ev) / kSecondsPerHour, 0).toStdString(), kCentreText);
  // deftextcol(0), white on black over a ground as wide as his fixed pitch
  const auto inverse = [&dl](double left, double bottom, const QString& caption, double height,
                             Primitive::Anchor anchor) {
    const double pitch = font_pitch(height);
    const double span = pitch * static_cast<double>(caption.size());
    Primitive ground;
    ground.kind = Primitive::Kind::kRect;
    ground.x1 = left + 0.5 * span;
    ground.y1 = bottom - 0.5 * height;
    ground.r1 = 0.5 * span + kInverseMargin;
    ground.r2 = 0.5 * height + kInverseMargin;
    ground.fill = kInkColor;
    ground.anchor = anchor;
    dl.items.push_back(ground);
    Primitive ink;
    ink.kind = Primitive::Kind::kText;
    ink.x1 = left;
    ink.y1 = bottom - 0.5 * height;
    ink.size = height;
    ink.pitch = pitch;
    ink.align_left = true;
    ink.color = kInvertedInk;
    ink.anchor = anchor;
    ink.text = caption.toStdString();
    dl.items.push_back(ink);
  };
  // k$ = @richtung$(arm,brm) at 15 over the wheel, the sign of the turn
  // names the direction
  const QString k = " " + (d.arc_deg >= 0.0 ? tr("DIREKT   ( + )") : tr("KONVERS  ( - )"));
  inverse(amh - 68.0, 15.0, k, kEdgeText, Primitive::Anchor::kSheet);
  // IF dif <> 0, VAR. STZ, VAR.-SUM.STZ and MITT.SUM.STZ above e$
  QStringList rows;
  const AxesSums& v = dir_sums_;
  if (v.dif != 0.0) {
    rows << tr("VAR. STZ  =%1°").arg(QString::asprintf("%6.2f", v.dif))
         << tr("VAR.-SUM.STZ =%1°").arg(QString::asprintf("%6.2f", v.sum))
         << tr("MITT.SUM.STZ =%1°").arg(QString::asprintf("%6.2f", v.va > 0 ? v.sum / v.va : 0.0));
  }
  double y = kFootBase - kFootStep * static_cast<double>(rows.size());
  for (const QString& row : rows) {
    Primitive line;
    line.kind = Primitive::Kind::kText;
    line.x1 = kSheetEdge;
    line.y1 = y;
    line.size = kCornerText;
    line.align_left = true;
    line.anchor = Primitive::Anchor::kCorner;
    line.text = row.toStdString();
    dl.items.push_back(line);
    y += kFootStep;
  }
  // e$, whose planets ride on the axes
  const QString e = dir_event_planets_ ? tr("Ereignis - Planeten") : tr("RADIX - Planeten");
  inverse(kSheetEdge, kFootBase + 0.5 * kCornerText, e, kCornerText, Primitive::Anchor::kCorner);
  // the corners of the sheet, "STZ:" + homise$ of the directed ARMC top
  // right, no mode line and no pair rows
  ClassicSheetText corners = classic_sheet_text();
  corners.mode.clear();
  corners.stz = (tr("STZ") + ":" + homise_text(d.armc_deg, 0) + " ").toStdString();
  corners.pair_name1.clear();
  corners.pair_moment1.clear();
  corners.pair_name2.clear();
  corners.pair_moment2.clear();
  corners.pair_list.clear();
  corners.pair_note.clear();
  add_corner_text(dl, corners, kSheetEdge, kScreenSheetWidth / 2.0, kScreenSheetWidth - kSheetEdge);
  full_sheet_ = false;
  wheel_->set_display_list(std::move(dl));
}

}  // namespace horcom
