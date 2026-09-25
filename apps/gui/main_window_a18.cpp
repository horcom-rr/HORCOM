// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// The a18 table flows of AUSWERTUNG, TRANSITE through a20 and a18asw and
// MUNDAN-ASPEKTE through mund, with the a18eing boxes both ask first.

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDate>
#include <QDateTime>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPrinter>
#include <QProgressBar>
#include <QPushButton>
#include <QScopeGuard>
#include <QTime>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

#include "a18_rows.hpp"
#include "banner.hpp"
#include "choice_dialog.hpp"
#include "event_filter.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/progressions.hpp"
#include "horcom/chart/symbolic.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/linear.hpp"
#include "horcom/render/pair_sheet.hpp"
#include "main_window.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"
#include "transit_list_dialog.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

// the ENDE-DATUM box opens a quarter year after the start
constexpr int kDefaultWindowMonths = 3;
// lje = MIN(180,lje), IF lja > 144 Sind das wirklich LEBENSJAHRE ?
constexpr double kLastLifeYear = 180.0;
constexpr double kCheckLifeYear = 144.0;
// the ENDE box opens ninety years after the start
constexpr double kDefaultLifeSpan = 90.0;
// SONSTIGE TEILER, the smallest base his a18ei1 took is 360 / 24
constexpr int kLargestDivisor = 24;
constexpr int kDefaultDivisor = 7;
// his factor box, row 3 EINZELNE ... ROT MARKIEREN, row 4 NUR DIESE
constexpr int kMarkRow = 3;
constexpr int kChooseRow = 4;
// the sweep window shows only for runs longer than this
constexpr int kSweepShowMs = 400;
// and hands the events on at most this often
constexpr int kSweepPollMs = 40;
// the progress bar counts in thousandths
constexpr int kSweepSteps = 1000;
// his lin! windows of one, four and sixteen months in days
constexpr double kWindowDays[3] = {31.0, 125.0, 488.0};
// the rows of prog_mode a20_horg leaves open, HÄUSER-DREHUNG and the
// STRENG PROPORTIONALE UMRECHNUNG
constexpr int kHouseTurnRow = 2;
constexpr int kProportionalRow = 3;
// the size of his textc lines in the centre of the zeitwi rings
constexpr double kZeitwiText = 10.0;

// the running factors of his plan_wahl list, the ltz$ captions
struct RunningObject {
  int slot;
  const char* caption;
};

constexpr RunningObject kRunningObjects[] = {
    {body::kSun, QT_TRANSLATE_NOOP("horcom::MainWindow", "SONNE")},
    {body::kMoon, QT_TRANSLATE_NOOP("horcom::MainWindow", "MOND")},
    {body::kMercury, QT_TRANSLATE_NOOP("horcom::MainWindow", "MERKUR")},
    {body::kVenus, QT_TRANSLATE_NOOP("horcom::MainWindow", "VENUS")},
    {body::kMars, QT_TRANSLATE_NOOP("horcom::MainWindow", "MARS")},
    {body::kJupiter, QT_TRANSLATE_NOOP("horcom::MainWindow", "JUPITER")},
    {body::kSaturn, QT_TRANSLATE_NOOP("horcom::MainWindow", "SATURN")},
    {body::kUranus, QT_TRANSLATE_NOOP("horcom::MainWindow", "URANUS")},
    {body::kNeptune, QT_TRANSLATE_NOOP("horcom::MainWindow", "NEPTUN")},
    {body::kPluto, QT_TRANSLATE_NOOP("horcom::MainWindow", "PLUTO")},
    {body::kNodeAsc, QT_TRANSLATE_NOOP("horcom::MainWindow", "MONDKNOTEN N")},
    {body::kApogee, QT_TRANSLATE_NOOP("horcom::MainWindow", "SCHWARZER MOND")},
    {body::kChiron, QT_TRANSLATE_NOOP("horcom::MainWindow", "CHIRON")},
    {body::kTranspluto, QT_TRANSLATE_NOOP("horcom::MainWindow", "TRANS-PLUTO   HYPOTH.")},
    {body::kCeres, QT_TRANSLATE_NOOP("horcom::MainWindow", "CERES")},
    {body::kPallas, QT_TRANSLATE_NOOP("horcom::MainWindow", "PALLAS")},
    {body::kJuno, QT_TRANSLATE_NOOP("horcom::MainWindow", "JUNO")},
    {body::kVesta, QT_TRANSLATE_NOOP("horcom::MainWindow", "VESTA")},
    {body::kCupido, QT_TRANSLATE_NOOP("horcom::MainWindow", "CUPIDO      HYPOTH.")},
    {body::kHades, QT_TRANSLATE_NOOP("horcom::MainWindow", "HADES      HYPOTH.")},
    {body::kZeus, QT_TRANSLATE_NOOP("horcom::MainWindow", "ZEUS       HYPOTH.")},
    {body::kKronos, QT_TRANSLATE_NOOP("horcom::MainWindow", "KRONOS     HYPOTH.")},
    {body::kApollon, QT_TRANSLATE_NOOP("horcom::MainWindow", "APOLLON    HYPOTH.")},
    {body::kAdmetos, QT_TRANSLATE_NOOP("horcom::MainWindow", "ADMETOS    HYPOTH.")},
    {body::kVulkanus, QT_TRANSLATE_NOOP("horcom::MainWindow", "VULKANUS   HYPOTH.")},
    {body::kPoseidon, QT_TRANSLATE_NOOP("horcom::MainWindow", "POSEIDON   HYPOTH.")},
    {body::kQuaoar, QT_TRANSLATE_NOOP("horcom::MainWindow", "QUAOAR")},
    {body::kHalley, QT_TRANSLATE_NOOP("horcom::MainWindow", "KOMET HALLEY")},
    {body::kPholus, QT_TRANSLATE_NOOP("horcom::MainWindow", "PHOLUS")},
    {body::kDamokles, QT_TRANSLATE_NOOP("horcom::MainWindow", "DAMOKLES")},
    {body::kNessus, QT_TRANSLATE_NOOP("horcom::MainWindow", "NESSUS")},
    {body::kXena, QT_TRANSLATE_NOOP("horcom::MainWindow", "XENA")},
};

QString date_text(double jd, Calendar calendar) {
  const CalendarDate d = calendar_date(jd, calendar);
  return QString::asprintf("%02d.%02d.%d", d.day, d.month, d.year);
}

bool contains(const std::vector<int>& v, int slot) {
  return std::find(v.begin(), v.end(), slot) != v.end();
}

// his scd$ in the heading, LEFT$(scd$,6) of the key as STR$ wrote it
QString key_text(double key) {
  return QString::number(key, 'g', 12).left(6);
}

// his raus over a long sweep, the window with the progress and ABBRUCH.
// ESC or ABBRUCH asks his prompt, ABBRUCH there ends the sweep and the
// list keeps what was found. Without a prompt the stop is at once, the
// ESC of the linear graph. The window is no QDialog, it only blocks the
// main window while the sweep runs
class SweepGuard final : public QWidget {
 public:
  SweepGuard(QWidget* parent, const QString& title, const QString& prompt)
      : QWidget(parent, Qt::Dialog), prompt_(prompt) {
    setWindowTitle("HORCOM");
    setWindowModality(Qt::WindowModal);
    auto* v = new QVBoxLayout(this);
    v->addWidget(new QLabel(title, this));
    bar_ = new QProgressBar(this);
    bar_->setRange(0, kSweepSteps);
    v->addWidget(bar_);
    auto* stop = new QPushButton(QCoreApplication::translate("horcom::MainWindow", "ABBRUCH"), this);
    v->addWidget(stop);
    QObject::connect(stop, &QPushButton::clicked, [this]() { requested_ = true; });
    clock_.start();
    QApplication::setOverrideCursor(Qt::BusyCursor);
  }
  ~SweepGuard() override { QApplication::restoreOverrideCursor(); }
  SweepGuard(const SweepGuard&) = delete;
  SweepGuard& operator=(const SweepGuard&) = delete;

  /// @return the callback the sweep reports to
  [[nodiscard]] SweepProgress callback() {
    return [this](double f) { return step(f); };
  }
  /// @return true when the run was stopped
  [[nodiscard]] bool cancelled() const { return cancelled_; }

 protected:
  void keyPressEvent(QKeyEvent* e) override {
    if (e->key() == Qt::Key_Escape) {
      requested_ = true;
      return;
    }
    QWidget::keyPressEvent(e);
  }
  void closeEvent(QCloseEvent* e) override {
    requested_ = true;
    e->ignore();
  }

 private:
  bool step(double f) {
    if (cancelled_) {
      return false;
    }
    if (!isVisible()) {
      if (clock_.elapsed() < kSweepShowMs) {
        return true;
      }
      show();
    }
    if (poll_.isValid() && poll_.elapsed() < kSweepPollMs) {
      return true;
    }
    poll_.start();
    bar_->setValue(static_cast<int>(std::lround(std::clamp(f, 0.0, 1.0) * kSweepSteps)));
    QCoreApplication::processEvents();
    if (!requested_) {
      return true;
    }
    requested_ = false;
    int b = 0;
    if (!prompt_.isEmpty()) {
      //RR HIER SCHON ABBRECHEN ?
      b = ChoiceDialog::ask(this, "HORCOM", {prompt_},
                            {QCoreApplication::translate("horcom::MainWindow", "ABBRUCH"),
                             QCoreApplication::translate("horcom::MainWindow", "WEITER")},
                            0);
    }
    cancelled_ = b == 0;
    return !cancelled_;
  }

  QString prompt_;
  QProgressBar* bar_ = nullptr;
  QElapsedTimer clock_;
  QElapsedTimer poll_;
  bool requested_ = false;
  bool cancelled_ = false;
};

}  // namespace

