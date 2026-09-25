// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// ZEIT-WANDERN, his zeitw and zeitwi. The chart walks by a chosen time
// step and redraws after a wait, keys and mouse steer the walk, the
// yellow counter windows sum the aspects and midpoints of the charts.

#include <QApplication>
#include <QElapsedTimer>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScopeGuard>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <cmath>
#include <limits>
#include <optional>

#include "choice_dialog.hpp"
#include "event_filter.hpp"
#include "horcom/chart/houses.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/place_file.hpp"
#include "main_window.hpp"
#include "robert_input.hpp"
#include "theme.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

// his twart% before the first mouse click, two seconds
constexpr int kWaitStartMs = 2000;
// the left button adds two seconds, the right takes one, never below one
constexpr int kWaitMoreMs = 2000;
constexpr int kWaitLessMs = 1000;
constexpr int kWaitLeastMs = 1000;
// his IF jdd < 1000, the Zeit-Diff box stays away beyond
constexpr double kDiffDaysShown = 1000.0;
// his IF ABS(jd - jd0) > 10 took the walked time back without a question
constexpr double kKeepDays = 10.0;
// the units of the walks in days, TAGE, STUNDEN, MINUTEN and SEKUNDEN
constexpr double kUnitDays[4] = {1.0, 1.0 / kHoursPerDay, 1.0 / kMinutesPerDay, 1.0 / kSecondsPerDay};
// the light of the bottom strip beside his BEEP
constexpr int kFlashMs = 250;

// the stepping screen of zeitwi, zeitwim and ortwi, the wheel with his
// bottom boxes and the counters. It keeps his pacing, twart% after the
// signal, one longer and one shorter mouse click per wait
class WanderView final : public QDialog {
 public:
  explicit WanderView(QWidget* parent) : QDialog(parent) {
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    auto* stage = new QWidget(this);
    auto* grid = new QGridLayout(stage);
    grid->setContentsMargins(0, 0, 0, 0);
    wheel = new WheelWidget(stage);
    grid->addWidget(wheel, 0, 0, 3, 3);
    const QFont mono = theme::mono_font(8);
    const auto box = [&](int r, int c, Qt::Alignment a) {
      auto* l = new QLabel(stage);
      l->setFont(mono);
      // his RGBCOLOR RGB(0,0,0),RGB(255,255,0), the yellow counter windows
      l->setStyleSheet("QLabel { background: #ffff00; color: #000000; border: 2px solid #808080; padding: 2px; }");
      l->hide();
      grid->addWidget(l, r, c, a);
      return l;
    };
    mid_box = box(0, 2, Qt::AlignTop | Qt::AlignRight);
    asp_box = box(2, 0, Qt::AlignBottom | Qt::AlignLeft);
    v->addWidget(stage, 1);
    bar_ = new QFrame(this);
    bar_->setObjectName("walkBar");
    auto* bottom = new QHBoxLayout(bar_);
    bottom->setContentsMargins(8, 2, 8, 4);
    direction = new QLabel(bar_);
    diff = new QLabel(bar_);
    diff->setFont(mono);
    diff->setStyleSheet("QLabel { border: 1px solid; padding: 2px 6px; }");
    bottom->addStretch(1);
    bottom->addWidget(direction);
    bottom->addSpacing(24);
    bottom->addWidget(diff);
    bottom->addStretch(1);
    v->addWidget(bar_);
    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this, [this]() {
      if (on_step) {
        on_step();
      }
    });
  }

  /// His BEEP after a drawn step and the wait of twart% for the next.
  ///
  /// @param signal false waits without the signal
  void arm(bool signal = true) {
    if (signal) {
      QApplication::beep();
      // most Linux desktops stay silent on the beep, the strip lights up
      bar_->setStyleSheet("QFrame#walkBar { background: #ffff00; }");
      QTimer::singleShot(kFlashMs, bar_, [bar = bar_]() { bar->setStyleSheet(QString()); });
    }
    more_used_ = false;
    less_used_ = false;
    waited_.restart();
    timer_.start(wait_ms_);
  }

  /// Stops the wait, a key steps at once or the walk ends.
  void halt() { timer_.stop(); }

  /// His Leertaste. It stops a running walk, any key of a stopped walk
  /// goes on with a step.
  ///
  /// @param key the pressed key
  /// @return true when the key stopped the walk
  bool pause_on(int key) {
    if (!paused_ && key == Qt::Key_Space) {
      timer_.stop();
      paused_ = true;
      return true;
    }
    paused_ = false;
    return false;
  }

  /// @return true while the walk stands after his Leertaste
  [[nodiscard]] bool paused() const { return paused_; }

  WheelWidget* wheel = nullptr;
  QLabel* direction = nullptr;
  QLabel* diff = nullptr;
  QLabel* mid_box = nullptr;
  QLabel* asp_box = nullptr;
  std::function<void(int)> on_key;
  std::function<void()> on_step;

 protected:
  void keyPressEvent(QKeyEvent* e) override {
    if (!on_key) {
      QDialog::keyPressEvent(e);
      return;
    }
    // his INKEY$ returned nothing for a lone modifier key
    switch (e->key()) {
      case Qt::Key_Shift:
      case Qt::Key_Control:
      case Qt::Key_Alt:
      case Qt::Key_AltGr:
      case Qt::Key_Meta:
      case Qt::Key_CapsLock:
      case Qt::Key_NumLock:
      case Qt::Key_ScrollLock:
        return;
      default:
        on_key(e->key());
    }
  }

  // Linke Maustaste = + 2 Sekunden, Rechte Maustaste = - 1 Sekunde, only
  // while the wait runs and each once per wait
  void mousePressEvent(QMouseEvent* e) override {
    if (paused_ || !timer_.isActive()) {
      return;
    }
    if (e->button() == Qt::LeftButton && !more_used_) {
      wait_ms_ += kWaitMoreMs;
      more_used_ = true;
    } else if (e->button() == Qt::RightButton && !less_used_) {
      wait_ms_ = std::max(wait_ms_ - kWaitLessMs, kWaitLeastMs);
      less_used_ = true;
    } else {
      return;
    }
    timer_.start(std::max(0, wait_ms_ - static_cast<int>(waited_.elapsed())));
  }

  // ESC belongs to the walk, it ends it through on_key
  void reject() override {
    if (on_key) {
      on_key(Qt::Key_Escape);
      return;
    }
    QDialog::reject();
  }

 private:
  QFrame* bar_ = nullptr;
  QTimer timer_;
  QElapsedTimer waited_;
  int wait_ms_ = kWaitStartMs;
  bool more_used_ = false;
  bool less_used_ = false;
  bool paused_ = false;
};

