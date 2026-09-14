// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "main_window.hpp"

#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QSettings>
#include <QTableWidget>
#include <QTimeEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <initializer_list>

#include "aspektarium_dialog.hpp"
#include "banner.hpp"
#include "direction_list_dialog.hpp"
#include "horcom/chart/composite.hpp"
#include "horcom/chart/directions.hpp"
#include "horcom/chart/dynamogram.hpp"
#include "horcom/chart/harmonics.hpp"
#include "horcom/chart/mundane.hpp"
#include "horcom/chart/arabic.hpp"
#include "horcom/chart/progressions.hpp"
#include "horcom/chart/planet_points.hpp"
#include "horcom/chart/rhythm.hpp"
#include "horcom/chart/riseset.hpp"
#include "horcom/chart/stars.hpp"
#include "horcom/chart/transit_search.hpp"
#include "ingress_dialog.hpp"
#include "kommen_dialog.hpp"
#include "painter.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"
#include "horcom/data/place_file.hpp"
#include "horcom/ephem/eclipses.hpp"
#include "horcom/ephem/precession.hpp"
#include "horcom/time/delta_t.hpp"
#include "horcom/time/sidereal.hpp"
#include "horcom/chart/histogram.hpp"
#include "horcom/data/statist.hpp"
#include "horcom/render/linear.hpp"
#include "horcom/render/svg.hpp"
#include "place_dialog.hpp"
#include "record_dialog.hpp"
#include "statist_dialog.hpp"
#include "theme.hpp"
#include "transit_list_dialog.hpp"
#include "wheel_widget.hpp"
#include "zone_dialog.hpp"

