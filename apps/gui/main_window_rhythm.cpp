// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDialog>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <set>

#include "banner.hpp"
#include "choice_dialog.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/mundane.hpp"
#include "horcom/chart/signs.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/items.hpp"
#include "horcom/render/rhythm_panel.hpp"
#include "main_window.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"
#include "sheet_view.hpp"
#include "theme.hpp"
#include "zodiac_cells.hpp"

namespace horcom {

namespace {

// his sen of VAL(LEFT$(septar$,3)), erg_rad set sen = 1 so a promoted
// SEPTAR runs from its own moment like a radix
int septar_sen(const QString& label) {
  if (label.endsWith(" ALS RADIX")) {
    return 1;
  }
  return std::max(1, label.section('.', 0, 0).toInt());
}

// his dbr& of the AUSWERTE-MODUS
constexpr int kGraph = 1;
constexpr int kTable = 2;
constexpr int kList = 3;

// @putbm(220,0,naf&,3), the name block beside the strip
constexpr double kRhythmNameX = 222.0;
// his SONSTIGE period, no bound in the inputbox, the port keeps it sane
constexpr double kMaxPeriod = 120.0;
// the oldest year of life the Septar box takes
constexpr double kMaxLifeYears = 150.0;

QString slot_tag(int slot) {
  // plnm names slot zero SP in the Rhythmenlehre
  if (slot == body::kFixpunkt) {
    return QStringLiteral("SP");
  }
  const std::string_view n = body::kName[static_cast<std::size_t>(slot)];
  return QString::fromUtf8(n.data(), static_cast<qsizetype>(n.size()));
}

// his grze forms gz0$ "%2d°TAG%2d'" and, spaced, gz1$ "%2d° TAG %2d'",
// the minutes rounded with a carry into the next sign
QString grze_text(double lon_rad, bool spaced) {
  const ZodiacSplit z = split_zodiac(lon_rad, false);
  return QString::asprintf(spaced ? "%2d\xC2\xB0 %s %2d'" : "%2d\xC2\xB0%s%2d'", z.deg, kSignTag[z.sign], z.min);
}

// the kind letters of his al$
QString kind_text(RhythmKind k) {
  switch (k) {
    case RhythmKind::kDirect: return QStringLiteral("D");
    case RhythmKind::kRuler: return QStringLiteral("P");
    case RhythmKind::kRuler2: return QStringLiteral("P2");
    case RhythmKind::kRuler3: return QStringLiteral("P3");
    case RhythmKind::kAspect: return QStringLiteral("A");
    case RhythmKind::kMirror: return QStringLiteral("S");
  }
  return {};
}

// his ausw_pl_hs captions of the extra bodies, the dbr& variant without
// the Glückspunkt
struct ExtraCaption {
  int slot;
  const char* caption;
};

constexpr ExtraCaption kExtraCaptions[] = {
    {body::kApogee, QT_TRANSLATE_NOOP("horcom::MainWindow", "  SCHWARZER MOND  ")},
    {body::kChiron, QT_TRANSLATE_NOOP("horcom::MainWindow", " CHIRON         a=13.61  e=0.38  i=6.94°  T=50 Jahre ")},
    {body::kTranspluto, QT_TRANSLATE_NOOP("horcom::MainWindow", "    TRANSPLUTO  HYPOTHETISCH")},
    {body::kCeres, QT_TRANSLATE_NOOP("horcom::MainWindow", " CERES")},
    {body::kPallas, QT_TRANSLATE_NOOP("horcom::MainWindow", " PALLAS")},
    {body::kJuno, QT_TRANSLATE_NOOP("horcom::MainWindow", " JUNO")},
    {body::kVesta, QT_TRANSLATE_NOOP("horcom::MainWindow", " VESTA")},
    {body::kCupido, QT_TRANSLATE_NOOP("horcom::MainWindow", "    CUPIDO    HYPOTHETISCH")},
    {body::kHades, QT_TRANSLATE_NOOP("horcom::MainWindow", "    HADES     HYP.")},
    {body::kZeus, QT_TRANSLATE_NOOP("horcom::MainWindow", "    ZEUS      HYP.")},
    {body::kKronos, QT_TRANSLATE_NOOP("horcom::MainWindow", "    KRONOS    HYP.")},
    {body::kApollon, QT_TRANSLATE_NOOP("horcom::MainWindow", "    APOLLON   HYP.")},
    {body::kAdmetos, QT_TRANSLATE_NOOP("horcom::MainWindow", "    ADMETOS   HYP.")},
    {body::kVulkanus, QT_TRANSLATE_NOOP("horcom::MainWindow", "    VULKANUS  HYP.")},
    {body::kPoseidon, QT_TRANSLATE_NOOP("horcom::MainWindow", "    POSEIDON  HYP.")},
    {body::kQuaoar, QT_TRANSLATE_NOOP("horcom::MainWindow", " QUAOAR         a=43.25 e=0.035 i=  7.99°   T=284 Jr.")},
    {body::kHalley, QT_TRANSLATE_NOOP("horcom::MainWindow", " KOMET HALLEY   a=17.94 e=0.97  i=162.24°   T= 76 Jr.")},
    {body::kPholus, QT_TRANSLATE_NOOP("horcom::MainWindow", " PHOLUS         a=20.23 e=0.57  i= 24.70°   T= 91 Jr.")},
    {body::kDamokles, QT_TRANSLATE_NOOP("horcom::MainWindow", " DAMOKLES       a=11.82 e=0.87  i= 61.84°   T= 41 Jr.")},
    {body::kNessus, QT_TRANSLATE_NOOP("horcom::MainWindow", " NESSUS         a=24.46 e=0.52  i= 15.66°   T=121 Jr.")},
    {body::kXena, QT_TRANSLATE_NOOP("horcom::MainWindow", " XENA           a=67.66 e=0.44  i= 44.12°   T=557 Jr.")},
};

// the rows of his HAUS NR. block under the bodies, each sends the
// definition back to the degree
constexpr int kHouseRowFirst = 100;

}  // namespace

void MainWindow::rhythm() {
  rhythm_run(0);
}

// the tester's own entry, the a17 chain straight into GRAD-DATUM-LISTE
void MainWindow::degree_date_list() {
  rhythm_run(kList);
}

QString MainWindow::rhythm_chart_label() const {
  // the SEPTAR of a16 names its sol$(2,ze) before the chart exists
  if (!rhythm_label_.isEmpty()) {
    return rhythm_label_;
  }
  // sol$(od,ze) = "COMBIN"
  if (!combin_name1_.empty()) {
    return QStringLiteral("COMBIN");
  }
  if (active_is_solar_ && active_solar_ >= 0 && active_solar_ < static_cast<int>(solar_labels_.size()) &&
      !solar_labels_[static_cast<std::size_t>(active_solar_)].isEmpty()) {
    return solar_labels_[static_cast<std::size_t>(active_solar_)];
  }
  if (!active_is_solar_ && active_slot_ >= 0 && active_slot_ < 5) {
    return radix_label(active_slot_);
  }
  // rd$ = "RADIX"
  return QStringLiteral("RADIX");
}

// ported from a17sol
QStringList MainWindow::rhythm_sol_lines() const {
  QStringList out;
  // " " + asl$ + "en nach W." + db$ + m$, m$ = " ( " + mu$ + " )"
  QString m;
  if (mundane_frame_) {
    m = " ( " + tr("Mundan") + " )";
  }
  out << " " + tr("Auslösungen nach W.DÖBEREINER") + m;
  out << "  " + rhythm_chart_label() + " ";
  if (last_chart_) {
    const QString unit = rhythm_unit_.isEmpty() ? QString() : (rhythm_months_ ? tr(" Monate") : tr(" Jahre"));
    const CalendarDate d = calendar_date(last_chart_->jd_ut, current_settings().calendar);
    out << " " + tr("Datum") + ": " + datum3_text(d) + "  " + tr("Periode") + " : " + rhythm_phase_ + unit;
  }
  // his q$ = RIGHT$(iv$,2) always read "R ", so the span of a Septar
  // never showed, and its start lacked the unit factor fa&
  const QString label = rhythm_chart_label();
  // a Septar still without its number has no span yet
  if (label.contains("SEPTAR") && label.contains('.') && !rhythm_phase_.isEmpty()) {
    const RhythmOptions o = rhythm_options();
    const int sen = septar_sen(label);
    const double from = septar_offset(sen, o);
    const double to = from + o.phase_years * (o.months ? 1.0 : kMonthsPerYear);
    out << " " + tr("%1 Bis %2  LEBENS-Jahre").arg(from, 0, 'g', 4).arg(to, 0, 'g', 4);
  }
  return out;
}

// ported from a17eing11
bool MainWindow::rhythm_unit_question(bool septar, bool check) {
  for (;;) {
    // w& = 2 for months and septars, 1 for the radix
    int w = 0;
    if (eingm_ == QLatin1String(" Monate")) {
      w = 1;
    }
    // sol$(od,ze) reads SEPTAR while a Septar is cast
    if (septar) {
      w = 1;
    } else if (rhythm_chart_label() == QLatin1String("RADIX")) {
      w = 0;
    }
    QStringList info = rhythm_sol_lines();
    info << QString() << tr("ZEIT-EINHEIT MARKIEREN!") << QString() << tr("NORMAL ist 'JAHR'")
         << tr("Bei 'SEPTAREN' i,a. 'MONAT'");
    const int b = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), info, {tr("JAHR"), tr("MONAT")}, w);
    if (b < 0) {
      return false;
    }
    rhythm_months_ = b == 1;
    // mon$ = "E", jahre$ = " Monate" bzw. " Jahre"
    rhythm_unit_ = rhythm_months_ ? QStringLiteral(" Monate") : QStringLiteral(" Jahre");
    //RR PARAMETER RICHTIG ?
    if (!eingm_.isEmpty() && check && eingm_ != rhythm_unit_) {
      const int ok = ChoiceDialog::ask(
          this, tr("AUSWAHL"),
          {tr("PARAMETER RICHTIG ?"), QString(), tr("ZEITEINHEIT : ") + (rhythm_months_ ? tr(" Monate") : tr(" Jahre"))},
          {tr("OK"), tr("ÄNDERN")}, 0);
      if (ok < 0) {
        return false;
      }
      if (ok == 1) {
        continue;
      }
    }
    eingm_ = rhythm_unit_;
    return true;
  }
}

