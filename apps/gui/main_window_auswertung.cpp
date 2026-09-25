// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// The AUSWERTUNG flows around the returns and the progressed moments,
// the SOLAR, LUNAR, PLANETARE and PERSONARE of a16 with their lists, the
// TERRAR under hrg!, taho and proho, plus the ort_wahl question they share.

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPrinter>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <cmath>
#include <limits>
#include <memory>

#include "banner.hpp"
#include "calendar_mark.hpp"
#include "choice_dialog.hpp"
#include "event_filter.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/progressions.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/place_file.hpp"
#include "horcom/render/items.hpp"
#include "main_window.hpp"
#include "place_dialog.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

// IF i& = 28, a column of the table
constexpr int kReturnColumnRows = 28;
// UNTIL ... (ja - ja&(1,ze)) > 135
constexpr int kReturnListYears = 135;
//RR Bis zu 84 Daten !
constexpr int kReturnListRows = 84;
// ADD jd,29 und SUB jd,26, the start of the next and the previous lunar
// search from the one on screen
constexpr double kLunarForwardDays = 29.0;
constexpr double kLunarBackwardDays = 26.0;
// SUB jd,29 before the LUNAR list, then ADD jd,33 from every lunar found
constexpr double kLunarListLeadDays = 29.0;
constexpr double kLunarListStepDays = 33.0;
// SUB jd,tja before the SOLAR list, then ADD jd,tja + 40 from every
// solar found
constexpr double kSolarListStepDays = 40.0;
// jdz = jd + 15, his STARTWERT of the SOLAR, SEPTAR and PLANETARE
// searches past the date
constexpr double kReturnSeedDays = 15.0;
// STR$(INT(ABS(VAL(a$))),3,0), the largest number his a16_i$ took
constexpr int kReturnNumberMax = 999;

// the answers of his SUCHEN mit box, the item data of the combo
constexpr int kByPast = 0;
constexpr int kByFuture = 1;
constexpr int kByDate = 2;

// ported from solnummer, the number of a return counted from the birth.
// He counted from the search seed with five days of slack, which suits
// the seeds of the SOLAR, while a LUNAR searched by date lies up to a
// lunation past its return and took the next number. The whole periods
// from the birth to the return found name it in every case
int return_number(double jd_return, double jd_birth, double period) {
  return static_cast<int>(std::lround((jd_return - jd_birth) / period));
}

// his alertbox SUCHEN mit : of the LUNAR and the PLANETARE, DATUM first
QComboBox* search_by_box(QWidget* parent) {
  auto* box = new QComboBox(parent);
  box->addItem(MainWindow::tr("DATUM"), kByDate);
  box->addItem(MainWindow::tr("NR. ZUKUNFT"), kByFuture);
  box->addItem(MainWindow::tr("NR. VERGANGENHEIT"), kByPast);
  return box;
}

// his NR. of a16_i$, zero the radix itself
QSpinBox* return_number_box(QWidget* parent) {
  auto* nr = new QSpinBox(parent);
  nr->setRange(0, kReturnNumberMax);
  return nr;
}

// the lines above his number and date boxes, a16_i$ for the numbers and
// the message of the LUNAR before its date
QString search_hint(int by, const QString& sol, bool lunar) {
  if (by == kByFuture) {
    return MainWindow::tr(" NR. 0 ENTSPRICHT der RADIX, NR. 1 dem 1. %1 in der ZUKUNFT usw.").arg(sol);
  }
  if (by == kByPast) {
    return MainWindow::tr(" NR. -1 ENTSPRICHT dem 1. %1 in der VERGANGENHEIT").arg(sol);
  }
  if (!lunar) {
    return {};
  }
  return MainWindow::tr("Nachfolgend Datum eingeben aus dem INTERESSIERENDEN ZEITRAUM ! ") + QChar(0x0A) +
         MainWindow::tr("Meist wird das Diesem Datum, 0H UT ,VORAUSGEHENDE LUNAR berechnet ! ") + QChar(0x0A) +
         MainWindow::tr("Wenn NICHT,ZEITPUNKT entsprechend VERSCHIEBEN !");
}

// today as the date the event boxes open with
QString date_text(const CalendarDate& d) {
  return QString::asprintf("%02d.%02d.%d", d.day, d.month, d.year);
}

// his textzentrl and textc on the 640 by 480 screen, the bottom line
void table_text(DisplayList& dl, double x, double bottom, const QString& s, double size, bool centred) {
  Primitive p = text_item(centred ? kCanvasWidth / 2.0 : x, bottom - 0.5 * size, size, s.toStdString(), kInkColor, centred);
  // te_w& = @textg(te_gr&), FONT WIDTH te_w&
  p.pitch = font_pitch(size);
  dl.items.push_back(std::move(p));
}

// ported from sol_lun_tabelle, up to 84 moments in three columns of 28
DisplayList return_sheet(const QString& head, const QString& birth, const QString& foot,
                         const std::vector<QString>& rows) {
  DisplayList dl;
  dl.width = kCanvasWidth;
  dl.height = kCanvasHeight;
  // @textzentrl(20,16,1,...), @textzentrl(40,16,1,...), @textzentrl(454,14,1,...)
  table_text(dl, 0.0, 20.0, head, 16.0, true);
  table_text(dl, 0.0, 40.0, birth, 16.0, true);
  table_text(dl, 0.0, 454.0, foot, 14.0, true);
  // @line(212,44,212,434), @line(422,44,422,434)
  for (const double x : {212.0, 422.0}) {
    dl.items.push_back(line_item(x, 44.0, x, 434.0));
  }
  double x0 = 4.0;
  int i = 0;
  for (const QString& row : rows) {
    // @textc(x0&,46 + i& * 14,16," " + datum3$ + " " + ze$), 28 to a column
    ++i;
    table_text(dl, x0, 46.0 + i * 14.0, row, 16.0, false);
    if (i == kReturnColumnRows) {
      x0 += 210.0;
      i = 0;
    }
  }
  return dl;
}

}  // namespace