// the walk keys of zeitwi and zeitwim, V and R turn the direction, the
// unit letters and + and - set the step, any other key only steps. His
// zeitwim never set m!, so its H, M and S did nothing although the legend
// promised them, the port keeps that promise
void walk_key(int key, bool seconds, int& rf, double& djd) {
  switch (key) {
    case Qt::Key_V:
      rf = 1;
      break;
    case Qt::Key_R:
      rf = -1;
      break;
    case Qt::Key_D:
      if (!seconds) {
        djd = kUnitDays[0];
      }
      break;
    case Qt::Key_H:
      djd = kUnitDays[1];
      break;
    case Qt::Key_M:
      djd = kUnitDays[2];
      break;
    case Qt::Key_S:
      if (seconds) {
        djd = kUnitDays[3];
      }
      break;
    case Qt::Key_Plus:
      djd *= 2.0;
      break;
    case Qt::Key_Minus:
      djd /= 2.0;
      break;
    default:
      break;
  }
}

// the symbolic units of zeitwi under p!, one year of life is one day of
// the progressed sky, a month a twelfth of it and a day one tja-th
double symbolic_unit(int row, double tja) {
  switch (row) {
    case 1:
      return 1.0 / kMonthsPerYear;
    case 2:
      return 1.0 / tja;
    default:
      return 1.0;
  }
}

// the walk keys of zeitwi under p!, J one symbolic year, M a month and D
// a day, V, R and the doubling like walk_key. His t! letters D, H and M
// stay silent there
void symbolic_key(int key, double tja, int& rf, double& djd) {
  switch (key) {
    case Qt::Key_V:
      rf = 1;
      break;
    case Qt::Key_R:
      rf = -1;
      break;
    case Qt::Key_J:
      djd = symbolic_unit(0, tja);
      break;
    case Qt::Key_M:
      djd = symbolic_unit(1, tja);
      break;
    case Qt::Key_D:
      djd = symbolic_unit(2, tja);
      break;
    case Qt::Key_Plus:
      djd *= 2.0;
      break;
    case Qt::Key_Minus:
      djd /= 2.0;
      break;
    default:
      break;
  }
}

// his @stop, any key closes the key sheet
void run_key_sheet(QDialog& sheet) {
  LambdaFilter any(LambdaFilter::keys([&sheet](int) {
    sheet.accept();
    return true;
  }));
  sheet.installEventFilter(&any);
  sheet.exec();
}

}  // namespace

// the walks and sessions step the panel many times, like his merk and
// merkr only the net change of the whole run counts
std::function<void()> MainWindow::hold_history() {
  flush_history();
  const bool held = restoring_;
  restoring_ = true;
  return [this, held]() { restoring_ = held; };
}

// ported from asp_halbs_zaehler, the switch box of the counters. The
// first answer holds for the whole session and for STATISTIK too
int MainWindow::counter_switch(bool statistik, const QString& whole) {
  // his IF halbszeitwaus! = 0, a session switch-off skips the box
  if (counters_off_session_) {
    return 0;
  }
  // his IF voll!, the midpoint counter joins the aspect counter
  const QString head = konsta_.voll ? tr("Die ASPEKTE - und HALBSUMMENZÄHLER EINSCHALTEN ?")
                                    : tr("Den ASPEKTE - ZÄHLER EINSCHALTEN ?");
  QStringList buttons;
  QString line;
  if (statistik) {
    line = tr("Der Zähler SUMMIERT die einzelnen Werte der Datensätze auf !");
    buttons << tr("Zähler DAUERND AUSSCHALTEN für diese HORCOM-Sitzung,auch für ZEITWANDERN !")
            << tr("Zähler EINSCHALTEN für EINZEL-Betrachtung")
            << tr("Zähler VORÜBERGEHEND AUSSCHALTEN ( für diese STATISTIK-Sitzung ! )");
    // his f$, AUSWERTUNG des ZÄHLERS for the whole file or list
    if (!whole.isEmpty()) {
      buttons << whole;
    }
  } else {
    line = tr("Der Zähler SUMMIERT die Werte der einzelnen Zeitpunkte auf !");
    buttons << tr("Zähler DAUERND AUSSCHALTEN für diese HORCOM-Sitzung,auch für STATISTIK !")
            << tr("Zähler EINSCHALTEN mit AUTOMATISCHEM ABLAUF und STOP - Möglichkeit ")
            << tr("Zähler VORÜBERGEHEND AUSSCHALTEN ( für diese ZEITWANDERN-Sitzung ! )");
  }
  const int es = ChoiceDialog::ask(this, tr("AUSWAHL"), {head, line}, buttons, 0);
  if (es < 0) {
    return -1;
  }
  if (es == 0) {
    // his halbszeitwaus! = -1
    counters_off_session_ = true;
  }
  return es + 1;
}

// ported from halbs_zaehl_gr. His POPUP-Graphik stood over the trees and
// had to be clicked away, the shell writes the same counts as a quiet line
// under the HALBSUMMEN-GRAPHIK and nothing pops up on its own
QString MainWindow::midpoint_count_line(const std::array<int, 4>& counts, bool quarter) const {
  const int total = counts[0] + counts[1] + counts[2] + counts[3];
  QStringList parts{tr("Direkt %1").arg(counts[0]), tr("Quadrat %1").arg(counts[1]),
                    tr("Halbquadrat %1").arg(counts[2])};
  // his IF asp2!, the HALBSUMMEN-GRAPHIK adds the fourth level
  if (quarter) {
    parts << tr("Viertelquadrat %1").arg(counts[3]);
  }
  parts << tr("ZUSAMMEN %1").arg(total);
  const AspectSettings& a = aspect_settings_;
  // his IF orbe!, "Grund-ORBIS = " + STR$(up * orbe(14),4,1), else STR$(orb,4,1)
  if (a.equal_probability) {
    parts << tr("Grund-ORBIS = %1°").arg(a.orbe[14] * kRadToDeg, 4, 'f', 1);
  } else {
    parts << tr("Grund-ORBIS%1°").arg(a.orb, 4, 'f', 1);
  }
  parts << tr("ORBIS-FAKTOR = %1%").arg(std::lround(a.orb * kPercent), 3);
  return tr("Anzahl Halbsummen :") + "  " + parts.join("  |  ");
}