// ported from a17eing12
bool MainWindow::rhythm_period_question(bool check) {
  static constexpr double kPeriods[7] = {1.0, 3.0, 4.0, 5.0, 6.0, 7.0, 10.0};
  for (;;) {
    // ze& from VAL(phas$), SIEBEN when none fits
    const double vp = rhythm_phase_.toDouble();
    int def = 5;
    for (int i = 0; i < 7; ++i) {
      if (vp == kPeriods[i]) {
        def = i;
      }
    }
    const int es = ChoiceDialog::ask(this, tr(" Periode PRO HAUS ? "), {},
                                     {tr("EINS"), tr("DREI"), tr("VIER"), tr("FÜNF"), tr("SECHS"), tr("SIEBEN"),
                                      tr("ZEHN"), tr("SONSTIGE"), tr("ABBRUCH")},
                                     def);
    // his second CASE 8 never fired, ABBRUCH went on with the old period
    if (es < 0 || es == 8) {
      return false;
    }
    if (es < 7) {
      rhythm_phase_ = QString::number(kPeriods[es]);
    } else {
      // phas$ = @inputbox$(160,eaz$,pi$ + " PRO HAUS IN " + jahre$ + "n ! ",phas$)
      const auto v = ask_number(this, tr("ZAHLEN-Eingabe !"),
                                rhythm_months_ ? tr("Periode PRO HAUS IN  Monaten ! ") : tr("Periode PRO HAUS IN  Jahren ! "),
                                -kMaxPeriod, kMaxPeriod, vp == 0.0 ? std::numeric_limits<double>::quiet_NaN() : vp, 2);
      if (!v) {
        return false;
      }
      // a period of zero has no walk
      if (*v == 0.0) {
        continue;
      }
      rhythm_phase_ = QString::number(*v);
    }
    // PARAMETER RICHTIG ?, be$ = pi$ + "/Haus : "
    if (!eingp_.isEmpty() && check && rhythm_phase_ != eingp_) {
      const int ok = ChoiceDialog::ask(this, tr("AUSWAHL"),
                                       {tr("PARAMETER RICHTIG ?"), QString(), tr("Periode/Haus : ") + rhythm_phase_},
                                       {tr("OK"), tr("ÄNDERN")}, 0);
      if (ok < 0) {
        return false;
      }
      if (ok == 1) {
        continue;
      }
    }
    eingp_ = rhythm_phase_;
    return true;
  }
}