// ported from ort_wahl
std::optional<MainWindow::EventPlace> MainWindow::ask_event_place(const QStringList& notes) {
  //RR EREIGNIS-Ort = GEBURTS-Ort ?
  const int b = ChoiceDialog::ask(this, "HORCOM", notes + QStringList{tr("EREIGNIS-Ort = GEBURTS-Ort ?")},
                                  {tr("NEIN ( ORTSDATEIEN holen ? )"), tr("JA"), tr("VORZUGSORT"), tr("ABBRUCH")}, 1);
  switch (b) {
    case 0: {
      PlaceDialog pick(data_dir_ / "places", data_dir_ / "landnima.int", this);
      if (pick.exec() != QDialog::Accepted) {
        return std::nullopt;
      }
      return EventPlace{pick.chosen().lon, pick.chosen().lat, pick.chosen_name().toStdString()};
    }
    case 1:
      return birth_place();
    case 2: {
      // @ortg, the preferred place of ORT.EXT
      const auto home = read_preferred_place(data_dir_ / "ort.ext");
      if (!home) {
        QMessageBox::information(this, "HORCOM", tr("Kein VORZUGSORT festgelegt, der Geburts-Ort gilt."));
        return birth_place();
      }
      return EventPlace{home->lon, home->lat, home->name};
    }
    default:
      return std::nullopt;
  }
}

MainWindow::EventPlace MainWindow::birth_place() const {
  const ChartInput in = radix_input();
  const bool solar_row = active_is_solar_ && active_solar_ >= 0 && slots_[static_cast<std::size_t>(active_solar_)];
  return {in.lon_deg_east, in.lat_deg, solar_row ? slots_[static_cast<std::size_t>(active_solar_)]->place : record_.place};
}

// the tester asked for the place in the return dialogs, the row shows it
// and the button asks his ort_wahl box, JA the preset
QLayout* MainWindow::event_place_row(QWidget* dialog, EventPlace* place, std::function<void()> changed) {
  const auto caption = [](const EventPlace& p) {
    const QString name = QString::fromStdString(p.name).trimmed();
    return name.isEmpty() ? QString::asprintf("%.4f / %.4f", p.lon, p.lat) : name;
  };
  auto* row = new QHBoxLayout();
  row->addWidget(new QLabel(tr("Ort:"), dialog));
  auto* name = new QLabel(caption(*place), dialog);
  name->setStyleSheet("font-weight: bold");
  row->addWidget(name, 1);
  auto* choose = new QPushButton(tr("Ort wählen…"), dialog);
  row->addWidget(choose);
  connect(choose, &QPushButton::clicked, dialog, [this, place, name, caption, changed = std::move(changed)]() {
    if (const std::optional<EventPlace> p = ask_event_place()) {
      *place = *p;
      name->setText(caption(*p));
      if (changed) {
        changed();
      }
    }
  });
  return row;
}

void MainWindow::set_panel_place(const EventPlace& p) {
  const QSignalBlocker b1(lon_);
  const QSignalBlocker b2(lat_);
  const QSignalBlocker b3(place_field_);
  lon_->setValue(p.lon);
  lat_->setValue(p.lat);
  place_field_->setText(QString::fromStdString(p.name));
  sync_coord_boxes();
}

// the derived charts build on the radix. A derived chart on the panel
// holds the birth data in record_, a SOLAR row taken back from the menu
// derives from the RADIX row of its number like sol$(2,ze) from ze
ChartInput MainWindow::radix_input() const {
  if (active_is_solar_ && active_solar_ >= 0 && slots_[static_cast<std::size_t>(active_solar_)]) {
    return record_input(*slots_[static_cast<std::size_t>(active_solar_)]);
  }
  const bool use_record = record_.day > 0 || record_.jd > 0.0;
  return use_record ? record_input(record_) : current_input();
}

Chart MainWindow::radix_chart() const {
  const Chart c = compute_chart(radix_input(), current_settings(), vsop_, eph_);
  return c.ok ? c : *last_chart_;
}

// ported from the SOLAR case of a16. The row names the event place of
// his ort_wahl, the year buttons are his FOLGENDES and VORHERGEHENDES
// Jahr and cast the solar at once
void MainWindow::solar_chart() {
  // IF hrg!, sol$ = "TERRAR"
  if (current_settings().heliocentric) {
    terrar_chart();
    return;
  }
  if (!last_chart_ || !last_chart_->b[body::kSun].valid) {
    return;
  }
  EventPlace place = birth_place();
  std::optional<int> shown;
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Solar"));
  auto* v = new QVBoxLayout(&dialog);
  v->addLayout(event_place_row(&dialog, &place, [&shown]() { shown.reset(); }));
  auto* year_row = new QHBoxLayout();
  year_row->addWidget(new QLabel(tr("Gewünschtes Kalender-Jahr:"), &dialog));
  auto* year_spin = new QSpinBox(&dialog);
  year_spin->setRange(1, 3000);
  year_spin->setValue(QDate::currentDate().year());
  // the year buttons below take the step, spin arrows at the field would
  // stand beside them as dead twins
  year_spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
  year_row->addWidget(year_spin, 1);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  //RR WEITERES SOLAR SUCHEN ? HOROSKOP für VORHERGEHENDES Jahr ?, FOLGENDES Jahr ?
  auto* prev_btn = buttons->addButton(tr("Vorheriges Jahr"), QDialogButtonBox::ActionRole);
  auto* next_btn = buttons->addButton(tr("Nächstes Jahr"), QDialogButtonBox::ActionRole);
  const auto run = [this, &place, &shown, year_spin]() {
    run_solar(year_spin->value(), place);
    shown = year_spin->value();
  };
  connect(prev_btn, &QPushButton::clicked, &dialog, [year_spin, run]() {
    year_spin->setValue(year_spin->value() - 1);
    run();
  });
  connect(next_btn, &QPushButton::clicked, &dialog, [year_spin, run]() {
    year_spin->setValue(year_spin->value() + 1);
    run();
  });
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(year_row);
  v->addWidget(buttons);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  // the solar on screen stays, a changed year or place is cast
  if (shown != year_spin->value()) {
    run();
  }
}

void MainWindow::show_solar(int year) {
  run_solar(year, birth_place());
}