// ported from a18eing with a17dat, delblackmoon and notvoidofcourse, the
// table branch of every a18 run, a18eing_plw asks from a18_factor_box
std::optional<MainWindow::A18Answers> MainWindow::a18_questions(A18Mode mode, bool linear) {
  A18Answers a;
  a.mode = mode;
  const ChartSettings s = current_settings();
  const bool helio = s.heliocentric;
  const bool dated = mode == A18Mode::kTransits || mode == A18Mode::kMundane;
  const bool arcs = mode == A18Mode::kSymbolic || mode == A18Mode::kSymbolicMundane || mode == A18Mode::kPrimary;

  // a17dat skips the box for the linear graph, ausgd! DATUM holds
  if (!dated && !linear) {
    //RR FORMAT der AUSGABE ?
    QStringList formats{tr("DATUM"), tr("LEBENSJAHR/MONAT")};
    if (arcs) {
      formats << tr("BOGEN");
    }
    const int f = ChoiceDialog::ask(this, "HORCOM", {tr("FORMAT der AUSGABE ?")}, formats, 0);
    if (f < 0) {
      return std::nullopt;
    }
    a.format = f == 0 ? DirectionFormat::kDate : (f == 1 ? DirectionFormat::kAge : DirectionFormat::kArc);
    if (f == 2) {
      //RR MAßEINHEIT für BOGEN ?
      const int u = ChoiceDialog::ask(this, "HORCOM", {tr("MAßEINHEIT für BOGEN ?")},
                                      {tr("GRAD-MINUTE-SEKUNDE"), tr("DEZIMAL-GRAD")}, 0);
      if (u < 0) {
        return std::nullopt;
      }
      a.format = u == 1 ? DirectionFormat::kArcDecimal : DirectionFormat::kArc;
    }
  }
  const bool arc_output = a.format == DirectionFormat::kArc || a.format == DirectionFormat::kArcDecimal;

  // IF mund! AND tab! AND hrg! = 0, the hour astrology note
  const bool hour_note = mode == A18Mode::kMundane && !helio && !linear;
  const QString t = tr(" => TEILER ");
  const QString note = tr(" ( auch eventl. für STUNDEN-Astrologie )");
  //RR GRUND-ASPEKT WÄHLEN!
  const QStringList grund{"15°" + t + "24",
                          "22.5°" + t + "16",
                          "30°" + t + "12",
                          "45°" + t + " 8",
                          hour_note ? tr("30° und 45°") + note : tr("30° und 45°") + t + tr("12 und 8"),
                          hour_note ? tr("60° und 45°") + note : tr("60° und 45°") + t + tr(" 6 und 8"),
                          "60°" + t + " 6",
                          "90°" + t + " 4",
                          hour_note ? tr("60° und 90°") + note : tr("60° und 90°") + t + tr(" 6 und 4"),
                          "180°" + t + " 2",
                          tr("360° = 0° = KONJUNKTION") + t + " 1",
                          tr("Sonstige ASPEKTE mit TEILER : 3,5,7,9,10,11,13,14,15,16"),
                          tr("ABBRUCH")};
  // alerte(9,...), 60 und 90 carries the return key, the linear graph
  // alerte(3,...," 1"," 2"," 5"," 6"," 9") greys the combined grids
  const int ga = linear ? ChoiceDialog::ask_with_disabled(this, tr("GRUND-ASPEKT WÄHLEN!"), {}, grund, 2, {0, 1, 4, 5, 8})
                        : ChoiceDialog::ask(this, tr("GRUND-ASPEKT WÄHLEN!"), {}, grund, 8);
  switch (ga) {
    case 0: a.base_deg = 15.0; break;
    case 1: a.base_deg = 22.5; break;
    case 2: a.base_deg = 30.0; break;
    case 3: a.base_deg = 45.0; break;
    case 4:
      a.base_deg = 15.0;
      a.grid = AspectGrid::k30And45;
      break;
    case 5:
      a.base_deg = 15.0;
      a.grid = AspectGrid::k60And45;
      break;
    case 6: a.base_deg = 60.0; break;
    case 7: a.base_deg = 90.0; break;
    case 8:
      a.base_deg = 30.0;
      a.grid = AspectGrid::k60And90;
      break;
    case 9: a.base_deg = 180.0; break;
    case 10: a.base_deg = 360.0; break;
    case 11: {
      //RR SONSTIGE TEILER : 3,5,7,9,11,13,14,15,16
      const auto divisor = ask_number(this, tr("SONSTIGE TEILER : 3,5,7,9,11,13,14,15,16"),
                                      tr("TEILER als GANZE ZAHL eingeben !"), 1.0, kLargestDivisor,
                                      kDefaultDivisor, 0);
      if (!divisor) {
        return std::nullopt;
      }
      a.base_deg = kDegPerCircle / *divisor;
      break;
    }
    default:
      return std::nullopt;
  }
  if (a.grid != AspectGrid::kPlain && mode == A18Mode::kMundane && !helio) {
    //RR LAUFENDEN Faktor NUR INNERHALB eines Zeichens ausgeben ?
    const int v = ChoiceDialog::ask(this, "HORCOM",
                                    {tr("LAUFENDEN Faktor NUR INNERHALB eines Zeichens ausgeben ?"),
                                     tr("Für STUNDEN - ASTROLOGIE"), tr("'Void of Course' ausschließen !")},
                                    {tr("JA"), tr("NEIN")}, 1);
    if (v < 0) {
      return std::nullopt;
    }
    a.within_sign = v == 0;
  }

  if (arcs && !arc_output) {
    //RR SCHLÜSSEL-ZAHL Eingeben !
    // NAIBOD = tja / 360 JAHRE/GRAD
    const double naibod = last_chart_->ta.tropical_year_days / kDegPerCircle;
    const int k = ChoiceDialog::ask(
        this, "HORCOM",
        {tr("SCHLÜSSEL-ZAHL Eingeben !"), QString(), tr("NAIBOD = %1JAHRE/GRAD").arg(QString::number(naibod, 'f', 6))},
        {tr("1JAHR/GRAD"), tr("NAIBOD"), tr("SONSTIGE")}, 1);
    if (k < 0) {
      return std::nullopt;
    }
    if (k == 0) {
      a.key = 1.0;
    } else if (k == 1) {
      a.key = naibod;
    } else {
      //RR Wert für 'JAHRE/GRAD' als DEZIMALZAHL eingeben !
      const auto key = ask_number(this, tr("JAHRE/GRAD eingeben !"), tr(" Wert für 'JAHRE/GRAD' als DEZIMALZAHL eingeben !"),
                                  0.001, 100.0, 1.0, 4);
      if (!key) {
        return std::nullopt;
      }
      a.key = *key;
    }
  }

  // a18eing_plw asks under tras!, mund!, prog!, sobg!, mob!, syb!, prim!
  // and ars!, the mundane mars! keeps every factor
  if (mode != A18Mode::kSymbolicMundane && !a18_factor_box(a, linear)) {
    return std::nullopt;
  }

  if (dated && linear) {
    // h$ = "   INTERVALL : 1,4 oder 16 MONATE", the window opens on the first
    const auto begin = ask_date(this, tr("BEGINN - DATUM") + tr("   INTERVALL : 1,4 oder 16 MONATE"), today_date(),
                                a.within_sign ? tr("GREENWICH-ZEIT") : QString());
    if (!begin) {
      return std::nullopt;
    }
    // IF lin!, ta = 1
    CalendarDate first = *begin;
    first.day = 1;
    if (!a.within_sign) {
      first.hour = 0.0;
      first.minute = 0.0;
    }
    a.jd_from = julian_day(first, s.calendar);
    //RR ZEIT-INTERVALL ?, 1 MONAT, 4 MONATE, 16 MONATE
    const int l1 = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("ZEIT-INTERVALL ?")},
                                     {tr("1 MONAT"), tr("4 MONATE"), tr("16 MONATE")}, 1);
    if (l1 < 0) {
      return std::nullopt;
    }
    a.jd_to = a.jd_from + kWindowDays[l1];
  } else if (dated) {
    // BEGINN - DATUM and ENDE-DATUM, an end before the start asks again
    CalendarDate start = today_date();
    for (;;) {
      // BEGINN - DATUM with GREENWICH-ZEIT for the hour astrology
      const auto begin =
          ask_date(this, tr("BEGINN - DATUM"), start, a.within_sign ? tr("GREENWICH-ZEIT") : QString());
      if (!begin) {
        return std::nullopt;
      }
      const QDate b(begin->year, begin->month, begin->day);
      const QDate e0 = b.isValid() ? b.addMonths(kDefaultWindowMonths) : QDate::currentDate();
      const auto end = ask_date(this, tr("ENDE-DATUM"), {e0.day(), e0.month(), e0.year(), 0.0, 0.0});
      if (!end) {
        return std::nullopt;
      }
      a.jd_from = julian_day(*begin, s.calendar);
      const double end_day = julian_day({end->day, end->month, end->year, 0.0, 0.0}, s.calendar);
      if (end_day <= a.jd_from) {
        //RR FEHL-EINGABE
        QMessageBox::warning(this, "HORCOM", tr("FEHL-EINGABE "));
        start = *begin;
        continue;
      }
      // his sweep ran on past the end day's midnight, the end day counts
      a.jd_to = end_day + 1.0;
      break;
    }
  } else if (linear) {
    // " BEGINN der ZÄHLUNG der LEBENSJAHRE", g$ = " INTERVALL : 5,10,20,40,80 JAHRE"
    for (;;) {
      const auto from = ask_number(this, tr(" BEGINN der ZÄHLUNG der %1").arg(tr("LEBENSJAHRE")),
                                   tr(" INTERVALL : 5,10,20,40,80 JAHRE"), 0.0, kLastLifeYear,
                                   std::numeric_limits<double>::quiet_NaN(), 1);
      if (!from) {
        return std::nullopt;
      }
      if (*from > kCheckLifeYear) {
        const int sure = ChoiceDialog::ask(this, "HORCOM", {tr("Sind das wirklich LEBENSJAHRE ?")},
                                           {tr("JA = WEITER"), tr("KORRIGIEREN")}, 0);
        if (sure != 0) {
          continue;
        }
      }
      a.from_years = *from + kEps;
      break;
    }
    //RR ZEIT-INTERVALL WÄHLEN !
    // lja$ + " bis " + lja + 5 * 2 ^ (i& - 1) + " JAHRE"
    QStringList spans;
    const double lja = std::floor(a.from_years + kEps * 10.0);
    for (int k = 0; k < 5; ++k) {
      spans << tr("%1 bis %2 JAHRE").arg(QString::number(lja)).arg(QString::number(lja + 5.0 * (1 << k)));
    }
    const int es = ChoiceDialog::ask(this, tr(" ZEIT-INTERVALL WÄHLEN !"), {}, spans, 2);
    if (es < 0) {
      return std::nullopt;
    }
    a.to_years = a.from_years + 5.0 * (1 << es);
  } else {
    // ZÄHLUNG der LEBENSJAHRE, BOGEN-WEITE for the arc output
    const QString what = arc_output ? tr("BOGEN-WEITE") : tr("LEBENSJAHRE");
    for (;;) {
      const auto from = ask_number(this, "HORCOM", tr(" BEGINN der ZÄHLUNG der %1").arg(what), 0.0, kLastLifeYear,
                                   0.0, 1);
      if (!from) {
        return std::nullopt;
      }
      if (*from > kCheckLifeYear && !arc_output) {
        //RR Sind das wirklich LEBENSJAHRE ?
        const int sure = ChoiceDialog::ask(this, "HORCOM", {tr("Sind das wirklich LEBENSJAHRE ?")},
                                           {tr("JA = WEITER"), tr("KORRIGIEREN")}, 0);
        if (sure != 0) {
          continue;
        }
      }
      a.from_years = *from + kEps;
      break;
    }
    const auto to = ask_number(this, "HORCOM", tr(" ENDE der ZÄHLUNG der %1").arg(what), 0.0, kLastLifeYear,
                               std::min(kLastLifeYear, a.from_years + kDefaultLifeSpan), 1);
    if (!to) {
      return std::nullopt;
    }
    // lje = VAL(lje$) + 1, at least a year and the key past the start,
    // never beyond 180
    a.to_years = *to + 1.0;
    a.to_years = std::max(a.to_years, a.from_years + 1.0 + (arcs ? a.key : 0.0));
    a.to_years = std::min(kLastLifeYear, a.to_years);
  }

  // IF lin!, @avd_lin
  if (linear && !linear_orientation()) {
    return std::nullopt;
  }

  // mas! = -1 in a19, asymb, ar_sys and aprim, the moon always takes part
  if (mode == A18Mode::kSunArc || mode == A18Mode::kMoonArc || arcs) {
    a.moon = true;
  }
  const bool asks_moon = mode == A18Mode::kTransits || mode == A18Mode::kMundane || mode == A18Mode::kSecondary;
  // NOT(lin! && ((jdend - jdbeg > 480) OR w4d < 30))
  const bool lin_skip = linear && ((dated && a.jd_to - a.jd_from > 480.0) || a.base_deg < 30.0);
  if (asks_moon && !lin_skip && a.chosen.empty() && a.marked.empty() && !helio && a.first_slot < body::kMars) {
    //RR MOND BERÜCKSICHTIGEN ?
    const int m = ChoiceDialog::ask(this, " ", {tr("MOND BERÜCKSICHTIGEN ?"), tr("( VIELE Auslösungen ! )")},
                                    {tr(" NEIN "), tr("JA")}, 0);
    if (m < 0) {
      return std::nullopt;
    }
    a.moon = m == 1;
  }
  // delblackmoon asks only in the table branch, his lin! = 0
  if (asks_moon && !linear && s.true_apogee && !helio && a.chosen.empty() && s.extra_bodies && s.nk[1] > 0) {
    //RR Wollen Sie auch den WAHREN schwarzen Mond als LAUFENDEN FAKTOR darstellen ?
    const int d = ChoiceDialog::ask(this, "HORCOM",
                                    {tr("Wollen Sie  auch den WAHREN schwarzen Mond"),
                                     tr("als LAUFENDEN FAKTOR darstellen ?"), tr("Das ergibt VIELE Auslösungen !")},
                                    {tr("NEIN"), tr("JA,TROTZDEM")}, 0);
    if (d < 0) {
      return std::nullopt;
    }
    a.drop_true_apogee = d == 0;
  }
  return a;
}

