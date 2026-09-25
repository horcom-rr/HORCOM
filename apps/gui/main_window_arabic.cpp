// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// ARABISCHE TEILE, his arabt with the own points of arab_eig, the
// define flow with glanz and the table of arabtex and stelk.

#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "choice_dialog.hpp"
#include "event_filter.hpp"
#include "horcom/chart/arabic.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/stars.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/wheel.hpp"
#include "main_window.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"
#include "theme.hpp"
#include "zodiac_cells.hpp"

namespace horcom {

namespace {

// the pseudo rows of his object list
constexpr int kRowHouse = 100;
constexpr int kRowRuler = 101;
constexpr int kRowDegree = 102;
// his IF k& <> 12 && z& < z1& + 88, four hits at 28 pixels
constexpr std::size_t kMaxHits = 4;
// the rows his arabt blanked without a Chiron ephemeris
constexpr int kChironRows[2] = {14, 31};

// the partners of stelk, the Fixpunkt, SO to MC without DS, AG and CH
bool stelk_partner(int slot) {
  return (slot >= body::kFixpunkt && slot <= body::kMc && slot != body::kNodeDesc) || slot == body::kApogee ||
         slot == body::kChiron;
}

}  // namespace

// ported from the NEU DEFINIEREN branch of arabt, arabt0 with glanz
int MainWindow::arabic_define(const std::filesystem::path& dir) {
  // ausw_pl_hs under arb!, the houses, cardinals and extras past CH dashed
  static constexpr const char* kCaption[15] = {
      QT_TRANSLATE_NOOP("horcom::MainWindow", "   FIXPUNKT"),  QT_TRANSLATE_NOOP("horcom::MainWindow", " SONNE"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " MOND"),         QT_TRANSLATE_NOOP("horcom::MainWindow", " MERKUR"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " VENUS"),        QT_TRANSLATE_NOOP("horcom::MainWindow", " MARS"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " JUPITER"),      QT_TRANSLATE_NOOP("horcom::MainWindow", " SATURN"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " URANUS"),       QT_TRANSLATE_NOOP("horcom::MainWindow", " NEPTUN"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", " PLUTO"),        QT_TRANSLATE_NOOP("horcom::MainWindow", "  MONDKNOTEN N"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "  MONDKNOTEN S"), QT_TRANSLATE_NOOP("horcom::MainWindow", "  ASZENDENT"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "     MC")};
  std::vector<std::pair<int, QString>> rows;
  if (fixpunkt_ >= 0.0) {
    rows.emplace_back(body::kFixpunkt, tr(kCaption[0]));
  }
  for (int slot = body::kSun; slot <= body::kMc; ++slot) {
    rows.emplace_back(slot, tr(kCaption[slot]));
  }
  for (int i = 0; i < 4; ++i) {
    rows.emplace_back(-1, QString());
  }
  rows.emplace_back(body::kApogee, tr("  SCHWARZER MOND  "));
  rows.emplace_back(body::kChiron, tr(" CHIRON         a=13.61  e=0.38  i=6.94°  T=50 Jahre "));
  rows.emplace_back(kRowHouse, tr("   HAUS NR.     "));
  rows.emplace_back(kRowRuler, tr(" HERR v. HAUS NR."));
  rows.emplace_back(kRowDegree, tr(" EKLIPTIK-GRAD"));
  const auto tag = [](int slot) {
    const std::string_view n = body::kName[static_cast<std::size_t>(slot)];
    return QString::fromUtf8(n.data(), static_cast<qsizetype>(n.size()));
  };
  for (;;) {
    const int jok = static_cast<int>(read_own_arabic(dir).size());
    // IF jok& < 36
    if (jok >= kMaxOwnArabic) {
      return 1;
    }
    // n$ = LEFT$(@inputbox$(150,eas$," NAME DEFINIEREN ! ",""),21)
    std::optional<QString> name;
    for (;;) {
      name = ask_text(this, tr("STRING-Eingabe !"), tr(" NAME DEFINIEREN ! "), {}, 21);
      if (!name || !name->trimmed().isEmpty()) {
        break;
      }
    }
    if (!name) {
      return -1;
    }
    const std::optional<QString> remark = ask_text(this, tr("STRING-Eingabe !"), tr(" BEMERKUNG EINTRAGEN ? "), {}, 14);
    if (!remark) {
      return -1;
    }
    const QStringList instruction{tr(" Bitte 3 Glieder wählen, "), tr(" gemäß der Formel.       "),
                                  tr(" Z.B für  AC+SO-MO :     "), tr(" Nacheinander auf        "),
                                  tr(" AC .. OK,SO .. OK       "), tr(" und MO .. OK KLICKEN.   ")};
    QStringList echo;
    std::array<OwnArabicTerm, 3> terms{};
    // his terms went to the file one by one, an ESC then left a torn
    // point behind, the port writes the point once it is whole
    bool whole = true;
    for (int zg = 1; zg <= 3 && whole; ++zg) {
      QStringList notes = instruction;
      notes << QString() << echo;
      const std::optional<int> pick = ask_object(this, rows, notes);
      if (!pick) {
        whole = false;
        break;
      }
      OwnArabicTerm t;
      QString line;
      if (*pick == kRowHouse || *pick == kRowRuler) {
        // numw1("SPITZE HAUS NR.?",1,12) bzw. "HERR von HAUS NR.?"
        const std::optional<int> h =
            ask_digit(this, *pick == kRowHouse ? tr("SPITZE HAUS NR.?") : tr("HERR von HAUS NR.?"), 1, 12);
        if (!h) {
          whole = false;
          break;
        }
        t = {*pick == kRowHouse ? 12 : 13, *h};
        line = (*pick == kRowHouse ? tr(" Spitze Haus  ") : tr(" Herr v.Haus  ")) + QString::asprintf("%2d", *h);
      } else if (*pick == kRowDegree) {
        // input_grmise_zod(ekl$ + eg$), a1& = CINT(wh * up)
        const std::optional<double> wh = ask_zodiac_position(this, tr("Ekliptikale Länge Eingeben"));
        if (!wh) {
          whole = false;
          break;
        }
        const int deg = static_cast<int>(std::lround(norm_rad(*wh) * kRadToDeg));
        t = {14, deg};
        line = tr(" Eklipt.-Grad ") + QString::asprintf("%2d", deg);
      } else {
        t = {0, *pick};
        line = QString(11, ' ') + tag(*pick);
      }
      terms[static_cast<std::size_t>(zg) - 1] = t;
      // a$ + " = " + STR$(zg&) + ".Glied "
      echo << line + " = " + QString::number(zg) + tr(".Glied ");
    }
    if (!whole) {
      return -1;
    }
    append_own_arabic_name(dir, jok, name->toStdString(), remark->toStdString());
    for (int zg = 1; zg <= 3; ++zg) {
      append_own_arabic_term(dir, jok, zg, terms[static_cast<std::size_t>(zg) - 1]);
    }
    // IF jok! && jok& < 36 && jok& > 0, wr$ + "EN PUNKT " + df$ + "EN ?"
    if (jok + 1 >= kMaxOwnArabic) {
      return 1;
    }
    const int o = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("WeiterEN PUNKT definierEN ?")},
                                    {tr("JA"), tr(" NEIN ")}, 0);
    if (o != 0) {
      // GOTO arbst
      return 0;
    }
  }
}

// ported from arabt with arabtex, stelk, stelt and arabso
void MainWindow::arabic_table() {
  if (!last_chart_) {
    return;
  }
  // his haeuser_pruef let the table run on without cusps, every part
  // reads the AC, the port says so
  if (!last_chart_->houses.ok) {
    QMessageBox::information(this, "HORCOM", tr("Kein Häusersystem gewählt !"));
    return;
  }
  // hrg! = FALSE, np& = 20, nk&(1) = 19, nk&(2) = 20
  ChartSettings s = current_settings();
  s.heliocentric = false;
  s.extra_bodies = true;
  s.nk.fill(0);
  s.nk[1] = body::kApogee;
  s.nk[2] = body::kChiron;
  Chart chart = compute_chart(current_input(), s, vsop_, eph_);
  if (!chart.ok || !chart.houses.ok) {
    return;
  }
  if (fixpunkt_ >= 0.0) {
    chart.b[body::kFixpunkt].present = true;
    chart.b[body::kFixpunkt].valid = true;
    chart.b[body::kFixpunkt].el = fixpunkt_;
  }
  const std::filesystem::path dir = data_dir_;
  const QString e = tr("Eigen-Punkte ");
  ArabicFormula af = ArabicFormula::kTraditional;
  QString ts;
  QString trd;
  for (;;) {
    // tr$ + "E FORMEL ?", "oder", it$, in$
    const int b = ChoiceDialog::ask(this, tr("AUSWAHL"),
                                    {tr("TRADITIONELLE FORMEL ?"), tr("oder"), tr("IMMER als 'TAG - GEBURT'"),
                                     tr("IMMER als 'NACHT-GEBURT'")},
                                    {tr("TRADITIONELL"), tr("TAG"), tr("NACHT")}, 0);
    if (b < 0) {
      return;
    }
    static constexpr ArabicFormula kFormula[3] = {ArabicFormula::kTraditional, ArabicFormula::kAlwaysDay,
                                                  ArabicFormula::kAlwaysNight};
    af = kFormula[b];
    // ts$ = "Tradit." / "Tag-Geb." / "Nacht-Gb."
    ts = b == 0 ? tr("Tradit.") : (b == 1 ? tr("Tag-Geb.") : tr("Nacht-Gb."));
    trd = b == 0 ? tr("TRADITIONELL") : (b == 1 ? tr("IMMER als 'TAG - GEBURT'") : tr("IMMER als 'NACHT-GEBURT'"));
    const int jok = static_cast<int>(read_own_arabic(dir).size());
    QStringList info;
    if (jok > 0) {
      // " " + STR$(jok&) + " " + e$ + "bereits " + df$ + "t !"
      info << " " + QString::number(jok) + " " + e + tr("bereits definiert !") << QString();
    }
    info << tr("Wollen Sie ") << e + tr("  DEFINIEREN ?") << e + tr("  LÖSCHEN    ?");
    const int q = ChoiceDialog::ask(this, tr("AUSWAHL"), info,
                                    {tr("NEU DEFINIEREN"), tr("TABELLEN-AUSGABE"), e + tr(" LÖSCHEN")}, 1);
    if (q < 0) {
      return;
    }
    if (q == 0) {
      const int r = arabic_define(dir);
      if (r < 0) {
        return;
      }
      if (r == 0) {
        continue;
      }
    } else if (q == 2 && jok > 0) {
      // e$ + "LÖSCHEN ?", " NEIN " / "JA"
      const int p = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), e + tr("LÖSCHEN ?")}, {tr(" NEIN "), tr("JA")}, 0);
      if (p == 1) {
        delete_own_arabic(dir);
        continue;
      }
    }
    break;
  }

  const std::vector<ArabicPart> parts = arabic_parts(chart, af, dir, alt_rulers_);
  QDialog dialog(this);
  mark_output(&dialog, menu_item::kArabicParts);
  dialog.setWindowTitle(tr("ARABISCHE TEILE"));
  auto* v = new QVBoxLayout(&dialog);
  // arabtex, "Name des Punktes" "Formel" ts$ "Ekl.Länge" "Aspekte" "Bemerkungen"
  auto* table = new QTableWidget(0, 5, &dialog);
  table->setHorizontalHeaderLabels({tr("Name des Punktes"), tr("Formel") + "   " + ts, tr("Ekl.Länge"), tr("Aspekte"),
                                    tr("Bemerkungen")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  table->setFocusPolicy(Qt::NoFocus);
  table->setFont(theme::mono_font());
  v->addWidget(table, 1);
  // stelt, LEFT$(na$,25) + "|" + dm$ + ":" + datum3$ + "|" + sol$ + "|Eph.:" + gena4$
  const ClassicSheetText sheet = classic_sheet_text();
  const QString label = rhythm_chart_label();
  v->addWidget(new QLabel(QString::fromStdString(sheet.name).left(25) + "|" + tr("Datum") + ":" +
                              datum3_text(calendar_date(chart.jd_ut, s.calendar)) + "|" + label + "|" + tr("Eph.:") +
                              gena4_text(),
                          &dialog));
  // "Formel : " + tr$
  v->addWidget(new QLabel(tr("Formel : ") + trd, &dialog));
  // fanz("SORTIEREN:" + lt$), the name order is the tester's
  auto* hint = new QLabel(tr("SORTIEREN:") + tr("Leertaste") + tr("  |  N = nach Namen"), &dialog);
  v->addWidget(hint);

  const double orb = aspect_settings_.orb;
  std::vector<std::size_t> order(parts.size());
  std::iota(order.begin(), order.end(), std::size_t{0});
  const bool chiron_missing = !chart.b[body::kChiron].valid;
  const auto fill = [&]() {
    table->setRowCount(0);
    for (const std::size_t m : order) {
      const ArabicPart& p = parts[m];
      const int row = table->rowCount();
      table->insertRow(row);
      // IF (m& = 14 OR m& = 31) && jdplanetex!(20), the row stays blank
      const bool blank = chiron_missing && (static_cast<int>(m) == kChironRows[0] || static_cast<int>(m) == kChironRows[1]);
      if (blank) {
        continue;
      }
      auto* name = new QTableWidgetItem(QString::fromUtf8(p.name.c_str()));
      auto* formula = new QTableWidgetItem(QString::fromUtf8(p.formula.c_str()));
      table->setItem(row, 0, name);
      table->setItem(row, 1, formula);
      table->setItem(row, 4, new QTableWidgetItem(QString::fromUtf8(p.remark.c_str())));
      if (!p.valid) {
        // an own point his old dispatch stored with a phantom term
        formula->setText(tr("UNGÜLTIGE DEFINITION"));
        continue;
      }
      table->setItem(row, 2, new QTableWidgetItem(zodiac_text(p.la, ZodiacForm::kGz1)));
      // stelk, the tag and the aspect sprite, at most four
      QString asp;
      std::size_t hits = 0;
      for (const auto& [slot, kind] : point_aspects(chart, p.la, orb)) {
        if (!stelk_partner(slot) || hits >= kMaxHits) {
          continue;
        }
        ++hits;
        const std::string_view n = body::kName[static_cast<std::size_t>(slot)];
        // stelk sets the aspect sprite at z& and the tag at z& + 8
        asp += QString::fromUtf8(aspect_glyph(star_aspect_family(kind))) + QString::fromUtf8(n.data(), static_cast<qsizetype>(n.size())) + " ";
      }
      table->setItem(row, 3, new QTableWidgetItem(asp.trimmed()));
      // IF prenbl& = 0, deftextcol(3), red on cyan on the screen only
      if (hits > 0 && konsta_.prenbl == 0) {
        for (QTableWidgetItem* item : {name, formula}) {
          item->setBackground(QColor(0x00, 0xFF, 0xFF));
          item->setForeground(QColor(0xFF, 0x00, 0x00));
        }
      }
    }
    table->resizeColumnsToContents();
  };
  LambdaFilter keys(LambdaFilter::keys([&](int key) {
    if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter) {
      // arabso, @sort(0,36) over FIX(10000 * at(m&))
      std::stable_sort(order.begin(), order.end(), [&parts](std::size_t a, std::size_t b) {
        return std::trunc(parts[a].la * 10000.0) < std::trunc(parts[b].la * 10000.0);
      });
      fill();
      return true;
    }
    if (key == Qt::Key_N) {
      std::stable_sort(order.begin(), order.end(), [&parts](std::size_t a, std::size_t b) {
        return QString::fromUtf8(parts[a].name.c_str()).localeAwareCompare(QString::fromUtf8(parts[b].name.c_str())) < 0;
      });
      fill();
      return true;
    }
    return false;
  }));
  dialog.installEventFilter(&keys);
  table->installEventFilter(&keys);
  fill();
  dialog.resize(900, 700);
  dialog.exec();
}

}  // namespace horcom