// ported from the a16sol loop of the SOLAR case of a16
void MainWindow::run_solar(int year, const EventPlace& place) {
  if (!last_chart_ || !last_chart_->b[body::kSun].valid) {
    return;
  }
  SearchContext ctx = make_context();
  ctx.base = radix_input();
  const double birth = julian_day(ctx.base.date_ut, ctx.settings.calendar);
  // pz = plz(1,ze,1), the Sun of the radix at the birth place
  const BodyLongitude natal_sun = body_longitude(birth, body::kSun, ctx);
  if (!natal_sun.valid) {
    banner_->set_record(tr("Natale Sonne konnte nicht bestimmt werden"));
    return;
  }
  // @ort_wahl set gl and gg before plant, the search runs at the event
  ctx.base.lon_deg_east = place.lon;
  ctx.base.lat_deg = place.lat;
  const LongitudeCrossing hit = solar_return(ctx.base.date_ut, natal_sun.el, year, ctx);
  if (!hit.ok) {
    banner_->set_record(tr("Kein Solar gefunden"));
    return;
  }
  set_panel_place(place);
  // solnummer, sol$(2,ze) = STR$(n) + "." + "SOLAR"
  const int n = return_number(hit.jd_ut, birth, last_chart_->ta.tropical_year_days);
  apply_moment(hit.jd_ut, QString("%1.SOLAR").arg(n), true);
}

// ported from taho, the moment of the chosen day with the true solar
// time of the birth at the event place, NÄCHSTER and VORHERGEHENDER TAG
// step on from the moment found
void MainWindow::day_chart() {
  if (!last_chart_) {
    return;
  }
  // hrg! = FALSE, the true solar time needs the geocentric Sun
  if (helio_->isChecked()) {
    helio_->setChecked(false);
  }
  const Chart radix = radix_chart();
  const std::optional<EventPlace> place = ask_event_place();
  if (!place) {
    return;
  }
  //RR DATUM EINGEBEN !
  const std::optional<CalendarDate> day = ask_date(this, tr("DATUM EINGEBEN !"), today_date());
  if (!day) {
    return;
  }
  SearchContext ctx = make_context();
  ctx.base = radix_input();
  ctx.base.lon_deg_east = place->lon;
  ctx.base.lat_deg = place->lat;
  double seed = event_at_radix_clock(radix, julian_day({day->day, day->month, day->year, 0, 0.0},
                                                       ctx.settings.calendar),
                                     false);
  for (;;) {
    const ProgressedMoment m = day_chart_moment(radix, seed, ctx);
    if (!m.ok) {
      banner_->set_record(tr("Kein Tages-Horoskop gefunden"));
      return;
    }
    set_panel_place(*place);
    // sol$(od,ze) = "TAG-HOR", every day in the same SOLAR row
    apply_moment(m.jd_ut, "TAG-HOR", true);
    //RR WEITERES TAGES-HOROSKOP ? bei GLEICHEM ORT ?
    const int ret = ChoiceDialog::ask(this, tr("TAGES-HOROSKOP"), {tr("WEITERES TAGES-HOROSKOP ?"), tr("bei GLEICHEM ORT ?")},
                                      {tr("NÄCHSTER TAG ?"), tr("VORHERGEHENDER TAG ?"), tr("Weiter ( = ENDE )")}, 2);
    if (ret == 0) {
      seed = m.jd_ut + 1.0;
    } else if (ret == 1) {
      seed = m.jd_ut - 1.0;
    } else {
      return;
    }
  }
}

// ported from proho with taho_proho_ini, one day of the sky per year of
// life, the event at its place and the four clock modes of prog_mode
void MainWindow::progression_chart() {
  if (!last_chart_) {
    return;
  }
  // hrg! = FALSE, the clock modes read the geocentric Sun
  if (helio_->isChecked()) {
    helio_->setChecked(false);
  }
  for (;;) {
    const Chart radix = radix_chart();
    const std::optional<EventPlace> place = ask_event_place();
    if (!place) {
      return;
    }
    const std::optional<CalendarDate> day = ask_date(this, tr("DATUM EINGEBEN !"), today_date());
    if (!day) {
      return;
    }
    //RR RECHEN-MODUS ?
    const int mode = ChoiceDialog::ask(this, tr("RECHEN-MODUS ?"), {},
                                       {tr("UT = RADIX-UT"), tr("WAHRE SONNENZEIT = WAHRE SONNENZEIT RADIX"),
                                        tr("HÄUSER-DREHUNG gemäß  '1 TAG = 1 JAHR'"),
                                        tr("STRENG PROPORTIONALE UMRECHNUNG des JD für PLANETEN und HÄUSER"),
                                        tr("ABBRUCH")});
    if (mode < 0 || mode > 3) {
      return;
    }
    SearchContext ctx = make_context();
    ctx.base = radix_input();
    ctx.base.lon_deg_east = place->lon;
    ctx.base.lat_deg = place->lat;
    const double event = event_at_radix_clock(
        radix, julian_day({day->day, day->month, day->year, 0, 0.0}, ctx.settings.calendar), true);
    const ProgressedMoment m = progressed_moment(radix, event, static_cast<ProgressionMode>(mode + 1), ctx);
    if (!m.ok) {
      banner_->set_record(tr("Keine Progression gefunden"));
      return;
    }
    // "Ereig: " + d$ und rd$ + ": " + datum3$
    progression_pending_ = true;
    prog_event_note_ = (tr("Ereig: ") + date_text(*day)).toStdString();
    prog_radix_note_ = (tr("RADIX: ") + date_text(calendar_date(radix.jd_ut, panel_calendar_))).toStdString();
    set_panel_place(*place);
    // sol$(od,ze) = "PROG-HOR", every try in the same SOLAR row
    apply_moment(m.jd_ut, "PROG-HOR", true);
    progression_pending_ = false;
    //RR WEITERES PROGRESSIONS-HOROSKOP ?
    const int ret = ChoiceDialog::ask(this, tr("PROGRESSIONS-HOROSKOP"), {tr("WEITERES PROGRESSIONS-HOROSKOP ?")},
                                      {tr(" WEITERER VERSUCH "), tr(" ENDE = Daten übernehmen")}, 1);
    if (ret != 0) {
      return;
    }
  }
}

