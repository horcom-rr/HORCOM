// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "main_window.hpp"

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
#include <QPushButton>
#include <QTableWidget>
#include <QTimeEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>
#include <initializer_list>

#include "banner.hpp"
#include "horcom/chart/transit_search.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/place_file.hpp"
#include "horcom/render/svg.hpp"
#include "place_dialog.hpp"
#include "record_dialog.hpp"
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
  file->addAction(tr("Als AAF speichern…"), QKeySequence::Save, this, &MainWindow::save_aaf);
  file->addAction(tr("Horoskop als SVG…"), this, &MainWindow::export_svg);
  file->addSeparator();
  file->addAction(tr("Beenden"), QKeySequence::Quit, this, &QWidget::close);
  // the return charts of his solar and lunar menu
  QMenu* horo = menuBar()->addMenu(tr("&Horoskop"));
  horo->addAction(tr("Solar…"), this, &MainWindow::solar_chart);
  horo->addAction(tr("Lunar…"), this, &MainWindow::lunar_chart);
  horo->addAction(tr("Transit-Liste…"), this, &MainWindow::transit_list);
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
  QMenu* help = menuBar()->addMenu(tr("&Hilfe"));
  help->addAction(tr("Über HORCOM"), this, &MainWindow::about);

  // recompute on every change like the original recalculated per screen
  connect(date_, &QDateEdit::dateChanged, this, &MainWindow::recompute);
  connect(time_, &QTimeEdit::timeChanged, this, &MainWindow::recompute);
  connect(zone_, &QDoubleSpinBox::valueChanged, this, &MainWindow::recompute);
  connect(lon_, &QDoubleSpinBox::valueChanged, this, &MainWindow::recompute);
  connect(lat_, &QDoubleSpinBox::valueChanged, this, &MainWindow::recompute);
  connect(houses_, &QComboBox::currentIndexChanged, this, &MainWindow::recompute);
  for (QCheckBox* box : {parallax_, extras_, true_node_, true_apogee_}) {
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
  const Chart chart = compute_chart(in, s, vsop_, eph_);
  if (!chart.ok) {
    //RR Geog. Breite zu groß !
    banner_->set_record(tr("Geog. Breite zu groß für dieses Häusersystem"));
    return;
  }
  const AspectResult aspects = scan_aspects(chart, s, aspect_settings_);
  last_chart_ = chart;
  last_aspects_ = aspects;
  bool transit_drawn = false;
  if (clock) {
    WheelOptions opt;
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
      WheelOptions opt;
      opt.center_label =
          QString("TRANSIT=>%1 %2 UT").arg(td.toString("dd.MM.yyyy"), tt.toString("HH:mm")).toStdString();
      wheel_->set_display_list(build_transit_wheel(chart, tchart, s, aspects, opt));
      transit_drawn = true;
    }
  }
  if (!transit_drawn) {
    wheel_->set_display_list(build_wheel(chart, s, aspects));
  }
  banner_->set_info(QString("JD(UT) %1   ΔT %2 min   ARMC %3°   %4%5")
                        .arg(chart.jd_ut, 0, 'f', 5)
                        .arg(chart.delt_minutes, 0, 'f', 2)
                        .arg(chart.armc_deg, 0, 'f', 4)
                        .arg(QString::fromUtf8(chart.houses.name.data(), static_cast<int>(chart.houses.name.size())))
                        .arg(s.topocentric_parallax ? "   MitParall." : ""));
  fill_tables(chart, aspects);
}

void MainWindow::fill_tables(const Chart& chart, const AspectResult& aspects) {
  bodies_->setRowCount(0);
  QStringList row_names;
  for (int slot = 0; slot <= 40; ++slot) {
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present) {
      continue;
    }
    const int row = bodies_->rowCount();
    bodies_->insertRow(row);
    row_names << QString::fromUtf8(body::kTag[static_cast<std::size_t>(slot)].data(),
                                   static_cast<int>(body::kTag[static_cast<std::size_t>(slot)].size()));
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
  for (int i = 1; i <= 12; ++i) {
    cusps_->setItem(i - 1, 0, new QTableWidgetItem(zodiac(chart.houses.cusp[static_cast<std::size_t>(i)])));
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
  if (seconds >= 86400) {
    seconds = 86399;
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
  if (seconds >= 86400) {
    seconds = 86399;
  }
  show_transits(QDate(d.year, d.month, d.day), QTime(seconds / 3600, (seconds / 60) % 60, seconds % 60));
  if (transit_on_->isChecked()) {
    recompute();
  }
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
  const QString path = QFileDialog::getOpenFileName(this, tr("Datensätze öffnen"), QString(),
                                                    tr("HORCOM Datensätze (*.DAT *.dat *.AAF *.aaf)"));
  if (path.isEmpty()) {
    return;
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
    return;
  }
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Datensatz wählen"));
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
    apply_record(records[static_cast<std::size_t>(list->currentRow())]);
  }
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

void MainWindow::about() {
  QMessageBox::about(this, tr("Über HORCOM"),
                     tr("<b>horcom</b><br>Die C++ Neufassung von HORCOM,<br>"
                        "geschrieben von Robert Rettig, 1970er bis 2010.<br><br>"
                        "Im Andenken an Robert Rettig, der all dies zuerst gebaut hat.<br><br>"
                        "GPL-3.0-or-later · betreut von Dominik Schwimmbeck"));
}

}  // namespace horcom
