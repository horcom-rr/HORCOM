// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// INGRESSE SONNE-MOND-MC-AC, his ingre with ingre1, ort_parall,
// ingr_ort and finst_a.

#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <limits>

#include "calendar_mark.hpp"
#include "choice_dialog.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/signs.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/wheel.hpp"
#include "main_window.hpp"
#include "paging_keys.hpp"
#include "record_mask_dialog.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"
#include "theme.hpp"
#include "zodiac_cells.hpp"

namespace horcom {

namespace {

// his el + 0.0000001 before grzemise
//RR vermeide 29/59/60
constexpr double kAvoidCarry = 1.0e-7;
// the day columns of the MC and AC tables
constexpr int kAngleDays = 3;
// the MOND table steps a month forward and a little less back
constexpr double kMoonForward = 29.0;
constexpr double kMoonBack = 28.0;
// the MC and AC tables step three days, his +1 after the table and -5
constexpr double kAngleForward = 1.0;
constexpr double kAngleBack = 5.0;

enum class Kind { kSun, kMoon, kMc, kAc, kBody };

struct Choice {
  Kind kind;
  int slot;
  const char* caption;
};

// his ze$() rows SONNE, MOND, MC and AC, the tester's bodies before the
// ABBRUCH
constexpr Choice kChoices[] = {
    {Kind::kSun, body::kSun, QT_TRANSLATE_NOOP("horcom::MainWindow", "SONNE")},
    {Kind::kMoon, body::kMoon, QT_TRANSLATE_NOOP("horcom::MainWindow", "MOND")},
    {Kind::kMc, body::kMc, QT_TRANSLATE_NOOP("horcom::MainWindow", "MC")},
    {Kind::kAc, body::kAscendant, QT_TRANSLATE_NOOP("horcom::MainWindow", "AC")},
    {Kind::kBody, body::kMercury, QT_TRANSLATE_NOOP("horcom::MainWindow", "MERKUR")},
    {Kind::kBody, body::kVenus, QT_TRANSLATE_NOOP("horcom::MainWindow", "VENUS")},
    {Kind::kBody, body::kMars, QT_TRANSLATE_NOOP("horcom::MainWindow", "MARS")},
    {Kind::kBody, body::kJupiter, QT_TRANSLATE_NOOP("horcom::MainWindow", "JUPITER")},
    {Kind::kBody, body::kSaturn, QT_TRANSLATE_NOOP("horcom::MainWindow", "SATURN")},
    {Kind::kBody, body::kUranus, QT_TRANSLATE_NOOP("horcom::MainWindow", "URANUS")},
    {Kind::kBody, body::kNeptune, QT_TRANSLATE_NOOP("horcom::MainWindow", "NEPTUN")},
    {Kind::kBody, body::kPluto, QT_TRANSLATE_NOOP("horcom::MainWindow", "PLUTO")},
    {Kind::kBody, body::kChiron, QT_TRANSLATE_NOOP("horcom::MainWindow", "CHIRON")},
    {Kind::kBody, body::kQuaoar, QT_TRANSLATE_NOOP("horcom::MainWindow", "QUAOAR")},
    {Kind::kBody, body::kXena, QT_TRANSLATE_NOOP("horcom::MainWindow", "XENA")},
};

// his grzemise with degree mark and seconds, from el + 1e-7
QTableWidgetItem* longitude_cell(double rad) {
  return zodiac_item(norm_rad(rad + kAvoidCarry), true, true);
}

// ze$ of zeit_form, homise with whole seconds
QString clock_text(double jd_ut, Calendar cal) {
  const CalendarDate d = calendar_date(jd_ut, cal);
  return homise_text((d.hour + d.minute / 60.0) * kDegPerHour, 0);
}

}  // namespace

// ported from ingre with ingre1, ort_parall, ingr_ort and finst_a
void MainWindow::ingress_table() {
  QStringList rows;
  for (const Choice& c : kChoices) {
    rows << tr(c.caption);
  }
  rows << tr("ABBRUCH");
  // his ue$(0)
  const int es = ChoiceDialog::ask(this, tr("OBJEKT für INGRESSE WÄHLEN !"), {}, rows, 0);
  if (es < 0 || es >= static_cast<int>(std::size(kChoices))) {
    return;
  }
  const Choice choice = kChoices[es];
  SearchContext ctx = make_context();
  // his CLR hrg!, the ingresses are geocentric
  ctx.settings.heliocentric = false;
  const Calendar cal = ctx.settings.calendar;
  const AafRecord record = panel_record();
  const ChartInput now = current_input();
  bool show_place = false;
  QString place = QString::fromStdString(record.place);
  double jd = 0.0;
  const bool angle = choice.kind == Kind::kMc || choice.kind == Kind::kAc;
  if (!angle) {
    // his ort_parall, IF par = 1
    if (ctx.settings.topocentric_parallax) {
      AafRecord seed = record;
      RecordMaskDialog mask(seed, tr(" Wegen PARALLAXE bitte Ereignis-Ort eingeben !"), RecordMaskDialog::Mode::kShow,
                            data_dir_, this);
      mask.limit_fields(10);
      if (mask.exec() != QDialog::Accepted) {
        return;
      }
      const ChartInput at = record_input(mask.record());
      ctx.base.lon_deg_east = at.lon_deg_east;
      ctx.base.lat_deg = at.lat_deg;
      place = QString::fromStdString(mask.record().place);
      show_place = true;
    }
    if (choice.kind == Kind::kSun) {
      // his inputbox "KALENDERJAHR ( JJJJ ) EINGEBEN !"
      const std::optional<double> year = ask_number(this, tr("KALENDERJAHR ( JJJJ ) EINGEBEN !"), QString(), -4000.0,
                                                    9999.0, std::numeric_limits<double>::quiet_NaN(), 0);
      if (!year) {
        return;
      }
      // his mo = 1, ta = 1 at the clock of the record
      jd = julian_day({1, 1, static_cast<int>(*year), now.date_ut.hour, now.date_ut.minute}, cal);
    } else {
      // his a37dat "SUCH-DATUM ( MONAT ) EINGEBEN !", the clock stays
      const std::optional<CalendarDate> day = ask_date(this, tr("SUCH-DATUM ( MONAT ) EINGEBEN !"), now.date_ut);
      if (!day) {
        return;
      }
      jd = julian_day({day->day, day->month, day->year, now.date_ut.hour, now.date_ut.minute}, cal);
    }
  } else {
    // his ze = 0, od = 0 and @eingabe(-1,-1,1,14), date and place of the table
    RecordMaskDialog mask(record, tr("EINGABE"), RecordMaskDialog::Mode::kShow, data_dir_, this);
    mask.limit_fields(14);
    if (mask.exec() != QDialog::Accepted) {
      return;
    }
    const AafRecord r = mask.record();
    const ChartInput at = record_input(r);
    ctx.base.lon_deg_east = at.lon_deg_east;
    ctx.base.lat_deg = at.lat_deg;
    place = QString::fromStdString(r.place);
    show_place = true;
    const CalendarDate d = calendar_date(julian_day(at.date_ut, cal), cal);
    jd = julian_day({d.day, d.month, d.year, 0.0, 0.0}, cal);
  }

  QDialog view(this);
  mark_output(&view, menu_item::kIngresses);
  // the caption of his menu
  view.setWindowTitle(tr("INGRESSE SONNE-MOND-MC-AC"));
  auto* v = new QVBoxLayout(&view);
  auto* title = new QLabel(&view);
  title->setAlignment(Qt::AlignHCenter);
  v->addWidget(title);
  auto* table = new QTableWidget(&view);
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setItemDelegate(new ZodiacDelegate(table));
  table->setFont(theme::mono_font());
  v->addWidget(table, 1);
  auto* place_line = new QLabel(&view);
  place_line->setAlignment(Qt::AlignHCenter);
  v->addWidget(place_line);
  auto* ephem_line = new QLabel(&view);
  ephem_line->setAlignment(Qt::AlignHCenter);
  v->addWidget(ephem_line);
  // his finst_a
  auto* foot = new QLabel(tr("Weiter mit Leertaste  |  Zurück mit 'R' |  Ende mit 'ESC'"), &view);
  foot->setAlignment(Qt::AlignHCenter);
  v->addWidget(foot);
  const QString jul = jul_mark(panel_calendar_);
  const QString gena4 = gena4_text();
  const std::string_view tag_view = body::kName[static_cast<std::size_t>(choice.slot)];
  const QString tag = QString::fromUtf8(tag_view.data(), static_cast<qsizetype>(tag_view.size()));
  std::vector<double> shown_jd;
  // plant leaves his jd on the last hit of ingre1, the PISCES ingress
  double last_hit = 0.0;
  // the tester's bodies page one circuit of the zodiac, his month of the
  // MOND would hardly move a slow body, Mercury and Venus circle with the
  // Sun in a year
  const double tja = time_arguments(jd).tropical_year_days;
  const double circuit = std::max(tja, body_period_days(choice.slot, tja));

  const auto fill = [&]() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    table->clear();
    shown_jd.clear();
    const CalendarDate d = calendar_date(jd, cal);
    // his ingr_ort
    place_line->setText(tr(" Ereignis-Ort : %1 | Länge : %2 | Breite : %3")
                            .arg(place.trimmed())
                            .arg(ctx.base.lon_deg_east, 0, 'f', 3)
                            .arg(ctx.base.lat_deg, 0, 'f', 3));
    place_line->setVisible(show_place);
    if (!angle) {
      if (choice.kind == Kind::kSun) {
        title->setText(tr(" Ingresse der SONNE im Kalenderjahr %1").arg(d.year));
      } else if (choice.kind == Kind::kMoon) {
        title->setText(tr(" Ingresse des MONDES bis ca. dem Datum : %1").arg(datum3_text(d)));
      } else {
        title->setText(tr(" Ingresse von %1 bis ca. dem Datum : %2").arg(tr(choice.caption), datum3_text(d)));
      }
      // his " Ephemeride : " + gena4$ + " " + jul$
      ephem_line->setText(tr(" Ephemeride : ") + gena4 + " " + jul);
      ephem_line->setVisible(true);
      table->setColumnCount(3);
      table->setRowCount(kSignCount);
      // his "Datum         UT = GMT      Länge " + pl$
      table->setHorizontalHeaderLabels({tr("Datum"), tr("UT = GMT"), tr("Länge ") + tag});
      const auto hits = sign_ingresses(jd, choice.slot, ctx);
      for (int t = 0; t < kSignCount; ++t) {
        const LongitudeCrossing& hit = hits[static_cast<std::size_t>(t)];
        shown_jd.push_back(hit.ok ? hit.jd_ut : 0.0);
        if (!hit.ok) {
          continue;
        }
        table->setItem(t, 0, new QTableWidgetItem(datum3_text(calendar_date(hit.jd_ut, cal))));
        table->setItem(t, 1, new QTableWidgetItem(clock_text(hit.jd_ut, cal)));
        const BodyLongitude bl = body_longitude(hit.jd_ut, choice.slot, ctx);
        table->setItem(t, 2, longitude_cell(bl.valid ? bl.el : kEps + t * kPi / 6.0));
      }
      // a missing PISCES ingress leaves the page on its own date
      last_hit = hits.back().ok ? hits.back().jd_ut : 0.0;
    } else {
      // his " Ingresse des MC ab dem Datum : " + datum3$ + " " + jul$
      title->setText((choice.kind == Kind::kMc ? tr(" Ingresse des MC ab dem Datum : %1") : tr(" Ingresse des AC ab dem Datum : %1"))
                         .arg(datum3_text(d) + " " + jul));
      ephem_line->setVisible(false);
      table->setColumnCount(kAngleDays + 1);
      table->setRowCount(kSignCount);
      QStringList heads;
      for (int c = 0; c < kAngleDays; ++c) {
        heads << tr("  Datum     UT = GMT ");
      }
      heads << tr("Länge ") + tag;
      table->setHorizontalHeaderLabels(heads);
      for (int c = 0; c < kAngleDays; ++c) {
        const auto hits = angle_ingresses(jd + c, choice.slot, ctx);
        for (int t = 0; t < kSignCount; ++t) {
          const LongitudeCrossing& hit = hits[static_cast<std::size_t>(t)];
          if (c == 0) {
            shown_jd.push_back(hit.ok ? hit.jd_ut : 0.0);
          }
          if (!hit.ok) {
            continue;
          }
          if (c == 0) {
            // his el(13) = f(1) bzw. el(14) = f(10) at the hit, the Länge
            // only in the first day column
            const Chart at = sky_chart(hit.jd_ut, ctx);
            if (at.ok && at.houses.ok) {
              const double axis = choice.kind == Kind::kAc ? at.houses.angles.ac : at.houses.angles.mc;
              table->setItem(t, kAngleDays, longitude_cell(axis));
            }
          }
          // his datum$ + "  " + ze$, the two digit year
          table->setItem(t, c, new QTableWidgetItem(datum_text(calendar_date(hit.jd_ut, cal)) + "  " + clock_text(hit.jd_ut, cal)));
        }
      }
    }
    table->resizeColumnsToContents();
    QApplication::restoreOverrideCursor();
  };
  const auto step = [&](bool forward) {
    switch (choice.kind) {
      case Kind::kSun: {
        // his ADD jd,365 from the first of January landed on the last of
        // December after a leap year and showed that year once more, the
        // port opens the next calendar year
        const int y = calendar_date(jd, cal).year + (forward ? 1 : -1);
        jd = julian_day({1, 1, y, now.date_ut.hour, now.date_ut.minute}, cal);
        break;
      }
      case Kind::kMc:
      case Kind::kAc:
        // the table stood on D to D+2, his ADD jd,1 or SUB jd,5 from there
        jd += forward ? (kAngleDays - 1) + kAngleForward : (kAngleDays - 1) - kAngleBack;
        break;
      case Kind::kMoon:
        // his ADD jd,29 bzw. SUB jd,28 count from the PISCES ingress
        jd = (last_hit > 0.0 ? last_hit : jd) + (forward ? kMoonForward : -kMoonBack);
        break;
      case Kind::kBody:
        jd += forward ? circuit : -circuit;
        break;
    }
    fill();
  };
  PagingKeys keys([&step]() { step(true); }, [&step]() { step(false); });
  view.installEventFilter(&keys);
  table->installEventFilter(&keys);
  // a double click hands the moment to the transit view, the tester's way on
  double chosen = 0.0;
  connect(table, &QTableWidget::cellDoubleClicked, &view, [&](int row, int) {
    if (row >= 0 && row < static_cast<int>(shown_jd.size()) && shown_jd[static_cast<std::size_t>(row)] > 0.0) {
      chosen = shown_jd[static_cast<std::size_t>(row)];
      view.accept();
    }
  });
  fill();
  view.resize(angle ? 900 : 640, 560);
  if (view.exec() == QDialog::Accepted && chosen > 0.0) {
    show_moment_transits(chosen);
  }
}

}  // namespace horcom