// ported from the LUNAR case of a16. His SUCHEN mit box, the message
// before the date and the a16_i$ hints stand in one dialog with the Ort
// row, the step keys search on from the one on screen like WEITERES
// LUNAR SUCHEN
void MainWindow::lunar_chart() {
  // hrg! knows no LUNAR, the moon slot carries the Earth there
  if (!last_chart_ || !last_chart_->b[body::kMoon].valid || current_settings().heliocentric) {
    return;
  }
  EventPlace place = birth_place();
  // the lunar on screen, and whether an answer changed since
  double shown = 0.0;
  bool dirty = true;
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Lunar"));
  auto* v = new QVBoxLayout(&dialog);
  v->addLayout(event_place_row(&dialog, &place, [&dirty]() { dirty = true; }));
  auto* form = new QFormLayout();
  auto* by = search_by_box(&dialog);
  auto* when = new QDateEdit(QDate::currentDate(), &dialog);
  when->setCalendarPopup(true);
  when->setDisplayFormat("dd.MM.yyyy");
  auto* nr = return_number_box(&dialog);
  nr->setButtonSymbols(QAbstractSpinBox::NoButtons);
  form->addRow(tr("SUCHEN mit :"), by);
  form->addRow(tr("Datum"), when);
  form->addRow(tr("Nummer"), nr);
  auto* hint = new QLabel(&dialog);
  hint->setWordWrap(true);
  const auto refresh = [by, when, nr, hint, &dirty]() {
    const int mode = by->currentData().toInt();
    when->setEnabled(mode == kByDate);
    nr->setEnabled(mode != kByDate);
    hint->setText(search_hint(mode, QStringLiteral("LUNAR"), true));
    dirty = true;
  };
  refresh();
  connect(by, &QComboBox::currentIndexChanged, &dialog, refresh);
  connect(when, &QDateEdit::dateChanged, &dialog, [&dirty]() { dirty = true; });
  connect(nr, &QSpinBox::valueChanged, &dialog, [&dirty]() { dirty = true; });
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  //RR WEITERES LUNAR SUCHEN ? In der ZUKUNFT ? In der Vergangenheit ?
  auto* prev_btn = buttons->addButton(tr("Vorheriges Lunar"), QDialogButtonBox::ActionRole);
  auto* next_btn = buttons->addButton(tr("Nächstes Lunar"), QDialogButtonBox::ActionRole);
  // the radix Moon at the birth place, the search at the event place
  struct Base {
    SearchContext ctx;
    double birth = 0.0;
    double moon = 0.0;
  };
  const auto base = [this, &place]() -> std::optional<Base> {
    Base b;
    b.ctx = make_context();
    b.ctx.base = radix_input();
    b.birth = julian_day(b.ctx.base.date_ut, b.ctx.settings.calendar);
    // pz = plz(1,ze,2)
    const BodyLongitude natal = body_longitude(b.birth, body::kMoon, b.ctx);
    if (!natal.valid) {
      banner_->set_record(tr("Nataler Mond konnte nicht bestimmt werden"));
      return std::nullopt;
    }
    b.moon = natal.el;
    b.ctx.base.lon_deg_east = place.lon;
    b.ctx.base.lat_deg = place.lat;
    return b;
  };
  const auto show = [this, &place, &shown, &dirty](const LongitudeCrossing& hit, double birth) {
    if (!hit.ok) {
      banner_->set_record(tr("Kein Lunar gefunden"));
      return;
    }
    shown = hit.jd_ut;
    dirty = false;
    set_panel_place(place);
    // solnummer, sol$(2,ze) = STR$(n) + "." + "LUNAR", counted from the
    // lunar found, his count from the search date named the next lunar
    // in the last days before it
    const int n = return_number(hit.jd_ut, birth, kTropicalMonthDays);
    apply_moment(hit.jd_ut, QString("%1.LUNAR").arg(n), true);
  };
  const auto run_lunar = [by, when, nr, base, show]() {
    const std::optional<Base> b = base();
    if (!b) {
      return;
    }
    const int mode = by->currentData().toInt();
    if (mode == kByDate) {
      // jdz = jd, the lunar before the date at 0h UT
      const QDate d = when->date();
      const double before = julian_day({d.day(), d.month(), d.year(), 0, 0.0}, b->ctx.settings.calendar);
      show(lunar_return(before, b->moon, b->ctx), b->birth);
    } else {
      // jd = jd(1,ze) + ns1& * ta(2) + 0.1 * ta(2)
      show(planetar_return(b->birth, body::kMoon, b->moon, nr->value(), mode == kByFuture, b->ctx), b->birth);
    }
  };
  const auto step = [&shown, &dirty, base, show, run_lunar](bool forward) {
    if (shown <= 0.0 || dirty) {
      run_lunar();
      return;
    }
    const std::optional<Base> b = base();
    if (!b) {
      return;
    }
    const double start = forward ? shown + kLunarForwardDays : shown - kLunarBackwardDays;
    show(lunar_return(start, b->moon, b->ctx), b->birth);
  };
  connect(prev_btn, &QPushButton::clicked, &dialog, [step]() { step(false); });
  connect(next_btn, &QPushButton::clicked, &dialog, [step]() { step(true); });
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(hint);
  v->addWidget(buttons);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  if (shown <= 0.0 || dirty) {
    run_lunar();
  }
}