// ported from einzel_plan_wahl1, his box before the chart outputs. The
// choice rides on the display of this output only
bool MainWindow::single_planet_choice(int preset, bool midpoints) {
  // his a$ = "AUSGEWÄHLTE Planeten im HOROSKOP NUR ROT MARKIEREN ", c$ is
  // " HALBSUMMEN " for the HALBSUMMEN-GRAPHIK and " ASPEKTE " otherwise
  const QString only = midpoints ? tr(" NUR AUSGEWÄHLTE Planeten und DEREN HALBSUMMEN  im HOROSKOP ANZEIGEN ")
                                 : tr(" NUR AUSGEWÄHLTE Planeten und DEREN ASPEKTE  im HOROSKOP ANZEIGEN ");
  const int ret = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("EINZELNE PlANETEN für das HOROSKOP AUSWÄHLEN ?")},
                                    {tr("AUSGEWÄHLTE Planeten im HOROSKOP NUR ROT MARKIEREN "), only,
                                     tr("NORMALE AUSGABE"), tr("ABBRUCH")},
                                    preset - 1);
  if (ret < 0 || ret == 3) {
    return false;
  }
  if (ret == 2) {
    // his plw&(i&) = i& with CLR plan_col!,plan_wahl!,asp_wahl!
    emphasis_.fill(0);
    recompute();
    return true;
  }
  if (!last_chart_) {
    return true;
  }
  // his @plan_wahl, the bodies the chart carries
  std::vector<int> objects;
  QStringList names;
  for (int slot = body::kSun; slot < body::kSlotCount; ++slot) {
    const BodyState& b = last_chart_->b[static_cast<std::size_t>(slot)];
    if (b.present && b.valid) {
      objects.push_back(slot);
      const std::string_view n = body::kName[static_cast<std::size_t>(slot)];
      names << QString::fromUtf8(n.data(), static_cast<qsizetype>(n.size()));
    }
  }
  const auto chosen = ask_objects(this, objects, names);
  if (!chosen) {
    return false;
  }
  for (const int slot : objects) {
    const bool picked = std::find(chosen->begin(), chosen->end(), slot) != chosen->end();
    auto& e = emphasis_[static_cast<std::size_t>(slot)];
    if (ret == 0) {
      // his plan_col!, the chosen ones in red
      e = picked ? 1 : 0;
    } else {
      // his plan_wahl! and asp_wahl!, only the chosen with their aspects
      e = picked ? 0 : -1;
    }
  }
  recompute();
  return true;
}

// the asp1 counters and the halbs1 midpoints of one chart
void MainWindow::count_chart(WanderCounts& c, const Chart& chart, const AspectResult& a, const ChartSettings& s) const {
  for (std::size_t i = 0; i < c.asp_now.size() && i < a.zh.size(); ++i) {
    c.asp_now[i] = a.zh[i];
    c.asp_sum[i] += a.zh[i];
  }
  c.troika_now = a.triga;
  c.troika_sum += a.triga;
  c.grand_trine_now = a.grand_trines;
  c.grand_trine_sum += a.grand_trines;
  const MidpointResult mid = scan_midpoints(chart, s, shown_aspect_settings());
  c.mid_now = {mid.direct, mid.square, mid.semi, 0};
  for (std::size_t i = 0; i < 4; ++i) {
    c.mid_sum[i] += c.mid_now[i];
  }
  ++c.charts;
}