RhythmOptions MainWindow::rhythm_options() const {
  RhythmOptions o;
  o.phase_years = rhythm_phase_.toDouble();
  o.months = rhythm_months_;
  //RR BEI MÜNCHNER RHYTHMENLEHRE AUCH SEXTIL BERÜCKSICHTIGEN ?
  o.sextile = konsta_.sext != 0.0;
  // kard!, MIT KARDINAL-PUNKTEN
  o.cardinals = konsta_.kard;
  o.mundane = mundane_frame_ && !current_settings().heliocentric;
  o.classic_rulers = alt_rulers_;
  // apog!, the apogee among the chosen planets
  o.apogee_opposite = last_chart_ && last_chart_->b[body::kApogee].present && last_chart_->b[body::kApogee].valid;
  return o;
}

// ported from a174init, the sol$ cases. The other derived charts of the
// rewrite run from their own moment like his Lunar
RhythmClock MainWindow::rhythm_clock(const RhythmOptions& opt) const {
  RhythmClock c;
  if (!last_chart_) {
    return c;
  }
  c.tja = last_chart_->ta.tropical_year_days;
  c.base_jd = last_chart_->jd_ut;
  const QString label = rhythm_chart_label();
  if (label.contains("SEPTAR")) {
    // sen = VAL(LEFT$(septar$(od,ze),3)), jd = jd(1,ze) + sn * tja + l
    const int sen = septar_sen(label);
    c.sn = septar_offset(sen, opt);
    c.base_jd = radix_chart().jd_ut + c.sn * c.tja;
  } else if (label.endsWith(QLatin1String(".SOLAR"))) {
    // sn = jas&(2,ze) - ja&(1,ze), jd = jd(2,ze) + l, the years his
    // solnummer put before the SOLAR
    c.sn = label.section('.', 0, 0).toInt();
  }
  return c;
}

// ported from a17sonderpkt
bool MainWindow::rhythm_special_question(bool septar, RhythmOptions& opt, const RhythmClock& clock) {
  // b$ = "SONDERPUNKT "
  const QString b = tr("SONDERPUNKT ");
  const bool stored = !konsta_.fixpunkt_rh.empty() && (rhythm_lpkt_ != 0.0 || konsta_.lpktg);
  const double stored_deg = QString::fromStdString(konsta_.fixpunkt_rh).trimmed().toDouble();
  // a degree defined point projects into the mundane frame like lpkt,
  // a date defined one was found on the mundane houses already
  const auto in_frame = [this, &opt](double rad, bool degree_defined) {
    if (opt.mundane && degree_defined && last_chart_) {
      return mundane_longitude(norm_rad(rad), kEps, last_chart_->smo.ekls, last_chart_->armc_deg * kDegToRad,
                               current_input().lat_deg);
    }
    return norm_rad(rad);
  };
  QStringList rows;
  std::vector<int> disabled;
  int def = 0;
  const QString gz0 = grze_text(stored_deg * kDegToRad, false);
  const QString by_date = septar ? QStringLiteral(" ") : b + tr("INDIREKT über DATUM DEFINIEREN");
  if (stored) {
    rows = {" " + gz0 + tr(" BEIBEHALTEN"), b + tr("als EKLIPTIK-GRAD NEU DEFINIEREN"), by_date,
            b + gz0 + tr(" LÖSCHEN"), QStringLiteral(" "), tr("ABBRUCH")};
    disabled = {4};
    def = 0;
  } else {
    rows = {QStringLiteral(" "), b + tr("als EKLIPTIK-GRAD NEU DEFINIEREN"), by_date, QStringLiteral(" "),
            tr("KEIN ") + b, tr("ABBRUCH")};
    disabled = {0, 3};
    def = 4;
  }
  if (septar) {
    disabled.push_back(2);
  }
  const int es = ChoiceDialog::ask_with_disabled(this, b + tr("( FIXPUNKT ) WÄHLEN ?"), rhythm_sol_lines(), rows, def,
                                                 disabled);
  switch (es) {
    case 0:
      opt.special = in_frame(stored_deg * kDegToRad, konsta_.lpktg);
      return true;
    case 1: {
      // " SONDERPUNKT als Ekliptik-Grad definieren !", input_grmise_zod(ekl$ + eg$ + " !")
      const auto wh = ask_zodiac_position(this, tr("Ekliptikale Länge Eingeben !"));
      if (!wh) {
        return false;
      }
      konsta_.lpktg = true;
      konsta_.fixpunkt_rh = QString::asprintf("%8.4f", norm_rad(*wh) * kRadToDeg).toStdString();
      persist_konsta();
      opt.special = in_frame(*wh, true);
      return true;
    }
    case 2: {
      // a37dat(72,"","DATUM-ZEIT-EINGABE"), lpkt = (jd - jd(od,ze)) / tja
      const QDate today = QDate::currentDate();
      const auto d = ask_date(this, tr("DATUM-ZEIT-EINGABE"), {today.day(), today.month(), today.year(), 0.0, 0.0},
                              tr("GREENWICH-ZEIT"), {" " + b + tr("INDIREKT über Datum definieren !")});
      if (!d) {
        return false;
      }
      // the inverse of rhythm_jd, his lpk subtracted sn a second time
      rhythm_lpkt_ = rhythm_years(clock, julian_day(*d, current_settings().calendar));
      konsta_.lpktg = false;
      const double deg = last_chart_ ? degree_at_age(*last_chart_, opt, rhythm_lpkt_) : -1.0;
      // IF l& => 0 && l& < 12
      if (deg >= 0.0) {
        opt.special = deg;
        konsta_.fixpunkt_rh = QString::asprintf("%8.4f", deg * kRadToDeg).toStdString();
      } else {
        opt.special = -1.0;
      }
      return true;
    }
    case 3:
    case 4:
      // CLR fixpunkt_rh$,lpkt,lpktg!, @param_sp
      konsta_.fixpunkt_rh.clear();
      rhythm_lpkt_ = 0.0;
      konsta_.lpktg = false;
      persist_konsta();
      opt.special = -1.0;
      return true;
    default:
      return false;
  }
}