namespace horcom {

namespace {

// the sign tags of the original zei$ table
constexpr const char* kSignTag[12] = {"AR", "TA", "GM", "CN", "LE", "VI", "LI", "SC", "SG", "CP", "AQ", "PS"};

//RR Bei Uhr alle 15 sek neu
constexpr int kClockRedrawMs = 15000;

// splits decimal degrees into the AAF degree minute second fields
void to_dms(double value, int& deg, int& min, int& sec) {
  const double a = std::abs(value);
  deg = static_cast<int>(a);
  const double rem = (a - deg) * 60.0;
  min = static_cast<int>(rem);
  sec = static_cast<int>((rem - min) * 60.0 + 0.5);
  if (sec >= 60) {
    sec -= 60;
    ++min;
  }
  if (min >= 60) {
    min -= 60;
    ++deg;
  }
}

QString zodiac(double rad) {
  const double deg = norm_deg(rad * kRadToDeg);
  const int sign = static_cast<int>(deg / kDegPerSign);
  const double in_sign = deg - sign * kDegPerSign;
  int total = static_cast<int>(in_sign * 3600.0 + 0.5);
  return QString::asprintf("%2d %s %02d'%02d\"", total / 3600, kSignTag[sign], (total / 60) % 60, total % 60);
}

QString degs(double rad) {
  return QString::asprintf("%+9.4f", rad * kRadToDeg);
}

// the comparison list body, running body, separation, radix body
QString cross_hits_text(const std::vector<CrossAspectHit>& hits) {
  QString out;
  int shown = 0;
  for (const CrossAspectHit& h : hits) {
    if (shown >= 14) {
      out += QString::fromUtf8("…");
      break;
    }
    if (shown > 0) {
      out += ",  ";
    }
    out += QString("%1 %2° %3")
               .arg(QString::fromUtf8(body::kTag[static_cast<std::size_t>(h.w)].data(),
                                      static_cast<int>(body::kTag[static_cast<std::size_t>(h.w)].size())))
               .arg(qRound(h.sep_deg))
               .arg(QString::fromUtf8(body::kTag[static_cast<std::size_t>(h.t)].data(),
                                      static_cast<int>(body::kTag[static_cast<std::size_t>(h.t)].size())));
    ++shown;
  }
  return out;
}

// lifts one 128 byte chart record into the exchange format, the DAT
// clock is already Universal Time
AafRecord aaf_from_chart_record(const ChartRecord& c) {
  AafRecord a;
  a.surname = c.name;
  a.day = c.day;
  a.month = c.month;
  a.year = c.year;
  a.hour = static_cast<int>(c.hour);
  a.minute = static_cast<int>(c.minute);
  a.second = static_cast<int>((c.minute - static_cast<int>(c.minute)) * 60.0 + 0.5);
  a.place = c.place;
  a.comment = c.remark;
  a.calendar = c.calendar();
  a.lat_ns = c.lat < 0 ? 'S' : 'N';
  a.lon_ew = c.lon < 0 ? 'W' : 'E';
  const double alat = std::abs(c.lat);
  const double alon = std::abs(c.lon);
  a.lat_deg = static_cast<int>(alat);
  a.lat_min = static_cast<int>((alat - a.lat_deg) * 60.0);
  a.lat_sec = static_cast<int>(((alat - a.lat_deg) * 60.0 - a.lat_min) * 60.0 + 0.5);
  a.lon_deg = static_cast<int>(alon);
  a.lon_min = static_cast<int>((alon - a.lon_deg) * 60.0);
  a.lon_sec = static_cast<int>(((alon - a.lon_deg) * 60.0 - a.lon_min) * 60.0 + 0.5);
  a.zone = "00hE00:00";
  return a;
}

}  // namespace

MainWindow::MainWindow(VsopTables vsop, Ephemerides eph, std::filesystem::path data_dir, QWidget* parent)
    : QMainWindow(parent), vsop_(std::move(vsop)), eph_(std::move(eph)), data_dir_(std::move(data_dir)) {
  // his final profile is the startup default, a konsta file placed next
  // to the data still overrides it, the historical name included
  konsta_ = robert_profile();
  for (const char* name : {"konsta.int", "KONSTA7P.INT"}) {
    if (const auto k = load_konsta(data_dir_ / name)) {
      konsta_ = *k;
      break;
    }
  }
  aspect_settings_ = konsta_.aspect_settings();
  build_ui();
  recompute();
}

void MainWindow::build_ui() {
  setWindowTitle("HORCOM");
  setWindowIcon(QIcon(":/logo.svg"));

  // the banner rides a locked toolbar so it spans the whole window
  // above the docks
  banner_ = new Banner(this);
  auto* banner_bar = new QToolBar(this);
  banner_bar->setObjectName("bannerBar");
  banner_bar->setMovable(false);
  banner_bar->setFloatable(false);
  banner_bar->setAllowedAreas(Qt::TopToolBarArea);
  banner_bar->setContextMenuPolicy(Qt::PreventContextMenu);
  banner_bar->addWidget(banner_);
  addToolBar(Qt::TopToolBarArea, banner_bar);

  wheel_ = new WheelWidget(this);
  setCentralWidget(wheel_);

  // the input panel
  auto* input_dock = new QDockWidget(tr("Eingabe"), this);
  input_dock->setFeatures(QDockWidget::DockWidgetMovable);
  auto* form_host = new QWidget(input_dock);
  auto* form = new QFormLayout(form_host);
  date_ = new QDateEdit(QDate(1992, 10, 13), form_host);
  date_->setCalendarPopup(true);
  date_->setDisplayFormat("dd.MM.yyyy");
  time_ = new QTimeEdit(QTime(3, 0), form_host);
  time_->setDisplayFormat("HH:mm:ss");
  // arrow keys and the mouse wheel step the fields, the tiny stepper
  // buttons would only clutter the panel
  time_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  zone_ = new QDoubleSpinBox(form_host);
  zone_->setRange(-14.0, 14.0);
  zone_->setDecimals(2);
  zone_->setSingleStep(0.5);
  zone_->setValue(0.0);
  zone_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  lon_ = new QDoubleSpinBox(form_host);
  lon_->setRange(-180.0, 180.0);
  lon_->setDecimals(4);
  lon_->setValue(11.3244);
  lon_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  lat_ = new QDoubleSpinBox(form_host);
  lat_->setRange(-89.99, 89.99);
  lat_->setDecimals(4);
  lat_->setValue(48.1742);
  lat_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  houses_ = new QComboBox(form_host);
  // his menu order in hausw
  houses_->addItems({"PLACIDUS", "TOPOZENTRISCH", "KOCH-GOH", "REGIOMONTANUS", "CAMPANUS",
                     "ÄQUAL EKLIPTIKAL ab AC", "ÄQUAL n. VEHLOW", "NUR AC und MC", "KEINE"});
  parallax_ = new QCheckBox(tr("Parallaxe (topozentrisch)"), form_host);
  // off at startup like the original klpl!
  extras_ = new QCheckBox(tr("Zusatzplaneten"), form_host);
  true_node_ = new QCheckBox(tr("Wahrer Mondknoten"), form_host);
  true_apogee_ = new QCheckBox(tr("Wahres Apogäum"), form_host);
  //RR HELIOZENTRISCH
  helio_ = new QCheckBox(tr("Heliozentrisch"), form_host);
  form->addRow(tr("Datum"), date_);
  form->addRow(tr("Zeit"), time_);
  // the zone field carries a picker into the zone name catalogue
  auto* zone_row = new QWidget(form_host);
  auto* zone_lay = new QHBoxLayout(zone_row);
  zone_lay->setContentsMargins(0, 0, 0, 0);
  zone_lay->setSpacing(4);
  zone_lay->addWidget(zone_, 1);
  auto* zone_pick = new QToolButton(zone_row);
  zone_pick->setText("…");
  zone_pick->setToolTip(tr("Zeit-Zonen Katalog"));
  zone_lay->addWidget(zone_pick);
  connect(zone_pick, &QToolButton::clicked, this, &MainWindow::pick_zone);
  form->addRow(tr("Zone (h östl.)"), zone_row);
  form->addRow(tr("Länge (Ost +)"), lon_);
  form->addRow(tr("Breite (Nord +)"), lat_);
  form->addRow(tr("Häuser"), houses_);
  form->addRow(parallax_);
  form->addRow(extras_);
  form->addRow(true_node_);
  form->addRow(true_apogee_);
  form->addRow(helio_);
  // the transit moment enters as Greenwich time like the original a20
  transit_on_ = new QCheckBox(tr("Transite"), form_host);
  tdate_ = new QDateEdit(QDate::currentDate(), form_host);
  tdate_->setCalendarPopup(true);
  tdate_->setDisplayFormat("dd.MM.yyyy");
  tdate_->setEnabled(false);
  ttime_ = new QTimeEdit(QTime(12, 0), form_host);
  ttime_->setDisplayFormat("HH:mm");
  ttime_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  ttime_->setEnabled(false);
  form->addRow(transit_on_);
  form->addRow(tr("Transit-Datum"), tdate_);
  form->addRow(tr("Zeit (UT)"), ttime_);
  input_dock->setWidget(form_host);
  addDockWidget(Qt::LeftDockWidgetArea, input_dock);

  // his profile presets the switches, klpl stays off at startup and the
  // KONSTA table only says which extras appear once it goes on
  const ChartSettings preset = konsta_.chart_settings();
  const int preset_houses = static_cast<int>(preset.houses) - 1;
  if (preset_houses >= 0 && preset_houses < houses_->count()) {
    houses_->setCurrentIndex(preset_houses);
  }
  parallax_->setChecked(preset.topocentric_parallax);
  true_node_->setChecked(preset.true_node);
  true_apogee_->setChecked(preset.true_apogee);
  // the preferred place of the original ORT.EXT seeds the coordinates
  if (const auto home = read_preferred_place(data_dir_ / "ort.ext")) {
    lon_->setValue(home->lon);
    lat_->setValue(home->lat);
  }

  // the result docks
  auto* body_dock = new QDockWidget(tr("Koordinaten"), this);
  bodies_ = new QTableWidget(0, 11, body_dock);
  //RR Spalte A ( = Acceleratio ) enthält das Vorzeichen der Beschleunigung
  // and ENTF carries the mutual distance in AU, then the mean node and
  // apsis points after Landscheidt
  bodies_->setHorizontalHeaderLabels({tr("Länge"), tr("Breite"), tr("Deklin."), tr("Geschw."), "A", tr("Entf."),
                                      tr("Kn.ND"), tr("Kn.SD"), tr("Perihel"), tr("Aphel"), ""});
  bodies_->horizontalHeader()->setStretchLastSection(true);
  bodies_->verticalHeader()->setDefaultSectionSize(18);
  bodies_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  body_dock->setWidget(bodies_);
  addDockWidget(Qt::RightDockWidgetArea, body_dock);

  auto* cusp_dock = new QDockWidget(tr("Häuser"), this);
  auto* cusp_host = new QWidget(cusp_dock);
  auto* cusp_layout = new QVBoxLayout(cusp_host);
  cusps_ = new QTableWidget(12, 1, cusp_host);
  cusps_->setHorizontalHeaderLabels({tr("Spitze")});
  cusps_->horizontalHeader()->setStretchLastSection(true);
  cusps_->verticalHeader()->setDefaultSectionSize(18);
  cusps_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  aspects_label_ = new QLabel(cusp_host);
  aspects_label_->setWordWrap(true);
  aspects_label_->setObjectName("aspectsLine");
  cusp_layout->setContentsMargins(0, 6, 0, 0);
  cusp_layout->addWidget(cusps_, 1);
  cusp_layout->addWidget(aspects_label_);
  cusp_dock->setWidget(cusp_host);
  addDockWidget(Qt::RightDockWidgetArea, cusp_dock);
  resizeDocks({body_dock, cusp_dock}, {395, 395}, Qt::Horizontal);

  // the menu
  //RR EIN-AUSG. | EPHEMERIDE | HOROSKOPE | AUSWERTUNG | DIVERSES
  QMenu* file = menuBar()->addMenu(tr("&Datei"));
  QMenu* ephem = menuBar()->addMenu(tr("&Ephemeride"));
  QMenu* horo = menuBar()->addMenu(tr("&Horoskope"));
  QMenu* ausw = menuBar()->addMenu(tr("&Auswertung"));
  QMenu* divers = menuBar()->addMenu(tr("&Diverses"));
  file->addAction(tr("Datensätze öffnen…"), QKeySequence::Open, this, &MainWindow::open_records);
  file->addAction(tr("Datensatz bearbeiten…"), QKeySequence(Qt::CTRL | Qt::Key_D), this, &MainWindow::edit_record);
  file->addAction(tr("Ort suchen…"), QKeySequence(Qt::CTRL | Qt::Key_L), this, &MainWindow::open_place);
  file->addAction(tr("Als AAF speichern…"), QKeySequence::Save, this, &MainWindow::save_aaf);
  file->addAction(tr("Horoskop als SVG…"), this, &MainWindow::export_svg);
  //RR DRUCKER-GRAPHIK, the druck_graph_ein world over one shared painter
  file->addAction(tr("Horoskop als PDF…"), this, &MainWindow::export_pdf);
  file->addAction(tr("Drucken…"), QKeySequence::Print, this, &MainWindow::print_chart);
  file->addAction(tr("Dateien verketten…"), this, &MainWindow::chain_files);
  file->addAction(tr("Statistik-Datei erstellen…"), this, &MainWindow::create_statistics);
  file->addAction(tr("AAF-Datei → HORCOM-Datei…"), this, &MainWindow::aaf_to_dat);
  file->addAction(tr("Datei trimmen/minimieren…"), this, &MainWindow::tidy_file);
  file->addAction(tr("Vorgaben (Orbes, Fixpunkt)…"), this, &MainWindow::orb_settings);
  file->addSeparator();
  file->addAction(tr("Beenden"), QKeySequence::Quit, this, &QWidget::close);
  // the return charts of his solar and lunar menu
  ausw->addAction(tr("Solar…"), this, &MainWindow::solar_chart);
  ausw->addAction(tr("Lunar…"), this, &MainWindow::lunar_chart);
  ausw->addAction(tr("Septar…"), this, &MainWindow::septar_chart);
  ausw->addAction(tr("Solar-Liste…"), this, [this]() { return_list(false); });
  ausw->addAction(tr("Lunar-Liste…"), this, [this]() { return_list(true); });
  ausw->addAction(tr("Planetar…"), this, &MainWindow::planetar_chart);
  ausw->addAction(tr("Personar…"), this, &MainWindow::personar_chart);
  ausw->addAction(tr("Progressions-Horoskop…"), this, &MainWindow::progression_chart);
  ausw->addAction(tr("Tages-Horoskop…"), this, &MainWindow::day_chart);
  ausw->addSeparator();
  ausw->addAction(tr("Transit-Liste…"), this, &MainWindow::transit_list);
  ephem->addAction(tr("Ingresse…"), this, &MainWindow::ingress_table);
  horo->addAction(tr("Aspektarium…"), this, &MainWindow::open_aspektarium);
  horo->addAction(tr("Halbsummen-Bäume…"), this, &MainWindow::midpoint_tree);
  horo->addAction(tr("Histogramme…"), this, &MainWindow::histogram_view);
  ephem->addAction(tr("Statistik…"), this, &MainWindow::open_statistics);
  ephem->addAction(tr("Grad-Liste…"), this, &MainWindow::degree_list);
  ephem->addAction(tr("Fixsterne…"), this, &MainWindow::fixed_star_table);
  ephem->addAction(tr("Arabische Teile…"), this, &MainWindow::arabic_table);
  ephem->addSeparator();
  ephem->addAction(tr("Umrechnungen…"), this, &MainWindow::converters);
  divers->addAction(tr("Häuser-Tabelle…"), this, &MainWindow::house_table);
  divers->addAction(tr("Aufgang/Untergang…"), this, &MainWindow::rise_set);
  divers->addAction(tr("Finsternisse…"), this, &MainWindow::eclipse_table);
  divers->addAction(tr("Großes Jahr…"), this, &MainWindow::great_year);
  divers->addSeparator();
  divers->addAction(tr("Korrektur…"), this, &MainWindow::correction);
  divers->addAction(tr("Zeit-Wandern…"), this, &MainWindow::time_wander);
  divers->addAction(tr("Ort-Wandern…"), this, &MainWindow::place_wander);
  ausw->addSeparator();
  ausw->addAction(tr("Rhythmenlehre (Auslösungen)…"), this, &MainWindow::rhythm_table);
  ausw->addAction(tr("Grad-Datum-Liste…"), this, &MainWindow::degree_date_list);
  ausw->addAction(tr("Dynamogramm…"), this, &MainWindow::dynamogram_view);
  ausw->addAction(tr("Linear-Graphik…"), this, &MainWindow::linear_graph);
  // the direction tables of the original evaluation menu in one place
  ausw->addAction(tr("Direktionen-Auswertung…"), this, [this]() {
    if (!last_chart_) {
      return;
    }
    DirectionListDialog dialog(*last_chart_, make_context(), this);
    dialog.exec();
  });
  // the double wheel of a12, a second person over the radix
  compare_action_ = horo->addAction(tr("Vergleich"));
  compare_action_->setCheckable(true);
  connect(compare_action_, &QAction::toggled, this, [this](bool on) {
    if (!on) {
      partner_chart_.reset();
      partner_name_.clear();
      recompute();
      banner_->set_record(record_label_.trimmed());
      return;
    }
    const auto r = choose_record(tr("Vergleichs-Datensatz wählen"));
    if (!r || !set_partner(*r)) {
      const QSignalBlocker block(compare_action_);
      compare_action_->setChecked(false);
      return;
    }
    recompute();
  });
  //RR 90°-KREIS, the second mode of the a12 double wheel
  dial_action_ = horo->addAction(tr("90°-Kreis"));
  dial_action_->setCheckable(true);
  connect(dial_action_, &QAction::toggled, this, [this](bool on) {
    if (on && !partner_chart_) {
      compare_action_->setChecked(true);
      if (!partner_chart_) {
        const QSignalBlocker block(dial_action_);
        dial_action_->setChecked(false);
        return;
      }
    }
    recompute();
  });
  //RR HARMONICS = GRUNDHOROSKOP * GANZZAHLIGEM FAKTOR !
  harmonic_action_ = horo->addAction(tr("Harmonic…"));
  harmonic_action_->setCheckable(true);
  connect(harmonic_action_, &QAction::toggled, this, [this](bool on) {
    if (!on) {
      harm_n_ = 0;
      recompute();
      banner_->set_record(record_label_.trimmed());
      return;
    }
    bool ok = false;
    //RR ORDNUNGS-ZAHL der HARMONIC !
    const int n = QInputDialog::getInt(this, tr("Harmonic"), tr("Ordnungs-Zahl der Harmonic"), 5, 1, 360, 1, &ok);
    if (!ok) {
      const QSignalBlocker block(harmonic_action_);
      harmonic_action_->setChecked(false);
      return;
    }
    harm_n_ = n;
    // the haus_ber question, like planets is his standard
    harm_new_mc_ = QMessageBox::question(this, tr("Harmonic"),
                                         tr("Häuser aufgrund des neuen MC neu berechnen?\n(Nein behandelt sie wie Planeten, der Standard.)"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
    recompute();
  });
  // the six age directed outer wheels of the original MULTI menu
  multi_action_ = horo->addAction(tr("Multi-Direktionen…"));
  multi_action_->setCheckable(true);
  connect(multi_action_, &QAction::toggled, this, [this](bool on) {
    if (!on) {
      multi_event_jd_ = 0.0;
      recompute();
      banner_->set_record(record_label_.trimmed());
      return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Multi-Direktionen"));
    auto* v = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout();
    auto* mode = new QComboBox(&dialog);
    mode->addItem("MULTI 1", 1);
    mode->addItem("MULTI 2", 2);
    mode->addItem("MULTI 3", 3);
    mode->addItem("MULTI-0-OST", 4);
    mode->addItem("MULTI-0-WEST", 5);
    mode->addItem("MULTI-ARC", 6);
    auto* when = new QDateEdit(QDate::currentDate(), &dialog);
    when->setCalendarPopup(true);
    when->setDisplayFormat("dd.MM.yyyy");
    // the reference point feeds only MULTI 3 and MULTI-ARC
    auto* ref = new QComboBox(&dialog);
    for (int slot = 1; slot <= 14; ++slot) {
      ref->addItem(QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                                     static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size())),
                   slot);
    }
    for (int h = 1; h <= 12; ++h) {
      ref->addItem(tr("Spitze H%1").arg(h), 100 + h);
    }
    for (int h = 1; h <= 12; ++h) {
      ref->addItem(tr("Herr H%1 (alt)").arg(h), 200 + h);
    }
    for (int z = 1; z <= 12; ++z) {
      ref->addItem(tr("0° %1").arg(kSignTag[z - 1]), 300 + z);
    }
    auto* hneu = new QCheckBox(tr("Häuser aufgrund des neuen MC neu berechnen"), &dialog);
    form->addRow(tr("Modus"), mode);
    form->addRow(tr("Ereignis-Datum"), when);
    form->addRow(tr("Bezugspunkt"), ref);
    v->addLayout(form);
    v->addWidget(hneu);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    v->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) {
      const QSignalBlocker block(multi_action_);
      multi_action_->setChecked(false);
      return;
    }
    multi_mode_ = static_cast<MultiMode>(mode->currentData().toInt());
    const QDate d = when->date();
    multi_event_jd_ = julian_day({d.day(), d.month(), d.year(), 0, 0.0}, current_settings().calendar);
    const int id = ref->currentData().toInt();
    multi_ref_ = MultiReference{};
    if (id >= 300) {
      multi_ref_.kind = MultiReference::Kind::kSignStart;
      multi_ref_.sign = id - 300;
    } else if (id >= 200) {
      multi_ref_.kind = MultiReference::Kind::kRuler;
      multi_ref_.house = id - 200;
    } else if (id >= 100) {
      multi_ref_.kind = MultiReference::Kind::kCusp;
      multi_ref_.house = id - 100;
    } else {
      multi_ref_.body = id;
    }
    multi_new_mc_ = hneu->isChecked();
    recompute();
  });
  // the composite over the same partner, house mode from his profile
  composite_action_ = horo->addAction(tr("Composit"));
  composite_action_->setCheckable(true);
  connect(composite_action_, &QAction::toggled, this, [this](bool on) {
    if (on && !partner_chart_) {
      const auto r = choose_record(tr("Vergleichs-Datensatz wählen"));
      if (!r || !set_partner(*r)) {
        const QSignalBlocker block(composite_action_);
        composite_action_->setChecked(false);
        return;
      }
    }
    recompute();
    if (!on && !partner_chart_) {
      banner_->set_record(record_label_.trimmed());
    }
  });
  horo->addAction(tr("Combin…"), this, &MainWindow::combin_chart);
  // the primary directed axes of prima with his sidereal time variation
  directions_action_ = ausw->addAction(tr("Direktionen…"));
  directions_action_->setCheckable(true);
  connect(directions_action_, &QAction::toggled, this, [this](bool on) {
    if (!on) {
      recompute();
      banner_->set_record(record_label_.trimmed());
      return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Direktionen"));
    auto* v = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout();
    auto* when = new QDateEdit(QDate::currentDate(), &dialog);
    when->setCalendarPopup(true);
    when->setDisplayFormat("dd.MM.yyyy");
    auto* dirbox = new QComboBox(&dialog);
    dirbox->addItem(tr("DIREKT ( + )"));
    dirbox->addItem(tr("KONVERS ( - )"));
    auto* vary = new QDoubleSpinBox(&dialog);
    vary->setRange(-30.0, 30.0);
    vary->setDecimals(3);
    vary->setSingleStep(0.25);
    //RR 1° STZ entspr. 4 Zeitminuten
    form->addRow(tr("Ereignis-Datum"), when);
    form->addRow(tr("Richtung"), dirbox);
    form->addRow(tr("STZ-Variation (°)"), vary);
    //RR die Verschiebungen können in einem Summenspeicher aufsummiert werden
    auto* accumulate = new QCheckBox(tr("Variation aufsummieren"), &dialog);
    auto* sum_label = new QLabel(vary_count_ > 0
                                     ? tr("Summenspeicher %1° aus %2 Variationen, Mittel %3°")
                                           .arg(vary_sum_, 0, 'f', 3)
                                           .arg(vary_count_)
                                           .arg(vary_sum_ / vary_count_, 0, 'f', 3)
                                     : tr("Summenspeicher leer."),
                                 &dialog);
    sum_label->setWordWrap(true);
    auto* note = new QLabel(tr("1° STZ entspricht 4 Zeitminuten."), &dialog);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    //RR der Mittelwert der Summe kann ins Radix übernommen werden
    auto* take = buttons->addButton(tr("Mittel ins Radix übernehmen"), QDialogButtonBox::ActionRole);
    take->setEnabled(vary_count_ > 0);
    connect(take, &QPushButton::clicked, &dialog, [this, &dialog]() {
      const double mean = vary_sum_ / vary_count_;
      vary_sum_ = 0.0;
      vary_count_ = 0;
      const double jd = last_chart_->jd_ut + mean / kDegPerHour / kSolarToSiderealRate / 24.0;
      dialog.reject();
      apply_moment(jd, tr("KORRIGIERT"));
    });
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    v->addLayout(form);
    v->addWidget(accumulate);
    v->addWidget(sum_label);
    v->addWidget(note);
    v->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) {
      const QSignalBlocker block(directions_action_);
      directions_action_->setChecked(false);
      return;
    }
    const QDate d = when->date();
    dir_jd_ = julian_day({d.day(), d.month(), d.year(), 0, 0.0}, current_settings().calendar);
    dir_converse_ = dirbox->currentIndex() == 1;
    dir_vary_ = vary->value();
    if (accumulate->isChecked() && dir_vary_ != 0.0) {
      vary_sum_ += dir_vary_;
      ++vary_count_;
    }
    recompute();
  });
  // the horm 2 view, semi arc house space instead of the ecliptic
  mundane_action_ = horo->addAction(tr("Mundan"));
  mundane_action_->setCheckable(true);
  connect(mundane_action_, &QAction::toggled, this, [this](bool on) {
    recompute();
    if (!on) {
      banner_->set_record(record_label_.trimmed());
    }
  });
  divers->addSeparator();
  //RR Solange UHR SICHTBAR wird HOROSKOP ALLE 15 SEK NACHGEZEICHNET !
  clock_action_ = divers->addAction(tr("Uhr"));
  clock_action_->setCheckable(true);
  clock_timer_ = new QTimer(this);
  clock_timer_->setInterval(kClockRedrawMs);
  connect(clock_timer_, &QTimer::timeout, this, &MainWindow::recompute);
  connect(clock_action_, &QAction::toggled, this, [this](bool on) {
    if (on) {
      clock_timer_->start();
    } else {
      clock_timer_->stop();
      banner_->set_record(record_label_.trimmed());
    }
    recompute();
  });
  // the text scale of the shell, the wheel keeps its own canvas scale
  QMenu* view = menuBar()->addMenu(tr("&Ansicht"));
  const auto set_scale = [](int scale) {
    QSettings settings;
    const int s = std::clamp(scale, theme::kTextScaleMin, theme::kTextScaleMax);
    settings.setValue(theme::kTextScaleKey, s);
    qApp->setStyleSheet(theme::stylesheet(s));
  };
  const auto scale_now = []() { return QSettings().value(theme::kTextScaleKey, theme::kTextScaleNormal).toInt(); };
  view->addAction(tr("Schrift größer"), QKeySequence::ZoomIn, this, [set_scale, scale_now]() { set_scale(scale_now() + theme::kTextScaleStep); });
  view->addAction(tr("Schrift kleiner"), QKeySequence::ZoomOut, this, [set_scale, scale_now]() { set_scale(scale_now() - theme::kTextScaleStep); });
  view->addAction(tr("Normale Schrift"), QKeySequence(Qt::CTRL | Qt::Key_0), this, [set_scale]() { set_scale(theme::kTextScaleNormal); });
  view->addSeparator();
  view->addAction(tr("Planeten-Auswahl…"), this, &MainWindow::planet_selection);
  view->addSeparator();
  // the language survives in the settings and applies on the next start
  QMenu* language = view->addMenu(tr("Sprache / Language"));
  auto* lang_group = new QActionGroup(language);
  const QString current = QSettings().value("language").toString();
  const auto add_lang = [this, language, lang_group, current](const QString& label, const QString& code) {
    QAction* a = language->addAction(label);
    a->setCheckable(true);
    a->setActionGroup(lang_group);
    a->setChecked(current == code);
    connect(a, &QAction::triggered, this, [this, code]() {
      QSettings().setValue("language", code);
      QMessageBox::information(this, "HORCOM",
                               tr("Die Sprache gilt ab dem nächsten Start.\n"
                                  "The language applies from the next start."));
    });
    return a;
  };
  add_lang(tr("Automatisch (Systemsprache)"), QString());
  add_lang("Deutsch", "de");
  add_lang("English", "en");

  QMenu* help = menuBar()->addMenu(tr("&Hilfe"));
  //RR TEXT-DATEI LESEN, his commentary texts from the local folder
  help->addAction(tr("Original-Kommentare…"), QKeySequence(Qt::Key_F1), this, [this]() {
    KommenDialog dialog(data_dir_ / "kommen", this);
    dialog.exec();
  });
  help->addAction(tr("Über HORCOM"), this, &MainWindow::about);

  // recompute on every change like the original recalculated per screen
  connect(date_, &QDateEdit::dateChanged, this, &MainWindow::recompute);
  connect(time_, &QTimeEdit::timeChanged, this, &MainWindow::recompute);
  connect(zone_, &QDoubleSpinBox::valueChanged, this, &MainWindow::recompute);
  connect(lon_, &QDoubleSpinBox::valueChanged, this, &MainWindow::recompute);
  connect(lat_, &QDoubleSpinBox::valueChanged, this, &MainWindow::recompute);
  connect(houses_, &QComboBox::currentIndexChanged, this, &MainWindow::recompute);
  for (QCheckBox* box : {parallax_, extras_, true_node_, true_apogee_, helio_}) {
    connect(box, &QCheckBox::toggled, this, &MainWindow::recompute);
  }
  connect(transit_on_, &QCheckBox::toggled, this, [this](bool on) {
    tdate_->setEnabled(on);
    ttime_->setEnabled(on);
    recompute();
  });
  connect(tdate_, &QDateEdit::dateChanged, this, &MainWindow::recompute);
  connect(ttime_, &QTimeEdit::timeChanged, this, &MainWindow::recompute);
  resize(1280, 760);
}

ChartInput MainWindow::current_input() const {
  ChartInput in;
  const QDate d = date_->date();
  const QTime t = time_->time();
  CalendarDate local{d.day(), d.month(), d.year(), static_cast<double>(t.hour()),
                     t.minute() + t.second() / 60.0};
  // zone hours east of Greenwich lead back to UT by subtraction
  const double jd_ut = julian_day(local) - zone_->value() / 24.0;
  in.date_ut = calendar_date(jd_ut);
  in.lon_deg_east = lon_->value();
  in.lat_deg = lat_->value();
  return in;
}

ChartSettings MainWindow::current_settings() const {
  // his profile carries the base, the panel switches ride on top
  ChartSettings s = konsta_.chart_settings();
  s.houses = static_cast<HouseSystem>(houses_->currentIndex() + 1);
  s.topocentric_parallax = parallax_->isChecked();
  s.true_node = true_node_->isChecked();
  s.true_apogee = true_apogee_->isChecked();
  s.heliocentric = helio_ != nullptr && helio_->isChecked();
  if (extras_->isChecked()) {
    if (!s.extra_bodies) {
      s.enable_standard_extras();
    }
    s.extra_bodies = true;
  } else {
    s.extra_bodies = false;
  }
  return s;
}

void MainWindow::recompute() {
  // the running clock chart shows the moment itself, the panel keeps
  // its radix untouched
  const bool clock = clock_action_ != nullptr && clock_action_->isChecked();
  ChartInput in;
  if (clock) {
    const QDateTime now = QDateTime::currentDateTimeUtc();
    in.date_ut = {now.date().day(), now.date().month(), now.date().year(),
                  static_cast<double>(now.time().hour()),
                  now.time().minute() + now.time().second() / 60.0};
    in.lon_deg_east = lon_->value();
    in.lat_deg = lat_->value();
  } else {
    in = current_input();
  }
  const ChartSettings s = current_settings();
  Chart chart = compute_chart(in, s, vsop_, eph_);
  // the horm 2 transform runs before every scanner like the original,
  // the hrg mode has no houses so mundane stays out like fixpunkt_def
  const bool mundane = mundane_action_ != nullptr && mundane_action_->isChecked() && !clock && !s.heliocentric;
  if (mundane && chart.ok) {
    to_mundane(chart, in.lat_deg);
  }
  if (!chart.ok) {
    //RR Geog. Breite zu groß !
    banner_->set_record(tr("Geog. Breite zu groß für dieses Häusersystem"));
    return;
  }
  // the user defined fixed point rides on slot zero like fixpunkt_def
  if (fixpunkt_ >= 0.0 && !s.heliocentric) {
    chart.b[0].present = true;
    chart.b[0].valid = true;
    chart.b[0].el = fixpunkt_;
    if (mundane) {
      chart.b[0].el = mundane_longitude(fixpunkt_, kEps, chart.smo.ekls, chart.armc_deg * kDegToRad, in.lat_deg);
    }
  }
  const AspectResult aspects = scan_aspects(chart, s, aspect_settings_);
  last_chart_ = chart;
  last_aspects_ = aspects;

  // the chart data block for the left margin of the paper, like bes11
  WheelOptions wopt;
  wopt.emphasis = emphasis_;
  if (chords_set_) {
    wopt.chord_divisor = chords_;
  }
  //RR der GEBURTSHERRSCHER wird hervorgehoben
  if (ruler_red_ && !current_settings().heliocentric && last_chart_ && last_chart_->houses.ok) {
    const int kp = sign_ruler(last_chart_->houses.cusp[1], false);
    if (kp > 0 && kp < body::kSlotCount && wopt.emphasis[static_cast<std::size_t>(kp)] == 0) {
      wopt.emphasis[static_cast<std::size_t>(kp)] = 1;
    }
  }
  {
    QString name = clock ? QStringLiteral("UHR")
                         : QString("%1 %2")
                               .arg(QString::fromStdString(record_.surname), QString::fromStdString(record_.given))
                               .trimmed();
    if (!name.isEmpty()) {
      wopt.info_lines.push_back(name.toStdString());
    }
    if (!clock && !record_.place.empty()) {
      wopt.info_lines.push_back(record_.place);
    }
    const CalendarDate& dd = in.date_ut;
    int sec = static_cast<int>((dd.hour * 60.0 + dd.minute) * 60.0 + 0.5);
    if (sec >= kSecondsPerDay) {
      sec = kSecondsPerDay - 1;
    }
    //RR day_w$, der WOCHENTAG im HOROSKOP-Formular
    static constexpr const char* kWeekday[7] = {QT_TR_NOOP("Sonntag"),    QT_TR_NOOP("Montag"),
                                                QT_TR_NOOP("Dienstag"),   QT_TR_NOOP("Mittwoch"),
                                                QT_TR_NOOP("Donnerstag"), QT_TR_NOOP("Freitag"),
                                                QT_TR_NOOP("Samstag")};
    const int wd = static_cast<int>(std::fmod(chart.jd_ut + 1.5, 7.0));
    wopt.info_lines.push_back(
        QString::asprintf("%02d.%02d.%04d", dd.day, dd.month, dd.year).toStdString());
    if (wd >= 0 && wd < 7) {
      wopt.info_lines.push_back(tr(kWeekday[wd]).toStdString());
    }
    wopt.info_lines.push_back(
        QString::asprintf("%02d:%02d:%02d UT", sec / 3600, (sec / 60) % 60, sec % 60).toStdString());
    wopt.info_lines.push_back(QString::fromUtf8("L %1°  B %2°")
                                  .arg(in.lon_deg_east, 0, 'f', 2)
                                  .arg(in.lat_deg, 0, 'f', 2)
                                  .toStdString());
    if (s.heliocentric) {
      //RR Heliozentrisch
      wopt.info_lines.push_back(tr("Heliozentrisch").toStdString());
    } else {
      QString hs = QString::fromUtf8(chart.houses.name.data(), static_cast<int>(chart.houses.name.size())).trimmed();
      if (s.topocentric_parallax) {
        //RR MitParall.
        hs += "  MitParall.";
      }
      wopt.info_lines.push_back(hs.toStdString());
    }
    wopt.heliocentric = s.heliocentric;
  }

  bool transit_drawn = false;
  QString cross_text;
  Chart comp_holder;
  AspectResult comp_aspects_holder;
  Chart dir_holder;
  const Chart* shown = &chart;
  const AspectResult* shown_aspects = &aspects;
  if (clock) {
    WheelOptions opt = wopt;
    //RR " UHR "
    opt.center_label = " UHR ";
    wheel_->set_display_list(build_wheel(chart, s, aspects, opt));
    banner_->set_record(QString("UHR %1 UT").arg(QDateTime::currentDateTimeUtc().time().toString("HH:mm:ss")));
    transit_drawn = true;
  } else if (transit_on_->isChecked()) {
    ChartInput tin;
    const QDate td = tdate_->date();
    const QTime tt = ttime_->time();
    tin.date_ut = {td.day(), td.month(), td.year(), static_cast<double>(tt.hour()),
                   tt.minute() + tt.second() / 60.0};
    tin.lon_deg_east = lon_->value();
    tin.lat_deg = lat_->value();
    const Chart tchart = compute_chart(tin, s, vsop_, eph_);
    if (tchart.ok) {
      WheelOptions opt = wopt;
      opt.center_label =
          QString("TRANSIT=>%1 %2 UT").arg(td.toString("dd.MM.yyyy"), tt.toString("HH:mm")).toStdString();
      wheel_->set_display_list(build_transit_wheel(chart, tchart, s, aspects, opt));
      transit_drawn = true;
      // the comparison list of a12asp with his one degree transit orb
      // rule, running body, separation, radix body
      const std::vector<CrossAspectHit> cross = scan_aspects_between(chart, tchart, aspect_settings_, true);
      cross_text = tr("<span style='color:#D4A94A'>TRANSITE</span>&nbsp; ");
      cross_text += cross.empty() ? tr("keine") : cross_hits_text(cross);
    }
  } else if (mundane) {
    WheelOptions opt = wopt;
    opt.center_label = "MUNDAN";
    wheel_->set_display_list(build_wheel(chart, s, aspects, opt));
    transit_drawn = true;
    banner_->set_record("MUNDAN");
  } else if (directions_action_ != nullptr && directions_action_->isChecked() && dir_jd_ > 0.0 && !s.heliocentric) {
    // the directed axes of prima over the radix positions, no chords
    // like primhorg
    const DirectedAxes d =
        direct_axes(chart, lon_->value(), lat_->value(), dir_jd_, dir_converse_, dir_vary_, s.houses);
    dir_holder = chart;
    dir_holder.houses = d.houses;
    dir_holder.armc_deg = d.armc_deg;
    dir_holder.b[body::kAscendant].el = d.houses.angles.ac;
    dir_holder.b[body::kMc].el = d.houses.angles.mc;
    WheelOptions opt = wopt;
    //RR STZ-DIFF=
    opt.center_label = QString("STZ-DIFF=%1°").arg(d.arc_deg, 0, 'f', 3).toStdString();
    opt.scale = kDirectedWheelScale;
    opt.aspect_lines = false;
    wheel_->set_display_list(build_wheel(dir_holder, s, aspects, opt));
    shown = &dir_holder;
    transit_drawn = true;
    banner_->set_record(QString("%1 %2°")
                            .arg(dir_converse_ ? "KONVERS" : "DIREKT")
                            .arg(d.arc_deg, 0, 'f', 3));
  } else if (multi_action_ != nullptr && multi_action_->isChecked() && multi_event_jd_ > 0.0 && !s.heliocentric) {
    // the directed outer wheel over the radix, one of the six MULTI ages
    static constexpr const char* kMultiName[7] = {"",        "MULTI 1",      "MULTI 2",  "MULTI 3",
                                                  "MULTI-0-OST", "MULTI-0-WEST", "MULTI-ARC"};
    const double lja = (multi_event_jd_ - chart.jd_ut) / chart.ta.tropical_year_days;
    const Chart mchart = multi_chart(chart, multi_mode_, lja, multi_ref_,
                                     multi_new_mc_ ? HarmonicHouses::kFromNewMc : HarmonicHouses::kLikeBodies,
                                     s.houses, lat_->value());
    WheelOptions opt = wopt;
    opt.center_label = kMultiName[static_cast<int>(multi_mode_)];
    wheel_->set_display_list(build_double_wheel(chart, mchart, s, aspects, opt));
    transit_drawn = true;
    //RR " LJ"
    banner_->set_record(QString("%1 = %2 LJ").arg(kMultiName[static_cast<int>(multi_mode_)]).arg(lja, 0, 'f', 3));
  } else if (harmonic_action_ != nullptr && harmonic_action_->isChecked() && harm_n_ > 0 && !s.heliocentric) {
    // the harmonic outside over the radix, the harm21 double wheel
    const Chart hc = harmonic_chart(chart, harm_n_,
                                    harm_new_mc_ ? HarmonicHouses::kFromNewMc : HarmonicHouses::kLikeBodies,
                                    s.houses, lat_->value());
    WheelOptions opt = wopt;
    //RR STR$(ha) + ".HARMONIC"
    opt.center_label = QString("%1.HARMONIC").arg(harm_n_).toStdString();
    wheel_->set_display_list(build_double_wheel(chart, hc, s, aspects, opt));
    transit_drawn = true;
    banner_->set_record(QString("%1.HARMONIC").arg(harm_n_));
  } else if (composite_action_ != nullptr && composite_action_->isChecked() && partner_chart_) {
    // the a13 composite, house mode from his profile flags, the panel
    // place stands in as the Robert Hand residence
    const CompositeHouses mode = konsta_.comp_mstz
                                     ? CompositeHouses::kMeanSidereal
                                     : (konsta_.comp_hand ? CompositeHouses::kRobertHand : CompositeHouses::kSchematic);
    comp_holder = composite_chart(chart, in, *partner_chart_, partner_input_, mode, lat_->value(), s);
    comp_aspects_holder = scan_aspects(comp_holder, s, aspect_settings_);
    WheelOptions opt = wopt;
    opt.center_label = "COMPOSIT";
    wheel_->set_display_list(build_wheel(comp_holder, s, comp_aspects_holder, opt));
    shown = &comp_holder;
    shown_aspects = &comp_aspects_holder;
    transit_drawn = true;
    QString mine = QString::fromStdString(record_.surname).trimmed();
    if (mine.isEmpty()) {
      mine = "RADIX";
    }
    banner_->set_record(QString("COMPOSIT %1-%2").arg(mine, partner_name_));
  } else if (partner_chart_) {
    QString mine = QString::fromStdString(record_.surname).trimmed();
    if (mine.isEmpty()) {
      mine = "RADIX";
    }
    if (dial_action_ != nullptr && dial_action_->isChecked()) {
      //RR 90°-KREIS, everything times four, the scan on the a12f state
      Chart d1 = dial_chart(chart, 4.0);
      Chart d2 = dial_chart(*partner_chart_, 4.0);
      const AspectResult da = scan_aspects(d1, s, aspect_settings_);
      const std::vector<CrossAspectHit> cross = scan_aspects_between(d1, d2, aspect_settings_, false);
      dial_display(d1, 4.0);
      dial_display(d2, 4.0);
      WheelOptions opt = wopt;
      opt.dial = true;
      opt.center_label = "90\xC2\xB0- KREIS";
      wheel_->set_display_list(build_double_wheel(d1, d2, s, da, opt));
      cross_text = tr("<span style='color:#D4A94A'>VERGLEICH 90°</span>&nbsp; ");
      cross_text += cross.empty() ? tr("keine") : cross_hits_text(cross);
      banner_->set_record(QString::fromUtf8("90° %1 × %2").arg(mine, partner_name_));
    } else {
      // the a12 double wheel, the partner outside at full scale
      wheel_->set_display_list(build_double_wheel(chart, *partner_chart_, s, aspects, wopt));
      const std::vector<CrossAspectHit> cross = scan_aspects_between(chart, *partner_chart_, aspect_settings_, false);
      cross_text = tr("<span style='color:#D4A94A'>VERGLEICH</span>&nbsp; ");
      cross_text += cross.empty() ? tr("keine") : cross_hits_text(cross);
      banner_->set_record(QString("%1 × %2").arg(mine, partner_name_));
    }
    transit_drawn = true;
  }
  if (!transit_drawn) {
    wheel_->set_display_list(build_wheel(chart, s, aspects, wopt));
  }
  banner_->set_info(QString("JD(UT) %1   ΔT %2 min   ARMC %3°   %4%5")
                        .arg(chart.jd_ut, 0, 'f', 5)
                        .arg(chart.delt_minutes, 0, 'f', 2)
                        .arg(chart.armc_deg, 0, 'f', 4)
                        .arg(s.heliocentric
                                 ? tr("Heliozentrisch")
                                 : QString::fromUtf8(chart.houses.name.data(), static_cast<int>(chart.houses.name.size())))
                        .arg(!s.heliocentric && s.topocentric_parallax ? "   MitParall." : ""));
  fill_tables(*shown, *shown_aspects);
  if (!cross_text.isEmpty()) {
    aspects_label_->setText(cross_text);
  }
}

void MainWindow::fill_tables(const Chart& chart, const AspectResult& aspects) {
  bodies_->setRowCount(0);
  // a chart without a sun but with the moon slot filled is the hrg
  // mode, the slot then carries the earth
  const bool helio = !chart.b[body::kSun].present && chart.b[body::kMoon].present;
  QStringList row_names;
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present) {
      continue;
    }
    const int row = bodies_->rowCount();
    bodies_->insertRow(row);
    std::string_view tag = (helio && slot == body::kMoon) ? body::kTag[0]
                                                           : body::kTag[static_cast<std::size_t>(slot)];
    if (slot == 0) {
      //RR Fixpunkt, sein SP
      tag = "sp";
    }
    row_names << QString::fromUtf8(tag.data(), static_cast<int>(tag.size()));
    if (!b.valid) {
      bodies_->setItem(row, 0, new QTableWidgetItem(tr("außerhalb der Ephemeride")));
      continue;
    }
    const bool angle_slot = slot == body::kAscendant || slot == body::kMc;
    bodies_->setItem(row, 0, new QTableWidgetItem(zodiac(b.el)));
    if (!angle_slot) {
      bodies_->setItem(row, 1, new QTableWidgetItem(degs(b.eb)));
      bodies_->setItem(row, 2, new QTableWidgetItem(degs(b.de)));
      bodies_->setItem(row, 3, new QTableWidgetItem(degs(b.tb)));
      // the sign of the acceleration, direct or retrograde at a station
      bodies_->setItem(row, 4, new QTableWidgetItem(b.ttb < 0.0 ? QString::fromUtf8("−") : "+"));
      if (b.dr > 0.0) {
        bodies_->setItem(row, 5, new QTableWidgetItem(QString::number(b.dr, 'f', 3)));
      }
      //RR die mittleren Planeten-KNOTEN und die PLANETEN-APSIDEN
      const PlanetPoints pts = planet_points(chart, slot, current_settings());
      if (pts.ok) {
        bodies_->setItem(row, 6, new QTableWidgetItem(zodiac(pts.node)));
        bodies_->setItem(row, 7, new QTableWidgetItem(zodiac(pts.node_south)));
        bodies_->setItem(row, 8, new QTableWidgetItem(zodiac(pts.perihelion)));
        bodies_->setItem(row, 9, new QTableWidgetItem(zodiac(pts.aphelion)));
      }
      auto* retro = new QTableWidgetItem(b.tb < 0.0 ? "R" : "");
      retro->setForeground(QColor(0xE8, 0x5D, 0x4E));
      bodies_->setItem(row, 10, retro);
    }
  }
  bodies_->setVerticalHeaderLabels(row_names);
  bodies_->resizeColumnsToContents();
  // bes111 lists no cusps in the hrg mode
  for (int i = 1; i <= 12; ++i) {
    cusps_->setItem(i - 1, 0,
                    new QTableWidgetItem(helio ? QString() : zodiac(chart.houses.cusp[static_cast<std::size_t>(i)])));
  }
  //RR die MONDPHASE ... ist die ekliptikale Längendifferenz MOND-SONNE
  // with his percent figure, full moon one hundred, new moon zero
  QString phase_text;
  if (!helio && chart.b[body::kSun].valid && chart.b[body::kMoon].valid) {
    const double d = norm_rad(chart.b[body::kMoon].el - chart.b[body::kSun].el) * kRadToDeg;
    const double pct = (180.0 - std::abs(d - 180.0)) / 180.0 * kPercent;
    phase_text = tr("<br><span style='color:#D4A94A'>MONDPHASE</span>&nbsp; %1° (%2%)")
                     .arg(d, 0, 'f', 0)
                     .arg(pct, 0, 'f', 0);
  }
  aspects_label_->setText(tr("<span style='color:#D4A94A'>ASPEKTE</span>&nbsp; "
                             "konj %1  opp %2  trigon %3  quadrat %4  sextil %5")
                              .arg(aspects.zh[1])
                              .arg(aspects.zh[2])
                              .arg(aspects.zh[3])
                              .arg(aspects.zh[4])
                              .arg(aspects.zh[6]) +
                          phase_text);
}