// the text of his two counter windows, anzeigen_halbs_zaehl and
// anzeigen_asp_zaehl. The divisors 14 and 15 shared his array cells with
// Troika and GrTrig, the port keeps them apart and sums every divisor
void MainWindow::counter_texts(const WanderCounts& c, QString& midpoints, QString& aspects, bool records) const {
  const long n = c.charts;
  const auto mean = [n](long sum) { return n > 0 ? QString::asprintf("%6.2f", static_cast<double>(sum) / n) : QString(6, ' '); };
  const long cur_tot = c.mid_now[0] + c.mid_now[1] + c.mid_now[2] + c.mid_now[3];
  const long sum_tot = c.mid_sum[0] + c.mid_sum[1] + c.mid_sum[2] + c.mid_sum[3];
  QStringList m;
  // his " HALBSUMMEN - ZÄHLER : " + STR$(halbszaus%,4) + v$ with
  // v$ = " VORGÄNGE     " under muuu& 83 and " DATENSÄTZE   " under 40
  m << (records ? tr(" HALBSUMMEN - ZÄHLER : %1 DATENSÄTZE   ") : tr(" HALBSUMMEN - ZÄHLER : %1 VORGÄNGE     ")).arg(n, 4)
    << tr("              EINZELN  SUMME    MITTEL   ");
  const char* rows[3] = {QT_TRANSLATE_NOOP("horcom::MainWindow", " Direkt     : "),
                         QT_TRANSLATE_NOOP("horcom::MainWindow", " Quadrat    : "),
                         QT_TRANSLATE_NOOP("horcom::MainWindow", " Halbquadrat: ")};
  for (int i = 0; i < 3; ++i) {
    m << tr(rows[i]) + QString::asprintf("%3ld    %5ld     ", c.mid_now[static_cast<std::size_t>(i)],
                                         c.mid_sum[static_cast<std::size_t>(i)]) +
             mean(c.mid_sum[static_cast<std::size_t>(i)]) + "    ";
  }
  m << tr(" ZUSAMMEN   : ") + QString::asprintf("%3ld    %5ld     ", cur_tot, sum_tot) + mean(sum_tot) + "    ";
  const AspectSettings& a = aspect_settings_;
  // his IF orbe!, Grund-ORBIS = orbe(14), else the orb factor
  const double base = a.equal_probability ? a.orbe[14] * kRadToDeg : a.orb;
  m << tr(" Grund-ORBIS=%1° | ORBIS-FAKTOR =%2%  ").arg(base, 4, 'f', 1).arg(std::lround(a.orb * kPercent), 3);
  midpoints = m.join(QChar(0x0A));

  long most = c.troika_sum + c.grand_trine_sum;
  for (const long s : c.asp_sum) {
    most += s;
  }
  QStringList s;
  s << tr("  ASPEKTE - ZÄHLER   ")
    << (records ? tr(" %1 DATENSÄTZE      ") : tr("   %1 VORGÄNGE      ")).arg(n, 4) << tr("      Anz. Sum.  %   ");
  // his row names Kon Opp Tri Qua Qui Sex Sep Okt Non Dez Und Dod
  static constexpr const char* kShort[12] = {
      QT_TRANSLATE_NOOP("horcom::MainWindow", "Kon"), QT_TRANSLATE_NOOP("horcom::MainWindow", "Opp"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "Tri"), QT_TRANSLATE_NOOP("horcom::MainWindow", "Qua"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "Qui"), QT_TRANSLATE_NOOP("horcom::MainWindow", "Sex"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "Sep"), QT_TRANSLATE_NOOP("horcom::MainWindow", "Okt"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "Non"), QT_TRANSLATE_NOOP("horcom::MainWindow", "Dez"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "Und"), QT_TRANSLATE_NOOP("horcom::MainWindow", "Dod")};
  const auto pct = [most](long v) { return QString::asprintf("%5.2f", 100.0 * static_cast<double>(v) / most); };
  for (int i = 0; i < 12; ++i) {
    s << (most > 0 ? QString(" %1: %2 %3 %4 ")
                         .arg(tr(kShort[i]))
                         .arg(c.asp_now[static_cast<std::size_t>(i + 1)], 2)
                         .arg(c.asp_sum[static_cast<std::size_t>(i + 1)], 5)
                         .arg(pct(c.asp_sum[static_cast<std::size_t>(i + 1)]))
                   : QString(" "));
  }
  s << (most > 0 ? QStringLiteral("-------------------- ") : QStringLiteral(" "));
  s << (most > 0 ? tr(" Troika:%1%2 %3 ").arg(c.troika_now, 2).arg(c.troika_sum, 4).arg(pct(c.troika_sum)) : QString(" "));
  s << (most > 0 ? tr(" GrTrig:%1%2 %3 ").arg(c.grand_trine_now, 2).arg(c.grand_trine_sum, 4).arg(pct(c.grand_trine_sum))
                 : QString(" "));
  s << (most > 0 ? QStringLiteral("-------------------- ") : QStringLiteral(" "));
  s << tr(" ORBIS-FAKTOR =%1%  ").arg(std::lround(a.orb * kPercent), 3);
  aspects = s.join(QChar(0x0A));
}

// ported from zeitw and zeitwi with eingalp, ran and halbs_ruecksetz. His
// zeitwi advanced the moment twice per frame, once at zw1 and once more
// before the redraw, so every screen stood two steps on. The port walks
// the step it names
void MainWindow::time_wander() {
  if (!last_chart_ || active_is_solar_) {
    return;
  }
  // his @merk, the start the walk may return to
  const double jd0 = last_chart_->jd_ut;
  const QString label = record_label_.trimmed();
  const std::array<int, body::kSlotCount> emphasis_before = emphasis_;
  // his ARRAYFILL halbsz%(),0 and aspz%(),0, the start chart is not counted
  WanderCounts counts;
  const int counting = counter_switch(false);
  if (counting < 0) {
    return;
  }
  // the answers one to three lead into einzel_plan_wahl1 with NORMALE
  // AUSGABE preset
  if (counting >= 1 && counting <= 3 && !single_planet_choice(3)) {
    emphasis_ = emphasis_before;
    recompute();
    return;
  }
  // his zaehl_asp_halbs& = 2, the counters count and show only then
  const bool show_counters = counting == 2;
  double jd = jd0;
  const double jds = jd0;
  {
    // the steps stay out of the Zurück list and the slot until the end
    const auto hold = qScopeGuard(hold_history());
    double djd = kUnitDays[0];
    int rf = 1;
    bool ask_unit = true;
    for (;;) {
      if (ask_unit) {
        const QStringList units{tr("TAGE"), tr("STUNDEN"), tr("MINUTEN")};
        const int rd = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("ZEIT-EINHEIT ?")}, units, 0);
        if (rd < 0) {
          break;
        }
        // his t$ = " als BELIEBIGE ZAHL EINGEBEN !"
        const std::optional<double> value =
            ask_number(this, tr("ZAHLEN-Eingabe !"), units[rd] + tr(" als BELIEBIGE ZAHL EINGEBEN !"), -1.0e9, 1.0e9,
                       std::numeric_limits<double>::quiet_NaN(), 6);
        if (!value) {
          break;
        }
        djd = *value * kUnitDays[rd];
        const int re = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("RICHTUNG ?")}, {tr("VOR"), tr("ZURÜCK")}, 0);
        if (re < 0) {
          break;
        }
        rf = re == 0 ? 1 : -1;
        wander_help_sheet(kWaitStartMs);
      }

      WanderView view(this);
      view.setWindowTitle(tr("ZEIT-WANDERN") + " | " + label);
      bool ended = false;
      const auto show_boxes = [&]() {
        QString mid_text;
        QString asp_text;
        counter_texts(counts, mid_text, asp_text);
        view.mid_box->setText(mid_text);
        view.asp_box->setText(asp_text);
        view.mid_box->setVisible(show_counters && konsta_.voll);
        view.asp_box->setVisible(show_counters);
      };
      view.on_step = [&]() {
        // his dj = rf& * djd with jd = jd + dj
        jd += rf * djd;
        apply_moment(jd, label, false, true);
        view.wheel->show_like(*wheel_);
        // his ran, Vorwärts ! or Rückwärts!
        view.direction->setText(rf > 0 ? tr("Vorwärts !") : tr("Rückwärts!"));
        // his v$ + "( " + D + " D " + H + " H " + M + " M )"
        const long total = std::lround(std::abs(jd - jds) * kMinutesPerDay);
        const long days = total / static_cast<long>(kMinutesPerDay);
        const QString sign = jd > jds ? "+" : (jd < jds ? "-" : " ");
        view.diff->setVisible(static_cast<double>(days) < kDiffDaysShown);
        view.diff->setText(tr("Zeit-Diff: D H M") + QChar(0x0A) +
                           QString("%1( %2 D %3 H %4 M )")
                               .arg(sign)
                               .arg(days, 3)
                               .arg((total / 60) % 24, 2)
                               .arg(total % 60, 2));
        // the counts of this chart join the running sums
        if (show_counters && last_chart_ && last_aspects_) {
          count_chart(counts, *last_chart_, *last_aspects_, current_settings());
        }
        show_boxes();
        view.arm();
      };
      view.on_key = [&](int key) {
        if (key == Qt::Key_Escape) {
          // ESC shows the counters once more without an increment
          view.halt();
          ended = true;
          show_boxes();
          view.done(QDialog::Accepted);
          return;
        }
        // his IF i$ = " ", STOP until the next key
        if (view.pause_on(key)) {
          return;
        }
        walk_key(key, false, rf, djd);
        view.halt();
        view.on_step();
      };
      QTimer::singleShot(0, &view, view.on_step);
      view.resize(size());
      view.exec();
      view.halt();
      if (!ended) {
        break;
      }
      // his wr$ + " Mit NEUER ZEITEINHEIT ?" with " NEIN = ENDE ", wr$ and ir$
      const int er = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("Weiter Mit NEUER ZEITEINHEIT ?")},
                                       {tr(" NEIN = ENDE "), tr("Weiter"), tr("Irrtum ( = UNDO = Zurück )")}, 0);
      if (er == 1 || er == 2) {
        // his halbs_ruecksetz asks only while zaehl_asp_halbs& = 2
        if (show_counters) {
          reset_counters_question(counts);
        }
        ask_unit = er == 1;
        continue;
      }
      break;
    }
  }
  // his IF ABS(jd - jd0) > 10 with @merkr(1), otherwise the question
  bool keep = false;
  if (std::abs(jd - jd0) <= kKeepDays && jd != jd0) {
    keep = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("VARIIERTE ZEIT in RADIX ÜBERNEHMEN ?")},
                             {tr(" NEIN "), tr("JA")}, 0) == 1;
  }
  emphasis_ = emphasis_before;
  if (keep) {
    if (active_slot_ >= 0) {
      set_slot(active_slot_, panel_record(), true);
    } else {
      recompute();
    }
  } else {
    apply_moment(jd0, label, false, true);
  }
}

