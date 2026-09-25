// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// STATISTIK, the output of stat2. The pages of list_ausg with warts and
// zeilklick, the row box of kotab_sta and the charts it shows with the
// counter windows.

#include <QApplication>
#include <QDialog>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

#include "choice_dialog.hpp"
#include "event_filter.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/stat_sheet.hpp"
#include "main_window_statist.hpp"
#include "print_pages.hpp"
#include "theme.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

// his S and A in place of the glyph
constexpr const char* kMirrorTag = "S";
constexpr const char* kArabicTag = "A";
// CHR$(4), the diamond of his OEM font before NIETE!
constexpr const char* kNieteMark = " \xE2\x99\xA6 NIETE!";
// the pages 'S' skips, his kl& + 120 at the end of a page and the next
// one after it
constexpr int kSkipPages = 6;
// his zdm& > 145, the second footer line
constexpr std::size_t kSkipFooterRows = 145;
// the band of rows zeilklick reads on the 640 by 459 screen
constexpr double kRowTop = 32.0;
constexpr double kRowBottom = 416.0;
constexpr double kRowHeight = 16.0;

// ported from kltext, the moment lines of a row
void moment_lines(const StatRecord& r, bool small, bool helio, StatSheetRow& row) {
  const int minute = static_cast<int>(r.minute);
  if (!small) {
    // STR$(ta,2)+"."+STR$(mo,2)+"."+STR$(ja,5)+"  "+STR$(ho,2)+" h "+STR$(mi,2)+"'"
    row.moment = QString::asprintf("%2d.%2d.%5d  %2d h %2d'", r.day, r.month, r.year, r.hour, minute).toStdString();
    return;
  }
  // "' L="+STR$(gl,7,2)+" B="+STR$(gg,6,2)
  row.moment = QString::asprintf("%2d.%2d.%5d %2dh%2d' L=%7.2f B=%6.2f", r.day, r.month, r.year, r.hour, minute,
                                 r.lon, r.lat)
                   .toStdString();
  const double p = kRadToDeg;
  if (!helio) {
    row.moment2 = QString::asprintf("SO=%5.1f MO=%5.1f AC=%5.1f MC=%5.1f", p * r.el[body::kSun],
                                    p * r.el[body::kMoon], p * r.ac, p * r.mc)
                      .toStdString();
  } else {
    row.moment2 = QString::asprintf("TE=%5.1f", p * r.el[body::kMoon]).toStdString();
  }
}

// a11_1 with @stop, the sheet until a key or a mouse button, the yellow
// counter windows of anzeigen_halbs_zaehl and anzeigen_asp_zaehl beside
// it when given
void show_sheet(QWidget* owner, const QString& title, const DisplayList& sheet, const QString& midpoints,
                const QString& aspects) {
  QDialog view(owner);
  mark_output(&view, menu_item::kStatistics);
  view.setWindowTitle(title);
  auto* grid = new QGridLayout(&view);
  grid->setContentsMargins(0, 0, 0, 0);
  auto* wheel = new WheelWidget(&view);
  // the screen sheet of sheet_wheel, centred with its corners on the
  // edges like every other preview
  wheel->set_display_list(sheet);
  grid->addWidget(wheel, 0, 0, 3, 3);
  const auto box = [&](const QString& text, int row, int col, Qt::Alignment al) {
    auto* l = new QLabel(text, &view);
    l->setFont(theme::mono_font(8));
    // RGBCOLOR RGB(0,0,0),RGB(255,255,0)
    l->setStyleSheet("QLabel { background: #ffff00; color: #000000; border: 2px solid #808080; padding: 2px; }");
    grid->addWidget(l, row, col, al);
  };
  if (!midpoints.isEmpty()) {
    box(midpoints, 0, 2, Qt::AlignTop | Qt::AlignRight);
  }
  if (!aspects.isEmpty()) {
    box(aspects, 2, 0, Qt::AlignBottom | Qt::AlignLeft);
  }
  // @stop, a key or a mouse button
  LambdaFilter stop([&view](QEvent* e) {
    if (e->type() == QEvent::KeyPress || e->type() == QEvent::MouseButtonPress) {
      view.accept();
      return true;
    }
    return false;
  });
  view.installEventFilter(&stop);
  wheel->installEventFilter(&stop);
  view.resize(owner->size());
  view.exec();
}