void MainWindow::open_place() {
  PlaceDialog dialog(data_dir_ / "places", data_dir_ / "landnima.int", this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const PlaceRecord& r = dialog.chosen();
  const QSignalBlocker b1(lon_);
  const QSignalBlocker b2(lat_);
  const QSignalBlocker b3(zone_);
  lon_->setValue(r.lon);
  lat_->setValue(r.lat);
  // the picker files store the step from zone time to UT, the panel
  // wants hours east
  if (const auto to_ut = r.zone_to_ut()) {
    zone_->setValue(-*to_ut);
  }
  recompute();
}

void MainWindow::apply_moment(double jd_ut, const QString& label) {
  const CalendarDate d = calendar_date(jd_ut, current_settings().calendar);
  int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
  if (seconds >= kSecondsPerDay) {
    seconds = kSecondsPerDay - 1;
  }
  const QSignalBlocker b1(date_);
  const QSignalBlocker b2(time_);
  const QSignalBlocker b3(zone_);
  date_->setDate(QDate(d.year, d.month, d.day));
  time_->setTime(QTime(seconds / 3600, (seconds / 60) % 60, seconds % 60));
  // the found moment is Universal Time
  zone_->setValue(0.0);
  recompute();
  banner_->set_record(label);
}

void MainWindow::solar_chart() {
  bool ok = false;
  const int year = QInputDialog::getInt(this, tr("Solar"), tr("Gewünschtes Kalender-Jahr"),
                                        QDate::currentDate().year(), 1, 3000, 1, &ok);
  if (ok) {
    run_solar(year);
  }
}

void MainWindow::show_solar(int year) {
  run_solar(year);
}

void MainWindow::show_clock() {
  clock_action_->setChecked(true);
}

// ported from the SOLAR and LUNAR list outputs, up to 84 return dates
// in one table, the lunar numbers serve the Troinsky tertiaries
void MainWindow::return_list(bool lunar) {
  if (!last_chart_ || !last_chart_->b[body::kSun].valid) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(lunar ? tr("Lunar-Liste") : tr("Solar-Liste"));
  auto* v = new QVBoxLayout(&dialog);
  auto* table = new QTableWidget(0, 2, &dialog);
  table->setHorizontalHeaderLabels({tr("Nr"), tr("Datum (UT)")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(18);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  auto* note = new QLabel(tr("Doppelklick übernimmt den Zeitpunkt ins Panel."), &dialog);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  const SearchContext ctx = make_context();
  const double birth = last_chart_->jd_ut;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::vector<std::pair<int, double>> rows;
  //RR bis zu 84 Daten
  for (int n = 1; n <= 84; ++n) {
    LongitudeCrossing hit;
    if (lunar) {
      hit = planetar_return(birth, body::kMoon, last_chart_->b[body::kMoon].el, n, true, ctx);
    } else {
      hit = solar_return(ctx.base.date_ut, last_chart_->b[body::kSun].el, ctx.base.date_ut.year + n, ctx);
    }
    if (hit.ok) {
      rows.push_back({n, hit.jd_ut});
    }
  }
  QApplication::restoreOverrideCursor();
  for (const auto& [n, jd] : rows) {
    const CalendarDate d = calendar_date(jd, current_settings().calendar);
    int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
    if (seconds >= kSecondsPerDay) {
      seconds = kSecondsPerDay - 1;
    }
    const int row = table->rowCount();
    table->insertRow(row);
    table->setItem(row, 0, new QTableWidgetItem(QString::number(n)));
    auto* item = new QTableWidgetItem(QString::asprintf("%02d.%02d.%04d %02d:%02d", d.day, d.month, d.year,
                                                        seconds / 3600, (seconds / 60) % 60));
    item->setData(Qt::UserRole, jd);
    table->setItem(row, 1, item);
  }
  connect(table, &QTableWidget::cellDoubleClicked, &dialog, [this, table, lunar, &dialog](int row, int) {
    const double jd = table->item(row, 1)->data(Qt::UserRole).toDouble();
    apply_moment(jd, (lunar ? tr("LUNAR %1") : tr("SOLAR-NR %1")).arg(table->item(row, 0)->text()));
    dialog.accept();
  });
  table->resizeColumnsToContents();
  v->addWidget(table, 1);
  v->addWidget(note);
  v->addWidget(buttons);
  dialog.resize(380, 640);
  dialog.exec();
}

void MainWindow::run_solar(int year) {
  if (!last_chart_ || !last_chart_->b[body::kSun].valid) {
    return;
  }
  SearchContext ctx;
  ctx.base = current_input();
  ctx.settings = current_settings();
  ctx.vsop = &vsop_;
  ctx.eph = &eph_;
  const LongitudeCrossing hit = solar_return(ctx.base.date_ut, last_chart_->b[body::kSun].el, year, ctx);
  if (!hit.ok) {
    banner_->set_record(tr("Kein Solar gefunden"));
    return;
  }
  apply_moment(hit.jd_ut, QString("SOLAR %1").arg(year));
}

void MainWindow::fixed_star_table() {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Fixsterne"));
  auto* v = new QVBoxLayout(&dialog);
  const std::vector<StarRow> rows = fixed_stars(*last_chart_, aspect_settings_.orb);
  auto* table = new QTableWidget(static_cast<int>(rows.size()), 8, &dialog);
  table->setHorizontalHeaderLabels({tr("Stern"), tr("Ekl.Länge"), tr("Aspekte"), tr("Qualität"),
                                    tr("Astron. Name"), tr("Breite"), "Rekt./Dekl.", "D/LJ"});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
    const StarRow& r = rows[static_cast<std::size_t>(i)];
    table->setItem(i, 0, new QTableWidgetItem(QString::fromUtf8(r.name.data(), static_cast<int>(r.name.size()))));
    table->setItem(i, 1, new QTableWidgetItem(zodiac(r.la)));
    QString asp;
    for (const auto& [slot, kind] : r.aspects) {
      if (!asp.isEmpty()) {
        asp += "  ";
      }
      asp += QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                               static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size())) +
             " " + QChar(kind);
    }
    auto* aspects = new QTableWidgetItem(asp);
    if (!asp.isEmpty()) {
      aspects->setForeground(QColor(0xD4, 0xA9, 0x4A));
    }
    table->setItem(i, 2, aspects);
    table->setItem(i, 3, new QTableWidgetItem(QString::fromUtf8(r.quality.data(), static_cast<int>(r.quality.size()))));
    table->setItem(i, 4, new QTableWidgetItem(QString::fromUtf8(r.astro.data(), static_cast<int>(r.astro.size()))));
    table->setItem(i, 5, new QTableWidgetItem(QString::asprintf("%+7.2f°", r.br * kRadToDeg)));
    table->setItem(i, 6, new QTableWidgetItem(QString::asprintf("%7.2f° / %+7.2f°", r.ar * kRadToDeg, r.de * kRadToDeg)));
    //RR NN
    table->setItem(i, 7, new QTableWidgetItem(r.lightyears > 0 ? QString::number(r.lightyears) : "NN"));
  }
  table->resizeColumnsToContents();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addWidget(table, 1);
  v->addWidget(buttons);
  dialog.resize(940, 640);
  dialog.exec();
}

void MainWindow::arabic_table() {
  if (!last_chart_ || !last_chart_->houses.ok) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Arabische Teile (Sensitive Punkte)"));
  auto* v = new QVBoxLayout(&dialog);
  auto* top = new QHBoxLayout();
  auto* mode = new QComboBox(&dialog);
  //RR TRADITIONELLE FORMEL / TAG / NACHT
  mode->addItem(tr("Traditionell"), 1);
  mode->addItem(tr("Immer als Tag-Geburt"), 2);
  mode->addItem(tr("Immer als Nacht-Geburt"), 3);
  top->addWidget(new QLabel(tr("Formel"), this));
  top->addWidget(mode);
  top->addStretch(1);
  auto* table = new QTableWidget(0, 4, &dialog);
  table->setHorizontalHeaderLabels({tr("Punkt"), tr("Länge"), tr("Formel"), tr("Bemerkung")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  const Chart chart = *last_chart_;
  const std::filesystem::path own = data_dir_;
  const auto fill = [table, chart, own, mode]() {
    const std::vector<ArabicPart> parts =
        arabic_parts(chart, static_cast<ArabicFormula>(mode->currentData().toInt()), own);
    table->setRowCount(0);
    for (const ArabicPart& p : parts) {
      const int row = table->rowCount();
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(QString::fromUtf8(p.name.c_str())));
      table->setItem(row, 1, new QTableWidgetItem(zodiac(p.la)));
      table->setItem(row, 2, new QTableWidgetItem(QString::fromUtf8(p.formula.c_str())));
      table->setItem(row, 3, new QTableWidgetItem(QString::fromUtf8(p.remark.c_str())));
    }
    table->resizeColumnsToContents();
  };
  connect(mode, &QComboBox::currentIndexChanged, &dialog, fill);
  fill();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(top);
  v->addWidget(table, 1);
  v->addWidget(buttons);
  dialog.resize(680, 640);
  dialog.exec();
}

void MainWindow::midpoint_tree() {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  //RR HALBSUMMEN
  dialog.setWindowTitle(tr("Halbsummen-Bäume"));
  auto* v = new QVBoxLayout(&dialog);
  const Chart& chart = *last_chart_;
  const MidpointResult mid = scan_midpoints(chart, current_settings(), aspect_settings_, true);
  auto* table = new QTableWidget(0, 4, &dialog);
  //RR 45°-Dial, die Sortierung über 8 mal die Länge
  table->setHorizontalHeaderLabels({tr("Punkt"), tr("Länge"), QString::fromUtf8("45°-Dial"), tr("Halbsummen auf dem Punkt")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  static constexpr const char* kLevel[9] = {"", "360", "180", "", "90", "", "", "", "45"};
  for (int slot = 1; slot < body::kSlotCount; ++slot) {
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid || slot == body::kNodeDesc) {
      continue;
    }
    const int row = table->rowCount();
    table->insertRow(row);
    table->setItem(row, 0, new QTableWidgetItem(QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                                                                  static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size()))));
    table->setItem(row, 1, new QTableWidgetItem(zodiac(b.el)));
    // his sort key, eight times the longitude folded into the circle
    const double dial = norm_rad(8.0 * b.el) * kRadToDeg;
    auto* dial_item = new QTableWidgetItem(QString::asprintf("%7.2f°", dial));
    // close company on the eight fold circle glows like his red rows
    for (int o = 1; o < body::kSlotCount; ++o) {
      const BodyState& ob = chart.b[static_cast<std::size_t>(o)];
      if (o == slot || !ob.present || !ob.valid || o == body::kNodeDesc) {
        continue;
      }
      if ((slot == 11 && o == 12) || (slot == 12 && o == 11)) {
        continue;
      }
      double w1 = norm_rad(8.0 * b.el);
      double w2 = norm_rad(8.0 * ob.el);
      vergl1(w1, w2);
      const double gate = std::max(org(aspect_settings_, slot, 8), org(aspect_settings_, o, 8));
      if (std::abs(w1 - w2) < gate && std::abs(w1 - w2) > 0.0) {
        dial_item->setForeground(QColor(0xE8, 0x5D, 0x4E));
        break;
      }
    }
    table->setItem(row, 2, dial_item);
    QString contacts;
    for (const MidpointHit& h : mid.hits) {
      if (h.t != slot) {
        continue;
      }
      if (!contacts.isEmpty()) {
        contacts += ",  ";
      }
      contacts += QString("%1/%2 (%3°)")
                      .arg(QString::fromUtf8(body::kTag[static_cast<std::size_t>(h.u)].data(),
                                             static_cast<int>(body::kTag[static_cast<std::size_t>(h.u)].size())),
                           QString::fromUtf8(body::kTag[static_cast<std::size_t>(h.w)].data(),
                                             static_cast<int>(body::kTag[static_cast<std::size_t>(h.w)].size())),
                           QString(kLevel[h.nh]));
    }
    table->setItem(row, 3, new QTableWidgetItem(contacts));
  }
  table->resizeColumnsToContents();
  auto* counts = new QLabel(tr("Direkt %1,  Quadrat %2,  Halbquadrat %3")
                                .arg(mid.direct)
                                .arg(mid.square)
                                .arg(mid.semi),
                            &dialog);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addWidget(table, 1);
  v->addWidget(counts);
  v->addWidget(buttons);
  dialog.resize(820, 640);
  dialog.exec();
}