// ported from a18eing_plw, LAUFENDE FAKTOREN AUSWÄHLEN ?, PLANETEN
// AUSWÄHLEN ? for the arcs. The linear graph blanks and greys ROT
// MARKIEREN, the arcs ask again after a single factor like his a18ei2 loop
bool MainWindow::a18_factor_box(A18Answers& a, bool linear) {
  const ChartSettings s = current_settings();
  const bool helio = s.heliocentric;
  const bool arcs =
      a.mode == A18Mode::kSymbolic || a.mode == A18Mode::kSymbolicMundane || a.mode == A18Mode::kPrimary;
  const bool running = !arcs;
  const QString single = running ? tr("LAUFENDE PLANETEN") : tr("PLANETEN");
  const QStringList factor_rows{tr("ALLE"), tr("Ab MARS    AUFWÄRTS"), tr("Ab JUPITER AUFWÄRTS"),
                                linear ? QStringLiteral("  ") : tr("EINZELNE %1 ROT MARKIEREN").arg(single),
                                tr("EINZELNE %1 WÄHLEN und NUR DIESE DARSTELLEN").arg(single)};
  const QString factor_title = running ? tr(" LAUFENDE FAKTOREN AUSWÄHLEN ? ") : tr(" PLANETEN AUSWÄHLEN ? ");
  for (bool asked = false; !asked;) {
    const int f = linear ? ChoiceDialog::ask_with_disabled(this, factor_title, {}, factor_rows, 0, {kMarkRow})
                         : ChoiceDialog::ask(this, factor_title, {}, factor_rows, 0);
    asked = true;
    switch (f) {
      case 0: a.first_slot = arcs ? 0 : body::kSun; break;
      case 1: a.first_slot = body::kMars; break;
      case 2: a.first_slot = body::kJupiter; break;
      case kMarkRow:
      case kChooseRow: {
        // plan_wahl offers what the chart carries, TERRA instead of the
        // moon and neither sun, node nor black moon in the hrg mode
        std::vector<int> offered;
        QStringList names;
        for (const RunningObject& o : kRunningObjects) {
          const BodyState& b = last_chart_->b[static_cast<std::size_t>(o.slot)];
          if (!b.present) {
            continue;
          }
          if (helio && (o.slot == body::kSun || o.slot == body::kNodeAsc || o.slot == body::kApogee)) {
            continue;
          }
          offered.push_back(o.slot);
          names << (helio && o.slot == body::kMoon ? tr("TERRA") : tr(o.caption));
        }
        if (arcs) {
          // ASZENDENT and MC stand in the list of the directions
          offered.push_back(body::kAscendant);
          names << tr("ASZENDENT");
          offered.push_back(body::kMc);
          names << tr("MC");
        }
        const auto picked = ask_objects(this, offered, names);
        if (!picked) {
          return false;
        }
        if (arcs && picked->size() == 1) {
          //RR MINDESTENS 2 FAKTOREN WÄHLEN !
          QMessageBox::information(this, "HORCOM", tr("MINDESTENS 2 FAKTOREN WÄHLEN !"));
          asked = false;
          continue;
        }
        // IF plw&(2) && hrg! = 0, mas! = -1
        if (!helio && contains(*picked, body::kMoon)) {
          a.moon = true;
        }
        a.first_slot = arcs ? 0 : body::kSun;
        if (f == kMarkRow) {
          a.marked = *picked;
        } else {
          a.chosen = *picked;
          // IF plw&(n1&) && apogw!, mas! = -1
          if (contains(*picked, body::kApogee) && s.true_apogee) {
            a.moon = true;
          }
        }
        break;
      }
      default:
        return false;
    }
  }
  return true;
}

// ported from a18kopf and grundaspekt$
QStringList MainWindow::a18_heading(const QString& title, const A18Answers& a, bool with_record) const {
  const ChartSettings s = current_settings();
  QString grund = " " + QString::number(a.base_deg, 'g', 6) + "°";
  switch (a.grid) {
    case AspectGrid::k30And45: grund = tr(" 30° u. 45°"); break;
    case AspectGrid::k60And45: grund = tr(" 60° u. 45°"); break;
    case AspectGrid::k60And90: grund = tr(" 60° u. 90°"); break;
    case AspectGrid::kPlain: break;
  }
  QString first = title + " |" + tr("Grund-Aspekt:") + grund;
  if (s.heliocentric) {
    first += tr(" Heliozentrisch");
  }
  // IF prim! OR ars! OR mars! OR syb!, schl$ in every format
  if (a.mode == A18Mode::kSymbolic || a.mode == A18Mode::kSymbolicMundane || a.mode == A18Mode::kPrimary) {
    first += " |" + tr("Schlüssel:%1 A/°").arg(key_text(a.key));
  }
  if (a.jd_to > a.jd_from) {
    first += "   " + tr("Ab %1").arg(date_text(a.jd_from, s.calendar));
  } else {
    first += "   " + tr("Lebensjahre %1 bis %2").arg(a.from_years, 0, 'f', 1).arg(a.to_years - 1.0, 0, 'f', 1);
  }
  QStringList out{first};
  if (with_record) {
    const ClassicSheetText txt = classic_sheet_text();
    // "Name : " + na$ + " |" + " " + sol$(od,ze) + " ", then "| Ort: " + go$
    out << tr("Name : %1 | %2 | Ort: %3   |Länge: %4 |Breite: %5")
               .arg(QString::fromStdString(txt.name).trimmed(), rhythm_chart_label(),
                    QString::fromStdString(txt.place).trimmed(), QString::fromStdString(txt.lon).trimmed(),
                    QString::fromStdString(txt.lat).trimmed());
    out << QString::fromStdString(txt.date).trimmed() + " | " + QString::fromStdString(txt.ut).trimmed();
  }
  return out;
}

// ported from ereig_ort, asked only with the parallax for the transits,
// the mundane aspects and the secondary direction, the answer names the
// Ereignis-Ort line of a18tab
bool MainWindow::event_place_for(SearchContext& ctx, QString& footer, bool mundane) {
  footer.clear();
  // with the parallax AC and MC of the running sky belong to the event place
  if (ctx.settings.heliocentric || !ctx.settings.topocentric_parallax) {
    return true;
  }
  // his textzentrl lines above the box, mund! asks with the one line
  const QStringList notes =
      mundane ? QStringList{tr(" EREIGNISORT ? ( Wegen PARALLAXE )")}
              : QStringList{tr(" PARALLAXE evtl. AUSSCHALTEN,da AC und MC NICHT DEM GEBURTSORT ENTSPRECHEN !"),
                            tr(" Oder den GEBURTSORT wählen !")};
  const auto place = ask_event_place(notes);
  if (!place) {
    return false;
  }
  ctx.base.lon_deg_east = place->lon;
  ctx.base.lat_deg = place->lat;
  // IF pr! && goe$ > "" && (gle > kk OR gge > kk)
  if (std::abs(place->lon) > kEps || std::abs(place->lat) > kEps) {
    footer = tr(" Ereignis-Ort : %1 | Länge : %2 | Breite : %3")
                 .arg(QString::fromStdString(place->name).trimmed())
                 .arg(place->lon, 0, 'f', 3)
                 .arg(place->lat, 0, 'f', 3);
  }
  return true;
}

