// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "main_window.hpp"

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
#include "horcom/chart/harmonics.hpp"
#include "horcom/chart/mundane.hpp"
#include "horcom/chart/progressions.hpp"
#include "horcom/chart/transit_search.hpp"
#include "ingress_dialog.hpp"
#include "kommen_dialog.hpp"
#include "painter.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/core/coords.hpp"
#include "horcom/data/place_file.hpp"
#include "horcom/ephem/precession.hpp"
#include "horcom/time/delta_t.hpp"
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
  bodies_ = new QTableWidget(0, 5, body_dock);
  bodies_->setHorizontalHeaderLabels({tr("Länge"), tr("Breite"), tr("Deklin."), tr("Geschw."), ""});
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
  QMenu* file = menuBar()->addMenu(tr("&Datei"));
  file->addAction(tr("Datensätze öffnen…"), QKeySequence::Open, this, &MainWindow::open_records);
  file->addAction(tr("Datensatz bearbeiten…"), QKeySequence(Qt::CTRL | Qt::Key_D), this, &MainWindow::edit_record);
  file->addAction(tr("Ort suchen…"), QKeySequence(Qt::CTRL | Qt::Key_L), this, &MainWindow::open_place);
  file->addAction(tr("Statistik…"), this, &MainWindow::open_statistics);
  file->addAction(tr("Als AAF speichern…"), QKeySequence::Save, this, &MainWindow::save_aaf);
  file->addAction(tr("Horoskop als SVG…"), this, &MainWindow::export_svg);
  //RR DRUCKER-GRAPHIK, the druck_graph_ein world over one shared painter
  file->addAction(tr("Horoskop als PDF…"), this, &MainWindow::export_pdf);
  file->addAction(tr("Drucken…"), QKeySequence::Print, this, &MainWindow::print_chart);
  file->addAction(tr("Umrechnungen…"), this, &MainWindow::converters);
  file->addSeparator();
  file->addAction(tr("Beenden"), QKeySequence::Quit, this, &QWidget::close);
  // the return charts of his solar and lunar menu
  QMenu* horo = menuBar()->addMenu(tr("&Horoskop"));
  horo->addAction(tr("Solar…"), this, &MainWindow::solar_chart);
  horo->addAction(tr("Lunar…"), this, &MainWindow::lunar_chart);
  horo->addAction(tr("Septar…"), this, &MainWindow::septar_chart);
  horo->addAction(tr("Planetar…"), this, &MainWindow::planetar_chart);
  horo->addAction(tr("Personar…"), this, &MainWindow::personar_chart);
  horo->addAction(tr("Progressions-Horoskop…"), this, &MainWindow::progression_chart);
  horo->addAction(tr("Tages-Horoskop…"), this, &MainWindow::day_chart);
  horo->addAction(tr("Transit-Liste…"), this, &MainWindow::transit_list);
  horo->addAction(tr("Ingresse…"), this, &MainWindow::ingress_table);
  horo->addAction(tr("Aspektarium…"), this, &MainWindow::open_aspektarium);
  horo->addAction(tr("Grad-Liste…"), this, &MainWindow::degree_list);
  horo->addAction(tr("Häuser-Tabelle…"), this, &MainWindow::house_table);
  horo->addAction(tr("Großes Jahr…"), this, &MainWindow::great_year);
  // the direction tables of the original evaluation menu in one place
  horo->addAction(QString::fromUtf8("Direktionen-Auswertung…"), this, [this]() {
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
  directions_action_ = horo->addAction(tr("Direktionen…"));
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
    auto* note = new QLabel(tr("1° STZ entspricht 4 Zeitminuten."), &dialog);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    v->addLayout(form);
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
  horo->addSeparator();
  //RR Solange UHR SICHTBAR wird HOROSKOP ALLE 15 SEK NACHGEZEICHNET !
  clock_action_ = horo->addAction(tr("Uhr"));
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

  QMenu* help = menuBar()->addMenu(tr("&Hilfe"));
  //RR TEXT-DATEI LESEN, his commentary texts from the local folder
  help->addAction(tr("Original-Kommentare…"), this, [this]() {
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
  const AspectResult aspects = scan_aspects(chart, s, aspect_settings_);
  last_chart_ = chart;
  last_aspects_ = aspects;

  // the chart data block for the left margin of the paper, like bes11
  WheelOptions wopt;
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
    wopt.info_lines.push_back(
        QString::asprintf("%02d.%02d.%04d", dd.day, dd.month, dd.year).toStdString());
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
    const std::string_view tag = (helio && slot == body::kMoon) ? body::kTag[0]
                                                                : body::kTag[static_cast<std::size_t>(slot)];
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
      auto* retro = new QTableWidgetItem(b.tb < 0.0 ? "R" : "");
      retro->setForeground(QColor(0xE8, 0x5D, 0x4E));
      bodies_->setItem(row, 4, retro);
    }
  }
  bodies_->setVerticalHeaderLabels(row_names);
  bodies_->resizeColumnsToContents();
  // bes111 lists no cusps in the hrg mode
  for (int i = 1; i <= 12; ++i) {
    cusps_->setItem(i - 1, 0,
                    new QTableWidgetItem(helio ? QString() : zodiac(chart.houses.cusp[static_cast<std::size_t>(i)])));
  }
  aspects_label_->setText(tr("<span style='color:#D4A94A'>ASPEKTE</span>&nbsp; "
                             "konj %1  opp %2  trigon %3  quadrat %4  sextil %5")
                              .arg(aspects.zh[1])
                              .arg(aspects.zh[2])
                              .arg(aspects.zh[3])
                              .arg(aspects.zh[4])
                              .arg(aspects.zh[6]));
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
  apply_moment(hit.jd_ut, QString("%1.%2").arg(count).arg(bodybox->currentText()));
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
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addWidget(note);
  v->addWidget(when);
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
  const double before = julian_day({d.day(), d.month(), d.year(), 0, 0.0}, ctx.settings.calendar);
  const LongitudeCrossing hit = lunar_return(before, last_chart_->b[body::kMoon].el, ctx);
  if (!hit.ok) {
    banner_->set_record(tr("Kein Lunar gefunden"));
    return;
  }
  apply_moment(hit.jd_ut, QString("LUNAR %1").arg(d.toString("dd.MM.yyyy")));
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
        // the DAT clock is already UT
        a.zone = "00hE00:00";
        records.push_back(std::move(a));
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
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  connect(list, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);
  v->addWidget(list, 1);
  v->addWidget(buttons);
  dialog.resize(560, 420);
  if (dialog.exec() == QDialog::Accepted && list->currentRow() >= 0) {
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
  p.end();
}

void MainWindow::about() {
  QMessageBox::about(this, tr("Über HORCOM"),
                     tr("<b>horcom</b><br>Die C++ Neufassung von HORCOM,<br>"
                        "geschrieben von Robert Rettig, 1989 bis 2010.<br><br>"
                        "Im Andenken an Robert Rettig, der all dies zuerst gebaut hat.<br><br>"
                        "GPL-3.0-or-later · betreut von Dominik Schwimmbeck"));
}

}  // namespace horcom