// ported from a17 with a17ach and a17eing
void MainWindow::rhythm_run(int preset) {
  if (!last_chart_) {
    return;
  }
  // a17ach, IF haw& <> 1
  if (houses_->currentIndex() != 0) {
    const int c = ChoiceDialog::ask(this, tr("!! ACHTUNG !!"),
                                    {QString(), tr("HÄUSER NICHT SCHULGERECHT !"), tr("'PLACIDUS ERFORDERLICH !")},
                                    {tr("NEUWAHL"), tr("WEITER")}, 0);
    if (c < 0) {
      return;
    }
    if (c == 0) {
      choose_house_system();
    }
  }
  // CLR hrg!, the walk reads the geocentric chart, the heliocentric sky
  // carries no Sun to rule or trigger
  if (helio_->isChecked()) {
    helio_->setChecked(false);
  }
  // without cusps no phase can be walked
  if (!last_chart_ || !last_chart_->houses.ok) {
    return;
  }
  int mode = preset;
  if (mode == 0) {
    //RR AUSWERTE-MODUS
    const int es = ChoiceDialog::ask(this, tr("AUSWERTE-MODUS"), {},
                                     {tr("GRAPHIK ( HOROSKOP )"), tr("AUSLÖSUNGS-TABELLE"), tr("GRAD-DATUM-LISTE ausgeben"),
                                      tr(" GRADE für GRAD-DATUM-LISTE NEU DEFINIEREN ! "),
                                      tr("SELBST DEFINIERTE GRADE LÖSCHEN ?"), tr("ABBRUCH")},
                                     0);
    switch (es) {
      case 0: mode = kGraph; break;
      case 1: mode = kTable; break;
      case 2: mode = kList; break;
      case 3: rhythm_define_degree(); return;
      case 4: rhythm_delete_degrees(); return;
      default: return;
    }
  }
  RhythmRun run;
  run.mode = mode;
  const auto with_sol = [this](const QStringList& lines) {
    QStringList info = rhythm_sol_lines();
    info << QString();
    info += lines;
    return info;
  };
  //RR RICHTUNGSSINN MARKIEREN !
  const int ri = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"),
                                   with_sol({tr("RICHTUNGSSINN MARKIEREN !"), QString(), tr("NORMAL ist 'RECHTS' !")}),
                                   {tr("RECHTS ( IM Uhrzeigersinn )"), tr("LINKS ( GEGEN den Uhrzeigersinn )")}, 0);
  if (ri < 0) {
    return;
  }
  // richt$ = "RECHTS" bzw. "LINKS "
  run.direction = ri == 0 ? tr("RECHTS") : tr("LINKS ");
  // a17dat, the list always dates
  if (mode < kList) {
    const int f = ChoiceDialog::ask(this, tr("AUSWAHL"), with_sol({tr("FORMAT der AUSGABE ?")}),
                                    {tr("DATUM"), tr("LEBENSJAHR/MONAT")}, 0);
    if (f < 0) {
      return;
    }
    run.dated = f == 0;
  }
  //RR BEGINN-PHASE MARKIEREN !
  const int bp = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), with_sol({tr("BEGINN-PHASE MARKIEREN !")}),
                                   {tr("EINS"), tr("VIER"), tr("SIEBEN")}, 0);
  if (bp < 0) {
    return;
  }
  static constexpr int kBegin[3] = {1, 4, 7};
  const bool septar = rhythm_chart_label().contains("SEPTAR");
  // IF eingm$ <> "" && q$ = "AR", the derived charts of the solar family
  const bool check = rhythm_chart_label().contains("AR");
  if (!rhythm_unit_question(septar, check) || !rhythm_period_question(check)) {
    return;
  }
  run.opt = rhythm_options();
  run.opt.leftward = ri == 1;
  run.opt.begin_house = kBegin[bp];
  run.clock = rhythm_clock(run.opt);
  // fixpunkt& = 2, the general fixed point rests during a17
  run.chart = *last_chart_;
  run.chart.b[body::kFixpunkt] = BodyState{};
  if (mode < kList) {
    if (!rhythm_special_question(septar, run.opt, run.clock)) {
      persist_konsta();
      return;
    }
    if (run.opt.special >= 0.0) {
      BodyState& sp = run.chart.b[body::kFixpunkt];
      sp.present = true;
      sp.valid = true;
      sp.el = run.opt.special;
    }
  }
  switch (mode) {
    case kGraph: rhythm_graph(run); break;
    case kTable: rhythm_table(run); break;
    default: rhythm_degree_list(run); break;
  }
  // @param_sp
  persist_konsta();
}