TransitScan MainWindow::a18_scan(const A18Answers& a) const {
  TransitScan scan;
  scan.jd_from_ut = a.jd_from;
  scan.jd_to_ut = a.jd_to;
  scan.base_angle_deg = a.base_deg;
  scan.grid = a.grid;
  scan.first_slot = std::max(1, a.first_slot);
  scan.chosen = a.chosen;
  scan.moon_aspects = a.moon;
  scan.drop_true_apogee = a.drop_true_apogee;
  scan.house_targets = konsta_.zwhd;
  scan.cardinal_targets = konsta_.kard;
  scan.midpoints = konsta_.halbs_dir;
  return scan;
}

// his a18asw box of the TRANSITE, SEKUNDÄR and BOGEN runs, the mob! run
// greys HOROSKOP-GRAPHIK
int MainWindow::ask_output_mode(bool moon_arc) {
  //RR Ausgabe-Modus ?
  const QStringList rows{tr("TABELLE"), tr("HOROSKOP-GRAPHIK"), tr("LINEAR-GRAPHIK"), tr("ABBRUCH")};
  if (moon_arc) {
    return ChoiceDialog::ask_with_disabled(this, tr("     Ausgabe-Modus ?     "), {}, rows, 0, {1});
  }
  return ChoiceDialog::ask(this, tr("     Ausgabe-Modus ?     "), {}, rows, 0);
}

void MainWindow::force_geocentric() {
  if (helio_->isChecked()) {
    helio_->setChecked(false);
  }
}

// ported from a20 with a18asw, TABELLE, HOROSKOP-GRAPHIK or LINEAR-GRAPHIK
void MainWindow::transite() {
  if (!last_chart_) {
    return;
  }
  switch (ask_output_mode()) {
    case 0: transit_list(); return;
    case 1: wheel_run(A18Mode::kTransits); return;
    case 2: linear_run(A18Mode::kTransits); return;
    default: return;
  }
}

// ported from a20_horg. The LAUFENDE FAKTOREN box of a18eing_plw first,
// then the moment, under tras! the event place of ort_wahl with the
// DATUM-ZEIT-EINGABE, under prog! and sobg! the SUCH-DATUM, and last his
// zeitwi walk. The transits ride on the transit view of the panel, the
// progressed rings on the arc view, every step runs through recompute so
// the main wheel follows the walk
void MainWindow::wheel_run(A18Mode mode) {
  if (!last_chart_) {
    return;
  }
  A18Answers a;
  a.mode = mode;
  if (!a18_factor_box(a, false)) {
    return;
  }
  const Chart radix = radix_chart();
  const double tja = radix.ta.tropical_year_days;
  const Calendar cal = current_settings().calendar;
  const bool transits = mode == A18Mode::kTransits;
  // his jd, the transit moment or the progressed moment jd1 + da
  double jd = 0.0;
  std::optional<EventPlace> place;
  QString title = tr(" TRANSITE ");
  if (transits) {
    // @ort_wahl(0,0) with zuort, the running sky stands at the event place
    place = ask_event_place();
    if (!place) {
      return;
    }
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const CalendarDate start{now.date().day(), now.date().month(), now.date().year(),
                             static_cast<double>(now.time().hour()), static_cast<double>(now.time().minute())};
    //RR DATUM-ZEIT-EINGABE, GREENWICH-ZEIT
    const auto d = ask_date(this, tr("DATUM-ZEIT-EINGABE"), start, tr("GREENWICH-ZEIT"));
    if (!d) {
      return;
    }
    jd = julian_day(*d, cal);
  } else {
    title = mode == A18Mode::kSecondary ? tr(" SEKUNDÄR-Direktion ") : tr(" SONNEN-BOGEN-DIR.");
    //RR SUCH-DATUM eingeben !
    const auto d = ask_date(this, tr("SUCH-DATUM eingeben !"), today_date());
    if (!d) {
      return;
    }
    // ho = ho(1,ze), mi = mi(1,ze), the day at the radix clock is jd2
    const double jd2 = event_at_radix_clock(radix, julian_day({d->day, d->month, d->year, 0.0, 0.0}, cal), false);
    // da = (jd2 - jd1) / tja with jd = jd1 + da, sobg! stays there
    ProgressionMode clock = ProgressionMode::kProportional;
    if (mode == A18Mode::kSecondary) {
      //RR am 24.11.06 abgeschaltet
      // his note at the IF prog! of the box, which still runs. The
      // prog_mode rows ze$(0) and ze$(1) stand blank and grey, the fourth
      // carries the return key
      const int es = ChoiceDialog::ask_with_disabled(
          this, tr("RECHEN-MODUS ?"), {},
          {QStringLiteral("   "), QStringLiteral("   "), tr("HÄUSER-DREHUNG gemäß  '1 TAG = 1 JAHR'"),
           tr("STRENG PROPORTIONALE UMRECHNUNG des JD für PLANETEN und HÄUSER")},
          kProportionalRow, {0, 1});
      if (es != kHouseTurnRow && es != kProportionalRow) {
        return;
      }
      // CASE 3, @hd_hs(da,hs1) turns the clock of the progressed day
      if (es == kHouseTurnRow) {
        clock = ProgressionMode::kHouseRotation;
      }
    }
    const ProgressedMoment m = progressed_moment(radix, jd2, clock, make_context());
    if (!m.ok) {
      return;
    }
    jd = m.jd_ut;
  }
  const QString walk_title = title.trimmed() + " | " + record_label_.trimmed();
  {
    // the steps stay out of the Zurück list, the whole run makes one step
    const auto hold = qScopeGuard(hold_history());
    running_ring_ = RunningRing{a.first_slot, a.chosen, a.marked};
    // the walk draws his full screen, the transit view keeps it after the
    // walk like a20_horge until it is left
    a20_transits_ = transits;
    const auto show = [&](double at) {
      if (transits) {
        show_moment_transits(at, place);
        return;
      }
      // jd1 + (jd - jd1) * tja, the life date the PROG: line names
      arc_prog_jd_ = at;
      arc_jd_ = radix.jd_ut + (at - radix.jd_ut) * tja;
      arc_moon_ = false;
      arc_secondary_ = mode == A18Mode::kSecondary;
      claim_wheel();
      arc_action_->setChecked(true);
      recompute();
    };
    // the ring of the chosen moment stands behind the questions of zeitwi
    show(jd);
    ring_walk(walk_title, !transits, tja, jd, show);
    running_ring_.reset();
  }
  // a20_horge, the view keeps the last moment of the walk with every
  // running factor
  recompute();
}

// the table branch of a18asw for TRANSITE
void MainWindow::transit_list() {
  if (!last_chart_) {
    return;
  }
  const auto a = a18_questions(A18Mode::kTransits);
  if (!a) {
    return;
  }
  SearchContext ctx = make_context();
  QString footer;
  if (!event_place_for(ctx, footer)) {
    return;
  }
  // a timer may redraw the wheel while the sweep yields, the sweep keeps
  // its own copy of the radix
  const Chart radix = *last_chart_;
  std::vector<TransitEvent> events;
  {
    //RR Bitte warten ! Rechenzeit erforderlich !
    SweepGuard guard(this, tr("Bitte warten ! Rechenzeit erforderlich !"), tr("     HIER SCHON ABBRECHEN ?"));
    TransitScan scan = a18_scan(*a);
    scan.progress = guard.callback();
    events = scan_transits(radix, scan, ctx);
  }
  A18Display display;
  // di$ = " TRANSITE "
  display.heading = a18_heading(tr(" TRANSITE "), *a, true);
  display.plinv = konsta_.plinv;
  display.marked = a->marked;
  display.settings = ctx.settings;
  display.base_angle_deg = a->base_deg;
  // IF halbs_dir& > 0, klsyt! = 0
  display.small_symbols = konsta_.klsyt && konsta_.halbs_dir == 0;
  display.footer = footer;
  TransitListDialog dialog(std::move(events), display, this);
  mark_output(&dialog, menu_item::kTransits);
  if (dialog.open_sorted() != QDialog::Accepted || dialog.chosen_jd() <= 0.0) {
    return;
  }
  show_moment_transits(dialog.chosen_jd(), EventPlace{ctx.base.lon_deg_east, ctx.base.lat_deg, {}});
}

// ported from mund, TABELLE or LINEAR-GRAPHIK and the table branch
void MainWindow::mundane_aspects() {
  if (!last_chart_) {
    return;
  }
  //RR Ausgabe-Modus ?
  const int es = ChoiceDialog::ask(this, tr("     Ausgabe-Modus ?     "), {},
                                   {tr("TABELLE"), tr("LINEAR-GRAPHIK"), tr("ABBRUCH")}, 0);
  if (es == 1) {
    linear_run(A18Mode::kMundane);
    return;
  }
  if (es != 0) {
    return;
  }
  const auto a = a18_questions(A18Mode::kMundane);
  if (!a) {
    return;
  }
  SearchContext ctx = make_context();
  QString footer;
  if (!event_place_for(ctx, footer, true)) {
    return;
  }
  MundaneScan scan;
  scan.jd_from_ut = a->jd_from;
  scan.jd_to_ut = a->jd_to;
  scan.base_angle_deg = a->base_deg;
  scan.grid = a->grid;
  scan.first_slot = a->first_slot;
  scan.chosen = a->chosen;
  scan.moon = a->moon;
  scan.drop_true_apogee = a->drop_true_apogee;
  scan.within_sign = a->within_sign;
  std::vector<MundaneAspect> aspects;
  {
    SweepGuard guard(this, tr("Bitte warten ! Rechenzeit erforderlich !"), tr("     HIER SCHON ABBRECHEN ?"));
    scan.progress = guard.callback();
    aspects = scan_mundane_aspects(scan, ctx);
  }
  A18Display display;
  // di$ = " Ekliptikale " + mu$ + "-Aspekte "
  display.heading = a18_heading(tr(" Ekliptikale Mundan-Aspekte "), *a, false);
  display.plinv = konsta_.plinv;
  display.marked = a->marked;
  display.settings = ctx.settings;
  display.base_angle_deg = a->base_deg;
  display.small_symbols = konsta_.klsyt && konsta_.halbs_dir == 0;
  display.footer = footer;
  TransitListDialog dialog(std::move(aspects), display, this);
  mark_output(&dialog, menu_item::kMundane);
  if (dialog.open_sorted() != QDialog::Accepted || dialog.chosen_jd() <= 0.0) {
    return;
  }
  show_moment_transits(dialog.chosen_jd(), EventPlace{ctx.base.lon_deg_east, ctx.base.lat_deg, {}});
}