void MainWindow::correction() {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  //RR Womit Korrigieren ?
  dialog.setWindowTitle(tr("Korrektur der Geburtszeit"));
  auto* v = new QVBoxLayout(&dialog);
  auto* form = new QFormLayout();
  auto* what = new QComboBox(&dialog);
  what->addItem(tr("Sternzeit (h)"), 1);
  what->addItem("MC", 2);
  what->addItem("AC", 3);
  what->addItem(tr("Sonne"), 4);
  what->addItem(tr("Mond"), 5);
  //RR mit ZWISCHENHÄUSERN ... korrigieren
  what->addItem(tr("Zwischenhaus"), 6);
  auto* cusp_nr = new QSpinBox(&dialog);
  cusp_nr->setRange(2, 12);
  cusp_nr->setPrefix(tr("Haus "));
  cusp_nr->setEnabled(false);
  connect(what, &QComboBox::currentIndexChanged, &dialog,
          [what, cusp_nr](int) { cusp_nr->setEnabled(what->currentData().toInt() == 6); });
  auto* target = new QDoubleSpinBox(&dialog);
  target->setRange(0.0, 360.0);
  target->setDecimals(4);
  form->addRow(tr("Korrigieren mit"), what);
  form->addRow(QString(), cusp_nr);
  form->addRow(tr("Soll-Wert"), target);
  auto* note = new QLabel(tr("Die Uhrzeit des Panels wird so verschoben, dass die gewählte Größe den Soll-Wert erreicht. Sternzeit in Stunden, alles andere in Grad."), &dialog);
  note->setWordWrap(true);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(note);
  v->addWidget(buttons);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const int mode = what->currentData().toInt();
  const Chart& radix = *last_chart_;
  const SearchContext ctx = make_context();
  double jd = radix.jd_ut;
  bool found = false;
  if (mode == 1 || mode == 2) {
    // sidereal time and midheaven turn into a clock directly
    double hs = target->value();
    if (mode == 2) {
      const double mc = target->value() * kDegToRad;
      const double z = std::sin(mc) * std::cos(radix.ekls0);
      hs = norm_hours(atn(z, std::cos(mc)) * kRadToDeg / kDegPerHour - lon_->value() / kDegPerHour);
    }
    const double d0 = std::floor(radix.jd_ut + 0.5) - 0.5;
    double hd = 0.0;
    for (int i = 0; i < 8; ++i) {
      const double h0 = gmst0_hours(d0);
      const double next = norm_hours(hs - h0) / kSolarToSiderealRate;
      if (std::abs(next - hd) < 1.0e-9) {
        hd = next;
        break;
      }
      hd = next;
    }
    jd = d0 + hd / 24.0;
    found = true;
  } else if (mode == 3 || mode == 6) {
    // the ascendant and the cusps need the damped walk on the daily turn
    const double pz = target->value() * kDegToRad;
    const int h = cusp_nr->value();
    for (int i = 0; i < 200 && !found; ++i) {
      ChartInput in = ctx.base;
      in.date_ut = calendar_date(jd, ctx.settings.calendar);
      const Chart c = compute_chart(in, ctx.settings, vsop_, eph_);
      if (!c.ok || !c.houses.ok) {
        break;
      }
      const double have = mode == 3 ? c.houses.angles.ac : c.houses.cusp[static_cast<std::size_t>(h)];
      double d = norm_rad(pz - have);
      if (d > kPi) {
        d -= kTwoPi;
      }
      if (std::abs(d) < 1.0e-8) {
        found = true;
        break;
      }
      jd += 0.5 * d / kTwoPi / kSolarToSiderealRate;
    }
  } else {
    // sun or moon walk to the wanted longitude near the birth
    const int slot = mode == 4 ? body::kSun : body::kMoon;
    const double pz = target->value() * kDegToRad;
    //RR jd-Startwert
    const LongitudeCrossing hit = find_longitude_backward(radix.jd_ut + 32.0, slot, pz, ctx);
    if (hit.ok) {
      jd = hit.jd_ut;
      found = true;
    }
  }
  if (!found) {
    banner_->set_record(tr("Korrektur nicht gefunden"));
    return;
  }
  apply_moment(jd, tr("KORRIGIERT"));
}

void MainWindow::chain_files() {
  //RR ZU VERKETTENDE DATEN-DATEIEN NACHEINANDER AUFRUFEN !
  const QStringList sources = QFileDialog::getOpenFileNames(
      this, tr("Zu verkettende Dateien wählen"), QString(),
      tr("HORCOM Daten (*.DAT *.dat *.AAF *.aaf)"));
  if (sources.isEmpty()) {
    return;
  }
  const QString target = QFileDialog::getSaveFileName(this, tr("Ketten-Datei"), "kette.aaf", tr("AAF (*.aaf *.AAF)"));
  if (target.isEmpty()) {
    return;
  }
  std::vector<AafRecord> all;
  for (const QString& src : sources) {
    const std::filesystem::path p(src.toStdWString());
    if (src.endsWith(".dat", Qt::CaseInsensitive)) {
      if (const auto records = read_chart_file(p)) {
        for (const ChartRecord& r : *records) {
          all.push_back(aaf_from_chart_record(r));
        }
      }
    } else if (const auto records = read_aaf(p)) {
      for (const AafRecord& r : *records) {
        all.push_back(r);
      }
    }
  }
  if (all.empty() || !write_aaf(std::filesystem::path(target.toStdWString()), all)) {
    QMessageBox::warning(this, "HORCOM", tr("Die Ketten-Datei ließ sich nicht schreiben."));
    return;
  }
  QMessageBox::information(this, "HORCOM", tr("%1 Datensätze verkettet.").arg(all.size()));
}

// ported from AAF-DATEI <> HORCOM-DATEI, the exchange records pressed
// into the fixed 128 byte layout
void MainWindow::aaf_to_dat() {
  const QString src = QFileDialog::getOpenFileName(this, tr("AAF-Datei wählen"), QString(), tr("AAF (*.aaf *.AAF)"));
  if (src.isEmpty()) {
    return;
  }
  const auto records = read_aaf(std::filesystem::path(src.toStdWString()));
  if (!records || records->empty()) {
    QMessageBox::warning(this, "HORCOM", tr("Die Datei enthält keine lesbaren Datensätze."));
    return;
  }
  const QString target = QFileDialog::getSaveFileName(this, tr("HORCOM-Datei"),
                                                      QFileInfo(src).completeBaseName() + ".dat",
                                                      tr("HORCOM Daten (*.dat *.DAT)"));
  if (target.isEmpty()) {
    return;
  }
  std::vector<ChartRecord> out;
  for (const AafRecord& r : *records) {
    const ChartInput in = record_input(r);
    ChartRecord c;
    c.day = in.date_ut.day;
    c.month = in.date_ut.month;
    c.year = in.date_ut.year;
    c.hour = in.date_ut.hour;
    c.minute = in.date_ut.minute;
    c.lon = in.lon_deg_east;
    c.lat = in.lat_deg;
    c.name = r.surname.empty() ? r.given : r.surname + " " + r.given;
    c.place = r.place;
    //RR die Zeichenfolge "(JULIAN.)" wird unter BEMERKG. gespeichert
    c.remark = r.calendar == Calendar::kJulian ? "(JULIAN.) " + r.comment : r.comment;
    out.push_back(std::move(c));
  }
  if (!write_chart_file(std::filesystem::path(target.toStdWString()), out)) {
    QMessageBox::warning(this, "HORCOM", tr("Die HORCOM-Datei ließ sich nicht schreiben."));
    return;
  }
  QMessageBox::information(this, "HORCOM", tr("%1 Datensätze umgewandelt.").arg(out.size()));
}

// ported from Datei TRIMMEN and Datei MINIMIEREN
void MainWindow::tidy_file() {
  const QString src = QFileDialog::getOpenFileName(this, tr("Daten-Datei wählen"), QString(), tr("HORCOM Daten (*.dat *.DAT)"));
  if (src.isEmpty()) {
    return;
  }
  const std::filesystem::path p(src.toStdWString());
  auto records = read_chart_file(p);
  if (!records) {
    QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht lesen."));
    return;
  }
  const std::size_t before = records->size();
  trim_records(*records);
  minimize_records(*records);
  if (records->size() == before) {
    QMessageBox::information(this, "HORCOM", tr("Nichts zu bereinigen, %1 Datensätze.").arg(before));
    return;
  }
  if (QMessageBox::question(this, "HORCOM",
                            tr("%1 von %2 Datensätzen bleiben. Datei überschreiben?").arg(records->size()).arg(before)) !=
      QMessageBox::Yes) {
    return;
  }
  if (!write_chart_file(p, *records)) {
    QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht schreiben."));
  }
}

// ported from AUSWERTEFÄHIGE DATEI ERSTELLEN, every record of a
// collection computed and packed into the STATIST7 store
void MainWindow::create_statistics() {
  const QString src = QFileDialog::getOpenFileName(
      this, tr("Daten-Datei für die Statistik wählen"), QString(),
      tr("HORCOM Daten (*.DAT *.dat *.AAF *.aaf)"));
  if (src.isEmpty()) {
    return;
  }
  std::vector<AafRecord> all;
  const std::filesystem::path p(src.toStdWString());
  if (src.endsWith(".dat", Qt::CaseInsensitive)) {
    if (const auto records = read_chart_file(p)) {
      for (const ChartRecord& r : *records) {
        all.push_back(aaf_from_chart_record(r));
      }
    }
  } else if (const auto records = read_aaf(p)) {
    all = *records;
  }
  if (all.empty()) {
    QMessageBox::warning(this, "HORCOM", tr("Die Datei enthält keine lesbaren Datensätze."));
    return;
  }
  QString base = QFileInfo(src).completeBaseName() + ".sta";
  const QString target = QFileDialog::getSaveFileName(this, tr("Statistik-Datei"), base, tr("Statistik (*.sta *.STA)"));
  if (target.isEmpty()) {
    return;
  }
  const ChartSettings s = current_settings();
  StatSet set;
  set.params.haw = static_cast<int>(s.houses);
  set.params.haus = houses_->currentText().toStdString();
  set.params.appa = static_cast<int>(s.apparent);
  set.params.apogw = s.true_apogee;
  set.params.moknw = s.true_node;
  set.params.par = s.topocentric_parallax ? 1.0 : 0.0;
  set.params.nk = s.nk;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  for (const AafRecord& r : all) {
    const ChartInput in = record_input(r);
    const Chart c = compute_chart(in, s, vsop_, eph_);
    if (!c.ok) {
      continue;
    }
    StatRecord rec;
    rec.name = r.surname.empty() ? r.given : r.surname + " " + r.given;
    rec.place = r.place;
    rec.day = in.date_ut.day;
    rec.month = in.date_ut.month;
    rec.year = in.date_ut.year;
    rec.hour = static_cast<int>(in.date_ut.hour);
    rec.minute = in.date_ut.minute;
    rec.lon = in.lon_deg_east;
    rec.lat = in.lat_deg;
    for (int slot = 0; slot < body::kSlotCount; ++slot) {
      const BodyState& b = c.b[static_cast<std::size_t>(slot)];
      if (b.present && b.valid) {
        rec.el[static_cast<std::size_t>(slot)] = b.el;
      }
    }
    if (c.houses.ok) {
      rec.ac = c.houses.cusp[1];
      rec.mc = c.houses.cusp[10];
      rec.h2 = c.houses.cusp[2];
      rec.h3 = c.houses.cusp[3];
      rec.h5 = c.houses.cusp[5];
      rec.h6 = c.houses.cusp[6];
    }
    set.records.push_back(std::move(rec));
  }
  QApplication::restoreOverrideCursor();
  if (set.records.empty() || !save_statistics(std::filesystem::path(target.toStdWString()), set)) {
    QMessageBox::warning(this, "HORCOM", tr("Die Statistik-Datei ließ sich nicht schreiben."));
    return;
  }
  QMessageBox::information(this, "HORCOM",
                           tr("%1 von %2 Datensätzen berechnet und gespeichert.").arg(set.records.size()).arg(all.size()));
}

void MainWindow::orb_settings() {
  QDialog dialog(this);
  //RR VORGABEN
  dialog.setWindowTitle(tr("Vorgaben, Orbes und Fixpunkt"));
  auto* v = new QVBoxLayout(&dialog);
  auto* form = new QFormLayout();
  auto* orb = new QDoubleSpinBox(&dialog);
  orb->setRange(0.1, 2.0);
  orb->setDecimals(3);
  orb->setSingleStep(0.125);
  orb->setValue(aspect_settings_.orb);
  auto* divisors = new QSpinBox(&dialog);
  divisors->setRange(1, 16);
  divisors->setValue(aspect_settings_.divisors);
  auto* equal = new QCheckBox(tr("Orbes gleicher Wahrscheinlichkeit"), &dialog);
  equal->setChecked(aspect_settings_.equal_probability);
  auto* fix_on = new QCheckBox(tr("Fixpunkt verwenden"), &dialog);
  fix_on->setChecked(fixpunkt_ >= 0.0);
  auto* fix_deg = new QDoubleSpinBox(&dialog);
  fix_deg->setRange(0.0, 360.0);
  fix_deg->setDecimals(4);
  fix_deg->setValue(fixpunkt_ >= 0.0 ? fixpunkt_ * kRadToDeg : 0.0);
  //RR bei den LÄNGEN kann zwischen WAHREN und APPARENTEN Werten gewählt
  //RR werden, App.1 nur Licht-Laufzeit, App.2 mit jährlicher Aberration
  auto* appa = new QComboBox(&dialog);
  appa->addItem(tr("Apparent 1 (Licht-Laufzeit)"), 1);
  appa->addItem(tr("Apparent 2 (mit Aberration)"), 2);
  appa->addItem(tr("Wahre (geometrische)"), 3);
  appa->setCurrentIndex(std::clamp(konsta_.appa, 1, 3) - 1);
  form->addRow(tr("Längen-Modus"), appa);
  form->addRow(tr("Orbis-Faktor"), orb);
  form->addRow(tr("Maximaler Teiler"), divisors);
  form->addRow(equal);
  form->addRow(fix_on);
  form->addRow(tr("Fixpunkt (Ekliptik-Grad)"), fix_deg);
  // the per body orb weights of his table, zero silences a body
  auto* weights = new QTableWidget(1, 14, &dialog);
  QStringList heads;
  for (int slot = 1; slot <= 14; ++slot) {
    heads << QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                               static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size()));
    auto* item = new QTableWidgetItem(QString::number(aspect_settings_.weight[static_cast<std::size_t>(slot)]));
    weights->setItem(0, slot - 1, item);
  }
  weights->setHorizontalHeaderLabels(heads);
  weights->verticalHeader()->setVisible(false);
  weights->setFixedHeight(64);
  auto* wlabel = new QLabel(tr("Planeten-Gewichte in Prozent, 0 schaltet einen Punkt stumm."), &dialog);
  wlabel->setWordWrap(true);
  auto* save = new QCheckBox(tr("Als konsta.int neben den Daten speichern"), &dialog);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(weights);
  v->addWidget(wlabel);
  v->addWidget(save);
  v->addWidget(buttons);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  aspect_settings_.orb = orb->value();
  aspect_settings_.divisors = divisors->value();
  aspect_settings_.equal_probability = equal->isChecked();
  if (aspect_settings_.equal_probability) {
    aspect_settings_.preset_equal_orbs();
  }
  for (int slot = 1; slot <= 14; ++slot) {
    bool ok = false;
    const int w = weights->item(0, slot - 1)->text().toInt(&ok);
    if (ok && w >= 0 && w <= 200) {
      aspect_settings_.weight[static_cast<std::size_t>(slot)] = w;
    }
  }
  fixpunkt_ = fix_on->isChecked() ? fix_deg->value() * kDegToRad : -1.0;
  konsta_.appa = appa->currentData().toInt();
  konsta_.appa_name = konsta_.appa == 1 ? "App.1" : (konsta_.appa == 2 ? "App.2" : "Wahre");
  if (save->isChecked()) {
    konsta_.orb = aspect_settings_.orb;
    konsta_.nasp = aspect_settings_.divisors;
    konsta_.orbe_on = aspect_settings_.equal_probability;
    for (int slot = 0; slot < body::kSlotCount; ++slot) {
      konsta_.or_weight[static_cast<std::size_t>(slot)] = aspect_settings_.weight[static_cast<std::size_t>(slot)];
    }
    konsta_.fixpunkt = fixpunkt_ >= 0.0 ? 1 : 2;
    konsta_.fixpunkt_name = fixpunkt_ >= 0.0 ? std::to_string(fixpunkt_ * kRadToDeg) : std::string();
    if (!save_konsta(data_dir_ / "konsta.int", konsta_)) {
      QMessageBox::warning(this, "HORCOM", tr("Die Vorgaben ließen sich nicht speichern."));
    }
  }
  recompute();
}

void MainWindow::degree_list() {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  //RR GRAD-LISTE
  dialog.setWindowTitle(tr("Grad-Liste"));
  auto* v = new QVBoxLayout(&dialog);
  auto* with_houses = new QCheckBox(tr("Zwischen-Häuser hinzunehmen"), &dialog);
  auto* table = new QTableWidget(0, 3, &dialog);
  table->setHorizontalHeaderLabels({tr("Grad"), tr("Punkt"), tr("Zeichen")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  const Chart chart = *last_chart_;
  const auto fill = [table, chart, with_houses]() {
    std::vector<std::pair<double, QString>> rows;
    for (int slot = 0; slot < body::kSlotCount; ++slot) {
      const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
      if (b.present && b.valid && b.el > kEps) {
        rows.emplace_back(b.el, QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                                                  static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size())));
      }
    }
    if (with_houses->isChecked()) {
      for (int h = 1; h <= 12; ++h) {
        if (chart.houses.cusp[static_cast<std::size_t>(h)] > 0.0) {
          rows.emplace_back(chart.houses.cusp[static_cast<std::size_t>(h)], QString("H%1").arg(h));
        }
      }
    }
    std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    table->setRowCount(0);
    for (const auto& [el, name] : rows) {
      const int row = table->rowCount();
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(QString::asprintf("%8.2f", el * kRadToDeg)));
      table->setItem(row, 1, new QTableWidgetItem(name));
      table->setItem(row, 2, new QTableWidgetItem(zodiac(el)));
    }
  };
  connect(with_houses, &QCheckBox::toggled, &dialog, fill);
  fill();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addWidget(with_houses);
  v->addWidget(table, 1);
  v->addWidget(buttons);
  dialog.resize(380, 560);
  dialog.exec();
}

void MainWindow::house_table() {
  if (!last_chart_ || !last_chart_->houses.ok) {
    return;
  }
  const Chart& chart = *last_chart_;
  QDialog dialog(this);
  //RR Häuserspitzen nach System, In wahrer ekliptikaler Länge u. AR
  dialog.setWindowTitle(tr("Häuser-Tabelle (%1)").arg(QString::fromUtf8(chart.houses.name.data(),
                                                                        static_cast<int>(chart.houses.name.size()))));
  auto* v = new QVBoxLayout(&dialog);
  const bool placidus = current_settings().houses == HouseSystem::kPlacidus;
  auto* table = new QTableWidget(13, placidus ? 6 : 4, &dialog);
  QStringList heads{tr("Haus"), tr("Länge"), "AR", tr("Dekl.")};
  if (placidus) {
    //RR AO und Polhöhe
    heads << "AO" << tr("Polhöhe");
  }
  table->setHorizontalHeaderLabels(heads);
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  const double armcb = chart.armc_deg * kDegToRad;
  for (int t = 1; t <= 13; ++t) {
    const bool vertex = t == 13;
    const double pa = vertex ? chart.houses.angles.vertex : chart.houses.cusp[static_cast<std::size_t>(t)];
    QString name = QString("H%1").arg(t);
    if (t == 1) {
      name = "AC";
    } else if (t == 10) {
      name = "MC";
    } else if (vertex) {
      name = "VERTEX";
    }
    table->setItem(t - 1, 0, new QTableWidgetItem(name));
    table->setItem(t - 1, 1, new QTableWidgetItem(zodiac(pa) + QString::asprintf("  = %8.3f°", pa * kRadToDeg)));
    const Equatorial eq = ecliptic_to_equatorial(pa, 0.0, chart.ekls0);
    table->setItem(t - 1, 2, new QTableWidgetItem(QString::asprintf("%8.3f°", eq.ra * kRadToDeg)));
    table->setItem(t - 1, 3, new QTableWidgetItem(QString::asprintf("%8.3f°", eq.dec * kRadToDeg)));
    if (placidus && !vertex) {
      // under Placidus every cusp owns an oblique ascension and the
      // pole that raises it there
      const double aoe = norm_rad(armcb + kPi / 2.0 + (t - 1) * kPi / 6.0);
      const double ph = std::atan(std::sin(eq.ra - aoe) / std::tan(eq.dec + kEps));
      table->setItem(t - 1, 4, new QTableWidgetItem(QString::asprintf("%8.3f°", aoe * kRadToDeg)));
      table->setItem(t - 1, 5, new QTableWidgetItem(QString::asprintf("%8.3f°", ph * kRadToDeg)));
    }
  }
  table->resizeColumnsToContents();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addWidget(table, 1);
  v->addWidget(buttons);
  dialog.resize(placidus ? 720 : 540, 440);
  dialog.exec();
}