// ported from zeitwi under drgrph!, the walk of the HOROSKOP-GRAPHIK of
// a20_horg. Only the running ring moves, one step per screen, his trprso!
// leaves out the counters and the VARIIERTE ZEIT question
void MainWindow::ring_walk(const QString& title, bool progressive, double tja, double& jd,
                           const std::function<void(double)>& show) {
  double djd = 1.0;
  int rf = 1;
  bool ask_unit = true;
  for (;;) {
    if (ask_unit) {
      // his n1$ to n3$, the SYMBOLISCHE units under p!
      const QStringList units = progressive ? QStringList{tr("SYMBOLISCHE 'JAHRE' "), tr("SYMBOLISCHE 'MONATE'"),
                                                          tr("SYMBOLISCHE 'TAGE'  ")}
                                            : QStringList{tr("TAGE"), tr("STUNDEN"), tr("MINUTEN")};
      const int rd = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("ZEIT-EINHEIT ?")}, units, 0);
      if (rd < 0) {
        return;
      }
      // his t$ = " als BELIEBIGE ZAHL EINGEBEN !"
      const std::optional<double> value =
          ask_number(this, tr("ZAHLEN-Eingabe !"), units[rd] + tr(" als BELIEBIGE ZAHL EINGEBEN !"), -1.0e9, 1.0e9,
                     std::numeric_limits<double>::quiet_NaN(), 6);
      if (!value) {
        return;
      }
      djd = *value * (progressive ? symbolic_unit(rd, tja) : kUnitDays[rd]);
      const int re = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("RICHTUNG ?")}, {tr("VOR"), tr("ZURÜCK")}, 0);
      if (re < 0) {
        return;
      }
      rf = re == 0 ? 1 : -1;
      wander_help_sheet(kWaitStartMs, false, progressive);
    }

    WanderView view(this);
    view.setWindowTitle(title);
    // the Zeit-Diff box belongs to zeitw! alone
    view.diff->hide();
    bool ended = false;
    view.on_step = [&]() {
      // zw1, jd = jd + dj with dj = rf& * djd, once per screen
      jd += rf * djd;
      show(jd);
      view.wheel->show_like(*wheel_);
      // his ran, Vorwärts ! or Rückwärts!
      view.direction->setText(rf > 0 ? tr("Vorwärts !") : tr("Rückwärts!"));
      view.arm();
    };
    view.on_key = [&](int key) {
      if (key == Qt::Key_Escape) {
        view.halt();
        ended = true;
        view.done(QDialog::Accepted);
        return;
      }
      // his IF i$ = " ", STOP until the next key
      if (view.pause_on(key)) {
        return;
      }
      if (progressive) {
        symbolic_key(key, tja, rf, djd);
      } else {
        walk_key(key, false, rf, djd);
      }
      view.halt();
      view.on_step();
    };
    QTimer::singleShot(0, &view, view.on_step);
    view.resize(size());
    view.exec();
    view.halt();
    if (!ended) {
      return;
    }
    // his wr$ + " Mit NEUER ZEITEINHEIT ?" with " NEIN = ENDE ", wr$ and ir$
    const int er = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("Weiter Mit NEUER ZEITEINHEIT ?")},
                                     {tr(" NEIN = ENDE "), tr("Weiter"), tr("Irrtum ( = UNDO = Zurück )")}, 0);
    if (er != 1 && er != 2) {
      return;
    }
    // wr$ asks a new unit at zw0, ir$ walks on at zw1
    ask_unit = er == 1;
  }
}