// ported from a18, the SEKUNDÄR-DIREKTION branch, the DYNAMOGRAMM
// branch keeps its own menu entry
void MainWindow::secondary_direction() {
  if (!last_chart_) {
    return;
  }
  // CLR hrg!, the direction runs geocentric
  force_geocentric();
  switch (ask_output_mode()) {
    case 0: {
      const auto a = a18_questions(A18Mode::kSecondary);
      if (!a) {
        return;
      }
      SearchContext ctx = make_context();
      QString footer;
      if (!event_place_for(ctx, footer)) {
        return;
      }
      const Chart radix = radix_chart();
      TransitScan life = a18_scan(*a);
      life.jd_from_ut = radix.jd_ut + a->from_years * radix.ta.tropical_year_days;
      life.jd_to_ut = radix.jd_ut + a->to_years * radix.ta.tropical_year_days;
      std::vector<DirectedEvent> events;
      {
        SweepGuard guard(this, tr("Bitte warten ! Rechenzeit erforderlich !"), tr("     HIER SCHON ABBRECHEN ?"));
        life.progress = guard.callback();
        events = secondary_direction_events(radix, life, ctx);
      }
      DirectionDisplay display;
      // di$ = " SEKUNDÄR-Direktion "
      display.heading = a18_heading(tr(" SEKUNDÄR-Direktion "), *a, true);
      display.footer = footer;
      display.format = a->format;
      display.plinv = konsta_.plinv;
      display.marked = a->marked;
      display.base_angle_deg = a->base_deg;
      display.calendar = ctx.settings.calendar;
      display.small_symbols = konsta_.klsyt && konsta_.halbs_dir == 0;
      DirectionListDialog dialog(std::move(events), radix, display, this);
      mark_output(&dialog, menu_item::kSecondary);
      dialog.open_sorted();
      return;
    }
    case 1:
      wheel_run(A18Mode::kSecondary);
      return;
    case 2:
      linear_run(A18Mode::kSecondary);
      return;
    default:
      return;
  }
}

// ported from a19, SONNEN- and MOND-BOGEN-DIREKTION
void MainWindow::arc_direction() {
  if (!last_chart_) {
    return;
  }
  // CLR hrg!, the direction runs geocentric
  force_geocentric();
  //RR SONNEN-BOGEN-DIREKTION ? oder MOND-BOGEN-DIREKTION
  const int which = ChoiceDialog::ask(
      this, "HORCOM",
      {tr("SONNEN-BOGEN-DIREKTION ?"), tr("oder"), tr("MOND-BOGEN-DIREKTION = 'TERTIÄR 2'-DIREKTION , viele Auslösungen !")},
      {tr("SONNE"), tr("MOND"), tr("ABBRUCH")}, 0);
  if (which != 0 && which != 1) {
    return;
  }
  const bool moon = which == 1;
  switch (ask_output_mode(moon)) {
    case 0: {
      const auto a = a18_questions(moon ? A18Mode::kMoonArc : A18Mode::kSunArc);
      if (!a) {
        return;
      }
      SearchContext ctx = make_context();
      const Chart radix = radix_chart();
      TransitScan life = a18_scan(*a);
      life.jd_from_ut = radix.jd_ut + a->from_years * radix.ta.tropical_year_days;
      life.jd_to_ut = radix.jd_ut + a->to_years * radix.ta.tropical_year_days;
      std::vector<DirectedEvent> events;
      {
        //RR PROGRAMM ABBRECHEN ? ( LISTE UNVOLLSTÄNDIG )
        SweepGuard guard(this, tr("Bitte warten ! Rechenzeit erforderlich !"),
                         tr("PROGRAMM ABBRECHEN ? ( LISTE UNVOLLSTÄNDIG )"));
        life.progress = guard.callback();
        events = arc_direction_events(radix, moon, life, ctx);
      }
      DirectionDisplay display;
      // di$ = " SONNEN-BOGEN-DIR." bzw. " MOND-BOGEN-DIR."
      display.heading = a18_heading(moon ? tr(" MOND-BOGEN-DIR.") : tr(" SONNEN-BOGEN-DIR."), *a, true);
      display.format = a->format;
      display.plinv = konsta_.plinv;
      display.marked = a->marked;
      display.base_angle_deg = a->base_deg;
      display.calendar = ctx.settings.calendar;
      display.small_symbols = konsta_.klsyt && konsta_.halbs_dir == 0;
      DirectionListDialog dialog(std::move(events), radix, display, this);
      mark_output(&dialog, menu_item::kArcDirection);
      dialog.open_sorted();
      return;
    }
    case 1:
      // the mob! run greys the row, a20_horg serves the sun arc alone
      if (!moon) {
        wheel_run(A18Mode::kSunArc);
      }
      return;
    case 2:
      linear_run(moon ? A18Mode::kMoonArc : A18Mode::kSunArc);
      return;
    default:
      return;
  }
}

// ported from asymb and ar_sys, the symbolic directions
void MainWindow::symbolic_direction(bool equatorial) {
  if (!last_chart_) {
    return;
  }
  DirectionMethod method = DirectionMethod::kSymbolicEcliptic;
  // di$ = " Eklipt. SYMBOL. DIREKTION "
  QString title = tr(" Eklipt. SYMBOL. DIREKTION ");
  if (equatorial) {
    //RR 'MUNDAN'-GEOMETRIE ? oder 'AR-SYSTEM' ?
    const int b = ChoiceDialog::ask(this, "HORCOM", {tr("'MUNDAN'-GEOMETRIE ?"), QString(), tr("'AR-SYSTEM' ?")},
                                    {tr("MUNDAN"), tr("AR-SYSTEM"), tr("ABBRUCH")}, 0);
    if (b == 0) {
      method = DirectionMethod::kSymbolicMundane;
      title = tr(" 'MUNDANE' SYMB. DIREKTION ");
    } else if (b == 1) {
      method = DirectionMethod::kSymbolicEquatorial;
      title = tr(" ÄQUAT. SYMBOL. DIREKTION ");
    } else {
      return;
    }
    // CLR hrg! in ar_sys, asymb keeps the heliocentric mode
    force_geocentric();
  }
  // mars! of the MUNDAN geometry, syb! and ars! share the boxes
  const auto a = a18_questions(method == DirectionMethod::kSymbolicMundane ? A18Mode::kSymbolicMundane
                                                                           : A18Mode::kSymbolic);
  if (!a) {
    return;
  }
  const Chart radix = radix_chart();
  DirectionRange range;
  range.from_years = a->from_years;
  range.to_years = a->to_years;
  range.base_angle_deg = a->base_deg;
  range.grid = a->grid;
  // IF wika!, scd$ = STR$(1), the arc output counts one year a degree
  const bool arc_output = a->format == DirectionFormat::kArc || a->format == DirectionFormat::kArcDecimal;
  range.key = arc_output ? 1.0 : a->key;
  range.first_slot = a->first_slot;
  // pl1& = aa&, ar_sys starts every walk at the first body
  if (equatorial) {
    range.first_slot = 0;
  }
  range.chosen = a->chosen;
  range.house_targets = konsta_.zwhd;
  range.cardinal_targets = konsta_.kard;
  range.midpoints = konsta_.halbs_dir;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::vector<DirectionHit> hits = direction_hits(radix, method, range, radix_input().lat_deg);
  QApplication::restoreOverrideCursor();
  show_direction_list(std::move(hits), *a, title, QString(),
                      equatorial ? menu_item::kSymbolicEquatorial : menu_item::kSymbolicEcliptic);
}

// ported from aprim with aprim1, the Kühr list
void MainWindow::primary_direction() {
  if (!last_chart_) {
    return;
  }
  // CLR hrg!, the direction runs geocentric
  force_geocentric();
  // @plre, PLACIDUS - HÄUSER ERFORDERLICH
  placidus_notice();
  const auto a = a18_questions(A18Mode::kPrimary);
  if (!a) {
    return;
  }
  const Chart radix = radix_chart();
  DirectionRange range;
  range.from_years = a->from_years;
  range.to_years = a->to_years;
  range.base_angle_deg = a->base_deg;
  range.grid = a->grid;
  const bool arc_output = a->format == DirectionFormat::kArc || a->format == DirectionFormat::kArcDecimal;
  range.key = arc_output ? 1.0 : a->key;
  range.first_slot = a->first_slot;
  range.chosen = a->chosen;
  range.significator_latitude = konsta_.bres == 1;
  range.with_latitude = konsta_.brep == 1;
  range.house_targets = konsta_.zwhd;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::vector<DirectionHit> hits = direction_hits(radix, DirectionMethod::kPrimary, range, radix_input().lat_deg);
  QApplication::restoreOverrideCursor();
  //RR Signifikator U. Promissor Mit Breite
  // the footer of a18tab
  QString footer;
  if (konsta_.bres == 1) {
    footer = konsta_.brep == 1 ? tr("Signifikator U. Promissor Mit Breite")
                               : tr("Signifikator Mit Promissor Ohne Breite");
  } else {
    footer = tr("Signifikator U. Promissor Ohne Breite");
  }
  // di$ = " PRIMÄR-Direktion "
  show_direction_list(std::move(hits), *a, tr(" PRIMÄR-Direktion "), footer, menu_item::kPrimary);
}

void MainWindow::show_direction_list(std::vector<DirectionHit> hits, const A18Answers& a, const QString& title,
                                     const QString& footer, int item) {
  DirectionDisplay display;
  display.heading = a18_heading(title, a, true);
  display.footer = footer;
  display.format = a.format;
  display.plinv = konsta_.plinv;
  display.marked = a.marked;
  display.base_angle_deg = a.base_deg;
  display.calendar = current_settings().calendar;
  // a181 draws the midpoint rows with the large symbols
  display.small_symbols = konsta_.klsyt && konsta_.halbs_dir == 0;
  const Chart radix = radix_chart();
  DirectionListDialog dialog(std::move(hits), radix, display, this);
  mark_output(&dialog, item);
  dialog.open_sorted();
}

// a picked row opens the running sky of its moment over the radix
void MainWindow::show_moment_transits(double jd_ut, std::optional<EventPlace> place) {
  transit_place_ = std::move(place);
  const CalendarDate d = calendar_date(jd_ut, current_settings().calendar);
  const int seconds = a18::clock_seconds(d);
  show_transits(QDate(d.year, d.month, d.day), QTime(seconds / 3600, (seconds / 60) % 60, seconds % 60));
  if (transit_on_->isChecked()) {
    recompute();
  }
}

