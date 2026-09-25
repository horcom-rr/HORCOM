// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// MULTIPLE DIREKTIONEN / HARMONICS, the multiple session with its MODUS
// box, the sheet of multi11 and harm21 and the weit_dat question.

#include <QAction>
#include <QApplication>
#include <QDate>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QKeyEvent>
#include <QLabel>
#include <QMenuBar>
#include <limits>

#include "banner.hpp"
#include "choice_dialog.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/progressions.hpp"
#include "horcom/chart/signs.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/pair_sheet.hpp"
#include "main_window.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

void sheet_text(DisplayList& dl, double x, double bottom, const QString& s, double size) {
  add_sheet_text(dl, x, bottom, s.toStdString(), size);
}

// the mul$ of the six MULTI modes
// the pixels between the wait bar and the foot of the waiting screen
constexpr int kWaitBarGap = 8;

constexpr const char* kMultiName[7] = {"", "MULTI 1", "MULTI 2", "MULTI 3", "MULTI-0-OST", "MULTI-0-WEST", "MULTI-ARC"};

// his wart, any key or a mouse button on the window goes on, ESC too.
// The function keys and the ALT letters stay with the main window like
// his druck_enbl_alt and mehrf_1 inside the wart loop, the right button
// with his einzel_plan_wahl. No menu shortcut and no mnemonic runs while
// the screen waits, else the HARDCOPY after the wait would print what
// the action changed. A box opened on top of the wait and an open popup
// keep their own keys and clicks
class KeyWait final : public QObject {
 public:
  explicit KeyWait(QEventLoop& loop) : loop_(loop), front_(QApplication::activeModalWidget()) {}
  int key = 0;

 protected:
  bool eventFilter(QObject*, QEvent* e) override {
    if (QApplication::activeModalWidget() != front_ || QApplication::activePopupWidget() != nullptr) {
      return false;
    }
    switch (e->type()) {
      case QEvent::ShortcutOverride:
        // the key arrives as a plain press instead of running an action
        e->accept();
        return true;
      case QEvent::KeyPress: {
        const auto* k = static_cast<QKeyEvent*>(e);
        if (passes(*k)) {
          return false;
        }
        key = k->key();
        loop_.quit();
        return true;
      }
      case QEvent::MouseButtonPress:
        if (static_cast<QMouseEvent*>(e)->button() == Qt::RightButton) {
          return false;
        }
        loop_.quit();
        return true;
      default:
        return false;
    }
  }

 private:
  // the keys that do not end the wait, a lone modifier and the keys the
  // main window serves
  static bool passes(const QKeyEvent& k) {
    switch (k.key()) {
      case Qt::Key_Shift:
      case Qt::Key_Control:
      case Qt::Key_Meta:
      case Qt::Key_Alt:
      case Qt::Key_AltGr:
        return true;
      default:
        return (k.key() >= Qt::Key_F1 && k.key() <= Qt::Key_F12) || k.modifiers() == Qt::AltModifier;
    }
  }

  QEventLoop& loop_;
  // the modal window the wait runs over, null for the main window
  const QWidget* front_;
};

}  // namespace

// ported from wart, the screen stays until a key or a click. His full
// screen chart left no menu to click, the shell greys its menu and a bar
// on the waiting screen names the key, so the wait never looks like a
// menu that stopped answering
int MainWindow::wait_key(QWidget* over) {
  QLabel bar(tr("WEITER mit LEERTASTE oder Mausklick"), over);
  bar.setObjectName(QStringLiteral("waitBar"));
  bar.adjustSize();
  bar.move((over->width() - bar.width()) / 2, over->height() - bar.height() - kWaitBarGap);
  bar.show();
  bar.raise();
  menuBar()->setEnabled(false);
  QEventLoop loop;
  KeyWait wait(loop);
  qApp->installEventFilter(&wait);
  loop.exec();
  qApp->removeEventFilter(&wait);
  menuBar()->setEnabled(true);
  return wait.key;
}

QString MainWindow::multi_name(MultiMode mode) {
  return QString(kMultiName[std::clamp(static_cast<int>(mode), 1, 6)]);
}

// ported from haus_ber, nothing after ABBRUCH
std::optional<bool> MainWindow::ask_house_mode() {
  // HÄUSER wie PLANETEN BEHANDELN ( = STANDARD ) ? ... NEU BERECHNEN ?
  const int rt = ChoiceDialog::ask(this, tr(" HINWEIS "),
                                   {tr("HÄUSER wie PLANETEN BEHANDELN ( = STANDARD ) ?"), tr("oder"),
                                    tr("AUFGRUND des NEUEN MC"), tr("NEU BERECHNEN ?")},
                                   {tr("Wie PLANETEN BEHANDELN"), tr("NEU RECHNEN"), tr("ABBRUCH")}, 0);
  if (rt < 0 || rt == 2) {
    return std::nullopt;
  }
  return rt == 1;
}