// ported from the SEPTAR case of a16 with a17eing11, a17eing12 and
// a17sonderpkt. The n-th Septar is the solar return of the (n - 1)th
// birthday, cast for the place of the event
void MainWindow::septar_chart() {
  if (!last_chart_) {
    return;
  }
  // the heliocentric sky has no Sun slot, a Septar needs the natal Sun
  if (!last_chart_->b[body::kSun].valid) {
    QMessageBox::information(this, tr("Septar"),
                             tr("Septar braucht die geozentrische Sonne. Heliozentrisch abschalten und erneut versuchen."));
    return;
  }
  // sol$(2,ze) = "SEPTAR" heads his boxes from the start, the number
  // joins it once the life year is known
  struct LabelScope {
    QString& label;
    ~LabelScope() { label.clear(); }
  } scope{rhythm_label_};
  rhythm_label_ = QStringLiteral("SEPTAR");
  // @ort_wahl(0,0)
  const std::optional<EventPlace> place = ask_event_place();
  if (!place) {
    return;
  }
  if (!rhythm_unit_question(true, false) || !rhythm_period_question(false)) {
    return;
  }
  // eingp$ = phas$, eingm$ = jahre$
  eingp_ = rhythm_phase_;
  eingm_ = rhythm_unit_;
  const double vp = rhythm_phase_.toDouble();
  // fa& = 1 with zeitm$ "MONAT", else fa& = 12 with "JAHR"
  const double fa = rhythm_months_ ? 1.0 : kMonthsPerYear;
  const QString zeitm = rhythm_months_ ? tr("MONAT") : tr("JAHR");
  // the tester's wish, the box opens with today's age
  const ChartInput base_in = radix_input();
  const int today_age = std::max(0, QDate::currentDate().year() - base_in.date_ut.year);
  // a$ = @inputbox$(160,eaz$,"INTERESSIERENDES LEBENSJAHR ?  GANZE ZAHL","")
  const auto age = ask_number(this, tr("ZAHLEN-Eingabe !"), tr("INTERESSIERENDES LEBENSJAHR ?  GANZE ZAHL"), 0.0,
                              kMaxLifeYears, today_age, 0);
  if (!age) {
    return;
  }
  // sen = FIX(a& / vp / fa& + 1)
  const int sen = static_cast<int>(std::trunc(*age / vp / fa + 1.0));
  // septar$(2,ze) = STR$(sen) + "." + "SEPTAR", sol$(2,ze) = septar$(2,ze)
  // before a17sonderpkt, so his a17sol lines name this Septar
  const QString label = QString("%1.SEPTAR").arg(sen);
  rhythm_label_ = label;
  RhythmOptions opt = rhythm_options();
  const RhythmClock clock = rhythm_clock(opt);
  if (!rhythm_special_question(true, opt, clock)) {
    persist_konsta();
    return;
  }
  persist_konsta();
  // jas&(2,ze) = ja&(1,ze) + sen - 1, the natal Sun of the birth place
  SearchContext ctx = make_context();
  ctx.base = base_in;
  const double birth_jd = julian_day(base_in.date_ut, ctx.settings.calendar);
  const BodyLongitude natal_sun = body_longitude(birth_jd, body::kSun, ctx);
  if (!natal_sun.valid) {
    QMessageBox::warning(this, tr("Septar"), tr("Natale Sonne konnte nicht bestimmt werden."));
    return;
  }
  // the search runs at the place of the event
  ctx.base.lon_deg_east = place->lon;
  ctx.base.lat_deg = place->lat;
  const int year = base_in.date_ut.year + sen - 1;
  const LongitudeCrossing hit = solar_return(base_in.date_ut, natal_sun.el, year, ctx);
  if (!hit.ok) {
    QMessageBox::warning(this, tr("Septar"), tr("Kein Septar gefunden."));
    banner_->set_record(tr("Kein Septar gefunden"));
    return;
  }
  set_panel_place(*place);
  rhythm_label_.clear();
  apply_moment(hit.jd_ut, label, true);
  // the Sonderpunkt lands on slot zero once the Septar is the active chart
  recompute();
  // mes1$ und mes2$, STR$(x,3)
  const double from = (sen - 1) * vp * fa;
  QMessageBox::information(
      this, tr(" Information "),
      tr("SEPTAR NR. %1 Gilt bei der Periode von %2 und der Zeit-Einheit %3").arg(sen).arg(rhythm_phase_, zeitm) +
          QChar(0x0A) +
          tr("Für die LEBENS-Jahre von %1 bis %2").arg(from, 3, 'g', 6).arg(from + vp * fa, 3, 'g', 6));
  // IF a& < vp * fa&, " 1. SEPTAR = RADIX !"
  if (*age < vp * fa) {
    QMessageBox::information(this, "HORCOM", tr(" 1. SEPTAR = RADIX !"));
  }
}

// ported from a17eing CASE 4, a17e1 to a17e3
void MainWindow::rhythm_define_degree() {
  const std::filesystem::path file = data_dir_ / "grade.int";
  const ChartSettings s = current_settings();
  // his ausw_pl_hs rows under dbr& 3, AC, MC and the houses of the chart
  // stay dashes, the HAUS NR. rows lead back to the degree
  std::vector<std::pair<int, QString>> rows;
  static constexpr const char* kCaption[11] = {
      QT_TRANSLATE_NOOP("horcom::MainWindow", " SONNE"),   QT_TRANSLATE_NOOP("horcom::MainWindow", " MOND"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " MERKUR"),  QT_TRANSLATE_NOOP("horcom::MainWindow", " VENUS"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " MARS"),    QT_TRANSLATE_NOOP("horcom::MainWindow", " JUPITER"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " SATURN"),  QT_TRANSLATE_NOOP("horcom::MainWindow", " URANUS"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " NEPTUN"),  QT_TRANSLATE_NOOP("horcom::MainWindow", " PLUTO"),
      nullptr};
  for (int slot = body::kSun; slot <= body::kPluto; ++slot) {
    rows.emplace_back(slot, tr(kCaption[slot - 1]));
  }
  rows.emplace_back(body::kNodeAsc, tr("  MONDKNOTEN N"));
  rows.emplace_back(body::kNodeDesc, tr("  MONDKNOTEN S"));
  for (int i = 0; i < 6; ++i) {
    rows.emplace_back(-1, QString());
  }
  for (const ExtraCaption& e : kExtraCaptions) {
    if (std::find(s.nk.begin(), s.nk.end(), e.slot) != s.nk.end()) {
      rows.emplace_back(e.slot, tr(e.caption));
    }
  }
  rows.emplace_back(kHouseRowFirst, tr("   HAUS NR.     "));
  rows.emplace_back(kHouseRowFirst + 1, tr(" HERR v. HAUS NR."));
  rows.emplace_back(kHouseRowFirst + 2, tr(" 0 GRAD eines ZEICHENS"));
  for (;;) {
    // a17eig1
    std::vector<CustomDegree> own = read_degrees(file);
    int half = 0;
    for (;;) {
      //RR GRAD EINGEBEN ! ( 0....359° ) , Auch HALBE Grade !
      const auto g = ask_number(this, tr("ZAHLEN-Eingabe !"), tr("GRAD EINGEBEN ! ( 0....359° ) , Auch HALBE Grade !"),
                                0.0, 360.0, std::numeric_limits<double>::quiet_NaN(), 1,
                                {tr(" Nur GANZE oder HALBE GRADE Eingeben"), tr(" Z.B: 125  =  5 GRAD LÖWE")});
      if (!g) {
        return;
      }
      // g& = 2 * g, the list knows half degrees only
      half = static_cast<int>(std::lround(2.0 * *g));
      // IF g& = 0 OR g& > 719, GOTO a17e1
      if (half > 0 && half <= 719) {
        break;
      }
    }
    // IF gs&(g&,0) = g&
    if (degree_known(half, own)) {
      QMessageBox::information(this, "HORCOM", tr("Dieser PUNKT ist BEREITS VORHANDEN "));
      return;
    }
    QMessageBox::information(this, "HORCOM", tr("Punkt ist NICHT von W.DÖBEREINER bzw. M.R. !"));
    //RR " Zwei Planeten nennen ! "
    const std::optional<int> p1 = ask_object(this, rows);
    if (!p1) {
      return;
    }
    if (*p1 >= kHouseRowFirst) {
      continue;
    }
    const std::optional<int> p2 = ask_object(this, rows);
    if (!p2) {
      return;
    }
    if (*p2 >= kHouseRowFirst) {
      continue;
    }
    // " PLANETEN : " + pl$(p1&) + " - " + pl$(p2&)
    const int b = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr(" PLANETEN : %1 - %2").arg(slot_tag(*p1), slot_tag(*p2))},
                                    {tr("OK"), tr(" KORRIGIEREN "), tr("ABBRUCH")}, 0);
    if (b == 1) {
      continue;
    }
    if (b != 0) {
      return;
    }
    // OPEN "A",#30,gri$, WRITE #30,gr$,p1$,p2$
    own.push_back({half / 2.0, *p1, *p2});
    if (!write_degrees(file, own)) {
      QMessageBox::warning(this, "HORCOM", tr("Die Grade ließen sich nicht speichern."));
    }
    return;
  }
}