// ported from zeitwim, the birth time walks in steps while the MULTI or
// HARMONIC sheet follows, the event date moves along so the age stays
bool MainWindow::multi_time_variation(const QString& mul) {
  const int rb = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("GEBURTSZEIT VARIIEREN ?"), tr("= ZEITWANDERN mit RADIX")},
                                   {tr(" NEIN "), tr("JA"), tr("ABBRUCH")}, 0);
  if (rb == 0) {
    return true;
  }
  if (rb != 1 || !last_chart_) {
    return false;
  }
  const double jd0 = last_chart_->jd_ut;
  const double event0 = multi_event_jd_;
  const QString label = record_label_.trimmed();
  double jd = jd0;
  {
    // the steps stay out of the Zurück list and the slot until the end
    const auto hold = qScopeGuard(hold_history());
    for (;;) {
      // his n1$ = "STUNDEN", n2$ = "MINUTEN", n3$ = "SEKUNDEN", MINUTEN preset
      const QStringList units{tr("STUNDEN"), tr("MINUTEN"), tr("SEKUNDEN")};
      const int rd = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("ZEIT-EINHEIT ?")}, units, 1);
      if (rd < 0) {
        break;
      }
      const std::optional<double> value =
          ask_number(this, tr("ZAHLEN-Eingabe !"), units[rd] + tr(" als BELIEBIGE ZAHL EINGEBEN !"), -1.0e9, 1.0e9,
                     std::numeric_limits<double>::quiet_NaN(), 6);
      if (!value) {
        break;
      }
      double djd = *value * kUnitDays[rd + 1];
      const int re = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("RICHTUNG ?")}, {tr("VOR"), tr("ZURÜCK")}, 0);
      if (re < 0) {
        break;
      }
      int rf = re == 0 ? 1 : -1;
      wander_help_sheet(kWaitStartMs, true);

      WanderView view(this);
      view.setWindowTitle(mul + " | " + label);
      view.direction->hide();
      view.diff->hide();
      bool ended = false;
      view.on_step = [&]() {
        // his dj = rf& * djd, jd = jd(1,ze) + dj, ADD jdd,dj
        const double dj = rf * djd;
        jd += dj;
        multi_event_jd_ += dj;
        apply_moment(jd, label, false, true);
        view.wheel->show_like(*wheel_);
        view.arm();
      };
      view.on_key = [&](int key) {
        if (key == Qt::Key_Escape) {
          view.halt();
          ended = true;
          view.done(QDialog::Accepted);
          return;
        }
        if (view.pause_on(key)) {
          return;
        }
        walk_key(key, true, rf, djd);
        view.halt();
        view.on_step();
      };
      view.wheel->show_like(*wheel_);
      view.arm(false);
      view.resize(size());
      view.exec();
      view.halt();
      if (!ended) {
        break;
      }
      // his wr$ + " Mit NEUER ZEITEINHEIT ?" with " NEIN = ENDE " and wr$
      const int er = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("Weiter Mit NEUER ZEITEINHEIT ?")},
                                       {tr(" NEIN = ENDE "), tr("Weiter")}, 0);
      if (er == 1) {
        continue;
      }
      break;
    }
  }
  // his NEIN to VARIIERTE ZEIT in RADIX ÜBERNEHMEN ? brings merkr(1)
  bool keep = false;
  if (jd != jd0) {
    keep = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("VARIIERTE ZEIT in RADIX ÜBERNEHMEN ?")},
                             {tr(" NEIN "), tr("JA")}, 0) == 1;
  }
  if (keep) {
    if (active_slot_ >= 0 && !active_is_solar_) {
      set_slot(active_slot_, panel_record(), true);
    } else {
      recompute();
    }
  } else if (jd != jd0) {
    multi_event_jd_ = event0;
    apply_moment(jd0, label, false, true);
  }
  return true;
}

// ported from eingalp, the sheet of the keys before the walk starts
void MainWindow::wander_help_sheet(int wait_ms, bool seconds, bool progressive) {
  QDialog sheet(this);
  sheet.setWindowTitle(tr("ZEIT-WANDERN"));
  auto* v = new QVBoxLayout(&sheet);
  v->setContentsMargins(24, 18, 24, 18);
  v->setSpacing(10);
  const auto line = [&](const QString& t) {
    auto* l = new QLabel(t, &sheet);
    l->setAlignment(Qt::AlignHCenter);
    v->addWidget(l);
  };
  // his iv$ = " < " + STR$(twart% / 1000,2) + " Sek nach Signalton ?"
  line(tr(" Ablauf STEUERBAR mit TASTATUR bzw. MAUS ") + tr(" < %1 Sek nach Signalton ?").arg(wait_ms / 1000, 2));
  line(tr(" Stop / Go :  Leertaste ! "));
  // the zeitwim sheet names hours, minutes and seconds instead, p! the
  // symbolic units of the progressed rings
  if (progressive) {
    const QString iv = tr("Zeitmaß Progressiv ( 1 TAG <> 1 JAHR ):");
    line(iv + tr("   J  =  1 Jahr   ( 1 Tag )           "));
    line(iv + tr("   M  =  1 Monat  ( 2 Stunden )       "));
    line(iv + tr("   D  =  1 Tag    ( 3 Min. 56.6 SEK.)"));
  } else if (seconds) {
    line(tr(" Zeitmaß : ") + tr("   H  =  1 Stunde "));
    line(tr(" Zeitmaß : ") + tr("   M  =  1 Minute "));
    line(tr(" Zeitmaß : ") + tr("   S  =  1 Sekunde "));
  } else {
    line(tr(" Zeitmaß : ") + tr("   D  =  1 Tag    "));
    line(tr(" Zeitmaß : ") + tr("   H  =  1 Stunde "));
    line(tr(" Zeitmaß : ") + tr("   M  =  1 Minute "));
  }
  line(tr(" Richtung :") + tr("   V  =  Vorwärts  "));
  line(tr(" Richtung :") + tr("   R  =  Rückwärts "));
  line(tr(" Intervall:") + tr("   +  =  Verdoppelung "));
  line(tr(" Intervall:") + tr("   -  =  Halbierung   "));
  line(tr(" Wartezeit :") + tr(" Linke  Maustaste = + 2 Sekunden "));
  line(tr(" Wartezeit :") + tr(" Rechte Maustaste = - 1 Sekunde  "));
  line(tr(" Weiter mit Leertaste "));
  run_key_sheet(sheet);
}

// ported from halbs_ruecksetz
void MainWindow::reset_counters_question(WanderCounts& counts) {
  if (counts.charts == 0 || counters_off_session_) {
    return;
  }
  // his "Die ZÄHLER RÜCKSETZEN ?" with JA and " NEIN ", NEIN preset
  if (ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("Die ZÄHLER RÜCKSETZEN ?")}, {tr("JA"), tr(" NEIN ")}, 1) == 0) {
    counts = WanderCounts{};
    QMessageBox::information(this, tr(" Information "), tr("ZÄHLER auf NULL !"));
  }
}