// ported from the PLANETARE case of a16 with plant and planth
void MainWindow::planetar_chart() {
  if (!last_chart_) {
    return;
  }
  const bool helio = current_settings().heliocentric;
  EventPlace place = birth_place();
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Planetar"));
  auto* v = new QVBoxLayout(&dialog);
  auto* form = new QFormLayout();
  auto* bodybox = new QComboBox(&dialog);
  // the names the original stamped on each planet's return
  static constexpr std::pair<int, const char*> kPlanetar[] = {
      {body::kMercury, "MERKURAR"}, {body::kVenus, "VENUSAR"},   {body::kMars, "MARSAR"},
      {body::kJupiter, "JUPITAR"},  {body::kSaturn, "SATURNAR"}, {body::kUranus, "URANAR"},
      {body::kNeptune, "NEPTUNAR"}, {body::kPluto, "PLUTAR"},    {body::kChiron, "CHIRONAR"},
      {body::kCeres, "CERESAR"},    {body::kPallas, "PALLASAR"}, {body::kJuno, "JUNAR"},
      {body::kVesta, "VESTAR"},     {body::kQuaoar, "QUAOARAR"}, {body::kHalley, "HALLEYAR"},
      {body::kPholus, "PHOLUSAR"},  {body::kDamokles, "DAMOKLESAR"}, {body::kNessus, "NESSUSAR"},
      {body::kXena, "XENAR"}};
  // the three extra planets of his final defaults stay available to the
  // family even with the panel switch Zusatz-Planeten off
  const auto always_offer = [](int slot) {
    return slot == body::kChiron || slot == body::kQuaoar || slot == body::kXena;
  };
  for (const auto& [slot, name] : kPlanetar) {
    const BodyState& b = last_chart_->b[static_cast<std::size_t>(slot)];
    if ((b.present && b.valid) || always_offer(slot)) {
      bodybox->addItem(name, slot);
    }
  }
  auto* dir = search_by_box(&dialog);
  auto* nr = return_number_box(&dialog);
  auto* hint = new QLabel(&dialog);
  hint->setWordWrap(true);
  const auto refresh = [dir, nr, hint, bodybox]() {
    const int mode = dir->currentData().toInt();
    nr->setEnabled(mode != kByDate);
    hint->setText(search_hint(mode, bodybox->currentText(), false));
  };
  refresh();
  connect(dir, &QComboBox::currentIndexChanged, &dialog, refresh);
  connect(bodybox, &QComboBox::currentIndexChanged, &dialog, refresh);
  // IF hrg! = 0 : @ort_wahl(0,0), the heliocentric planetar has no place
  if (!helio) {
    v->addLayout(event_place_row(&dialog, &place));
  }
  form->addRow(tr("Planet"), bodybox);
  form->addRow(tr("SUCHEN mit :"), dir);
  form->addRow(tr("Nummer"), nr);
  //RR NICHT SINNVOLL für Horoskope von MENSCHEN !
  auto* note = new QLabel(tr("Die Wiederkehr der langsamen Körper übersteigt ein Menschenleben."), &dialog);
  note->setWordWrap(true);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(hint);
  v->addWidget(note);
  v->addWidget(buttons);
  if (dialog.exec() != QDialog::Accepted || bodybox->count() == 0) {
    return;
  }
  const int slot = bodybox->currentData().toInt();
  // CASE 9,10,n17&,n22&, fanz("NICHT SINNVOLL für Horoskope von MENSCHEN !")
  if (slot == body::kNeptune || slot == body::kPluto || slot == body::kQuaoar || slot == body::kXena) {
    QMessageBox::information(this, tr(" HINWEIS "), tr("NICHT SINNVOLL für Horoskope von MENSCHEN !"));
  }
  SearchContext ctx = make_context();
  // pz = plz(1,ze,pl&), the natal place of the body at the birth place
  ctx.base = radix_input();
  const double birth = julian_day(ctx.base.date_ut, ctx.settings.calendar);
  const BodyLongitude bl = body_longitude(birth, slot, ctx);
  if (!bl.valid) {
    QMessageBox::warning(this, tr("Planetar"),
                         tr("Radix-Koordinate für den gewählten Körper konnte nicht bestimmt werden."));
    return;
  }
  const double radix = bl.el;
  // the search at the place of the event like his ort_wahl
  ctx.base.lon_deg_east = place.lon;
  ctx.base.lat_deg = place.lat;
  const double tja = last_chart_->ta.tropical_year_days;
  const QString name = bodybox->currentText();
  const std::string_view tag_view = body::kName[static_cast<std::size_t>(slot)];
  const QString tag = QString::fromUtf8(tag_view.data(), static_cast<int>(tag_view.size()));
  const int mode = dir->currentData().toInt();
  LongitudeCrossing hit;
  int count = 0;
  if (mode == kByDate) {
    // a37dat(148,"","SUCH-DATUM ?  UMLAUFZEIT " + pl$ + "=" + STR$(ta/tja,7,3) + " JAHRE = " + STR$(ta,7,1) + " TAGE"),
    // the period of the mean motion, his a16_ta made Jupiter to Pluto too long
    const double period = planetar_count_period(slot, tja, true);
    const std::optional<CalendarDate> day =
        ask_date(this, tr("SUCH-DATUM ?  UMLAUFZEIT ") + tag + "=" + QString::asprintf("%7.3f", period / tja) +
                           tr(" JAHRE = ") + QString::asprintf("%7.1f", period) + tr(" TAGE"),
                 today_date());
    if (!day) {
      return;
    }
    // @jseckp, the day at the clock of the radix
    const double seed = event_at_radix_clock(radix_chart(), julian_day({day->day, day->month, day->year, 0, 0.0},
                                                                       ctx.settings.calendar), false);
    // ns1& = FIX((jd - jd(1,ze)) / ta(pl&)). He counted geocentric Mercury
    // and Venus by their revolution where his NR seeds count them by the
    // year, and truncated toward zero so a date before the birth lost one
    count = static_cast<int>(std::floor((seed - birth) / planetar_count_period(slot, tja, helio)));
    hit = find_longitude_backward(seed + kReturnSeedDays, slot, radix, ctx);
  } else {
    hit = planetar_return(birth, slot, radix, nr->value(), mode == kByFuture, ctx);
    // sol$ = STR$(ns1&) + "." + name, the entered number, negative into the past
    count = mode == kByFuture ? nr->value() : -nr->value();
  }
  if (!hit.ok) {
    //RR GÜLTIGKEIT ÜBERSCHRITTEN !
    QMessageBox::information(this, tr(" HINWEIS "), tr("GÜLTIGKEIT ÜBERSCHRITTEN !"));
    return;
  }
  const auto moment_row = [this](const LongitudeCrossing& c) {
    const CalendarDate d = calendar_date(c.jd_ut, current_settings().calendar);
    // datum3$ + "  " + ze$ + " / DIREKTLÄUFIG " or " / RÜCKLÄUFIG "
    return datum3_text(d) + "  " + homise_text((d.hour + d.minute / 60.0) * kDegPerHour, 0) +
           (c.retrograde ? tr(" / RÜCKLÄUFIG ") : tr(" / DIREKTLÄUFIG "));
  };
  for (;;) {
    // planth, the passages of this return walking back in time
    std::vector<LongitudeCrossing> points{hit};
    if (!helio) {
      for (const LongitudeCrossing& c : planetar_earlier(hit, slot, radix, ctx)) {
        points.push_back(c);
      }
    }
    QStringList rows;
    for (const LongitudeCrossing& c : points) {
      rows << moment_row(c);
    }
    // NÄCHSTES DIREKTLÄUFIGES <X> in VERGANGENHEIT SUCHEN ?, helio NÄCHSTES <X>
    rows << (helio ? tr("NÄCHSTES %1 in VERGANGENHEIT SUCHEN ?").arg(name)
                   : tr("NÄCHSTES DIREKTLÄUFIGES %1 in VERGANGENHEIT SUCHEN ?").arg(name))
         << tr("ABBRUCH");
    // " Datensatz : " + name, " <ns1>.<X>  | Ekl. Länge = " + gzg$
    const QString record_name = (QString::fromStdString(record_.surname).trimmed() + " " +
                                 QString::fromStdString(record_.given).trimmed())
                                    .trimmed();
    const int pick = ChoiceDialog::ask(
        this, tr("ZEIT ÜBERNEHMEN oder WEITERE PUNKTE SUCHEN ?"),
        {tr(" Datensatz : ") + record_name, QString(" %1.%2  | ").arg(count).arg(name) + tr("Ekl. Länge = ") + zodiac(radix),
         tr(" Der INTERPOLATIONSFEHLER in ZEIT beträgt meist < 5 Sekunden !")},
        rows, helio ? static_cast<int>(points.size()) : 0);
    if (pick < 0 || pick == rows.size() - 1) {
      return;
    }
    if (pick == static_cast<int>(points.size())) {
      // ns1& - 1, the return before
      --count;
      hit = find_longitude_backward(points.back().jd_ut - 1.0, slot, radix, ctx);
      if (!hit.ok) {
        QMessageBox::information(this, tr(" HINWEIS "), tr("GÜLTIGKEIT ÜBERSCHRITTEN !"));
        return;
      }
      if (!helio) {
        // the direct passage that ends the return before
        while (hit.ok && hit.retrograde) {
          hit = find_longitude_backward(hit.jd_ut - 1.0, slot, radix, ctx);
        }
        if (!hit.ok) {
          return;
        }
      }
      continue;
    }
    const LongitudeCrossing chosen = points[static_cast<std::size_t>(pick)];
    //RR HOROSKOP ANSEHEN ? ZEIT-WERT ÜBERNEHMEN ?
    const int see = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("HOROSKOP ANSEHEN ?"), QString(), tr("ZEIT-WERT ÜBERNEHMEN ?")},
                                      {tr("HOROSKOP ANSEHEN"), tr("ZEIT-WERT ÜBERNEHMEN")}, 0);
    if (see < 0) {
      continue;
    }
    set_panel_place(place);
    // every look writes the same SOLAR row
    apply_moment(chosen.jd_ut, QString("%1.%2").arg(count).arg(name), true);
    if (see == 1) {
      return;
    }
    // the chart until a key, then his menu again
    wart(menu_item::kReturns, true);
  }
}