void MainWindow::rhythm_table() {
  if (!last_chart_ || !last_chart_->houses.ok) {
    return;
  }
  QDialog dialog(this);
  //RR nach W.Döbereiner
  dialog.setWindowTitle(tr("Münchner Rhythmenlehre, Auslösungen nach W. Döbereiner"));
  auto* v = new QVBoxLayout(&dialog);
  auto* top = new QHBoxLayout();
  auto* phase = new QDoubleSpinBox(&dialog);
  phase->setRange(-30.0, 30.0);
  phase->setDecimals(1);
  phase->setValue(7.0);
  phase->setPrefix(tr("Phase "));
  auto* unit = new QComboBox(&dialog);
  unit->addItem(tr("Jahr"), 0);
  unit->addItem(tr("Monat"), 1);
  auto* begin = new QSpinBox(&dialog);
  begin->setRange(1, 12);
  begin->setPrefix(tr("ab Haus "));
  auto* direction = new QComboBox(&dialog);
  direction->addItem(tr("Links"), 1);
  direction->addItem(tr("Rechts"), 0);
  auto* sextile = new QCheckBox(tr("Sextil"), &dialog);
  //RR SONDERPUNKT ( FIXPUNKT ) WÄHLEN, als Grad oder INDIREKT über DATUM
  auto* sp_mode = new QComboBox(&dialog);
  sp_mode->addItem(tr("Kein Sonderpunkt"), 0);
  sp_mode->addItem(tr("Sonderpunkt Grad"), 1);
  sp_mode->addItem(tr("Sonderpunkt Datum"), 2);
  auto* sp_deg = new QDoubleSpinBox(&dialog);
  sp_deg->setRange(0.0, 360.0);
  sp_deg->setDecimals(2);
  sp_deg->setSuffix(QString::fromUtf8("°"));
  auto* sp_date = new QDateEdit(QDate::currentDate(), &dialog);
  sp_date->setCalendarPopup(true);
  sp_date->setDisplayFormat("dd.MM.yyyy");
  auto* run = new QPushButton(tr("Rechnen"), &dialog);
  top->addWidget(phase);
  top->addWidget(unit);
  top->addWidget(begin);
  top->addWidget(direction);
  top->addWidget(sextile);
  top->addWidget(sp_mode);
  top->addWidget(sp_deg);
  top->addWidget(sp_date);
  top->addWidget(run, 1);
  auto* table = new QTableWidget(0, 6, &dialog);
  table->setHorizontalHeaderLabels({tr("Phase"), tr("Haus"), tr("Alter"), tr("Punkt"), tr("Art"), tr("Quelle")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  auto* count = new QLabel(&dialog);
  const Chart chart = *last_chart_;
  const ChartSettings cs = current_settings();
  const AspectSettings base = aspect_settings_;
  const auto fill = [this, table, count, chart, cs, base, phase, unit, begin, direction, sextile, sp_mode, sp_deg,
                     sp_date]() {
    RhythmOptions opt;
    opt.phase_years = phase->value();
    opt.months = unit->currentData().toInt() == 1;
    opt.begin_house = begin->value();
    opt.leftward = direction->currentData().toInt() == 1;
    opt.sextile = sextile->isChecked();
    opt.apogee_opposite = true_apogee_ != nullptr && extras_ != nullptr && extras_->isChecked();
    if (sp_mode->currentData().toInt() == 1) {
      opt.special = sp_deg->value() * kDegToRad;
    } else if (sp_mode->currentData().toInt() == 2) {
      //RR INDIREKT über DATUM, das Alter des Datums auf den Grad gelegt
      const QDate d = sp_date->date();
      const double jd = julian_day({d.day(), d.month(), d.year(), 12, 0.0}, cs.calendar);
      const double years = (jd - chart.jd_ut) / chart.ta.tropical_year_days;
      opt.special = degree_at_age(chart, opt, years);
    }
    AspectSettings a = base;
    //RR nasp bei der Rhythmenlehre 4, mit Sextil 6
    a.divisors = opt.sextile ? 6 : 4;
    const AspectResult scan = scan_aspects(chart, cs, a);
    const std::vector<RhythmTrigger> rows = rhythm_triggers(chart, scan, a, opt);
    static constexpr const char* kKind[6] = {"D", "P", "P2", "P3", "A", "S"};
    table->setRowCount(0);
    for (const RhythmTrigger& t : rows) {
      const int row = table->rowCount();
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(QString::number(t.phase)));
      table->setItem(row, 1, new QTableWidgetItem(QString("H%1").arg(t.house)));
      table->setItem(row, 2, new QTableWidgetItem(QString::asprintf("%8.3f", t.value)));
      //RR der Sonderpunkt als rotes F
      auto* point = new QTableWidgetItem(
          t.slot == 0 ? QStringLiteral("F")
                      : QString::fromUtf8(body::kTag[static_cast<std::size_t>(t.slot)].data(),
                                          static_cast<int>(body::kTag[static_cast<std::size_t>(t.slot)].size())));
      if (t.slot == 0) {
        point->setForeground(QColor(0xE8, 0x5D, 0x4E));
      }
      table->setItem(row, 3, point);
      QString art = kKind[static_cast<int>(t.kind)];
      if (t.kind == RhythmKind::kAspect) {
        art += QString::asprintf(" %g°", t.angle_deg);
      }
      table->setItem(row, 4, new QTableWidgetItem(art));
      table->setItem(row, 5,
                     new QTableWidgetItem(t.source > 0
                                              ? QString::fromUtf8(body::kTag[static_cast<std::size_t>(t.source)].data(),
                                                                  static_cast<int>(body::kTag[static_cast<std::size_t>(t.source)].size()))
                                              : QString()));
    }
    count->setText(tr("%1 Auslösungen").arg(rows.size()));
  };
  connect(run, &QPushButton::clicked, &dialog, fill);
  fill();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(top);
  v->addWidget(table, 1);
  v->addWidget(count);
  v->addWidget(buttons);
  dialog.resize(680, 640);
  dialog.exec();
}

// ported from the right mouse selection of the chart screen, single
// planets red or alone, the ruler highlight and the aspect line choice
void MainWindow::planet_selection() {
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Planeten-Auswahl"));
  auto* v = new QVBoxLayout(&dialog);
  auto* table = new QTableWidget(0, 3, &dialog);
  table->setHorizontalHeaderLabels({tr("Punkt"), tr("Zeigen"), tr("Rot")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    if (slot == body::kAscendant || slot == body::kMc || (slot >= 15 && slot <= 18)) {
      continue;
    }
    if (!last_chart_ || !last_chart_->b[static_cast<std::size_t>(slot)].present) {
      continue;
    }
    const int row = table->rowCount();
    table->insertRow(row);
    auto* name = new QTableWidgetItem(QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                                                        static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size())));
    name->setData(Qt::UserRole, slot);
    name->setFlags(Qt::ItemIsEnabled);
    table->setItem(row, 0, name);
    auto* shown = new QTableWidgetItem();
    shown->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    shown->setCheckState(emphasis_[static_cast<std::size_t>(slot)] < 0 ? Qt::Unchecked : Qt::Checked);
    table->setItem(row, 1, shown);
    auto* red = new QTableWidgetItem();
    red->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    red->setCheckState(emphasis_[static_cast<std::size_t>(slot)] > 0 ? Qt::Checked : Qt::Unchecked);
    table->setItem(row, 2, red);
  }
  auto* ruler = new QCheckBox(tr("Geburtsherrscher rot hervorheben"), &dialog);
  ruler->setChecked(ruler_red_);
  //RR die ASPEKT-LINIEN bis zur 12. Teilung einzeln vorgeben
  auto* chords_box = new QHBoxLayout();
  chords_box->addWidget(new QLabel(tr("Aspekt-Linien Teiler"), &dialog));
  std::array<QCheckBox*, 17> chord_checks{};
  WheelOptions defaults;
  for (int n = 1; n <= 12; ++n) {
    auto* c = new QCheckBox(QString::number(n), &dialog);
    c->setChecked(chords_set_ ? chords_[static_cast<std::size_t>(n)] : defaults.chord_divisor[static_cast<std::size_t>(n)]);
    chord_checks[static_cast<std::size_t>(n)] = c;
    chords_box->addWidget(c);
  }
  chords_box->addStretch(1);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  auto* reset = buttons->addButton(tr("Zurücksetzen"), QDialogButtonBox::ResetRole);
  connect(reset, &QPushButton::clicked, &dialog, [this, &dialog]() {
    emphasis_.fill(0);
    ruler_red_ = false;
    chords_set_ = false;
    dialog.reject();
    recompute();
  });
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addWidget(table, 1);
  v->addWidget(ruler);
  v->addLayout(chords_box);
  v->addWidget(buttons);
  dialog.resize(420, 620);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  for (int row = 0; row < table->rowCount(); ++row) {
    const int slot = table->item(row, 0)->data(Qt::UserRole).toInt();
    int e = 0;
    if (table->item(row, 1)->checkState() == Qt::Unchecked) {
      e = -1;
    } else if (table->item(row, 2)->checkState() == Qt::Checked) {
      e = 1;
    }
    emphasis_[static_cast<std::size_t>(slot)] = e;
  }
  ruler_red_ = ruler->isChecked();
  chords_set_ = true;
  chords_ = defaults.chord_divisor;
  for (int n = 1; n <= 12; ++n) {
    chords_[static_cast<std::size_t>(n)] = chord_checks[static_cast<std::size_t>(n)]->isChecked();
  }
  recompute();
}

// ported from the GRAD-DATUM-LISTE of a17_3 with the published
// Gruppenschicksals-Grade and the self defined degrees of GRADE.INT
void MainWindow::degree_date_list() {
  if (!last_chart_ || !last_chart_->houses.ok) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Grad-Datum-Liste"));
  auto* v = new QVBoxLayout(&dialog);
  auto* top = new QHBoxLayout();
  auto* phase = new QDoubleSpinBox(&dialog);
  phase->setRange(-30.0, 30.0);
  phase->setDecimals(1);
  phase->setValue(7.0);
  phase->setPrefix(tr("Phase "));
  phase->setSuffix(tr(" J."));
  auto* unit = new QComboBox(&dialog);
  unit->addItem(tr("Jahre"), 0);
  unit->addItem(tr("Monate"), 1);
  auto* dir = new QComboBox(&dialog);
  //RR RECHTS = Uhrzeigersinn
  dir->addItem(tr("Links"), 1);
  dir->addItem(tr("Rechts"), 0);
  auto* mundan = new QCheckBox(tr("Mundan"), &dialog);
  auto* only_marked = new QCheckBox(tr("Nur markierte Grade"), &dialog);
  top->addWidget(phase);
  top->addWidget(unit);
  top->addWidget(dir);
  top->addWidget(mundan);
  top->addWidget(only_marked);
  top->addStretch(1);
  auto* table = new QTableWidget(0, 4, &dialog);
  table->setHorizontalHeaderLabels({tr("Grad"), tr("Haus"), tr("Alter"), tr("Charakteristik")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(18);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  //RR GRADE für GRAD-DATUM-LISTE NEU DEFINIEREN
  auto* bottom = new QHBoxLayout();
  auto* new_deg = new QDoubleSpinBox(&dialog);
  new_deg->setRange(0.0, 359.5);
  new_deg->setDecimals(1);
  new_deg->setSingleStep(0.5);
  new_deg->setSuffix(QString::fromUtf8("°"));
  auto* pa = new QComboBox(&dialog);
  auto* pb = new QComboBox(&dialog);
  for (int slot = 1; slot <= 10; ++slot) {
    const QString tag = QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                                          static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size()));
    pa->addItem(tag, slot);
    pb->addItem(tag, slot);
  }
  auto* add = new QPushButton(tr("Grad definieren"), &dialog);
  auto* clear = new QPushButton(tr("Eigene Grade löschen"), &dialog);
  bottom->addWidget(new_deg);
  bottom->addWidget(pa);
  bottom->addWidget(pb);
  bottom->addWidget(add);
  bottom->addWidget(clear);
  bottom->addStretch(1);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  const std::filesystem::path grade_file = data_dir_ / "grade.int";
  const auto tag_of = [](int slot) {
    return QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                             static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size()));
  };
  const auto refresh = [this, table, phase, unit, dir, mundan, only_marked, grade_file, tag_of]() {
    RhythmOptions opt;
    opt.phase_years = phase->value();
    opt.months = unit->currentData().toInt() == 1;
    opt.leftward = dir->currentData().toInt() == 1;
    const auto own = read_degrees(grade_file);
    const auto rows = degree_dates(*last_chart_, opt, own, mundan->isChecked(), lat_->value());
    table->setRowCount(0);
    for (const DegreeDate& r : rows) {
      const bool marked = r.p > 0 || r.custom;
      if (only_marked->isChecked() && !marked) {
        continue;
      }
      const int row = table->rowCount();
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(zodiac(r.degree * kDegToRad)));
      table->setItem(row, 1, new QTableWidgetItem(QString::number(r.house)));
      table->setItem(row, 2, new QTableWidgetItem(QString::number(r.value, 'f', 2)));
      QString ch;
      if (r.p > 0) {
        ch = tag_of(r.p) + "-" + tag_of(r.q);
        if (r.mirror) {
          ch += tr(" (Spiegel)");
        }
      }
      auto* item = new QTableWidgetItem(ch);
      if (r.custom) {
        //RR eigene Grade in Rot
        item->setForeground(QColor(0xE8, 0x5D, 0x4E));
      }
      table->setItem(row, 3, item);
      //RR die Kardinalpunkte rot markiert
      if (r.degree == 0.0 || r.degree == 90.0 || r.degree == 180.0 || r.degree == 270.0) {
        table->item(row, 0)->setForeground(QColor(0xE8, 0x5D, 0x4E));
      }
    }
    table->resizeColumnsToContents();
  };
  connect(phase, &QDoubleSpinBox::valueChanged, &dialog, refresh);
  connect(unit, &QComboBox::currentIndexChanged, &dialog, refresh);
  connect(dir, &QComboBox::currentIndexChanged, &dialog, refresh);
  connect(mundan, &QCheckBox::toggled, &dialog, refresh);
  connect(only_marked, &QCheckBox::toggled, &dialog, refresh);
  connect(add, &QPushButton::clicked, &dialog, [this, new_deg, pa, pb, grade_file, refresh]() {
    auto own = read_degrees(grade_file);
    own.push_back({new_deg->value(), pa->currentData().toInt(), pb->currentData().toInt()});
    if (!write_degrees(grade_file, own)) {
      QMessageBox::warning(this, "HORCOM", tr("Die Grade ließen sich nicht speichern."));
    }
    refresh();
  });
  connect(clear, &QPushButton::clicked, &dialog, [this, grade_file, refresh]() {
    std::error_code ec;
    std::filesystem::remove(grade_file, ec);
    refresh();
  });
  only_marked->setChecked(true);
  refresh();
  v->addLayout(top);
  v->addWidget(table, 1);
  v->addLayout(bottom);
  v->addWidget(buttons);
  dialog.resize(640, 680);
  dialog.exec();
}