// the BEZUGS-FAKTOR of multi3 and multi_arc, ausw_pl_hs with his three
// extra rows and the numw prompts behind them
std::optional<MultiReference> MainWindow::ask_multi_reference(const QString& mul) {
  const Chart& c = *last_chart_;
  std::vector<std::pair<int, QString>> rows;
  // IF fixpunkt& = 1 && hrg! = 0, ltp$(0) = "   FIXPUNKT", the loops of
  // multi31 and multiarc1 direct it from aa& = 0
  if (fixpunkt_ >= 0.0) {
    rows.emplace_back(body::kFixpunkt, tr("   FIXPUNKT"));
  }
  // ltp$(), the bodies of the chart, dashes where MULTI has none
  static constexpr const char* kBase[15] = {
      "",
      QT_TR_NOOP(" SONNE"),
      QT_TR_NOOP(" MOND"),
      QT_TR_NOOP(" MERKUR"),
      QT_TR_NOOP(" VENUS"),
      QT_TR_NOOP(" MARS"),
      QT_TR_NOOP(" JUPITER"),
      QT_TR_NOOP(" SATURN"),
      QT_TR_NOOP(" URANUS"),
      QT_TR_NOOP(" NEPTUN"),
      QT_TR_NOOP(" PLUTO"),
      QT_TR_NOOP("  MONDKNOTEN N"),
      QT_TR_NOOP("  MONDKNOTEN S"),
      QT_TR_NOOP("  ASZENDENT"),
      QT_TR_NOOP("     MC")};
  for (int slot = 1; slot <= body::kMc; ++slot) {
    rows.emplace_back(slot, tr(kBase[slot]));
  }
  for (int slot = body::kApogee; slot < body::kSlotCount; ++slot) {
    const BodyState& b = c.b[static_cast<std::size_t>(slot)];
    // under mult! his box dashes Transpluto and the Hamburg factors, the
    // CASE lists of mc_armcb1 and the directions never reach them
    const bool hamburg = slot >= body::kCupido && slot <= body::kPoseidon;
    if (!b.present || !b.valid || slot == body::kTranspluto || hamburg) {
      continue;
    }
    rows.emplace_back(slot, "  " + QString::fromUtf8(body::kName[static_cast<std::size_t>(slot)].data(),
                                                      static_cast<int>(body::kName[static_cast<std::size_t>(slot)].size())));
  }
  // ltp$(z& + 1) = "   HAUS NR.     ", HERR v. HAUS NR., 0 GRAD eines ZEICHENS
  constexpr int kHouseRow = 101;
  constexpr int kRulerRow = 102;
  constexpr int kSignRow = 103;
  rows.emplace_back(kHouseRow, tr("   HAUS NR.     "));
  rows.emplace_back(kRulerRow, tr(" HERR v. HAUS NR."));
  rows.emplace_back(kSignRow, tr(" 0 GRAD eines ZEICHENS"));
  const QString title = tr(" MULTIPLE DIREKTIONEN nach STEPHAN A. LEHRIEDER ! | %1").arg(mul);
  const std::optional<int> v = ask_object(this, rows, {title, tr(" BEZUGS-FAKTOR WÄHLEN !")});
  if (!v) {
    return std::nullopt;
  }
  MultiReference ref;
  if (*v == kHouseRow) {
    // hf& = @numw("HAUS NR. ? ",1,12)
    const std::optional<int> h = ask_digit(this, tr("HAUS NR. ? "), 1, 12);
    if (!h) {
      return std::nullopt;
    }
    ref.kind = MultiReference::Kind::kCusp;
    ref.house = *h;
  } else if (*v == kRulerRow) {
    const std::optional<int> h = ask_digit(this, tr("HERR von HAUS NR.? ( ALTE ZUORDNUNG ! )"), 1, 12);
    if (!h) {
      return std::nullopt;
    }
    ref.kind = MultiReference::Kind::kRuler;
    ref.house = *h;
  } else if (*v == kSignRow) {
    // ZEICHEN WÄHLEN dessen NULLPUNKT gelten soll !
    static constexpr const char* kSigns[12] = {QT_TR_NOOP("WIDDER"), QT_TR_NOOP("STIER"), QT_TR_NOOP("ZWILLINGE"), QT_TR_NOOP("KREBS"), QT_TR_NOOP("LÖWE"), QT_TR_NOOP("JUNGFRAU"),
                                               QT_TR_NOOP("WAAGE"), QT_TR_NOOP("SKORPION"), QT_TR_NOOP("SCHÜTZE"), QT_TR_NOOP("STEINBOCK"), QT_TR_NOOP("WASSERMANN"), QT_TR_NOOP("FISCHE")};
    QStringList signs;
    for (const char* sgn : kSigns) {
      signs << tr(sgn);
    }
    const int z = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("ZEICHEN WÄHLEN dessen NULLPUNKT gelten soll !")}, signs, 0);
    if (z < 0) {
      return std::nullopt;
    }
    ref.kind = MultiReference::Kind::kSignStart;
    ref.sign = z + 1;
  } else {
    ref.kind = MultiReference::Kind::kBody;
    ref.body = *v;
  }
  return ref;
}