// ported from the PERSONARE case of a16, the planet first, then the
// place, the chart until a key and his WEITERES PERSONAR box
void MainWindow::personar_chart() {
  const bool helio = current_settings().heliocentric;
  if (!last_chart_ || (!helio && !last_chart_->b[body::kSun].valid)) {
    return;
  }
  static constexpr std::pair<int, const char*> kPersonar[] = {
      {body::kMoon, "MOND-PERS"},     {body::kMercury, "MERKUR-PERS"}, {body::kVenus, "VENUS-PERS"},
      {body::kMars, "MARS-PERS"},     {body::kJupiter, "JUPITER-PERS"}, {body::kSaturn, "SATURN-PERS"},
      {body::kUranus, "URANUS-PERS"}, {body::kNeptune, "NEPTUN-PERS"}, {body::kPluto, "PLUTO-PERS"},
      {body::kChiron, "CHIRON-PERS"}, {body::kCeres, "CERES-PERS"},    {body::kPallas, "PALLAS-PERS"},
      {body::kJuno, "JUNO-PERS"},     {body::kVesta, "VESTA-PERS"},    {body::kQuaoar, "QUAOAR-PERS"},
      {body::kHalley, "HALLEY-PERS"}, {body::kPholus, "PHOLUS-PERS"},  {body::kDamokles, "DAMOKLES-PERS"},
      {body::kNessus, "NESSUS-PERS"}, {body::kXena, "XENA-PERS"}};
  // persst, JA of his closing box comes back here
  for (;;) {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Personar"));
    auto* v = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout();
    auto* bodybox = new QComboBox(&dialog);
    for (const auto& [slot, name] : kPersonar) {
      const BodyState& b = last_chart_->b[static_cast<std::size_t>(slot)];
      // the moon slot carries the Earth under hrg!
      if (b.present && b.valid && !(helio && slot == body::kMoon)) {
        bodybox->addItem(name, slot);
      }
    }
    form->addRow(tr("Planet"), bodybox);
    auto* note = new QLabel(tr("Das Personar ist der Lauf der Sonne über den Radix-Stand des gewählten Planeten im ersten Lebensjahr."), &dialog);
    note->setWordWrap(true);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    v->addLayout(form);
    v->addWidget(note);
    v->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted || bodybox->count() == 0) {
      return;
    }
    const int slot = bodybox->currentData().toInt();
    const QString label = bodybox->currentText();
    SearchContext ctx = make_context();
    // pz = el(plp&) of the radix, under hrg! hel(plp&)
    ctx.base = radix_input();
    const double birth = julian_day(ctx.base.date_ut, ctx.settings.calendar);
    const double tja = last_chart_->ta.tropical_year_days;
    const BodyLongitude bl = body_longitude(birth, slot, ctx);
    if (!bl.valid) {
      QMessageBox::warning(this, tr("Personar"),
                           tr("Radix-Koordinate für den gewählten Körper konnte nicht bestimmt werden."));
      return;
    }
    // IF hrg! = 0 : @ort_wahl(0,0), the personar stands at the event place
    std::optional<EventPlace> place;
    if (!helio) {
      place = ask_event_place();
      if (!place) {
        return;
      }
      ctx.base.lon_deg_east = place->lon;
      ctx.base.lat_deg = place->lat;
    }
    //RR STARTWERT
    double start = birth + tja;
    // a16korr, EIN JAHR VORWÄRTS and ZURÜCK search again from here
    for (;;) {
      // plant(jdz,1,pz), under hrg! el(1) = hel(1), the Earth runs to the
      // heliocentric radix place of the body
      const LongitudeCrossing hit = find_longitude_backward(start, helio ? body::kMoon : body::kSun, bl.el, ctx);
      if (!hit.ok) {
        banner_->set_record(tr("Kein Personar gefunden"));
        return;
      }
      if (place) {
        set_panel_place(*place);
      }
      apply_moment(hit.jd_ut, label, true);
      wart(menu_item::kReturns, true);
      //RR WEITERES PERSONAR ERSTELLEN ?
      const int ret = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("WEITERES PERSONAR ERSTELLEN ?")},
                                        {tr("JA"), tr("NEIN"), tr("EIN JAHR VORWÄRTS ( Für GRENZFÄLLE ! )"),
                                         tr("EIN JAHR ZURÜCK ( Für GRENZFÄLLE ! )")},
                                        0);
      if (ret == 0) {
        break;
      }
      if (ret == 2) {
        // jdz = jd(1,ze) + 2 * tja
        start = birth + 2.0 * tja;
      } else if (ret == 3) {
        // jdz = jd(1,ze) + 3
        start = birth + 3.0;
      } else {
        return;
      }
    }
  }
}