// the nearest place of his WCAPITAL.INT, auswert_ortelist. His search took
// the cosine of the walked latitude and printed with the capital's, did
// not fold the longitude across the date line and fell through to the
// last record when ESC broke the first pass, the port keeps one pass
std::optional<MainWindow::NearestCapital> MainWindow::nearest_capital(double lon, double lat) const {
  const auto list = read_place_file(data_dir_ / "spez_ort" / "wcapital.int");
  if (!list || list->empty()) {
    return std::nullopt;
  }
  // his ra = 6371.229315
  //RR km =mittl. Erdradius
  constexpr double kEarthRadiusKm = 6371.229315;
  const double c = std::cos(lat * kDegToRad);
  NearestCapital best;
  double best_km = std::numeric_limits<double>::max();
  for (const PlaceRecord& p : *list) {
    const double east_km = kEarthRadiusKm * fold_rad((lon - p.lon) * kDegToRad) * c;
    const double north_km = kEarthRadiusKm * (lat - p.lat) * kDegToRad;
    const double km = std::hypot(east_km, north_km);
    if (km < best_km) {
      best_km = km;
      best.lon = p.lon;
      best.lat = p.lat;
      best.name = QString::fromStdString(p.name).trimmed();
      best.east_km = east_km;
      best.north_km = north_km;
    }
  }
  return best;
}

// ported from ortwandern and ortwi with ortgalp, ort_koord_diff and
// auswert_ortelist. The moment stays and only the place walks, his
// counters were switched off here
void MainWindow::place_wander() {
  if (!last_chart_) {
    return;
  }
  ChartSettings s = current_settings();
  // his hrg! = 0
  s.heliocentric = false;
  const ChartInput start = current_input();
  const double gls = start.lon_deg_east;
  const double ggs = start.lat_deg;
  const double ekls = last_chart_->smo.ekls;
  double gl = gls;
  double gg = ggs;
  AafRecord shown = panel_record();
  double dln = 0.0;
  double dbr = 0.0;
  int rl = 0;
  int rb = 0;
  bool setup = true;
  bool aborted = false;
  const QString label = active_is_solar_ && active_solar_ >= 0 ? solar_labels_[static_cast<std::size_t>(active_solar_)]
                                                               : record_label_.trimmed();
  const auto ask_shift = [this](const QString& what, const QString& plus, const QString& minus, double& step,
                                int& dir) -> int {
    // his t$ + "als BELIEBIGE ZAHL EINGEBEN ! " over inputbox$(160,eaz$,n1$ + t$,"")
    const std::optional<double> v =
        ask_number(this, tr("ZAHLEN-Eingabe !"), tr("GRAD") + what, -kDegPerCircle, kDegPerCircle,
                   std::numeric_limits<double>::quiet_NaN(), 6, {what + tr("als BELIEBIGE ZAHL EINGEBEN ! ")});
    if (!v) {
      return -1;
    }
    step = *v;
    if (step == 0.0) {
      return 0;
    }
    // his "RICHTUNG der" + t$ + " ?" with ir$ and ac$
    const int re = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("RICHTUNG der%1 ?").arg(what)},
                                     {plus, minus, tr("Irrtum ( = UNDO = Zurück )"), tr("ABBRUCH")}, 0);
    if (re == 2) {
      return 2;
    }
    if (re < 0 || re == 3) {
      return -1;
    }
    dir = re == 0 ? 1 : -1;
    return 0;
  };
  for (;;) {
    if (setup) {
      // his ortw0 with CLR dln,dbr,rl&
      rl = 0;
      const int a = ask_shift(tr(" LÄNGEN-VERSCHIEBUNG "), tr("ÖSTLICH"), tr("WESTLICH"), dln, rl);
      if (a == 2) {
        continue;
      }
      if (a < 0) {
        aborted = true;
        break;
      }
      const int b = ask_shift(tr(" BREITEN-VERSCHIEBUNG "), tr("NÖRDLICH"), tr("SÜDLICH"), dbr, rb);
      if (b == 2) {
        continue;
      }
      if (b < 0) {
        aborted = true;
        break;
      }
      place_help_sheet(kWaitStartMs);
      setup = false;
    }

    WanderView view(this);
    view.setWindowTitle(tr("ORT-WANDERN") + " | " + label);
    view.direction->hide();
    bool ended = false;
    bool invalid = false;
    std::optional<NearestCapital> capital;
    // his ortw1, the chart at the walked place and the difference box
    const auto draw = [&]() {
      ChartInput in = start;
      in.lon_deg_east = gl;
      in.lat_deg = gg;
      if (const auto dl = sheet_wheel(shown, in, s)) {
        view.wheel->set_display_list(*dl);
      }
      // his "LÄNGEN-Differenz" + STR$(gl - gls,9,3) + "°"
      view.diff->setText(tr("LÄNGEN-Differenz") + QChar(0x0A) + QString::asprintf("%9.3f", gl - gls) + QChar(0xB0) +
                         QChar(0x0A) + tr("BREITEN-Differenz") + QChar(0x0A) + QString::asprintf("%9.3f", gg - ggs) +
                         QChar(0xB0));
      // his checks after the drawing, the offending shift steps back
      QString problem;
      if (s.houses == HouseSystem::kPlacidus || s.houses == HouseSystem::kKochGoh) {
        const QString haus = s.houses == HouseSystem::kPlacidus ? QStringLiteral("Placidus") : QStringLiteral("Koch-GOH");
        // his PI / 2 - pu * gg < ekls, the maxbreit limit of the cusps
        if (gg > polar_limit_deg(ekls)) {
          problem = tr("%1 UNGÜLTIG !").arg(haus);
          gg -= dbr;
        } else if (gg < -polar_limit_deg(ekls)) {
          problem = tr("%1 UNGÜLTIG !").arg(haus);
          gg += dbr;
        }
      } else if (gg > kDegPerQuadrant) {
        problem = tr("Geogr. Breite > 90° Nord !");
        gg -= dbr;
      } else if (gg < -kDegPerQuadrant) {
        problem = tr("Geogr. Breite > 90° Süd !");
        gg += dbr;
      }
      if (problem.isEmpty()) {
        if (gl > kDegPerCircle / 2.0) {
          gl -= dln;
          problem = tr("Geogr. Länge > 180° Ost !");
        } else if (gl < -kDegPerCircle / 2.0) {
          gl += dln;
          problem = tr("Geogr. Länge > 180° West !");
        }
      }
      if (!problem.isEmpty()) {
        view.halt();
        invalid = true;
        QMessageBox::information(&view, tr(" Information "), problem);
        view.done(QDialog::Rejected);
        return;
      }
      view.arm();
    };
    view.on_step = [&]() {
      // his @ort_koord_diff with gl = gl + rl& * dln and gg = gg + rb& * dbr
      gl += rl * dln;
      gg += rb * dbr;
      draw();
    };
    view.on_key = [&](int key) {
      if (capital) {
        // his BYTE(kg%) = 13, EINRASTEN at the capital
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
          gl = capital->lon;
          gg = capital->lat;
          shown.place = capital->name.toStdString();
          QMessageBox::information(&view, tr(" Information "),
                                   tr("Im Folgenden weiter ab den Koordinaten von %1 !").arg(capital->name));
        } else if (key == Qt::Key_Space) {
          capital.reset();
          view.mid_box->hide();
          draw();
        }
        return;
      }
      if (key == Qt::Key_Escape) {
        view.halt();
        ended = true;
        view.done(QDialog::Accepted);
        return;
      }
      // his MENU(12) = VK_F10 inside the wait, the nearest metropolis. The
      // pause loop saw F10 like any other key, it goes on with a step
      if (key == Qt::Key_F10 && !view.paused()) {
        view.halt();
        capital = nearest_capital(gl, gg);
        if (!capital) {
          draw();
          return;
        }
        // his " Ca." + STR$(ABS(dl),4,0) + " km westlich  " or " km östlich   "
        const QString ew = capital->east_km < 0.0 ? tr(" km westlich  ") : tr(" km östlich   ");
        const QString ns = capital->north_km < 0.0 ? tr(" km südlich   ") : tr(" km nördlich  ");
        view.mid_box->setText(QString(" Ca.%1").arg(std::lround(std::abs(capital->east_km)), 4) + ew + QChar(0x0A) +
                              QString(" Ca.%1").arg(std::lround(std::abs(capital->north_km)), 4) + ns + QChar(0x0A) +
                              " " + capital->name + " ");
        view.mid_box->setStyleSheet("QLabel { background: #c0c0c0; color: #000000; border: 2px solid #808080; padding: 2px; }");
        view.mid_box->show();
        QApplication::beep();
        return;
      }
      if (view.pause_on(key)) {
        return;
      }
      switch (key) {
        // + doubles and - halves both shifts
        case Qt::Key_Plus:
          dln *= 2.0;
          dbr *= 2.0;
          break;
        case Qt::Key_Minus:
          dln /= 2.0;
          dbr /= 2.0;
          break;
        // L and B turn the direction of the longitude or the latitude
        case Qt::Key_L:
          rl = -rl;
          break;
        case Qt::Key_B:
          rb = -rb;
          break;
        default:
          break;
      }
      view.halt();
      view.on_step();
    };
    QTimer::singleShot(0, &view, draw);
    view.resize(size());
    view.exec();
    view.halt();
    if (invalid) {
      setup = true;
      continue;
    }
    if (!ended) {
      break;
    }
    // his wr$ + " Mit NEUEN Einstellungen ?" with " NEIN = ENDE ", wr$ and ir$
    const int er = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("Weiter Mit NEUEN Einstellungen ?")},
                                     {tr(" NEIN = ENDE "), tr("Weiter"), tr("Irrtum ( = UNDO = Zurück )")}, 0);
    if (er == 1) {
      setup = true;
      continue;
    }
    if (er == 2) {
      continue;
    }
    break;
  }
  if (aborted || (gl == gls && gg == ggs)) {
    return;
  }
  // his "Geänderte Koordinaten für " + sol$(od,ze) + " BEIBEHALTEN ?" with NEIN and JA
  const QString sol = active_is_solar_ && active_solar_ >= 0 ? solar_labels_[static_cast<std::size_t>(active_solar_)]
                                                             : QStringLiteral("RADIX");
  if (ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("Geänderte Koordinaten für %1 BEIBEHALTEN ?").arg(sol)},
                        {tr("NEIN"), tr("JA")}, 0) != 1) {
    return;
  }
  // his go$(od,ze) = "GEÄNDERT", the moment stays in UT
  set_panel_place(EventPlace{gl, gg, "GEÄNDERT"});
  apply_moment(julian_day(start.date_ut, s.calendar), label, false, true);
}

