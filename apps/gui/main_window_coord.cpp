// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// PLANETEN-KOORDINATEN and ZUSATZ-PLANETEN-KOORDINATEN, his a91 and a10
// with ko_ta, and the ZEIT VARIIEREN walk of a9zw over either table.

#include <QApplication>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath>
#include <limits>
#include <optional>

#include "choice_dialog.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/corrections.hpp"
#include "horcom/chart/distances.hpp"
#include "horcom/chart/planet_points.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "main_window.hpp"
#include "painter.hpp"
#include "robert_input.hpp"
#include "table_fit.hpp"
#include "theme.hpp"
#include "zodiac_cells.hpp"

namespace horcom {

namespace {

// the columns of ko_ta, the Knot and Apsiden columns give way to the
// Name column in ZUSATZ
enum Column { kColPl, kColLon, kColLat, kColVel, kColA, kColDist, kColRa, kColDec, kColNode, kColSouth, kColApsides };

// the step wait of zeitwi, two seconds, the mouse moves it
constexpr int kWaitStartMs = 2000;
constexpr int kWaitMoreMs = 2000;
constexpr int kWaitLessMs = 1000;
constexpr int kWaitLeastMs = 1000;
// his IF ABS(jd - jd0) > 10 skipped the question and restored the moment
constexpr double kKeepDays = 10.0;

// his ko_tab00$, STR$(z,v + n + 1,n), empty at zero
QString field(double z, int v, int n) {
  if (std::abs(z) < kEps) {
    return {};
  }
  return QString::asprintf("%*.*f", v + n + 1, n, z);
}

// the verbatim Name column of a10
QString extra_label(int slot) {
  switch (slot) {
    case body::kChiron: return QStringLiteral("Chiron            CH");
    case body::kTranspluto: return QStringLiteral("Transpluto = Isis TP");
    case body::kFortune: return QString::fromUtf8("Glückspunkt       GP");
    case body::kCeres: return QStringLiteral("Ceres             CE");
    case body::kPallas: return QStringLiteral("Pallas            PA");
    case body::kJuno: return QStringLiteral("Juno              JN");
    case body::kVesta: return QStringLiteral("Vesta             VS");
    case body::kCupido: return QStringLiteral("Cupido            CU");
    case body::kHades: return QStringLiteral("Hades             HA");
    case body::kZeus: return QStringLiteral("Zeus              ZE");
    case body::kKronos: return QStringLiteral("Kronos            KR");
    case body::kApollon: return QStringLiteral("Apollon           AP");
    case body::kAdmetos: return QStringLiteral("Admetos           AD");
    case body::kVulkanus: return QStringLiteral("Vulkanus          VU");
    case body::kPoseidon: return QStringLiteral("Poseidon          PO");
    case body::kQuaoar: return QStringLiteral("Quaoar            QU");
    case body::kHalley: return QStringLiteral("Komet Halley      HL");
    case body::kPholus: return QStringLiteral("Pholus            PH");
    case body::kDamokles: return QStringLiteral("Damokles          DA");
    case body::kNessus: return QStringLiteral("Nessus            NS");
    case body::kXena: return QStringLiteral("Xena              XE");
    default: return {};
  }
}

// the real small bodies with their own ephemerides, n2& and n5& to n22&
bool real_extra(int slot) {
  return slot == body::kChiron || (slot >= body::kCeres && slot <= body::kVesta) ||
         (slot >= body::kQuaoar && slot <= body::kXena);
}

// the hypothetical points of the Hamburg school, CU to PO
bool hypothetical(int slot) {
  return slot >= body::kCupido && slot <= body::kPoseidon;
}

// the stepping screen of a9zw, the table with his ran line under it
class CoordView final : public QDialog {
 public:
  explicit CoordView(QWidget* parent) : QDialog(parent) {}
  std::function<bool(int)> on_key;
  std::function<void(Qt::MouseButton)> on_mouse;