// f$ of asp_halbs_zaehler, the fourth answer of the counter switch,
// the whole file after OHNE EINSCHRÄNKUNG or NAME
QString whole_counter_text(int suc, int obj, const std::filesystem::path& sta) {
  return (suc == kStatSucAll || obj == kStatObjName)
             ? MainWindow::tr("AUSWERTUNG des ZÄHLERS für die GESAMTE DATEI : ") + stat_file_label(sta)
             : MainWindow::tr("AUSWERTUNG des ZÄHLERS für die GESAMTE LISTE");
}

}  // namespace

// ported from the NICHT ÜBERNEHMEN branch of kotab_sta, a11_1 after
// stat2parl while the panel keeps its own record
void MainWindow::stat_view(const StatSession& ss, int record, bool counters) {
  const StatRecord& r = ss.set.records[static_cast<std::size_t>(record)];
  // @stat2parl(datr$), then @a11_1
  const ChartSettings s = stat_chart_settings(konsta_, ss.set.params, ss.helio);
  const std::optional<DisplayList> sheet = sheet_wheel(stat_aaf_record(r), stat_input(r), s);
  if (!sheet) {
    return;
  }
  QString mid_text;
  QString asp_text;
  if (counters) {
    counter_texts(ss.counts, mid_text, asp_text, true);
    // anzeigen_halbs_zaehl only under voll!
    if (!konsta_.voll) {
      mid_text.clear();
    }
  }
  show_sheet(this, QString::fromStdString(r.name).trimmed(), *sheet, mid_text, asp_text);
}

// the counting of a11_1 under zaehl_asp_halbs& 2 and 4, one dataset
// chart into the sums
void MainWindow::stat_count(StatSession& ss, int record) const {
  const ChartSettings s = stat_chart_settings(konsta_, ss.set.params, ss.helio);
  const Chart c = compute_chart(stat_input(ss.set.records[static_cast<std::size_t>(record)]), s, vsop_, eph_);
  if (c.ok) {
    count_chart(ss.counts, c, scan_aspects(c, s, shown_aspect_settings()), s);
  }
}

// ported from kotab_sta, the box of a clicked row
bool MainWindow::stat_row(StatSession& ss, int record) {
  // u$ = " ÜBERNEHMEN", "HOROSKOP nur ANSCHAUEN ?","oder" + das$ + u$,"für WEITERE BERECHNUNGEN ?"
  const int c = ChoiceDialog::ask(this, tr("AUSWAHL"),
                                  {tr("HOROSKOP nur ANSCHAUEN ?"), tr("oderDatensatz  ÜBERNEHMEN"),
                                   tr("für WEITERE BERECHNUNGEN ?")},
                                  {tr("NICHT  ÜBERNEHMEN"), tr(" ÜBERNEHMEN"), tr("ABBRUCH")}, 0);
  if (c < 0 || c == 2) {
    return false;
  }
  const StatRecord& r = ss.set.records[static_cast<std::size_t>(record)];
  if (c == 0) {
    // IF NOT(zaehl_asp_halbs& > 0 OR halbszeitwaus!) : @asp_halbs_zaehler
    if (stat_counter_mode_ == 0 && !counters_off_session_) {
      const int mode = counter_switch(true, whole_counter_text(ss.suc, ss.obj, ss.sta));
      stat_counter_mode_ = mode > 0 ? mode : 3;
      if (mode >= 1 && mode <= 3) {
        ss.counts = WanderCounts{};
        // @einzel_plan_display, ABBRUCH there only redraws and the chart
        // follows all the same
        static_cast<void>(single_planet_choice(3));
      }
    }
    // a11_1 counts under zaehl_asp_halbs& 2 and shows the windows. The
    // click that chose 4 shows its chart plain, the list counts after it
    const bool counting = stat_counter_mode_ == 2 && !counters_off_session_;
    if (counting) {
      stat_count(ss, record);
    }
    stat_view(ss, record, counting);
    return true;
  }
  // zmsp! = @a2113_1(zmsp), every takeover claims a RADIX slot
  const int slot = claim_radix_slot();
  if (slot < 0) {
    return true;
  }
  AafRecord a = stat_aaf_record(r);
  // bem$(1,ze) = "DATEN AUS 'STATISTIK'-DATEI ÜBERNOMMEN"
  a.comment = "DATEN AUS 'STATISTIK'-DATEI ÜBERNOMMEN";
  set_slot(slot, a, true);
  // @a11_1 and @stop with the parameters of the panel
  if (const std::optional<DisplayList> sheet = sheet_wheel(a, record_input(a), current_settings())) {
    show_sheet(this, QString::fromStdString(a.surname), *sheet, {}, {});
  }
  return true;
}