// ported from a17eing CASE 5
void MainWindow::rhythm_delete_degrees() {
  const std::filesystem::path file = data_dir_ / "grade.int";
  std::error_code ec;
  if (!std::filesystem::exists(file, ec)) {
    QMessageBox::information(this, "HORCOM", tr("KEINE ZUSÄTZLICHEN Grade definiert !"));
    return;
  }
  const int b = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("ALLE selbst definierten Grade WIRKLICH LÖSCHEN ?")},
                                  {tr(" NEIN "), tr("JA")}, 0);
  if (b == 1) {
    std::filesystem::remove(file, ec);
  }
}

// ported from a170 with a178, a1781, a1791, a1792, a1795 and a170_1tit
DisplayList MainWindow::rhythm_phase_screen(const RhythmRun& run, const std::vector<RhythmTrigger>& rows,
                                            int phase) const {
  const ChartSettings s = current_settings();
  AspectSettings a = aspect_settings_;
  // nasp& = 6 with sext!, 4 otherwise
  a.divisors = run.opt.sextile ? 6 : 4;
  const AspectResult scan = scan_aspects(run.chart, s, a);
  WheelOptions wo = radix_wheel_options(run.chart, s);
  // his red F
  wo.emphasis[body::kFixpunkt] = 1;
  wo.phase_house = rhythm_phase_house(run.opt, phase);
  const double vp = run.opt.phase_years;
  const double fm = run.opt.months ? kMonthsPerYear : 1.0;
  const double length = vp / fm;
  // a = (psn& + bg& - 2) * vp / fm&
  const double start = (phase + run.opt.begin_house - 2) * length;
  const Calendar cal = s.calendar;
  const auto age_parts = [&run](double years) { return rhythm_age(years, run.clock.sn, true); };
  const auto lj_text = [&age_parts](double years) {
    // lj1$ = STR$(lj) + "/", mon1$ = STR$(mon,2)
    const RhythmAge g = age_parts(years);
    return QString("%1%2/%3").arg(g.negative ? "-" : "").arg(g.years).arg(static_cast<int>(g.months), 2);
  };
  std::map<int, RhythmPanelEntry> left;
  std::map<int, RhythmPanelEntry> right;
  std::set<int> rulers;
  for (const RhythmTrigger& t : rows) {
    if (t.phase != phase) {
      continue;
    }
    const double y = rhythm_axis_y(t.value, start, length);
    const QString date = datum_text(calendar_date(rhythm_jd(run.clock, t.value), cal));
    if (t.kind == RhythmKind::kAspect || t.kind == RhythmKind::kMirror) {
      // NOT (v& = 11 OR v& = 12), the nodes stay off the right column
      if (t.slot == body::kNodeAsc || t.slot == body::kNodeDesc) {
        continue;
      }
      RhythmPanelEntry& e = right[t.slot];
      e.slot = t.slot;
      e.source = t.source;
      e.y = y;
      // al$(u&,w&,2) = "S" stays once written, the sprite rides al&
      if (t.kind == RhythmKind::kMirror) {
        e.mirror = true;
      } else {
        e.family = t.family;
      }
      e.label = (run.dated ? date : "  " + lj_text(t.value)).toStdString();
      continue;
    }
    RhythmPanelEntry& e = left[t.slot];
    e.slot = t.slot;
    e.y = y;
    // lja$ = datum$ + " " + a$ bzw. lj1$ + mon1$ + " " + a$
    e.label = ((run.dated ? date : lj_text(t.value)) + " " + kind_text(t.kind)).toStdString();
    //RR Der (die) PHASEN-HERRSCHER ist (sind) INVERS dargestellt,falls er nicht gleichzeitig direkt angetroffen wird.
    e.inverse = rhythm_ruler(t.kind);
    if (rhythm_ruler(t.kind)) {
      rulers.insert(t.slot);
    }
  }
  // plinv of a170, the rulers of the phase flip their stamp in the wheel
  for (const int r : rulers) {
    wo.flip_inverted.push_back(r);
  }
  DisplayList dl = build_wheel(run.chart, s, scan, wo);
  // his naf& block moved ten pixels right, clear of the strip line at 218
  add_corner_text(dl, classic_sheet_text(), kRhythmNameX, 383.0, kCanvasWidth - 8.0, true);
  RhythmPanel p;
  // asl$ + " nach " + db$ + ":"
  p.title = tr("Auslösung nach DÖBEREINER:").toStdString();
  const QString unit = rhythm_months_ ? tr(" Monate") : tr(" Jahre");
  // LEFT$(pi$ + ": " + phas$ + jahre$ + ": " + richt$ + "     ",26)
  p.period = (tr("Periode") + ": " + rhythm_phase_ + unit + ": " + run.direction + "     ").left(26).toStdString();
  if (vp < 0.0) {
    p.negative1 = tr("NEGATIVE Periode !").toStdString();
    p.negative2 = tr("Richtung VERGANGENHEIT !").toStdString();
  }
  p.dated = run.dated;
  if (!run.dated) {
    // "Von " + a$ + d$ + " BIS " + b$ + e$, "( LJ/MON )"
    const auto span = [&run](double years) {
      const RhythmAge g = rhythm_age(years, run.clock.sn, false);
      return QString("%1%2.0/").arg(g.negative ? "-" : "").arg(g.years) + QString::asprintf("%4.1f", g.months);
    };
    p.span = (tr("Von ") + span(start) + tr(" BIS ") + span(start + length)).toStdString();
    p.span_unit = tr("( LJ/MON )").toStdString();
  }
  for (const auto& [slot, e] : left) {
    p.left.push_back(e);
  }
  for (const auto& [slot, e] : right) {
    p.right.push_back(e);
  }
  // "WEITER mit " + lt$
  p.footer = (tr("WEITER mit ") + tr("Leertaste")).toStdString();
  add_rhythm_panel(dl, p);
  // @text(340,gdyh& - 3,13,"SP:" + gz1$)
  if (run.opt.special >= 0.0 && !(run.opt.mundane && !konsta_.lpktg)) {
    dl.items.push_back(screen_text(340.0, 456.0, 13.0, ("SP:" + grze_text(run.opt.special, true)).toStdString()));
  }
  return dl;
}