 protected:
  void keyPressEvent(QKeyEvent* e) override {
    if (on_key && on_key(e->key())) {
      return;
    }
    QDialog::keyPressEvent(e);
  }
  void mousePressEvent(QMouseEvent* e) override {
    if (on_mouse) {
      on_mouse(e->button());
    }
  }
};

}  // namespace

// ported from horgt
QString MainWindow::horgt_text() const {
  const ChartSettings s = current_settings();
  return s.heliocentric ? tr("Heliozentrisch") : (s.topocentric_parallax ? tr("Topozentrisch") : tr("Geozentrisch"));
}

// ported from genau, gena2$
QString MainWindow::gena2_text() const {
  if (current_settings().heliocentric) {
    return tr(" Ephemeride: Mittl.Äquin.");
  }
  return tr(" Ephemeride:") + gena4_text();
}

// ported from genau, gena4$ = w$ + "," + p$ + a$
QString MainWindow::gena4_text(int appa) const {
  const int mode = appa > 0 ? appa : konsta_.appa;
  const QString w = mode == 2 ? QStringLiteral("App.2") : (mode == 3 ? tr("Wahr") : QStringLiteral("App.1"));
  return w + "," + (current_settings().topocentric_parallax ? tr("Mit Parallaxe") : tr("Ohne Parallaxe"));
}

// ported from ko_ta with ko_tab0, the rows of one table
void MainWindow::fill_coordinate_table(QTableWidget* table, const Chart& chart, const ChartSettings& s, bool extras) const {
  table->setRowCount(0);
  const bool helio = s.heliocentric;
  const LunarRates rates = lunar_rates(chart, s.calendar);
  // his IF plinv& = 1 OR plinv& = 3, the heavy planets inverted
  const bool invert_heavy = konsta_.plinv == 1 || konsta_.plinv == 3;
  const auto tag = [helio](int slot) {
    // his pl$(2) = "TE" in the hrg mode
    const std::string_view n = (helio && slot == body::kMoon) ? body::kEarthName : body::kName[static_cast<std::size_t>(slot)];
    return QString::fromUtf8(n.data(), static_cast<qsizetype>(n.size()));
  };
  struct Row {
    int slot = 0;
    // the node or apogee variant of the Mittel and Wahr rows
    bool mean = false;
    bool truth = false;
  };
  std::vector<Row> rows;
  if (!extras) {
    for (int slot = body::kSun; slot <= body::kPluto; ++slot) {
      // his IF NOT (hrg! && i& = 1)
      if (!(helio && slot == body::kSun)) {
        rows.push_back({slot});
      }
    }
    if (!helio) {
      rows.push_back({body::kNodeAsc, true, false});
      rows.push_back({body::kNodeAsc, false, true});
    }
  } else {
    if (!helio) {
      rows.push_back({body::kApogee, true, false});
      rows.push_back({body::kApogee, false, true});
    }
    for (int slot = body::kChiron; slot <= body::kXena; ++slot) {
      // his IF NOT (hrg! && i& = n4&), no Glückspunkt in the hrg mode
      if (!(helio && slot == body::kFortune)) {
        rows.push_back({slot});
      }
    }
  }
  const auto set = [table](int row, int col, QTableWidgetItem* item) { table->setItem(row, col, item); };
  const auto text = [&set](int row, int col, const QString& t) {
    auto* item = new QTableWidgetItem(t);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    set(row, col, item);
  };
  const int last_col = table->columnCount() - 1;
  for (const Row& r : rows) {
    const BodyState& b = chart.b[static_cast<std::size_t>(r.slot)];
    const int row = table->rowCount();
    table->insertRow(row);
    auto* head = new QTableWidgetItem(tag(r.slot));
    const QImage img = glyph_sprite(QString::fromUtf8(body_glyph(r.slot)), to_rgb(theme::ink_now()));
    if (!img.isNull()) {
      head->setIcon(QIcon(QPixmap::fromImage(img)));
    }
    set(row, kColPl, head);
    auto* tail = new QTableWidgetItem;
    if (!img.isNull()) {
      tail->setIcon(QIcon(QPixmap::fromImage(img)));
    }
    set(row, last_col, tail);
    // his IF NOT jdplanetex!(i&) = TRUE, a body outside its span stays blank
    const bool node_row = r.slot == body::kNodeAsc;
    const bool apogee_row = r.slot == body::kApogee && (r.mean || r.truth);
    if (!node_row && !apogee_row && (!b.present || !b.valid)) {
      continue;
    }
    double el = b.el;
    double eb = b.eb;
    double tb = b.tb;
    double ttb = b.ttb;
    if (node_row) {
      el = r.truth ? chart.lunar.true_node : chart.lunar.mean_node;
      eb = 0.0;
      // the mean node runs his fixed -0.00092422029 per day
      tb = r.truth ? rates.node_tb : chart.lunar.mean_node_speed;
      ttb = r.truth ? rates.node_ttb : 0.0;
    } else if (apogee_row) {
      el = r.truth ? chart.lunar.true_apogee : chart.lunar.mean_apogee;
      eb = r.truth ? chart.lunar.true_apogee_lat : chart.lunar.mean_apogee_lat;
      tb = r.truth ? rates.apogee_tb : chart.lunar.mean_apogee_speed;
      ttb = r.truth ? rates.apogee_ttb : 0.0;
    }
    // the minutes format for the true points, the Hamburg points,
    // Transpluto and the Glückspunkt, seconds elsewhere
    const bool minutes = r.truth || hypothetical(r.slot) || r.slot == body::kTranspluto || r.slot == body::kFortune;
    if (el > kEps) {
      QTableWidgetItem* lon = zodiac_item(el, !minutes, true);
      // the true node and the true apogee stamped inverted, and with
      // plinv the heavy planets
      const bool heavy = r.slot == body::kMars || (r.slot >= body::kSaturn && r.slot <= body::kPluto);
      if (!helio && (r.truth || (invert_heavy && heavy))) {
        auto* h = table->item(row, kColPl);
        h->setBackground(QColor(0, 0, 0));
        h->setForeground(QColor(0xFF, 0xFF, 0xFF));
        // the sprite turns white on the dark ground like his NOTSRCCOPY
        const QImage white = glyph_sprite(QString::fromUtf8(body_glyph(r.slot)), 0xFFFFFF);
        if (!white.isNull()) {
          h->setIcon(QIcon(QPixmap::fromImage(white)));
        }
      }
      set(row, kColLon, lon);
    }
    // his IF i& < 27 OR i& > 34, the Breite column
    if (!hypothetical(r.slot)) {
      text(row, kColLat, field(eb * kRadToDeg, 3, 2));
    }
    // his z = up * 60 * tb, the formats by size
    const double z = tb * kRadToDeg * kArcminPerDeg;
    if (std::abs(z) > 100.0) {
      text(row, kColVel, field(z, 5, 1));
    } else if (std::abs(z) > 10.0) {
      text(row, kColVel, field(z, 4, 1));
    } else {
      text(row, kColVel, field(z, 4, 2));
    }
    // the A column, blank in the hrg mode, for the Fixpunkt and the
    // Glückspunkt
    if (!helio && r.slot != body::kFortune) {
      text(row, kColA, ttb > 0.0 ? QStringLiteral("+") : QStringLiteral("-"));
    }
    // Entf., percent of his mean distances or astronomical units
    QString dist;
    bool extreme = false;
    const auto slot_index = static_cast<std::size_t>(r.slot);
    if (helio) {
      if (r.slot >= body::kMoon && r.slot <= body::kPluto) {
        dist = konsta_.entf == 1 ? QString::asprintf("%4ld%%", std::lround(kPercent * b.r / kMeanHelioAu[slot_index]))
                                 : field(b.r, 2, 1);
      } else if (r.slot != body::kNodeDesc) {
        dist = field(b.r, 2, 1);
      }
    } else if (r.slot == body::kSun || (r.slot >= body::kMercury && r.slot <= body::kPluto)) {
      dist = konsta_.entf == 1 ? QString::asprintf("%4ld%%", std::lround(kPercent * b.dr / kMeanGeoAu[slot_index]))
                               : field(b.dr, 2, 1);
      extreme = near_distance_extreme(r.slot, b.dr);
    } else if (r.slot != body::kMoon && !node_row) {
      dist = field(b.dr, 2, 1);
    }
    text(row, kColDist, dist);
    if (extreme) {
      // his deftextcol(3), red on cyan
      table->item(row, kColDist)->setForeground(QColor(0xFF, 0x00, 0x00));
      table->item(row, kColDist)->setBackground(QColor(0x00, 0xFF, 0xFF));
    }
    // Rekt. and Dekl. for SO to PL, the node, CH, CE to VS, QU to XE
    if (!helio && ((r.slot >= body::kSun && r.slot <= body::kNodeAsc) || real_extra(r.slot))) {
      double ar = b.ar;
      double de = b.de;
      if (node_row) {
        BodyPosition p{el, 0.0, 0.0, 0.0, 0.0, 0.0};
        to_equatorial(p, chart.smo.ekls);
        ar = p.ar;
        de = p.de;
      }
      text(row, kColRa, field(norm_rad(ar) * kRadToDeg, 3, 3));
      text(row, kColDec, field(de * kRadToDeg, 3, 2));
    }
    // the Name column of a10, the Mittel and Wahr rows of the apogee
    if (extras) {
      QString label = extra_label(r.slot);
      if (apogee_row) {
        label = r.truth ? tr("Schw. Mond,WAHR   AG") : tr("Schw. Mond,MITTEL AG");
      }
      auto* name = new QTableWidgetItem(label);
      set(row, kColNode, name);
      continue;
    }
    if (node_row) {
      // his textrc of "Mittel" and "Wahr" beside the node rows
      auto* mark = new QTableWidgetItem(r.truth ? tr("Wahr") : tr("Mittel"));
      mark->setBackground(QColor(0xFF, 0xFF, 0x00));
      mark->setForeground(QColor(0, 0, 0));
      set(row, kColNode, mark);
      continue;
    }
    // the mean planetary nodes and apsides of plko10 and plko12
    const PlanetPoints pts = planet_points(chart, r.slot, s);
    if (!pts.ok) {
      continue;
    }
    if (pts.node > kEps) {
      set(row, kColNode, zodiac_item(pts.node, false, true));
      // his IF pa > kk && NOT ABS(pa - PI) < kk
      set(row, kColSouth, zodiac_item(pts.node_south, false, true));
    }
    auto* aps = new QTableWidgetItem(zodiac_text(pts.perihelion, ZodiacForm::kGz8) + QChar(0x0A) +
                                     zodiac_text(pts.aphelion, ZodiacForm::kGz8));
    set(row, kColApsides, aps);
  }
  table->resizeColumnsToContents();
  table->resizeRowsToContents();
}

// ported from a91 and a10 with ko_ta, hsa0 and a9zw
void MainWindow::coordinate_table(bool extras) {
  if (!last_chart_) {
    return;
  }
  const QString label = record_label_.trimmed();
  // a10nk, every extra body whatever the panel chose, apogw! = 0 for the
  // base run, the true apogee comes from the lunar points
  const auto table_settings = [this, extras]() {
    ChartSettings s = current_settings();
    if (extras) {
      s.extra_bodies = true;
      for (int i = 1; i < static_cast<int>(s.nk.size()); ++i) {
        s.nk[static_cast<std::size_t>(i)] = 18 + i;
      }
      s.true_apogee = false;
    }
    return s;
  };
  const auto table_chart = [this, extras, table_settings]() {
    if (!extras) {
      return *last_chart_;
    }
    return compute_chart(current_input(), table_settings(), vsop_, eph_);
  };
  // ported from haust with bes111, the cusps of AC, H2, H3, MC, H11 and
  // H12 in his gz2$ form
  const auto cusp_box_lines = [](const Chart& chart, const ChartSettings& s) {
    QStringList out;
    // his IF hrg! = 0 && haw& < 9
    if (s.heliocentric || !chart.houses.ok || s.houses >= HouseSystem::kNone) {
      return out;
    }
    const bool vehlow = s.houses == HouseSystem::kEqualVehlow;
    const bool equal_asc = s.houses == HouseSystem::kEqualAsc;
    // his aeqh, H1 and H10 where the cusp is no axis
    const QString first = vehlow ? QStringLiteral("H1 ") : QStringLiteral("AC");
    const QString tenth = (vehlow || equal_asc) ? QStringLiteral("H10") : QStringLiteral("MC");
    const std::string_view system = chart.houses.name;
    out << tr("Häuserspitzen")
        << "(" + QString::fromUtf8(system.data(), static_cast<qsizetype>(system.size())).trimmed() + ")";
    const auto gz2 = [](double rad) { return zodiac_text(rad, ZodiacForm::kGz2); };
    for (const int k : {1, 2, 3, 10, 11, 12}) {
      const double w = chart.houses.cusp[static_cast<std::size_t>(k)];
      // his IF f(k&) > 0, the axis only system keeps AC and MC
      if (w <= 0.0) {
        continue;
      }
      if (k == 1) {
        out << " " + first + ":" + gz2(w);
      } else if (k == 10) {
        out << " " + tenth + ":" + gz2(w);
      } else {
        out << QString::asprintf("H%2d :", k) + gz2(w);
      }
      // the equal systems add the axes behind their cusps
      if (k == 1 && vehlow) {
        out << " AC :" + gz2(chart.b[body::kAscendant].el);
      }
      if (k == 10 && (vehlow || equal_asc)) {
        out << " MC :" + gz2(chart.b[body::kMc].el);
      }
    }
    return out;
  };

  CoordView view(this);
  mark_output(&view, extras ? menu_item::kExtraCoordinates : menu_item::kPlanetCoordinates);
  // his mend1$, the menu caption as the window title
  view.setWindowTitle(extras ? tr("ZUSATZ-PLANETEN-KOORDINATEN") : tr("PLANETEN-KOORDINATEN"));
  auto* v = new QVBoxLayout(&view);
  auto* head = new QLabel(&view);
  head->setTextFormat(Qt::RichText);
  v->addWidget(head);
  auto* table = new QTableWidget(0, extras ? 10 : 12, &view);
  QStringList columns{tr("Pl"), tr("Ekl. Länge"), tr("Breite"), tr("Vel.'"), tr("A"), tr("Entf."), tr("Rekt.°"),
                      tr("Dekl.°")};
  if (extras) {
    columns << tr(" Name          Abkrz.") << tr("Pl");
  } else {
    columns << tr("Knot.ND") << tr("Knot.SD") << tr("Apsiden") << tr("Pl");
  }
  table->setHorizontalHeaderLabels(columns);
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  table->setItemDelegate(new ZodiacDelegate(table));
  table->setFocusPolicy(Qt::NoFocus);
  v->addWidget(table, 1);
  auto* foot_row = new QHBoxLayout();
  v->addLayout(foot_row);
  auto* foot = new QLabel(&view);
  foot->setTextFormat(Qt::RichText);
  foot->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
  foot_row->addWidget(foot);
  foot_row->addStretch(1);
  // his haust, the cusp box right of the notes, nearer the middle in
  // ZUSATZ where the Name column fills the right
  auto* cusp_box = new QLabel(&view);
  cusp_box->setObjectName("cuspBox");
  cusp_box->setFont(theme::mono_font());
  cusp_box->setFrameShape(QFrame::Box);
  cusp_box->setMargin(4);
  foot_row->addWidget(cusp_box);
  if (extras) {
    foot_row->addStretch(1);
  }
  auto* ran = new QLabel(&view);
  ran->setAlignment(Qt::AlignHCenter);
  v->addWidget(ran);
  // his coordinate screen filled the display, the text grows with the
  // window and the notes around the table grow along
  auto* zoom = new TableZoom(table, false);
  for (QLabel* l : {head, foot, cusp_box, ran}) {
    zoom->follow(l);
  }

  const auto refill = [&]() {
    const ChartSettings s = table_settings();
    const Chart chart = table_chart();
    fill_coordinate_table(table, chart, s, extras);
    // his hsa0, sol$ in the yellow box before the name
    QStringList lines = hsa0_lines(chart);
    QString first = lines.value(0);
    const QString sol = rhythm_chart_label();
    if (first.startsWith(sol)) {
      first = first.mid(sol.size());
    }
    QString html = theme::heading_span(sol.toHtmlEscaped()) + first.toHtmlEscaped();
    for (int i = 1; i < lines.size(); ++i) {
      html += "<br>" + lines[i].toHtmlEscaped();
    }
    // his horgt$ + "e " + pe$ + "-Koordinaten " + gena2$, ZUSATZ without it
    QString title = horgt_text() + tr("e Planeten-Koordinaten ");
    if (!extras) {
      title += gena2_text();
    }
    html += "<br><br>" + theme::heading_span(title.toHtmlEscaped());
    head->setText(html);
    QStringList notes;
    // his fixpunkt_anz, the red Fixpunkt = gz2$
    if (fixpunkt_ >= 0.0 && !s.heliocentric) {
      notes << "<span style='color:#e00000'>" +
                   tr("Fixpunkt = %1").arg(zodiac_text(fixpunkt_, ZodiacForm::kGz2).toHtmlEscaped()) + "</span>";
    }
    if (!extras && !s.heliocentric) {
      // his w$ under btab& = 11
      notes << theme::heading_span(s.true_apogee ? tr("MOND-Apsiden : Wahrer Wert") : tr("MOND-Apsiden : Mittelwert"));
    }
    foot->setText(notes.join("<br>"));
    foot->setVisible(!notes.isEmpty());
    const QStringList cusps = cusp_box_lines(chart, s);
    cusp_box->setText(cusps.join(QChar(0x0A)));
    cusp_box->setVisible(!cusps.isEmpty());
  };
  refill();

  // a9zw, the wart before the question, ZEIT VARIIEREN ?
  bool asked = false;
  bool walking = false;
  const double jd0 = last_chart_->jd_ut;
  double jd = jd0;
  QTimer timer;
  timer.setSingleShot(true);
  QElapsedTimer waited;
  int wait_ms = kWaitStartMs;
  bool more_used = false;
  bool less_used = false;
  bool paused = false;
  double djd = 1.0;
  int rf = 1;
  const auto step = [&]() {
    // his jd = jd + rf& * djd with @dat, @jseckp and @a316061
    jd += rf * djd;
    apply_moment(jd, label, false, true);
    refill();
    // his ran, "Vorwärts !" or "Rückwärts!"
    ran->setText(theme::heading_span(rf > 0 ? tr("Vorwärts !") : tr("Rückwärts!")));
    QApplication::beep();
    more_used = false;
    less_used = false;
    waited.restart();
    timer.start(wait_ms);
  };
  // the unit chain of zw0, false when ESC ends it
  const auto ask_walk = [&]() {
    const QStringList units{tr("TAGE"), tr("STUNDEN"), tr("MINUTEN")};
    const int rd = ChoiceDialog::ask(&view, tr("AUSWAHL"), {QString(), tr("ZEIT-EINHEIT ?")}, units, 0);
    if (rd < 0) {
      return false;
    }
    // his t$ = " als BELIEBIGE ZAHL EINGEBEN !"
    const std::optional<double> value = ask_number(&view, tr("ZAHLEN-Eingabe !"), units[rd] + tr(" als BELIEBIGE ZAHL EINGEBEN !"),
                                                   -1.0e9, 1.0e9, std::numeric_limits<double>::quiet_NaN(), 6);
    if (!value) {
      return false;
    }
    static constexpr double kUnitDays[3] = {1.0, 1.0 / kHoursPerDay, 1.0 / kMinutesPerDay};
    djd = *value * kUnitDays[rd];
    const int re = ChoiceDialog::ask(&view, tr("AUSWAHL"), {QString(), tr("RICHTUNG ?")}, {tr("VOR"), tr("ZURÜCK")}, 0);
    if (re < 0) {
      return false;
    }
    rf = re == 0 ? 1 : -1;
    // his twart% = 2000, every new unit starts from the plain wait and
    // drops what the mouse added or took before
    wait_ms = kWaitStartMs;
    wander_help_sheet(wait_ms);
    return true;
  };
  // the end of zeitwi, a new unit, the walk on or the end
  const auto walk_end = [&]() {
    timer.stop();
    walking = false;
    const int er = ChoiceDialog::ask(&view, tr("AUSWAHL"), {QString(), tr("Weiter Mit NEUER ZEITEINHEIT ?")},
                                     {tr(" NEIN = ENDE "), tr("Weiter"), tr("Irrtum ( = UNDO = Zurück )")}, 0);
    if (er == 1) {
      if (ask_walk()) {
        walking = true;
        step();
        return;
      }
    } else if (er == 2) {
      walking = true;
      step();
      return;
    }
    // his IF ABS(jd - jd0) > 10 restored the moment without the question
    bool keep = false;
    if (std::abs(jd - jd0) <= kKeepDays && jd != jd0) {
      keep = ChoiceDialog::ask(&view, tr("AUSWAHL"), {QString(), tr("VARIIERTE ZEIT in RADIX ÜBERNEHMEN ?")},
                               {tr(" NEIN "), tr("JA")}, 0) == 1;
    }
    // his JA set od = 1 before @merk and @merkr(1), the varied moment goes
    // into the RADIX slot of the same SATZ number like the question says,
    // also while the table showed a SOLAR slot
    const int satz = active_is_solar_ ? active_solar_ : active_slot_;
    if (keep && satz >= 0) {
      active_is_solar_ = false;
      set_slot(satz, panel_record(), true, radix_labels_[static_cast<std::size_t>(satz)]);
      update_solar_actions();
    } else if (!keep) {
      apply_moment(jd0, label, false, true);
    }
    jd = jd0;
    refill();
    ran->clear();
  };
  view.on_key = [&](int key) {
    if (!asked) {
      if (key == Qt::Key_Escape) {
        return false;
      }
      asked = true;
      // his alertbox "ZEIT VARIIEREN ?" with " NEIN " and "JA"
      const int c = ChoiceDialog::ask(&view, tr("AUSWAHL"), {QString(), tr("ZEIT VARIIEREN ?")}, {tr(" NEIN "), tr("JA")}, 0);
      if (c == 1 && ask_walk()) {
        walking = true;
        step();
      }
      return true;
    }
    if (!walking) {
      return false;
    }
    if (key == Qt::Key_Escape) {
      walk_end();
      return true;
    }
    //RR STOP
    // on the blank, the walk waits for the next key
    if (key == Qt::Key_Space && !paused) {
      timer.stop();
      paused = true;
      return true;
    }
    paused = false;
    switch (key) {
      case Qt::Key_V: rf = 1; break;
      case Qt::Key_R: rf = -1; break;
      // his eingalp sheet, D = 1 Tag, H = 1 Stunde, M = 1 Minute
      case Qt::Key_D: djd = 1.0; break;
      case Qt::Key_H: djd = 1.0 / kHoursPerDay; break;
      case Qt::Key_M: djd = 1.0 / kMinutesPerDay; break;
      // and + = Verdoppelung, - = Halbierung
      case Qt::Key_Plus: djd *= 2.0; break;
      case Qt::Key_Minus: djd /= 2.0; break;
      default: break;
    }
    timer.stop();
    step();
    return true;
  };
  view.on_mouse = [&](Qt::MouseButton b) {
    if (!walking || paused || !timer.isActive()) {
      return;
    }
    // his sheet, Linke Maustaste = + 2 Sekunden, Rechte Maustaste = - 1 Sekunde
    if (b == Qt::LeftButton && !more_used) {
      wait_ms += kWaitMoreMs;
      more_used = true;
    } else if (b == Qt::RightButton && !less_used) {
      wait_ms = std::max(wait_ms - kWaitLessMs, kWaitLeastMs);
      less_used = true;
    } else {
      return;
    }
    timer.start(std::max(0, wait_ms - static_cast<int>(waited.elapsed())));
  };
  connect(&timer, &QTimer::timeout, &view, step);
  view.resize(std::max(width(), 1000), std::max(height(), 640));
  view.exec();
  timer.stop();
  // a closed window ends a running walk like ESC, the moment returns
  if (walking || jd != jd0) {
    apply_moment(jd0, label, false, true);
  }
}

}  // namespace horcom