// ported from the pl1& start and auswahl_flag of plein1 in the zeitwi
// ring, plan_wahl! draws only the chosen factors, plan_col! draws them
// all and marks the chosen red. The running AC and MC follow the same
// choice like every factor of plein1(212)
void MainWindow::trim_running_ring(Chart& ring, WheelOptions& opt) const {
  if (!running_ring_) {
    return;
  }
  const RunningRing& r = *running_ring_;
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    if (slot < r.first_slot || (!r.chosen.empty() && !contains(r.chosen, slot))) {
      ring.b[static_cast<std::size_t>(slot)].present = false;
    }
  }
  opt.outer_marked = r.marked;
}

// ported from the drgrph! branch of zeitwi for prog! and sobg!, a9 at the
// progressed moment or the radix moved by the sun's arc sb, plein1(212)
// and the texts in the centre
bool MainWindow::show_ring(const Chart& radix, const ChartInput& in, const ChartSettings& s,
                           const AspectResult& aspects, const WheelOptions& wopt) {
  Chart ring;
  if (arc_secondary_) {
    // IF tras! OR zeitw! OR prog!, @a9 at the radix place
    ChartInput at = in;
    at.date_ut = calendar_date(arc_prog_jd_, s.calendar);
    ring = compute_chart(at, s, vsop_, eph_);
  } else {
    // sb = FN nb(el(1) - ca(1)), cb(t&) = FN nb(ca(t&) + sb)
    ring = arc_directed_chart(radix, arc_moon_, arc_jd_, make_context());
  }
  if (!ring.ok) {
    return false;
  }
  WheelOptions opt = wopt;
  opt.chart_label.clear();
  opt.chart_sub_label.clear();
  opt.outer_axes = true;
  trim_running_ring(ring, opt);
  DisplayList dl = build_transit_wheel(radix, ring, s, aspects, opt);
  constexpr double amh = kWheelCenterX;
  constexpr double bmh = kWheelCenterY;
  // a$ + "=>" + q$ with a$ = "SECDIR." or "SOBDIR." and q$ = sol$(od,ze)
  const QString kind = arc_secondary_ ? QStringLiteral("SECDIR.") : QStringLiteral("SOBDIR.");
  add_sheet_text(dl, amh - 58.0, bmh - 12.0, (kind + "=>" + rhythm_chart_label()).toStdString(), kZeitwiText);
  // IF p!, datum3$ of the progressed moment, under prog! its ze$
  const CalendarDate p = calendar_date(arc_prog_jd_, s.calendar);
  add_sheet_text(dl, amh - 50.0, bmh - 4.0, datum3_text(p).toStdString(), kZeitwiText);
  if (arc_secondary_) {
    // zeit_form, w = 15 * (ho + mi / 60) through homise
    const double hours = a18::clock_seconds(p) / kSecondsPerHour;
    add_sheet_text(dl, amh - 50.0, bmh + 4.0, homise_text(kDegPerHour * hours, 0).toStdString(), kZeitwiText);
  }
  // "PROG:" + datum3$ of jd1 + (jd - jd1) * tja
  add_sheet_text(dl, amh - 50.0, bmh + 20.0, ("PROG:" + datum3_text(calendar_date(arc_jd_, s.calendar))).toStdString(),
                 kZeitwiText);
  // go$ of the radix, prog! and sobg! stand at the birth place
  const EventPlace home{in.lon_deg_east, in.lat_deg, record_.place};
  show_full_sheet(
      a20_sheet(radix, ring, s, arc_secondary_ ? A18Mode::kSecondary : A18Mode::kSunArc, home, std::move(dl)));
  // di$ with the life date of the ring
  const QString di = arc_secondary_ ? tr(" SEKUNDÄR-Direktion ") : tr(" SONNEN-BOGEN-DIR.");
  banner_->set_record(di.trimmed() + " " + date_text(arc_jd_, s.calendar));
  return true;
}

// ported from the drgrph! screen of a20_horg and zeitwi. bes11 writes the
// radix at xt& = 2 and the running chart at xt& = 112 from yt& = 15, each
// with the cusps of bes111, then ADD yt&,8 and a12asp at xt& = 2 with the
// running body first and his one degree orb. bes1 sets the rule at 220
// with the name beside it and the moment of the radix at h& = 494, bes111
// the Ereig.-Ort at 440
DisplayList MainWindow::a20_sheet(const Chart& radix, const Chart& ring, const ChartSettings& s, A18Mode mode,
                                  const EventPlace& place, DisplayList wheel) const {
  DisplayList& dl = wheel;
  const auto text = [&dl](double x, double bottom, const QString& t, double size) {
    add_sheet_text(dl, x, bottom, t.toStdString(), size);
  };
  // b$ + a$, Ekl.Länge: or Länge: under vl! with the mode tag A1 A2 W
  const QString tag = konsta_.appa == 2 ? "A2" : (konsta_.appa == 3 ? "W" : "A1");
  PairColumnOptions col;
  col.header = ((konsta_.voll ? tr("Länge:") : tr("Ekl.Länge:")) + tag).toStdString();
  int extras = 0;
  for (int slot = body::kApogee; slot < body::kSlotCount; ++slot) {
    extras += radix.b[static_cast<std::size_t>(slot)].present ? 1 : 0;
  }
  col.compact = s.extra_bodies && extras > 1;
  col.parallax = s.topocentric_parallax;
  col.true_node = s.true_node;
  col.true_apogee = s.true_apogee;
  col.heliocentric = s.heliocentric;
  col.extras = s.extra_bodies;
  col.houses_header = (konsta_.voll ? tr("Häusersp.") : tr("Häuserspitzen")).toStdString();
  col.house_name = QString::fromUtf8(radix.houses.name.data(), static_cast<int>(radix.houses.name.size())).trimmed().toStdString();
  constexpr double kColumnTop = 15.0;
  double y = add_pair_bodies(dl, radix, col, 2.0, kColumnTop);
  y = add_pair_houses(dl, radix, col, 2.0, y);
  // n$ over the running column, Transit:, Progress.: or SO-Bog.Dir:
  PairColumnOptions run = col;
  run.header = (mode == A18Mode::kTransits    ? tr("Transit:")
                : mode == A18Mode::kSecondary ? tr("Progress.:")
                                              : tr("SO-Bog.Dir:"))
                   .toStdString();
  run.present_only = true;
  run.house_name = QString::fromUtf8(ring.houses.name.data(), static_cast<int>(ring.houses.name.size())).trimmed().toStdString();
  double yr = add_pair_bodies(dl, ring, run, 112.0, kColumnTop);
  yr = add_pair_houses(dl, ring, run, 112.0, yr);
  // d1$ and d2$, T and R under tras!, P and R under prog! and sobg!
  CrossScanOptions scan;
  scan.orbs = CrossOrbs::kTransit;
  scan.extras = s.extra_bodies;
  scan.heliocentric = s.heliocentric;
  CrossGridOptions grid;
  grid.left_tag = mode == A18Mode::kTransits ? " T" : " P";
  grid.right_tag = " R";
  grid.left_tag2 = grid.left_tag;
  grid.right_tag2 = grid.right_tag;
  grid.running_first = true;
  grid.outer_color = outer_color_;
  grid.more_label = tr(" MEHR ").toStdString();
  // his yt& runs on from the running column, which closes up when factors
  // stay away, the grid starts under the longer of the two columns
  add_cross_grid(dl, scan_aspects_between(radix, ring, shown_aspect_settings(), scan), grid, 2.0, std::max(y, yr) + 8.0);

  // bes1 with trprso!, "Name:" at 202 + 20 and nam(25,12) under it, the
  // first word on its line and the rest below
  const ClassicSheetText own = classic_sheet_text();
  constexpr double kNameX = 222.0;
  constexpr double kCornerText = 14.0;
  text(kNameX, 14.0, own.name_label.empty() ? tr("Name:") : QString::fromStdString(own.name_label), kCornerText);
  const QString name = QString::fromStdString(own.name).trimmed();
  const qsizetype gap = name.indexOf(' ');
  text(kNameX, 26.0, (gap < 0 ? name : name.left(gap)).left(25), kCornerText);
  if (gap >= 0) {
    text(kNameX, 38.0, name.mid(gap + 1).trimmed().left(25), kCornerText);
  }
  // bes111, "Ereig.-Ort:" + LEFT$(go$,16) and the coordinates at 440
  constexpr double kPlaceX = 440.0;
  constexpr double kPlaceText = 12.0;
  text(kPlaceX, 14.0, tr("Ereig.-Ort:") + QString::fromStdString(place.name).trimmed().left(16), kPlaceText);
  text(kPlaceX, 26.0,
       tr("Lä:") + QString::asprintf("%6.2f %c", std::abs(place.lon), place.lon < 0.0 ? 'W' : 'E') + tr("|Br:") +
           QString::asprintf("%5.2f %c", std::abs(place.lat), place.lat < 0.0 ? 'S' : 'N'),
       kPlaceText);
  // the moment of the radix, dm$ + ":" + datum3$, "UT: " + ze$ and day_w$
  constexpr double kMomentX = 494.0;
  text(kMomentX, 435.0, QString::fromStdString(own.date), kCornerText);
  text(kMomentX, 445.0, QString::fromStdString(own.ut), kCornerText);
  text(kMomentX + 32.0, 455.0, QString::fromStdString(own.weekday), kCornerText);
  return wheel;
}