// ported from multiple with multi1 to multi_arc and harm, the MODUS box,
// the event date, his haus_ber and BEZUGS-FAKTOR, the drawing, zeitwim
// and weit_dat, the radix returns at the end like merkr(1)
void MainWindow::multi_session() {
  // his multiple clears hrg! for the session, the heliocentric panel
  // chart gets a geocentric MULTI sheet from recompute
  if (!last_chart_) {
    return;
  }
  // ue$(0) = "MODUS WÄHLEN !"
  const int es = ChoiceDialog::ask(this, tr("MODUS WÄHLEN !"), {},
                                   {tr("  MULTI 1"), tr("  MULTI 2"), tr("  MULTI 3"), tr("MULTI NULL OST"),
                                    tr("MULTI NULL WEST"), tr("MULTI-ARC ( VERSUCH )"), tr("HARMONICS"), tr(" ABBRUCH")},
                                   0);
  if (es < 0 || es == 7) {
    return;
  }
  const bool harmonic = es == 6;
  const MultiMode mode = static_cast<MultiMode>(es + 1);
  const QString mul = harmonic ? QString() : multi_name(mode);
  // the das_anz line under the title
  const ClassicSheetText sheet = classic_sheet_text();
  const QString das = tr(" Datensatz : %1 | %2 | %3")
                          .arg(QString::fromStdString(sheet.name).trimmed(),
                               datum3_text(calendar_date(radix_chart().jd_ut, current_settings().calendar)).trimmed(),
                               QString::fromStdString(record_.place).trimmed());
  const QString title = harmonic ? tr(" HARMONICS = GRUNDHOROSKOP * GANZZAHLIGEM FAKTOR !")
                                 : tr(" MULTIPLE DIREKTIONEN nach STEPHAN A. LEHRIEDER ! | %1").arg(mul);
  for (;;) {
    std::optional<bool> hneu;
    QString shown = mul;
    if (harmonic) {
      // h$ = @inputbox$(160,eaz$," ORDNUNGS-ZAHL der HARMONIC !",""), ha = VAL(h$).
      // An order of zero folds every body onto 0 Aries and draws nothing
      // but the radix, the box asks again
      std::optional<double> ha;
      do {
        ha = ask_number(this, tr("ZAHLEN-Eingabe !"), tr(" ORDNUNGS-ZAHL der HARMONIC !"), -1.0e6, 1.0e6,
                        std::numeric_limits<double>::quiet_NaN(), 3, {title, das});
      } while (ha && *ha == 0.0);
      if (!ha) {
        break;
      }
      hneu = ask_house_mode();
      if (!hneu) {
        break;
      }
      harm_n_ = *ha;
      harm_new_mc_ = *hneu;
      shown = QString::number(*ha, 'g', 6) + ".HARMONIC";
      const QSignalBlocker b1(harmonic_action_);
      const QSignalBlocker b2(multi_action_);
      multi_action_->setChecked(false);
      harmonic_action_->setChecked(true);
    } else {
      // MULTINULLWEST asks haus_ber before the date
      if (mode == MultiMode::kZeroWest) {
        hneu = ask_house_mode();
        if (!hneu) {
          break;
        }
      }
      // @a37dat(200,"","Ereignis - DATUM eingeben !")
      const std::optional<CalendarDate> day =
          ask_date(this, tr("Ereignis - DATUM eingeben !"), today_date(), QString(), {title, das});
      if (!day) {
        break;
      }
      if (!hneu) {
        hneu = ask_house_mode();
        if (!hneu) {
          break;
        }
      }
      MultiReference ref;
      if (mode == MultiMode::kMulti3 || mode == MultiMode::kArc) {
        const std::optional<MultiReference> r = ask_multi_reference(mul);
        if (!r) {
          break;
        }
        ref = *r;
      }
      multi_mode_ = mode;
      multi_ref_ = ref;
      multi_new_mc_ = *hneu;
      // ho = ho(1,ze), mi = mi(1,ze), the event day at the birth clock
      const CalendarDate d = *day;
      multi_event_jd_ = event_at_radix_clock(
          radix_chart(), julian_day({d.day, d.month, d.year, 0, 0.0}, current_settings().calendar), false);
      const QSignalBlocker b1(harmonic_action_);
      const QSignalBlocker b2(multi_action_);
      harmonic_action_->setChecked(false);
      multi_action_->setChecked(true);
    }
    leave_views({harmonic_action_, multi_action_});
    claim_wheel();
    recompute();
    setWindowTitle(windowTitle().section("  |", 0, 0) + "  |  " + shown);
    // the chart stands before his questions go on
    wart(menu_item::kMulti, true);
    // zeitwim, harm clears expr! and still asks weit_dat. Every way out
    // of zeitwim passes zwme and his druck_horm
    const bool varied = multi_time_variation(shown);
    const bool printed = multi_print_offer();
    if ((!varied || !printed) && !harmonic) {
      break;
    }
    wart(menu_item::kMulti, true);
    // da$ = TRIM$(na$(1,ze)) + " " + STR$(ta,2) + "." + STR$(mo,2) + "." + STR$(ja,5)
    const CalendarDate birth = calendar_date(radix_chart().jd_ut, current_settings().calendar);
    const QString da = QString::fromStdString(sheet.name).trimmed() +
                       QString::asprintf(" %2d.%2d.%5d", birth.day, birth.month, birth.year);
    const int re = harmonic ? ChoiceDialog::ask(this, tr(" HINWEIS "),
                                                {tr("Weiteres HARMONIC untersuchen ?"), tr("Mit DATENSATZ :"), da},
                                                {tr("WEITERES HARMONIC"), tr("HARMONIC BEENDEN")}, 0)
                            : ChoiceDialog::ask(this, tr(" HINWEIS "),
                                                {tr("Weiteres DATUM untersuchen ?"), tr("Mit DATENSATZ :"), da},
                                                {tr("WEITERES DATUM mit %1").arg(mul), tr("%1  BEENDEN").arg(mul)}, 0);
    if (re != 0) {
      break;
    }
  }
  setWindowTitle(windowTitle().section("  |", 0, 0));
}