// ported from the HISTOGRAMM der ELEMENTE und KARD-FIX-GEM columns of
// the chart view with his punkte_pla weighting dialog
void MainWindow::histogram_view() {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Histogramme"));
  auto* v = new QVBoxLayout(&dialog);
  auto* table = new QTableWidget(7, 3, &dialog);
  table->setHorizontalHeaderLabels({tr("Klasse"), tr("Zeichen"), tr("Häuser")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  auto* haus1 = new QCheckBox(tr("Planeten im 1. Haus doppelt"), &dialog);
  haus1->setChecked(true);
  auto* herr = new QCheckBox(tr("Geburtsherrscher doppelt"), &dialog);
  herr->setChecked(true);
  //RR PUNKTE-WERT 0....9 EINGEBEN !
  auto* weights = new QTableWidget(1, 15, &dialog);
  QStringList heads;
  for (int i = 1; i <= 14; ++i) {
    heads << QString::fromUtf8(body::kTag[static_cast<std::size_t>(i)].data(),
                               static_cast<int>(body::kTag[static_cast<std::size_t>(i)].size()));
  }
  heads << tr("Zusatz");
  weights->setHorizontalHeaderLabels(heads);
  weights->verticalHeader()->setVisible(false);
  weights->setFixedHeight(64);
  {
    const auto points = histogram_points(konsta_.pn);
    for (int i = 1; i <= 14; ++i) {
      weights->setItem(0, i - 1, new QTableWidgetItem(QString::number(points[static_cast<std::size_t>(i)])));
    }
    weights->setItem(0, 14, new QTableWidgetItem(QString::number(points[body::kChiron])));
  }
  auto* save = new QCheckBox(tr("Gewichte als konsta.int speichern"), &dialog);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  const auto refresh = [this, table, haus1, herr, weights]() {
    HistogramOptions opt;
    std::array<int, 16> pn{};
    for (int i = 1; i <= 15; ++i) {
      bool ok = false;
      const int w = weights->item(0, i - 1)->text().toInt(&ok);
      pn[static_cast<std::size_t>(i)] = ok ? std::clamp(w, 0, 9) : 0;
    }
    opt.points = histogram_points(pn);
    opt.double_first_house = haus1->isChecked();
    opt.double_ruler = herr->isChecked();
    const Histogram h = chart_histogram(*last_chart_, current_settings(), opt);
    const int top = std::max({h.element_sign[1], h.element_sign[2], h.element_sign[3], h.element_sign[4],
                              h.element_house[1], h.element_house[2], h.element_house[3], h.element_house[4],
                              h.quality_sign[1], h.quality_sign[2], h.quality_sign[3], 1});
    const auto bar = [top](int value) {
      const int len = (value * 24 + top / 2) / top;
      QString s = QString::number(value) + "  ";
      for (int i = 0; i < len; ++i) {
        s += QString::fromUtf8("█");
      }
      return s;
    };
    const char* names[7] = {"Feuer", "Erde", "Luft", "Wasser", "Kardinal", "Fix", "Gemeinschaftlich"};
    for (int r = 0; r < 7; ++r) {
      table->setItem(r, 0, new QTableWidgetItem(tr(names[r])));
      const bool element = r < 4;
      const int idx = element ? r + 1 : r - 3;
      const int sign = element ? h.element_sign[static_cast<std::size_t>(idx)] : h.quality_sign[static_cast<std::size_t>(idx)];
      const int house = element ? h.element_house[static_cast<std::size_t>(idx)] : h.quality_house[static_cast<std::size_t>(idx)];
      table->setItem(r, 1, new QTableWidgetItem(bar(sign)));
      table->setItem(r, 2, new QTableWidgetItem(h.houses_counted ? bar(house) : tr("-")));
    }
    table->resizeColumnsToContents();
  };
  connect(haus1, &QCheckBox::toggled, &dialog, refresh);
  connect(herr, &QCheckBox::toggled, &dialog, refresh);
  connect(weights, &QTableWidget::cellChanged, &dialog, [refresh](int, int) { refresh(); });
  connect(save, &QCheckBox::toggled, &dialog, [this, weights](bool on) {
    if (!on) {
      return;
    }
    for (int i = 1; i <= 15; ++i) {
      bool ok = false;
      const int w = weights->item(0, i - 1)->text().toInt(&ok);
      konsta_.pn[static_cast<std::size_t>(i)] = ok ? std::clamp(w, 0, 9) : 0;
    }
    if (!save_konsta(data_dir_ / "konsta.int", konsta_)) {
      QMessageBox::warning(this, "HORCOM", tr("Die Vorgaben ließen sich nicht speichern."));
    }
  });
  refresh();
  v->addWidget(table, 1);
  v->addWidget(haus1);
  v->addWidget(herr);
  v->addWidget(weights);
  v->addWidget(save);
  v->addWidget(buttons);
  dialog.resize(560, 520);
  dialog.exec();
}

void MainWindow::time_wander() {
  wander_dialog(false);
}

void MainWindow::place_wander() {
  wander_dialog(true);
}

// ported from the ZEIT-WANDERN and ORT-WANDERN walks with the Schiemenz
// aspect counter and the midpoint counter, the sums ride along like his
// running means over the stepped charts
void MainWindow::wander_dialog(bool place) {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(place ? tr("Ort-Wandern") : tr("Zeit-Wandern"));
  auto* v = new QVBoxLayout(&dialog);
  auto* form = new QFormLayout();
  QComboBox* unit = nullptr;
  QDoubleSpinBox* dlon = nullptr;
  QDoubleSpinBox* dlat = nullptr;
  if (place) {
    dlon = new QDoubleSpinBox(&dialog);
    dlon->setRange(-30.0, 30.0);
    dlon->setDecimals(2);
    dlon->setValue(1.0);
    dlat = new QDoubleSpinBox(&dialog);
    dlat->setRange(-30.0, 30.0);
    dlat->setDecimals(2);
    form->addRow(tr("Schritt Länge (Grad)"), dlon);
    form->addRow(tr("Schritt Breite (Grad)"), dlat);
  } else {
    unit = new QComboBox(&dialog);
    //RR Zeitmaß, D = 1 Tag, H = 1 Stunde, M = 1 Minute
    unit->addItem(tr("Minute"), 1.0 / (24.0 * 60.0));
    unit->addItem(tr("Stunde"), 1.0 / 24.0);
    unit->addItem(tr("Tag"), 1.0);
    unit->addItem(tr("Monat"), last_chart_->ta.tropical_year_days / 12.0);
    unit->addItem(tr("Jahr"), last_chart_->ta.tropical_year_days);
    unit->setCurrentIndex(2);
    form->addRow(tr("Zeitmaß"), unit);
  }
  //RR Richtung, V = Vorwärts, R = Rückwärts
  auto* back = new QCheckBox(tr("Rückwärts"), &dialog);
  //RR Intervall, + = Verdoppelung, - = Halbierung
  auto* factor = new QLabel("1", &dialog);
  auto* doubler = new QPushButton("+", &dialog);
  auto* halver = new QPushButton(QString::fromUtf8("−"), &dialog);
  doubler->setFixedWidth(32);
  halver->setFixedWidth(32);
  auto* ivl = new QHBoxLayout();
  ivl->addWidget(factor);
  ivl->addWidget(doubler);
  ivl->addWidget(halver);
  ivl->addStretch(1);
  form->addRow(tr("Intervall-Faktor"), ivl);
  //RR Wartezeit
  auto* wait = new QDoubleSpinBox(&dialog);
  wait->setRange(0.2, 10.0);
  wait->setSingleStep(0.2);
  wait->setValue(1.0);
  wait->setSuffix(" s");
  form->addRow(tr("Wartezeit"), wait);
  auto* counters = new QLabel(&dialog);
  auto* go = new QPushButton(tr("Start / Stop"), &dialog);
  auto* reset = new QPushButton(tr("Zähler zurücksetzen"), &dialog);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(go);
  v->addWidget(counters);
  v->addWidget(reset);
  v->addWidget(buttons);

  auto* timer = new QTimer(&dialog);
  double mul = 1.0;
  double jd = last_chart_->jd_ut;
  long steps = 0;
  double sum_asp = 0.0, sum_triga = 0.0, sum_gt = 0.0, sum_mid = 0.0;
  const auto show = [this, counters, &steps, &sum_asp, &sum_triga, &sum_gt, &sum_mid]() {
    if (!last_chart_ || !last_aspects_) {
      return;
    }
    const AspectResult& a = *last_aspects_;
    const MidpointResult m = scan_midpoints(*last_chart_, current_settings(), aspect_settings_);
    const int mid = m.direct + m.square + m.semi;
    ++steps;
    sum_asp += static_cast<double>(a.hits.size());
    sum_triga += a.triga;
    sum_gt += a.grand_trines;
    sum_mid += mid;
    counters->setText(tr("Schritt %1\nAspekte %2  (Mittel %3)\nTriga %4  Großtrigone %5\nHalbsummen %6  (Mittel %7)")
                          .arg(steps)
                          .arg(a.hits.size())
                          .arg(sum_asp / steps, 0, 'f', 2)
                          .arg(a.triga)
                          .arg(a.grand_trines)
                          .arg(mid)
                          .arg(sum_mid / steps, 0, 'f', 2));
  };
  const auto step = [this, place, unit, dlon, dlat, back, &mul, &jd, show]() {
    const double dir = back->isChecked() ? -1.0 : 1.0;
    if (place) {
      const QSignalBlocker b1(lon_);
      const QSignalBlocker b2(lat_);
      lon_->setValue(lon_->value() + dir * mul * dlon->value());
      lat_->setValue(std::clamp(lat_->value() + dir * mul * dlat->value(), -89.9, 89.9));
      recompute();
      banner_->set_record(tr("ORT-WANDERN"));
    } else {
      jd += dir * mul * unit->currentData().toDouble();
      apply_moment(jd, tr("ZEIT-WANDERN"));
    }
    show();
  };
  connect(timer, &QTimer::timeout, &dialog, step);
  connect(go, &QPushButton::clicked, &dialog, [timer, wait]() {
    if (timer->isActive()) {
      timer->stop();
    } else {
      timer->start(static_cast<int>(wait->value() * 1000.0));
    }
  });
  connect(wait, &QDoubleSpinBox::valueChanged, &dialog, [timer](double s) {
    if (timer->isActive()) {
      timer->setInterval(static_cast<int>(s * 1000.0));
    }
  });
  connect(doubler, &QPushButton::clicked, &dialog, [&mul, factor]() {
    mul = std::min(mul * 2.0, 64.0);
    factor->setText(QString::number(mul));
  });
  connect(halver, &QPushButton::clicked, &dialog, [&mul, factor]() {
    mul = std::max(mul / 2.0, 0.0625);
    factor->setText(QString::number(mul));
  });
  connect(reset, &QPushButton::clicked, &dialog, [&steps, &sum_asp, &sum_triga, &sum_gt, &sum_mid, counters]() {
    steps = 0;
    sum_asp = sum_triga = sum_gt = sum_mid = 0.0;
    counters->setText(QString());
  });
  counters->setText(tr("Start drückt die Wanderung an, das Ergebnis überschreibt das Panel."));
  dialog.exec();
  timer->stop();
}

void MainWindow::linear_graph() {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  //RR LINEAR-GRAPHIK
  dialog.setWindowTitle(tr("Linear-Graphik"));
  auto* v = new QVBoxLayout(&dialog);
  auto* top = new QHBoxLayout();
  auto* kind = new QComboBox(&dialog);
  kind->addItem(tr("Sekundär-Direktion"), 1);
  kind->addItem(tr("Sonnenbogen-Direktion"), 2);
  //RR MOND-BOGEN-DIREKTION = 'TERTIÄR 2'-DIREKTION , viele Auslösungen !
  kind->addItem(tr("Mondbogen-Direktion"), 3);
  kind->addItem(tr("Transite"), 0);
  auto* base = new QComboBox(&dialog);
  //RR jeder Winkel der sich durch ganzzahlige Teilung von 360 Grad ergibt
  for (double b : {360.0, 180.0, 120.0, 90.0, 60.0, 45.0, 30.0, 15.0}) {
    base->addItem(QString::number(b) + QString::fromUtf8("°"), b);
  }
  base->setCurrentIndex(3);
  auto* from_age = new QSpinBox(&dialog);
  from_age->setRange(0, 150);
  from_age->setSuffix(tr(" J."));
  auto* span = new QComboBox(&dialog);
  //RR ein Zeitraum von 5,10,20,40,80 oder 160 Jahren
  for (int s : {5, 10, 20, 40, 80, 160}) {
    span->addItem(tr("%1 Jahre").arg(s), s);
  }
  span->setCurrentIndex(2);
  auto* down = new QCheckBox(tr("Nach unten positiv (R. Ebertin)"), &dialog);
  auto* zeichen = new QCheckBox(tr("Zeichen"), &dialog);
  auto* houses = new QCheckBox(tr("Zwischenhäuser"), &dialog);
  top->addWidget(kind);
  top->addWidget(new QLabel(tr("Grundwinkel"), &dialog));
  top->addWidget(base);
  top->addWidget(new QLabel(tr("ab Lebensjahr"), &dialog));
  top->addWidget(from_age);
  top->addWidget(span);
  top->addWidget(down);
  top->addWidget(zeichen);
  top->addWidget(houses);
  top->addStretch(1);
  auto* view = new WheelWidget(&dialog);
  view->setMinimumSize(780, 560);
  v->addLayout(top);
  v->addWidget(view, 1);
  const auto draw = [this, kind, base, from_age, span, down, zeichen, houses, view]() {
    const Chart& radix = *last_chart_;
    LinearOptions opt;
    switch (kind->currentData().toInt()) {
      case 0: opt.kind = LinearKind::kTransits; break;
      case 2: opt.kind = LinearKind::kSunArc; break;
      case 3: opt.kind = LinearKind::kMoonArc; break;
      default: opt.kind = LinearKind::kSecondary; break;
    }
    opt.base_angle_deg = base->currentData().toDouble();
    const double tja = radix.ta.tropical_year_days;
    opt.jd_from_ut = radix.jd_ut + from_age->value() * tja;
    opt.jd_to_ut = opt.jd_from_ut + span->currentData().toInt() * tja;
    opt.downward = down->isChecked();
    opt.sign_lines = zeichen->isChecked();
    opt.with_houses = houses->isChecked();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    view->set_display_list(build_linear_graph(radix, opt, make_context()));
    QApplication::restoreOverrideCursor();
  };
  connect(kind, &QComboBox::currentIndexChanged, &dialog, draw);
  connect(base, &QComboBox::currentIndexChanged, &dialog, draw);
  connect(span, &QComboBox::currentIndexChanged, &dialog, draw);
  connect(from_age, &QSpinBox::valueChanged, &dialog, draw);
  connect(down, &QCheckBox::toggled, &dialog, draw);
  connect(zeichen, &QCheckBox::toggled, &dialog, draw);
  connect(houses, &QCheckBox::toggled, &dialog, draw);
  draw();
  dialog.resize(1040, 700);
  dialog.exec();
}

void MainWindow::dynamogram_view() {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  //RR nach KRAFFT-GOERNER
  dialog.setWindowTitle(tr("Dynamogramm nach Krafft-Goerner"));
  auto* v = new QVBoxLayout(&dialog);
  auto* top = new QHBoxLayout();
  auto* age = new QDoubleSpinBox(&dialog);
  age->setRange(0.0, 120.0);
  age->setDecimals(0);
  age->setValue(std::floor((julian_day({QDate::currentDate().day(), QDate::currentDate().month(),
                                        QDate::currentDate().year(), 12, 0.0}) -
                            last_chart_->jd_ut) /
                           last_chart_->ta.tropical_year_days));
  age->setPrefix(tr("ab Lebensjahr "));
  auto* moon = new QCheckBox(tr("Mond"), &dialog);
  auto* gauss = new QCheckBox(tr("Gauß-Kurven"), &dialog);
  auto* minors = new QCheckBox(tr("Nebenaspekte"), &dialog);
  auto* regress = new QCheckBox(tr("Regressiv dazu"), &dialog);
  auto* run = new QPushButton(tr("Rechnen"), &dialog);
  top->addWidget(age);
  top->addWidget(moon);
  top->addWidget(gauss);
  top->addWidget(minors);
  top->addWidget(regress);
  top->addWidget(run, 1);
  auto* graph = new WheelWidget(&dialog);
  auto* note = new QLabel(tr("Gold die Grundstimmung, Rot die existenzielle Linie der Achsen. Fünf Lebensjahre je Bild."), &dialog);
  note->setWordWrap(true);
  const auto fill = [this, age, moon, gauss, minors, regress, graph]() {
    DynamogramOptions opt;
    opt.from_age = age->value();
    opt.with_moon = moon->isChecked();
    opt.gauss = gauss->isChecked();
    opt.classic_minors = minors->isChecked();
    opt.regressive = regress->isChecked();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const Dynamogram d = dynamogram(*last_chart_, opt, make_context());
    QApplication::restoreOverrideCursor();
    // the visible window of the original, five years from the asked
    // age, one hundred twenty samples a year on a 640 wide sheet
    DisplayList dl;
    const double x0 = 20.0;
    const double y0 = 240.0;
    const double sx = 600.0 / 600.0;
    dl.items.push_back({Primitive::Kind::kLine, x0, y0, x0 + 600.0, y0});
    double peak = 1.0;
    for (int i = 3000; i <= 3600; ++i) {
      peak = std::max({peak, std::abs(d.mood[static_cast<std::size_t>(i)]),
                       std::abs(d.existential[static_cast<std::size_t>(i)])});
    }
    const double sy = 200.0 / peak;
    for (int year = 0; year <= 5; ++year) {
      const double x = x0 + year * 120.0 * sx;
      dl.items.push_back({Primitive::Kind::kLine, x, y0 - 6.0, x, y0 + 6.0});
      Primitive t;
      t.kind = Primitive::Kind::kText;
      t.x1 = x;
      t.y1 = y0 + 22.0;
      t.size = 12.0;
      t.text = std::to_string(static_cast<int>(opt.from_age) + year);
      dl.items.push_back(t);
    }
    const auto curve = [&](const std::vector<double>& c, Rgb color) {
      for (int i = 3000; i < 3600; ++i) {
        Primitive l;
        l.kind = Primitive::Kind::kLine;
        l.x1 = x0 + (i - 3000) * sx;
        l.y1 = y0 - c[static_cast<std::size_t>(i)] * sy;
        l.x2 = x0 + (i + 1 - 3000) * sx;
        l.y2 = y0 - c[static_cast<std::size_t>(i + 1)] * sy;
        l.color = color;
        l.width = 1.4;
        dl.items.push_back(l);
      }
    };
    curve(d.mood, 0xB8860B);
    curve(d.existential, 0xC03020);
    graph->set_display_list(dl);
  };
  connect(run, &QPushButton::clicked, &dialog, fill);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(top);
  v->addWidget(graph, 1);
  v->addWidget(note);
  v->addWidget(buttons);
  dialog.resize(880, 620);
  fill();
  dialog.exec();
}

void MainWindow::rise_set() {
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Aufgang, Meridian-Durchgang, Untergang"));
  auto* v = new QVBoxLayout(&dialog);
  auto* top = new QHBoxLayout();
  auto* when = new QDateEdit(date_->date(), &dialog);
  when->setCalendarPopup(true);
  when->setDisplayFormat("dd.MM.yyyy");
  auto* mode = new QComboBox(&dialog);
  //RR WAHRE POSITION ? SCHEINBARE POSITION ?
  mode->addItem(tr("Scheinbar"), 0);
  mode->addItem(tr("Wahr"), 1);
  auto* run = new QPushButton(tr("Rechnen"), &dialog);
  top->addWidget(when);
  top->addWidget(mode);
  top->addWidget(run, 1);
  auto* table = new QTableWidget(0, 4, &dialog);
  table->setHorizontalHeaderLabels({tr("Planet"), tr("Aufgang"), tr("Meridian"), tr("Untergang")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  auto* note = new QLabel(tr("Zeiten in UT für den Ort des Panels."), &dialog);
  const auto clock = [this](double jd) {
    const CalendarDate d = calendar_date(jd, current_settings().calendar);
    int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
    if (seconds >= kSecondsPerDay) {
      seconds = kSecondsPerDay - 1;
    }
    return QString::asprintf("%02d:%02d", seconds / 3600, (seconds / 60) % 60);
  };
  const auto fill = [this, when, mode, table, clock]() {
    const QDate d = when->date();
    const double jd = julian_day({d.day(), d.month(), d.year(), 12, 0.0}, current_settings().calendar);
    const SearchContext ctx = make_context();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    table->setRowCount(0);
    for (int slot = 1; slot <= 10; ++slot) {
      const RiseSet rs = rise_transit_set(jd, slot, mode->currentData().toInt() == 1, ctx);
      const int row = table->rowCount();
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                                                                    static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size()))));
      if (!rs.ok) {
        //RR AUßER BEREICH !
        table->setItem(row, 1, new QTableWidgetItem(rs.circumpolar ? tr("außer Bereich") : QString::fromUtf8("—")));
        continue;
      }
      table->setItem(row, 1, new QTableWidgetItem(clock(rs.jd_rise_ut)));
      table->setItem(row, 2, new QTableWidgetItem(clock(rs.jd_transit_ut)));
      table->setItem(row, 3, new QTableWidgetItem(clock(rs.jd_set_ut)));
    }
    QApplication::restoreOverrideCursor();
  };
  connect(run, &QPushButton::clicked, &dialog, fill);
  fill();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(top);
  v->addWidget(table, 1);
  v->addWidget(note);
  v->addWidget(buttons);
  dialog.resize(460, 420);
  dialog.exec();
}

void MainWindow::eclipse_table() {
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Neumond, Vollmond und Finsternisse"));
  auto* v = new QVBoxLayout(&dialog);
  auto* top = new QHBoxLayout();
  auto* when = new QDateEdit(date_->date(), &dialog);
  when->setCalendarPopup(true);
  when->setDisplayFormat("dd.MM.yyyy");
  auto* run = new QPushButton(tr("Rechnen"), &dialog);
  top->addWidget(new QLabel(tr("Such-Datum"), this));
  top->addWidget(when);
  top->addWidget(run, 1);
  auto* table = new QTableWidget(0, 6, &dialog);
  table->setHorizontalHeaderLabels({tr("Neumond (UT)"), tr("Sonnenfinsternis"), tr("Aspekte SO"), tr("Vollmond (UT)"),
                                    tr("Mondfinsternis"), tr("Aspekte MO")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  //RR ASPEKTE mit GÜLTIGEM DATENSATZ UNTERSUCHEN ?
  auto* with_aspects = new QCheckBox(tr("Aspekte mit gültigem Datensatz"), &dialog);
  auto* base = new QComboBox(&dialog);
  //RR GRUND-ASPEKT WÄHLEN! the maximal divisor follows from it
  for (int b : {30, 45, 60, 90, 180, 360}) {
    base->addItem(QString::number(b) + QString::fromUtf8("°"), b);
  }
  auto* orbf = new QComboBox(&dialog);
  //RR ORBIS-FAKTOR ? his four steps
  for (double f : {0.125, 0.25, 0.5, 1.0}) {
    orbf->addItem(QString::number(f), f);
  }
  orbf->setCurrentIndex(2);
  top->addWidget(with_aspects);
  top->addWidget(new QLabel(tr("Grund-Aspekt"), &dialog));
  top->addWidget(base);
  top->addWidget(new QLabel(tr("Orbis-Faktor"), &dialog));
  top->addWidget(orbf);
  //RR die Zeichen-Erklärung seines Schirms
  auto* legend = new QLabel(tr("ZT = zentral, EX = exzentrisch, TOT = total, RF = ringförmig, N/S = nördlich/südlich,\n"
                               "KERNSCH = Kernschatten, HALBSCH = Halbschatten. Zeiten in UT."),
                            &dialog);
  legend->setWordWrap(true);
  const auto stamp = [this](double jd) {
    const CalendarDate d = calendar_date(jd, current_settings().calendar);
    int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
    if (seconds >= kSecondsPerDay) {
      seconds = kSecondsPerDay - 1;
    }
    return QString::asprintf("%02d.%02d.%04d %02d:%02d", d.day, d.month, d.year, seconds / 3600, (seconds / 60) % 60);
  };
  // ported from suchas, the syzygy light against every radix factor
  const auto syzygy_aspects = [this](double jd_ut, bool full, int base_deg, double orb_factor) {
    if (!last_chart_) {
      return QString();
    }
    const Chart& radix = *last_chart_;
    const int light = full ? body::kMoon : body::kSun;
    const BodyLongitude bl = body_longitude(jd_ut, light, make_context());
    if (!bl.valid) {
      return QString();
    }
    const int d1 = static_cast<int>(1.0e-5 + 360.0 / base_deg);
    const double o1 = org(aspect_settings_, light, 1);
    QString out;
    for (int slot = 1; slot < body::kSlotCount; ++slot) {
      const BodyState& b = radix.b[static_cast<std::size_t>(slot)];
      // his skips, never the descending node, never a zero position
      if (!b.present || !b.valid || slot == body::kNodeDesc || b.el == 0.0) {
        continue;
      }
      const double o2 = org(aspect_settings_, slot, 1);
      const double w3 = norm_rad(bl.el - b.el);
      bool taken = false;
      for (int w = 1; w <= d1 && !taken; ++w) {
        const double pn = kTwoPi / w;
        const double dds = orbis_discr2(o1, o2, orb_factor * pn / 30.0);
        if (dds <= 0.0) {
          break;
        }
        if (w == 1) {
          if (w3 > 0.0 && (w3 < dds || w3 > kTwoPi - dds)) {
            taken = true;
            if (!out.isEmpty()) {
              out += "  ";
            }
            out += QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                                     static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size())) +
                   QString::fromUtf8(" 0°");
          }
          continue;
        }
        for (int m = 1; m < w && !taken; ++m) {
          double w1 = norm_rad(m * pn - dds);
          double w2 = norm_rad(m * pn + dds);
          double v3 = w3;
          vergl2(w1, w2, v3);
          if (w1 < v3 && v3 < w2) {
            taken = true;
            if (!out.isEmpty()) {
              out += "  ";
            }
            out += QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                                     static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size())) +
                   QString::asprintf(" %.0f\xC2\xB0", m * pn * kRadToDeg);
          }
        }
      }
    }
    return out;
  };
  const auto fill = [this, when, table, stamp, with_aspects, base, orbf, syzygy_aspects]() {
    const QDate d = when->date();
    const double jd = julian_day({d.day(), d.month(), d.year(), 0, 0.0}, current_settings().calendar);
    //RR 27 Zeilen wie sein Schirm
    const std::vector<Lunation> nm = lunations(jd, 27, false);
    const std::vector<Lunation> fm = lunations(jd, 27, true);
    const bool asp = with_aspects->isChecked() && last_chart_;
    if (asp) {
      QApplication::setOverrideCursor(Qt::WaitCursor);
    }
    table->setRowCount(0);
    for (std::size_t i = 0; i < nm.size(); ++i) {
      const int row = table->rowCount();
      table->insertRow(row);
      auto* ndate = new QTableWidgetItem(stamp(nm[i].jd_ut));
      auto* nkind = new QTableWidgetItem(QString::fromUtf8(nm[i].kind.c_str()));
      if (nm[i].eclipse) {
        ndate->setForeground(QColor(0xE8, 0x5D, 0x4E));
        nkind->setForeground(QColor(0xE8, 0x5D, 0x4E));
      }
      table->setItem(row, 0, ndate);
      table->setItem(row, 1, nkind);
      if (asp) {
        table->setItem(row, 2, new QTableWidgetItem(syzygy_aspects(nm[i].jd_ut, false, base->currentData().toInt(),
                                                                   orbf->currentData().toDouble())));
      }
      auto* fdate = new QTableWidgetItem(stamp(fm[i].jd_ut));
      auto* fkind = new QTableWidgetItem(QString::fromUtf8(fm[i].kind.c_str()));
      if (fm[i].eclipse) {
        fdate->setForeground(QColor(0xE8, 0x5D, 0x4E));
        fkind->setForeground(QColor(0xE8, 0x5D, 0x4E));
      }
      table->setItem(row, 3, fdate);
      table->setItem(row, 4, fkind);
      if (asp) {
        table->setItem(row, 5, new QTableWidgetItem(syzygy_aspects(fm[i].jd_ut, true, base->currentData().toInt(),
                                                                   orbf->currentData().toDouble())));
      }
    }
    if (asp) {
      QApplication::restoreOverrideCursor();
    }
    table->resizeColumnsToContents();
  };
  connect(with_aspects, &QCheckBox::toggled, &dialog, fill);
  connect(base, &QComboBox::currentIndexChanged, &dialog, fill);
  connect(orbf, &QComboBox::currentIndexChanged, &dialog, fill);
  connect(run, &QPushButton::clicked, &dialog, fill);
  fill();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(top);
  v->addWidget(table, 1);
  v->addWidget(legend);
  v->addWidget(buttons);
  dialog.resize(640, 640);
  dialog.exec();
}