// ported from avd, VORGABEN DIREKTIONEN ÄNDERN. The picked topic and
// every topic after it come in turn like his INC as& chain, EXIT ends it
void MainWindow::vorgaben_direktionen() {
  const QString exit = tr("EXIT");
  //RR GEWÜNSCHTES THEMA ANKLICKEN !
  const int first = ChoiceDialog::ask(
      this, tr("GEWÜNSCHTES THEMA ANKLICKEN ! "), {},
      {tr("BEI MÜNCHNER RHYTHMENLEHRE auch SEXTIL berücksichtigen ?"),
       tr("Bei mehreren Bildschirmen bis zum ENDE GEHEN ? "), tr("GROß - oder KLEIN - Symbole in TABELLEN ? "),
       tr(" INVERTIERUNG von MA,SA,UR,NE,PL und ( oder ) EINFÄRBUNG der ASPEKTE in TABELLEN ? "),
       tr("DIREKTIONEN MIT oder OHNE ZWISCHENHÄUSERN ?"),
       tr("EKLIPTIKALE DIREKTIONEN MIT oder OHNE KARDINALPUNKTE  ?"),
       tr("SIGNIFIKATOR bei PRIMÄR-DIR. MIT oder OHNE BREITE ?"),
       tr("PROMISSOR    bei PRIMÄR-DIR. MIT oder OHNE BREITE ?"),
       tr("HALBSUMMEN ( Ekliptikal ) in TABELLEN mit ANZEIGEN ?"),
       tr("ORDINATEN-RICHTUNG bei LINEAR-GRAPHIKEN: Nach OBEN oder nach UNTEN ?"), exit});
  const QString primary = tr("VORGABEN für PRIMÄR-DIREKTION:");
  for (int topic = first; topic >= 0 && topic <= 9; ++topic) {
    int b = -1;
    switch (topic) {
      case 0:
        b = ChoiceDialog::ask(this, "HORCOM", {tr("BEI MÜNCHNER RHYTHMENLEHRE"), tr("AUCH SEXTIL BERÜCKSICHTIGEN ?")},
                              {tr(" NEIN "), tr("JA"), exit}, konsta_.sext != 0.0 ? 1 : 0);
        if (b == 0 || b == 1) {
          // sext! = TRUE, stored like his -1
          konsta_.sext = b == 1 ? -1.0 : 0.0;
        }
        break;
      case 1:
        // the answer is kept for his file, the tables of this edition
        // always run to the end and scroll
        b = ChoiceDialog::ask(this, "HORCOM",
                              {tr("BEI MEHREREN UNSORTIERTEN BILDSCHIRMEN"), tr("BIS ZU ENDE RECHNEN ?"),
                               tr("oder erst"), tr("BILDSCHIRM FüR BILDSCHIRM anschauen ?"),
                               tr("( HINWEIS: Die TABELLEN rechnen hier stets bis zum ENDE und ROLLEN )")},
                              {tr("BIS ENDE"), tr("EINZEL"), exit}, konsta_.plusl ? 0 : 1);
        if (b == 0 || b == 1) {
          konsta_.plusl = b == 0;
        }
        break;
      case 2:
        b = ChoiceDialog::ask(this, "HORCOM", {tr("KLEINSYMBOLE in TABELLEN ?")}, {tr("KLEIN"), tr("GROß"), exit},
                              konsta_.klsyt ? 0 : 1);
        if (b == 0 || b == 1) {
          konsta_.klsyt = b == 0;
        }
        break;
      case 3:
        //RR KENNZEICHNUNG bestimmter PLANETEN bzw. ASPEKTE in TABELLEN ?
        b = ChoiceDialog::ask(this, tr("KENNZEICHNUNG bestimmter PLANETEN bzw. ASPEKTE in TABELLEN ?"), {},
                              {tr("KEINERLEI KENNZEICHNUNG !"), tr("NUR SYMBOLE von MA,SA,UR,NE,PL INVERTIEREN !"),
                               tr("NUR ASPEKTE EINFÄRBEN : HARTE in ROT,HARMONISCHE in GRÜN !"),
                               tr("SOWHL SYMBOLE INVERTIEREN als auch ASPEKTE EINFÄRBEN !"), exit},
                              std::clamp(konsta_.plinv, 0, 3));
        if (b >= 0 && b <= 3) {
          konsta_.plinv = b;
        }
        b = b == 4 ? 2 : (b < 0 ? -1 : 0);
        break;
      case 4:
        b = ChoiceDialog::ask(this, "HORCOM", {tr("DIREKTIONEN MIT/OHNE"), QString(), tr("ZWISCHENHÄUSER ?")},
                              {tr("MIT"), tr("OHNE"), exit}, konsta_.zwhd ? 0 : 1);
        if (b == 0 || b == 1) {
          konsta_.zwhd = b == 0;
        }
        break;
      case 5:
        b = ChoiceDialog::ask(this, "HORCOM", {tr("MIT KARDINAL-PUNKTEN ?"), QString(), tr("0 AR / 0 CN / 0 LI / 0 CP")},
                              {tr("MIT"), tr("OHNE"), exit}, konsta_.kard ? 0 : 1);
        if (b == 0 || b == 1) {
          konsta_.kard = b == 0;
        }
        break;
      case 6:
        b = ChoiceDialog::ask(this, "HORCOM", {primary, QString(), tr("SIGNIFIKAT. MIT/OHNE Breite ?")},
                              {tr("MIT"), tr("OHNE"), exit}, konsta_.bres == 1 ? 0 : 1);
        if (b == 0 || b == 1) {
          konsta_.bres = b == 0 ? 1 : 0;
        }
        break;
      case 7:
        // PROMISSOREN mit Breite only after a significator with latitude
        if (konsta_.bres == 1) {
          b = ChoiceDialog::ask(this, "HORCOM", {primary, QString(), tr("PROMISSOREN mit Breite ?")},
                                {tr("MIT"), tr("OHNE"), exit}, konsta_.brep == 1 ? 0 : 1);
          if (b == 0 || b == 1) {
            konsta_.brep = b == 0 ? 1 : 0;
          }
        } else {
          konsta_.brep = 0;
          b = 0;
        }
        break;
      case 8:
        b = ChoiceDialog::ask(this, "HORCOM",
                              {primary, tr("HALBSUMMEN in TABELLEN ANZEIGEN ?"), tr("Nur bei EKLIPTIKALEN Methoden !"),
                               tr("SEHR VIELE AUSLÖSUNGEN !!!")},
                              {tr("NICHT ANZEIGEN"), tr("ALLE ANZEIGEN"),
                               tr(" NUR DIE MIT 3 UNTERSCHIEDLICHEN Faktoren ANZEIGEN"), exit},
                              std::clamp(konsta_.halbs_dir, 0, 2));
        if (b >= 0 && b <= 2) {
          konsta_.halbs_dir = b;
        }
        b = b == 3 ? 2 : (b < 0 ? -1 : 0);
        break;
      case 9:
        b = ChoiceDialog::ask(this, "HORCOM",
                              {tr("ORDINATEN-RICHTUNG bei LINEAR-GRAPHIKEN ?"), tr("Nach OBEN oder nach UNTEN ?")},
                              {tr("Nach OBEN POSITIV"), tr("Nach UNTEN POSITIV ( R.EBERTIN ) "), exit},
                              konsta_.lin_inv ? 1 : 0);
        if (b == 0 || b == 1) {
          konsta_.lin_inv = b == 1;
        }
        break;
      default:
        break;
    }
    // EXIT or ESC ends the chain like his GOTO avde
    if (b < 0 || b == 2) {
      break;
    }
  }
  // @kon_dsp at the end of avd
  persist_konsta();
}

// ported from avd_lin, the orientation of the graph and the signs, both
// kept with the Vorgaben like his param_sp
bool MainWindow::linear_orientation() {
  // d& = 1 with linie!, 2 with gitter!, else 3
  const int d = konsta_.linie ? 0 : (konsta_.gitter ? 1 : 2);
  const int c = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("ORIENTIERUNG in LINEAR - GRAPHIK")},
                                  {tr("'TREFFER - LINIEN' MARKIEREN ?"), tr("FESTES LINIEN - GITTER ?"),
                                   tr("KEINE SENKRECHTEN LINIEN")},
                                  d);
  if (c < 0) {
    return false;
  }
  konsta_.linie = c == 0;
  konsta_.gitter = c == 1;
  // l$ = "ZEICHEN", "In LINEAR - GRAPHIK", l$ + " MARKIEREN ?"
  const int z = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("In LINEAR - GRAPHIK"), tr("ZEICHEN MARKIEREN ?")},
                                  {tr("MIT ZEICHEN"), tr("OHNE ZEICHEN"), tr("EXIT")}, konsta_.zeichen ? 0 : 1);
  if (z < 0 || z == 2) {
    persist_konsta();
    return false;
  }
  konsta_.zeichen = z == 0;
  // @param_sp
  persist_konsta();
  return true;
}

namespace {

// his wart over the linear screen, Space goes on, ESC ends, the mouse
// buttons drag a blue or red reading line like dragline_y
class LinearView final : public QDialog {
 public:
  explicit LinearView(QWidget* parent) : QDialog(parent) {
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    canvas = new WheelWidget(this);
    v->addWidget(canvas);
    canvas->installEventFilter(new LambdaFilter(
        [this](QEvent* e) {
          if (e->type() == QEvent::MouseButtonRelease) {
            reading_line(*static_cast<QMouseEvent*>(e));
          }
          return false;
        },
        this));
  }
  WheelWidget* canvas = nullptr;
  DisplayList base;

 protected:
  void keyPressEvent(QKeyEvent* e) override {
    if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
      accept();
      return;
    }
    QDialog::keyPressEvent(e);
  }

 private:
  // IF MOUSEY > 40 AND MOUSEY < _Y - 20, blue left, red right, the canvas
  // scales the 640 wide sheet into the widget
  void reading_line(const QMouseEvent& m) {
    const double scale = std::min(canvas->width() / base.width, canvas->height() / base.height);
    const double ox = (canvas->width() - base.width * scale) / 2.0;
    const double x = (m.position().x() - ox) / scale;
    if (x > kLinearFrameLeft && x < kLinearFrameRight) {
      Primitive p;
      p.kind = Primitive::Kind::kLine;
      p.x1 = x;
      p.x2 = x;
      p.y1 = kLinearBandTop;
      p.y2 = kLinearBandBottom;
      p.color = m.button() == Qt::RightButton ? kMarkRed : 0x0000FF;
      base.items.push_back(p);
      canvas->set_plain_list(base);
    }
  }
};

}  // namespace