// ported from the SOLAR and LUNAR list outputs of a16. The solar list
// starts at the ERSTES KALENDER-Jahr, the lunar list at the BEGINN-DATUM,
// both run eighty four returns forward
void MainWindow::return_list(bool lunar) {
  if (!last_chart_) {
    return;
  }
  SearchContext ctx = make_context();
  const bool helio = ctx.settings.heliocentric;
  // hrg! has no LUNAR, the SOLAR becomes the TERRAR
  if (helio && lunar) {
    return;
  }
  if (!helio && !last_chart_->b[body::kSun].valid) {
    return;
  }
  ctx.base = radix_input();
  const double birth = julian_day(ctx.base.date_ut, ctx.settings.calendar);
  // pz = plz(1,ze,pl&), the radix at the birth place, under hrg! the
  // Earth on slot 2
  const int slot = helio || lunar ? body::kMoon : body::kSun;
  const BodyLongitude natal = body_longitude(birth, slot, ctx);
  if (!natal.valid) {
    return;
  }
  // IF hrg! = 0 AND par = 1 : @ort_wahl(0,0), the place only matters topocentric
  std::optional<EventPlace> event;
  if (!helio && ctx.settings.topocentric_parallax) {
    event = ask_event_place();
    if (!event) {
      return;
    }
    ctx.base.lon_deg_east = event->lon;
    ctx.base.lat_deg = event->lat;
  }
  int first_year = 0;
  double first_jd = 0.0;
  if (lunar) {
    //RR BEGINN - DATUM EINGEBEN !
    const std::optional<CalendarDate> start = ask_date(this, tr("BEGINN - DATUM EINGEBEN !"), today_date());
    if (!start) {
      return;
    }
    first_jd = julian_day({start->day, start->month, start->year, 0, 0.0}, ctx.settings.calendar);
  } else {
    //RR ERSTES KALENDER-Jahr 'JJJJ' EINGEBEN !
    const std::optional<double> year = ask_number(this, tr("ZAHLEN-Eingabe !"), tr("ERSTES KALENDER-Jahr 'JJJJ' EINGEBEN !"),
                                                  -3000.0, 3000.0, QDate::currentDate().year(), 0);
    if (!year) {
      return;
    }
    first_year = static_cast<int>(*year);
  }
  // tabelle! = -1, IF prenbl& : moda& = @druck_graph_ein, BILDSCHIRM or DIN A5
  int moda = ask_graphic_output(menu_item::kReturns, false);
  if (moda == kOutputNone) {
    return;
  }
  std::unique_ptr<QPrinter> printer;
  if (moda == kOutputA5) {
    printer = std::make_unique<QPrinter>(QPrinter::HighResolution);
    if (!prepare_printer(this, *printer, PrintPage::kGraphicA5, konsta_.halbs != 0)) {
      printer_failed(this);
      return;
    }
    //RR Druck beginnt erst nach Berechnung der KOMPLETTEN Liste !
    const int re = ChoiceDialog::ask(this, tr("!! ACHTUNG !!"), {tr("Druck beginnt erst nach Berechnung der KOMPLETTEN Liste ! ")},
                                     {tr(" WEITER ( Bitte Geduld üben ! Druck beginnt nach Signalton )"),
                                      tr(" Ausgabe auf BILDSCHIRM "), tr("Zurück zum HAUPT - MENÜ")},
                                     0);
    if (re == 1) {
      moda = kOutputScreen;
    } else if (re != 0) {
      return;
    }
  }
  const double tja = last_chart_->ta.tropical_year_days;
  const CalendarDate b = ctx.base.date_ut;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::vector<double> moments;
  // the REPEAT of sol_lun_tabelle, each search runs back from its seed
  // and the next seed steps on from the return found
  double jd = 0.0;
  double step = 0.0;
  if (lunar) {
    // SUB jd,29 from the BEGINN-DATUM, then ADD jd,33
    jd = first_jd - kLunarListLeadDays;
    step = kLunarListStepDays;
  } else {
    // a16_1, the birthday clock of the first year, SUB jd,tja, then
    // ADD jd,tja + 40
    jd = julian_day({b.day, b.month, first_year, b.hour, b.minute}, ctx.settings.calendar) - tja;
    step = tja + kSolarListStepDays;
  }
  for (int n = 1; n <= kReturnListRows; ++n) {
    jd += step;
    const LongitudeCrossing hit = find_longitude_backward(jd, slot, natal.el, ctx);
    if (!hit.ok) {
      break;
    }
    moments.push_back(hit.jd_ut);
    jd = hit.jd_ut;
    if (calendar_date(hit.jd_ut, ctx.settings.calendar).year - b.year > kReturnListYears) {
      break;
    }
  }
  QApplication::restoreOverrideCursor();

  // sol$ = "SOLAR", "LUNAR" or under hrg! "TERRAR"
  const QString sol = helio ? QStringLiteral("TERRAR") : (lunar ? QStringLiteral("LUNAR") : QStringLiteral("SOLAR"));
  const QString name = (QString::fromStdString(record_.surname).trimmed() + " " +
                        QString::fromStdString(record_.given).trimmed())
                           .trimmed();
  // p$ = " | Ereignis-Ort : " + go$, o$ = " |Geb.-Ort : " + go$(1,ze), both empty under hrg!
  const QString event_name = event ? QString::fromStdString(event->name).trimmed() : QString();
  const QString p = event_name.isEmpty() ? QString() : tr(" | Ereignis-Ort : ") + event_name;
  const QString o = helio ? QString() : tr(" |Geb.-Ort : ") + QString::fromStdString(birth_place().name).trimmed();
  const CalendarDate bd = calendar_date(birth, ctx.settings.calendar);
  // dat$ = datum3$ + " ", zeit$ = ze$
  const QString dat = datum3_text(bd) + " ";
  const QString zeit = homise_text((bd.hour + bd.minute / 60.0) * kDegPerHour, 0);
  // g$ = " Heliozentrische " + gena2$ or " Ephemeride : " + gena4$
  const QString g = helio ? tr(" Heliozentrische ") + gena2_text() : tr(" Ephemeride : ") + gena4_text();
  const QString jul = jul_mark(panel_calendar_);
  std::vector<QString> rows;
  for (const double m : moments) {
    const CalendarDate d = calendar_date(m, ctx.settings.calendar);
    rows.push_back(" " + datum3_text(d) + " " + homise_text((d.hour + d.minute / 60.0) * kDegPerHour, 0));
  }
  const DisplayList sheet = return_sheet(" " + sol + tr(" - Zeitpunkte für ") + name + p,
                                         tr(" Geburts-Datum : ") + dat + zeit + o,
                                         g + tr(" | Zeit in UT = GMT") + " " + jul, rows);
  if (moda == kOutputA5) {
    // VIEWPORT gdxp& / 20,gdyp& / 90, @sol_lun_tabelle, @drad2, BEEP
    QPainter pen(printer.get());
    if (!pen.isActive()) {
      printer_failed(this);
      return;
    }
    paint_robert_page(pen, sheet, robert_page_rect(*printer, page_margin::kLeft, page_margin::kTop, 1.0));
    pen.end();
    QApplication::beep();
    return;
  }
  QDialog dialog(this);
  mark_output(&dialog, menu_item::kReturns);
  // _WIN$(WIN(win&)) = sol$(2,ze)
  dialog.setWindowTitle(sol);
  QVariantList moment_list;
  for (const double m : moments) {
    moment_list << m;
  }
  dialog.setProperty("moments", moment_list);
  auto* v = new QVBoxLayout(&dialog);
  v->setContentsMargins(0, 0, 0, 0);
  auto* canvas = new WheelWidget(&dialog);
  canvas->set_plain_list(sheet);
  v->addWidget(canvas);
  // a click on a moment takes it into the panel, an addition of the port,
  // his table only stood until a key. The rows stand centred on the
  // bottom line 46 + 14 i of textc, 45 to 59 for the first
  const double period = lunar ? kTropicalMonthDays : tja;
  LambdaFilter click([&](QEvent* e) {
    if (e->type() == QEvent::KeyPress) {
      dialog.reject();
      return true;
    }
    if (e->type() != QEvent::MouseButtonPress) {
      return false;
    }
    const QPointF at = canvas->to_canvas(static_cast<QMouseEvent*>(e)->position());
    const int column = at.x() < 212.0 ? 0 : (at.x() < 422.0 ? 1 : 2);
    const int line = static_cast<int>(std::floor((at.y() - 45.0) / 14.0));
    const int index = column * kReturnColumnRows + line;
    if (line >= 0 && line < kReturnColumnRows && index < static_cast<int>(moments.size())) {
      const double m = moments[static_cast<std::size_t>(index)];
      set_panel_place(event ? *event : birth_place());
      // solnummer, the count of the return before sol$
      apply_moment(m, QString("%1.%2").arg(return_number(m, birth, period)).arg(sol), true);
      dialog.accept();
    }
    return true;
  });
  dialog.installEventFilter(&click);
  canvas->installEventFilter(&click);
  dialog.resize(size());
  dialog.exec();
}