void MainWindow::great_year() {
  if (!last_chart_) {
    return;
  }
  const Chart& chart = *last_chart_;
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Großes (Platonisches) Jahr"));
  auto* v = new QVBoxLayout(&dialog);
  auto* form = new QFormLayout();
  auto* age = new QComboBox(&dialog);
  //RR Zeitalter - Start wählen !
  age->addItem(tr("Widder = 30°"), 30);
  age->addItem(tr("Fische = 360°"), 360);
  age->addItem(tr("Wassermann = 330°"), 330);
  age->addItem(tr("Steinbock = 300°"), 300);
  age->setCurrentIndex(2);
  //RR CHAUVIN f.AQU.
  const CalendarDate ref0 = calendar_date(2370832.0, current_settings().calendar);
  auto* refdate = new QDateEdit(QDate(ref0.year, ref0.month, ref0.day), &dialog);
  refdate->setCalendarPopup(true);
  refdate->setDisplayFormat("dd.MM.yyyy");
  form->addRow(tr("Zeitalters-Punkt"), age);
  form->addRow(tr("Bezugsdatum"), refdate);
  auto* result = new QLabel(&dialog);
  result->setWordWrap(true);
  auto* note = new QLabel(tr("Pro Zeichen ca. 2148 Jahre, rund 50.269\" pro Jahr, der Punkt wandert rückläufig."), &dialog);
  note->setWordWrap(true);
  const auto update = [this, &chart, age, refdate, result]() {
    const QDate rd = refdate->date();
    const double jda = julian_day({rd.day(), rd.month(), rd.year(), 12, 0.0}, current_settings().calendar);
    const double la0 = age->currentData().toInt() * kDegToRad;
    const Equatorial eq0 = ecliptic_to_equatorial(la0, 0.0, chart.smo.ekls);
    double ar = 0.0;
    double de = 0.0;
    precess_newcomb(chart.jd_et, chart.ta.tropical_year_days, jda, eq0.ra, eq0.dec, ar, de);
    const Ecliptic ec = equatorial_to_ecliptic(ar, de, chart.smo.ekls);
    const double di = (la0 - ec.lon) * kRadToDeg;
    const double point = norm_deg(age->currentData().toInt() + di);
    result->setText(tr("Längen-Differenz zur Bezugs-Länge: %1°\n"
                       "Der Zeitalter-Punkt für das Horoskop-Datum steht bei %2 = %3°")
                        .arg(std::min(std::abs(di), std::abs(360.0 - di)), 0, 'f', 4)
                        .arg(zodiac(point * kDegToRad))
                        .arg(point, 0, 'f', 4));
  };
  connect(age, &QComboBox::currentIndexChanged, &dialog, update);
  connect(refdate, &QDateEdit::dateChanged, &dialog, update);
  update();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(result);
  v->addWidget(note);
  v->addWidget(buttons);
  dialog.exec();
}

void MainWindow::converters() {
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Umrechnungen"));
  auto* v = new QVBoxLayout(&dialog);
  auto* form = new QFormLayout();
  // julian date against the calendar, both ways
  auto* jd_in = new QDoubleSpinBox(&dialog);
  jd_in->setRange(0.0, 4000000.0);
  jd_in->setDecimals(5);
  jd_in->setValue(2451545.0);
  auto* jd_out = new QLabel(&dialog);
  form->addRow(tr("Julianisches Datum"), jd_in);
  form->addRow(tr("ergibt (UT)"), jd_out);
  const Calendar cal = current_settings().calendar;
  const auto jd_update = [jd_in, jd_out, cal]() {
    const CalendarDate d = calendar_date(jd_in->value(), cal);
    int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
    if (seconds >= kSecondsPerDay) {
      seconds = kSecondsPerDay - 1;
    }
    jd_out->setText(QString::asprintf("%02d.%02d.%d  %02d:%02d:%02d", d.day, d.month, d.year, seconds / 3600,
                                      (seconds / 60) % 60, seconds % 60));
  };
  connect(jd_in, &QDoubleSpinBox::valueChanged, &dialog, jd_update);
  jd_update();
  // delta T of the panel date carries UT to ET and back
  const double jd_panel = last_chart_ ? last_chart_->jd_ut : 2451545.0;
  auto* delt = new QLabel(QString::fromUtf8("ΔT = %1 min,  ET = UT + ΔT")
                              .arg(delta_t_minutes(jd_panel), 0, 'f', 2),
                          &dialog);
  form->addRow(tr("Zum Panel-Datum"), delt);
  // local time against greenwich by the panel longitude
  auto* lt = new QLabel(QString::fromUtf8("LT = UT %1%2 h (Länge %3°)")
                            .arg(lon_->value() >= 0 ? "+ " : "− ")
                            .arg(std::abs(lon_->value()) / kDegPerHour, 0, 'f', 4)
                            .arg(lon_->value(), 0, 'f', 4),
                        &dialog);
  form->addRow(tr("Ortszeit"), lt);
  // decimal degrees against degree minute second
  auto* deg_in = new QDoubleSpinBox(&dialog);
  deg_in->setRange(-360.0, 360.0);
  deg_in->setDecimals(6);
  auto* deg_out = new QLabel(&dialog);
  form->addRow(tr("Winkel dezimal"), deg_in);
  form->addRow(tr("ergibt"), deg_out);
  const auto deg_update = [deg_in, deg_out]() {
    int d = 0;
    int m = 0;
    int s = 0;
    to_dms(deg_in->value(), d, m, s);
    deg_out->setText(QString::fromUtf8("%1%2° %3' %4\"")
                         .arg(deg_in->value() < 0 ? "−" : "")
                         .arg(d)
                         .arg(m, 2, 10, QChar('0'))
                         .arg(s, 2, 10, QChar('0')));
  };
  connect(deg_in, &QDoubleSpinBox::valueChanged, &dialog, deg_update);
  deg_update();
  //RR AR-DE aus EL-EB und EL-EB aus AR-DE, das wahre Äquinoktium
  const double ekls = last_chart_ ? last_chart_->smo.ekls : 0.409092804;
  auto* el_in = new QDoubleSpinBox(&dialog);
  el_in->setRange(0.0, 360.0);
  el_in->setDecimals(4);
  auto* eb_in = new QDoubleSpinBox(&dialog);
  eb_in->setRange(-90.0, 90.0);
  eb_in->setDecimals(4);
  auto* eq_out = new QLabel(&dialog);
  auto* ecl_row = new QHBoxLayout();
  ecl_row->addWidget(el_in);
  ecl_row->addWidget(eb_in);
  form->addRow(tr("Ekl. Länge / Breite"), ecl_row);
  form->addRow(tr("ergibt AR / DE"), eq_out);
  const auto eq_update = [el_in, eb_in, eq_out, ekls]() {
    const Equatorial eq = ecliptic_to_equatorial(el_in->value() * kDegToRad, eb_in->value() * kDegToRad, ekls);
    eq_out->setText(QString::fromUtf8("AR %1°   DE %2°")
                        .arg(norm_rad(eq.ra) * kRadToDeg, 0, 'f', 4)
                        .arg(eq.dec * kRadToDeg, 0, 'f', 4));
  };
  connect(el_in, &QDoubleSpinBox::valueChanged, &dialog, eq_update);
  connect(eb_in, &QDoubleSpinBox::valueChanged, &dialog, eq_update);
  eq_update();
  auto* ar_in = new QDoubleSpinBox(&dialog);
  ar_in->setRange(0.0, 360.0);
  ar_in->setDecimals(4);
  auto* de_in = new QDoubleSpinBox(&dialog);
  de_in->setRange(-90.0, 90.0);
  de_in->setDecimals(4);
  auto* ecl_out = new QLabel(&dialog);
  auto* eq_row = new QHBoxLayout();
  eq_row->addWidget(ar_in);
  eq_row->addWidget(de_in);
  form->addRow(tr("AR / Deklination"), eq_row);
  form->addRow(tr("ergibt Länge / Breite"), ecl_out);
  const auto ecl_update = [ar_in, de_in, ecl_out, ekls]() {
    const Ecliptic ec = equatorial_to_ecliptic(ar_in->value() * kDegToRad, de_in->value() * kDegToRad, ekls);
    ecl_out->setText(QString::fromUtf8("Länge %1°   Breite %2°")
                         .arg(norm_rad(ec.lon) * kRadToDeg, 0, 'f', 4)
                         .arg(ec.lat * kRadToDeg, 0, 'f', 4));
  };
  connect(ar_in, &QDoubleSpinBox::valueChanged, &dialog, ecl_update);
  connect(de_in, &QDoubleSpinBox::valueChanged, &dialog, ecl_update);
  ecl_update();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(buttons);
  dialog.exec();
}

SearchContext MainWindow::make_context() const {
  SearchContext ctx;
  ctx.base = current_input();
  ctx.settings = current_settings();
  ctx.vsop = &vsop_;
  ctx.eph = &eph_;
  return ctx;
}

void MainWindow::septar_chart() {
  if (!last_chart_ || !last_chart_->b[body::kSun].valid) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Septar"));
  auto* v = new QVBoxLayout(&dialog);
  auto* form = new QFormLayout();
  auto* age = new QSpinBox(&dialog);
  age->setRange(0, 150);
  auto* phase = new QDoubleSpinBox(&dialog);
  phase->setRange(1.0, 30.0);
  phase->setDecimals(1);
  //RR die Phase der Rhythmenlehre, sieben Jahre je Septar
  phase->setValue(7.0);
  auto* unit = new QComboBox(&dialog);
  unit->addItem(tr("Monat"), 1);
  unit->addItem(tr("Jahr"), 12);
  form->addRow(tr("Interessierendes Lebensjahr"), age);
  form->addRow(tr("Phase"), phase);
  form->addRow(tr("Zeit-Einheit"), unit);
  auto* note = new QLabel(tr("Ein Septar ist das Solar jenes Lebensjahres, in dessen Sieben-Jahres-Phase das gewählte Alter fällt."), &dialog);
  note->setWordWrap(true);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(note);
  v->addWidget(buttons);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const double vp = phase->value();
  const int fa = unit->currentData().toInt();
  const int sen = static_cast<int>(std::trunc(age->value() / vp / fa)) + 1;
  const SearchContext ctx = make_context();
  const int year = date_->date().year() + sen - 1;
  const LongitudeCrossing hit = solar_return(ctx.base.date_ut, last_chart_->b[body::kSun].el, year, ctx);
  if (!hit.ok) {
    banner_->set_record(tr("Kein Septar gefunden"));
    return;
  }
  if (age->value() < vp * fa) {
    //RR 1. SEPTAR = RADIX !
    QMessageBox::information(this, tr("Septar"), tr("Das erste Septar ist das Radix selbst."));
  }
  apply_moment(hit.jd_ut, QString("%1.SEPTAR").arg(sen));
}

void MainWindow::planetar_chart() {
  if (!last_chart_) {
    return;
  }
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
  for (const auto& [slot, name] : kPlanetar) {
    const BodyState& b = last_chart_->b[static_cast<std::size_t>(slot)];
    if (b.present && b.valid) {
      bodybox->addItem(name, slot);
    }
  }
  auto* nr = new QSpinBox(&dialog);
  nr->setRange(1, 200);
  auto* dir = new QComboBox(&dialog);
  dir->addItem(tr("Zukunft"), 1);
  dir->addItem(tr("Vergangenheit"), 0);
  form->addRow(tr("Planet"), bodybox);
  form->addRow(tr("Nummer"), nr);
  form->addRow(tr("Richtung"), dir);
  //RR NICHT SINNVOLL für Horoskope von MENSCHEN !
  auto* note = new QLabel(tr("Die Wiederkehr der langsamen Körper übersteigt ein Menschenleben."), &dialog);
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
  const SearchContext ctx = make_context();
  const double birth = last_chart_->jd_ut;
  const double radix = last_chart_->b[static_cast<std::size_t>(slot)].el;
  const LongitudeCrossing hit = planetar_return(birth, slot, radix, nr->value(), dir->currentData().toInt() == 1, ctx);
  if (!hit.ok) {
    banner_->set_record(tr("Kein Planetar gefunden"));
    return;
  }
  // the return count in front of the name, his solnummer
  const double ta = body_period_days(slot, last_chart_->ta.tropical_year_days);
  const double n = (5.0 + hit.jd_ut - birth) / ta;
  const int count = hit.jd_ut >= birth ? static_cast<int>(std::trunc(n)) : static_cast<int>(std::trunc(n)) - 1;
  double moment = hit.jd_ut;
  //RR bei mehrdeutigen PLANETAREN die 3 oder mehr Zeitpunkte der gleichen Nr.
  const auto extra = planetar_multiples(hit, slot, radix, ctx);
  if (!extra.empty()) {
    QStringList choices;
    const auto pretty = [this](const LongitudeCrossing& c) {
      const CalendarDate cd = calendar_date(c.jd_ut, current_settings().calendar);
      return QString::asprintf("%02d.%02d.%04d", cd.day, cd.month, cd.year) + (c.retrograde ? tr(" (rückläufig)") : QString());
    };
    choices << pretty(hit);
    for (const LongitudeCrossing& c : extra) {
      choices << pretty(c);
    }
    bool picked = false;
    const QString sel = QInputDialog::getItem(this, tr("Mehrdeutiges Planetar"),
                                              tr("Der Körper überläuft den Punkt mehrfach:"), choices, 0, false, &picked);
    if (!picked) {
      return;
    }
    const int idx = choices.indexOf(sel);
    if (idx > 0) {
      moment = extra[static_cast<std::size_t>(idx - 1)].jd_ut;
    }
  }
  apply_moment(moment, QString("%1.%2").arg(count).arg(bodybox->currentText()));
}

void MainWindow::personar_chart() {
  if (!last_chart_ || !last_chart_->b[body::kSun].valid) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Personar"));
  auto* v = new QVBoxLayout(&dialog);
  auto* form = new QFormLayout();
  auto* bodybox = new QComboBox(&dialog);
  static constexpr std::pair<int, const char*> kPersonar[] = {
      {body::kMoon, "MOND-PERS"},     {body::kMercury, "MERKUR-PERS"}, {body::kVenus, "VENUS-PERS"},
      {body::kMars, "MARS-PERS"},     {body::kJupiter, "JUPITER-PERS"}, {body::kSaturn, "SATURN-PERS"},
      {body::kUranus, "URANUS-PERS"}, {body::kNeptune, "NEPTUN-PERS"}, {body::kPluto, "PLUTO-PERS"},
      {body::kChiron, "CHIRON-PERS"}, {body::kCeres, "CERES-PERS"},    {body::kPallas, "PALLAS-PERS"},
      {body::kJuno, "JUNO-PERS"},     {body::kVesta, "VESTA-PERS"},    {body::kQuaoar, "QUAOAR-PERS"},
      {body::kHalley, "HALLEY-PERS"}, {body::kPholus, "PHOLUS-PERS"},  {body::kDamokles, "DAMOKLES-PERS"},
      {body::kNessus, "NESSUS-PERS"}, {body::kXena, "XENA-PERS"}};
  for (const auto& [slot, name] : kPersonar) {
    const BodyState& b = last_chart_->b[static_cast<std::size_t>(slot)];
    if (b.present && b.valid) {
      bodybox->addItem(name, slot);
    }
  }
  auto* seed = new QComboBox(&dialog);
  seed->addItem(tr("Normal"), 0);
  //RR Für GRENZFÄLLE !
  seed->addItem(tr("Ein Jahr vorwärts (Grenzfälle)"), 1);
  seed->addItem(tr("Ein Jahr zurück (Grenzfälle)"), 2);
  form->addRow(tr("Planet"), bodybox);
  form->addRow(tr("Suche"), seed);
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
  const SearchContext ctx = make_context();
  const double birth = last_chart_->jd_ut;
  const double tja = last_chart_->ta.tropical_year_days;
  double start = birth + tja;
  if (seed->currentData().toInt() == 1) {
    start = birth + 2.0 * tja;
  } else if (seed->currentData().toInt() == 2) {
    start = birth + 3.0;
  }
  const LongitudeCrossing hit =
      find_longitude_backward(start, body::kSun, last_chart_->b[static_cast<std::size_t>(slot)].el, ctx);
  if (!hit.ok) {
    banner_->set_record(tr("Kein Personar gefunden"));
    return;
  }
  apply_moment(hit.jd_ut, bodybox->currentText());
}

void MainWindow::progression_chart() {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Progressions-Horoskop"));
  auto* v = new QVBoxLayout(&dialog);
  auto* form = new QFormLayout();
  auto* when = new QDateEdit(QDate::currentDate(), &dialog);
  when->setCalendarPopup(true);
  when->setDisplayFormat("dd.MM.yyyy");
  auto* mode = new QComboBox(&dialog);
  //RR RECHEN-MODUS ?
  mode->addItem(tr("UT = Radix-UT"), 1);
  mode->addItem(tr("Wahre Sonnenzeit = wahre Sonnenzeit Radix"), 2);
  mode->addItem(tr("Häuser-Drehung gemäß '1 Tag = 1 Jahr'"), 3);
  mode->addItem(tr("Streng proportionale Umrechnung des JD"), 4);
  form->addRow(tr("Ereignis-Datum"), when);
  form->addRow(tr("Rechen-Modus"), mode);
  auto* note = new QLabel(tr("Ein Tag Himmelslauf steht für ein Lebensjahr."), &dialog);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addLayout(form);
  v->addWidget(note);
  v->addWidget(buttons);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const QDate d = when->date();
  const double event = julian_day({d.day(), d.month(), d.year(), 0, 0.0}, current_settings().calendar);
  const ProgressedMoment m = progressed_moment(*last_chart_, event,
                                               static_cast<ProgressionMode>(mode->currentData().toInt()),
                                               make_context());
  if (!m.ok) {
    banner_->set_record(tr("Keine Progression gefunden"));
    return;
  }
  apply_moment(m.jd_ut, "PROG-HOR");
}

void MainWindow::day_chart() {
  if (!last_chart_) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Tages-Horoskop"));
  auto* v = new QVBoxLayout(&dialog);
  auto* note = new QLabel(tr("Der Moment des gewählten Tages mit der wahren Sonnenzeit der Geburt."), &dialog);
  note->setWordWrap(true);
  auto* when = new QDateEdit(QDate::currentDate(), &dialog);
  when->setCalendarPopup(true);
  when->setDisplayFormat("dd.MM.yyyy");
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addWidget(note);
  v->addWidget(when);
  v->addWidget(buttons);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const QDate d = when->date();
  const double seed = julian_day({d.day(), d.month(), d.year(), 12, 0.0}, current_settings().calendar);
  const ProgressedMoment m = day_chart_moment(*last_chart_, seed, make_context());
  if (!m.ok) {
    banner_->set_record(tr("Kein Tages-Horoskop gefunden"));
    return;
  }
  apply_moment(m.jd_ut, "TAG-HOR");
}