// ported from ortgalp
void MainWindow::place_help_sheet(int wait_ms) {
  QDialog sheet(this);
  sheet.setWindowTitle(tr("ORT-WANDERN"));
  auto* v = new QVBoxLayout(&sheet);
  v->setContentsMargins(24, 18, 24, 18);
  v->setSpacing(10);
  const auto line = [&](const QString& t) {
    auto* l = new QLabel(t, &sheet);
    l->setAlignment(Qt::AlignHCenter);
    v->addWidget(l);
  };
  line(tr(" Ablauf STEUERBAR mit TASTATUR bzw. MAUS ") + tr(" < %1 Sek nach Signalton ?").arg(wait_ms / 1000, 2));
  line(tr(" Stop / Go :  Leertaste ! "));
  line(tr(" Verschiebungen :") + tr("   +  =  Verdoppelung "));
  line(tr(" Verschiebungen :") + tr("   -  =  Halbierung   "));
  line(tr(" Richtungs-Änderung :") + tr("   L  =  Länge  "));
  line(tr(" Richtungs-Änderung :") + tr("   B  =  Breite "));
  line(tr(" Wartezeit nach Signalton :") + tr(" Linke  Maustaste = + 2 Sekunden "));
  line(tr(" Wartezeit nach Signalton :") + tr(" Rechte Maustaste = - 1 Sekunde  "));
  line(tr(" Nächstliegende Metropole anzeigen : ") + tr(" Mit Funktionstaste F10 "));
  line(tr(" Evtl. dann dort EINRASTEN mit der 'ENTER' - Taste "));
  line(tr(" Weiter mit Leertaste "));
  run_key_sheet(sheet);
}

}  // namespace horcom