// ported from the SOLAR case of a16 under hrg!, the TERRAR. The Earth
// returns to its heliocentric radix place, no place question, the year
// by number
void MainWindow::terrar_chart() {
  if (!last_chart_) {
    return;
  }
  SearchContext ctx = make_context();
  ctx.base = radix_input();
  // j$ = @inputbox$(160,eaz$,"GEWÜNSCHTES KALENDER-JAHR 'JJJJ' EINGEBEN !","")
  const std::optional<double> year = ask_number(this, tr("ZAHLEN-Eingabe !"),
                                                tr("GEWÜNSCHTES KALENDER-JAHR 'JJJJ' EINGEBEN !"), -3000.0, 3000.0,
                                                std::numeric_limits<double>::quiet_NaN(), 0);
  if (!year) {
    return;
  }
  const double birth = julian_day(ctx.base.date_ut, ctx.settings.calendar);
  // pz = plz(1,ze,2), the heliocentric Earth of the radix
  const BodyLongitude natal = body_longitude(birth, body::kMoon, ctx);
  if (!natal.valid) {
    return;
  }
  const double tja = last_chart_->ta.tropical_year_days;
  const CalendarDate b = ctx.base.date_ut;
  int ja = static_cast<int>(*year);
  // a16_1, the birthday clock in the year ja
  double jd = julian_day({b.day, b.month, ja, b.hour, b.minute}, ctx.settings.calendar);
  for (;;) {
    // jdz = jd + 15, plant(jdz,1,pz)
    const LongitudeCrossing hit = find_longitude_backward(jd + kReturnSeedDays, body::kMoon, natal.el, ctx);
    if (!hit.ok) {
      banner_->set_record(tr("Kein Terrar gefunden"));
      return;
    }
    // solnummer, sol$(2,ze) = STR$(n) + "." + "TERRAR"
    apply_moment(hit.jd_ut, QString("%1.TERRAR").arg(return_number(hit.jd_ut, birth, tja)), true);
    wart(menu_item::kReturns, true);
    //RR WEITERES SOLAR SUCHEN ? bei GLEICHEM ORT ?
    const int ret = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("WEITERES SOLAR SUCHEN ?"), tr("bei GLEICHEM ORT ?")},
                                      {tr("HOROSKOP für   FOLGENDES    Jahr ?"), tr("HOROSKOP für VORHERGEHENDES Jahr ?"),
                                       tr("Weiter ( = ENDE )")},
                                      2);
    if (ret == 0) {
      ++ja;
    } else if (ret == 1) {
      --ja;
    } else {
      return;
    }
    // a16sol with a16_1 and jseckp, the birthday clock of ja again
    jd = julian_day({b.day, b.month, ja, b.hour, b.minute}, ctx.settings.calendar);
  }
}

}  // namespace horcom