void MainWindow::rhythm_graph(const RhythmRun& run) {
  AspectSettings a = aspect_settings_;
  a.divisors = run.opt.sextile ? 6 : 4;
  const AspectResult scan = scan_aspects(run.chart, current_settings(), a);
  const std::vector<RhythmTrigger> rows = rhythm_triggers(run.chart, scan, a, run.opt);
  const int phases = rhythm_phase_count(run.opt);
  int phase = 1;
  SheetView view(this);
  view.set_step([&]() {
    if (phase >= phases) {
      return false;
    }
    ++phase;
    view.canvas()->set_plain_list(rhythm_phase_screen(run, rows, phase));
    return true;
  });
  mark_output(&view, menu_item::kRhythm);
  // " " + asl$ + " nach W." + db$ + mun$
  view.setWindowTitle(" " + tr("Auslösung nach W.DÖBEREINER") + (run.opt.mundane ? "/MUNDAN" : "") + " | " +
                      tr("WEITER mit Leertaste"));
  view.canvas()->set_plain_list(rhythm_phase_screen(run, rows, phase));
  view.resize(size());
  view.exec();
}

// ported from a18kopf with dbr& > 0
QStringList MainWindow::rhythm_heading(const RhythmRun& run) const {
  const QString unit = rhythm_months_ ? tr(" Monate") : tr(" Jahre");
  // di$ = " " + asl$ + " nach W." + db$ + mun$
  QString first = " " + tr("Auslösung nach W.DÖBEREINER") + (run.opt.mundane ? "/MUNDAN" : "");
  // d$ = " |Richtung:" + richt$ + " |" + pi$ + ": " + phas$ + jahre$
  first += " |" + tr("Richtung:") + run.direction + " |" + tr("Periode") + ": " + rhythm_phase_ + unit;
  if (run.opt.phase_years < 0.0) {
    first += tr(" Vergangenheit!");
  }
  const ClassicSheetText txt = classic_sheet_text();
  QStringList out{first};
  out << tr("Name : %1 | %2 ").arg(QString::fromStdString(txt.name).trimmed(), rhythm_chart_label()) +
             tr("| Ort: %1").arg(QString::fromStdString(txt.place).trimmed());
  out << tr("|Länge: %1 |Breite: %2")
             .arg(QString::fromStdString(txt.lon).trimmed(), QString::fromStdString(txt.lat).trimmed());
  out << QString::fromStdString(txt.date).trimmed() + " | " + QString::fromStdString(txt.ut).trimmed();
  return out;
}

namespace {

// the table of a18list, one row per trigger under the heading of its phase
QTableWidget* rhythm_sheet(QDialog* dialog, const QStringList& heading, const QStringList& columns, bool past) {
  auto* v = new QVBoxLayout(dialog);
  for (int i = 0; i < heading.size(); ++i) {
    auto* l = new QLabel(heading[i], dialog);
    if (i == 0 && past) {
      // RGBCOLOR RGB(255,0,0), the past in red
      l->setStyleSheet(QStringLiteral("color: #d00000;"));
    }
    v->addWidget(l);
  }
  auto* table = new QTableWidget(0, static_cast<int>(columns.size()), dialog);
  table->setHorizontalHeaderLabels(columns);
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(18);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  table->setFont(theme::mono_font());
  v->addWidget(table, 1);
  return table;
}

// t$ = "PHASE " + STR$(pb& + bg& - 1) + " = HS " + STR$(lsph&)
void phase_row(QTableWidget* table, const QString& text) {
  const int row = table->rowCount();
  table->insertRow(row);
  auto* item = new QTableWidgetItem(text);
  QFont f = table->font();
  f.setBold(true);
  item->setFont(f);
  item->setBackground(QColor(0xE8, 0xE8, 0xE8));
  item->setForeground(QColor(0, 0, 0));
  table->setItem(row, 0, item);
  table->setSpan(row, 0, 1, table->columnCount());
}

}  // namespace