// ported from list_ausg with warts, zeilklick and inf_box4
int MainWindow::stat_list(StatSession& ss) {
  //RR Zähler auf 0
  ss.counts = WanderCounts{};
  const int complete = ss.list->mark_complete(ss.conditions);
  std::vector<StatEntry> entries = ss.list->entries();
  // QSORT mz%() for 0-360, QSORT so$() WITH vg|() else, the groups unsorted
  sort_stat_entries(entries, ss.suc == kStatSucAll ? StatSort::kByValue
                                                   : (ss.groups ? StatSort::kNone : StatSort::kByName));
  const bool chained = ss.list->chained();
  // IF (zind& = 0 && odu$ = "UND  ") OR zdm& = 0
  const bool niete = (chained && complete == 0) || entries.empty();
  if (niete) {
    // fanz("KEIN " + das$ + "ERFÜLLT ALLE BEDINGUNGEN ! EINGABE-FEHLER ?")
    QMessageBox::information(this, tr(" HINWEIS "), tr("KEIN Datensatz ERFÜLLT ALLE BEDINGUNGEN ! EINGABE-FEHLER ?"));
    if (!ss.windows.isEmpty()) {
      ss.windows.last() += QString::fromUtf8(kNieteMark);
    }
    // IF odex!, the evaluation ends
    if (ss.list->exclusive()) {
      stat_counter_mode_ = 0;
      return 0;
    }
  }
  // EXKLUSIV shows the complete chains alone, his cd&()
  std::vector<StatEntry> shown;
  for (const StatEntry& e : entries) {
    if (!ss.list->exclusive() || e.complete) {
      shown.push_back(e);
    }
  }
  const int total = static_cast<int>(ss.set.records.size());
  const int all = ss.list->exclusive() ? ss.list->before_exclusive() : static_cast<int>(ss.list->entries().size());
  const int und = chained ? complete : 0;
  const int pages = std::max(1, static_cast<int>((shown.size() + kStatRowsPerPage - 1) / kStatRowsPerPage));
  const bool multi = !ss.odu.isEmpty();
  const auto percent = [total](int n) { return total > 0 ? kPercent * n / total : 0.0; };

  const auto row_of = [&](const StatEntry& e) {
    const StatRecord& r = ss.set.records[static_cast<std::size_t>(e.record)];
    StatSheetRow row;
    row.label = e.label;
    row.inverse = chained && e.complete;
    if (!multi) {
      row.label = QString::fromStdString(r.name).left(static_cast<qsizetype>(kStatNameWidth)).toStdString();
      if (ss.obj == kStatObjMirror) {
        row.tag = kMirrorTag;
      } else if (ss.obj == kStatObjArabic) {
        row.tag = kArabicTag;
      } else if (ss.obj != kStatObjMidpoint && ss.obj != kStatObjAspect) {
        // plan_ds, no glyph for the stored cusps
        row.slot = stat_cusp_slot(e.slot) ? 0 : e.slot;
      }
      row.aspect = ss.obj == kStatObjAspect;
      row.value = e.value;
      // the value of this entry, his mz%(m%) printed the last value of the
      // record on every row of it
      row.has_value = e.value > 0.0 && !stat_out_of_range(r, e.slot);
    }
    moment_lines(r, ss.small, ss.helio, row);
    if (!ss.small && !ss.helio) {
      row.has_angle = true;
      // IF m&(1) = 13 && obj& = 1, the MC beside an AC search
      row.angle = ss.angle_mc ? r.mc : r.ac;
    }
    return row;
  };

  StatSheetText text;
  text.small = ss.small;
  text.multi = multi;
  if (!ss.small) {
    if (!ss.helio) {
      text.heads = (tr("Datum     Zeit(UT)    ") + (ss.angle_mc ? "MC" : "AC")).toStdString();
    }
  } else {
    text.heads = tr("Daten ( Dezimal-Grad )").toStdString();
  }
  text.file = (tr(" Datei : ") + stat_file_label(ss.sta) + " ").toStdString();
  text.label_signs = tr("Zeichen").toStdString();
  text.label_houses = tr("Häuser").toStdString();
  text.label_total = tr("Total =").toStdString();
  text.label_partial = tr("Partial=").toStdString();
  text.label_several = tr("Mehrere Bedingungen !").toStdString();
  text.label_aspect = tr("ISTWERT / GRAD").toStdString();
  if (!multi) {
    text.object_line = ss.object_line.toStdString();
    text.window_line = ss.window_line.toStdString();
    text.aspect_box = ss.obj == kStatObjAspect;
  }
  // inf_box2, the bars of every single condition but ASPEKT and NAME,
  // the count box else
  const bool plain = !multi && ss.obj != kStatObjAspect && ss.obj != kStatObjName;
  const auto count_lines = [&](const QString& how) {
    // STR$(laf&,4) + d$ + "Wurden durchsucht", the relative frequency
    const int zl = und > 0 ? und : all;
    text.counts = {(QString::asprintf("%4d", total) + tr(" DATENSÄTZE Wurden durchsucht")).toStdString(),
                   (QString::asprintf("%4d", chained && und == 0 ? und : zl) + tr(" DATENSÄTZE ERFÜLLEN ") + how).toStdString(),
                   ("=" + QString::asprintf("%6.2f", percent(zl)) + tr("%  Relative Häufigkeit")).toStdString()};
  };
  const QString how = chained ? (und == 0 ? tr(" Alle Beding.") : tr("UND-Bedingung")) : tr(" Je eine Bed.");
  if (plain) {
    text.bars = true;
    text.by_house = ss.suc == kStatSucHouse;
    text.sums = ss.sums;
    text.framed = ss.framed;
    text.object_tag = ss.object_tag.toStdString();
    text.total = total;
    text.partial = ss.suc != kStatSucAll;
    text.partial_count = all;
  } else if (!multi) {
    text.counts_x = 20;
    count_lines(how);
  } else {
    text.several = true;
    text.counts_x = 100;
    count_lines(how);
    for (const QString& line : ss.condition_lines()) {
      text.conditions.push_back(line.toStdString());
    }
  }
  // "* Blättern: " + lt$ + " " + rck$ + "|Weitere Beding: 'W'|" + esc$ + " *"
  text.footer = tr("* Blättern: Leertaste | Zurück mit 'R'|Weitere Beding: 'W'|ENDE: Mit 'ESC' *").toStdString();
  if (shown.size() > kSkipFooterRows) {
    text.footer2 = tr("* 6 Bildschirme Vorwärts : 'S' | Zum Anfang :  'H' |  *").toStdString();
  }
  // the count lines of inf_box4, zdms& under EXKLUSIV
  text.info_counts = {(QString::asprintf("%4d", total) + tr(" DATENSÄTZE wurden durchgesucht")).toStdString(),
                      (QString::asprintf("%4d", all) + tr(" DATENSÄTZE ERFÜLLEN Eine Beding.")).toStdString(),
                      ("=" + QString::asprintf("%6.2f", percent(all)) + tr("% Relative Häufigkeit")).toStdString()};
  if (chained) {
    text.info_counts.push_back((QString::asprintf("%4d", und) + tr(" DATENSÄTZE ERFÜLLEN") +
                                (und == 0 ? tr(" Alle Bedingungen") : tr(" UND-Bedingung")))
                                   .toStdString());
    text.info_counts.push_back(("=" + QString::asprintf("%6.2f", percent(und)) + tr("% Relative Häufigkeit")).toStdString());
  }

  // his mode 4, every listed record counted in one pass, the last chart
  // shown with both windows until the counter is cleared. False when the
  // count was stopped
  const auto count_all = [&]() {
    ss.counts = WanderCounts{};
    StatProgress progress(this, tr("STATISTIK"), {whole_counter_text(ss.suc, ss.obj, ss.sta)});
    progress.show();
    progress.setFocus();
    for (std::size_t i = 0; i < shown.size(); ++i) {
      stat_count(ss, shown[i].record);
      const long pct = std::lround(kPercent * static_cast<double>(i + 1) / static_cast<double>(shown.size()));
      progress.set_text(QString(" %1").arg(pct, 5) + tr(" % Der LISTE ausgezählt !"));
      QApplication::processEvents();
      if (progress.take_cancel()) {
        return false;
      }
    }
    progress.hide();
    for (;;) {
      // kota0, the last chart again without a recount
      stat_view(ss, shown.back().record, true);
      // ALERT 3,"Wollen Sie WEITERGEHEN|und den ZÄHLER LÖSCHEN ?",1,"NEIN|JA"
      if (ChoiceDialog::ask(this, "HORCOM", {tr("Wollen Sie WEITERGEHEN"), tr("und den ZÄHLER LÖSCHEN ?")},
                            {tr("NEIN"), tr("JA")}, 0) == 1) {
        return true;
      }
    }
  };

  int page = 0;
  bool info_box = false;
  int result = -1;
  for (;;) {
    // his WHILE kl& < zdm& never ran on an empty list, no page then
    if (!shown.empty() && stat_counter_mode_ == 4) {
      static_cast<void>(count_all());
    } else if (!shown.empty()) {
      QDialog view(this);
      mark_output(&view, menu_item::kStatistics);
      // TITLEW, STATISTIK and the file
      view.setWindowTitle(tr("STATISTIK") + " | " + stat_file_label(ss.sta));
      auto* v = new QVBoxLayout(&view);
      v->setContentsMargins(0, 0, 0, 0);
      auto* canvas = new WheelWidget(&view);
      canvas->setObjectName("statList");
      v->addWidget(canvas);
      const auto draw = [&]() {
        std::vector<StatSheetRow> rows;
        const std::size_t from = static_cast<std::size_t>(page) * kStatRowsPerPage;
        for (std::size_t i = from; i < shown.size() && i < from + kStatRowsPerPage; ++i) {
          rows.push_back(row_of(shown[i]));
        }
        text.page = page + 1;
        text.info = info_box;
        canvas->set_plain_list(build_stat_page(rows, text));
      };
      int action = -1;
      const auto click = [&](QMouseEvent* me) {
        if (me->button() == Qt::RightButton) {
          //RR INF-BOX WAAGERECHT
          // IF NOT suc& = 5, none under OHNE EINSCHRÄNKUNG
          if (ss.suc != kStatSucAll) {
            info_box = !info_box;
            draw();
          }
          return;
        }
        // zeilklick, the row under the mouse
        const double sy = canvas->to_canvas(me->position()).y();
        if (sy <= kRowTop || sy >= kRowBottom) {
          return;
        }
        const auto zl = static_cast<std::size_t>((sy - kRowTop) / kRowHeight);
        const std::size_t i = static_cast<std::size_t>(page) * kStatRowsPerPage + zl;
        if (i >= shown.size()) {
          return;
        }
        const StatEntry& hit = shown[i];
        const StatRecord& r = ss.set.records[static_cast<std::size_t>(hit.record)];
        if (stat_out_of_range(r, hit.slot)) {
          // fanz(pl$(mz|(kl&)) + " AUßER BEREICH !")
          const std::string_view tag = body::kName[static_cast<std::size_t>(hit.slot)];
          QMessageBox::information(&view, tr(" HINWEIS "),
                                   QString::fromUtf8(tag.data(), static_cast<int>(tag.size())) + tr(" AUßER BEREICH !"));
          return;
        }
        if (!stat_row(ss, hit.record)) {
          action = 0;
          view.accept();
          return;
        }
        if (stat_counter_mode_ == 4) {
          action = 3;
          view.accept();
        }
      };
      // warts, the keys as his WM_KEYDOWN virtual key codes 13, 32, 33,
      // 34 and the letters
      const auto key = [&](int k) {
        switch (k) {
          case Qt::Key_Space:
          case Qt::Key_Return:
          case Qt::Key_Enter:
          case Qt::Key_V:
          case Qt::Key_PageDown:
            // vorw!, past the last page the list ends
            if (page + 1 < pages) {
              ++page;
              draw();
            } else {
              action = 1;
              view.accept();
            }
            return;
          case Qt::Key_R:
          case Qt::Key_PageUp:
            if (page > 0) {
              --page;
              draw();
            }
            return;
          case Qt::Key_W:
            // IF weit! && NOT (suc& = 5 OR n!)
            if (ss.suc != kStatSucAll && !niete) {
              action = 2;
              view.accept();
            }
            return;
          case Qt::Key_S:
            // schn!, IF kl& + 120 < zdm& the fifth page after this one
            // ends the jump and the next one shows
            if (page + kSkipPages < pages) {
              page += kSkipPages;
              draw();
            }
            return;
          case Qt::Key_H:
            page = 0;
            draw();
            return;
          case Qt::Key_Escape:
            // "  'STATISTIK'   BEENDEN ?"
            if (ChoiceDialog::ask(&view, tr("AUSWAHL"), {QString(), tr("  'STATISTIK'   BEENDEN ?")},
                                  {tr("JA"), tr("NEIN")}, 0) == 0) {
              action = 0;
              view.accept();
            }
            return;
          default:
            return;
        }
      };
      LambdaFilter keys([&](QEvent* e) {
        if (e->type() == QEvent::MouseButtonPress) {
          click(static_cast<QMouseEvent*>(e));
          return true;
        }
        if (e->type() == QEvent::KeyPress) {
          key(static_cast<QKeyEvent*>(e)->key());
          return true;
        }
        return false;
      });
      view.installEventFilter(&keys);
      canvas->installEventFilter(&keys);
      draw();
      view.resize(size());
      view.exec();
      if (action == 3) {
        continue;
      }
      if (action == 0) {
        result = 0;
        break;
      }
      if (action == 2) {
        // @od_un with weit!, then GOTO stat2
        ss.weit = true;
        const int j = stat_join(ss);
        result = j >= 1 && j <= 3 ? 2 : (j == 4 ? -1 : 0);
        if (result != -1) {
          break;
        }
        continue;
      }
    }
    // lend, a NIETE starts a new evaluation
    if (niete) {
      result = 1;
      break;
    }
    // "","WEITERE AUSWERTUNG","","Mit DIESER DATEI ? " + datr$
    const int en = ChoiceDialog::ask(this, tr("AUSWAHL"),
                                     {QString(), tr("WEITERE AUSWERTUNG"), QString(),
                                      tr("Mit DIESER DATEI ? ") + QString::fromStdWString(ss.sta.wstring())},
                                     {tr("JA"), tr(" Beenden"), tr("Irrtum ( = UNDO = Zurück )")}, 0);
    if (en == 0) {
      result = 1;
      break;
    }
    if (en == 2) {
      if (stat_counter_mode_ == 4) {
        // ALERT 3,"Liste NEU AUSZÄHLEN ?",1,"NEIN = ENDE|JA"
        if (ChoiceDialog::ask(this, "HORCOM", {tr("Liste NEU AUSZÄHLEN ?")}, {tr("NEIN = ENDE"), tr("JA")}, 0) != 1) {
          result = 0;
          break;
        }
      }
      page = 0;
      continue;
    }
    result = 0;
    break;
  }
  // list_a, CLR zaehl_asp_halbs&
  stat_counter_mode_ = 0;
  return result;
}

}  // namespace horcom