void MainWindow::lunar_chart() {
  if (!last_chart_ || !last_chart_->b[body::kMoon].valid) {
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Lunar"));
  auto* v = new QVBoxLayout(&dialog);
  //RR Meist wird das Diesem Datum, 0H UT ,VORAUSGEHENDE LUNAR berechnet !
  auto* note = new QLabel(tr("Das diesem Datum, 0h UT, vorausgehende Lunar wird berechnet."), &dialog);
  note->setWordWrap(true);
  auto* when = new QDateEdit(QDate::currentDate(), &dialog);
  when->setCalendarPopup(true);
  when->setDisplayFormat("dd.MM.yyyy");
  //RR die Eingabe einer NUMMER für die TERTIÄR-Direktionen nach TROINSKY
  auto* nr = new QSpinBox(&dialog);
  nr->setRange(-600, 600);
  nr->setPrefix(tr("Nummer "));
  auto* nr_note = new QLabel(tr("Nummer 0 nimmt das Datum, sonst zählt das n-te Lunar ab Geburt."), &dialog);
  nr_note->setWordWrap(true);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addWidget(note);
  v->addWidget(when);
  v->addWidget(nr);
  v->addWidget(nr_note);
  v->addWidget(buttons);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  SearchContext ctx;
  ctx.base = current_input();
  ctx.settings = current_settings();
  ctx.vsop = &vsop_;
  ctx.eph = &eph_;
  const QDate d = when->date();
  LongitudeCrossing hit;
  QString label;
  if (nr->value() != 0) {
    hit = planetar_return(last_chart_->jd_ut, body::kMoon, last_chart_->b[body::kMoon].el, std::abs(nr->value()),
                          nr->value() > 0, ctx);
    label = QString("LUNAR NR %1").arg(nr->value());
  } else {
    const double before = julian_day({d.day(), d.month(), d.year(), 0, 0.0}, ctx.settings.calendar);
    hit = lunar_return(before, last_chart_->b[body::kMoon].el, ctx);
    label = QString("LUNAR %1").arg(d.toString("dd.MM.yyyy"));
  }
  if (!hit.ok) {
    banner_->set_record(tr("Kein Lunar gefunden"));
    return;
  }
  apply_moment(hit.jd_ut, label);
}

void MainWindow::transit_list() {
  if (!last_chart_) {
    return;
  }
  SearchContext ctx;
  ctx.base = current_input();
  ctx.settings = current_settings();
  ctx.vsop = &vsop_;
  ctx.eph = &eph_;
  TransitListDialog dialog(*last_chart_, ctx, this);
  if (dialog.exec() != QDialog::Accepted || dialog.chosen_jd() <= 0.0) {
    return;
  }
  // the picked event opens in the transit view over the radix
  const CalendarDate d = calendar_date(dialog.chosen_jd(), ctx.settings.calendar);
  int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
  if (seconds >= kSecondsPerDay) {
    seconds = kSecondsPerDay - 1;
  }
  show_transits(QDate(d.year, d.month, d.day), QTime(seconds / 3600, (seconds / 60) % 60, seconds % 60));
  if (transit_on_->isChecked()) {
    recompute();
  }
}

void MainWindow::ingress_table() {
  SearchContext ctx;
  ctx.base = current_input();
  ctx.settings = current_settings();
  ctx.vsop = &vsop_;
  ctx.eph = &eph_;
  IngressDialog dialog(ctx, this);
  if (dialog.exec() != QDialog::Accepted || dialog.chosen_jd() <= 0.0) {
    return;
  }
  const CalendarDate d = calendar_date(dialog.chosen_jd(), ctx.settings.calendar);
  int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
  if (seconds >= kSecondsPerDay) {
    seconds = kSecondsPerDay - 1;
  }
  show_transits(QDate(d.year, d.month, d.day), QTime(seconds / 3600, (seconds / 60) % 60, seconds % 60));
  if (transit_on_->isChecked()) {
    recompute();
  }
}

void MainWindow::combin_chart() {
  const auto r = choose_record(tr("Combin-Datensatz wählen"));
  if (!r || (r->year < 1 && r->jd <= 0.0)) {
    return;
  }
  // the a14 mean of moment and place lands in the panel as one chart
  const ChartInput mixed = combin_input({current_input(), record_input(*r)}, current_settings().calendar);
  const QSignalBlocker b1(lon_);
  const QSignalBlocker b2(lat_);
  lon_->setValue(mixed.lon_deg_east);
  lat_->setValue(mixed.lat_deg);
  QString mine = QString::fromStdString(record_.surname).trimmed();
  if (mine.isEmpty()) {
    mine = "RADIX";
  }
  apply_moment(julian_day(mixed.date_ut, current_settings().calendar),
               QString("COMBIN %1-%2").arg(mine, QString::fromStdString(r->surname).trimmed()));
}

void MainWindow::show_transits(const QDate& date, const QTime& time) {
  const QSignalBlocker b1(tdate_);
  const QSignalBlocker b2(ttime_);
  tdate_->setDate(date);
  ttime_->setTime(time);
  transit_on_->setChecked(true);
}

void MainWindow::pick_zone() {
  ZoneDialog dialog(data_dir_ / "zonnamen.int", this);
  if (!dialog.loaded()) {
    //RR ZEITZONEN-Datei fehlt !
    QMessageBox::warning(this, "HORCOM", tr("Die Datei zonnamen.int fehlt im Datenordner."));
    return;
  }
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const ZoneEntry& z = dialog.chosen();
  const QSignalBlocker b(zone_);
  if (z.is_local_time()) {
    // local time follows from the longitude like in zeitzon_nam_aaf,
    // the true local time refinement before 1810 stays with the operator
    zone_->setValue(lon_->value() / kDegPerHour);
  } else if (z.to_ut_hours) {
    // the catalogue stores the step from zone time to UT, the panel
    // wants hours east
    zone_->setValue(-*z.to_ut_hours);
  }
  recompute();
}

void MainWindow::open_records() {
  if (const auto r = choose_record(tr("Datensatz wählen"))) {
    apply_record(*r);
  }
}

std::optional<AafRecord> MainWindow::choose_record(const QString& title) {
  const QString path = QFileDialog::getOpenFileName(this, tr("Datensätze öffnen"), QString(),
                                                    tr("HORCOM Datensätze (*.DAT *.dat *.AAF *.aaf)"));
  if (path.isEmpty()) {
    return std::nullopt;
  }
  std::vector<AafRecord> records;
  if (path.endsWith(".aaf", Qt::CaseInsensitive)) {
    const auto r = read_aaf(path.toStdWString());
    if (r) {
      records = *r;
    }
  } else {
    const auto r = read_chart_file(path.toStdWString());
    if (r) {
      for (const ChartRecord& c : *r) {
        records.push_back(aaf_from_chart_record(c));
      }
    }
  }
  if (records.empty()) {
    QMessageBox::warning(this, "HORCOM", tr("Keine Datensätze gefunden."));
    return std::nullopt;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(title);
  auto* v = new QVBoxLayout(&dialog);
  auto* list = new QListWidget(&dialog);
  for (const AafRecord& r : records) {
    list->addItem(QString("%1 %2   %3.%4.%5   %6")
                      .arg(QString::fromStdString(r.surname), QString::fromStdString(r.given))
                      .arg(r.day, 2, 10, QChar('0'))
                      .arg(r.month, 2, 10, QChar('0'))
                      .arg(r.year)
                      .arg(QString::fromStdString(r.place)));
  }
  //RR das LÖSCHEN nicht mehr benötigter Datensätze
  list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  auto* erase = buttons->addButton(tr("Löschen"), QDialogButtonBox::ActionRole);
  connect(erase, &QPushButton::clicked, &dialog, [this, &records, list, path]() {
    QList<QListWidgetItem*> sel = list->selectedItems();
    if (sel.isEmpty()) {
      return;
    }
    if (QMessageBox::question(this, "HORCOM", tr("%1 Datensätze aus der Datei löschen?").arg(sel.size())) !=
        QMessageBox::Yes) {
      return;
    }
    std::vector<int> rows;
    for (QListWidgetItem* item : sel) {
      rows.push_back(list->row(item));
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int r : rows) {
      records.erase(records.begin() + r);
      delete list->takeItem(r);
    }
    bool ok = false;
    if (path.endsWith(".aaf", Qt::CaseInsensitive)) {
      ok = write_aaf(std::filesystem::path(path.toStdWString()), records);
    } else {
      std::vector<ChartRecord> out;
      for (const AafRecord& a : records) {
        const ChartInput in = record_input(a);
        ChartRecord c;
        c.day = in.date_ut.day;
        c.month = in.date_ut.month;
        c.year = in.date_ut.year;
        c.hour = in.date_ut.hour;
        c.minute = in.date_ut.minute;
        c.lon = in.lon_deg_east;
        c.lat = in.lat_deg;
        c.name = a.surname.empty() ? a.given : a.surname + " " + a.given;
        c.place = a.place;
        // the remark came through unchanged, the calendar flag included
        c.remark = a.comment;
        out.push_back(std::move(c));
      }
      ok = write_chart_file(std::filesystem::path(path.toStdWString()), out);
    }
    if (!ok) {
      QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht schreiben."));
    }
  });
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  connect(list, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);
  v->addWidget(list, 1);
  v->addWidget(buttons);
  dialog.resize(560, 420);
  if (dialog.exec() == QDialog::Accepted && list->currentRow() >= 0 && !records.empty()) {
    return records[static_cast<std::size_t>(list->currentRow())];
  }
  return std::nullopt;
}

ChartInput MainWindow::record_input(const AafRecord& r) const {
  ChartInput in;
  if (r.jd > 0.0) {
    // the julian date outranks the clock fields like the loader rule
    in.date_ut = calendar_date(r.jd, r.calendar);
  } else {
    const CalendarDate local{r.day, r.month, r.year, static_cast<double>(r.hour),
                             r.minute + r.second / 60.0};
    double zone_hours = 0.0;
    if (r.zone.size() >= 3 && (r.zone.find('E') != std::string::npos || r.zone.find('W') != std::string::npos)) {
      zone_hours = std::atof(r.zone.c_str());
      if (r.zone.find('W') != std::string::npos) {
        zone_hours = -zone_hours;
      }
    }
    in.date_ut = calendar_date(julian_day(local, r.calendar) - zone_hours / 24.0, r.calendar);
  }
  in.lon_deg_east = r.longitude();
  in.lat_deg = r.latitude();
  return in;
}

bool MainWindow::set_partner(const AafRecord& r) {
  if (r.year < 1 && r.jd <= 0.0) {
    return false;
  }
  const ChartInput pin = record_input(r);
  const Chart partner = compute_chart(pin, current_settings(), vsop_, eph_);
  if (!partner.ok) {
    return false;
  }
  partner_chart_ = partner;
  partner_input_ = pin;
  partner_name_ = QString::fromStdString(r.surname).trimmed();
  if (partner_name_.isEmpty()) {
    partner_name_ = QString::fromStdString(r.given).trimmed();
  }
  return true;
}

void MainWindow::show_compare(const AafRecord& partner) {
  if (set_partner(partner)) {
    const QSignalBlocker block(compare_action_);
    compare_action_->setChecked(true);
    recompute();
  }
}

void MainWindow::show_composite(const AafRecord& partner) {
  if (set_partner(partner)) {
    composite_action_->setChecked(true);
  }
}

void MainWindow::show_mundane() {
  mundane_action_->setChecked(true);
}

void MainWindow::show_helio() {
  helio_->setChecked(true);
}

void MainWindow::show_harmonic(int n) {
  harm_n_ = n;
  harm_new_mc_ = false;
  const QSignalBlocker block(harmonic_action_);
  harmonic_action_->setChecked(true);
  recompute();
}

void MainWindow::show_dial(const AafRecord& partner) {
  if (!set_partner(partner)) {
    return;
  }
  const QSignalBlocker b1(compare_action_);
  compare_action_->setChecked(true);
  const QSignalBlocker b2(dial_action_);
  dial_action_->setChecked(true);
  recompute();
}

void MainWindow::show_directions(double jd_event_ut, bool converse) {
  dir_jd_ = jd_event_ut;
  dir_converse_ = converse;
  dir_vary_ = 0.0;
  const QSignalBlocker block(directions_action_);
  directions_action_->setChecked(true);
  recompute();
}

void MainWindow::apply_record(const AafRecord& r) {
  if (r.year < 1) {
    QMessageBox::information(this, "HORCOM",
                             tr("Jahre vor 1 n.Chr. berechnet derzeit nur das Kommandozeilenwerkzeug."));
    return;
  }
  record_ = r;
  const QSignalBlocker b1(date_);
  const QSignalBlocker b2(time_);
  const QSignalBlocker b3(zone_);
  const QSignalBlocker b4(lon_);
  const QSignalBlocker b5(lat_);
  date_->setDate(QDate(r.year, r.month, r.day));
  time_->setTime(QTime(r.hour, r.minute, r.second));
  // zone strings like 01hE00:00 mean the clock is zone time, east leads
  double zone_hours = 0.0;
  if (r.zone.size() >= 3 && (r.zone.find('E') != std::string::npos || r.zone.find('W') != std::string::npos)) {
    zone_hours = std::atof(r.zone.c_str());
    if (r.zone.find('W') != std::string::npos) {
      zone_hours = -zone_hours;
    }
  }
  zone_->setValue(zone_hours);
  lon_->setValue(r.longitude());
  lat_->setValue(r.latitude());
  recompute();
  refresh_record_label();
}

void MainWindow::refresh_record_label() {
  const QDate d = date_->date();
  record_label_ = QString("%1 %2   %3.%4.%5")
                      .arg(QString::fromStdString(record_.surname), QString::fromStdString(record_.given))
                      .arg(d.day(), 2, 10, QChar('0'))
                      .arg(d.month(), 2, 10, QChar('0'))
                      .arg(d.year());
  banner_->set_record(record_label_.trimmed());
}

void MainWindow::open_statistics() {
  const QString path = QFileDialog::getOpenFileName(this, tr("Statistik-Datei öffnen"), QString(),
                                                    tr("HORCOM Statistik (*.STA *.sta)"));
  if (path.isEmpty()) {
    return;
  }
  StatistDialog dialog(aspect_settings_, this);
  if (!dialog.load(path)) {
    QMessageBox::warning(this, "HORCOM", tr("Die Statistik-Datei ließ sich nicht laden, fehlt die .PAR daneben?"));
    return;
  }
  if (dialog.exec() != QDialog::Accepted || !dialog.chosen()) {
    return;
  }
  const StatRecord& r = *dialog.chosen();
  if (r.year < 1) {
    QMessageBox::information(this, "HORCOM",
                             tr("Jahre vor 1 n.Chr. berechnet derzeit nur das Kommandozeilenwerkzeug."));
    return;
  }
  // the store keeps the clock as the original computed it, treated as UT
  record_ = AafRecord{};
  record_.surname = r.name;
  record_.place = r.place;
  const QSignalBlocker b1(date_);
  const QSignalBlocker b2(time_);
  const QSignalBlocker b3(zone_);
  const QSignalBlocker b4(lon_);
  const QSignalBlocker b5(lat_);
  date_->setDate(QDate(r.year, r.month, r.day));
  int seconds = static_cast<int>((r.hour * 60.0 + r.minute) * 60.0 + 0.5);
  if (seconds >= kSecondsPerDay) {
    seconds = kSecondsPerDay - 1;
  }
  time_->setTime(QTime(seconds / 3600, (seconds / 60) % 60, seconds % 60));
  zone_->setValue(0.0);
  lon_->setValue(r.lon);
  lat_->setValue(r.lat);
  recompute();
  refresh_record_label();
}

void MainWindow::open_aspektarium() {
  if (!last_chart_) {
    return;
  }
  AspektariumDialog dialog(*last_chart_, current_settings(), aspect_settings_, record_label_.trimmed(), this);
  dialog.exec();
}

void MainWindow::edit_record() {
  std::vector<GermanCountry> countries;
  if (const auto c = load_german_countries(data_dir_ / "laender.int")) {
    countries = *c;
  }
  RecordDialog dialog(record_, countries, this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  record_ = dialog.record();
  refresh_record_label();
}

void MainWindow::save_aaf() {
  const QString path = QFileDialog::getSaveFileName(this, tr("Als AAF speichern"), "chart.aaf",
                                                    tr("AAF (*.aaf)"), nullptr, QFileDialog::DontConfirmOverwrite);
  if (path.isEmpty()) {
    return;
  }
  // the record carries the person, the panel rules the moment and the
  // coordinates, the clock stays civil with the zone beside it
  AafRecord r = record_;
  const QDate d = date_->date();
  const QTime t = time_->time();
  r.day = d.day();
  r.month = d.month();
  r.year = d.year();
  r.hour = t.hour();
  r.minute = t.minute();
  r.second = t.second();
  r.zone = aaf_zone(zone_->value());
  if (r.dst.empty()) {
    r.dst = "*";
  }
  to_dms(lat_->value(), r.lat_deg, r.lat_min, r.lat_sec);
  r.lat_ns = lat_->value() < 0 ? 'S' : 'N';
  to_dms(lon_->value(), r.lon_deg, r.lon_min, r.lon_sec);
  r.lon_ew = lon_->value() < 0 ? 'W' : 'E';
  r.jd = julian_day(current_input().date_ut);
  std::vector<AafRecord> records{r};
  if (QFile::exists(path)) {
    QMessageBox ask(this);
    ask.setWindowTitle("HORCOM");
    ask.setText(tr("Die Datei gibt es schon. Datensatz an die Sammlung anhängen?"));
    auto* append = ask.addButton(tr("Anhängen"), QMessageBox::AcceptRole);
    ask.addButton(tr("Überschreiben"), QMessageBox::DestructiveRole);
    auto* cancel = ask.addButton(QMessageBox::Cancel);
    ask.exec();
    if (ask.clickedButton() == cancel) {
      return;
    }
    if (ask.clickedButton() == append) {
      if (const auto existing = read_aaf(path.toStdWString())) {
        records = *existing;
        records.push_back(r);
      }
    }
  }
  if (!write_aaf(path.toStdWString(), records)) {
    QMessageBox::warning(this, "HORCOM", tr("Speichern fehlgeschlagen."));
  }
}

void MainWindow::export_svg() {
  if (!last_chart_ || !last_aspects_) {
    return;
  }
  const QString path = QFileDialog::getSaveFileName(this, tr("Horoskop als SVG"), "wheel.svg", tr("SVG (*.svg)"));
  if (path.isEmpty()) {
    return;
  }
  const std::string svg = to_svg(build_wheel(*last_chart_, current_settings(), *last_aspects_));
  QFile f(path);
  if (f.open(QIODevice::WriteOnly)) {
    f.write(svg.data(), static_cast<qint64>(svg.size()));
  }
}

// ported from bes_big, the data sheet of the DINA4 chart print with
// the coordinate block, the houses and the midpoint list
void MainWindow::paint_data_sheet(QPainter& p, const QRectF& page) {
  if (!last_chart_ || !last_aspects_) {
    return;
  }
  const Chart& chart = *last_chart_;
  QFont font = p.font();
  font.setPixelSize(static_cast<int>(page.height() / 52.0));
  p.setFont(font);
  p.setPen(Qt::black);
  const double lh = page.height() / 46.0;
  const double col_w = page.width() / 3.0;
  const auto tag_of = [](int slot) {
    return QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                             static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size()));
  };
  double y = page.top() + lh;
  p.drawText(QPointF(page.left(), y), tr("KOORDINATEN"));
  y += lh;
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid || slot == body::kNodeDesc || (slot >= 15 && slot <= 18)) {
      continue;
    }
    p.drawText(QPointF(page.left(), y),
               QString("%1  %2  %3'/d").arg(slot == 0 ? "sp" : tag_of(slot), -4).arg(zodiac(b.el)).arg(b.tb * kRadToDeg * 60.0, 0, 'f', 1));
    y += lh;
    if (y > page.bottom() - lh) {
      break;
    }
  }
  y = page.top() + lh;
  p.drawText(QPointF(page.left() + col_w, y), tr("HÄUSER"));
  y += lh;
  if (chart.houses.ok) {
    for (int i = 1; i <= 12; ++i) {
      p.drawText(QPointF(page.left() + col_w, y),
                 QString("H%1  %2").arg(i, 2).arg(zodiac(chart.houses.cusp[static_cast<std::size_t>(i)])));
      y += lh;
    }
  }
  //RR die Halbsummenliste der GANZSEITEN-Graphik
  y = page.top() + lh;
  p.drawText(QPointF(page.left() + 2.0 * col_w, y), tr("HALBSUMMEN"));
  y += lh;
  const MidpointResult mid = scan_midpoints(chart, current_settings(), aspect_settings_);
  static constexpr const char* kLevel[9] = {"", "360", "180", "", "90", "", "", "", "45"};
  for (const MidpointHit& h : mid.hits) {
    p.drawText(QPointF(page.left() + 2.0 * col_w, y),
               QString("%1 = %2/%3  %4°").arg(tag_of(h.t), tag_of(h.u), tag_of(h.w), QString(kLevel[h.nh])));
    y += lh;
    if (y > page.bottom() - lh) {
      p.drawText(QPointF(page.left() + 2.0 * col_w, y), QString::fromUtf8("…"));
      break;
    }
  }
}

bool MainWindow::export_pdf_to(const QString& path) {
  const DisplayList& dl = wheel_->display_list();
  if (dl.items.empty()) {
    return false;
  }
  QPdfWriter writer(path);
  writer.setPageSize(QPageSize(QPageSize::A4));
  //RR DIN A4, the wheel canvas lies landscape
  writer.setPageOrientation(QPageLayout::Landscape);
  writer.setResolution(300);
  writer.setTitle("HORCOM");
  QPainter p(&writer);
  if (!p.isActive()) {
    return false;
  }
  paint_fitted(p, dl, QRectF(0, 0, writer.width(), writer.height()));
  if (last_chart_ && last_aspects_) {
    writer.newPage();
    paint_data_sheet(p, QRectF(writer.width() * 0.04, writer.height() * 0.04, writer.width() * 0.92,
                               writer.height() * 0.92));
  }
  p.end();
  return true;
}

void MainWindow::export_pdf() {
  const QString path = QFileDialog::getSaveFileName(this, tr("Horoskop als PDF"), "horoskop.pdf", tr("PDF (*.pdf)"));
  if (path.isEmpty()) {
    return;
  }
  if (!export_pdf_to(path)) {
    QMessageBox::warning(this, "HORCOM", tr("Die PDF-Datei ließ sich nicht schreiben."));
  }
}

void MainWindow::print_chart() {
  const DisplayList& dl = wheel_->display_list();
  if (dl.items.empty()) {
    return;
  }
  QPrinter printer(QPrinter::HighResolution);
  printer.setPageOrientation(QPageLayout::Landscape);
  QPrintDialog dialog(&printer, this);
  //RR AUSGABE auf BILDSCHIRM oder als DRUCKER-GRAPHIK ?
  dialog.setWindowTitle(tr("Horoskop drucken"));
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  QPainter p(&printer);
  if (!p.isActive()) {
    QMessageBox::warning(this, "HORCOM", tr("Der Drucker nahm die Seite nicht an."));
    return;
  }
  paint_fitted(p, dl, QRectF(printer.pageRect(QPrinter::DevicePixel)));
  if (last_chart_ && last_aspects_) {
    printer.newPage();
    const QRectF page(printer.pageRect(QPrinter::DevicePixel));
    paint_data_sheet(p, page.adjusted(page.width() * 0.04, page.height() * 0.04, -page.width() * 0.04,
                                      -page.height() * 0.04));
  }
  p.end();
}

void MainWindow::about() {
  QMessageBox::about(this, tr("Über HORCOM"),
                     tr("<b>horcom</b><br>Die C++ Neufassung von HORCOM,<br>"
                        "geschrieben von Robert Rettig, 1989 bis 2010.<br><br>"
                        "Im Andenken an Robert Rettig, der all dies zuerst gebaut hat.<br><br>"
                        "In C++ neu geschrieben und betreut von Dominik Schwimmbeck.<br>"
                        "GPL-3.0-or-later"));
}

}  // namespace horcom