// ported from multi11 and its siblings with mult_rad, mult_mult and
// halbsm, harm21 for the harmonics, the full MULTI sheet
DisplayList MainWindow::multi_sheet(const Chart& radix, const Chart& multi, const ChartSettings& s, bool harmonic,
                                    const QString& mul, double lja, double event_jd, DisplayList wheel) const {
  DisplayList& dl = wheel;
  const QString mode = konsta_.appa == 2 ? "A2" : (konsta_.appa == 3 ? "W" : "A1");
  PairColumnOptions col;
  col.header = ((konsta_.voll ? tr("Länge:") : tr("Ekl.Länge:")) + mode).toStdString();
  int extras = 0;
  for (int slot = body::kApogee; slot < body::kSlotCount; ++slot) {
    extras += radix.b[static_cast<std::size_t>(slot)].present ? 1 : 0;
  }
  // vf!, mult! with klpl! and np& > 19
  col.compact = s.extra_bodies && extras > 1;
  col.parallax = s.topocentric_parallax;
  col.true_node = s.true_node;
  col.true_apogee = s.true_apogee;
  col.extras = s.extra_bodies;
  // IF vl! OR mult!, t$ = "Häusersp."
  col.houses_header = tr("Häusersp.").toStdString();
  col.house_name = QString::fromUtf8(radix.houses.name.data(), static_cast<int>(radix.houses.name.size())).trimmed().toStdString();
  // mult_rad, "RADIX :" over the column at xt& = 2
  sheet_text(dl, 6.0, 12.0, tr("RADIX :"), 12.0);
  const double y1 = add_pair_bodies(dl, radix, col, 2.0, 22.0);
  const double ys = add_pair_houses(dl, radix, col, 2.0, y1) + 10.0;
  // mult_mult, mul$ + ":" at xt& = 112, the houses from y1&
  sheet_text(dl, 116.0, 12.0, mul + ":", 12.0);
  PairColumnOptions mcol = col;
  // IF mult4! OR mult5! OR mult6!, only AC and MC
  mcol.angles_only = !harmonic && (multi_mode_ == MultiMode::kZeroEast || multi_mode_ == MultiMode::kZeroWest ||
                                   multi_mode_ == MultiMode::kArc);
  add_pair_bodies(dl, multi, mcol, 112.0, 22.0);
  add_pair_houses(dl, multi, mcol, 112.0, y1);
  // a12asp twice, the directed against the radix and among themselves.
  // His tags said R over the directed body and M over the radix one, the
  // port writes them the right way round
  CrossScanOptions scan;
  scan.orbs = harmonic ? CrossOrbs::kHarmonic : CrossOrbs::kMulti;
  scan.extras = s.extra_bodies;
  const AspectSettings orbs = shown_aspect_settings();
  CrossGridOptions grid;
  grid.multi = true;
  grid.left_tag = harmonic ? " H" : " M";
  grid.right_tag = " R";
  grid.left_tag2.clear();
  grid.right_tag2.clear();
  grid.outer_color = outer_color_;
  grid.more_label = tr(" MEHR ").toStdString();
  add_cross_grid(dl, scan_aspects_between(multi, radix, orbs, scan), grid, 2.0, ys);
  CrossScanOptions within = scan;
  within.within = true;
  CrossGridOptions inner = grid;
  inner.left_tag2 = harmonic ? " H" : " M";
  inner.right_tag2 = harmonic ? " H" : " M";
  add_cross_grid(dl, scan_aspects_between(multi, multi, orbs, within), inner, 112.0, ys);

  // the centre texts, amh& 430 and bmh& 224, size 13
  constexpr double amh = kWheelCenterX;
  constexpr double bmh = kWheelCenterY;
  const QString date = datum3_text(calendar_date(event_jd, s.calendar));
  const QString age = "=" + QString::asprintf("%10.6f", lja) + tr(" LJ");
  if (harmonic) {
    sheet_text(dl, amh - 36.0, bmh - 6.0, mul, 13.0);
    sheet_text(dl, amh - 40.0, bmh + 6.0, tr("INNEN Radix"), 13.0);
  } else if (multi_mode_ == MultiMode::kMulti3 || multi_mode_ == MultiMode::kArc) {
    sheet_text(dl, amh - 28.0, bmh - 13.0, mul, 13.0);
    QString ref;
    double dx = 56.0;
    switch (multi_ref_.kind) {
      case MultiReference::Kind::kBody:
        ref = tr("Auf %1 bezogen")
                  .arg(QString::fromUtf8(body::kName[static_cast<std::size_t>(multi_ref_.body)].data(),
                                         static_cast<int>(body::kName[static_cast<std::size_t>(multi_ref_.body)].size())));
        break;
      case MultiReference::Kind::kCusp:
        ref = tr("Auf Haus %1 bez.").arg(multi_ref_.house, 2);
        dx = 62.0;
        break;
      case MultiReference::Kind::kRuler:
        ref = tr("Auf H.v.H.%1 bez.").arg(multi_ref_.house, 2);
        dx = multi_mode_ == MultiMode::kArc ? 62.0 : 64.0;
        break;
      case MultiReference::Kind::kSignStart:
        ref = tr("Auf 0 Grad %1 bez.").arg(kSignTag[std::clamp(multi_ref_.sign, 1, 12) - 1]);
        dx = multi_mode_ == MultiMode::kArc ? 62.0 : 64.0;
        break;
    }
    sheet_text(dl, amh - dx, bmh, ref, 13.0);
    sheet_text(dl, amh - 40.0, bmh + 13.0, date, 13.0);
    sheet_text(dl, amh - 56.0, bmh + 26.0, age, 13.0);
  } else {
    // multi0ost1 and multi0west1 set their longer mul$ at amh& - 44,
    // multi11 and multi21 at amh& - 28
    const bool zero_point = multi_mode_ == MultiMode::kZeroEast || multi_mode_ == MultiMode::kZeroWest;
    sheet_text(dl, amh - (zero_point ? 44.0 : 28.0), bmh - 6.0, mul, 13.0);
    sheet_text(dl, amh - 40.0, bmh + 6.0, date, 13.0);
    sheet_text(dl, amh - 56.0, bmh + 19.0, age, 13.0);
  }
  // halbsm, the directed pairs on the axes and cusps
  add_multi_midpoints(dl, multi_midpoints(radix, multi, aspect_settings_.orb), konsta_.weiss);
  return wheel;
}

}  // namespace horcom