// ported from a174 and a175 with dbr& 2
void MainWindow::rhythm_table(const RhythmRun& run) {
  AspectSettings a = aspect_settings_;
  a.divisors = run.opt.sextile ? 6 : 4;
  const AspectResult scan = scan_aspects(run.chart, current_settings(), a);
  const std::vector<RhythmTrigger> rows = rhythm_triggers(run.chart, scan, a, run.opt);
  QDialog dialog(this);
  mark_output(&dialog, menu_item::kRhythm);
  dialog.setWindowTitle(" " + tr("Auslösung nach W.DÖBEREINER") + (run.opt.mundane ? "/MUNDAN" : ""));
  // " " + dm$ + "   PL ART" bzw. " LJ  MO  PL ART"
  QTableWidget* table = rhythm_sheet(&dialog, rhythm_heading(run),
                                     {run.dated ? tr("Datum") : tr("LJ  MO"), tr("PL"), tr("ART")},
                                     run.opt.phase_years < 0.0);
  const Calendar cal = current_settings().calendar;
  int last_phase = 0;
  for (const RhythmTrigger& t : rows) {
    if (t.phase != last_phase) {
      last_phase = t.phase;
      phase_row(table, tr("PHASE %1 = HS %2").arg(t.phase + run.opt.begin_house - 1).arg(t.house));
    }
    const int row = table->rowCount();
    table->insertRow(row);
    QString when;
    if (run.dated) {
      when = datum_text(calendar_date(rhythm_jd(run.clock, t.value), cal));
    } else {
      // a175, bb$ three places and aa$ one decimal
      const RhythmAge g = rhythm_age(t.value, run.clock.sn, false);
      when = QString("%1 %2").arg(QString(g.negative ? "-" : "") + QString::number(g.years), 3).arg(g.months, 4, 'f', 1);
    }
    table->setItem(row, 0, new QTableWidgetItem(when));
    // a175 stamps the sprite of the planet, it stands before the tag
    auto* planet = new QTableWidgetItem(slot_tag(t.slot));
    set_body_sprite(planet, t.slot, theme::ink_now());
    table->setItem(row, 1, planet);
    QString art;
    if (t.kind == RhythmKind::kAspect) {
      // asps&(al1&) and the planet of the trigger, his r&
      art = QString::fromUtf8(aspect_glyph(t.family)) + " " + slot_tag(t.source);
    } else {
      art = kind_text(t.kind);
    }
    table->setItem(row, 2, new QTableWidgetItem(art));
  }
  table->resizeColumnsToContents();
  dialog.resize(520, 640);
  dialog.exec();
}

// ported from a17_3 and a174 with dbr& 3
void MainWindow::rhythm_degree_list(const RhythmRun& run) {
  const std::vector<CustomDegree> own = read_degrees(data_dir_ / "grade.int");
  const std::vector<DegreeDate> rows = degree_dates(run.chart, run.opt, own, run.opt.mundane, current_input().lat_deg);
  QDialog dialog(this);
  mark_output(&dialog, menu_item::kRhythm);
  dialog.setWindowTitle(" " + tr("Auslösung nach W.DÖBEREINER") + (run.opt.mundane ? "/MUNDAN" : ""));
  // " GRAD    " + dm$ + " "
  QTableWidget* table = rhythm_sheet(&dialog, rhythm_heading(run), {tr("GRAD"), QString(), tr("Datum")},
                                     run.opt.phase_years < 0.0);
  // his sprites in the mark column, the pairs boxed like a174g
  table->setItemDelegateForColumn(1, new SpriteRowDelegate(table));
  const Calendar cal = current_settings().calendar;
  const QColor red(0xE0, 0x00, 0x00);
  for (int k = 0; k < rhythm_phase_count(run.opt); ++k) {
    const int house = rhythm_phase_house(run.opt, k + 1);
    phase_row(table, tr("PHASE %1 = HS %2").arg(k + run.opt.begin_house).arg(house));
    for (std::size_t i = 0; i < rows.size(); ++i) {
      const DegreeDate& r = rows[i];
      if (r.house != house) {
        continue;
      }
      const int row = table->rowCount();
      table->insertRow(row);
      // za = w - 30 * FIX(w / 30), STR$(za,4,1)
      const double za = r.degree - kDegPerSign * std::floor(r.degree / kDegPerSign);
      table->setItem(row, 0, new QTableWidgetItem(QString::asprintf("%4.1f", za)));
      const int sign = static_cast<int>(r.degree / kDegPerSign) % 12;
      auto* mark = new QTableWidgetItem;
      const bool cardinal = i % 180 == 0;
      const QString sign_sprite = QString::fromUtf8(sign_glyph(sign));
      // IF q& = 0, q& = p&, the planet stands twice in its box
      const QStringList pair{QString::fromUtf8(body_glyph(r.p)), QString::fromUtf8(body_glyph(r.q > 0 ? r.q : r.p))};
      if (r.p > 0 && !r.custom) {
        // a174g, the planet pair of the Gruppenschicksals-Grad
        mark->setText(slot_tag(r.p) + "-" + slot_tag(r.q));
        mark->setData(kSpritesRole, pair);
        mark->setData(kFrameRole, true);
      } else if (cardinal) {
        // CASE 0,180,360,540, the sign framed in red
        mark->setText(sign_sprite);
        mark->setForeground(red);
        mark->setData(kSpritesRole, QStringList{sign_sprite});
        mark->setData(kFrameRole, true);
      } else if (r.custom) {
        // his plan_col! paints the own pairs in colour, here in red
        mark->setText(slot_tag(r.p) + "-" + slot_tag(r.q) + (r.mirror ? tr(" (Spiegel)") : QString()));
        mark->setForeground(red);
        mark->setData(kSpritesRole, pair);
        mark->setData(kFrameRole, true);
      } else {
        // zeichp_dspl, the plain sign
        mark->setText(sign_sprite);
        mark->setData(kSpritesRole, QStringList{sign_sprite});
      }
      // the sprites stand for the tags, the tip names them
      mark->setToolTip(mark->text());
      table->setItem(row, 1, mark);
      table->setItem(row, 2, new QTableWidgetItem(datum_text(calendar_date(rhythm_jd(run.clock, r.value), cal))));
    }
  }
  table->resizeColumnsToContents();
  dialog.resize(420, 680);
  dialog.exec();
}

}  // namespace horcom