// the run of the LINEAR-GRAPHIK branch of a18asw, his a18eing with lin!,
// the FEIN or DICKER box, the graph and his loop over the intervals, and
// the LINEAR-GRAPHIK branch of mund
void MainWindow::linear_run(A18Mode mode) {
  if (!last_chart_) {
    return;
  }
  const bool mundane = mode == A18Mode::kMundane;
  const bool dated = mode == A18Mode::kTransits || mundane;
  // CLR hrg!, the direction runs geocentric
  if (!dated) {
    force_geocentric();
  }
  auto a = a18_questions(mode, true);
  if (!a) {
    return;
  }
  SearchContext ctx = make_context();
  QString footer;
  // IF (mas! OR par = 1) && jdend - jdbeg < 150, @ereig_ort, mund asks
  // only with the moon
  const bool ask_place = mundane ? a->moon : a->jd_to - a->jd_from < 150.0;
  if (dated && ask_place && !event_place_for(ctx, footer, mundane)) {
    return;
  }
  // IF prenbl& : moda& = @druck_graph_ein, his TRANSITE, SEKUNDÄR and
  // BOGEN entries, mund draws on the screen only
  const int item = mode == A18Mode::kSecondary
                       ? menu_item::kSecondary
                       : (mode == A18Mode::kSunArc || mode == A18Mode::kMoonArc
                              ? menu_item::kArcDirection
                              : (mundane ? menu_item::kMundane : menu_item::kTransits));
  const int moda = mundane ? kOutputScreen : ask_graphic_output(item, true);
  if (moda == kOutputNone) {
    return;
  }
  // bildsdick& 1 FEIN, 2 DICKER or MITTEL, 3 STARK
  int width = 1;
  std::unique_ptr<QPrinter> printer;
  if (moda == kOutputScreen) {
    //RR FEINE oder DICKERE LINIEN ZEICHNEN ?
    const int thick =
        mundane ? ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("FEINE oder DICKERE Linien zeichnen ?")},
                                    {tr("FEIN"), tr("DICKER")}, 0)
                : ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("FEINE oder DICKERE LINIEN ZEICHNEN ?")},
                                    {tr("FEIN"), tr("DICKER")}, 0);
    if (thick < 0) {
      return;
    }
    width = thick + 1;
  } else {
    // alertbox(1,"FEINE oder DICKERE LINIEN ZEICHNEN ?","","","",1,"FEIN","MITTEL","STARK",ac$)
    const int rr = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("FEINE oder DICKERE LINIEN ZEICHNEN ?")},
                                     {tr("FEIN"), tr("MITTEL", "line width"), tr("STARK"), tr("ABBRUCH")}, 0);
    if (rr < 0 || rr == 3) {
      return;
    }
    width = rr + 1;
    printer = std::make_unique<QPrinter>(QPrinter::HighResolution);
    if (!prepare_printer(this, *printer, moda == kOutputA4 ? PrintPage::kLinearA4 : PrintPage::kGraphicA5,
                         konsta_.halbs != 0)) {
      printer_failed(this);
      return;
    }
  }
  const Chart radix = radix_chart();
  const double tja = radix.ta.tropical_year_days;
  QString title;
  LinearKind kind = LinearKind::kTransits;
  switch (mode) {
    case A18Mode::kSecondary:
      title = tr(" SEKUNDÄR-Direktion ");
      kind = LinearKind::kSecondary;
      break;
    case A18Mode::kSunArc:
      title = tr(" SONNEN-BOGEN-DIR.");
      kind = LinearKind::kSunArc;
      break;
    case A18Mode::kMoonArc:
      title = tr(" MOND-BOGEN-DIR.");
      kind = LinearKind::kMoonArc;
      break;
    case A18Mode::kMundane:
      title = tr(" Ekliptikale Mundan-Aspekte ");
      kind = LinearKind::kMundane;
      break;
    default:
      title = tr(" TRANSITE ");
      break;
  }
  for (;;) {
    LinearOptions opt;
    opt.kind = kind;
    opt.base_angle_deg = a->base_deg;
    opt.jd_from_ut = a->jd_from;
    opt.jd_to_ut = a->jd_to;
    opt.from_years = a->from_years;
    opt.to_years = a->to_years;
    opt.downward = konsta_.lin_inv;
    opt.signs = konsta_.zeichen;
    opt.with_houses = konsta_.zwhd;
    opt.hit_lines = konsta_.linie;
    opt.grid = konsta_.gitter;
    opt.line_width = static_cast<double>(width);
    opt.first_slot = a->first_slot;
    opt.chosen = a->chosen;
    opt.moon = a->moon;
    // a18kopf, di$ and a18ko1$, "Name : " + na$ + " |" + sol$, "| Ort: " + go$,
    // mund! names no record
    opt.title = title.toStdString();
    if (!mundane) {
      const ClassicSheetText txt = classic_sheet_text();
      opt.record =
          (QString::fromStdString(txt.date).trimmed() + " | " + QString::fromStdString(txt.ut).trimmed()).toStdString();
      opt.name =
          tr("Name : %1 | %2 ").arg(QString::fromStdString(txt.name).trimmed(), rhythm_chart_label()).toStdString();
      opt.place = tr("| Ort: %1").arg(QString::fromStdString(txt.place).trimmed()).toStdString();
    }
    if (dated) {
      opt.start = tr("Ab %1").arg(datum3_text(calendar_date(a->jd_from, ctx.settings.calendar))).toStdString();
    }
    const int nbl = linear_pixels_per_step(opt);
    DisplayList page;
    // the guard lives while the hits and the curves are made
    auto guard = std::make_unique<SweepGuard>(this, tr("Bitte warten ! Rechenzeit erforderlich !"), QString());
    const SweepProgress report = guard->callback();
    // the exact hits of the window from the sweep of the table branch,
    // lin! takes no stationary touches
    TransitScan scan = a18_scan(*a);
    scan.station_touches = false;
    scan.progress = [&report](double f) { return report(0.5 * f); };
    const auto to_hit = [](const TransitEvent& e, double jd) {
      LinearHit h;
      h.jd_ut = jd;
      h.running = e.transiting;
      h.radix = e.radix;
      h.cusp = e.cusp;
      h.angle_deg = e.angle_deg;
      return h;
    };
    if (mundane) {
      MundaneScan mscan;
      mscan.jd_from_ut = a->jd_from;
      mscan.jd_to_ut = a->jd_to;
      mscan.base_angle_deg = a->base_deg;
      mscan.grid = a->grid;
      mscan.first_slot = a->first_slot;
      mscan.chosen = a->chosen;
      mscan.moon = a->moon;
      mscan.progress = scan.progress;
      for (const MundaneAspect& m : scan_mundane_aspects(mscan, ctx)) {
        LinearHit h;
        h.jd_ut = m.jd_ut;
        h.running = m.second;
        h.radix = m.first;
        h.angle_deg = m.angle_deg;
        h.lon = m.first_lon;
        opt.hits.push_back(h);
      }
    } else if (dated) {
      const ChartSettings& s = ctx.settings;
      for (const TransitEvent& e : scan_transits(radix, scan, ctx)) {
        // a181 draws no transit of the moon on the coarse axes under a
        // small base, none of the true node or the true black moon
        const bool quiet = (e.transiting == body::kMoon && !s.heliocentric && nbl < 16 && a->base_deg < 180.0) ||
                           (s.true_node && e.transiting == body::kNodeAsc) ||
                           (s.true_apogee && e.transiting == body::kApogee);
        if (e.radix2 == 0 && !quiet) {
          opt.hits.push_back(to_hit(e, e.jd_ut));
        }
      }
    } else {
      scan.jd_from_ut = radix.jd_ut + a->from_years * tja;
      scan.jd_to_ut = radix.jd_ut + a->to_years * tja;
      const std::vector<DirectedEvent> events =
          kind == LinearKind::kSecondary ? secondary_direction_events(radix, scan, ctx)
                                         : arc_direction_events(radix, kind == LinearKind::kMoonArc, scan, ctx);
      for (const DirectedEvent& d : events) {
        if (d.event.radix2 == 0) {
          opt.hits.push_back(to_hit(d.event, d.jd_life_ut));
        }
      }
    }
    // lin! && l$ = CHR$(27), GOTO a180e, ESC ends the drawing at once
    if (guard->cancelled()) {
      return;
    }
    opt.progress = [&report](double f) { return report(0.5 + 0.5 * f); };
    page = build_linear_graph(radix, opt, ctx);
    if (guard->cancelled()) {
      return;
    }
    guard.reset();
    if (printer) {
      // CASE 2 VIEWPORT gdxp& / 20,gdyp& / 90, CASE 3 Querformat gdxp& / 20,gdyp& / 20 times 1.4
      QPainter p(printer.get());
      if (!p.isActive()) {
        printer_failed(this);
        return;
      }
      paint_robert_page(p, page,
                        moda == kOutputA4
                            ? robert_page_rect(*printer, page_margin::kLeft, page_margin::kLinearTop, page_margin::kFullFactor)
                            : robert_page_rect(*printer, page_margin::kLeft, page_margin::kTop, 1.0));
      p.end();
      // merk_grph! = -1, expr! = -1, GOTO a18asw
      return;
    }
    LinearView view(this);
    mark_output(&view, item);
    view.base = std::move(page);
    view.canvas->set_plain_list(view.base);
    // lg$ = "LINEAR-GRAPHIK"
    view.setWindowTitle(tr("LINEAR-GRAPHIK") + " |" + title);
    view.resize(size());
    if (view.exec() != QDialog::Accepted) {
      return;
    }
    //RR WEITERE AUSWERTUNG anschließen ?, NÄCHSTES / VORHERGEHENDES Intervall
    const QString more = mundane ? tr("Weitere Auswertung anschließen ?") : tr("WEITERE AUSWERTUNG anschließen ?");
    const int re = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {more},
                                     {tr("NÄCHSTES Intervall"), tr("VORHERGEHENDES Intervall"), tr("BEENDEN")}, 0);
    if (re != 0 && re != 1) {
      return;
    }
    step_linear_window(*a, re == 0, ctx.settings.calendar);
  }
}

// ported from a18asw1. His step back of one month landed on jdbeg - 31
// days and kept that month, from the first of March it went to January.
// The port opens every window on the first of the month the step names,
// at 0 h like his ho = 0 and mi = 0
void MainWindow::step_linear_window(A18Answers& a, bool forward, Calendar cal) {
  if (a.jd_to > a.jd_from) {
    const double span = a.jd_to - a.jd_from;
    // djd = 31, 125 or 488, one, four or sixteen months
    const int months = span < 60.0 ? 1 : (span < 200.0 ? 4 : 16);
    const int per_year = static_cast<int>(kMonthsPerYear);
    const CalendarDate d = calendar_date(a.jd_from + 0.5, cal);
    int m = d.month - 1 + (forward ? months : -months);
    const int y = d.year + (m >= 0 ? m / per_year : -((per_year - 1 - m) / per_year));
    m = ((m % per_year) + per_year) % per_year;
    a.jd_from = julian_day({1, m + 1, y, 0.0, 0.0}, cal);
    a.jd_to = a.jd_from + span;
    return;
  }
  // lja = lje and lje = lja + dlj, back by two spans
  const double dlj = a.to_years - a.from_years;
  if (forward) {
    a.from_years = a.to_years;
  } else {
    a.from_years = a.to_years - 2.0 * dlj;
  }
  a.to_years = a.from_years + dlj;
}

}  // namespace horcom
