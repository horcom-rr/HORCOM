// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "main_window.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QScreen>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFile>
#include <QColorDialog>
#include <QStatusBar>
#include <QFileDialog>
#include <QFontInfo>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QIntValidator>
#include <QKeyEvent>
#include <QDoubleValidator>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QMenuBar>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPrintDialog>
#include <QPrinter>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QScrollArea>
#include <QSettings>
#include <QTableWidget>
#include <QTimeEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <utility>
#include <cmath>
#include <functional>
#include <initializer_list>

#include "aspektarium_dialog.hpp"
#include "auto_advance.hpp"
#include "banner.hpp"
#include "direction_list_dialog.hpp"
#include "horcom/chart/composite.hpp"
#include "horcom/data/statist_eval.hpp"
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
#include "horcom/chart/great_year.hpp"
#include "horcom/chart/houses.hpp"
#include "horcom/ephem/sunmoon.hpp"
#include "horcom/chart/histogram.hpp"
#include "horcom/chart/signs.hpp"
#include "horcom/data/statist.hpp"
#include "horcom/chart/degree_list.hpp"
#include "horcom/render/aspektarium.hpp"
#include "horcom/render/grad_sheet.hpp"
#include "horcom/render/midpoint_trees.hpp"
#include "horcom/render/linear.hpp"
#include "horcom/render/pair_sheet.hpp"
#include "horcom/render/svg.hpp"
#include "aaf_mask_dialog.hpp"
#include "calendar_mark.hpp"
#include "choice_dialog.hpp"
#include "dock_title.hpp"
#include "event_filter.hpp"
#include "horcom/data/record_order.hpp"
#include "paging_keys.hpp"
#include "place_dialog.hpp"
#include "place_hub.hpp"
#include "horcom/data/collection.hpp"
#include "horcom/data/file_io.hpp"
#include "record_list_dialog.hpp"
#include "record_mask_dialog.hpp"
#include "robert_input.hpp"
#include "robert_text.hpp"
#include "table_fit.hpp"
#include "zodiac_cells.hpp"
#include "theme.hpp"
#include "transit_list_dialog.hpp"
#include "wheel_widget.hpp"
#include "zone_dialog.hpp"

namespace horcom {

namespace {

// calls back when the watched widget changes its size
class ResizeHook final : public QObject {
 public:
  ResizeHook(std::function<void()> on_resize, QObject* parent) : QObject(parent), on_resize_(std::move(on_resize)) {}

 protected:
  bool eventFilter(QObject*, QEvent* e) override {
    if (e->type() == QEvent::Resize) {
      on_resize_();
    }
    return false;
  }

 private:
  std::function<void()> on_resize_;
};

// the object name of the HINTERGRUND-FARBEN entry of the FARBEN menu
constexpr const char* kOwnColorsAction = "ownColorsAction";

}  // namespace

namespace {

// the sign tags of the original zei$ table

//RR Bei Uhr alle 15 sek neu
// his acmcl strip refreshes every second
constexpr int kClockStripMs = 1000;

// a burst of edits settles into one history step after this pause
constexpr int kHistorySettleMs = 800;
// the panel history keeps this many steps
constexpr std::size_t kHistoryDepth = 200;


// the fractional variant, seconds carry the double's precision so the
// panel does not throw away the accuracy the place file already knows
void to_dms_frac(double value, int& deg, int& min, double& sec) {
  const double a = std::abs(value);
  deg = static_cast<int>(a);
  const double rem = (a - deg) * 60.0;
  min = static_cast<int>(rem);
  sec = (rem - min) * 60.0;
  // guard the sexagesimal rollover a floating point round could produce
  if (sec >= 60.0) {
    sec -= 60.0;
    ++min;
  }
  if (min >= 60) {
    min -= 60;
    ++deg;
  }
}

// Robert Rettig's coordinate rows use four small boxes ° ' " plus a
// hemisphere letter, the record mask carries the same idiom. The
// seconds field takes decimals so the panel keeps the sub-arcsecond
// precision the double coordinate carries, integer seconds would drop
// the last ~30 m of accuracy the place file already knows
QLineEdit* dms_box(QWidget* parent, int chars, int max_value, bool decimals = false) {
  auto* e = new QLineEdit(parent);
  e->setAlignment(Qt::AlignRight);
  e->setMaxLength(chars);
  e->setFixedWidth(20 + 10 * chars);
  if (decimals) {
    auto* v = new QDoubleValidator(0.0, static_cast<double>(max_value + 1), 3, e);
    v->setNotation(QDoubleValidator::StandardNotation);
    v->setLocale(QLocale::c());
    e->setValidator(v);
  } else if (max_value > 0) {
    e->setValidator(new QIntValidator(0, max_value, e));
  }
  return e;
}

QString degs(double rad) {
  return QString::asprintf("%+9.4f", rad * kRadToDeg);
}

// QDate knows no year zero, the astronomical count of the original
// does, jaa <= 0 stored years before Christ as 1 - historical year
int qdate_year(int astro) {
  return astro <= 0 ? astro - 1 : astro;
}

// the comparison list body, running body, separation, radix body
// the running body leads each entry, a12asp gives it the colour of the
// outer symbols under drgrph!, outer_css empty keeps it plain
QString cross_hits_text(const std::vector<CrossAspectHit>& hits, const QString& outer_css = {}) {
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
    QString running = QString::fromUtf8(body::kName[static_cast<std::size_t>(h.w)].data(),
                                        static_cast<int>(body::kName[static_cast<std::size_t>(h.w)].size()));
    if (!outer_css.isEmpty()) {
      running = "<span style=\"color:" + outer_css + "\">" + running + "</span>";
    }
    out += QString("%1 %2° %3")
               .arg(running)
               .arg(qRound(h.sep_deg))
               .arg(QString::fromUtf8(body::kName[static_cast<std::size_t>(h.t)].data(),
                                      static_cast<int>(body::kName[static_cast<std::size_t>(h.t)].size())));
    ++shown;
  }
  return out;
}

// the folder of the AAF files, his aafpth$. The twin folder of the
// working Daten-Datei comes first, then an AAFDATEN folder of the data,
// then the data folder itself
std::filesystem::path aaf_folder(const QString& data_file, const std::filesystem::path& data_dir) {
  if (!data_file.isEmpty()) {
    return aaf_twin_path(std::filesystem::path(data_file.toStdWString())).parent_path();
  }
  const std::filesystem::path dir = find_case_blind(data_dir, "aafdaten");
  std::error_code ec;
  return std::filesystem::is_directory(dir, ec) ? dir : data_dir;
}

// where his FILESELECT over SPEZIAL\*.DAT opens, the working file or a
// SPEZIAL folder of the data, never a file name that is no folder
QString collection_start(const QString& data_file, const std::filesystem::path& data_dir) {
  if (!data_file.isEmpty()) {
    return data_file;
  }
  const std::filesystem::path dir = find_case_blind(data_dir, "spezial");
  std::error_code ec;
  return QString::fromStdWString((std::filesystem::is_directory(dir, ec) ? dir : data_dir).wstring());
}

// a name typed without extension gets the one of the file kind
QString with_suffix(QString path, const QString& suffix) {
  if (!path.isEmpty() && QFileInfo(path).suffix().isEmpty()) {
    path += "." + suffix;
  }
  return path;
}

// surname and given name in capitals for the SATZ rows and buttons
QString slot_name(const AafRecord& r) {
  return QString::fromStdString(record_name(r)).toUpper();
}

// the numbered corner rows of the paired sheets, ported from a13aus.
//RR LEFT$(na$(oo,zz),20), his name cut keeps the row short of the wheel
constexpr int kPairNameLength = 20;

std::string pair_name_row(int nr, QString who, const QString& fallback) {
  who = who.trimmed();
  if (who.isEmpty()) {
    who = fallback;
  }
  return QString("%1: %2").arg(nr).arg(who.left(kPairNameLength)).toStdString();
}

std::string pair_moment_row(int nr, const CalendarDate& d) {
  int s = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
  if (s >= kSecondsPerDay) {
    s = kSecondsPerDay - 1;
  }
  return QString::asprintf("%d: %02d.%02d.%d  UT %02d:%02d:%02d", nr, d.day, d.month, d.year,
                           s / 3600, (s / 60) % 60, s % 60)
      .toStdString();
}

// the house mode of a13 from comp_mstz and comp_hand
CompositeHouses composite_mode(const Konsta& k, HouseSystem sys) {
  //RR IF haw& = 6 OR haw& = 7 : CLR comp_hand!,comp_mstz!
  if (sys == HouseSystem::kEqualAsc || sys == HouseSystem::kEqualVehlow) {
    return CompositeHouses::kSchematic;
  }
  if (k.comp_mstz) {
    return CompositeHouses::kMeanSidereal;
  }
  return k.comp_hand ? CompositeHouses::kRobertHand : CompositeHouses::kSchematic;
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
  konsta_file_ = data_dir_ / "konsta.int";
  // his tabstop& = 0, AUTOMATISCH WEITERSCHALTEN
  set_auto_advance(konsta_.tabstop == 0);
  aspect_settings_ = konsta_.aspect_settings();
  // horm&, the BEZUGS-SYSTEM of the profile
  mundane_frame_ = konsta_.horm == 2;
  // the colours of HINTERGRUND-FARBEN once chosen, the dress otherwise
  if (QSettings().value(theme::kOwnColorsKey, false).toBool() && qApp != nullptr) {
    theme::set_own_colors(theme::dialog_color(konsta_.col_dial), theme::passive_color(konsta_.col_backg));
  }
  build_ui();
  recompute();
  // ALLES ZURÜCKSETZEN starts a fresh program and closes this one, an
  // AppImage through its image file since its mount goes with this process
  restart_ = [this]() {
    const QString image = qEnvironmentVariable("APPIMAGE");
    const QString program = image.isEmpty() ? QCoreApplication::applicationFilePath() : image;
    if (!QProcess::startDetached(program, {})) {
      QMessageBox::information(this, "HORCOM", tr("Bitte HORCOM neu starten."));
    }
    close();
  };
  // his function keys reach the outputs too, the filter sits on the
  // application
  if (qApp != nullptr) {
    qApp->installEventFilter(this);
  }
}

void MainWindow::build_ui() {
  // the version stamped by CI (HORCOM_VERSION) rides in the title beside
  // the name so a screenshot always says which build shot it
#ifdef HORCOM_VERSION_STRING
  const QString version = QStringLiteral(HORCOM_VERSION_STRING);
  setWindowTitle(version.isEmpty() ? QStringLiteral("HORCOM")
                                   : QStringLiteral("HORCOM  %1").arg(version));
#else
  setWindowTitle("HORCOM");
#endif
  // his hard&, the colour of the outer symbols lives in KONSTA only like
  // his param_sp kept it. His final KONSTA7P.INT says 1, so a fresh start
  // paints them red like his running program
  outer_color_ = konsta_.hard;
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
  // his IF moda& = 1 && MOUSEK = 2, einzel_plan_wahl on the HOROSKOP-GRAPHIK.
  // The button stays with it, no context menu event climbs on to the
  // Erste Hilfe of the main screen
  wheel_->setContextMenuPolicy(Qt::PreventContextMenu);
  connect(wheel_, &WheelWidget::right_clicked, this, [this]() {
    if (plain_view_ && !uhr_on_) {
      (void)single_planet_choice(3);
    }
  });

  // the input panel
  auto* input_dock = new QDockWidget(tr("Eingabe"), this);
  input_dock->setFeatures(QDockWidget::DockWidgetMovable);
  dress_dock_title(input_dock);
  auto* form_host = new QWidget(input_dock);
  auto* form = new QFormLayout(form_host);
  // Zurück walks to the previous step, every settled panel change one
  // step, Vor brings it back, so Zurück doubles as the undo key
  back_action_ = new QAction(tr("◀ Zurück"), this);
  back_action_->setShortcuts({QKeySequence::Back, QKeySequence::Undo});
  back_action_->setToolTip(tr("Zum vorhergehenden Schritt (Alt+Links oder Strg+Z)"));
  connect(back_action_, &QAction::triggered, this, &MainWindow::history_back);
  addAction(back_action_);
  forward_action_ = new QAction(tr("Vor ▶"), this);
  forward_action_->setShortcuts({QKeySequence::Forward, QKeySequence::Redo});
  forward_action_->setToolTip(tr("Schritt wiederherstellen (Alt+Rechts oder Strg+Y)"));
  connect(forward_action_, &QAction::triggered, this, &MainWindow::history_forward);
  addAction(forward_action_);
  auto* nav_row = new QHBoxLayout();
  auto* back_button = new QToolButton(form_host);
  back_button->setDefaultAction(back_action_);
  auto* forward_button = new QToolButton(form_host);
  forward_button->setDefaultAction(forward_action_);
  nav_row->addWidget(back_button);
  nav_row->addWidget(forward_button);
  nav_row->addStretch(1);
  form->addRow(nav_row);
  //RR Daten-Datei
  // the bound collection of his main screen with its record count beside
  // the drive column
  data_file_label_ = new QLabel(form_host);
  data_file_label_->setWordWrap(true);
  form->addRow(tr("Daten-Datei"), data_file_label_);
  refresh_data_file_label();
  history_timer_ = new QTimer(this);
  history_timer_->setSingleShot(true);
  history_timer_->setInterval(kHistorySettleMs);
  connect(history_timer_, &QTimer::timeout, this, &MainWindow::flush_history);
  update_history_actions();
  // the person stands first like on his parameter screen, edits land
  // in the record and on the sheet
  given_ = new QLineEdit(form_host);
  surname_ = new QLineEdit(form_host);
  form->addRow(tr("Vorname"), given_);
  form->addRow(tr("Name"), surname_);
  // the place name of the record, the same field the sheet corner
  // carries. ORTS-DATEIEN / ORT SUCHEN fills it too, edits here reach
  // the sheet on the next recompute. The picker beside it opens the
  // same Orts-Dateien dialog the EIN-AUSG menu carries, one click into
  // the catalogue from the panel
  // the place-name field is built here but added to the form further
  // down, between Zeit and Zeit-Zone where the place decides the zone
  place_field_ = new QLineEdit(form_host);
  auto* place_row = new QWidget(form_host);
  auto* place_lay = new QHBoxLayout(place_row);
  place_lay->setContentsMargins(0, 0, 0, 0);
  place_lay->setSpacing(4);
  place_lay->addWidget(place_field_, 1);
  auto* place_pick = new QToolButton(place_row);
  place_pick->setText("…");
  place_pick->setToolTip(tr("Orts-Dateien"));
  place_lay->addWidget(place_pick);
  connect(place_pick, &QToolButton::clicked, this, &MainWindow::open_place);
  // the original entered dates as plain TT MM JJJJ fields and a
  // calendar widget cannot hold years before Christ, so the date is a
  // text field, TT.MM.JJJJ, years BC with the vC of his chooser list
  date_ = new QLineEdit("13.10.1992", form_host);
  date_->setMaxLength(14);
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
  // the geographic longitude and latitude wear his degree, minute and
  // second boxes again like the EINGABE box, the file and calculation
  // layers behind them keep speaking decimal degrees
  // the double coordinate stays IEEE double, decimals ten mean the
  // spin box does not round the value away, sub-arcsecond precision
  // from the place file rides all the way to the DMS boxes and back
  lon_ = new QDoubleSpinBox(form_host);
  lon_->setRange(-180.0, 180.0);
  lon_->setDecimals(10);
  lon_->setValue(11.3244);
  lon_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  lon_->setVisible(false);
  lat_ = new QDoubleSpinBox(form_host);
  lat_->setRange(-89.99, 89.99);
  lat_->setDecimals(10);
  lat_->setValue(48.1742);
  lat_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  lat_->setVisible(false);
  // Länge visible row, deg ° min ' sec " + O/W letter
  auto* lon_row = new QWidget(form_host);
  auto* lon_lay = new QHBoxLayout(lon_row);
  lon_lay->setContentsMargins(0, 0, 0, 0);
  lon_lay->setSpacing(2);
  auto* lon_deg = dms_box(lon_row, 3, 180);
  auto* lon_min = dms_box(lon_row, 2, 59);
  // the seconds field takes decimals so the panel does not lose the
  // sub-arcsecond precision the place file carries as a double
  auto* lon_sec = dms_box(lon_row, 6, 59, true);
  auto* lon_hemi = dms_box(lon_row, 1, 0);
  lon_lay->addWidget(lon_deg);
  lon_lay->addWidget(new QLabel(QStringLiteral("°"), lon_row));
  lon_lay->addWidget(lon_min);
  lon_lay->addWidget(new QLabel(QStringLiteral("'"), lon_row));
  lon_lay->addWidget(lon_sec);
  lon_lay->addWidget(new QLabel(QStringLiteral("\""), lon_row));
  lon_lay->addSpacing(6);
  lon_lay->addWidget(lon_hemi);
  lon_lay->addStretch(1);
  auto* lat_row = new QWidget(form_host);
  auto* lat_lay = new QHBoxLayout(lat_row);
  lat_lay->setContentsMargins(0, 0, 0, 0);
  lat_lay->setSpacing(2);
  auto* lat_deg = dms_box(lat_row, 2, 89);
  auto* lat_min = dms_box(lat_row, 2, 59);
  auto* lat_sec = dms_box(lat_row, 6, 59, true);
  auto* lat_hemi = dms_box(lat_row, 1, 0);
  lat_lay->addWidget(lat_deg);
  lat_lay->addWidget(new QLabel(QStringLiteral("°"), lat_row));
  lat_lay->addWidget(lat_min);
  lat_lay->addWidget(new QLabel(QStringLiteral("'"), lat_row));
  lat_lay->addWidget(lat_sec);
  lat_lay->addWidget(new QLabel(QStringLiteral("\""), lat_row));
  lat_lay->addSpacing(6);
  lat_lay->addWidget(lat_hemi);
  lat_lay->addStretch(1);
  // pull the current value from the hidden spinbox and write it back
  // whenever a DMS box loses focus, the spinbox valueChanged signal then
  // fires the recompute connect set up further down. The seconds text
  // carries three decimals so the double precision from the place file
  // survives the round trip through the panel
  auto format_sec = [](double s) {
    QString t = QString::number(s, 'f', 3);
    // trim trailing zeros so a whole arc-second reads plain
    while (t.contains(QLatin1Char('.')) && (t.endsWith(QLatin1Char('0')) || t.endsWith(QLatin1Char('.')))) {
      t.chop(1);
    }
    return t;
  };
  auto refresh_lon = [lon_deg, lon_min, lon_sec, lon_hemi, format_sec, this]() {
    int d = 0, m = 0;
    double s = 0.0;
    to_dms_frac(lon_->value(), d, m, s);
    lon_deg->setText(QString::number(d));
    lon_min->setText(QString::number(m));
    lon_sec->setText(format_sec(s));
    lon_hemi->setText(lon_->value() < 0.0 ? QStringLiteral("W") : QStringLiteral("E"));
  };
  auto refresh_lat = [lat_deg, lat_min, lat_sec, lat_hemi, format_sec, this]() {
    int d = 0, m = 0;
    double s = 0.0;
    to_dms_frac(lat_->value(), d, m, s);
    lat_deg->setText(QString::number(d));
    lat_min->setText(QString::number(m));
    lat_sec->setText(format_sec(s));
    lat_hemi->setText(lat_->value() < 0.0 ? QStringLiteral("S") : QStringLiteral("N"));
  };
  auto commit_lon = [lon_deg, lon_min, lon_sec, lon_hemi, this]() {
    const double v = lon_deg->text().toInt() + lon_min->text().toInt() / 60.0 +
                     lon_sec->text().toDouble() / 3600.0;
    const QString h = lon_hemi->text().trimmed().toUpper();
    lon_->setValue(h == "W" ? -v : v);
  };
  auto commit_lat = [lat_deg, lat_min, lat_sec, lat_hemi, this]() {
    const double v = lat_deg->text().toInt() + lat_min->text().toInt() / 60.0 +
                     lat_sec->text().toDouble() / 3600.0;
    const QString h = lat_hemi->text().trimmed().toUpper();
    lat_->setValue(h == "S" ? -v : v);
  };
  for (auto* box : {lon_deg, lon_min, lon_sec, lon_hemi}) {
    connect(box, &QLineEdit::editingFinished, this, commit_lon);
  }
  for (auto* box : {lat_deg, lat_min, lat_sec, lat_hemi}) {
    connect(box, &QLineEdit::editingFinished, this, commit_lat);
  }
  connect(lon_, &QDoubleSpinBox::valueChanged, this, refresh_lon);
  connect(lat_, &QDoubleSpinBox::valueChanged, this, refresh_lat);
  // store the refresh entry points so callers that setValue under a
  // signal blocker (apply_record, restore_state, open_place) can still
  // pull the DMS boxes back in sync
  refresh_lon_dms_ = refresh_lon;
  refresh_lat_dms_ = refresh_lat;
  refresh_lon();
  refresh_lat();
  houses_ = new QComboBox(form_host);
  // his menu order in hausw
  houses_->addItems({"PLACIDUS", "TOPOZENTRISCH", "KOCH-GOH", "REGIOMONTANUS", "CAMPANUS",
                     "ÄQUAL EKLIPTIKAL ab AC", "ÄQUAL n. VEHLOW", "NUR AC und MC", "KEINE",
                     "KEINE, OHNE MONDKNOTEN"});
  parallax_ = new QCheckBox(tr("Parallaxe (topozentrisch)"), form_host);
  // off at startup like the original klpl!, the real extras stay inline
  // beside their label so the panel wears his three letters. The
  // Hamburg factors and every other body group
  // live behind Andere Elemente and reach the wheel through the
  // Planeten-Auswahl dialog like his ANSICHT screen
  // the extra planets CH QU XE of the original, the plain name choice of
  // his own final defaults
  extras_ = new QCheckBox(tr("Zusatz-Planeten CH QU XE"), form_host);
  // the panel shortcut mirrors the CH QU XE flag in the per slot list
  // so the Planeten-Auswahl dialog shows the same three checked
  connect(extras_, &QCheckBox::toggled, this, [this](bool on) {
    included_[body::kChiron] = on;
    included_[body::kQuaoar] = on;
    included_[body::kXena] = on;
  });
  hamburg_ = new QCheckBox(tr("Hamburger Planeten"), form_host);
  hamburg_->setVisible(false);
  // the hidden Hamburger switch fans the eight factors when preset_extras
  // asks for them, so the wheel and the Planeten-Auswahl agree
  connect(hamburg_, &QCheckBox::toggled, this, [this](bool on) {
    for (int slot : {body::kCupido, body::kHades, body::kZeus, body::kKronos, body::kApollon, body::kAdmetos,
                     body::kVulkanus, body::kPoseidon}) {
      included_[static_cast<std::size_t>(slot)] = on;
    }
  });
  // the Mondknoten row, DR and DS true as the family reads it
  node_show_ = new QCheckBox(tr("Mondknoten"), form_host);
  node_show_->setChecked(true);
  auto* node_wahr = new QRadioButton(tr("Wahrer"), form_host);
  auto* node_mittel = new QRadioButton(tr("Mittlerer"), form_host);
  // the Schwarzer Mond row, built like the Mondknoten row
  apogee_show_ = new QCheckBox(tr("Schwarzer Mond"), form_host);
  // the panel switch owns AG's inclusion, the group slot mirrors it so the
  // Planeten-Auswahl dialog shows AG checked when Schwarzer Mond is on
  connect(apogee_show_, &QCheckBox::toggled, this,
          [this](bool on) { included_[body::kApogee] = on; });
  auto* apogee_wahr = new QRadioButton(tr("Wahrer"), form_host);
  auto* apogee_mittel = new QRadioButton(tr("Mittlerer"), form_host);
  true_node_ = new QCheckBox(form_host);
  true_node_->setVisible(false);
  true_apogee_ = new QCheckBox(form_host);
  true_apogee_->setVisible(false);
  // radio buttons follow the two hidden checkboxes so the existing
  // logic that reads true_node_->isChecked() and true_apogee_ stays
  auto sync_node_radios = [node_wahr, node_mittel, this]() {
    QSignalBlocker b1(node_wahr), b2(node_mittel);
    const bool w = true_node_->isChecked();
    node_wahr->setChecked(w);
    node_mittel->setChecked(!w);
  };
  auto sync_apogee_radios = [apogee_wahr, apogee_mittel, this]() {
    QSignalBlocker b1(apogee_wahr), b2(apogee_mittel);
    const bool w = true_apogee_->isChecked();
    apogee_wahr->setChecked(w);
    apogee_mittel->setChecked(!w);
  };
  connect(node_wahr, &QRadioButton::toggled, this, [this](bool on) { true_node_->setChecked(on); });
  connect(apogee_wahr, &QRadioButton::toggled, this, [this](bool on) { true_apogee_->setChecked(on); });
  connect(true_node_, &QCheckBox::toggled, this, sync_node_radios);
  connect(true_apogee_, &QCheckBox::toggled, this, sync_apogee_radios);
  sync_lunar_radios_ = [sync_node_radios, sync_apogee_radios]() {
    sync_node_radios();
    sync_apogee_radios();
  };
  //RR HELIOZENTRISCH
  helio_ = new QCheckBox(tr("Heliozentrisch"), form_host);
  //RR SOMMERZEIT
  // one hour added to the zone, the value in force stands on its right
  // like in his summer time box
  sommerzeit_ = new QCheckBox(tr("Sommerzeit"), form_host);
  //RR DOPPELTE SOMMERZEIT = DDSZ
  // the second switch of his zone box
  double_dst_ = new QCheckBox(tr("doppelt"), form_host);
  double_dst_->setToolTip(tr("DOPPELTE SOMMERZEIT = DDSZ, zwei Stunden Zuschlag"));
  double_dst_->setEnabled(false);
  auto* sommer_row = new QWidget(form_host);
  auto* sommer_lay = new QHBoxLayout(sommer_row);
  sommer_lay->setContentsMargins(0, 0, 0, 0);
  sommer_lay->setSpacing(8);
  sommer_lay->addWidget(sommerzeit_);
  sommer_lay->addWidget(double_dst_);
  auto* sommer_effect = new QLabel(sommer_row);
  sommer_effect->setToolTip(tr("wirksame Zeit-Zone bei aktivem Sommerzeit-Zuschlag"));
  sommer_lay->addWidget(sommer_effect);
  sommer_lay->addStretch(1);
  auto refresh_sommer = [this, sommer_effect]() {
    if (clock_kind_ != ClockKind::kZone) {
      // LMT = MOZ and LTT = WOZ, the labels of his zeit_discr
      sommer_effect->setText(clock_kind_ == ClockKind::kTrueLocal ? tr("(Ortszeit LTT = WOZ)")
                                                                  : tr("(Ortszeit LMT = MOZ)"));
    } else if (dst_hours_ > 0.0) {
      const double eff = zone_->value() + dst_hours_;
      sommer_effect->setText(QString("(%1%2 h)").arg(eff >= 0.0 ? "+" : "").arg(eff, 0, 'g', 3));
    } else {
      sommer_effect->clear();
    }
  };
  connect(sommerzeit_, &QCheckBox::toggled, this, [this](bool on) {
    set_dst(on ? (double_dst_->isChecked() ? 2.0 : 1.0) : 0.0);
  });
  connect(double_dst_, &QCheckBox::toggled, this, [this](bool on) {
    if (sommerzeit_->isChecked()) {
      set_dst(on ? 2.0 : 1.0);
    }
  });
  connect(zone_, &QDoubleSpinBox::valueChanged, this, refresh_sommer);
  //RR ORTSZEIT (HISTORISCHE HOROSKOPE)
  //RR NOCH JULIANISCH ( NACH 1582 )
  // the two switches of his EINGABE box
  local_time_ = new QCheckBox(tr("Ortszeit"), form_host);
  local_time_->setToolTip(tr("ORTSZEIT (HISTORISCHE HOROSKOPE), die Uhr folgt der geographischen Länge"));
  julian_ = new QCheckBox(tr("noch julianisch"), form_host);
  julian_->setToolTip(tr("NOCH JULIANISCH ( NACH 1582 ), das Datum gilt im julianischen Kalender"));
  auto* history_row = new QWidget(form_host);
  auto* history_lay = new QHBoxLayout(history_row);
  history_lay->setContentsMargins(0, 0, 0, 0);
  history_lay->setSpacing(8);
  history_lay->addWidget(local_time_);
  history_lay->addWidget(julian_);
  history_lay->addStretch(1);
  connect(local_time_, &QCheckBox::toggled, this, [this](bool on) { set_local_time(on, true); });
  connect(julian_, &QCheckBox::toggled, this, [this](bool on) {
    set_panel_calendar(on ? Calendar::kJulian : Calendar::kAuto);
    // his ortsz!(ze) = TRUE, NOCH JULIANISCH brings in the local time too
    if (on && !local_time_->isChecked()) {
      set_local_time(true, true);
    }
  });
  // a new longitude moves the shown local zone along
  connect(lon_, &QDoubleSpinBox::valueChanged, this, [this](double lon) {
    if (clock_kind_ != ClockKind::kZone) {
      const QSignalBlocker b(zone_);
      zone_->setValue(lon / kDegPerHour);
    }
  });
  // callers that setChecked or setValue under a QSignalBlocker (restore_state,
  // apply_record) run the label through this entry point so the hint stays
  // in step with the two fields that feed it
  refresh_sommer_effect_ = refresh_sommer;
  form->addRow(tr("Datum"), date_);
  // the birth time reads as local clock, the zone plus Sommerzeit
  // convert it to UT the moment the recompute runs
  form->addRow(tr("Zeit"), time_);
  // Ortsname sits between the clock time and the zone, the ORT SUCHEN
  // picker fills the zone and the coordinates below
  form->addRow(tr("Ortsname"), place_row);
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
  form->addRow(tr("Zeit-Zone"), zone_row);
  form->addRow(sommer_row);
  form->addRow(history_row);
  // the hidden lon_/lat_ spinboxes stay off the form, the DMS rows carry
  // the visible entry and mirror the double back into them
  form->addRow(tr("Länge (Ost +)"), lon_row);
  form->addRow(tr("Breite (Nord +)"), lat_row);
  form->addRow(tr("Häuser"), houses_);
  form->addRow(parallax_);
  form->addRow(extras_);
  // Mondknoten row, the two radio buttons ride beside the checkbox so
  // the panel shows the choice inline
  auto* node_row = new QWidget(form_host);
  auto* node_lay = new QHBoxLayout(node_row);
  node_lay->setContentsMargins(0, 0, 0, 0);
  node_lay->setSpacing(8);
  node_lay->addWidget(node_show_);
  node_lay->addWidget(node_wahr);
  node_lay->addWidget(node_mittel);
  node_lay->addStretch(1);
  form->addRow(node_row);
  // Schwarzer Mond row, same layout as the Mondknoten row
  auto* apogee_row = new QWidget(form_host);
  auto* apogee_lay = new QHBoxLayout(apogee_row);
  apogee_lay->setContentsMargins(0, 0, 0, 0);
  apogee_lay->setSpacing(8);
  apogee_lay->addWidget(apogee_show_);
  apogee_lay->addWidget(apogee_wahr);
  apogee_lay->addWidget(apogee_mittel);
  apogee_lay->addStretch(1);
  form->addRow(apogee_row);
  // Andere Elemente row, the picker opens the Planeten-Auswahl dialog,
  // the same screen the ANSICHT menu carries. From there the Asteroiden,
  // the Planetoiden and the Hamburger Faktoren reach the wheel
  auto* extra_row = new QWidget(form_host);
  auto* extra_lay = new QHBoxLayout(extra_row);
  extra_lay->setContentsMargins(0, 0, 0, 0);
  extra_lay->setSpacing(4);
  auto* extra_pick = new QToolButton(extra_row);
  extra_pick->setText("…");
  extra_pick->setToolTip(tr("Planeten-Auswahl"));
  extra_lay->addWidget(extra_pick);
  extra_lay->addStretch(1);
  connect(extra_pick, &QToolButton::clicked, this, &MainWindow::planet_selection);
  form->addRow(tr("Andere Elemente"), extra_row);
  form->addRow(helio_);
  sync_node_radios();
  sync_apogee_radios();
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
  // a konsta file may preselect extras through its nk table, the panel
  // shortcut Zusatz-Planeten covers CH QU XE like his own final profile
  // and stays on by default, the three belong to the everyday reading of
  // a chart. Every other preset body lands in the per slot list
  extras_->setChecked(true);
  for (int i = 2; i <= 22; ++i) {
    if (preset.nk[static_cast<std::size_t>(i)] > 0) {
      included_[static_cast<std::size_t>(extra_slot(i))] = true;
    }
  }
  bool hamburg_on = false;
  for (int i = 9; i <= 16; ++i) {
    hamburg_on = hamburg_on || preset.nk[static_cast<std::size_t>(i)] > 0;
  }
  hamburg_->setChecked(hamburg_on);
  apogee_show_->setChecked(preset.nk[1] > 0);
  // the preferred place of the original ORT.EXT seeds the coordinates
  if (const auto home = read_preferred_place(data_dir_ / "ort.ext")) {
    lon_->setValue(home->lon);
    lat_->setValue(home->lat);
  }

  // the result docks. Each carries the X close button of Qt, plus a
  // toggle action in the ANSICHT menu below, so a dock clicked away comes
  // back through the menu
  auto* body_dock = new QDockWidget(tr("Planeten-Koordinaten"), this);
  body_dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable |
                         QDockWidget::DockWidgetFloatable);
  dress_dock_title(body_dock);
  bodies_ = new QTableWidget(0, 11, body_dock);
  //RR Spalte A ( = Acceleratio ) enthält das Vorzeichen der Beschleunigung
  // and ENTF carries the mutual distance in AU, then the mean node and
  // apsis points after Landscheidt
  // his Vel.', the arc minutes per day of his coordinate screen
  bodies_->setHorizontalHeaderLabels({tr("Länge"), tr("Breite"), tr("Deklin."), "Vel.'", "A", tr("Entf."),
                                      tr("Kn.ND"), tr("Kn.SD"), tr("Perihel"), tr("Aphel"), ""});
  bodies_->horizontalHeader()->setStretchLastSection(true);
  bodies_->verticalHeader()->setDefaultSectionSize(18);
  bodies_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  bodies_->setItemDelegate(new ZodiacDelegate(bodies_));
  body_dock->setWidget(bodies_);
  addDockWidget(Qt::RightDockWidgetArea, body_dock);

  auto* cusp_dock = new QDockWidget(tr("Häuser-Spitzen"), this);
  cusp_dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable |
                         QDockWidget::DockWidgetFloatable);
  dress_dock_title(cusp_dock);
  auto* cusp_host = new QWidget(cusp_dock);
  auto* cusp_layout = new QVBoxLayout(cusp_host);
  cusps_ = new QTableWidget(12, 1, cusp_host);
  // the dock title already reads Häuser-Spitzen, a one-column header
  // would only echo it
  cusps_->horizontalHeader()->setVisible(false);
  cusps_->horizontalHeader()->setStretchLastSection(true);
  cusps_->verticalHeader()->setDefaultSectionSize(18);
  cusps_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  cusps_->setItemDelegate(new ZodiacDelegate(cusps_));
  // the summary box scrolls once the KOMPAKT-AUSWERTUNG list of halbs1
  // grows past a third of the dock, the house cusps keep their room
  aspects_scroll_ = new QScrollArea(cusp_host);
  aspects_scroll_->setWidgetResizable(true);
  aspects_scroll_->setFrameShape(QFrame::NoFrame);
  aspects_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  aspects_label_ = new QLabel(aspects_scroll_);
  aspects_label_->setWordWrap(true);
  aspects_label_->setObjectName("aspectsLine");
  aspects_label_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  aspects_scroll_->setWidget(aspects_label_);
  // the wrap changes with the dock width, the box follows it
  aspects_scroll_->installEventFilter(new ResizeHook([this]() { fit_summary(); }, aspects_scroll_));
  // the same breath below the aspects line as above it, the box should
  // not sit cut off on the window edge
  cusp_layout->setContentsMargins(0, 6, 0, 6);
  cusp_layout->addWidget(cusps_, 1);
  cusp_layout->addWidget(aspects_scroll_);
  cusp_dock->setWidget(cusp_host);
  addDockWidget(Qt::RightDockWidgetArea, cusp_dock);
  // once the window stands, the coordinate dock closes flush on its
  // columns, the remaining columns stay a scroll away
  body_dock_ = body_dock;
  cusp_dock_ = cusp_dock;
  resizeDocks({body_dock, cusp_dock}, {420, 420}, Qt::Horizontal);
  dock_fit_ = new QTimer(this);
  dock_fit_->setSingleShot(true);
  dock_fit_->setInterval(0);
  connect(dock_fit_, &QTimer::timeout, this, &MainWindow::fit_body_dock);
  // the result docks stand for his output screens, a right click on them
  // does not climb on to the Erste Hilfe of the main screen
  for (QDockWidget* dock : {body_dock, cusp_dock}) {
    dock->setContextMenuPolicy(Qt::PreventContextMenu);
  }

  // the menu bar of the original, ÜBER HORCOM first, then his five
  // working menus, the Ansicht menu is the one modern addition. The
  // captions keep his mnemonics from men2 letter for letter, they leave
  // ALT + H, D, C, R, F, Z and M to the function keys of his legend
  QMenu* ueber = menuBar()->addMenu(tr("&ÜBER HORCOM"));
  QMenu* file = menuBar()->addMenu(tr("EI&N-AUSG."));
  QMenu* ephem = menuBar()->addMenu(tr("&EPHEMERIDE"));
  QMenu* horo = menuBar()->addMenu(tr("H&OROSKOPE"));
  QMenu* ausw = menuBar()->addMenu(tr("&AUSWERTUNG"));
  QMenu* divers = menuBar()->addMenu(tr("D&IVERSES"));
  // his commentary texts, every menu keeps its ERLÄUTERUNG entry opening
  // the matching text
  const auto erlaeuterung = [this](const QString& stem) {
    // the English shell opens the English edition, his German original
    // stays one list click away, the language is the one main chose
    KommenDialog dialog(data_dir_ / "kommen", stem, english_edition(), this);
    dialog.exec();
  };
  //RR EINFÜHRUNG = ERLÄUTERUNG 1
  ueber->addAction(tr("EINFÜHRUNG = ERLÄUTERUNG 1"), this, [erlaeuterung]() { erlaeuterung("komm1"); });
  // his MENU 2 TO 8 stay grey, the rows are the legend of the function
  // keys the event filter serves. F3 names the calculator of any system
  // and F7 the picture the port saves, his MSPAINT and GIF were Windows
  static constexpr const char* kKeyLegend[] = {
      QT_TRANSLATE_NOOP("horcom::MainWindow", "FUNKTIONS-Tasten : F1 ( oder ALT + E ) = ZUSTÄNDIGE ERLÄUTERUNG"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "F2 ( oder ALT + A ) = HOROSKOP ANSEHEN ( In sonstigen Ausgaben )"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "F3 ( oder ALT + C ) =  RECHNER ( CALCULATOR ) STARTEN"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "F5 ( oder ALT + R ) = WINKEL/ZEIT DEZIMAL in G/H MIN SEK"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "F6 ( oder ALT + H ) = HELIO- bzw. GEOZENTRISCH UMSCHALTEN"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "F7 ( oder ALT + F ) = AUSGABE als BILD SPEICHERN ( PNG )"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "F8 ( oder ALT + D ) = DRUCKER-OPTION EIN-AUS"),
      QT_TRANSLATE_NOOP("horcom::MainWindow", "F9 ( oder ALT + M ) = DOPPEL-AUSDRUCK AKTIVIEREN")};
  for (const char* legend : kKeyLegend) {
    ueber->addAction(tr(legend))->setEnabled(false);
  }
  ueber->addSeparator();
  ueber->addAction(tr("ÜBER HORCOM"), this, &MainWindow::about);
  //RR DATEN-DATEI EIN-AUSGABE
  file->addAction(tr("DATEN-DATEI EIN-AUSGABE…"), QKeySequence::Open, this, &MainWindow::data_file_io);
  //RR NEU-EINGABE von DATENSÄTZEN
  file->addAction(tr("NEU-EINGABE von DATENSÄTZEN…"), QKeySequence::New, this, &MainWindow::new_records_entry);
  //RR VORGABEN EIN-AUSGABE ÄNDERN
  file->addAction(tr("VORGABEN EIN-AUSGABE ÄNDERN…"), this, &MainWindow::vorgaben_ein_ausgabe);
  //RR AKTUELLEN Datensatz EINTRAGEN ?
  file->addAction(tr("AKTUELLEN Datensatz EINTRAGEN…"), QKeySequence::Save, this, &MainWindow::save_record);
  file->addAction(tr("DATENSATZ BEARBEITEN…"), QKeySequence(Qt::CTRL | Qt::Key_D), this, &MainWindow::edit_record);
  //RR ORTS-DATEIEN : HOLEN - EINTRAGEN - LÖSCHEN
  file->addAction(tr("ORTS-DATEIEN / ORT SUCHEN…"), QKeySequence(Qt::CTRL | Qt::Key_L), this, &MainWindow::open_place);
  file->addAction(tr("ORT in Orts-Datei EINTRAGEN…"), this, &MainWindow::save_place);
  //RR RADIX-DATEN: SATZ1 bis SATZ5
  // the loaded slots stay visible in the menu and the checked one is the
  // chart on the wheel
  file->addSection(tr("RADIX-DATEN:"));
  auto* slot_group = new QActionGroup(this);
  slot_group->setExclusive(true);
  for (int i = 0; i < 5; ++i) {
    QAction* a = file->addAction(QString("SATZ%1").arg(i + 1));
    a->setCheckable(true);
    a->setEnabled(false);
    slot_group->addAction(a);
    connect(a, &QAction::triggered, this, [this, i]() { open_slot({false, i}); });
    slot_actions_[static_cast<std::size_t>(i)] = a;
  }
  //RR SOLAR...-DATEN:
  // the derived charts written back into slots
  file->addSection(tr("SOLAR...-DATEN:"));
  for (int i = 0; i < 5; ++i) {
    QAction* a = file->addAction(QString("SATZ%1").arg(i + 1));
    a->setCheckable(true);
    a->setEnabled(false);
    slot_group->addAction(a);
    connect(a, &QAction::triggered, this, [this, i]() { open_slot({true, i}); });
    solar_actions_[static_cast<std::size_t>(i)] = a;
  }
  //RR DOPPEL-DATEN:
  // the double chart family of the EIN-AUSG. menu, a row names its stored
  // pair and brings it back like the od = 0 slots
  file->addSection(tr("DOPPEL-DATEN:"));
  double_actions_[kDoubleComposit] = file->addAction(tr("COMPOSIT"), this, [this]() { recall_double(kDoubleComposit); });
  double_actions_[kDoubleCombin] = file->addAction(tr("COMBIN"), this, [this]() { recall_double(kDoubleCombin); });
  double_actions_[kDoubleWheel] = file->addAction(tr("DOPPEL-KREIS"), this, [this]() { recall_double(kDoubleWheel); });
  file->addSeparator();
  //RR AUFRÄUMEN / RÜCKSETZEN
  file->addAction(tr("AUFRÄUMEN / RÜCKSETZEN"), this, &MainWindow::clear_slots);
  //RR DRUCKER-OPTION EIN / AUS
  file->addAction(tr("DRUCKER-OPTION EIN / AUS"), this, &MainWindow::printer_option_entry);
  //RR LETZTES BILD ZEIGEN / bzw.SPEICHERN
  file->addAction(tr("LETZTES BILD ZEIGEN / bzw.SPEICHERN…"), this, &MainWindow::last_picture);
  file->addSeparator();
  // the exports of the port beside his picture store, DRUCKEN runs his
  // DRUCKER-GRAPHIK of druck_graph_ein
  file->addAction(tr("HOROSKOP als SVG SPEICHERN…"), this, &MainWindow::export_svg);
  file->addAction(tr("HOROSKOP als PDF SPEICHERN…"), this, &MainWindow::export_pdf);
  file->addAction(tr("DRUCKEN…"), QKeySequence::Print, this, &MainWindow::print_chart);
  file->addAction(tr("ERLÄUTERUNG 2…"), this, [erlaeuterung]() { erlaeuterung("komm2"); });
  // EPHEMERIDE in the order of his menu tree
  //RR VORGABEN EPHEMERIDE ÄNDERN
  ephem->addAction(tr("VORGABEN EPHEMERIDE ÄNDERN…"), this, &MainWindow::vorgaben_ephemeride);
  //RR PLANETEN-KOORDINATEN, ZUSATZ-PLANETEN-KOORDINATEN
  // his a91 and a10 screens, the permanent coordinate dock toggles from
  // ANSICHT
  ephem->addAction(tr("PLANETEN-KOORDINATEN"), this, [this]() { coordinate_table(false); });
  ephem->addAction(tr("ZUSATZ-PLANETEN-KOORDINATEN"), this, [this]() { coordinate_table(true); });
  //RR HELIOZENTRISCHE VERSION EIN/AUS
  ephem->addAction(tr("HELIOZENTRISCHE VERSION EIN/AUS"), this, &MainWindow::toggle_helio);
  ephem->addSeparator();
  //RR * STATISTIK G/H, ERLÄUTERUNG STATISTIK
  // his men3 sets scrout! once STATISTIK is done, LETZTES BILD refuses
  ephem->addAction(tr("STATISTIK G/H…"), this, [this]() {
    statistics_hub();
    scrout_ = true;
  });
  ephem->addAction(tr("ERLÄUTERUNG STATISTIK…"), this, [erlaeuterung]() { erlaeuterung("kommstat"); });
  ephem->addSeparator();
  //RR GRAD-LISTE
  ephem->addAction(tr("GRAD-LISTE G/H…"), this, &MainWindow::degree_list);
  //RR FIX-STERN-POSITIONEN
  ephem->addAction(tr("FIX-STERN-POSITIONEN…"), this, &MainWindow::fixed_star_table);
  //RR ARABISCHE TEILE ( SENS.PUNKTE )
  ephem->addAction(tr("ARABISCHE TEILE ( SENS.PUNKTE )…"), this, &MainWindow::arabic_table);
  //RR INGRESSE SONNE-MOND-MC-AC
  ephem->addAction(tr("INGRESSE SONNE-MOND-MC-AC…"), this, &MainWindow::ingress_table);
  ephem->addSeparator();
  //RR * ET aus UT, * UT aus ET, * DATUM aus JD
  ephem->addAction(tr("ET aus UT…"), this, &MainWindow::et_from_ut);
  ephem->addAction(tr("UT aus ET…"), this, &MainWindow::ut_from_et);
  ephem->addAction(tr("DATUM aus JD…"), this, &MainWindow::date_from_jd);
  ephem->addAction(tr("ERLÄUTERUNG 3…"), this, [erlaeuterung]() { erlaeuterung("komm3"); });
  // HOROSKOPE head, the toggles follow below in his order
  //RR VORGABEN HOROSKOP ÄNDERN
  horo->addAction(tr("VORGABEN HOROSKOP ÄNDERN…"), this, &MainWindow::vorgaben_horoskop);
  //RR HOROSKOP - GRAPHIK
  // back to the plain radix wheel
  horo->addAction(tr("HOROSKOP - GRAPHIK"), this, &MainWindow::horoskop_graphik);
  //RR ASPEKTARIUM G/H
  horo->addAction(tr("ASPEKTARIUM G/H…"), this, &MainWindow::open_aspektarium);
  //RR HALBSUMMEN-GRAPHIK G/H
  horo->addAction(tr("HALBSUMMEN-GRAPHIK G/H…"), this, &MainWindow::midpoint_tree);
  // AUSWERTUNG in three blocks, the return family charts and the Münchner
  // rhythm block first with SEPTAR beside the rhythm since the septars
  // belong to the Münchner Rhythmenlehre, then the direction block, then
  // the transit and graphic block
  //RR VORGABEN DIREKTIONEN ÄNDERN
  ausw->addAction(tr("VORGABEN DIREKTIONEN ÄNDERN…"), this, &MainWindow::vorgaben_direktionen);
  //RR SOLAR-SEPTAR-LUNAR-PLANETARE-PERSONARE
  // each LISTE rides beside its parent
  solar_action_ = ausw->addAction(tr("SOLAR…"), this, &MainWindow::solar_chart);
  solar_list_action_ = ausw->addAction(tr("SOLAR-LISTE…"), this, [this]() { return_list(false); });
  lunar_action_ = ausw->addAction(tr("LUNAR…"), this, &MainWindow::lunar_chart);
  lunar_list_action_ = ausw->addAction(tr("LUNAR-LISTE…"), this, [this]() { return_list(true); });
  ausw->addAction(tr("PLANETAR…"), this, &MainWindow::planetar_chart);
  ausw->addAction(tr("PERSONAR…"), this, &MainWindow::personar_chart);
  // a thin rule under PERSONAR, the return charts above it, the day and
  // rhythm block below
  ausw->addSeparator();
  //RR TAGES-HOR.
  ausw->addAction(tr("TAGES-HOROSKOP…"), this, &MainWindow::day_chart);
  //RR SEKUNDÄR-DIREKTION / DYNAMOGRAMM
  // the entry keeps the short label, SEKUNDÄR-DIREKTION has its own row
  ausw->addAction(tr("DYNAMOGRAMM…"), this, &MainWindow::dynamogram_view);
  // a rule parts the day and dynamogram block from the Munich block,
  // ERLÄUTERUNG 5 and 6 belong to the rhythm theory and SEPTAR follows
  // last because the septars belong to the same school
  ausw->addSeparator();
  //RR MÜNCHNER RHYTHMENLEHRE
  ausw->addAction(tr("MÜNCHNER RHYTHMENLEHRE…"), this, &MainWindow::rhythm);
  ausw->addAction(tr("ERLÄUTERUNG 5…"), this, [erlaeuterung]() { erlaeuterung("komm5"); });
  ausw->addAction(tr("ERLÄUTERUNG 6…"), this, [erlaeuterung]() { erlaeuterung("komm6"); });
  septar_action_ = ausw->addAction(tr("SEPTAR…"), this, &MainWindow::septar_chart);
  // his a16 menu under hrg!, TERRAR in place of SOLAR, the lunars and the
  // septar blank
  const auto helio_menu = [this](bool helio) {
    solar_action_->setText(helio ? tr("TERRAR…") : tr("SOLAR…"));
    solar_list_action_->setText(helio ? tr("TERRAR - DATEN - TABELLE…") : tr("SOLAR-LISTE…"));
    for (QAction* a : {lunar_action_, lunar_list_action_, septar_action_}) {
      a->setEnabled(!helio);
    }
  };
  connect(helio_, &QCheckBox::toggled, this, helio_menu);
  helio_menu(helio_->isChecked());
  helio_menu_ = helio_menu;
  ausw->addSeparator();
  //RR PROGRESS.- HOR.
  // plus the direction group in the order of his menu, SEKUNDÄR-DIREKTION,
  // SONNE (MOND)-BOGEN-DIREKTION, PRIMÄR-DIREKTION ( E.C.KÜHR ), SYMB.
  // DIREKTION ÄQUATORIAL and EKLIPT.
  ausw->addAction(tr("PROGRESSIONS-HOROSKOP…"), this, &MainWindow::progression_chart);
  ausw->addAction(tr("SEKUNDÄR-DIREKTION…"), this, &MainWindow::secondary_direction);
  ausw->addAction(tr("SONNE (MOND)-BOGEN-DIREKTION…"), this, &MainWindow::arc_direction);
  ausw->addAction(tr("PRIMÄR-DIREKTION ( E.C.KÜHR )…"), this, &MainWindow::primary_direction);
  ausw->addAction(tr("SYMB. DIREKTION: ÄQUATORIAL…"), this, [this]() { symbolic_direction(true); });
  ausw->addAction(tr("SYMB. DIREKTION: EKLIPT. G/H…"), this, [this]() { symbolic_direction(false); });
  ausw->addAction(tr("ERLÄUTERUNG 7…"), this, [erlaeuterung]() { erlaeuterung("komm7"); });
  ausw->addAction(tr("GRAD-DATUM-LISTE…"), this, &MainWindow::degree_date_list);
  ausw->addSeparator();
  ausw->addAction(tr("LINEAR-GRAPHIK…"), this, &MainWindow::linear_graph);
  //RR TRANSITE
  // with the Ausgabe-Modus box of a18asw
  ausw->addAction(tr("TRANSITE…"), this, &MainWindow::transite);
  // MUNDAN-ASPEKTE follows TRANSIT-LISTE and is wired below like the
  // primär direction action, both insert into the slots reserved here
  // DIVERSES in the order of his menu tree
  //RR HÄUSER-SYSTEM
  divers->addAction(tr("HÄUSER-SYSTEM…"), this, &MainWindow::choose_house_system);
  //RR HÄUSER-TABELLE
  divers->addAction(tr("HÄUSER-TABELLE…"), this, &MainWindow::house_table);
  divers->addAction(tr("ERLÄUTERUNG 8…"), this, [erlaeuterung]() { erlaeuterung("komm8"); });
  //RR KORREKTUR
  korrektur_action_ = divers->addAction(tr("KORREKTUR…"), this, &MainWindow::correction);
  //RR ZEIT-WANDERN G/H
  // his men3 sets scrout! once the walk is done, LETZTES BILD refuses
  time_wander_action_ = divers->addAction(tr("ZEIT-WANDERN G/H…"), this, [this]() {
    time_wander();
    scrout_ = true;
  });
  //RR Solange UHR SICHTBAR wird HOROSKOP ALLE 15 SEK NACHGEZEICHNET !
  //RR UHR G/H
  // his uhr& clock with the acmcl strip every second
  clock_action_ = divers->addAction(tr("UHR G/H…"), this, &MainWindow::uhr);
  clock_timer_ = new QTimer(this);
  clock_timer_->setInterval(kClockStripMs);
  connect(clock_timer_, &QTimer::timeout, this, &MainWindow::clock_tick);
  clock_timer_->start();
  clock_strip_ = new QLabel(this);
  clock_strip_->setObjectName("clockStrip");
  statusBar()->addPermanentWidget(clock_strip_, 1);
  // the strip only stands while the clock runs, like his acmcl
  statusBar()->hide();
  //RR * AR-DE aus EL-EB, * EL-EB aus AR-DE, * LT aus UT, * UT aus LT
  divers->addAction(tr("AR-DE aus EL-EB…"), this, &MainWindow::arde_from_eleb);
  divers->addAction(tr("EL-EB aus AR-DE…"), this, &MainWindow::eleb_from_arde);
  divers->addAction(tr("LT aus UT…"), this, [this]() { local_time_convert(true); });
  divers->addAction(tr("UT aus LT…"), this, [this]() { local_time_convert(false); });
  divers->addSeparator();
  //RR AUFGANG.........
  divers->addAction(tr("AUFGANG / UNTERGANG…"), this, &MainWindow::rise_set);
  //RR FINSTERNISSE....
  divers->addAction(tr("FINSTERNISSE…"), this, &MainWindow::eclipse_table);
  divers->addSeparator();
  //RR DATEIEN VERKETTEN
  divers->addAction(tr("DATEIEN VERKETTEN…"), this, &MainWindow::chain_files);
  //RR AAF-DATEI < > HORCOM-DATEI
  divers->addAction(tr("AAF-DATEI < > HORCOM-DATEI…"), this, &MainWindow::aaf_convert);
  divers->addSeparator();
  //RR * WINKEL-UMRECHNUNG, F5 ( oder ALT + R ) = WINKEL/ZEIT DEZIMAL in G/H MIN SEK
  divers->addAction(tr("WINKEL-UMRECHNUNG…"), this, [this]() { angle_converter(this); });
  divers->addSeparator();
  //RR * HINTERGRUND-FARBEN
  divers->addAction(tr("HINTERGRUND-FARBEN…"), this, &MainWindow::background_colors);
  //RR ORT-WANDERN
  divers->addAction(tr("ORT-WANDERN…"), this, &MainWindow::place_wander);
  //RR GROßES ( = PLATONISCHES ) JAHR
  // grey on the SOLAR level like his MENU 98
  great_year_action_ = divers->addAction(tr("GROßES ( = PLATONISCHES ) JAHR…"), this, &MainWindow::great_year);
  //RR DESKTOP ( QUIT HORCOM )
  // his muuu& = 99 asks the quit question, the one way out of his menu
  // and the one the quit key of the system takes
  divers->addAction(tr("DESKTOP ( QUIT HORCOM )"), QKeySequence::Quit, this, &MainWindow::desktop_quit);
  //RR ERGEBNIS als RADIX
  // the derived chart moves into the EIN-AUSG. menu as a RADIX record of
  // its own like his erg_rad
  result_as_radix_action_ = divers->addAction(tr("ERGEBNIS als RADIX…"), this, &MainWindow::result_as_radix);
  result_as_radix_action_->setEnabled(false);
  divers->addAction(tr("ERLÄUTERUNG 9…"), this, [erlaeuterung]() { erlaeuterung("komm9"); });
  //RR ÄNDERUNGEN / HINWEISE / KURZANL.
  divers->addAction(tr("ÄNDERUNGEN / HINWEISE / KURZANL.…"), this, &MainWindow::anmerkungen);
  //RR MULTIPLE DIREKTIONEN / HARMONICS G/H
  // one entry with his MODUS box
  horo->addAction(tr("MULTIPLE DIREKTIONEN / HARMONICS G/H…"), this, &MainWindow::multi_session);
  // the two sheets as view states, the session sets them, HOROSKOP -
  // GRAPHIK clears them
  harmonic_action_ = new QAction(this);
  harmonic_action_->setCheckable(true);
  connect(harmonic_action_, &QAction::toggled, this, [this](bool on) {
    if (!on) {
      harm_n_ = 0.0;
      recompute();
      banner_->set_record(record_label_.trimmed());
    }
  });
  multi_action_ = new QAction(this);
  multi_action_->setCheckable(true);
  connect(multi_action_, &QAction::toggled, this, [this](bool on) {
    if (!on) {
      multi_event_jd_ = 0.0;
      recompute();
      banner_->set_record(record_label_.trimmed());
    }
  });
  //RR COMPOSIT ^
  // his a13 with the MODUS box and the two SATZ clicks
  horo->addAction(tr("COMPOSIT…"), this, &MainWindow::composite_session);
  // the composite as a view state, the session sets it, HOROSKOP -
  // GRAPHIK clears it together with its pair
  composite_action_ = new QAction(this);
  composite_action_->setCheckable(true);
  connect(composite_action_, &QAction::toggled, this, [this](bool on) {
    if (!on) {
      partner_chart_.reset();
      partner_name_.clear();
      comp_residence_.reset();
      recompute();
      banner_->set_record(record_label_.trimmed());
    }
  });
  horo->addAction(tr("COMBIN…"), this, &MainWindow::combin_chart);
  //RR DOPPEL-KREIS / 90-GRAD-KREIS ^
  // one entry with his MODUS box
  horo->addAction(tr("DOPPEL-KREIS / 90-GRAD-KREIS…"), this, &MainWindow::double_wheel_session);
  // the double wheel of a12 as a view state, the session above sets it
  compare_action_ = new QAction(this);
  compare_action_->setCheckable(true);
  connect(compare_action_, &QAction::toggled, this, [this](bool on) {
    if (!on) {
      partner_chart_.reset();
      partner_name_.clear();
      recompute();
      banner_->set_record(record_label_.trimmed());
      return;
    }
    if (!partner_chart_) {
      // without a pair the view opens through his a12 session, the MODUS
      // box and the two SATZ clicks, never a chooser of its own
      {
        const QSignalBlocker block(compare_action_);
        compare_action_->setChecked(false);
      }
      double_wheel_session();
      return;
    }
    claim_wheel();
    recompute();
  });
  // the 90-GRAD-KREIS, the second mode of the a12 double wheel
  dial_action_ = new QAction(this);
  dial_action_->setCheckable(true);
  connect(dial_action_, &QAction::toggled, this, [this](bool on) {
    if (on && !partner_chart_) {
      // the 90-GRAD-KREIS without a pair starts his a12 session too
      {
        const QSignalBlocker block(dial_action_);
        dial_action_->setChecked(false);
      }
      double_wheel_session();
      return;
    }
    if (on) {
      claim_wheel();
    }
    recompute();
  });
  horo->addAction(tr("ERLÄUTERUNG 4…"), this, [erlaeuterung]() { erlaeuterung("komm4"); });
  // the directed axes of prima on the wheel, a state switch only, the
  // PRIMÄR DIRIGIERTE ACHSEN session of KORREKTUR drives it
  directions_action_ = new QAction(this);
  directions_action_->setCheckable(true);
  connect(directions_action_, &QAction::toggled, this, [this](bool on) {
    recompute();
    if (!on) {
      banner_->set_record(record_label_.trimmed());
    }
  });
  // the HOROSKOP-GRAPHIK of SONNEN- and MOND-BOGEN, a hidden view state
  arc_action_ = new QAction(this);
  arc_action_->setCheckable(true);
  // MUNDAN-ASPEKTE, the running bodies among themselves of mund. It sits
  // as the last entry of AUSWERTUNG like the last row of his menu, the
  // horm 2 frame lives in VORGABEN HOROSKOP ÄNDERN like his BEZUGS-SYSTEM
  ausw->addAction(tr("MUNDAN-ASPEKTE…"), this, &MainWindow::mundane_aspects);
  // the ERLÄUTERUNG of F1 once every entry of his six menus stands
  tag_help_stems({{ueber, "komm1"}, {file, "komm2"}, {ephem, "komm3"}, {horo, "komm4"}, {ausw, "komm5"}, {divers, "komm9"}});
  // the text scale of the shell, the wheel keeps its own canvas scale
  QMenu* view = menuBar()->addMenu(tr("ANSICH&T"));
  const auto set_scale = [](int scale) {
    QSettings settings;
    const int s = std::clamp(scale, theme::kTextScaleMin, theme::kTextScaleMax);
    settings.setValue(theme::kTextScaleKey, s);
    theme::apply(s, theme::dark_theme());
  };
  const auto scale_now = []() { return QSettings().value(theme::kTextScaleKey, theme::kTextScaleNormal).toInt(); };
  //RR Parameter - Einstellungen = VORGABEN
  // his main screen panel
  view->addAction(tr("VORGABEN-ÜBERSICHT…"), this, &MainWindow::vorgaben_overview);
  // the element and quality table of the chart, a rewrite addition kept
  // out of his EPHEMERIDE menu
  view->addAction(tr("HISTOGRAMME…"), this, &MainWindow::histogram_view);
  // the permanent result docks beside the wheel, rewrite additions that
  // stay open while his menu entries open his own table screens. The
  // original had no entry for the cusps, his houses never left the sheet
  const auto dock_toggle = [](QDockWidget* dock) {
    if (dock != nullptr) {
      const bool bring_back = !dock->isVisible();
      dock->setVisible(bring_back);
      if (bring_back) {
        dock->raise();
      }
    }
  };
  view->addAction(tr("KOORDINATEN-TAFEL EIN / AUS"), this, [this, dock_toggle]() { dock_toggle(body_dock_); });
  view->addAction(tr("HÄUSER-SPITZEN EIN / AUS"), this, [this, dock_toggle]() { dock_toggle(cusp_dock_); });
  view->addSeparator();
  view->addAction(tr("SCHRIFT GRÖßER"), QKeySequence::ZoomIn, this, [set_scale, scale_now]() { set_scale(scale_now() + theme::kTextScaleStep); });
  view->addAction(tr("SCHRIFT KLEINER"), QKeySequence::ZoomOut, this, [set_scale, scale_now]() { set_scale(scale_now() - theme::kTextScaleStep); });
  view->addAction(tr("NORMALE SCHRIFT"), QKeySequence(Qt::CTRL | Qt::Key_0), this, [set_scale]() { set_scale(theme::kTextScaleNormal); });
  view->addSeparator();
  // the two dresses of the shell, black on white like his working
  // screens or the night sky of his splash, and his own colours of
  // HINTERGRUND-FARBEN over the dress in force
  QMenu* colors = view->addMenu(tr("FARBEN"));
  auto* theme_group = new QActionGroup(colors);
  const bool own_colors = QSettings().value(theme::kOwnColorsKey, false).toBool();
  const auto set_dark = [this, scale_now](bool dark) {
    QSettings().setValue(theme::kDarkKey, dark);
    // a chosen dress replaces the colours of HINTERGRUND-FARBEN
    QSettings().setValue(theme::kOwnColorsKey, false);
    theme::set_own_colors(QColor(), QColor());
    theme::apply(scale_now(), dark);
    wheel_->update();
    banner_->update();
    recompute();
  };
  auto* light_action = colors->addAction(tr("SCHWARZ auf WEIß"));
  light_action->setCheckable(true);
  light_action->setActionGroup(theme_group);
  light_action->setChecked(!own_colors && !theme::dark_theme());
  connect(light_action, &QAction::triggered, this, [set_dark]() { set_dark(false); });
  auto* dark_action = colors->addAction(tr("NACHTHIMMEL"));
  dark_action->setCheckable(true);
  dark_action->setActionGroup(theme_group);
  dark_action->setChecked(!own_colors && theme::dark_theme());
  connect(dark_action, &QAction::triggered, this, [set_dark]() { set_dark(true); });
  //RR HINTERGRUND-FARBEN
  // the colours his KONSTA keeps, chosen in the DIVERSES entry
  auto* own_action = colors->addAction(tr("HINTERGRUND-FARBEN"));
  own_action->setObjectName(kOwnColorsAction);
  own_action->setCheckable(true);
  own_action->setActionGroup(theme_group);
  own_action->setChecked(own_colors);
  connect(own_action, &QAction::triggered, this, [this]() {
    QSettings().setValue(theme::kOwnColorsKey, true);
    theme::set_own_colors(theme::dialog_color(konsta_.col_dial), theme::passive_color(konsta_.col_backg));
    wheel_->update();
    banner_->update();
  });
  view->addSeparator();
  view->addAction(tr("PLANETEN-AUSWAHL…"), this, &MainWindow::planet_selection);
  view->addSeparator();
  // the language survives in the settings and applies on the next start
  QMenu* language = view->addMenu(tr("SPRACHE / LANGUAGE"));
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
  add_lang(tr("AUTOMATISCH ( SYSTEMSPRACHE )"), QString());
  add_lang("DEUTSCH", "de");
  add_lang("ENGLISH", "en");
  view->addSeparator();
  // the whole program back to its first start, beside his AUFRÄUMEN /
  // RÜCKSETZEN that clears the session alone
  view->addAction(tr("ALLES ZURÜCKSETZEN…"), this, &MainWindow::full_reset);

  // name edits land in the record, the banner and the sheet corner
  const auto apply_name = [this]() {
    const std::string given = given_->text().trimmed().toStdString();
    const std::string surname = surname_->text().trimmed().toStdString();
    if (given == record_.given && surname == record_.surname) {
      return;
    }
    record_.given = given;
    record_.surname = surname;
    refresh_record_label();
    recompute();
  };
  connect(given_, &QLineEdit::editingFinished, this, apply_name);
  connect(surname_, &QLineEdit::editingFinished, this, apply_name);
  // the place text edits reach the record, ORTS-DATEIEN sets it too
  connect(place_field_, &QLineEdit::editingFinished, this, [this]() {
    const std::string place = place_field_->text().trimmed().toStdString();
    if (place == record_.place) {
      return;
    }
    record_.place = place;
    recompute();
  });
  // recompute on every change like the original recalculated per screen
  // the free text date needs a small settle window, editingFinished
  // only fires on Enter or focus out, without this the wheel keeps the
  // old date until the user leaves the field. The timer only fires
  // when the field parses as a complete TT.MM.JJJJ, half typed dates
  // wait for their last digit
  date_timer_ = new QTimer(this);
  date_timer_->setSingleShot(true);
  date_timer_->setInterval(500);
  connect(date_timer_, &QTimer::timeout, this, [this]() {
    if (panel_day_valid()) {
      recompute();
    }
  });
  connect(date_, &QLineEdit::textEdited, this, [this](const QString&) { date_timer_->start(); });
  connect(date_, &QLineEdit::editingFinished, this, [this]() {
    date_timer_->stop();
    recompute();
  });
  connect(time_, &QTimeEdit::timeChanged, this, &MainWindow::recompute);
  connect(zone_, &QDoubleSpinBox::valueChanged, this, &MainWindow::recompute);
  connect(lon_, &QDoubleSpinBox::valueChanged, this, &MainWindow::recompute);
  connect(lat_, &QDoubleSpinBox::valueChanged, this, &MainWindow::recompute);
  connect(houses_, &QComboBox::currentIndexChanged, this, &MainWindow::recompute);
  for (QCheckBox* box : {parallax_, extras_, hamburg_, apogee_show_, node_show_, true_node_, true_apogee_, helio_,
                         sommerzeit_, double_dst_, local_time_, julian_}) {
    connect(box, &QCheckBox::toggled, this, &MainWindow::recompute);
  }
  connect(transit_on_, &QCheckBox::toggled, this, [this](bool on) {
    tdate_->setEnabled(on);
    ttime_->setEnabled(on);
    // the switch of the panel leaves the screen of the TRANSITE walk
    if (!on) {
      a20_transits_ = false;
    }
    recompute();
  });
  connect(tdate_, &QDateEdit::dateChanged, this, &MainWindow::recompute);
  connect(ttime_, &QTimeEdit::timeChanged, this, &MainWindow::recompute);
  resize(1280, 760);
}

// the panel date reads in its own calendar, his jul$, every ChartInput
// leaving here carries the settings calendar the engine converts with
ChartInput MainWindow::current_input() const {
  ChartInput in;
  CalendarDate local = panel_day();
  const QTime t = time_->time();
  local.hour = static_cast<double>(t.hour());
  local.minute = t.minute() + t.second() / 60.0;
  const double jd_local = julian_day(local, panel_calendar_);
  double jd_ut = 0.0;
  if (clock_kind_ != ClockKind::kZone) {
    //RR jd = jd - gl / 360, with the equation of time for a true clock
    jd_ut = ut_from_local_clock(jd_local, lon_->value(), clock_kind_);
  } else {
    // zone hours east of Greenwich lead back to UT by subtraction, the
    // summer shift rides on the zone like his ZONEN-Datei carried it
    jd_ut = jd_local - (zone_->value() + dst_hours_) / kHoursPerDay;
  }
  in.date_ut = calendar_date(jd_ut, current_settings().calendar);
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
  // his final defaults named CH QU XE as extra planets, the panel switch
  // Zusatz-Planeten brings the three in through included_, the planet
  // selection adds the remaining elements one by one. The true or mean
  // radio only picks the formula, the Schwarzer Mond box switches AG on
  // and off
  const bool apogee_on = apogee_show_->isChecked();
  const auto want = [&](int slot) { return included_[static_cast<std::size_t>(slot)]; };
  s.nk = {};
  if (apogee_on) {
    s.nk[1] = body::kApogee;
  }
  if (want(body::kChiron)) {
    s.nk[2] = body::kChiron;
  }
  if (want(body::kTranspluto)) {
    s.nk[3] = body::kTranspluto;
  }
  if (want(body::kFortune)) {
    s.nk[4] = body::kFortune;
  }
  if (want(body::kCeres)) {
    s.nk[5] = body::kCeres;
  }
  if (want(body::kPallas)) {
    s.nk[6] = body::kPallas;
  }
  if (want(body::kJuno)) {
    s.nk[7] = body::kJuno;
  }
  if (want(body::kVesta)) {
    s.nk[8] = body::kVesta;
  }
  // the Hamburger Faktoren travel through their own slots, the hidden
  // hamburg_ shortcut fans them and the Planeten-Auswahl can opt any off
  for (int i = 9; i <= 16; ++i) {
    const int slot = 18 + i;
    if (want(slot)) {
      s.nk[static_cast<std::size_t>(i)] = slot;
    }
  }
  if (want(body::kQuaoar)) {
    s.nk[17] = body::kQuaoar;
  }
  if (want(body::kHalley)) {
    s.nk[18] = body::kHalley;
  }
  if (want(body::kPholus)) {
    s.nk[19] = body::kPholus;
  }
  if (want(body::kDamokles)) {
    s.nk[20] = body::kDamokles;
  }
  if (want(body::kNessus)) {
    s.nk[21] = body::kNessus;
  }
  if (want(body::kXena)) {
    s.nk[22] = body::kXena;
  }
  // any nk slot filled marks the extras pass on
  bool any = apogee_on;
  for (int i = 2; i <= 22; ++i) {
    if (s.nk[static_cast<std::size_t>(i)] > 0) {
      any = true;
      break;
    }
  }
  s.extra_bodies = any;
  return s;
}

// the transit ring and the clock rank first in the drawing ladder, a
// special view switching on must take the wheel from them or nothing
// visibly changes
void MainWindow::claim_wheel() {
  a20_transits_ = false;
  if (transit_on_ != nullptr && transit_on_->isChecked()) {
    const QSignalBlocker block(transit_on_);
    transit_on_->setChecked(false);
    tdate_->setEnabled(false);
    ttime_->setEnabled(false);
  }
}

MainWindow::PanelState MainWindow::panel_state() const {
  PanelState s;
  s.given = given_->text();
  s.surname = surname_->text();
  s.place = place_field_->text();
  s.date = date_->text();
  s.time = time_->time();
  s.zone = zone_->value();
  s.dst = dst_hours_;
  s.calendar = panel_calendar_;
  s.clock = clock_kind_;
  s.lon = lon_->value();
  s.lat = lat_->value();
  s.houses = houses_->currentIndex();
  s.parallax = parallax_->isChecked();
  s.extras = extras_->isChecked();
  s.hamburg = hamburg_->isChecked();
  s.apogee = apogee_show_->isChecked();
  s.node_show = node_show_ != nullptr && node_show_->isChecked();
  s.true_node = true_node_->isChecked();
  s.true_apogee = true_apogee_->isChecked();
  s.helio = helio_->isChecked();
  s.transit_on = transit_on_->isChecked();
  s.tdate = tdate_->date();
  s.ttime = ttime_->time();
  s.record = record_;
  s.solar = active_is_solar_;
  s.slot = active_is_solar_ ? active_solar_ : active_slot_;
  return s;
}

void MainWindow::restore_state(const PanelState& s) {
  restoring_ = true;
  {
    const QSignalBlocker b0a(given_);
    const QSignalBlocker b0b(surname_);
    const QSignalBlocker b0c(place_field_);
    const QSignalBlocker b1(date_);
    const QSignalBlocker b2(time_);
    const QSignalBlocker b3(zone_);
    const QSignalBlocker b3s(sommerzeit_);
    const QSignalBlocker b3d(double_dst_);
    const QSignalBlocker b3l(local_time_);
    const QSignalBlocker b3j(julian_);
    const QSignalBlocker b4(lon_);
    const QSignalBlocker b5(lat_);
    const QSignalBlocker b6(houses_);
    const QSignalBlocker b7(parallax_);
    const QSignalBlocker b8(extras_);
    const QSignalBlocker b9(hamburg_);
    const QSignalBlocker b10(apogee_show_);
    const QSignalBlocker b10n(node_show_);
    const QSignalBlocker b11(true_node_);
    const QSignalBlocker b12(true_apogee_);
    const QSignalBlocker b13(helio_);
    const QSignalBlocker b14(transit_on_);
    const QSignalBlocker b15(tdate_);
    const QSignalBlocker b16(ttime_);
    given_->setText(s.given);
    surname_->setText(s.surname);
    place_field_->setText(s.place);
    date_->setText(s.date);
    time_->setTime(s.time);
    zone_->setValue(s.zone);
    clock_kind_ = s.clock;
    set_dst(s.dst);
    set_panel_calendar(s.calendar);
    local_time_->setChecked(s.clock != ClockKind::kZone);
    zone_->setEnabled(s.clock == ClockKind::kZone);
    sommerzeit_->setEnabled(s.clock == ClockKind::kZone);
    lon_->setValue(s.lon);
    lat_->setValue(s.lat);
    sync_coord_boxes();
    houses_->setCurrentIndex(s.houses);
    parallax_->setChecked(s.parallax);
    extras_->setChecked(s.extras);
    hamburg_->setChecked(s.hamburg);
    // the shortcut checkboxes carry their group slots via the toggled
    // connect, restore blocks that signal so re-sync the group slots
    // by hand, the dialog picks stay in included_ between panel steps
    for (int slot : {body::kChiron, body::kQuaoar, body::kXena}) {
      included_[static_cast<std::size_t>(slot)] = s.extras;
    }
    for (int slot : {body::kCupido, body::kHades, body::kZeus, body::kKronos, body::kApollon,
                     body::kAdmetos, body::kVulkanus, body::kPoseidon}) {
      included_[static_cast<std::size_t>(slot)] = s.hamburg;
    }
    apogee_show_->setChecked(s.apogee);
    // the toggled connect is blocked here so mirror the AG group slot by
    // hand, the dialog picks stay in included_ between panel steps
    included_[body::kApogee] = s.apogee;
    // the Sommerzeit hint reads zone_ and sommerzeit_, both blocked above,
    // so pull the label through its refresh entry point by hand
    if (refresh_sommer_effect_) refresh_sommer_effect_();
    if (node_show_ != nullptr) {
      node_show_->setChecked(s.node_show);
    }
    true_node_->setChecked(s.true_node);
    true_apogee_->setChecked(s.true_apogee);
    helio_->setChecked(s.helio);
    transit_on_->setChecked(s.transit_on);
    tdate_->setDate(s.tdate);
    ttime_->setTime(s.ttime);
    tdate_->setEnabled(s.transit_on);
    ttime_->setEnabled(s.transit_on);
  }
  // the helpers the blocked toggles would have run
  if (sync_lunar_radios_) sync_lunar_radios_();
  if (helio_menu_) helio_menu_(s.helio);
  if (!s.transit_on) {
    a20_transits_ = false;
  }
  // the slot of the step leads again while it still holds a record
  const auto holds = [this](bool solar, int i) {
    return i >= 0 && i < static_cast<int>(slots_.size()) &&
           (solar ? solar_slots_[static_cast<std::size_t>(i)].has_value() : slots_[static_cast<std::size_t>(i)].has_value());
  };
  active_is_solar_ = s.solar && holds(true, s.slot);
  active_solar_ = active_is_solar_ ? s.slot : -1;
  active_slot_ = !s.solar && holds(false, s.slot) ? s.slot : -1;
  record_ = s.record;
  current_state_ = s;
  current_state_.slot = active_is_solar_ ? active_solar_ : active_slot_;
  current_state_.solar = active_is_solar_;
  state_init_ = true;
  recompute();
  refresh_record_label();
  if (active_is_solar_) {
    banner_->set_record(solar_labels_[static_cast<std::size_t>(active_solar_)]);
  }
  update_slot_actions();
  update_solar_actions();
  restoring_ = false;
  update_history_actions();
}

void MainWindow::track_history() {
  if (restoring_) {
    return;
  }
  const PanelState now = panel_state();
  if (state_init_ && !(now == current_state_)) {
    if (!pending_) {
      pending_ = current_state_;
      forward_.clear();
    }
    history_timer_->start();
  }
  current_state_ = now;
  state_init_ = true;
  update_history_actions();
}

void MainWindow::flush_history() {
  history_timer_->stop();
  if (!pending_) {
    return;
  }
  back_.push_back(*pending_);
  pending_.reset();
  if (back_.size() > kHistoryDepth) {
    back_.erase(back_.begin());
  }
  update_history_actions();
  store_active_slot();
}

void MainWindow::store_active_slot() {
  if (active_is_solar_) {
    if (active_solar_ >= 0) {
      solar_slots_[static_cast<std::size_t>(active_solar_)] = panel_record();
      update_solar_actions();
    }
  } else if (active_slot_ >= 0) {
    slots_[static_cast<std::size_t>(active_slot_)] = panel_record();
    update_slot_actions();
  }
}

void MainWindow::history_back() {
  flush_history();
  if (back_.empty()) {
    return;
  }
  forward_.push_back(panel_state());
  const PanelState s = back_.back();
  back_.pop_back();
  restore_state(s);
  // the slot takes the step back like any settled edit
  store_active_slot();
}

void MainWindow::history_forward() {
  flush_history();
  if (forward_.empty()) {
    return;
  }
  back_.push_back(panel_state());
  const PanelState s = forward_.back();
  forward_.pop_back();
  restore_state(s);
  store_active_slot();
}

void MainWindow::update_history_actions() {
  if (back_action_ == nullptr || forward_action_ == nullptr) {
    return;
  }
  back_action_->setEnabled(!back_.empty() || pending_.has_value());
  forward_action_->setEnabled(!forward_.empty());
}

void MainWindow::recompute() {
  track_history();
  // his zeuhr, the taken over clock record is the chart while its slot
  // leads and shows the moment itself
  const bool clock = uhr_slot_ >= 0 && active_slot_ == uhr_slot_ && !active_is_solar_ && uhr_place_set_;
  ChartInput in;
  if (clock) {
    in = clock_input();
    // the record ticks along with the moment, and like his
    // mainkont_dat_zeit under acmcl the panel date and time follow it.
    // A tick is no step of the history
    const AafRecord r = clock_record(in.date_ut);
    slots_[static_cast<std::size_t>(uhr_slot_)] = r;
    const QSignalBlocker b1(date_);
    const QSignalBlocker b2(time_);
    set_panel_day(r.day, r.month, r.year);
    time_->setTime(QTime(r.hour, r.minute, r.second));
    current_state_ = panel_state();
  } else {
    in = current_input();
  }
  const ChartSettings s = current_settings();
  Chart chart = compute_chart(in, s, vsop_, eph_);
  if (!chart.ok) {
    // beyond the polar circle the original still computes the chart
    // and lists only AC and MC, the intermediate cusps stay empty
    ChartSettings fallback = s;
    fallback.houses = HouseSystem::kAcMcOnly;
    chart = compute_chart(in, fallback, vsop_, eph_);
  }
  // the horm 2 transform runs before every scanner like the original,
  // the hrg mode has no houses so mundane stays out like fixpunkt_def
  const bool mundane = mundane_frame_ && !clock && !s.heliocentric;
  if (mundane && chart.ok) {
    to_mundane(chart, in.lat_deg);
  }
  if (!chart.ok) {
    //RR Geog. Breite zu groß !
    banner_->set_record(tr("Geog. Breite zu groß für dieses Häusersystem"));
    return;
  }
  // the user defined fixed point rides on slot zero like fixpunkt_def,
  // a Septar carries the Sonderpunkt of the Rhythmenlehre there instead
  // lpkt, IF INSTR(sol$(od,ze),"SEPTAR") > 0, fixpunkt& = 2
  const bool septar = !clock && rhythm_chart_label().contains("SEPTAR");
  if (septar && !s.heliocentric) {
    if (!konsta_.fixpunkt_rh.empty() && (konsta_.lpktg || rhythm_lpkt_ != 0.0)) {
      const double sp = norm_rad(QString::fromStdString(konsta_.fixpunkt_rh).trimmed().toDouble() * kDegToRad);
      chart.b[0].present = true;
      chart.b[0].valid = true;
      chart.b[0].el = sp;
      // a degree defined point projects, a date defined one stems from
      // the mundane houses already
      if (mundane && konsta_.lpktg) {
        chart.b[0].el = mundane_longitude(sp, kEps, chart.smo.ekls, chart.armc_deg * kDegToRad, in.lat_deg);
      }
    }
  } else if (fixpunkt_ >= 0.0 && !s.heliocentric) {
    chart.b[0].present = true;
    chart.b[0].valid = true;
    chart.b[0].el = fixpunkt_;
    if (mundane) {
      chart.b[0].el = mundane_longitude(fixpunkt_, kEps, chart.smo.ekls, chart.armc_deg * kDegToRad, in.lat_deg);
    }
  }
  const AspectSettings shown_settings = shown_aspect_settings();
  const AspectResult aspects = scan_aspects(chart, s, shown_settings);
  last_chart_ = chart;
  last_aspects_ = aspects;

  const WheelOptions wopt = radix_wheel_options(chart, s);

  bool transit_drawn = false;
  QString cross_text;
  Chart comp_holder;
  AspectResult comp_aspects_holder;
  Chart dir_holder;
  const Chart* shown = &chart;
  const AspectResult* shown_aspects = &aspects;
  if (clock) {
    WheelOptions opt = wopt;
    //RR " UHR ", bes2 stays silent under uhr!
    opt.center_label = " UHR ";
    opt.chart_label.clear();
    opt.chart_sub_label.clear();
    show_wheel(build_wheel(chart, s, aspects, opt));
    banner_->set_record(QString("UHR %1 UT").arg(QDateTime::currentDateTimeUtc().time().toString("HH:mm:ss")));
    transit_drawn = true;
  } else if (transit_on_->isChecked()) {
    ChartInput tin;
    const QDate td = tdate_->date();
    const QTime tt = ttime_->time();
    tin.date_ut = {td.day(), td.month(), td.year(), static_cast<double>(tt.hour()),
                   tt.minute() + tt.second() / 60.0};
    // the EREIGNIS-Ort of ort_wahl, the birth place when none was asked
    tin.lon_deg_east = transit_place_ ? transit_place_->lon : lon_->value();
    tin.lat_deg = transit_place_ ? transit_place_->lat : lat_->value();
    Chart tchart = compute_chart(tin, s, vsop_, eph_);
    if (tchart.ok) {
      WheelOptions opt = wopt;
      opt.center_label =
          QString("TRANSIT=>%1 %2 UT").arg(td.toString("dd.MM.yyyy"), tt.toString("HH:mm")).toStdString();
      opt.chart_label.clear();
      opt.chart_sub_label.clear();
      // the LAUFENDE FAKTOREN of a20_horg while its walk runs
      trim_running_ring(tchart, opt);
      if (a20_transits_) {
        // the screen of the TRANSITE walk, the running AC and MC ride
        // outside and zeitwi writes TRANSIT=> + q$, datum3$ and ze$
        opt.center_label.clear();
        opt.outer_axes = true;
        DisplayList dl = build_transit_wheel(chart, tchart, s, aspects, opt);
        constexpr double amh = kWheelCenterX;
        constexpr double bmh = kWheelCenterY;
        constexpr double kZeitwiText = 10.0;
        const CalendarDate moment = calendar_date(tchart.jd_ut, s.calendar);
        add_sheet_text(dl, amh - 58.0, bmh - 12.0, ("TRANSIT=>" + rhythm_chart_label()).toStdString(), kZeitwiText);
        add_sheet_text(dl, amh - 50.0, bmh + 2.0, datum3_text(moment).toStdString(), kZeitwiText);
        add_sheet_text(dl, amh - 50.0, bmh + 12.0, homise_text(kDegPerHour * (moment.hour + moment.minute / 60.0), 0).toStdString(),
                       kZeitwiText);
        const EventPlace at = transit_place_ ? *transit_place_ : EventPlace{lon_->value(), lat_->value(), record_.place};
        show_full_sheet(a20_sheet(chart, tchart, s, A18Mode::kTransits, at, std::move(dl)));
      } else {
        show_wheel(build_transit_wheel(chart, tchart, s, aspects, opt));
      }
      transit_drawn = true;
      // the comparison list of a12asp with his one degree transit orb
      // rule, running body, separation, radix body
      const std::vector<CrossAspectHit> cross = scan_aspects_between(chart, tchart, aspect_settings_, true);
      cross_text = theme::heading_span(tr("TRANSITE")) + "&nbsp; ";
      // hard&, 1 red, 3 blue, 2 the normal ink
      const QString hard = outer_color_ == 1 ? "#ff0000" : (outer_color_ == 3 ? "#0000ff" : QString());
      cross_text += cross.empty() ? tr("keine") : cross_hits_text(cross, hard);
    }
  } else if (arc_action_ != nullptr && arc_action_->isChecked() && arc_jd_ > 0.0 && !s.heliocentric) {
    // the HOROSKOP-GRAPHIK of a20_horg for SEKUNDÄR and SONNEN-BOGEN, the
    // progressed sky or the radix moved by the sun's arc rides outside
    transit_drawn = show_ring(chart, in, s, aspects, wopt);
  } else if (directions_action_ != nullptr && directions_action_->isChecked() && dir_jd_ > 0.0 && !s.heliocentric) {
    // the directed axes of prima under Placidus at the event place, over
    // the radix or the event positions, no chords like primhorg
    // @plre, Nur PLACIDUS !
    const DirectedAxes d =
        direct_axes(chart, dir_lon_, dir_lat_, dir_jd_, dir_converse_, dir_vary_, HouseSystem::kPlacidus);
    dir_holder = chart;
    if (dir_event_planets_) {
      ChartInput ev;
      ev.date_ut = calendar_date(dir_jd_, s.calendar);
      ev.lon_deg_east = dir_lon_;
      ev.lat_deg = dir_lat_;
      const Chart event_chart = compute_chart(ev, s, vsop_, eph_);
      if (event_chart.ok) {
        dir_holder = event_chart;
      }
    }
    dir_holder.houses = d.houses;
    dir_holder.armc_deg = d.armc_deg;
    dir_holder.b[body::kAscendant].el = d.houses.angles.ac;
    dir_holder.b[body::kMc].el = d.houses.angles.mc;
    WheelOptions opt = wopt;
    // bes2 stays silent under prima!, primhorg writes his own texts
    opt.chart_label.clear();
    opt.chart_sub_label.clear();
    opt.scale = kDirectedWheelScale;
    opt.aspect_lines = false;
    show_directed_axes(build_wheel(dir_holder, s, aspects, opt), d);
    shown = &dir_holder;
    transit_drawn = true;
    banner_->set_record(QString("%1 %2°")
                            .arg(dir_converse_ ? "KONVERS" : "DIREKT")
                            .arg(d.arc_deg, 0, 'f', 3));
  } else if ((multi_action_ != nullptr && multi_action_->isChecked() && multi_event_jd_ > 0.0) ||
             (harmonic_action_ != nullptr && harmonic_action_->isChecked() && harm_n_ != 0.0)) {
    // the directed outer wheel over the radix, one of the six MULTI ages
    // or the harm21 harmonic. Both run on a radix of their own, geocentric
    // with the mean node and apogee in the ecliptic frame, whatever the
    // panel holds
    const bool harmonic = !(multi_action_ != nullptr && multi_action_->isChecked() && multi_event_jd_ > 0.0);
    const auto [radix, ms] = multi_radix(in, s);
    const AspectResult radix_aspects = scan_aspects(radix, ms, shown_settings);
    const HarmonicHouses houses_mode =
        (harmonic ? harm_new_mc_ : multi_new_mc_) ? HarmonicHouses::kFromNewMc : HarmonicHouses::kLikeBodies;
    const double lja = harmonic ? 0.0 : (multi_event_jd_ - radix.jd_ut) / radix.ta.tropical_year_days;
    const Chart directed = harmonic ? harmonic_chart(radix, harm_n_, houses_mode, ms.houses, lat_->value())
                                    : multi_chart(radix, multi_mode_, lja, multi_ref_, houses_mode, ms.houses, lat_->value());
    WheelOptions opt = radix_wheel_options(radix, ms, false);
    opt.chart_label.clear();
    opt.chart_sub_label.clear();
    // the outer symbols of multi1 keep their black, hard& is for the
    // transit, ZEIT-WANDERN and DOPPELKREIS rings
    opt.outer_tint = false;
    // km = 0.82, the scale multi1 and harm share
    opt.scale = kMultiWheelScale;
    // STR$(ha) + ".HARMONIC" or the mul$ of the mode
    const QString mul = harmonic ? QString::number(harm_n_, 'g', 6) + ".HARMONIC" : multi_name(multi_mode_);
    show_full_sheet(multi_sheet(radix, directed, ms, harmonic, mul, lja, harmonic ? radix.jd_ut : multi_event_jd_,
                                build_double_wheel(radix, directed, ms, radix_aspects, opt)));
    transit_drawn = true;
    banner_->set_record(harmonic ? mul : QString("%1 = %2 LJ").arg(mul).arg(lja, 0, 'f', 3));
  } else if (composite_action_ != nullptr && composite_action_->isChecked() && partner_chart_) {
    // the a13 composite, house mode from his profile flags, the ROBERT
    // HAND residence from the session or else the panel place
    const CompositeHouses mode = composite_mode(konsta_, s.houses);
    const double residence_lat = comp_residence_ ? comp_residence_->lat : lat_->value();
    // fixpunkt_def puts the one fixed point on every record, a13 pairs
    // from aa& and so carries it into the composite
    Chart partner = *partner_chart_;
    if (fixpunkt_ >= 0.0 && chart.b[body::kFixpunkt].present) {
      partner.b[body::kFixpunkt] = chart.b[body::kFixpunkt];
    }
    comp_holder = composite_chart(chart, in, partner, partner_input_, mode, residence_lat, s);
    comp_aspects_holder = scan_aspects(comp_holder, s, shown_settings);
    WheelOptions opt = wopt;
    // bes2 with bes2_comp, the method under COMPOSIT
    opt.chart_label = tr("COMPOSIT").toStdString();
    opt.chart_sub_label = (mode == CompositeHouses::kMeanSidereal ? tr("Mittl.STZ...")
                           : mode == CompositeHouses::kRobertHand ? tr("N.ROB. HAND")
                                                                  : tr("Schematisch"))
                              .toStdString();
    // IF w& = 1 && comp! && (comp_mstz! OR comp_hand!)
    opt.composite_axes = mode != CompositeHouses::kSchematic;
    show_wheel(build_wheel(comp_holder, s, comp_aspects_holder, opt));
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
    // a12asp, the natal positions of both charts even on the 90° circle
    CrossScanOptions scan;
    scan.extras = s.extra_bodies;
    scan.heliocentric = s.heliocentric;
    const std::vector<CrossAspectHit> cross = scan_aspects_between(chart, *partner_chart_, shown_settings, scan);
    if (dial_action_ != nullptr && dial_action_->isChecked()) {
      // 90°-KREIS, everything times four, the scan on the a12f state
      Chart d1 = dial_chart(chart, 4.0);
      Chart d2 = dial_chart(*partner_chart_, 4.0);
      const AspectResult da = scan_aspects(d1, s, shown_settings);
      dial_display(d1, 4.0);
      dial_display(d2, 4.0);
      WheelOptions opt = wopt;
      opt.dial = true;
      // IF dop = 4, a$ = "90" + "°" + "- KREIS"
      opt.chart_label = tr("90°- KREIS").toStdString();
      opt.chart_sub_label.clear();
      show_full_sheet(a12_sheet(chart, *partner_chart_, s, true, build_double_wheel(d1, d2, s, da, opt)));
      cross_text = theme::heading_span(tr("VERGLEICH 90°")) + "&nbsp; ";
      cross_text += cross.empty() ? tr("keine") : cross_hits_text(cross);
      banner_->set_record(QString::fromUtf8("90° %1 × %2").arg(mine, partner_name_));
    } else {
      // the a12 double wheel, the partner outside at full scale, bes2
      // writes nothing in its centre, HELIOZ. in the hrg mode
      WheelOptions opt = wopt;
      opt.chart_label = s.heliocentric ? tr("HELIOZ.").toStdString() : std::string();
      opt.chart_sub_label.clear();
      show_full_sheet(a12_sheet(chart, *partner_chart_, s, false, build_double_wheel(chart, *partner_chart_, s, aspects, opt)));
      cross_text = theme::heading_span(tr("VERGLEICH")) + "&nbsp; ";
      cross_text += cross.empty() ? tr("keine") : cross_hits_text(cross);
      banner_->set_record(QString("%1 × %2").arg(mine, partner_name_));
    }
    transit_drawn = true;
  }
  // einzel_plan_wahl answers only on the HOROSKOP-GRAPHIK of a chart
  plain_view_ = !transit_drawn;
  if (!transit_drawn && mundane) {
    // the horm 2 frame, bes2 adds Mundan to the sol$ of the centre
    show_wheel(build_wheel(chart, s, aspects, wopt));
    banner_->set_record("MUNDAN");
  } else if (!transit_drawn) {
    show_wheel(build_wheel(chart, s, aspects, wopt));
  }
  banner_->set_info(QString("JD(UT) %1   ΔT %2 min   ARMC %3°   %4%5")
                        .arg(chart.jd_ut, 0, 'f', 5)
                        .arg(chart.delt_minutes, 0, 'f', 2)
                        .arg(chart.armc_deg, 0, 'f', 4)
                        .arg(s.heliocentric
                                 ? tr("Heliozentrisch")
                                 : QString::fromUtf8(chart.houses.name.data(), static_cast<int>(chart.houses.name.size())))
                        .arg(!s.heliocentric && s.topocentric_parallax ? "   MitParall." : ""));
  fill_tables(*shown, *shown_aspects, shown == &comp_holder);
  if (!cross_text.isEmpty()) {
    aspects_label_->setText(cross_text);
    fit_summary();
  }
}

void MainWindow::fill_tables(const Chart& chart, const AspectResult& aspects, bool longitudes_only) {
  bodies_->setRowCount(0);
  // a midpoint chart has no latitude, speed or distance, bes10 of the
  // composite lists its longitudes alone
  for (int col = 1; col < bodies_->columnCount(); ++col) {
    bodies_->setColumnHidden(col, longitudes_only);
  }
  // a chart without a sun but with the moon slot filled is the hrg
  // mode, the slot then carries the earth
  const bool helio = !chart.b[body::kSun].present && chart.b[body::kMoon].present;
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    const BodyState& b = chart.b[static_cast<std::size_t>(slot)];
    if (!b.present) {
      continue;
    }
    const int row = bodies_->rowCount();
    bodies_->insertRow(row);
    // pl$(2) = "TE", the moon slot carries the earth in the hrg mode
    const std::string_view tag = (helio && slot == body::kMoon) ? body::kEarthName
                                                                : body::kName[static_cast<std::size_t>(slot)];
    const QString name = QString::fromUtf8(tag.data(), static_cast<int>(tag.size()));
    // his Pl column drew the black planet sprite, it rides beside the
    // uppercase tag the family wished for
    auto* head = new QTableWidgetItem(name);
    const QImage head_img = glyph_sprite(name, to_rgb(theme::ink_now()));
    if (!head_img.isNull()) {
      head->setIcon(QIcon(QPixmap::fromImage(head_img)));
    }
    bodies_->setVerticalHeaderItem(row, head);
    if (!b.valid) {
      bodies_->setItem(row, 0, new QTableWidgetItem(tr("außerhalb der Ephemeride")));
      continue;
    }
    const bool angle_slot = slot == body::kAscendant || slot == body::kMc;
    bodies_->setItem(row, 0, zodiac_item(b.el));
    if (!angle_slot && !longitudes_only) {
      bodies_->setItem(row, 1, new QTableWidgetItem(degs(b.eb)));
      bodies_->setItem(row, 2, new QTableWidgetItem(degs(b.de)));
      // Vel.', his velocity column counts arc minutes per day
      bodies_->setItem(row, 3, new QTableWidgetItem(QString::number(b.tb * kRadToDeg * 60.0, 'f', 1)));
      // the sign of the acceleration, direct or retrograde at a station
      bodies_->setItem(row, 4, new QTableWidgetItem(b.ttb < 0.0 ? QString::fromUtf8("−") : "+"));
      if (b.dr > 0.0) {
        //RR ENTFERNUNGSWERTE der GROßEN PLANETEN RELATIV oder ABSOLUT ?
        //RR Mittelwerte nach MEYERS Lexikon Weltall
        static constexpr double kMeanGeo[11] = {0.0, 1.0, 0.0,    1.0,    1.0,   1.52, 5.195,
                                                9.525, 19.215, 30.055, 39.44};
        static constexpr double kMeanHelio[11] = {0.0, 1.0, 1.0,    0.387,  0.723, 1.542, 5.205,
                                                  9.567, 19.281, 30.142, 39.880};
        const double mean = (slot >= 1 && slot <= 10)
                                ? (helio ? kMeanHelio[slot] : kMeanGeo[slot])
                                : 0.0;
        if (konsta_.entf == 1) {
          // percent of the mean, bodies without a mean stay blank
          if (mean > 0.0) {
            bodies_->setItem(row, 5,
                             new QTableWidgetItem(QString::number(std::lround(kPercent * b.dr / mean)) + "%"));
          }
        } else {
          bodies_->setItem(row, 5, new QTableWidgetItem(QString::number(b.dr, 'f', 3)));
        }
      }
      //RR die mittleren Planeten-KNOTEN und die PLANETEN-APSIDEN
      const PlanetPoints pts = planet_points(chart, slot, current_settings());
      if (pts.ok) {
        if (pts.node >= 0.0) {
          bodies_->setItem(row, 6, zodiac_item(pts.node));
          bodies_->setItem(row, 7, zodiac_item(pts.node_south));
        }
        bodies_->setItem(row, 8, zodiac_item(pts.perihelion));
        bodies_->setItem(row, 9, zodiac_item(pts.aphelion));
      }
      auto* retro = new QTableWidgetItem(b.tb < 0.0 ? "R" : "");
      retro->setForeground(QColor(0xE8, 0x5D, 0x4E));
      bodies_->setItem(row, 10, retro);
    }
  }
  bodies_->resizeColumnsToContents();
  // the columns only widen in a session, a chart with narrower values
  // keeps them, so the table and its dock stand still while one clicks
  // in the input panel. A new text size measures afresh
  const int px = QFontInfo(bodies_->font()).pixelSize();
  if (px != body_widths_px_) {
    body_widths_.clear();
    body_widths_px_ = px;
  }
  body_widths_.resize(static_cast<std::size_t>(bodies_->columnCount()), 0);
  for (int c = 0; c + 1 < bodies_->columnCount(); ++c) {
    if (!bodies_->isColumnHidden(c)) {
      int& widest = body_widths_[static_cast<std::size_t>(c)];
      widest = std::max(widest, bodies_->columnWidth(c));
      bodies_->setColumnWidth(c, widest);
    }
  }
  // bes111 lists no cusps in the hrg mode
  for (int i = 1; i <= 12; ++i) {
    cusps_->setItem(i - 1, 0,
                    helio ? new QTableWidgetItem(QString())
                          : zodiac_item(chart.houses.cusp[static_cast<std::size_t>(i)]));
  }
  //RR die MONDPHASE ... ist die ekliptikale Längendifferenz MOND-SONNE
  // with his percent figure, full moon one hundred, new moon zero
  QString phase_text;
  if (!helio && chart.b[body::kSun].valid && chart.b[body::kMoon].valid) {
    const double d = norm_rad(chart.b[body::kMoon].el - chart.b[body::kSun].el) * kRadToDeg;
    const double pct = (180.0 - std::abs(d - 180.0)) / 180.0 * kPercent;
    phase_text = "<br>" + theme::heading_span(tr("MONDPHASE")) +
                 tr("&nbsp; %1° (%2%)").arg(d, 0, 'f', 0).arg(pct, 0, 'f', 0);
  }
  // Spiegelung, the mirror point pairs of spieg1
  QString mirror_text;
  if (!aspects.mirrors.empty()) {
    QStringList pairs;
    for (const auto& [t, w] : aspects.mirrors) {
      const auto name = [](int slot) {
        const std::string_view v = body::kName[static_cast<std::size_t>(slot)];
        return QString::fromUtf8(v.data(), static_cast<int>(v.size()));
      };
      pairs << name(t) + "/" + name(w);
    }
    mirror_text = "<br>" + theme::heading_span(tr("SPIEGELUNG")) + "&nbsp; " + pairs.join("  ");
  }
  aspects_label_->setText(theme::heading_span(tr("ASPEKTE")) +
                          tr("&nbsp; konj %1  opp %2  trigon %3  quadrat %4  sextil %5")
                              .arg(aspects.zh[1])
                              .arg(aspects.zh[2])
                              .arg(aspects.zh[3])
                              .arg(aspects.zh[4])
                              .arg(aspects.zh[6]) +
                          mirror_text + phase_text + compact_midpoints(chart));
  fit_summary();
}

void MainWindow::fit_summary() {
  if (aspects_scroll_ == nullptr) {
    return;
  }
  const int width = aspects_scroll_->viewport()->width();
  const int text = width > 0 ? aspects_label_->heightForWidth(width) : aspects_label_->sizeHint().height();
  const int cap = aspects_label_->fontMetrics().lineSpacing() * 8 + 12;
  // the box only grows in a session, the cusp table above keeps its
  // place from chart to chart, a smaller text size lowers the cap
  summary_height_ = std::min(cap, std::max(summary_height_, std::max(text, aspects_label_->sizeHint().height())));
  aspects_scroll_->setFixedHeight(summary_height_);
}

// ported from halbs1 with halbs11 and halbsa, the KOMPAKT-AUSWERTUNG of
// his sheet. IF voll! && asp1! = FALSE && horm& = 1 the midpoint list
// stands beside the aspects, a family heading each, t=u-w per entry,
// the bodies of the Planeten-Auswahl red like col_halbsel
QString MainWindow::compact_midpoints(const Chart& chart) const {
  const ChartSettings s = current_settings();
  if (!konsta_.voll || (mundane_frame_ && !s.heliocentric)) {
    return {};
  }
  const MidpointResult mids = scan_midpoints(chart, s, shown_aspect_settings());
  const std::array<int, body::kSlotCount> marks = shown_emphasis();
  const bool helio = s.heliocentric;
  const auto name = [&](int slot) {
    const std::string_view v = (helio && slot == body::kMoon) ? body::kEarthName : body::kName[static_cast<std::size_t>(slot)];
    const QString n = QString::fromUtf8(v.data(), static_cast<int>(v.size()));
    return marks[static_cast<std::size_t>(slot)] > 0 ? "<span style=\"color:#ff0000\">" + n + "</span>" : n;
  };
  // "HALBSUM." then "DIREKT:", "QUADRAT:" and "HALBQU:"
  static constexpr std::pair<int, const char*> kFamilies[3] = {
      {1, QT_TR_NOOP("DIREKT:")}, {2, QT_TR_NOOP("QUADRAT:")}, {4, QT_TR_NOOP("HALBQU:")}};
  QString out = "<br>" + theme::heading_span(tr("HALBSUM."));
  for (const auto& [nh, heading] : kFamilies) {
    QStringList entries;
    for (const MidpointHit& h : mids.hits) {
      if (h.nh == nh) {
        entries << name(h.t) + "=" + name(h.u) + "-" + name(h.w);
      }
    }
    out += "&nbsp; <b>" + tr(heading) + "</b> " + (entries.isEmpty() ? QString("-") : entries.join("  "));
  }
  return out;
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
  sync_coord_boxes();
  // the picker files store the step from zone time to UT, the panel
  // wants hours east
  if (const auto to_ut = r.zone_to_ut()) {
    zone_->setValue(-*to_ut);
  }
  // the new place belongs on the sheet, not only its coordinates
  record_.place = dialog.chosen_name().toStdString();
  {
    const QSignalBlocker bp(place_field_);
    place_field_->setText(QString::fromStdString(record_.place));
  }
  if (refresh_sommer_effect_) refresh_sommer_effect_();
  recompute();
}

// the EINTRAGEN branch of a2ort, the current panel place is written
// into an ORTS-DATEI, a chosen existing one grows or a new one starts
void MainWindow::save_place() {
  PlaceRecord r;
  r.lon = lon_->value();
  r.lat = lat_->value();
  r.name = record_.place;
  if (r.name.empty()) {
    r.name = "ORT";
  }
  enter_place(this, data_dir_, r);
}

void MainWindow::apply_moment(double jd_ut, const QString& label, bool solar_slot, bool keep_clock) {
  if (!progression_pending_) {
    prog_event_note_.clear();
    prog_radix_note_.clear();
  }
  // a derived chart is written in UT, a corrected radix in its own clock
  double jd_clock = jd_ut;
  if (!keep_clock) {
    set_panel_calendar(Calendar::kAuto);
    set_local_time(false, false);
    set_dst(0.0);
    const QSignalBlocker b3(zone_);
    zone_->setValue(0.0);
  } else if (clock_kind_ != ClockKind::kZone) {
    // the inverse of zuo, the equation of time barely moves in the step
    jd_clock = jd_ut + lon_->value() / kDegPerCircle;
    if (clock_kind_ == ClockKind::kTrueLocal) {
      jd_clock += equation_of_time_days(jd_clock);
    }
  } else {
    jd_clock = jd_ut + (zone_->value() + dst_hours_) / kHoursPerDay;
  }
  const CalendarDate d = calendar_date(jd_clock, panel_calendar_);
  int seconds = static_cast<int>((d.hour * 60.0 + d.minute) * 60.0 + 0.5);
  if (seconds >= kSecondsPerDay) {
    seconds = kSecondsPerDay - 1;
  }
  const QSignalBlocker b1(date_);
  const QSignalBlocker b2(time_);
  set_panel_day(d.day, d.month, d.year);
  time_->setTime(QTime(seconds / 3600, (seconds / 60) % 60, seconds % 60));
  if (refresh_sommer_effect_) refresh_sommer_effect_();
  recompute();
  banner_->set_record(label);
  // the ^ items of his menu write the result back into a SOLAR slot
  if (solar_slot) {
    store_solar(label);
  }
}

// the capture hook, the panel record becomes the running clock record
// without his boxes
void MainWindow::show_clock() {
  set_clock_place(panel_record());
  uhr_on_ = true;
  int slot = next_slot();
  if (slot < 0) {
    slot = 4;
  }
  uhr_slot_ = slot;
  set_slot(slot, clock_record(clock_now()), true);
  clock_menu_update();
  clock_strip_update();
}

// ported from stella with stelk, stelt and stelaus
void MainWindow::fixed_star_table() {
  if (!last_chart_) {
    return;
  }
  // his appa& = 2 with @a9, the stars stand in Apparent 2 whatever the
  // Vorgaben say, the profile returns on the way out
  constexpr int kStarAppa = 2;
  ChartSettings s = current_settings();
  s.apparent = ApparentMode::kLightTimeAberration;
  Chart chart = compute_chart(current_input(), s, vsop_, eph_);
  if (!chart.ok) {
    return;
  }
  // his fixpunkt_def sets aa& = 0, stelk then starts at the Fixpunkt
  if (fixpunkt_ >= 0.0) {
    BodyState& fp = chart.b[body::kFixpunkt];
    fp.present = true;
    fp.valid = true;
    fp.el = fixpunkt_;
  }
  QDialog dialog(this);
  mark_output(&dialog, menu_item::kFixedStars);
  // his mend1$, the menu caption as the window title
  dialog.setWindowTitle(tr("FIX-STERN-POSITIONEN"));
  auto* v = new QVBoxLayout(&dialog);
  const std::vector<StarRow> rows = fixed_stars(chart, aspect_settings_.orb);
  auto* table = new QTableWidget(static_cast<int>(rows.size()), 9, &dialog);
  // his column heads of stel0
  table->setHorizontalHeaderLabels({tr("Stern - Name"), tr("Ekl.Länge"), tr("Aspekte"), tr("Qualität"),
                                    tr("Astron.Name"), tr("Breite"), tr("Rekt."), tr("Dekl."), tr("D/LJ")});
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  table->setFocusPolicy(Qt::NoFocus);
  table->setFont(theme::mono_font());
  // his stelaus, STR$(b * up,6,2), NN for a zero
  const auto stelaus = [](double rad) {
    return rad == 0.0 ? QStringLiteral("  NN  ") : QString::asprintf("%6.2f", rad * kRadToDeg);
  };
  // stelk draws at most four hits, 28 pixels each below z1& + 88
  constexpr std::size_t kMaxStarHits = 4;
  const auto text = [](std::string_view t) { return QString::fromUtf8(t.data(), static_cast<qsizetype>(t.size())); };
  for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
    const StarRow& r = rows[static_cast<std::size_t>(i)];
    auto* name_item = new QTableWidgetItem(text(r.name));
    QString asp;
    std::size_t hits = 0;
    for (const auto& [slot, kind] : r.aspects) {
      if (hits >= kMaxStarHits) {
        break;
      }
      ++hits;
      // stelk sets the aspect sprite at z& and the tag at z& + 8
      asp += QString::fromUtf8(aspect_glyph(star_aspect_family(kind))) + text(body::kName[static_cast<std::size_t>(slot)]) + " ";
    }
    // his deftextcol(3), the aspected name red on cyan while the
    // DRUCKER-OPTION is off so a HARDCOPY prints plain
    if (hits > 0 && konsta_.prenbl == 0) {
      name_item->setBackground(QColor(0x00, 0xFF, 0xFF));
      name_item->setForeground(QColor(0xFF, 0x00, 0x00));
    }
    table->setItem(i, 0, name_item);
    // his gz1$ of grze
    table->setItem(i, 1, new QTableWidgetItem(zodiac_text(r.la, ZodiacForm::kGz1)));
    table->setItem(i, 2, new QTableWidgetItem(asp.trimmed()));
    table->setItem(i, 3, new QTableWidgetItem(text(r.quality)));
    table->setItem(i, 4, new QTableWidgetItem(text(r.astro)));
    table->setItem(i, 5, new QTableWidgetItem(stelaus(r.br)));
    table->setItem(i, 6, new QTableWidgetItem(stelaus(norm_rad(r.ar))));
    table->setItem(i, 7, new QTableWidgetItem(stelaus(r.de)));
    // his e$ = STR$(ef&,6), NN where the distance is unknown
    table->setItem(i, 8, new QTableWidgetItem(r.lightyears > 0 ? QString::asprintf("%6d", r.lightyears)
                                                               : QStringLiteral("    NN")));
  }
  table->resizeColumnsToContents();
  v->addWidget(table, 1);
  // his stelt, LEFT$(na$,25) + "|" + dm$ + ":" + datum3$ + "|" + sol$ + "|Eph.:" + gena4$
  const ClassicSheetText sheet = classic_sheet_text();
  const QString label = rhythm_chart_label();
  auto* stelt = new QLabel(QString::fromStdString(sheet.name).left(25) + "|" + tr("Datum") + ":" +
                               datum3_text(calendar_date(chart.jd_ut, s.calendar)) + "|" + label + "|" + tr("Eph.:") +
                               gena4_text(kStarAppa),
                           &dialog);
  stelt->setObjectName("stelt");
  v->addWidget(stelt);
  dialog.resize(940, 640);
  dialog.exec();
}

// ported from aspar2, the trees on two bands of eleven, 22 a page. Space,
// Enter and the quote key turn the page, ESC ends. After the last page
// the geocentric chart asks for his 45 degree sort, the right mouse
// opens einzel_plan_wahl1 in its HALBSUMMEN wording
void MainWindow::midpoint_tree() {
  if (!last_chart_) {
    return;
  }
  const Chart& chart = *last_chart_;
  const ChartSettings s = current_settings();
  const auto trees_now = [this, &chart, &s]() {
    std::array<bool, body::kSlotCount> partners{};
    const std::array<int, body::kSlotCount> shown = shown_emphasis();
    for (std::size_t i = 0; i < partners.size(); ++i) {
      // IF NOT (plw&(u&) = 0 && asp_wahl!)
      partners[i] = shown[i] >= 0;
    }
    return midpoint_trees(chart, s, aspect_settings_, partners);
  };
  std::vector<MidpointTree> plain = trees_now();
  MidpointTreeText text;
  const ClassicSheetText sheet = classic_sheet_text();
  text.sol = rhythm_chart_label().toStdString();
  text.frame = horgt_text().toStdString();
  // "Name: " + na$(od,ze)
  text.name = (tr("Name: ") + QString::fromStdString(sheet.name).trimmed()).toStdString();
  // dm$ + " : " + datum3$
  text.date = (tr("Datum") + " : " + datum3_text(calendar_date(chart.jd_ut, s.calendar))).toStdString();
  // "Zeit(UT):" + ze$ + "   Länge:" + STR$(gl,6,2) + "°" + gl$ + "  Breite:" + STR$(gg,5,2) + "°" + gg$
  {
    const CalendarDate d = calendar_date(chart.jd_ut, s.calendar);
    const double lon = lon_->value();
    const double lat = lat_->value();
    text.clock = (tr("Zeit(UT):") + homise_text((d.hour + d.minute / 60.0) * kDegPerHour, 0) + tr("   Länge:") +
                  QString::asprintf("%6.2f", std::abs(lon)) + QString::fromUtf8("°") + (lon < 0.0 ? "W" : "E") +
                  tr("  Breite:") + QString::asprintf("%5.2f", std::abs(lat)) + QString::fromUtf8("°") +
                  (lat < 0.0 ? "S" : "N"))
                     .toStdString();
  }
  text.ephem = gena2_text().toStdString();

  QDialog view(this);
  mark_output(&view, menu_item::kMidpointGraphic);
  const QString title = tr("HALBSUMMEN-GRAPHIK");
  view.setWindowTitle(title);
  auto* v = new QVBoxLayout(&view);
  v->setContentsMargins(0, 0, 0, 0);
  auto* canvas = new WheelWidget(&view);
  v->addWidget(canvas, 1);
  // the counts of halbs_zaehl_gr under the trees, no popup to click away
  auto* counts = new QLabel(&view);
  counts->setObjectName(QStringLiteral("midpointCounts"));
  counts->setContentsMargins(10, 4, 10, 4);
  v->addWidget(counts);
  std::vector<MidpointTree> shown = plain;
  std::vector<bool> close;
  bool sorted = false;
  // halbsz1% to halbsz4%, the levels 1, 2, 4 and 8 of halbs111
  const auto level_counts = [&plain]() {
    std::array<int, 4> n{};
    for (const MidpointTree& t : plain) {
      for (const MidpointHit& h : t.hits) {
        const int level = h.nh == 1 ? 0 : (h.nh == 2 ? 1 : (h.nh == 4 ? 2 : 3));
        ++n[static_cast<std::size_t>(level)];
      }
    }
    return n;
  };
  // sort& 0 not asked yet, 1 JA, 2 NEIN
  int sort_answer = s.heliocentric ? 2 : 0;
  int page = 0;
  bool asking = false;
  const int pages = [&]() {
    return std::max(1, static_cast<int>((plain.size() + kTreesPerPage - 1) / kTreesPerPage));
  }();
  // plein2 stamps the nodes and the Black Moon inverted like the wheel
  TreeGlyphs glyphs;
  {
    const WheelOptions dress = radix_wheel_options(chart, s);
    glyphs.invert_nodes = dress.invert_nodes;
    glyphs.invert_apogee = dress.invert_apogee;
  }
  const auto draw = [&]() {
    canvas->set_plain_list(build_midpoint_trees(shown, page, text, sorted, close, shown_emphasis(), glyphs));
    // _WIN$ + "  |  WEITER mit LEERTASTE" before the sort question
    view.setWindowTitle(page + 1 == pages && sort_answer == 0 ? title + tr("  |  WEITER mit LEERTASTE") : title);
    counts->setText(midpoint_count_line(level_counts(), true));
  };
  LambdaFilter keys([&](QEvent* e) {
    if (e->type() == QEvent::MouseButtonPress && static_cast<QMouseEvent*>(e)->button() == Qt::RightButton) {
      // einzel_plan_wahl, muuu& 55
      if (single_planet_choice(3, true)) {
        plain = trees_now();
        shown = sorted ? sort_trees_by_dial(plain) : plain;
        close = tree_dial_close(shown, aspect_settings_);
        page = 0;
        draw();
      }
      return true;
    }
    if (e->type() != QEvent::KeyPress || asking) {
      return false;
    }
    const int key = static_cast<QKeyEvent*>(e)->key();
    if (key == Qt::Key_Escape) {
      view.reject();
      return true;
    }
    // CASE 13,32,34, the virtual keys Enter, Space and PageDown
    const bool on = key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_PageDown;
    if (page + 1 < pages) {
      if (on) {
        ++page;
        draw();
      }
      return true;
    }
    if (sort_answer == 0) {
      // SORTIEREN nach KRITERIUM :, any key opens the box
      asking = true;
      const int r = ChoiceDialog::ask(&view, tr("AUSWAHL"),
                                      {tr("SORTIEREN nach KRITERIUM :"), QString(),
                                       tr("8*Länge auf 360° REDUZIERT?"), tr("MACHT 'PLANETENBILDER' ERKENNBAR !")},
                                      {tr("JA"), tr(" NEIN ")}, 0);
      asking = false;
      sort_answer = r == 0 ? 1 : 2;
      view.setWindowTitle(title);
      if (r == 0) {
        sorted = true;
        shown = sort_trees_by_dial(plain);
        close = tree_dial_close(shown, aspect_settings_);
        page = 0;
        draw();
      }
      return true;
    }
    if (on) {
      // GOTO aspa1, the trees start over
      page = 0;
      draw();
      return true;
    }
    view.reject();
    return true;
  });
  view.installEventFilter(&keys);
  canvas->installEventFilter(&keys);
  draw();
  view.resize(size());
  view.exec();
}

// ported from daterw and the chain branch of a200dat. A record joins the
// chain with day and month, and the trim pass of his VERKETTEN BEENDEN,
// a2f_tr_dat(-1), drops every record without place coordinates. The
// port chains DAT and AAF files in one go into an AAF or a DAT file
void MainWindow::chain_files() {
  //RR ZU VERKETTENDE HORCOM - DATEN - DATEIEN NACHEINANDER AUFRUFEN !
  const QStringList sources = QFileDialog::getOpenFileNames(
      this, tr("Zu verkettende Dateien wählen"), collection_start(data_file_, data_dir_),
      tr("HORCOM Daten (*.DAT *.dat *.AAF *.aaf)"));
  if (sources.isEmpty()) {
    return;
  }
  QString kind;
  QString target = QFileDialog::getSaveFileName(
      this, tr("Ketten-Datei"), QString::fromStdWString((aaf_folder(data_file_, data_dir_) / "kette.aaf").wstring()),
      tr("AAF (*.aaf *.AAF);;HORCOM Daten-Dateien (*.DAT *.dat)"), &kind);
  if (target.isEmpty()) {
    return;
  }
  target = with_suffix(target, kind.startsWith("AAF") ? "AAF" : "DAT");
  std::vector<AafRecord> all;
  const auto join = [&all](const AafRecord& r) {
    if (chain_keeps(r)) {
      all.push_back(r);
    }
  };
  for (const QString& src : sources) {
    const std::filesystem::path p(src.toStdWString());
    if (src.endsWith(".dat", Qt::CaseInsensitive)) {
      if (const auto records = read_chart_file(p)) {
        for (const ChartRecord& r : *records) {
          join(aaf_from_chart_record(r));
        }
      }
    } else if (const auto records = read_aaf(p)) {
      for (const AafRecord& r : *records) {
        join(r);
      }
    }
  }
  const std::filesystem::path out(target.toStdWString());
  if (all.empty() ||
      !(target.endsWith(".dat", Qt::CaseInsensitive) ? write_dat_from_aaf(out, all) : write_aaf(out, all))) {
    QMessageBox::warning(this, "HORCOM", tr("Die Ketten-Datei ließ sich nicht schreiben."));
    return;
  }
  QMessageBox::information(this, "HORCOM", tr("%1 Datensätze verkettet.").arg(all.size()));
}

// ported from aaf_horcom, AAF-DATEI < > HORCOM-DATEI in both directions,
// the converted file is the twin of the chosen one
void MainWindow::aaf_convert() {
  //RR Wie umwandeln ?
  const int rr = ChoiceDialog::ask(this, "HORCOM", {tr("Wie umwandeln ?")},
                                   {tr("AAF - in HORCOM - Format ( = AAF-Datei IMPORTIEREN )"),
                                    tr("HORCOM - in AAF - Format"), tr("AAF-Help von M.GARMS aufrufen"),
                                    tr("ABBRUCH")});
  if (rr == 0) {
    import_aaf_file();
  } else if (rr == 1) {
    //RR HORCOM-Format umwandeln in AAF-Format ist nur sinnvoll ...
    const int al = ChoiceDialog::ask(
        this, "HORCOM",
        {tr("HORCOM-Format umwandeln in AAF-Format ist nur sinnvoll"),
         tr("Wenn Sie ältere HORCOM-Dateien überabeiten oder mit Usern austauschen wollen"),
         tr("deren Programm das AAF-Format aber nicht das HORCOM-Format versteht !!")},
        {tr("ABBRUCH"), tr(" TROTZDEM WEITER ")});
    if (al == 1) {
      export_dat_file();
    }
  } else if (rr == 2) {
    KommenDialog help(data_dir_ / "kommen", "aaf_komm", english_edition(), this);
    help.exec();
  }
}

// his inputbox for a new file name without extension. The eight letters
// of his LEFT$(s$,8) were the DOS limit, the port keeps the whole name
std::optional<QString> MainWindow::ask_file_stem() {
  bool ok = false;
  //RR ANDEREN Namen EINGEBEN !
  QString stem = QInputDialog::getText(this, tr("ANDEREN Namen EINGEBEN !"), tr("Name OHNE EXTENSION !"),
                                       QLineEdit::Normal, QString(), &ok);
  stem = stem.trimmed();
  if (const int dot = stem.indexOf('.'); dot >= 0) {
    stem = stem.left(dot);
  }
  if (!ok || stem.isEmpty()) {
    return std::nullopt;
  }
  return stem;
}

// ported from aaf_horcom0 and aaf_horcom2, an AAF file into its DAT twin
void MainWindow::import_aaf_file() {
  const std::filesystem::path start = aaf_folder(data_file_, data_dir_);
  //RR Umzuwandelndes  AAF - FILE Auswählen !
  const QString src = QFileDialog::getOpenFileName(this, tr("Umzuwandelndes  AAF - FILE Auswählen !"),
                                                   QString::fromStdWString(start.wstring()),
                                                   tr("AAF (*.aaf *.AAF)"));
  if (src.isEmpty()) {
    return;
  }
  const std::filesystem::path aaf(src.toStdWString());
  const auto records = read_aaf(aaf);
  if (!records || records->empty()) {
    //RR DATEI ???
    QMessageBox::warning(this, "HORCOM", tr("DATEI ???"));
    return;
  }
  std::filesystem::path dat = dat_twin_path(aaf);
  while (std::filesystem::exists(dat)) {
    //RR Existierendes File ... ERSETZEN ?
    const int r = ChoiceDialog::ask(this, "HORCOM",
                                    {tr("Existierendes File"), QString::fromStdWString(dat.wstring()), tr("ERSETZEN ?")},
                                    {tr(" JA "), tr(" NEIN "), tr("ABBRUCH")});
    if (r == 0) {
      break;
    }
    if (r != 1) {
      return;
    }
    const auto stem = ask_file_stem();
    if (!stem) {
      return;
    }
    dat = dat.parent_path() / (stem->toStdWString() + L".DAT");
  }
  std::vector<ChartRecord> out;
  out.reserve(records->size());
  for (const AafRecord& r : *records) {
    out.push_back(dat_from_record(r));
  }
  if (!write_chart_file(dat, out)) {
    QMessageBox::warning(this, "HORCOM", tr("Die HORCOM-Datei ließ sich nicht schreiben."));
    return;
  }
  //RR " Aus dem File " + aaffile$, " Wird ein File " + horcfile$ + " gebildet !"
  QMessageBox::information(this, "HORCOM",
                           tr(" Aus dem File %1").arg(QString::fromStdWString(aaf.wstring())) + "\n" +
                               tr(" Wird ein File %1 gebildet !").arg(QString::fromStdWString(dat.wstring())));
}

// ported from horcom_aaf and horcom_aaf2, a DAT file into its AAF twin
void MainWindow::export_dat_file() {
  //RR Umzuwandelndes  HORCOM - FILE Auswählen !
  const QString src = QFileDialog::getOpenFileName(this, tr("Umzuwandelndes  HORCOM - FILE Auswählen !"),
                                                   collection_start(data_file_, data_dir_),
                                                   tr("HORCOM Daten-Dateien (*.DAT *.dat)"));
  if (src.isEmpty()) {
    return;
  }
  std::filesystem::path dat(src.toStdWString());
  const auto records = read_chart_file(dat);
  if (!records) {
    QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht lesen."));
    return;
  }
  std::filesystem::path aaf = aaf_twin_path(dat);
  std::optional<QString> renamed;
  if (std::filesystem::exists(aaf)) {
    //RR Ein AAF-File ... existiert schon !
    const int r = ChoiceDialog::ask(
        this, "HORCOM",
        {tr("Ein AAF-File %1 existiert schon !").arg(QString::fromStdWString(aaf.wstring())),
         tr("ANDEREN Namen für das AAF - FILE eingeben ?"), tr(" Sonst evtl. INFORMATIONS-VERLUST ! "),
         tr(" Falls es sich um ein ORIGINÄRES AAF-FILE handelt ! ")},
        {tr(" JA = Name ÄNDERN "), tr(" TROTZDEM WEITER "), tr("ABBRUCH")});
    if (r == 0) {
      // the DAT follows the new name so the pair stays a pair
      do {
        renamed = ask_file_stem();
        if (!renamed) {
          return;
        }
        aaf = aaf.parent_path() / (renamed->toStdWString() + L".AAF");
      } while (std::filesystem::exists(aaf));
    } else if (r != 1) {
      return;
    }
  }
  std::vector<AafRecord> out;
  out.reserve(records->size());
  for (const ChartRecord& c : *records) {
    out.push_back(aaf_export_record(c));
  }
  if (!write_aaf(aaf, out)) {
    QMessageBox::warning(this, "HORCOM", tr("Die AAF-Datei ließ sich nicht schreiben."));
    return;
  }
  if (renamed) {
    //RR NAME horcfile$ AS hrc$ + "\SPEZIAL\" + s$ + ".DAT"
    const std::filesystem::path moved = dat.parent_path() / (renamed->toStdWString() + L".DAT");
    std::error_code ec;
    std::filesystem::rename(dat, moved, ec);
    if (!ec) {
      if (QFileInfo(data_file_) == QFileInfo(src)) {
        bind_data_file(QString::fromStdWString(moved.wstring()));
      }
      dat = moved;
    }
  }
  //RR " Aus dem File " + horcfile$, " Wird ein File " + aaffile$ + " gebildet !"
  QMessageBox::information(this, "HORCOM",
                           tr(" Aus dem File %1").arg(QString::fromStdWString(dat.wstring())) + "\n" +
                               tr(" Wird ein File %1 gebildet !").arg(QString::fromStdWString(aaf.wstring())));
}

// ported from grli with grlinit, grli1 and listscal
void MainWindow::degree_list() {
  if (!last_chart_) {
    return;
  }
  const ChartSettings s = current_settings();
  bool cusps = false;
  // his IF hrg! = 0 && haw& < 8 before "ZWISCHEN - HÄUSER HINZUNEHMEN ?"
  if (!s.heliocentric && houses_->currentIndex() < 7 && last_chart_->houses.ok) {
    const int r = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("ZWISCHEN - HÄUSER HINZUNEHMEN ?")},
                                    {tr(" JA "), tr(" NEIN "), tr("ABBRUCH")}, 1);
    if (r < 0 || r == 2) {
      return;
    }
    cusps = r == 0;
  }
  const std::vector<GradEntry> plain = grad_list(*last_chart_, s, cusps);
  // his @sort(1,mmax&) over ce%(m&) = FIX(10000 * w)
  std::vector<GradEntry> sorted = plain;
  std::stable_sort(sorted.begin(), sorted.end(), [](const GradEntry& a, const GradEntry& b) {
    return std::trunc(a.deg * 10000.0) < std::trunc(b.deg * 10000.0);
  });
  // his "Grad-Liste |" + LEFT$(na$,20) + "|" + dm$ + ": " + datum3$ + " |" + sol$
  GradSheetText text;
  const ClassicSheetText sheet = classic_sheet_text();
  const QString label = rhythm_chart_label();
  text.title = (tr("Grad-Liste |") + QString::fromStdString(sheet.name).left(20) + "|" + tr("Datum") + ": " +
                datum3_text(calendar_date(last_chart_->jd_ut, s.calendar)) + " |" + label)
                   .toStdString();
  text.frame = horgt_text().toStdString();
  text.ephem = gena2_text().toStdString();
  // the heads of his listscal
  text.panel_head = tr("Gesamt -").toStdString();
  text.panel_sub = tr("Verteilung :").toStdString();
  const int unit = grad_unit(s);
  const int pages = std::max(1, static_cast<int>((plain.size() + kGradPerPage - 1) / kGradPerPage));

  QDialog view(this);
  mark_output(&view, menu_item::kDegreeList);
  // his mend1$, the menu caption as the window title
  view.setWindowTitle(tr("GRAD-LISTE G/H"));
  auto* v = new QVBoxLayout(&view);
  v->setContentsMargins(0, 0, 0, 0);
  auto* canvas = new WheelWidget(&view);
  v->addWidget(canvas);
  bool sorted_mode = false;
  int page = 0;
  const auto draw = [&]() {
    canvas->set_plain_list(build_grad_sheet(sorted_mode ? sorted : plain, page, text, sorted_mode, unit));
  };
  // his wart between the pages, Space, Enter, PgUp and PgDn go on, ESC ends
  LambdaFilter keys(LambdaFilter::keys([&](int key) {
    if (key == Qt::Key_Escape) {
      view.reject();
      return true;
    }
    if (page + 1 < pages) {
      ++page;
      draw();
      return true;
    }
    if (sorted_mode) {
      // his GOTO grli, the sorted list starts over
      page = 0;
      draw();
      return true;
    }
    // his SWITCH asc&, CASE 13,32 asks, any other key ends
    if (key != Qt::Key_Space && key != Qt::Key_Return && key != Qt::Key_Enter) {
      view.reject();
      return true;
    }
    const int r = ChoiceDialog::ask(&view, tr("AUSWAHL"), {tr("Nach Länge SORTIEREN ?"), tr("und VERTEILUNG DARSTELLEN ?")},
                                    {tr(" JA "), tr(" NEIN "), tr("ABBRUCH")}, 0);
    if (r < 0 || r == 2) {
      view.reject();
      return true;
    }
    sorted_mode = r == 0;
    page = 0;
    draw();
    return true;
  }));
  view.installEventFilter(&keys);
  canvas->installEventFilter(&keys);
  draw();
  view.resize(size());
  view.exec();
}

// ported from hsa0 and mondph$
QStringList MainWindow::hsa0_lines(const Chart& chart) const {
  const ClassicSheetText txt = classic_sheet_text();
  const auto hms = [](double hours) {
    int s = static_cast<int>(std::lround(norm_deg(hours * kDegPerHour) / kDegPerHour * 3600.0)) % kSecondsPerDay;
    return QString::asprintf("%2dh %02dm %02ds", s / 3600, (s / 60) % 60, s % 60);
  };
  QStringList out;
  // s$ = sol$(od,ze), the kind of chart, the name follows
  out << rhythm_chart_label() + tr(" Name: %1 | Ort: %2").arg(QString::fromStdString(txt.name).trimmed(),
                                                 QString::fromStdString(txt.place).trimmed());
  // his lj$ = " = " + STR$((jd(od,ze) - jd(1,ze)) / tja,6,2) + " LJ"
  QString lj;
  const ChartInput birth = radix_input();
  const double jd_birth = julian_day(birth.date_ut, current_settings().calendar);
  if (std::abs(chart.jd_ut - jd_birth) > kEps) {
    lj = QString::asprintf(" = %6.2f LJ", (chart.jd_ut - jd_birth) / chart.ta.tropical_year_days);
  }
  out << QString::fromStdString(txt.date).trimmed() + " | " + tr("Zeit (UT): ") +
             QString::fromStdString(txt.ut).trimmed().remove("UT:").trimmed() + lj + " | " +
             QString::fromStdString(txt.weekday).trimmed();
  // his t1 = (jd - 2415020) / 36525 of the UT moment his a31 stored
  out << tr("STZ 0H GRW =%1| ARMC=%2|JD=%3|T=%4")
             .arg(hms(chart.h0), hms(chart.armc_deg / kDegPerHour))
             .arg(chart.jd_ut, 0, 'f', 5)
             .arg((chart.jd_ut - kJdEpoch1900) / kDaysPerCentury, 16, 'f', 13);
  QString phase;
  const BodyState& sun = chart.b[body::kSun];
  const BodyState& moon = chart.b[body::kMoon];
  if (sun.valid && moon.valid && !current_settings().heliocentric) {
    // his mondph$, the elongation of the moon and its lit share
    const double el = norm_deg((moon.el - sun.el) * kRadToDeg);
    const int e = static_cast<int>(std::lround(el));
    const int share = e <= 180 ? 100 * e / 180 : 100 * (360 - e) / 180;
    QString what = tr(" Abnehmend");
    if (e == 0 || e == 360) {
      what = tr("  Neumond");
    } else if (e < 180) {
      what = tr(" Zunehmend");
    } else if (e == 180) {
      what = tr(" Vollmond");
    }
    phase = tr("Mond-Phase:%1° = %2%%3").arg(el, 5, 'f', 1).arg(share, 3).arg(what);
  }
  out << QString::fromStdString(txt.lon).trimmed() + " |" + QString::fromStdString(txt.lat).trimmed() + "| " + phase;
  return out;
}

// ported from hausa with the hsa0 header
void MainWindow::house_table() {
  if (!last_chart_) {
    return;
  }
  const HouseSystem sys = current_settings().houses;
  // his IF haw& < 9, Kein Häusersystem gewählt !
  if (sys >= HouseSystem::kNone || !last_chart_->houses.ok) {
    QMessageBox::information(this, "HORCOM", tr("Kein Häusersystem gewählt !"));
    return;
  }
  // hausa sets horm& = 1, the table always reads the ecliptic frame
  Chart chart = *last_chart_;
  if (mundane_frame_) {
    const Chart plain = compute_chart(current_input(), current_settings(), vsop_, eph_);
    if (plain.ok) {
      chart = plain;
    }
  }
  QDialog dialog(this);
  mark_output(&dialog, menu_item::kHouseTable);
  const QString haus = QString::fromUtf8(chart.houses.name.data(), static_cast<int>(chart.houses.name.size())).trimmed();
  dialog.setWindowTitle(tr("Häuser-Tabelle (%1)").arg(haus));
  auto* v = new QVBoxLayout(&dialog);
  for (const QString& line : hsa0_lines(chart)) {
    v->addWidget(new QLabel(line, &dialog));
  }
  // his Häuserspitzen nach System, In wahrer ekliptikaler Länge u. AR
  auto* title = new QLabel(tr("Häuserspitzen nach System %1 In wahrer ekliptikaler Länge u. AR").arg(haus), &dialog);
  QFont bold = title->font();
  bold.setBold(true);
  title->setFont(bold);
  v->addWidget(title);
  const bool placidus = sys == HouseSystem::kPlacidus;
  // his ah& = 13, 14 for Äqual with the MC row, 15 for Vehlow with AC and MC
  const int rows = sys == HouseSystem::kEqualVehlow ? 15 : (sys == HouseSystem::kEqualAsc ? 14 : 13);
  auto* table = new QTableWidget(rows, placidus ? 6 : 4, &dialog);
  QStringList heads{tr("Haus"), tr("Länge"), "AR", tr("Dekl.")};
  if (placidus) {
    // his AO und Polhöhe
    heads << "AO" << tr("Polhöhe");
  }
  table->setHorizontalHeaderLabels(heads);
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  const double armcb = chart.armc_deg * kDegToRad;
  // his aeqh, the axis rows carry his labels, the equal systems name them
  // as houses and list the true angles in rows of their own
  const bool vehlow = sys == HouseSystem::kEqualVehlow;
  const bool equal = sys == HouseSystem::kEqualAsc || vehlow;
  const QString h1 = vehlow ? "H1" : "AC";
  const QString h4 = equal ? "H4" : "IC";
  const QString h7 = vehlow ? "H7" : "DC";
  const QString h10 = equal ? "H10" : "MC";
  for (int t = 1; t <= rows; ++t) {
    const bool vertex = t == 13;
    double pa = vertex ? chart.houses.angles.vertex : (t <= 12 ? chart.houses.cusp[static_cast<std::size_t>(t)] : 0.0);
    QString name = QString("H%1").arg(t);
    if (t == 1) {
      name = h1;
    } else if (t == 4) {
      name = h4;
    } else if (t == 7) {
      name = h7;
    } else if (t == 10) {
      name = h10;
    } else if (vertex) {
      name = "VERTEX";
    } else if (t == 14) {
      name = vehlow ? "AC" : "MC";
      pa = vehlow ? chart.houses.angles.ac : chart.houses.angles.mc;
    } else if (t == 15) {
      name = "MC";
      pa = chart.houses.angles.mc;
    }
    table->setItem(t - 1, 0, new QTableWidgetItem(name));
    table->setItem(t - 1, 1, new QTableWidgetItem(zodiac(pa) + QString::asprintf("  = %8.3f°", pa * kRadToDeg)));
    const Equatorial eq = ecliptic_to_equatorial(pa, 0.0, chart.ekls0);
    table->setItem(t - 1, 2, new QTableWidgetItem(QString::asprintf("%8.3f°", eq.ra * kRadToDeg)));
    table->setItem(t - 1, 3, new QTableWidgetItem(QString::asprintf("%8.3f°", eq.dec * kRadToDeg)));
    if (placidus && t <= 12) {
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

// ported from the right mouse selection of the chart screen, single
// planets red or alone, the ruler highlight and the aspect line choice.
// The rows group by Zusatz, Asteroiden, Planetoiden, Andere Elemente and
// the Hamburger Faktoren so every extra body reaches the wheel from one
// screen
void MainWindow::planet_selection() {
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Planeten-Auswahl"));
  auto* v = new QVBoxLayout(&dialog);
  auto* table = new QTableWidget(0, 3, &dialog);
  table->setHorizontalHeaderLabels({tr("Punkt"), tr("Zeigen"), tr("Rot")});
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(22);
  // clicks on the checkbox rows should never open an editor, the stock
  // double-click edit trigger is what makes stock QTableWidgetItem
  // checkboxes swallow rapid clicks
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  // the Punkt column stretches to the dialog width, the two checkbox
  // columns hug their indicator so no dead space rides beside the tick
  auto* header = table->horizontalHeader();
  header->setSectionResizeMode(0, QHeaderView::Stretch);
  header->setSectionResizeMode(1, QHeaderView::Fixed);
  header->setSectionResizeMode(2, QHeaderView::Fixed);
  // wide enough for the German header text "Zeigen" without truncation
  const int check_col_width = 68;
  table->setColumnWidth(1, check_col_width);
  table->setColumnWidth(2, check_col_width);

  // Section headers span the row like his group captions on the panel.
  // A body slot below the previous is off by default until the tester
  // opts it in, the main planets are always included so they only carry
  // the show and red switches. The kExtra flag marks slots whose
  // include state flows into the settings (see current_settings). The
  // eight Hamburger Faktoren travel as one collective row like his mockup
  // shows, the row's slot is 0 and the group_hamburg flag rules the group
  struct Row {
    int slot = -1;      // -1 means header, 0 means Hamburger group
    QString label;
    bool extra = false;
  };
  // the main planets and the three extra planets lead without a heading,
  // the further elements stand under their group titles
  const std::vector<Row> rows = {
      {body::kSun, {}, false},
      {body::kMoon, {}, false},
      {body::kMercury, {}, false},
      {body::kVenus, {}, false},
      {body::kMars, {}, false},
      {body::kJupiter, {}, false},
      {body::kSaturn, {}, false},
      {body::kUranus, {}, false},
      {body::kNeptune, {}, false},
      {body::kPluto, {}, false},
      {body::kNodeAsc, {}, false},
      {body::kNodeDesc, {}, false},
      {body::kApogee, {}, true},
      {body::kChiron, {}, true},
      {body::kQuaoar, {}, true},
      {body::kXena, {}, true},
      {-1, tr("Asteroiden"), false},
      {body::kCeres, {}, true},
      {body::kPallas, {}, true},
      {body::kJuno, {}, true},
      {body::kVesta, {}, true},
      {-1, tr("Planetoiden"), false},
      {body::kPholus, {}, true},
      {body::kDamokles, {}, true},
      {body::kNessus, {}, true},
      {-1, tr("Andere Elemente"), false},
      {body::kHalley, {}, true},
      {body::kTranspluto, {}, true},
      {body::kFortune, {}, true},
      {0, tr("Hamburger Planeten"), true},
  };
  // his yellow @checkBg@ shade for the section headers, the same tone
  // the dock titles carry
  const QColor header_bg(0xFF, 0xFB, 0xC8);
  const QColor header_fg(0x2E, 0x33, 0x38);
  // the eight Hamburg factors, his row from Cupido to Poseidon, carry
  // only one collective button in the panel
  static const std::array<int, 8> kHamburger = {body::kCupido, body::kHades,   body::kZeus,     body::kKronos,
                                                body::kApollon, body::kAdmetos, body::kVulkanus, body::kPoseidon};
  // real QCheckBox widgets ride in the two checkbox columns so every
  // click reaches a live indicator without a double-click swallow, and
  // the yellow @checkBg@ shape from QCheckBox::indicator carries through
  // to the table cells. A centred wrapper keeps the tick in the middle
  // of the column without expanding the click target beyond the visible
  // indicator
  auto make_check_cell = [](QWidget* parent, bool on) {
    auto* wrap = new QWidget(parent);
    auto* lay = new QHBoxLayout(wrap);
    lay->setContentsMargins(0, 0, 0, 0);
    auto* box = new QCheckBox(wrap);
    box->setChecked(on);
    box->setFocusPolicy(Qt::NoFocus);
    lay->addWidget(box, 0, Qt::AlignCenter);
    return std::pair<QWidget*, QCheckBox*>{wrap, box};
  };
  for (const Row& r : rows) {
    const int row = table->rowCount();
    table->insertRow(row);
    if (r.slot < 0) {
      auto* head = new QTableWidgetItem(r.label);
      head->setFlags(Qt::ItemIsEnabled);
      // -1 in UserRole marks a header so the accept loop skips it
      head->setData(Qt::UserRole, -1);
      QFont f = head->font();
      f.setBold(true);
      head->setFont(f);
      head->setBackground(header_bg);
      head->setForeground(header_fg);
      table->setItem(row, 0, head);
      table->setSpan(row, 0, 1, 3);
      continue;
    }
    const int slot = r.slot;
    const QString label =
        r.label.isEmpty() ? QString::fromUtf8(body::kName[static_cast<std::size_t>(slot)].data(),
                                              static_cast<int>(body::kName[static_cast<std::size_t>(slot)].size()))
                          : r.label;
    auto* name = new QTableWidgetItem(label);
    // slot 0 marks the Hamburger group row, the eight factors ride under it
    name->setData(Qt::UserRole, slot);
    name->setData(Qt::UserRole + 1, r.extra);
    name->setFlags(Qt::ItemIsEnabled);
    table->setItem(row, 0, name);
    // the main planets read the emphasis, the extras additionally lean
    // on the per slot included flag so an off row leaves them
    // uncomputed like his nk zero. The Hamburger row's state derives
    // from the eight factors, ANY on lights the row
    bool on = false;
    if (slot == 0) {
      for (int h : kHamburger) {
        if (included_[static_cast<std::size_t>(h)]) {
          on = true;
          break;
        }
      }
    } else {
      on = emphasis_[static_cast<std::size_t>(slot)] >= 0;
      if (r.extra) {
        on = on && included_[static_cast<std::size_t>(slot)];
      }
    }
    auto [shown_cell, shown_box] = make_check_cell(table, on);
    table->setCellWidget(row, 1, shown_cell);
    // stash the checkbox pointer on the name item so the accept loop can
    // read it back without another cellWidget lookup
    name->setData(Qt::UserRole + 2, QVariant::fromValue<void*>(shown_box));
    bool red_on = false;
    if (slot == 0) {
      // Hamburger group red = any factor is red
      for (int h : kHamburger) {
        if (emphasis_[static_cast<std::size_t>(h)] > 0) {
          red_on = true;
          break;
        }
      }
    } else {
      red_on = emphasis_[static_cast<std::size_t>(slot)] > 0;
    }
    auto [red_cell, red_box] = make_check_cell(table, red_on);
    table->setCellWidget(row, 2, red_cell);
    name->setData(Qt::UserRole + 3, QVariant::fromValue<void*>(red_box));
  }
  auto* ruler = new QCheckBox(tr("Geburtsherrscher rot hervorheben"), &dialog);
  ruler->setChecked(ruler_red_);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  auto* reset = buttons->addButton(tr("Zurücksetzen"), QDialogButtonBox::ResetRole);
  connect(reset, &QPushButton::clicked, &dialog, [this, &dialog]() {
    emphasis_.fill(0);
    included_.fill(false);
    ruler_red_ = false;
    dialog.reject();
    recompute();
  });
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  v->addWidget(table, 1);
  v->addWidget(ruler);
  v->addWidget(buttons);
  dialog.resize(360, 900);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  for (int row = 0; row < table->rowCount(); ++row) {
    QTableWidgetItem* name = table->item(row, 0);
    if (name == nullptr || name->data(Qt::UserRole).toInt() < 0) {
      continue;
    }
    const int slot = name->data(Qt::UserRole).toInt();
    const bool extra = name->data(Qt::UserRole + 1).toBool();
    auto* shown_box = static_cast<QCheckBox*>(name->data(Qt::UserRole + 2).value<void*>());
    auto* red_box = static_cast<QCheckBox*>(name->data(Qt::UserRole + 3).value<void*>());
    const bool zeigen = shown_box != nullptr && shown_box->isChecked();
    const bool rot = red_box != nullptr && red_box->isChecked();
    int e = 0;
    if (!zeigen) {
      e = -1;
    } else if (rot) {
      e = 1;
    }
    if (slot == 0) {
      // the Hamburger group row fans the state to all eight factors
      for (int h : kHamburger) {
        emphasis_[static_cast<std::size_t>(h)] = e;
        included_[static_cast<std::size_t>(h)] = zeigen;
      }
      if (hamburg_ != nullptr) {
        const QSignalBlocker b(hamburg_);
        hamburg_->setChecked(zeigen);
      }
    } else {
      emphasis_[static_cast<std::size_t>(slot)] = e;
      if (extra) {
        included_[static_cast<std::size_t>(slot)] = zeigen;
      }
      // AG mirrors the panel Schwarzer Mond, so the two rows stay
      // in step whichever screen the tester used to change it
      if (slot == body::kApogee && apogee_show_ != nullptr) {
        const QSignalBlocker b(apogee_show_);
        apogee_show_->setChecked(zeigen);
      }
    }
  }
  ruler_red_ = ruler->isChecked();
  recompute();
}

namespace {

// the dress of avh9 and the ASPEKT-LINIEN screen, the render library
// derives it from the settings like the defaults of every wheel
void apply_ring_dress(const Konsta& k, WheelOptions& w) {
  const WheelDress d = konsta_dress(k);
  w.chords = d.chords;
  w.hist_colors = d.hist_colors;
  w.ring_colors = d.ring_colors;
  w.own_colors = d.own_colors;
  w.ring_fill = d.ring_fill;
  w.hist_fill = d.hist_fill;
  w.colored_signs = d.colored_signs;
  w.outer_color = d.outer_color;
}

}  // namespace

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
  auto* haus1 = new QCheckBox(tr("PUNKTE im 1. HAUS DOPPELT"), &dialog);
  haus1->setChecked(konsta_.haus1_dop);
  auto* herr = new QCheckBox(tr("1. GEBURTSHERRSCHER DOPPELT"), &dialog);
  herr->setChecked(konsta_.gebherr_dop);
  //RR PUNKTE-WERT 0....9 EINGEBEN !
  auto* weights = new QTableWidget(1, 15, &dialog);
  QStringList heads;
  for (int i = 1; i <= 14; ++i) {
    heads << QString::fromUtf8(body::kName[static_cast<std::size_t>(i)].data(),
                               static_cast<int>(body::kName[static_cast<std::size_t>(i)].size()));
  }
  heads << tr("Zusatz");
  weights->setHorizontalHeaderLabels(heads);
  weights->verticalHeader()->setVisible(false);
  {
    const auto points = histogram_points(konsta_.pn);
    for (int i = 1; i <= 14; ++i) {
      weights->setItem(0, i - 1, new QTableWidgetItem(QString::number(points[static_cast<std::size_t>(i)])));
    }
    weights->setItem(0, 14, new QTableWidgetItem(QString::number(points[body::kChiron])));
    weights->resizeColumnsToContents();
    fit_columns(weights);
    weights->setFixedHeight(weights->horizontalHeader()->sizeHint().height() + weights->rowHeight(0) +
                            2 * weights->frameWidth() + weights->horizontalScrollBar()->sizeHint().height());
  }
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  //RR ZIFFER von 0 bis 9
  const auto edited_row = [weights]() {
    std::array<int, 16> pn{};
    for (int i = 1; i <= 15; ++i) {
      bool ok = false;
      const int w = weights->item(0, i - 1)->text().toInt(&ok);
      pn[static_cast<std::size_t>(i)] = ok ? std::clamp(w, 0, 9) : 0;
    }
    return pn;
  };
  const auto refresh = [this, table, haus1, herr, edited_row]() {
    HistogramOptions opt;
    opt.points = histogram_points(edited_row());
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
    // the bars wear elem_col like elemhist1 and kard_fix_gem1, the own
    // colours under EIGENE FARBEN, the qualities the first three. The
    // family reads the mutable signs as Veränderlich instead of his
    // Gemischt
    WheelOptions dress;
    apply_ring_dress(konsta_, dress);
    const auto bar_color = [&dress](int e) {
      const Rgb c = dress.ring_colors[static_cast<std::size_t>(e)];
      // pure cyan drowns on white, it rides darker like on the A4 sheet
      return c == 0x00FFFF ? QColor(0x00, 0xC8, 0xC8) : QColor((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
    };
    const char* names[7] = {"Feuer", "Erde", "Luft", "Wasser", "Kardinal", "Fix", "Veränderlich"};
    for (int r = 0; r < 7; ++r) {
      table->setItem(r, 0, new QTableWidgetItem(tr(names[r])));
      const bool element = r < 4;
      const int idx = element ? r + 1 : r - 3;
      const int sign = element ? h.element_sign[static_cast<std::size_t>(idx)] : h.quality_sign[static_cast<std::size_t>(idx)];
      const int house = element ? h.element_house[static_cast<std::size_t>(idx)] : h.quality_house[static_cast<std::size_t>(idx)];
      auto* sign_item = new QTableWidgetItem(bar(sign));
      auto* house_item = new QTableWidgetItem(h.houses_counted ? bar(house) : tr("-"));
      const QColor col = bar_color(idx);
      sign_item->setForeground(col);
      house_item->setForeground(col);
      table->setItem(r, 1, sign_item);
      table->setItem(r, 2, house_item);
    }
    table->resizeColumnsToContents();
    fit_columns(table);
  };
  connect(haus1, &QCheckBox::toggled, &dialog, refresh);
  connect(herr, &QCheckBox::toggled, &dialog, refresh);
  connect(weights, &QTableWidget::cellChanged, &dialog, [refresh](int, int) { refresh(); });
  refresh();
  v->addWidget(table, 1);
  v->addWidget(haus1);
  v->addWidget(herr);
  v->addWidget(weights);
  v->addWidget(buttons);
  dialog.resize(560, 520);
  dialog.exec();
  // punkte_pla keeps the weights and both switches for the session, the
  // A4 sheet counts with them
  konsta_.pn = edited_row();
  konsta_.haus1_dop = haus1->isChecked();
  konsta_.gebherr_dop = herr->isChecked();
  //RR @kon_dsp
  persist_konsta();
}

void MainWindow::linear_graph() {
  linear_graph_of(-1);
}

// the tester's own entry of the LINEAR-GRAPHIK, his a18asw reached it
// from TRANSITE and the direction entries, here the kind comes first
void MainWindow::linear_graph_of(int preset_kind) {
  int kind = preset_kind;
  if (kind < 0) {
    kind = ChoiceDialog::ask(this, tr("LINEAR-GRAPHIK"), {},
                             {tr("TRANSITE"), tr("SEKUNDÄR-DIREKTION"), tr("SONNEN-BOGEN-DIREKTION"),
                              tr("MOND-BOGEN-DIREKTION"), tr("ABBRUCH")},
                             0);
  }
  switch (kind) {
    case 0: linear_run(A18Mode::kTransits); return;
    case 1: linear_run(A18Mode::kSecondary); return;
    case 2: linear_run(A18Mode::kSunArc); return;
    case 3: linear_run(A18Mode::kMoonArc); return;
    default: return;
  }
}

namespace {

// the objects of ausw_pl_hs under aufg!, only real bodies take a row
struct RiseObject {
  int slot;
  const char* caption;
};

constexpr RiseObject kRiseObjects[] = {
    {body::kSun, QT_TRANSLATE_NOOP("horcom::MainWindow", " SONNE")},
    {body::kMoon, QT_TRANSLATE_NOOP("horcom::MainWindow", " MOND")},
    {body::kMercury, QT_TRANSLATE_NOOP("horcom::MainWindow", " MERKUR")},
    {body::kVenus, QT_TRANSLATE_NOOP("horcom::MainWindow", " VENUS")},
    {body::kMars, QT_TRANSLATE_NOOP("horcom::MainWindow", " MARS")},
    {body::kJupiter, QT_TRANSLATE_NOOP("horcom::MainWindow", " JUPITER")},
    {body::kSaturn, QT_TRANSLATE_NOOP("horcom::MainWindow", " SATURN")},
    {body::kUranus, QT_TRANSLATE_NOOP("horcom::MainWindow", " URANUS")},
    {body::kNeptune, QT_TRANSLATE_NOOP("horcom::MainWindow", " NEPTUN")},
    {body::kPluto, QT_TRANSLATE_NOOP("horcom::MainWindow", " PLUTO")},
    {body::kChiron, QT_TRANSLATE_NOOP("horcom::MainWindow", " CHIRON         a=13.61  e=0.38  i=6.94°  T=50 Jahre ")},
    {body::kCeres, QT_TRANSLATE_NOOP("horcom::MainWindow", " CERES")},
    {body::kPallas, QT_TRANSLATE_NOOP("horcom::MainWindow", " PALLAS")},
    {body::kJuno, QT_TRANSLATE_NOOP("horcom::MainWindow", " JUNO")},
    {body::kVesta, QT_TRANSLATE_NOOP("horcom::MainWindow", " VESTA")},
    {body::kQuaoar, QT_TRANSLATE_NOOP("horcom::MainWindow", " QUAOAR         a=43.25 e=0.035 i=  7.99°   T=284 Jr.")},
    {body::kHalley, QT_TRANSLATE_NOOP("horcom::MainWindow", " KOMET HALLEY   a=17.94 e=0.97  i=162.24°   T= 76 Jr.")},
    {body::kPholus, QT_TRANSLATE_NOOP("horcom::MainWindow", " PHOLUS         a=20.23 e=0.57  i= 24.70°   T= 91 Jr.")},
    {body::kDamokles, QT_TRANSLATE_NOOP("horcom::MainWindow", " DAMOKLES       a=11.82 e=0.87  i= 61.84°   T= 41 Jr.")},
    {body::kNessus, QT_TRANSLATE_NOOP("horcom::MainWindow", " NESSUS         a=24.46 e=0.52  i= 15.66°   T=121 Jr.")},
    {body::kXena, QT_TRANSLATE_NOOP("horcom::MainWindow", " XENA           a=67.66 e=0.44  i= 44.12°   T=557 Jr.")},
};

//RR di& blocks of 64 pixels, five to a screen
constexpr int kRiseBlocks = 5;
// five text lines per block and one line of his @line under it
constexpr int kRiseLines = 6;

}  // namespace

// ported from auf_unt with auf_unt3, auf_pl and ausw_pl_hs. A single body
// walks five days per screen, all real bodies stand at one date five to
// a screen, the columns hold rising, meridian passage and setting
void MainWindow::rise_set() {
  // his ea$ = "Für ALLE vorgewählen ECHTEN Planeten  :  FESTES Datum "
  const QString ea = tr("Für ALLE vorgewählen ECHTEN Planeten  :  FESTES Datum ");
  const int ea_answer =
      ChoiceDialog::ask(this, tr("AUSWAHL"),
                        {tr("Liste für EINZELNEN Planeten bei LAUFENDEM Datum ?"), tr("oder"), ea,
                         tr("KEINE HYPOTHETISCHEN Planeten verwenden ! ! !")},
                        {tr("EINZELNER Planet  :  Datum LAUFEND"), ea}, 0);
  if (ea_answer < 0) {
    return;
  }
  const bool single = ea_answer == 0;
  // his WAHRE POSITION ? oder SCHEINBARE POSITION ?
  const int ws = ChoiceDialog::ask(this, tr("AUSWAHL"),
                                   {QString(), tr("WAHRE POSITION ?"), tr("oder"), tr("SCHEINBARE POSITION ?")},
                                   {tr("WAHR"), tr("SCHEINBAR")}, 0);
  if (ws < 0) {
    return;
  }
  const bool true_position = ws == 0;

  // his et$(1) = "AUFGANG.....", @eingabe(0,-1,1,14), place and date only
  AafRecord seed = panel_record();
  seed.surname = "AUFGANG.....";
  seed.given.clear();
  RecordMaskDialog mask(seed, tr("EREIGNIS-ORT und DATUM EINGEBEN !  ->  EVENTL. TAB - TASTE !"),
                        RecordMaskDialog::Mode::kShow, data_dir_, this);
  mask.limit_fields(14);
  if (mask.exec() != QDialog::Accepted) {
    return;
  }
  const AafRecord where = mask.record();
  const ChartInput at = record_input(where);
  const Calendar cal = current_settings().calendar;
  // his CLR ho,mi, the day starts at 0h UT
  const CalendarDate day0{where.day, where.month, where.year, 0.0, 0.0};
  double day_jd = julian_day(day0, cal);
  // his juld with juld1 and somo left the obliquity of the entered date
  // for the test IF ABS(gg) > 90 - ekls * up
  const double limit = polar_limit_deg(somo(time_arguments(day_jd), day0).ekls);
  if (std::abs(at.lat_deg) > limit) {
    QMessageBox::information(this, "HORCOM", tr("Geog. Breite zu groß ! Nur bis  +- %1° >> ABBRECHEN !").arg(limit, 6, 'f', 3));
    return;
  }

  // the real bodies on offer, the extras only when chosen in PLANETEN-AUSWAHL
  const ChartSettings settings = current_settings();
  const auto chosen = [&settings](int slot) {
    return slot <= body::kPluto || std::find(settings.nk.begin(), settings.nk.end(), slot) != settings.nk.end();
  };
  std::vector<int> bodies;
  std::vector<std::pair<int, QString>> rows;
  for (int slot = body::kSun; slot < body::kSlotCount; ++slot) {
    const auto it = std::find_if(std::begin(kRiseObjects), std::end(kRiseObjects),
                                 [slot](const RiseObject& o) { return o.slot == slot; });
    if (it != std::end(kRiseObjects) && chosen(slot)) {
      bodies.push_back(slot);
      rows.emplace_back(slot, tr(it->caption));
    } else {
      // his IF aufg!, the angles, points and hypothetical bodies read s$
      rows.emplace_back(-1, QString());
    }
  }
  int slot = body::kSun;
  if (single) {
    const std::optional<int> pick = ask_object(this, rows);
    if (!pick) {
      return;
    }
    slot = *pick;
  }

  SearchContext ctx = make_context();
  ctx.base.lon_deg_east = at.lon_deg_east;
  ctx.base.lat_deg = at.lat_deg;
  const auto tag = [](int s) {
    const std::string_view n = body::kName[static_cast<std::size_t>(s)];
    return QString::fromUtf8(n.data(), static_cast<qsizetype>(n.size()));
  };

  QDialog dialog(this);
  mark_output(&dialog, menu_item::kRiseSet);
  dialog.setWindowTitle(tr("* AUFGANG.........| WEITER mit Leertaste ! | ENDE mit 'ESC' !"));
  auto* v = new QVBoxLayout(&dialog);
  auto* heads = new QHBoxLayout();
  // his plein2 beside every heading when one body runs
  const QString marker = single ? "  " + tag(slot) : QString();
  std::vector<QLabel*> notes;
  for (const QString& h : {tr("AUFGANG"), tr("MERIDIAN-DURCHGANG"), tr("UNTERGANG")}) {
    notes.push_back(new QLabel(h + marker, &dialog));
    heads->addWidget(notes.back(), 1);
  }
  v->addLayout(heads);
  auto* table = new QTableWidget(kRiseBlocks * kRiseLines, 3, &dialog);
  table->horizontalHeader()->setVisible(false);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(19);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  table->setShowGrid(false);
  table->setItemDelegate(new ZodiacDelegate(table));
  table->setFont(theme::mono_font());
  v->addWidget(table, 1);
  // his w$ + " | Zeiten in UT ( GMT )"
  notes.push_back(new QLabel((true_position ? tr(" Wahre Werte ") : tr(" Scheinbare Werte ")) + tr(" | Zeiten in UT ( GMT )"),
                             &dialog));
  v->addWidget(notes.back(), 0, Qt::AlignHCenter);
  // his go$ + " | " + a$ + " | " + b$, Ortsname NN without a place
  QString go = QString::fromStdString(where.place).trimmed();
  if (go.isEmpty()) {
    go = tr("Ortsname NN ");
  }
  notes.push_back(new QLabel(QString("%1 | %2%3 %4 | %5%6 %7")
                                 .arg(go, tr("Geog. Länge :"), grmi_text(at.lon_deg_east, 1),
                                      at.lon_deg_east < 0.0 ? "W" : "E", tr("Geog. Breite:"),
                                      grmi_text(at.lat_deg, 1), at.lat_deg < 0.0 ? "S" : "N"),
                             &dialog));
  v->addWidget(notes.back(), 0, Qt::AlignHCenter);
  // his three columns filled the display, the text grows with the window
  auto* zoom = new TableZoom(table, true);
  for (QLabel* l : notes) {
    zoom->follow(l);
  }
  auto* keys = new QHBoxLayout();
  // his Weiter mit Leertaste  |  Zurück mit 'R' |  Ende mit 'ESC'
  auto* next = new QPushButton(tr("Weiter mit Leertaste"), &dialog);
  auto* back = new QPushButton(tr("Zurück mit 'R'"), &dialog);
  auto* quit = new QPushButton(tr("Ende mit 'ESC'"), &dialog);
  keys->addWidget(next);
  keys->addWidget(back);
  keys->addStretch(1);
  keys->addWidget(quit);
  v->addLayout(keys);

  std::size_t first = 0;
  // ported from auf_unt3, one block of five lines
  const auto block = [&](int b, int col, int s, const RiseSetMoment& mt) {
    const int row = b * kRiseLines;
    if (!mt.ok) {
      return;
    }
    // his b$ = ze1$ for MO and JU to PL and for SCHEINBAR, ze$ with seconds else
    const bool seconds = true_position && s != body::kMoon && !(s >= body::kJupiter && s <= body::kPluto);
    const double step = seconds ? kSecondsPerDay : kMinutesPerDay;
    const CalendarDate d = calendar_date(std::floor(mt.jd_ut * step + 0.5) / step, cal);
    const int total = static_cast<int>(std::lround((d.hour * 60.0 + d.minute) * 60.0));
    // his datum2$ + " | " + b$
    const QString clock = seconds ? QString::asprintf("%2dh %2dm %2ds", total / 3600, (total / 60) % 60, total % 60)
                                  : QString::asprintf("%2dh %3dm", total / 3600, (total / 60) % 60);
    table->setItem(row, col, new QTableWidgetItem(QString::asprintf("%2d.%2d.%4d | ", d.day, d.month, d.year) + clock));
    table->setItem(row + 1, col, new QTableWidgetItem(tr("Sternz.(GRW.)= %1°").arg(mt.gst_deg, 7, 'f', 3)));
    QTableWidgetItem* len = zodiac_item(mt.el);
    len->setText(tr("Länge   = ") + tag(s) + " " + len->text());
    table->setItem(row + 2, col, len);
    table->setItem(row + 3, col, new QTableWidgetItem(tr("Breite  = ") + (mt.eb < 0.0 ? "-" : "") +
                                                      grmise_text(mt.eb * kRadToDeg)));
    const double de = mt.de * kRadToDeg;
    table->setItem(row + 4, col, new QTableWidgetItem("AR=" + grmise_text(norm_rad(mt.ar) * kRadToDeg) + " DE=" +
                                                      (de < 0.0 ? "-" + grmi_text(de, 0).trimmed() : grmi_text(de, 0))));
  };
  const auto fill = [&]() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    table->clearContents();
    for (int b = 0; b < kRiseBlocks; ++b) {
      const std::size_t index = first + static_cast<std::size_t>(b);
      if (!single && index >= bodies.size()) {
        break;
      }
      const int s = single ? slot : bodies[index];
      const double day = single ? day_jd + b : day_jd;
      const RiseSet rs = rise_transit_set(day, s, true_position, ctx);
      if (rs.circumpolar) {
        // his AUßER BEREICH ! box ended the whole screen, the port marks
        // the block and goes on with the next day or body
        for (int col = 0; col < 3; ++col) {
          table->setItem(b * kRiseLines, col, new QTableWidgetItem(tag(s) + "  " + tr("AUßER BEREICH !")));
        }
        continue;
      }
      if (!rs.ok) {
        continue;
      }
      block(b, 0, s, rs.rise);
      block(b, 1, s, rs.transit);
      block(b, 2, s, rs.set);
    }
    QApplication::restoreOverrideCursor();
  };
  // his INC jd, the next five days, or the next five bodies from the first
  // again after the last
  const auto forward = [&]() {
    if (single) {
      day_jd += kRiseBlocks;
    } else {
      first += kRiseBlocks;
      if (first >= bodies.size()) {
        first = 0;
      }
    }
    fill();
  };
  // his SUB jd,9 after the fifth day, or v& = aa&
  const auto backward = [&]() {
    if (single) {
      day_jd -= kRiseBlocks;
    } else {
      first = 0;
    }
    fill();
  };
  connect(next, &QPushButton::clicked, &dialog, forward);
  connect(back, &QPushButton::clicked, &dialog, backward);
  connect(quit, &QPushButton::clicked, &dialog, &QDialog::reject);
  PagingKeys key_filter(forward, backward);
  dialog.installEventFilter(&key_filter);
  table->installEventFilter(&key_filter);
  fill();
  dialog.resize(1100, 780);
  dialog.exec();
}

// ported from finst with neu_voll, finst_1, finst_s and suchas. Two
// columns of 27 rows like his screen, new moons left and full moons
// right. An eclipse adds an inverse row at its greatest moment, and with
// a record the aspect rows of the light follow every row
void MainWindow::eclipse_table() {
  const bool have_record = last_chart_ && (record_.day > 0 || record_.jd > 0.0);
  bool with_aspects = false;
  int base_deg = 45;
  double orb_factor = 0.25;
  if (have_record) {
    // his ASPEKTE mit GÜLTIGEM DATENSATZ UNTERSUCHEN ?
    const int bs = ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("ASPEKTE mit GÜLTIGEM DATENSATZ"), tr("UNTERSUCHEN ?")},
                                     {tr("JA"), tr(" NEIN "), tr("ABBRUCH")}, 1);
    if (bs < 0 || bs == 2) {
      return;
    }
    with_aspects = bs == 0;
    if (with_aspects) {
      // his k$ = "° = MAX. TEILER ", GRUND-ASPEKT WÄHLEN!
      const QString k = tr("° = MAX. TEILER ");
      const int es = ChoiceDialog::ask(this, tr("GRUND-ASPEKT WÄHLEN!"), {},
                                       {"    30" + k + "12", "    45" + k + " 8", "    60" + k + " 6", "    90" + k + " 4",
                                        "   180" + k + " 2", tr("360 ° = 0° = KONJUNKTION")},
                                       1);
      if (es < 0) {
        return;
      }
      static constexpr int kBase[6] = {30, 45, 60, 90, 180, 360};
      base_deg = kBase[es];
      // his ORBIS-FAKTOR ?
      const int of = ChoiceDialog::ask(this, tr("ORBIS-FAKTOR ?"), {}, {"    0.125", "    0.25", "    0.5", "    1.0"}, 1);
      if (of < 0) {
        return;
      }
      static constexpr double kFactor[4] = {0.125, 0.25, 0.5, 1.0};
      orb_factor = kFactor[of];
    }
  }
  // his a37dat(200,"","SUCH-DATUM EINGEBEN !")
  const std::optional<CalendarDate> day = ask_date(this, tr("SUCH-DATUM EINGEBEN !"), panel_day(), QString());
  if (!day) {
    return;
  }
  double k1 = lunation_number(*day);

  // his par = 2 for the screen, the light positions run the ET chain
  SearchContext ctx = make_context();
  ctx.settings.topocentric_parallax = false;
  // his di& = 27
  constexpr std::size_t kRows = 27;
  struct Row {
    QString text;
    double el = -1.0;
    QString kind;
    bool inverse = false;
    bool aspect = false;
    bool lunation = false;
  };
  const auto stamp = [this](double jd_ut) {
    // datum3$ + " " + ze1$. His homi$ rounded the minutes without a
    // carry and could print 60m, the port rounds the moment first
    const double jd = std::floor(jd_ut * kMinutesPerDay + 0.5) / kMinutesPerDay;
    const CalendarDate d = calendar_date(jd, current_settings().calendar);
    const int minutes = std::min(static_cast<int>(std::lround(d.hour * 60.0 + d.minute)), static_cast<int>(kMinutesPerDay) - 1);
    return datum3_text(d) + QString::asprintf(" %2dh %3dm", minutes / 60, minutes % 60);
  };
  const auto tag = [](int slot) {
    const std::string_view n = body::kName[static_cast<std::size_t>(slot)];
    return QString::fromUtf8(n.data(), static_cast<qsizetype>(n.size()));
  };
  const auto light_at = [&ctx](double jd_ut, int light) {
    const BodyLongitude bl = body_longitude(jd_ut, light, ctx);
    return bl.valid ? bl.el : -1.0;
  };
  // ported from suchas, the light against the factors of the active
  // chart, his plz(od,ze), harmonic by harmonic. The ase marks clear with
  // every harmonic, so a pair may return under a higher one like his.
  // His rows ran on to 30 over the legend, the port stops at the 27 rows
  const auto suchas = [&](int light, double ql, std::vector<Row>& rows) {
    if (ql < 0.0) {
      return;
    }
    const Chart& active = *last_chart_;
    const int d1 = static_cast<int>(1.0e-5 + kDegPerCircle / base_deg);
    const double o1 = org(aspect_settings_, light, 1);
    const QString a_name = tag(light);
    for (int w = 1; w <= d1; ++w) {
      std::array<bool, body::kSlotCount> ase{};
      const double pn = kTwoPi / w;
      const double dd = orb_factor * pn / kDefaultOrbDivisor;
      // his asp0 range aa& = 1 to bb& = 14, Sun through MC
      for (int ww = body::kSun; ww <= body::kMc; ++ww) {
        if (rows.size() >= kRows) {
          return;
        }
        const BodyState& b = active.b[static_cast<std::size_t>(ww)];
        // his IF NOT (ql(p&) = 0 OR plz(od,ze,ww&) = 0 OR ww& = 12)
        if (!b.present || !b.valid || b.el == 0.0 || ww == body::kNodeDesc || ase[static_cast<std::size_t>(ww)]) {
          continue;
        }
        const double dds = orbis_discr2(o1, org(aspect_settings_, ww, 1), dd);
        // his w3 is a VAR of vergl2 and keeps its lift through the m loop
        double w3 = norm_rad(ql - b.el);
        const QString pair = "=> " + a_name + "-" + tag(ww);
        const QString orb = QString::asprintf("| ORB=%4.1f\xC2\xB0", dds * kRadToDeg);
        if (w == 1) {
          if (w3 > 0.0 && (w3 < dds || w3 > kTwoPi - dds)) {
            ase[static_cast<std::size_t>(ww)] = true;
            // his e$ = LEFT$(d$,13)
            rows.push_back({pair + QString::fromUtf8("   0\xC2\xB0"), -1.0, {}, true, true});
          }
          continue;
        }
        for (int m = 1; m <= w - 1 && rows.size() < kRows; ++m) {
          const double w1 = norm_rad(m * pn - dds);
          double w2 = norm_rad(m * pn + dds);
          vergl2(w1, w2, w3);
          if (!(w1 < w3 && w3 < w2) || m * pn <= kEps) {
            continue;
          }
          ase[static_cast<std::size_t>(ww)] = true;
          const double ang = m * pn * kRadToDeg;
          QString text;
          if (w == 7 || w == 11) {
            text = pair + QString::asprintf(" %5.1f\xC2\xB0 =(360/%d)*%d", ang, w, m) + orb;
          } else if (w > 3 && m > 2) {
            text = pair + QString::asprintf(" %3.0f\xC2\xB0 =(360/%d)*%d  ", ang, w, m) + orb;
          } else {
            text = pair + QString::asprintf(" %3.0f\xC2\xB0", ang) + QString(13, ' ') + orb;
          }
          rows.push_back({text, -1.0, {}, true, true});
        }
      }
    }
  };
  // one column of his screen, zf counts its lunation and eclipse rows
  const auto column = [&](bool full, int& zf) {
    std::vector<Row> rows;
    const int light = full ? body::kMoon : body::kSun;
    double k = k1 - 2.0 - (full ? 0.5 : 0.0);
    zf = 0;
    while (rows.size() < kRows) {
      k += 1.0;
      const Lunation l = lunation_at(k);
      Row r;
      r.text = stamp(l.jd_ut);
      r.el = light_at(l.jd_ut, light);
      r.lunation = true;
      rows.push_back(r);
      ++zf;
      if (with_aspects) {
        suchas(light, r.el, rows);
      }
      if (l.eclipse && rows.size() < kRows) {
        Row e;
        e.text = stamp(l.max_ut);
        e.el = light_at(l.max_ut, light);
        e.kind = QString::fromStdString(l.kind);
        e.inverse = true;
        rows.push_back(e);
        ++zf;
        if (with_aspects) {
          suchas(light, e.el, rows);
        }
      }
    }
    rows.resize(kRows);
    return rows;
  };

  QDialog dialog(this);
  mark_output(&dialog, menu_item::kEclipses);
  dialog.setWindowTitle(tr("FINSTERNISSE....| WEITER mit Leertaste ! | ENDE mit 'ESC' !"));
  auto* v = new QVBoxLayout(&dialog);
  auto* heads = new QHBoxLayout();
  heads->addWidget(new QLabel(tr("NEUMOND-DATEN | SONNENFINSTERNISSE (UT)"), &dialog), 1);
  heads->addWidget(new QLabel(tr("VOLLMOND-DATEN | MONDFINSTERNISSE (UT)"), &dialog), 1);
  v->addLayout(heads);
  auto* table = new QTableWidget(static_cast<int>(kRows), 8, &dialog);
  table->horizontalHeader()->setVisible(false);
  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(20);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  table->setShowGrid(false);
  table->setItemDelegate(new ZodiacDelegate(table));
  table->setFont(theme::mono_font());
  v->addWidget(table, 1);
  // the legend of his screen
  v->addWidget(new QLabel(tr("'ZT'=ZENTRAL | 'EX'=EXZENT | 'TOT'=TOTAL | 'RF'=RINGF | 'N' = NÖRDL 'S' = SÜDL"), &dialog));
  // his gena4$ = w$ + "," + p$ + a$, the screen runs with par = 2
  const QString ephem = QString(konsta_.appa == 2 ? "App.2" : (konsta_.appa == 3 ? "Wahr" : "App.1")) + tr(",Ohne Parallaxe");
  v->addWidget(new QLabel(tr("'KERNSCH' = KERNSCHATTEN | 'HALBSCH'=HALBSCHATTEN | EPHEM:") + ephem, &dialog));
  if (with_aspects) {
    // his "Mit ASPEKTEN SO - bzw. MO - mit " + sol$(od,ze) + "  " + na$(od,ze)
    const QString sol = active_is_solar_ && active_solar_ >= 0 ? solar_labels_[static_cast<std::size_t>(active_solar_)]
                                                               : QStringLiteral("RADIX");
    const QString who = QString("%1 %2").arg(QString::fromStdString(record_.surname), QString::fromStdString(record_.given)).trimmed();
    v->addWidget(new QLabel(tr("Mit ASPEKTEN SO - bzw. MO - mit %1  %2").arg(sol, who), &dialog));
  }
  auto* keys = new QHBoxLayout();
  // his Weiter mit Leertaste  |  Zurück mit 'R' |  Ende mit 'ESC'
  auto* next = new QPushButton(tr("Weiter mit Leertaste"), &dialog);
  auto* back = new QPushButton(tr("Zurück mit 'R'"), &dialog);
  auto* quit = new QPushButton(tr("Ende mit 'ESC'"), &dialog);
  keys->addWidget(next);
  keys->addWidget(back);
  keys->addStretch(1);
  keys->addWidget(quit);
  v->addLayout(keys);

  int zf = 0;
  const auto fill = [&]() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int right_zf = 0;
    const std::vector<Row> left = column(false, zf);
    const std::vector<Row> right = column(true, right_zf);
    table->clearSpans();
    table->clearContents();
    const auto put = [table, &tag](int row, int col, const Row& r, int light) {
      if (r.text.isEmpty()) {
        return;
      }
      auto* t = new QTableWidgetItem(r.text);
      if (r.inverse) {
        t->setBackground(QColor(0, 0, 0));
        t->setForeground(QColor(0xFF, 0xFF, 0xFF));
      }
      table->setItem(row, col, t);
      if (r.aspect) {
        table->setSpan(row, col, 1, 4);
        return;
      }
      // his SWITCH i&, CASE 1,8,15,22,29, @plein2
      if (r.lunation && row % 7 == 0) {
        auto* marker = new QTableWidgetItem(tag(light));
        set_body_sprite(marker, light, theme::ink_now());
        table->setItem(row, col + 1, marker);
      }
      if (r.el >= 0.0) {
        table->setItem(row, col + 2, zodiac_item(r.el));
      }
      if (!r.kind.isEmpty()) {
        table->setItem(row, col + 3, new QTableWidgetItem(" " + r.kind));
      }
    };
    for (int i = 0; i < static_cast<int>(kRows); ++i) {
      put(i, 0, left[static_cast<std::size_t>(i)], body::kSun);
      put(i, 4, right[static_cast<std::size_t>(i)], body::kMoon);
    }
    table->resizeColumnsToContents();
    QApplication::restoreOverrideCursor();
  };
  // his k1 = k1 + INT(ABS(zf& - zf& / 5)), the next screen overlaps a fifth
  const auto forward = [&]() {
    k1 += std::floor(std::abs(zf - zf / 5.0));
    fill();
  };
  const auto backward = [&]() {
    k1 -= std::floor(2.0 * std::abs(zf - zf / 5.0));
    fill();
  };
  connect(next, &QPushButton::clicked, &dialog, forward);
  connect(back, &QPushButton::clicked, &dialog, backward);
  connect(quit, &QPushButton::clicked, &dialog, &QDialog::reject);
  PagingKeys key_filter(forward, backward);
  dialog.installEventFilter(&key_filter);
  table->installEventFilter(&key_filter);
  fill();
  dialog.resize(1080, 700);
  dialog.exec();
}

namespace {

// his zipu$ after SELECT zal_grossj&, the kept setting names the age in
// mixed case, a fresh choice in capitals
QString age_name(int zal, bool capitals) {
  struct Age {
    int zal;
    const char* name;
    const char* caps;
  };
  static constexpr Age kAges[] = {
      {30, QT_TRANSLATE_NOOP("horcom::MainWindow", "Widder"), QT_TRANSLATE_NOOP("horcom::MainWindow", "WIDDER")},
      {360, QT_TRANSLATE_NOOP("horcom::MainWindow", "Fische"), QT_TRANSLATE_NOOP("horcom::MainWindow", "FISCHE")},
      {330, QT_TRANSLATE_NOOP("horcom::MainWindow", "Wassermann"), QT_TRANSLATE_NOOP("horcom::MainWindow", "WASSERMANN")},
      {300, QT_TRANSLATE_NOOP("horcom::MainWindow", "Steinbock"), QT_TRANSLATE_NOOP("horcom::MainWindow", "STEINBOCK")}};
  for (const Age& a : kAges) {
    if (a.zal == zal) {
      return QCoreApplication::translate("horcom::MainWindow", capitals ? a.caps : a.name);
    }
  }
  return {};
}

}  // namespace

// ported from grossj1, the texts of the result and of the yellow box
QStringList MainWindow::great_year_lines(bool& outside) const {
  const Chart& c = *last_chart_;
  const GreatYearPoint p =
      great_year_point(c.jd_ut, c.ta.tropical_year_days, c.smo.ekls, konsta_.jdgross, konsta_.zal_grossj);
  outside = p.outside;
  const Calendar cal = current_settings().calendar;
  return {tr("Ekliptikale Bezugs-Länge = %1° Entspr. %2 - Zeitalter").arg(konsta_.zal_grossj, 3).arg(great_year_age_),
          tr("Bezugs-Zeitpunkt = %1").arg(datum3_text(calendar_date(konsta_.jdgross, cal))),
          tr("Längen - Differenz zur Bezugs - Länge = %1°").arg(std::abs(p.di_deg), 8, 'f', 4),
          tr("Der 'Zeitalter - Punkt' für das Datum  %1 %2").arg(datum3_text(panel_day()), jul_mark(panel_calendar_)),
          tr("hat die Ekliptikale Länge  %1 =  %2°").arg(zodiac(p.point_deg * kDegToRad)).arg(p.point_deg, 8, 'f', 4)};
}

// ported from grossj. The reference date and the start of the age persist
// with the Vorgaben like his param_sp, the yellow box on the VORGABEN
// overview stays while the display is on
void MainWindow::great_year() {
  if (!last_chart_ || active_is_solar_) {
    return;
  }
  // his hrg! = FALSE
  if (helio_->isChecked()) {
    helio_->setChecked(false);
  }
  QString who;
  if (konsta_.jdgross != 0.0) {
    const Calendar cal = current_settings().calendar;
    great_year_age_ = age_name(konsta_.zal_grossj, false);
    // his box printed 30° for every age, the port names the chosen start
    const int r = ChoiceDialog::ask(
        this, tr("ENTSCHEIDUNG !"),
        {tr("Einstellung Beibehalten ?"), "  ", tr("Bezugsdatum : %1").arg(datum3_text(calendar_date(konsta_.jdgross, cal))),
         tr("Zeitalters-Punkt : %1° %2").arg(konsta_.zal_grossj).arg(great_year_age_)},
        {tr(" Beibehalten "), tr(" Ändern "), tr("Anzeige abschalten = Programm beenden")}, 0);
    if (r < 0) {
      return;
    }
    if (r == 2) {
      great_year_on_ = false;
      persist_konsta();
      return;
    }
    if (r == 1) {
      // his d$ = "Für DATENSATZ  :  " + na$(od,ze)
      who = tr("Für DATENSATZ  :  %1").arg(record_label_.trimmed());
      const int es = ChoiceDialog::ask(this, tr("Zeitalter - Start wählen !"),
                                       {tr("Ekliptik-Grad Eingeben ab dem gerechnet werden soll !"),
                                        tr("Z.B. 330° für den Beginn des 'Wassermann' - Zeitalters"),
                                        tr("Der 'Zeitalters - Punkt' wandert von dort ab RÜCKLÄUFIG !"), QString(), who},
                                       {tr("WIDDER     =  30°"), tr("FISCHE     = 360°"), tr("WASSERMANN = 330°"),
                                        tr("STEINBOCK  = 300°")},
                                       2);
      if (es < 0) {
        return;
      }
      static constexpr int kStart[4] = {30, 360, 330, 300};
      const int zal = kStart[es];
      // his a37dat(110,"","DATUM-ZEIT-EINGABE"), the old reference in the fields
      const std::optional<CalendarDate> day =
          ask_date(this, tr("DATUM-ZEIT-EINGABE"), calendar_date(konsta_.jdgross, cal), QString(),
                   {tr("Bezugsdatum Eingeben entsprechend %1° = %2").arg(zal).arg(age_name(zal, true)),
                    tr("Die Geschwindigkeit des 'Zeitalter - Punktes' ist :"),
                    tr("Pro Zeichen ca. 2148 Jahre ~  50.269\" pro Jahr"), QString(), who});
      if (!day) {
        return;
      }
      konsta_.zal_grossj = zal;
      great_year_age_ = age_name(zal, true);
      // noon like his CHAUVIN reference and the ho = 12 of date_form
      konsta_.jdgross = julian_day({day->day, day->month, day->year, 12.0, 0.0}, cal);
    }
    great_year_on_ = true;
  }
  bool outside = false;
  const QStringList mes = great_year_lines(outside);
  if (outside) {
    // his NICHT MEHR im <zipu$> - ZEITALTER !
    QMessageBox::information(this, "HORCOM", tr("NICHT MEHR im %1 - ZEITALTER !").arg(great_year_age_));
  }
  // his d$ + " | " + sol$(od,ze) + " | Datum : " + datum3$ + " " + jul$(od,ze)
  const QString title = tr("%1 | RADIX | Datum : %2 %3").arg(who, datum3_text(panel_day()), jul_mark(panel_calendar_));
  QMessageBox::information(this, title, mes.join(QChar(0x0A)));
  persist_konsta();
}

std::array<int, body::kSlotCount> MainWindow::shown_emphasis() const {
  std::array<int, body::kSlotCount> e = emphasis_;
  // the Mondknoten row hides DR and DS when unchecked, the panel switch
  // rides on top of the Planeten-Auswahl markings
  if (node_show_ != nullptr && !node_show_->isChecked()) {
    e[body::kNodeAsc] = -1;
    e[body::kNodeDesc] = -1;
  }
  return e;
}

// ported from the asp_wahl filter of aspz1, IF NOT (plw&(t&) = 0 &&
// asp_wahl!), a hidden body neither aspects nor counts
AspectSettings MainWindow::shown_aspect_settings() const {
  AspectSettings a = aspect_settings_;
  const std::array<int, body::kSlotCount> e = shown_emphasis();
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    if (e[static_cast<std::size_t>(slot)] < 0) {
      a.weight[static_cast<std::size_t>(slot)] = 0;
    }
  }
  return a;
}


WheelOptions MainWindow::radix_wheel_options(const Chart& chart, const ChartSettings& s, bool frame) const {
  WheelOptions wopt;
  apply_ring_dress(konsta_, wopt);
  wopt.outer_color = outer_color_;
  wopt.heliocentric = s.heliocentric;
  const bool mundane = frame && mundane_frame_ && !s.heliocentric;
  // IF nasp& > 1, asp1 with its chords, the axes and the inner marks
  // runs only with a divisor above one, aspz1 and plein11 want horm& = 1
  wopt.aspect_lines = aspect_settings_.divisors > 1 && !mundane;
  // asp1, nas& = nasp&, the equal probability orbs reach to twelve
  wopt.divisors = aspect_settings_.equal_probability ? std::min(aspect_settings_.divisors, kMaxEqualOrbDivisor)
                                                     : aspect_settings_.divisors;
  wopt.small_symbols = konsta_.klsy;
  // IF horm& < 2, IF pziff < 3 the degrees, pziff = 1 also the R
  wopt.degree_numbers = konsta_.pziff < 3.0 && !mundane;
  wopt.retro_marks = konsta_.pziff == 1.0 && !mundane;
  // bes2, a$ = sol$(od,ze), horm& = 2 adds mu$
  wopt.chart_label = rhythm_chart_label().toStdString();
  if (mundane) {
    wopt.chart_sub_label = tr("Mundan").toStdString();
  }
  // horbeg, the mundane frame keeps begz& = 1
  wopt.begin = (frame && mundane_frame_) ? 1 : std::clamp(konsta_.begz, 1, 5);
  wopt.begin_lon = QString::fromStdString(konsta_.begz_name).replace(',', '.').toDouble() * kDegToRad;
  wopt.emphasis = shown_emphasis();
  // the fixed point wears red so it catches the eye at once, as the
  // tester asked for it, the Sonderpunkt of a Septar his red F
  if (chart.b[0].present && chart.b[0].valid && !s.heliocentric && wopt.emphasis[0] == 0) {
    wopt.emphasis[0] = 1;
  }
  // haw& = 10, WEDER HÄUSER noch AC oder MC noch MONDKNOTEN, the nodes
  // leave the wheel with their line
  if (s.houses == HouseSystem::kNoneNoNodes && !s.heliocentric) {
    wopt.emphasis[body::kNodeAsc] = -1;
    wopt.emphasis[body::kNodeDesc] = -1;
  }
  // the node line needs both nodes chosen, plw&(11) && plw&(12)
  wopt.node_axis = wopt.emphasis[body::kNodeAsc] >= 0 && wopt.emphasis[body::kNodeDesc] >= 0 && !s.heliocentric;
  // IF nk&(1) > 0 && hrg! = 0 && apog! && horm& = 1, the Black Moon line
  wopt.apogee_axis = !s.heliocentric && chart.b[body::kApogee].present && chart.b[body::kApogee].valid &&
                     wopt.emphasis[body::kApogee] >= 0;
  // the GEBURTSHERRSCHER, the original stamps him inverted like the
  // nodes, the Planeten-Auswahl red marking rides on top of that
  if (!s.heliocentric && chart.houses.ok) {
    const int kp = sign_ruler(chart.houses.cusp[1], alt_rulers_);
    if (kp > 0 && kp < body::kSlotCount) {
      wopt.ruler_slot = kp;
      if (ruler_red_ && wopt.emphasis[static_cast<std::size_t>(kp)] == 0) {
        wopt.emphasis[static_cast<std::size_t>(kp)] = 1;
      }
    }
    // geb_herr adds the ruler of a sign intercepted in the first house
    const int k3 = birth_rulers(chart.houses.cusp[1], chart.houses.cusp[2], alt_rulers_).second;
    if (k3 > 0 && k3 < body::kSlotCount) {
      wopt.ruler_slot2 = k3;
    }
  }
  // the original stamped the true nodes inverted like the rulers, the
  // author's family asked for them as normal planets, so only the true
  // apogee keeps his SRCINVERT dress
  wopt.invert_nodes = false;
  wopt.invert_apogee = s.true_apogee;
  return wopt;
}

// ported from the prologue of multiple and harm with fixpunkt_def_mult
std::pair<Chart, ChartSettings> MainWindow::multi_radix(const ChartInput& in, const ChartSettings& s) const {
  // hrg! = 0, moknw! = 0, apogw! = 0, horm& = 1. His kard! = 0 has no
  // part in the directed charts of the port
  ChartSettings ms = s;
  ms.heliocentric = false;
  ms.true_node = false;
  ms.true_apogee = false;
  Chart c = compute_chart(in, ms, vsop_, eph_);
  if (!c.ok) {
    ChartSettings fallback = ms;
    fallback.houses = HouseSystem::kAcMcOnly;
    c = compute_chart(in, fallback, vsop_, eph_);
  }
  // IF fixpunkt& = 1 && hrg! = 0, aa& = 0 and plz(1,ze,0) = pu * VAL(fixpunkt$)
  if (fixpunkt_ >= 0.0) {
    c.b[body::kFixpunkt].present = true;
    c.b[body::kFixpunkt].valid = true;
    c.b[body::kFixpunkt].el = fixpunkt_;
  }
  return {c, ms};
}

Histogram MainWindow::sheet_histogram(const Chart& chart, const ChartSettings& s) const {
  HistogramOptions opt;
  opt.points = histogram_points(konsta_.pn);
  opt.double_first_house = konsta_.haus1_dop;
  opt.double_ruler = konsta_.gebherr_dop;
  opt.classic_rulers = alt_rulers_;
  // elem1 and kard_fix_gem walk every computed body, the Planeten-Auswahl
  // only thins the drawing
  return chart_histogram(chart, s, opt);
}

SearchContext MainWindow::make_context() const {
  SearchContext ctx;
  ctx.base = current_input();
  ctx.settings = current_settings();
  ctx.vsop = &vsop_;
  ctx.eph = &eph_;
  return ctx;
}

// the a14 mean and the corner rows of a13aus and a14auscomb
void MainWindow::combin_of(const std::vector<AafRecord>& parts, const std::vector<int>& sets) {
  if (parts.size() < 2) {
    return;
  }
  const Calendar cal = current_settings().calendar;
  std::vector<ChartInput> inputs;
  for (const AafRecord& r : parts) {
    inputs.push_back(record_input(r));
  }
  const ChartInput mixed = combin_input(inputs, cal);
  // go$(0,2) = "COMBIN-ORT", record data stays in his German
  set_panel_place({mixed.lon_deg_east, mixed.lat_deg, "COMBIN-ORT"});
  const auto short_name = [](const AafRecord& r, int length) {
    QString n = QString::fromStdString(r.surname).trimmed();
    if (n.isEmpty()) {
      n = QString::fromStdString(r.given).trimmed();
    }
    return n.left(length);
  };
  combin_list_.clear();
  if (parts.size() == 2) {
    // the tester wanted the names and moments of both charts beside the
    // mean, his comb! showed the names only
    combin_name1_ = pair_name_row(1, short_name(parts[0], kDatNameLength), "RADIX");
    combin_moment1_ = pair_moment_row(1, inputs[0].date_ut);
    combin_name2_ = pair_name_row(2, short_name(parts[1], kDatNameLength), tr("Hor 2"));
    combin_moment2_ = pair_moment_row(2, inputs[1].date_ut);
  } else {
    // "SÄTZE: " + z$, the names LEFT$ 20 stacked above the bottom edge
    QStringList numbers;
    for (const int n : sets) {
      numbers << QString::number(n);
    }
    combin_name1_ = (tr("SÄTZE: ") + numbers.join(",")).toStdString();
    combin_moment1_.clear();
    combin_name2_.clear();
    combin_moment2_.clear();
    for (const AafRecord& r : parts) {
      combin_list_.push_back(short_name(r, 20).toStdString());
    }
  }
  int s = static_cast<int>((mixed.date_ut.hour * 60.0 + mixed.date_ut.minute) * 60.0 + 0.5);
  if (s >= kSecondsPerDay) {
    s = kSecondsPerDay - 1;
  }
  // sol$(od,ze) + "-UT:", datum3$ and ze$ of a14auscomb
  combin_note_ = (tr("COMBIN-UT:") + QString::asprintf(" %02d.%02d.%d  %02d:%02d:%02d", mixed.date_ut.day,
                                                          mixed.date_ut.month, mixed.date_ut.year, s / 3600,
                                                          (s / 60) % 60, s % 60))
                     .toStdString();
  // na$(od,ze) = LEFT$(na$(od21,ze21),10) + "-" + LEFT$(na$(od22,ze22),10)
  const QString na = short_name(parts[0], 10) + "-" + short_name(parts[1], 10);
  apply_moment(julian_day(mixed.date_ut, cal), "COMBIN " + na);
  combin_na_ = na;
  update_solar_actions();
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
  if (z.abbrev == "LMT") {
    // zeitzon_nam_aaf takes LMT from the longitude alone, the equation of
    // time is already part of the mean local clock
    set_local_time(true, false);
    clock_kind_ = ClockKind::kMeanLocal;
  } else if (z.is_local_time()) {
    // LTT, the questions of zuo decide between true and mean local time
    set_local_time(true, true);
  } else if (z.to_ut_hours) {
    set_local_time(false, false);
    // the catalogue stores the step from zone time to UT, the panel
    // wants hours east
    const QSignalBlocker b(zone_);
    zone_->setValue(-*z.to_ut_hours);
  }
  if (refresh_sommer_effect_) refresh_sommer_effect_();
  recompute();
}

// ported from a2dat and a2fdat, the FILESELECT into the file hub. One
// action per visit like the original, the file stays bound afterwards
void MainWindow::data_file_io() {
  // once a working Daten-Datei is bound the menu goes straight to the
  // hub, one file per session as the original demanded. Only an unbound
  // slot or an explicit ANDERE DATEI opens the picker, that spared the
  // tester the impression that the program keeps asking to save
  bool need_pick = data_file_.isEmpty() ||
                   !std::filesystem::exists(std::filesystem::path(data_file_.toStdWString()));
  while (true) {
    if (need_pick) {
      // FILESELECT hrc$+"\SPEZIAL\*.DAT", a fresh name starts a new file
      QString kind;
      QString path = QFileDialog::getSaveFileName(
          this, tr("Daten-Datei wählen oder neu anlegen"), collection_start(data_file_, data_dir_),
          tr("HORCOM Daten-Dateien (*.DAT *.dat);;AAF (*.AAF *.aaf)"), &kind, QFileDialog::DontConfirmOverwrite);
      if (path.isEmpty()) {
        return;
      }
      path = with_suffix(path, kind.startsWith("AAF") ? "AAF" : "DAT");
      std::filesystem::path dat(path.toStdWString());
      if (path.endsWith(".aaf", Qt::CaseInsensitive)) {
        //RR Die parallele HORCOM-Datei GLEICHEN NAMENS dient als Pilot
        const std::filesystem::path aaf = dat;
        dat = dat_twin_path(aaf);
        if (!std::filesystem::exists(dat) && std::filesystem::exists(aaf)) {
          const auto records = read_aaf(aaf);
          if (records && !records->empty()) {
            write_dat_from_aaf(dat, *records);
          }
        }
      }
      if (!std::filesystem::exists(dat)) {
        // his a2dat on a new name, the file starts with the current record
        // when one with date and name is loaded, and the visit ends there
        const AafRecord current = panel_record();
        const bool loaded = active_slot_ >= 0 && slots_[static_cast<std::size_t>(active_slot_)].has_value();
        if (!loaded || current.day <= 0 || record_name(current).empty()) {
          //RR KEIN Datensatz!  ERST EINGEBEN !
          QMessageBox::information(this, "HORCOM", tr("KEIN Datensatz!  ERST EINGEBEN !"));
          return;
        }
        if (!write_chart_file(dat, {dat_from_record(current)})) {
          QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht anlegen."));
          return;
        }
        //RR NEUE DATEN-DATEI ... !
        QMessageBox::information(
            this, "HORCOM",
            tr("NEUE DATEN-DATEI %1 !").arg(QString::fromStdWString(dat.filename().wstring())));
        bind_data_file(QString::fromStdWString(dat.wstring()));
        return;
      }
      bind_data_file(QString::fromStdWString(dat.wstring()));
      need_pick = false;
    }
    const QString label = QFileInfo(data_file_).fileName();
    // the hub of a2fdat, one question per box, ANDERE DATEI added so
    // the working file can be swapped without leaving the hub
    const int action = ChoiceDialog::ask(this, tr("DATEI : %1").arg(label), {},
                                         {tr("Datensätze HOLEN ?"), tr("AKTUELLEN Datensatz EINTRAGEN ?"),
                                          tr("Datensätze LÖSCHEN ?"), tr("Datei TRIMMEN ?"),
                                          tr("Datei MINIMIEREN ?"), tr("ANDERE DATEI wählen…"),
                                          tr("ABBRUCH")});
    switch (action) {
      case 0:
        fetch_from_file();
        return;
      case 1:
        save_record();
        return;
      case 2:
        delete_from_file();
        return;
      case 3:
        tidy_data_file(false);
        return;
      case 4:
        tidy_data_file(true);
        return;
      case 5:
        need_pick = true;
        continue;
      default:
        return;
    }
  }
}

void MainWindow::bind_data_file(const QString& path) {
  data_file_ = path;
  data_count_ = 0;
  if (!path.isEmpty()) {
    const auto records = read_chart_file(std::filesystem::path(path.toStdWString()));
    data_count_ = records ? static_cast<int>(records->size()) : 0;
  }
  refresh_data_file_label();
}

void MainWindow::refresh_data_file_label() {
  if (data_file_label_ == nullptr) {
    return;
  }
  if (data_file_.isEmpty()) {
    data_file_label_->setText(tr("keine"));
    return;
  }
  //RR Daten-Datei: ... Anzahl Dats.:
  data_file_label_->setText(QString("%1\n%2").arg(QFileInfo(data_file_).fileName(),
                                                  tr("Anzahl Dats.: %1").arg(data_count_)));
}

// reads a collection into the exchange form, DAT or AAF
std::optional<std::vector<AafRecord>> MainWindow::load_collection(const QString& path) const {
  std::vector<AafRecord> records;
  if (path.endsWith(".aaf", Qt::CaseInsensitive)) {
    const auto r = read_aaf(path.toStdWString());
    if (!r) {
      return std::nullopt;
    }
    records = *r;
  } else {
    const auto r = read_chart_file(path.toStdWString());
    if (!r) {
      return std::nullopt;
    }
    records.reserve(r->size());
    for (const ChartRecord& c : *r) {
      records.push_back(aaf_from_chart_record(c));
    }
  }
  return records;
}

// ported from a210, the SORTIER-MODUS box before the chooser, short
// files skip the question and sort by name like the original
std::vector<std::size_t> MainWindow::ask_order(const std::vector<AafRecord>& records, const QString& file_label) {
  RecordOrder order = RecordOrder::kName123;
  // IF datein! AND laf& > 45
  if (records.size() > 45) {
    const int es = ChoiceDialog::ask(this, tr("SORTIER-MODUS ?  DATEI : %1").arg(file_label), {},
                                     {tr("ALPHABETISCH: 1., 2. und 3. NAME"), tr("ALPHABETISCH: 2. und 3. NAME"),
                                      tr("ALPHABETISCH: nur 3. NAME"), tr("GEBURTSTAG"), tr("DATUM"),
                                      tr("ABBRUCH")});
    switch (es) {
      case 0:
        order = RecordOrder::kName123;
        break;
      case 1:
        order = RecordOrder::kName23;
        break;
      case 2:
        order = RecordOrder::kName3;
        break;
      case 3:
        order = RecordOrder::kBirthday;
        break;
      case 4:
        order = RecordOrder::kDate;
        break;
      default:
        return {};
    }
  }
  std::vector<OrderKeySource> keys;
  keys.reserve(records.size());
  for (const AafRecord& r : records) {
    OrderKeySource k;
    k.name = record_name(r);
    k.day = r.day;
    k.month = r.month;
    k.year = r.year;
    keys.push_back(std::move(k));
  }
  return record_order(keys, order);
}

int MainWindow::next_slot() const {
  for (int i = 0; i < 5; ++i) {
    if (!slots_[static_cast<std::size_t>(i)]) {
      return i;
    }
  }
  return -1;
}

void MainWindow::set_slot(int index, const AafRecord& r, bool activate, const QString& label) {
  // an edit still settling belongs to the slot the panel leaves
  if (activate) {
    flush_history();
  }
  slots_[static_cast<std::size_t>(index)] = r;
  radix_labels_[static_cast<std::size_t>(index)] = label;
  if (activate) {
    active_slot_ = index;
    apply_record(r);
  }
  update_slot_actions();
}

// ported from a4 and a5, the menu click on a SATZ row. Outside the paired
// sessions his direkt goes on into eingabe, the EINGABE- und ANZEIGE-BOX
// of the slot, whose ABSPEICHERN writes the edited record to the file
void MainWindow::open_slot(const SlotChoice& c) {
  activate_slot(c);
  const auto i = static_cast<std::size_t>(c.index);
  const auto& stored = c.solar ? solar_slots_[i] : slots_[i];
  if (!stored) {
    return;
  }
  const QString label = c.solar ? solar_labels_[i] : radix_label(c.index);
  RecordMaskDialog mask(*stored, tr("EINGABE- und ANZEIGE-BOX | %1 NR.%2").arg(label).arg(c.index + 1),
                        RecordMaskDialog::Mode::kShow, data_dir_, this);
  if (!data_file_.isEmpty()) {
    mask.bind_file(QFileInfo(data_file_).fileName(), aaf_twin_path(std::filesystem::path(data_file_.toStdWString())));
  }
  if (mask.exec() != QDialog::Accepted) {
    return;
  }
  if (c.solar) {
    solar_slots_[i] = mask.record();
    apply_record(*solar_slots_[i], false);
    banner_->set_record(solar_labels_[i]);
    update_solar_actions();
  } else {
    set_slot(c.index, mask.record(), true, radix_labels_[i]);
  }
  if (mask.save_requested()) {
    store_record(false);
  }
}

// ported from a2113_1, the next free RADIX slot. With all five filled
// the number of the active chart gives way after his question
int MainWindow::claim_radix_slot(int zmsp) {
  const int slot = next_slot();
  if (slot >= 0) {
    return slot;
  }
  // zmsp = ze
  if (zmsp < 0) {
    zmsp = active_is_solar_ ? active_solar_ : active_slot_;
  }
  zmsp = std::clamp(zmsp, 0, 4);
  const auto& old = slots_[static_cast<std::size_t>(zmsp)];
  // TRIM$(na$(od,zmsp)), the whole name of the record that gives way
  const QString name = old ? slot_name(*old) : QString();
  //RR DIESER DATENSATZ ÜBERSCHREIBT DEN VORHERGEHENDEN !
  const int es = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"),
                                   {QString(), tr("DIESER DATENSATZ ÜBERSCHREIBT"), tr("DEN VORHERGEHENDEN !"),
                                    tr("NR. %1 :  %2").arg(zmsp + 1).arg(name)},
                                   {tr("Weiter"), tr("Irrtum ( = UNDO = Zurück )")}, 0);
  if (es != 0) {
    return -1;
  }
  // @dats_l(zmsp) empties his three rows of that column, the SOLAR row of
  // the same number holds the derived chart of the record that gives way
  // and the DOPPEL row is numbered like the radix slots
  solar_slots_[static_cast<std::size_t>(zmsp)].reset();
  solar_labels_[static_cast<std::size_t>(zmsp)].clear();
  if (active_is_solar_ && active_solar_ == zmsp) {
    active_is_solar_ = false;
  }
  if (zmsp < kDoubleKinds) {
    double_slots_[static_cast<std::size_t>(zmsp)].reset();
  }
  // his IF zesp = zeuhr && zeuhr = 5 stopped only a clock in the fifth
  // slot, a clock in a slot that gave way went on writing its moment into
  // the new record every 15 seconds. The clock stops when its slot goes
  if (uhr_slot_ == zmsp) {
    clock_off();
  }
  update_solar_actions();
  update_double_actions();
  return zmsp;
}

void MainWindow::update_slot_actions() {
  for (int i = 0; i < 5; ++i) {
    QAction* a = slot_actions_[static_cast<std::size_t>(i)];
    if (a == nullptr) {
      continue;
    }
    const auto& slot = slots_[static_cast<std::size_t>(i)];
    if (slot) {
      // SATZ1: RADIX  <name>, the menu shows what every slot holds
      a->setText(QString("SATZ%1: %2  %3").arg(i + 1).arg(radix_label(i), slot_name(*slot)));
      a->setEnabled(true);
      a->setChecked(!active_is_solar_ && i == active_slot_);
    } else {
      a->setText(QString("SATZ%1").arg(i + 1));
      a->setEnabled(false);
      a->setChecked(false);
    }
  }
}

// the SOLAR...-DATEN slots, every derived chart writes itself back like
// his ^ menu items registered sol$(2,ze). The row is the number ze of
// the radix it came from, a new result overwrites the old one there
void MainWindow::store_solar(const QString& label) {
  int idx = active_is_solar_ ? active_solar_ : active_slot_;
  if (idx < 0) {
    // a panel chart without a RADIX row stands at the first number
    idx = 0;
  }
  solar_slots_[static_cast<std::size_t>(idx)] = panel_record();
  solar_labels_[static_cast<std::size_t>(idx)] = label;
  active_is_solar_ = true;
  active_solar_ = idx;
  update_slot_actions();
  update_solar_actions();
}

void MainWindow::update_solar_actions() {
  for (int i = 0; i < 5; ++i) {
    QAction* a = solar_actions_[static_cast<std::size_t>(i)];
    if (a == nullptr) {
      continue;
    }
    const auto& slot = solar_slots_[static_cast<std::size_t>(i)];
    if (slot) {
      //RR SATZ1: <sol$>  <na$>
      const QString name = (QString::fromStdString(slot->surname).trimmed() + " " +
                            QString::fromStdString(slot->given).trimmed())
                               .trimmed()
                               .toUpper();
      a->setText(QString("SATZ%1: %2  %3").arg(i + 1).arg(solar_labels_[static_cast<std::size_t>(i)], name));
      a->setEnabled(true);
      a->setChecked(active_is_solar_ && i == active_solar_);
    } else {
      a->setText(QString("SATZ%1").arg(i + 1));
      a->setEnabled(false);
      a->setChecked(false);
    }
  }
  // the ERGEBNIS als RADIX action rides on top of the active derived chart,
  // it stays disabled while a radix drives the panel
  if (result_as_radix_action_ != nullptr) {
    result_as_radix_action_->setEnabled(can_promote_result());
  }
  //RR IF od = 2, MENU 82, 83 and 98 MF_GRAYED
  for (QAction* a : {great_year_action_, korrektur_action_, time_wander_action_}) {
    if (a != nullptr) {
      a->setEnabled(!active_is_solar_);
    }
  }
}

QString MainWindow::radix_label(int index) const {
  const QString& l = radix_labels_[static_cast<std::size_t>(index)];
  //RR rd$ = "RADIX"
  return l.isEmpty() ? QStringLiteral("RADIX") : l;
}

// the SOLAR results and the COMBIN can become a radix, a COMPOSIT has no
// moment of its own
std::optional<std::pair<AafRecord, QString>> MainWindow::promotable_result() const {
  if (active_is_solar_ && active_solar_ >= 0 && solar_slots_[static_cast<std::size_t>(active_solar_)]) {
    return std::make_pair(*solar_slots_[static_cast<std::size_t>(active_solar_)],
                          solar_labels_[static_cast<std::size_t>(active_solar_)].trimmed());
  }
  if (!combin_name1_.empty()) {
    //RR a31 takes the moment and the place of the COMBIN, na$ = n1-n2
    AafRecord r = panel_record();
    r.surname = combin_na_.toStdString();
    r.given.clear();
    return std::make_pair(r, QStringLiteral("COMBIN"));
  }
  return std::nullopt;
}

bool MainWindow::can_promote_result() const {
  return promotable_result().has_value();
}

// ported from erg_rad, a derived chart becomes a RADIX of its own so it
// can be examined like a birth chart
void MainWindow::result_as_radix() {
  // his a311 reads the result before a2113_1 clears its labels
  const auto result = promotable_result();
  if (!result) {
    return;
  }
  // alertbox(1,"Nur für GEÜBTE !"," ","Das CHAOS DROHT !","",1,"&ABBRUCH","&WEITER")
  const int go = ChoiceDialog::ask(this, tr("ENTSCHEIDUNG !"), {tr("Nur für GEÜBTE !"), " ", tr("Das CHAOS DROHT !")},
                                   {tr("ABBRUCH"), tr("WEITER")}, 0);
  if (go != 1) {
    return;
  }
  // zmsp! = @a2113_1(zmsp), the COMBIN stands at ze = 2. His Irrtum
  // set expr! but erg_rad wrote the slot anyway, the port aborts
  const int slot = claim_radix_slot(active_is_solar_ ? active_solar_ : kDoubleCombin);
  if (slot < 0) {
    return;
  }
  // b$ = s$ + " ALS " + rd$, sol$(od,ze) = b$, the name stays
  const QString label = result->second + " ALS RADIX";
  set_slot(slot, result->first, true, label);
  // @direkt(0,-1,1,18), the EINGABE- und ANZEIGE-BOX of the new radix
  RecordMaskDialog mask(result->first, tr("EINGABE- und ANZEIGE-BOX | %1 NR.%2").arg(label).arg(slot + 1),
                        RecordMaskDialog::Mode::kShow, data_dir_, this);
  if (!data_file_.isEmpty()) {
    mask.bind_file(QFileInfo(data_file_).fileName(), aaf_twin_path(std::filesystem::path(data_file_.toStdWString())));
  }
  if (mask.exec() == QDialog::Accepted) {
    set_slot(slot, mask.record(), true, label);
    if (mask.save_requested()) {
      store_record(false);
    }
  }
}

// the operator warning strips out, so the capture hook and later chained
// workflows can promote without waiting on a modal
void MainWindow::promote_result_to_radix_scripted(int slot) {
  const auto result = promotable_result();
  if (!result) {
    return;
  }
  int idx = slot >= 0 ? slot : next_slot();
  if (idx < 0) {
    idx = active_is_solar_ ? active_solar_ : kDoubleCombin;
  }
  set_slot(idx, result->first, true, result->second + " ALS RADIX");
}

// the plain HOROSKOP - GRAPHIK entry, every special view steps aside
void MainWindow::reset_views() {
  leave_views({});
  claim_wheel();
  recompute();
}

// the end of a11_1, halbszaus% = 1 and @halbs_zaehl_gr(-1) for the
// midpoints of the KOMPAKT-AUSWERTUNG
void MainWindow::horoskop_graphik() {
  // his IF plan_wahl! = 0 && plan_col! = 0 : moda& = @druck_graph_ein, the
  // menu entry clears both flags first in clr_main. DIN A4 stands in the
  // box only under his IF tabelle! = 0 && horm& = 1, the ecliptic frame
  const int moda = ask_graphic_output(menu_item::kChartGraphic, !mundane_frame_);
  if (moda == kOutputNone) {
    return;
  }
  reset_views();
  if (moda != kOutputScreen) {
    // his CASE 2 at gdxp& / 15, CASE 3 the DIN A4 page, no screen
    (void)print_graphic(moda == kOutputA4 ? a4_export_list() : classic_export_list(), moda == kOutputA4,
                        page_margin::kLeftChart);
    return;
  }
  // his scget in men3, the chart is the last picture, then the chart
  // until a key and the HARDCOPY while the option is on
  remember_picture(this);
  if (konsta_.prenbl != 0) {
    wart(menu_item::kChartGraphic);
  }
}

void MainWindow::leave_views(std::initializer_list<const QAction*> keep) {
  for (QAction* a : {compare_action_, dial_action_, harmonic_action_, multi_action_, composite_action_,
                     directions_action_, arc_action_}) {
    if (a != nullptr && a->isChecked() && std::find(keep.begin(), keep.end(), a) == keep.end()) {
      a->setChecked(false);
    }
  }
}

//RR AUFRÄUMEN / RÜCKSETZEN, ported from areg, the slots empty and the
// views return to the plain wheel
void MainWindow::clear_slots() {
  //RR DATEN und GESPEICHERTE BILDER dieser Sitzung LÖSCHEN ?
  const auto answer = QMessageBox::question(this, tr("RÜCKSETZEN ?"),
                                            tr("DATEN und GESPEICHERTE BILDER dieser Sitzung LÖSCHEN ?"),
                                            QMessageBox::Ok | QMessageBox::Cancel);
  if (answer != QMessageBox::Ok) {
    return;
  }
  // the steps of Zurück and Vor are data of this session too
  history_timer_->stop();
  pending_.reset();
  back_.clear();
  forward_.clear();
  update_history_actions();
  slots_.fill(std::nullopt);
  double_slots_.fill(std::nullopt);
  update_double_actions();
  solar_slots_.fill(std::nullopt);
  solar_labels_.fill(QString());
  radix_labels_.fill(QString());
  active_slot_ = -1;
  active_solar_ = -1;
  active_is_solar_ = false;
  // areg11 calls uhr_kon_l, the running clock stops with its slots
  clock_off();
  //RR GESPEICHERTE BILDER dieser Sitzung, the last picture and the double memory
  last_page_ = DisplayList{};
  last_shot_ = QPixmap();
  double_clear();
  update_slot_actions();
  update_solar_actions();
  reset_views();
}

// ported from ave with plgen11, plgenkl and fixp_def. One question per
// box, every answer moves on to the next topic like his INC as&, R or
// PgUp steps back like his zurueck!, EXIT or ESC anywhere ends the chain
// and the distance topic closes it
void MainWindow::vorgaben_ephemeride() {
  enum Topic { kAppa, kNode, kParallax, kExtras, kFixpunkt, kDistance, kTopicCount };
  // his ue$(0) and ze$() of ave
  int topic = ChoiceDialog::ask(this, tr("GEWÜNSCHTES THEMA ANKLICKEN !"), {},
                                {tr("MODUS DER Planeten-POSITIONEN ?"),
                                 tr("MONDKNOTEN : MITTELWERT ? oder WAHRER Wert ?"),
                                 tr("SO,MO und Planeten MIT oder OHNE Parallaxe ?"), tr("ZUSATZ - PLANETEN WÄHLEN ?"),
                                 tr("FIXPUNKT als 'PLANET' DEFINIEREN ?"),
                                 tr("ENTFERNUNGSWERTE der GROßEN PLANETEN RELATIV oder ABSOLUT ?"), tr("EXIT")},
                                0);
  // the answer of one box, the chain goes on, steps back or ends
  enum class Next { kOn, kBack, kEnd };
  const auto next_of = [](int answer, int exit_row) {
    if (answer == ChoiceDialog::kBack) {
      return Next::kBack;
    }
    return answer < 0 || answer == exit_row ? Next::kEnd : Next::kOn;
  };
  while (topic >= kAppa && topic < kTopicCount) {
    Next next = Next::kOn;
    switch (topic) {
      case kAppa: {
        const int es = ChoiceDialog::ask_step(this, tr("MODUS DER Planeten-POSITIONEN ?"), {},
                                              {tr("APPARENT 1 = Mit LICHTLAUFZEIT-EFFEKT"),
                                               tr("APPARENT 2 = ZUSÄTZLICH ABERRATION"),
                                               tr("WAHR = GEOMETRISCHE POSITION"), tr("EXIT")},
                                              std::clamp(konsta_.appa, 1, 3) - 1);
        next = next_of(es, 3);
        if (es >= 0 && es <= 2) {
          konsta_.appa = es + 1;
          // his appa$ = "App.1", "App.2" bzw. "Wahr"
          konsta_.appa_name = es == 0 ? "App.1" : (es == 1 ? "App.2" : "Wahr");
          recompute();
        }
        break;
      }
      case kNode: {
        const int es = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                              {tr("MONDKNOTEN :"), tr("MITTELWERT ?"), tr("oder"), tr("WAHRER WERT ?")},
                                              {tr("WAHR = MOMENTAN"), tr("MITTEL"), tr("EXIT")},
                                              true_node_->isChecked() ? 0 : 1);
        next = next_of(es, 2);
        if (es == 0) {
          true_node_->setChecked(true);
        } else if (es == 1) {
          true_node_->setChecked(false);
        }
        break;
      }
      case kParallax: {
        // his plgen11
        const int es = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                              {tr("SO,MO und Planeten mit Parallaxe ?"), tr("Vom EREIGNISORT aus")},
                                              {tr("Mit = Topozentrisch"), tr("Ohne = Geozentrisch"), tr("EXIT")},
                                              parallax_->isChecked() ? 0 : 1);
        next = next_of(es, 2);
        if (es == 0) {
          parallax_->setChecked(true);
        } else if (es == 1) {
          parallax_->setChecked(false);
        }
        break;
      }
      case kExtras: {
        // his plgenkl with the chosen tags under BISHER GEWÄHLT
        QStringList chosen;
        if (extras_->isChecked()) {
          chosen << "CH QU XE";
        }
        if (hamburg_->isChecked()) {
          chosen << tr("HAMBURGER PLANETEN");
        }
        if (apogee_show_->isChecked()) {
          chosen << "AG";
        }
        const int es = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                              {tr("ZUSATZ - PLANETEN ?"), tr("BISHER GEWÄHLT :"),
                                               chosen.isEmpty() ? tr("KEINE") : chosen.join("   ")},
                                              {tr("NEU - WAHL"), tr("KEINE"), tr("NICHT ÄNDERN"), tr("EXIT")}, 2);
        next = next_of(es, 3);
        if (es == 0) {
          planet_selection();
        } else if (es == 1) {
          extras_->setChecked(false);
          hamburg_->setChecked(false);
          apogee_show_->setChecked(false);
          true_apogee_->setChecked(false);
          included_.fill(false);
        }
        break;
      }
      case kFixpunkt: {
        // his fixp_def, a defined point is kept by default, else EXIT
        // stands ready. The grey rows are his blank and his note
        const bool defined = fixpunkt_ >= 0.0;
        const QString gz0 = defined ? zodiac_text(fixpunkt_, ZodiacForm::kGz0) : QString();
        const QStringList rows =
            defined ? QStringList{tr("FIXPUNKT %1 BEIBEHALTEN").arg(gz0), tr("FIXPUNKT als EKLIPTIK-GRAD NEU DEFINIEREN"),
                                  tr("FIXPUNKT %1 LÖSCHEN").arg(gz0), " ", tr("EXIT")}
                    : QStringList{tr("KEIN FIXPUNKT  DEFINIERT !"), tr("FIXPUNKT als EKLIPTIK-GRAD NEU DEFINIEREN"), " ",
                                  tr("KEINEN FIXPUNKT DEFINIEREN !"), tr("EXIT")};
        const int es = ChoiceDialog::ask_step(this, tr("FIXPUNKT als 'PLANET' DEFINIEREN ?"), {}, rows,
                                              defined ? 0 : 4, defined ? std::vector<int>{3} : std::vector<int>{0, 2});
        next = next_of(es, 4);
        if (es == 1) {
          // his input_grmise_zod(ekl$ + eg$ + " !")
          const std::optional<double> wh = ask_zodiac_position(this, tr("Ekliptikale Länge Eingeben !"));
          if (wh) {
            fixpunkt_ = norm_rad(*wh);
            recompute();
          }
        } else if (es == 2 && defined) {
          fixpunkt_ = -1.0;
          recompute();
        }
        break;
      }
      case kDistance: {
        const int es = ChoiceDialog::ask_step(this, tr("AUSWAHL"),
                                              {tr("ENTFERNUNGSWERTE der GROßEN PLANETEN"), tr("In % des MITTELWERTES"),
                                               tr("MITTELWERT = 100 %"), tr("Oder ABSOLUT in AE ?")},
                                              {tr("PROZENTUAL"), tr("ABSOLUT"), tr("EXIT")}, konsta_.entf == 1 ? 0 : 1);
        next = next_of(es, 2);
        if (es == 0 || es == 1) {
          konsta_.entf = es + 1;
          recompute();
        }
        // his GOTO avee, the last topic ends the chain
        if (next == Next::kOn) {
          next = Next::kEnd;
        }
        break;
      }
      default:
        break;
    }
    if (next == Next::kEnd) {
      break;
    }
    topic = next == Next::kBack ? std::max(static_cast<int>(kAppa), topic - 1) : topic + 1;
  }
  // his @param_sp, the Vorgaben written on the way out
  persist_konsta();
}

// ported from mainkont, the parameter panel of his main screen as one
// overview box, every line a live value
void MainWindow::vorgaben_overview() {
  QDialog dialog(this);
  //RR Parameter - Einstellungen = VORGABEN
  dialog.setWindowTitle(tr("Parameter - Einstellungen = VORGABEN"));
  auto* outer = new QVBoxLayout(&dialog);
  outer->setContentsMargins(18, 16, 18, 16);
  // his IF grossj! && jdgross > 0 && zal_grossj& <> 0, the yellow box on
  // top of the main screen with the grossj1 lines of the active record
  if (great_year_on_ && konsta_.jdgross > 0.0 && konsta_.zal_grossj != 0 && last_chart_) {
    bool outside = false;
    QStringList lines = great_year_lines(outside);
    lines << tr(" Datensatz : %1 ").arg(record_label_.trimmed());
    auto* box = new QLabel(lines.join(QChar(0x0A)), &dialog);
    box->setObjectName("greatYearBox");
    box->setAlignment(Qt::AlignHCenter);
    box->setStyleSheet("QLabel { background: #ffff00; color: #000000; border: 1px solid #000000; padding: 4px 12px; }");
    outer->addWidget(box, 0, Qt::AlignHCenter);
  }
  auto* row = new QHBoxLayout();
  outer->addLayout(row);
  row->setSpacing(28);
  const auto head = [](const QString& s) { return theme::heading_span(s) + "<br>"; };
  const ClassicSheetText sheet = classic_sheet_text();
  QString left;
  left += head(tr("Name :")) + QString::fromStdString(sheet.name).toHtmlEscaped() + "<br>";
  left += head(tr("Ort :")) + QString::fromStdString(sheet.place).toHtmlEscaped() + "<br>";
  left += QString::fromStdString(sheet.lon).toHtmlEscaped() + "<br>" +
          QString::fromStdString(sheet.lat).toHtmlEscaped() + "<br><br>";
  left += QString::fromStdString(sheet.date).toHtmlEscaped() + "<br>" +
          QString::fromStdString(sheet.ut).toHtmlEscaped() + "<br><br>";
  if (!record_.comment.empty()) {
    left += head(tr("BEM:")) + QString::fromStdString(record_.comment).toHtmlEscaped() + "<br><br>";
  }
  left += head(tr("HÄUSER : %1").arg(houses_->currentText()));
  left += "<br>" + head(QString::fromStdString(sheet.mode));
  //RR Ebene : RADIX
  left += head(tr("Ebene : %1").arg(uhr_slot_ >= 0 && active_slot_ == uhr_slot_ && !active_is_solar_ ? "UHR" : "RADIX"));
  QString mid;
  mid += head(tr("PARAM. Ephemeride:"));
  mid += tr("Ekl.Länge: %1").arg(QString::fromStdString(konsta_.appa_name.empty() ? "App.1" : konsta_.appa_name)) + "<br>";
  mid += (parallax_->isChecked() ? tr("Mit Parallaxe") : tr("Ohne Parallaxe")) + QString("<br>");
  mid += (true_node_->isChecked() ? tr("Wahrer Mondknoten") : tr("Mittl. Mondknoten")) + QString("<br>");
  mid += (true_apogee_->isChecked() ? tr("Wahres Apogäum") : tr("Mittl. Apogäum")) + QString("<br><br>");
  QStringList extra;
  if (extras_->isChecked()) {
    extra << "CH  QU  XE";
  }
  if (apogee_show_->isChecked()) {
    extra << "AG";
  }
  // his Andere Elemente list, only body tags whose Planeten-Auswahl
  // said include ride here, so the overview reads what the wheel does
  QStringList andere;
  for (int i = 2; i <= 22; ++i) {
    const int slot = current_settings().nk[static_cast<std::size_t>(i)];
    if (slot <= 0) continue;
    if (slot == body::kChiron || slot == body::kQuaoar || slot == body::kXena || slot == body::kApogee) {
      continue;
    }
    andere << QString::fromUtf8(body::kName[static_cast<std::size_t>(slot)].data(),
                                static_cast<int>(body::kName[static_cast<std::size_t>(slot)].size()));
  }
  if (!andere.isEmpty()) {
    extra << andere.join(" ");
  }
  mid += head(tr("Zusatz-Plan:")) + (extra.isEmpty() ? tr("KEINE") : extra.join("  ")) + "<br><br>";
  mid += head(tr("PARAM. Horoskop:"));
  //RR Aspekt-Orbes : Selbst definiert bzw. Von HORCOM gegeben
  mid += (aspect_settings_.equal_probability ? tr("Aspekt-Orbes : Selbst definiert")
                                             : tr("Aspekt-Orbes : Von HORCOM gegeben")) + QString("<br>");
  mid += tr("Orbis-Faktor = %1").arg(aspect_settings_.orb) + "<br>";
  // his m$ and e$ = "Aspekte 1....", voll! wins with Kompakt-Auswertung
  QString m = aspect_settings_.divisors <= 1 ? tr("Ohne Asp.-Linien")
                                             : tr("Aspekte 1....%1").arg(aspect_settings_.divisors);
  if (konsta_.voll) {
    m = tr("Kompakt-Auswertung");
  }
  mid += m + "<br>";
  mid += (konsta_.klsy ? tr("Symbole : Klein") : tr("Symbole : Normal")) + QString("<br>");
  mid += (konsta_.pziff < 3.0 ? tr("Mit Grad-Anzeige") : tr("Ohne Grad-Anzeige")) + QString("<br>");
  //RR Beginn Horoskop : Aszendent, MC, 0 Widder, 0 Waage, Eigene Wahl
  static constexpr const char* kBegin[5] = {QT_TR_NOOP("Aszendent"), QT_TR_NOOP("MC"), QT_TR_NOOP("0 Widder"),
                                            QT_TR_NOOP("0 Waage"), QT_TR_NOOP("Eigene Wahl")};
  mid += tr("Beginn Horoskop : %1").arg(tr(kBegin[std::clamp(konsta_.begz, 1, 5) - 1])) + "<br>";
  mid += (alt_rulers_ ? tr("Zuordng.ZE-PL: Alt") : tr("Zuordng.ZE-PL: Neu")) + QString("<br>");
  //RR COMPOS. Mittl. STZ, COMPOS. n. R.HAND or COMPOSIT Schemat.
  mid += (konsta_.comp_mstz ? tr("COMPOS. Mittl. STZ")
                            : (konsta_.comp_hand ? tr("COMPOS. n. R.HAND") : tr("COMPOSIT Schemat."))) +
         QString("<br>");
  // his sc$ = "Farbe : " with Schraffur, Pur or Weiß
  QString ring = tr("Schraffur");
  if (konsta_.farbp) {
    ring = tr("Pur");
  }
  if (konsta_.weiss) {
    ring = tr("Weiß");
  }
  mid += tr("Farbe : %1").arg(ring) + "<br>";
  mid += (fixpunkt_ >= 0.0 ? tr("Fixpunkt : %1°").arg(fixpunkt_ * kRadToDeg, 0, 'f', 2)
                           : tr("Kein Fixpunkt")) + QString("<br>");
  mid += (konsta_.entf == 1 ? tr("Entfernungen : prozentual") : tr("Entfernungen : absolut in AE"));
  QString right;
  right += head(tr("Daten-Datei:"));
  if (data_file_.isEmpty()) {
    right += tr("keine") + QString("<br>");
  } else {
    right += QFileInfo(data_file_).fileName().toHtmlEscaped() + "<br>";
    right += tr("Anzahl Dats.: %1").arg(data_count_) + "<br>";
  }
  // his deftextcol(0) with "Drucker-Option EIN" or "AUS", "Hardcopy : DIN A5" or "DIN A4"
  right += "<br>" + head(konsta_.prenbl != 0 ? tr("Drucker-Option EIN") : tr("Drucker-Option AUS"));
  right += head(konsta_.halbs != 0 ? tr("Hardcopy : DIN A5") : tr("Hardcopy : DIN A4"));
  right += "<br>" + head(tr("Auflösung:"));
  right += QString("X:Y = %1: %2").arg(width()).arg(height()) + "<br><br>";
  // the dress of the shell is a rewrite addition, his Farbe line above
  // names the sign band
  right += head(tr("Ansicht :")) + (theme::dark_now() ? tr("Nachthimmel") : tr("Weiß"));
  for (const QString& text : {left, mid, right}) {
    auto* label = new QLabel(text, &dialog);
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    row->addWidget(label, 1);
  }
  dialog.exec();
}

// ported from hausw, his ten systems without an ABBRUCH, ESC keeps the
// choice. The HÄUSER-TABELLE of hausa follows every real house system
void MainWindow::choose_house_system() {
  const QStringList items{"PLACIDUS",
                          "TOPOZENTRISCH",
                          "KOCH-GOH",
                          "REGIOMONTANUS",
                          "CAMPANUS",
                          "ÄQUAL  EKLIPTIKAL ab AC",
                          "ÄQUAL  EKLIPTIKAL n. VEHLOW",
                          tr("KEINE Häuser,NUR AC und MC"),
                          tr("WEDER HÄUSER noch  AC oder MC"),
                          tr("WEDER HÄUSER noch  AC oder MC noch MONDKNOTEN")};
  const int es = ChoiceDialog::ask(this, tr("HÄUSERSYSTEM WÄHLEN !"), {}, items, houses_->currentIndex());
  if (es >= 0 && es < houses_->count()) {
    houses_->setCurrentIndex(es);
  }
  // his param_sp keeps the choice with the Vorgaben
  konsta_.haw = houses_->currentIndex() + 1;
  konsta_.haus = std::string(compute_houses(static_cast<HouseSystem>(konsta_.haw), 0.0, 0.0, 0.0).name);
  persist_konsta();
  // his IF haw& > 0 && haw& < 8 && ze > 0 && od > 0, @hausa
  if (konsta_.haw < 8 && (record_.day > 0 || record_.jd > 0.0)) {
    house_table();
  }
}

// ported from a210 through a2113, HOLEN fills the free slots and every
// record passes through the ANZEIGE box on its way in
void MainWindow::fetch_from_file() {
  // the family wants to pick the file on every fetch like in the
  // original, without going back through the hub. The dialog takes the
  // current file as default and offers another file beside it
  {
    const QString label = data_file_.isEmpty()
                              ? tr("keine gebunden")
                              : QFileInfo(data_file_).fileName();
    const int action = ChoiceDialog::ask(
        this, tr("Datensätze HOLEN"),
        {tr("Aus welcher Daten-Datei sollen Datensätze geholt werden?")},
        {tr("Aus %1").arg(label), tr("Andere Datei wählen…"), tr("ABBRUCH")},
        data_file_.isEmpty() ? 1 : 0);
    if (action < 0 || action == 2) {
      return;
    }
    if (action == 1 || data_file_.isEmpty()) {
      const QString path = QFileDialog::getOpenFileName(
          this, tr("Andere Daten-Datei wählen"), collection_start(data_file_, data_dir_),
          tr("HORCOM Daten-Dateien (*.DAT *.dat);;AAF (*.AAF *.aaf)"));
      if (path.isEmpty()) {
        return;
      }
      std::filesystem::path dat(path.toStdWString());
      if (path.endsWith(".aaf", Qt::CaseInsensitive)) {
        dat = dat_twin_path(dat);
      }
      if (!std::filesystem::exists(dat)) {
        QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht öffnen."));
        return;
      }
      bind_data_file(QString::fromStdWString(dat.wstring()));
    }
  }
  const auto records = load_collection(data_file_);
  if (!records || records->empty()) {
    QMessageBox::warning(this, "HORCOM", tr("Keine Datensätze gefunden."));
    return;
  }
  const QString label = QFileInfo(data_file_).fileName();
  const auto order = ask_order(*records, label);
  if (order.empty()) {
    return;
  }
  int free = 0;
  for (const auto& s : slots_) {
    free += s ? 0 : 1;
  }
  const int max_pick = free > 0 ? free : 1;
  RecordListDialog list(*records, order, label, max_pick, RecordListDialog::Mode::kFetch, this);
  if (next_slot() >= 0) {
    list.set_preview([this](const AafRecord& r) { preview_record(r); });
  }
  if (list.exec() != QDialog::Accepted || list.picked().empty()) {
    return;
  }
  bool first = true;
  const std::filesystem::path dat(data_file_.toStdWString());
  for (const std::size_t i : list.picked()) {
    const int slot = claim_radix_slot();
    if (slot < 0) {
      break;
    }
    //RR EINGABE- und ANZEIGE-BOX | RADIX NR.n
    RecordMaskDialog mask((*records)[i], tr("EINGABE- und ANZEIGE-BOX | RADIX NR.%1").arg(slot + 1),
                          RecordMaskDialog::Mode::kShow, data_dir_, this);
    mask.bind_file(label, aaf_twin_path(dat));
    if (mask.exec() != QDialog::Accepted) {
      continue;
    }
    // his ABSPEICHERN of the edited record works on the record in view
    set_slot(slot, mask.record(), first || mask.save_requested());
    first = false;
    if (mask.aaf_saved()) {
      // his aaf_satz_speich, datru$ = horcfile$, the DAT twin is the file
      const std::filesystem::path twin_dat = dat_twin_path(*mask.aaf_saved());
      if (std::filesystem::exists(twin_dat)) {
        bind_data_file(QString::fromStdWString(twin_dat.wstring()));
      }
    }
    if (mask.save_requested()) {
      store_record(false);
    }
  }
}

// ported from the LÖSCHEN branch of a2fdat and ausw_datei, the marked
// records leave the file, on request their AAF twins go with them. Weiter
// LÖSCHEN runs another round, LÖSCHEN Beenden rebuilds the DAT from its
// AAF twin like his aaf_horcom2. His rebuild also ran after AAF-Datensätze
// BEIBEHALTEN and brought every deleted record back, the port rebuilds
// only when the AAF file lost the same records
void MainWindow::delete_from_file() {
  const std::filesystem::path dat(data_file_.toStdWString());
  const std::filesystem::path twin = aaf_twin_path(dat);
  const QString label = QFileInfo(data_file_).fileName();
  bool aaf_kept = false;
  for (;;) {
    auto stored = read_chart_file(dat);
    if (!stored || stored->empty()) {
      QMessageBox::warning(this, "HORCOM", tr("Keine Datensätze gefunden."));
      break;
    }
    bool also_aaf = false;
    if (std::filesystem::exists(twin)) {
      //RR Wollen Sie auch die korrespondierenden AAF-Datensätze LÖSCHEN ?
      const int es = ChoiceDialog::ask(this, "HORCOM",
                                       {tr("Wollen Sie auch die korrespondierenden AAF-Datensätze"),
                                        tr("der Datei %1 LÖSCHEN ?").arg(label),
                                        tr("Dies ist in der Regel zweckmäßig !")},
                                       {tr("JA = LÖSCHEN"), tr("AAF-Datensätze BEIBEHALTEN")});
      if (es < 0) {
        break;
      }
      also_aaf = es == 0;
    }
    std::vector<AafRecord> view;
    view.reserve(stored->size());
    for (const ChartRecord& c : *stored) {
      view.push_back(aaf_from_chart_record(c));
    }
    const auto order = ask_order(view, label);
    if (order.empty()) {
      break;
    }
    //RR Bis zu 10 zu LÖSCHENDE DATENSÄTZE markieren !
    RecordListDialog list(view, order, label, 10, RecordListDialog::Mode::kDelete, this);
    if (list.exec() != QDialog::Accepted || list.picked().empty()) {
      break;
    }
    const int sure = ChoiceDialog::ask(this, tr("DATEI : %1").arg(label), {},
                                       {tr("%1 Datensätze LÖSCHEN ?").arg(list.picked().size()), tr("ABBRUCH")});
    if (sure != 0) {
      break;
    }
    //RR RRESERVE.DAT, the reserve copy before the destructive pass
    std::error_code ec;
    std::filesystem::copy_file(dat, dat.parent_path() / "RRESERVE.DAT",
                               std::filesystem::copy_options::overwrite_existing, ec);
    std::vector<std::string> doomed_names;
    for (const std::size_t i : list.picked()) {
      doomed_names.push_back((*stored)[i].name);
    }
    delete_records(*stored, list.picked());
    if (!write_chart_file(dat, *stored)) {
      QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht schreiben."));
      break;
    }
    if (also_aaf) {
      if (auto aaf_records = read_aaf(twin)) {
        for (const std::string& name : doomed_names) {
          // aaf_ident, the record of the same name in the AAF file
          if (const auto k = aaf_ident(*aaf_records, name)) {
            aaf_records->erase(aaf_records->begin() + static_cast<std::ptrdiff_t>(*k));
          }
        }
        write_aaf(twin, *aaf_records);
      }
    } else if (std::filesystem::exists(twin)) {
      aaf_kept = true;
    }
    //RR WEITER in der DATEI ... LÖSCHEN ?
    const int more = ChoiceDialog::ask(this, "HORCOM", {tr("WEITER in der DATEI"), label, tr("LÖSCHEN ?")},
                                       {tr("LÖSCHEN Beenden"), tr("Weiter LÖSCHEN")});
    if (more == 1) {
      continue;
    }
    // his aaf_horcom2 at the end, the DAT follows its AAF pilot again
    if (std::filesystem::exists(twin) && !aaf_kept) {
      if (const auto aaf_records = read_aaf(twin)) {
        write_dat_from_aaf(dat, *aaf_records);
      }
    }
    break;
  }
  bind_data_file(data_file_);
}

// ported from Datei TRIMMEN and Datei MINIMIEREN, the file is replaced
// only when the pass removed something, like the original rename rule
void MainWindow::tidy_data_file(bool minimize) {
  const std::filesystem::path dat(data_file_.toStdWString());
  auto records = read_chart_file(dat);
  if (!records) {
    QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht lesen."));
    return;
  }
  const std::size_t before = records->size();
  std::error_code ec;
  std::filesystem::copy_file(dat, dat.parent_path() / "RRESERVE.DAT",
                             std::filesystem::copy_options::overwrite_existing, ec);
  if (minimize) {
    minimize_records(*records);
  } else {
    trim_records(*records);
  }
  if (records->size() == before) {
    QMessageBox::information(this, "HORCOM", tr("Nichts zu bereinigen, %1 Datensätze.").arg(before));
    return;
  }
  if (!write_chart_file(dat, *records)) {
    QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht schreiben."));
    return;
  }
  QMessageBox::information(this, "HORCOM",
                           tr("%1 von %2 Datensätzen bleiben.").arg(records->size()).arg(before));
  bind_data_file(data_file_);
}

// ported from a3 with the OK path of eingabe. One record per call on the
// HORCOM path, his WEITEREN question lived only on the AAF path
void MainWindow::new_records_entry() {
  // IF EXIST(aafpth$ + "\*.AAF"), the AAF folder beside the working file
  const std::filesystem::path aaf_dir = aaf_folder(data_file_, data_dir_);
  bool aaf_files = false;
  {
    std::error_code ec;
    for (const auto& e : std::filesystem::directory_iterator(aaf_dir, ec)) {
      if (QString::fromStdWString(e.path().extension().wstring()).toUpper() == ".AAF") {
        aaf_files = true;
        break;
      }
    }
  }
  int format = 0;
  if (aaf_files) {
    format = ChoiceDialog::ask(this, "HORCOM", {tr("In welchem Daten-FORMAT EINGEBEN ?")},
                               {tr("HORCOM - Format"), tr("AAF-Format"), tr("ABBRUCH")});
    if (format != 0 && format != 1) {
      return;
    }
  }
  const auto claim = [this]() { return claim_radix_slot(); };

  if (format == 1) {
    // the AAF path keeps his loop over further records
    for (;;) {
      const int slot = claim();
      if (slot < 0) {
        return;
      }
      // aafein!, OK = Speichern writes into the AAF file of the working
      // Daten-Datei by default and rebuilds its DAT twin
      AafMaskDialog mask(AafRecord{}, AafMaskDialog::Mode::kEntry, data_dir_, this);
      mask.set_aaf_file(data_file_.isEmpty() ? aaf_dir
                                             : aaf_twin_path(std::filesystem::path(data_file_.toStdWString())));
      if (mask.exec() != QDialog::Accepted) {
        return;
      }
      const AafRecord got = mask.record();
      if (got.day <= 0 && got.jd <= 0.0) {
        //RR Kein Datensatz eingegeben !
        QMessageBox::information(this, "HORCOM", tr("Kein Datensatz eingegeben !"));
        return;
      }
      if (mask.saved_file()) {
        // aaf_satz_speich, datru$ = horcfile$, the twin is the working file
        const std::filesystem::path dat = dat_twin_path(*mask.saved_file());
        if (std::filesystem::exists(dat)) {
          bind_data_file(QString::fromStdWString(dat.wstring()));
        }
      }
      set_slot(slot, got, true);
      //RR WEITEREN Datensatz NEU EINGEBEN ?
      const int more = ChoiceDialog::ask(this, "HORCOM", {tr("WEITEREN Datensatz NEU EINGEBEN ?")},
                                         {tr("Zurück zum HAUPT - MENÜ"), tr("WEITERE NEU - EINGABE")});
      if (more != 1) {
        return;
      }
    }
  }

  const int slot = claim();
  if (slot < 0) {
    return;
  }
  // a fresh mask like his et$ cleared by nein!, the clock counts as UT
  // until the zone box says otherwise
  AafRecord seed;
  bool seed_local = false;
  for (;;) {
    RecordMaskDialog mask(seed, tr("NEU-EINGABE von DATENSÄTZEN  :   |  RADIX NR.%1").arg(slot + 1),
                          RecordMaskDialog::Mode::kEntry, data_dir_, this);
    mask.preset_local_time(seed_local);
    if (mask.exec() != QDialog::Accepted) {
      return;
    }
    AafRecord got = mask.record();
    seed = got;
    seed_local = mask.local_time();
    //RR DATUM FEHLT !
    // his mask emptied every field here, the port keeps them so the entry
    // is not lost
    if (got.day <= 0 && got.jd <= 0.0) {
      QMessageBox::information(this, "HORCOM", tr("DATUM FEHLT !"));
      continue;
    }
    //RR Ungültige geogr. Länge !
    if (std::abs(got.longitude()) > 180.0) {
      QMessageBox::information(this, "HORCOM", tr("Ungültige geogr. Länge !"));
      seed.lon_deg = 0;
      seed.lon_min = 0;
      seed.lon_sec = 0;
      continue;
    }
    if (seed_local) {
      // zuo with ortsz!, the panel asks his WAHRE or MITTLERE questions and
      // hands back the moment with the longitude as zone
      apply_record(got, false);
      set_local_time(true, true);
      recompute();
      got = panel_record();
    }
    //RR Datensatz ABSPEICHERN ? Oder Weiter EDITIEREN ?
    const int es = ChoiceDialog::ask(this, "HORCOM", {tr("Datensatz ABSPEICHERN ?"), QString(), tr("Oder Weiter EDITIEREN ?")},
                                     {tr("ABSPEICHERN"), tr("Weiter EDITIEREN"), tr("NUR als Datensatz ÜBERNEHMEN")}, 2);
    if (es == 1) {
      continue;
    }
    if (es < 0) {
      return;
    }
    set_slot(slot, got, true);
    if (es == 0) {
      // a2dat, FILESELECT SPEZIAL\*.DAT with the last file as default,
      // a new name starts a file with this record as its first
      const QString path = with_suffix(
          QFileDialog::getSaveFileName(this, tr("Daten-Datei wählen oder neu anlegen"),
                                       collection_start(data_file_, data_dir_), tr("HORCOM Daten-Dateien (*.DAT *.dat)"),
                                       nullptr, QFileDialog::DontConfirmOverwrite),
          "DAT");
      if (path.isEmpty()) {
        return;
      }
      const std::filesystem::path dat(path.toStdWString());
      if (!std::filesystem::exists(dat)) {
        if (!write_chart_file(dat, {dat_from_record(got)})) {
          QMessageBox::warning(this, "HORCOM", tr("Die Datei ließ sich nicht anlegen."));
          return;
        }
        bind_data_file(path);
        //RR NEUE DATEN-DATEI <path> !
        QMessageBox::information(this, "HORCOM",
                                 tr("NEUE DATEN-DATEI %1 !").arg(QString::fromStdWString(dat.filename().wstring())));
        return;
      }
      bind_data_file(path);
      store_record(true);
    }
    return;
  }
}

// ported from avg, VORGABEN EIN-AUSGABE ÄNDERN. The topics follow one
// another like his INC as&, EXIT ends the walk and the answers are kept
// like his param_sp
void MainWindow::vorgaben_ein_ausgabe() {
  const QString exit = tr("EXIT");
  //RR GEWÜNSCHTES THEMA ANKLICKEN !
  int topic = ChoiceDialog::ask(this, tr("GEWÜNSCHTES THEMA ANKLICKEN ! "), {},
                                {tr("EINGABE-MODUS in EDITIERFELDERN ?"),
                                 tr("DRUCKER-OPTION beim Programmstart EINSCHALTEN ?"),
                                 tr("FORMAT für HORCOM - HARDCOPY"), exit});
  for (; topic >= 0 && topic <= 2; ++topic) {
    int r = -1;
    if (topic == 0) {
      //RR EINGABE-MODUS in EDITIERFELDERN ?
      r = ChoiceDialog::ask(this, "HORCOM", {tr("EINGABE-MODUS in EDITIERFELDERN ?")},
                            {tr("MIT TABSTOP oder MAUS"), tr("AUTOMATISCH WEITERSCHALTEN"), exit},
                            konsta_.tabstop == 1 ? 0 : 1);
      if (r == 0 || r == 1) {
        konsta_.tabstop = r == 0 ? 1 : 0;
        set_auto_advance(konsta_.tabstop == 0);
      }
    } else if (topic == 1) {
      //RR DRUCKER-OPTION beim Programmstart EINSCHALTEN ?
      r = ChoiceDialog::ask(this, "HORCOM", {tr("DRUCKER-OPTION beim Programmstart EINSCHALTEN ?")},
                            {tr("NEIN"), tr("JA"), exit}, konsta_.prenbl == 0 ? 0 : 1);
      if (r == 0 || r == 1) {
        konsta_.prenbl = r;
      }
    } else {
      //RR FORMAT für HORCOM - HARDCOPY ?
      r = ChoiceDialog::ask(this, "HORCOM", {tr("FORMAT für HORCOM - HARDCOPY ?")},
                            {tr("HALB-SEITE"), tr("GANZ-SEITE"), exit}, konsta_.halbs == 1 ? 0 : 1);
      if (r == 0 || r == 1) {
        konsta_.halbs = r == 0 ? 1 : 0;
      }
    }
    if (r < 0 || r == 2) {
      break;
    }
  }
  //RR @param_sp
  persist_konsta();
}

// the partner chooser, loaded slots first like the original, the file
// only when the wanted chart is not in memory yet
std::optional<AafRecord> MainWindow::choose_record(const QString& title) {
  QStringList buttons;
  std::vector<int> map;
  for (int i = 0; i < 5; ++i) {
    if (slots_[static_cast<std::size_t>(i)] && i != active_slot_) {
      buttons << QString("SATZ%1: RADIX  %2").arg(i + 1).arg(slot_name(*slots_[static_cast<std::size_t>(i)]));
      map.push_back(i);
    }
  }
  if (!buttons.isEmpty()) {
    buttons << tr("Aus Datei wählen …") << tr("ABBRUCH");
    const int es = ChoiceDialog::ask(this, title, {}, buttons);
    if (es < 0 || es == buttons.size() - 1) {
      return std::nullopt;
    }
    if (es < static_cast<int>(map.size())) {
      return slots_[static_cast<std::size_t>(map[static_cast<std::size_t>(es)])];
    }
  }
  return choose_record_from_file(title);
}

std::optional<AafRecord> MainWindow::choose_record_from_file(const QString& title) {
  QString path = data_file_;
  if (path.isEmpty()) {
    path = QFileDialog::getOpenFileName(this, title, QString(),
                                        tr("HORCOM Datensätze (*.DAT *.dat *.AAF *.aaf)"));
    if (path.isEmpty()) {
      return std::nullopt;
    }
  }
  const auto records = load_collection(path);
  if (!records || records->empty()) {
    QMessageBox::warning(this, "HORCOM", tr("Keine Datensätze gefunden."));
    return std::nullopt;
  }
  const QString label = QFileInfo(path).fileName();
  const auto order = ask_order(*records, label);
  if (order.empty()) {
    return std::nullopt;
  }
  RecordListDialog list(*records, order, label, 1, RecordListDialog::Mode::kSingle, this);
  if (list.exec() != QDialog::Accepted || list.picked().empty()) {
    return std::nullopt;
  }
  return (*records)[list.picked().front()];
}

// one 128 byte record from the exchange form, shared by the delete
// path, the converter and the save dialog
ChartRecord MainWindow::dat_from_record(const AafRecord& r) const {
  return chart_record_from_aaf(r);
}

// ported from the ZZD and Sommerzeit branches of aaf_horcom2, the
// julian date outranks the clock fields like the loader rule
double MainWindow::record_jd_ut(const AafRecord& r) const {
  return aaf_moment_jd_ut(r);
}

ChartInput MainWindow::record_input(const AafRecord& r) const {
  ChartInput in;
  in.date_ut = calendar_date(record_jd_ut(r), current_settings().calendar);
  in.lon_deg_east = r.longitude();
  in.lat_deg = r.latitude();
  return in;
}

bool MainWindow::set_partner(const AafRecord& r) {
  // an empty record has no day, years before Christ are welcome
  if (r.day <= 0 && r.jd <= 0.0) {
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
  partner_place_ = QString::fromStdString(r.place).trimmed();
  partner_record_ = r;
  return true;
}

// the od = 0 slots of his DOPPEL-DATEN rows, the pair a double chart was
// made from, named like na$ = LEFT$(na1$,10) + "-" + LEFT$(na2$,10)
void MainWindow::remember_double(int kind, const AafRecord& partner) {
  double_slots_[static_cast<std::size_t>(kind)] = DoubleSlot{record_, partner};
  update_double_actions();
}

void MainWindow::update_double_actions() {
  static constexpr const char* kCaption[kDoubleKinds] = {QT_TR_NOOP("COMPOSIT"), QT_TR_NOOP("COMBIN"),
                                                         QT_TR_NOOP("DOPPEL-KREIS")};
  const auto short_name = [](const AafRecord& r) {
    return QString::fromStdString(record_name(r)).left(10);
  };
  for (int k = 0; k < kDoubleKinds; ++k) {
    QAction* a = double_actions_[static_cast<std::size_t>(k)];
    if (a == nullptr) {
      continue;
    }
    const auto& slot = double_slots_[static_cast<std::size_t>(k)];
    //RR menu$ + ": " + TRIM$(na$(0,i& - 26)) + " "
    a->setText(slot ? QString("%1: %2-%3 ").arg(tr(kCaption[k]), short_name(slot->base), short_name(slot->partner))
                    : tr(kCaption[k]));
  }
}

// a DOPPEL-DATEN row, the stored pair comes back without asking, an
// empty row starts the chart like the HOROSKOPE menu
void MainWindow::recall_double(int kind) {
  const auto& slot = double_slots_[static_cast<std::size_t>(kind)];
  if (!slot) {
    if (kind == kDoubleCombin) {
      combin_chart();
    } else if (kind == kDoubleComposit) {
      composite_session();
    } else {
      // a12 with its MODUS box and the INNEN and AUSSEN picks
      double_wheel_session();
    }
    return;
  }
  // a13 and a14 CLR hrg! on every run, the stored pair too
  if ((kind == kDoubleCombin || kind == kDoubleComposit) && helio_->isChecked()) {
    helio_->setChecked(false);
  }
  const DoubleSlot pair = *slot;
  if (kind == kDoubleCombin) {
    leave_views({});
    apply_record(pair.base, false);
    combin_of(pair.parts, pair.sets);
    return;
  }
  QAction* view = kind == kDoubleComposit ? composite_action_ : compare_action_;
  leave_views({view});
  apply_record(pair.base, false);
  if (!set_partner(pair.partner)) {
    return;
  }
  if (kind == kDoubleComposit) {
    comp_residence_ = pair.residence;
  }
  {
    const QSignalBlocker block(view);
    view->setChecked(true);
  }
  claim_wheel();
  recompute();
}

void MainWindow::show_compare(const AafRecord& partner) {
  if (set_partner(partner)) {
    const QSignalBlocker block(compare_action_);
    compare_action_->setChecked(true);
    recompute();
  }
}

void MainWindow::show_composite(const AafRecord& partner) {
  leave_views({composite_action_});
  if (set_partner(partner)) {
    const QSignalBlocker block(composite_action_);
    composite_action_->setChecked(true);
    claim_wheel();
    recompute();
  }
}

void MainWindow::show_mundane() {
  set_mundane_frame(true);
}

void MainWindow::set_mundane_frame(bool on) {
  mundane_frame_ = on;
  konsta_.horm = on ? 2 : 1;
  if (on) {
    claim_wheel();
  }
  recompute();
  if (!on) {
    banner_->set_record(record_label_.trimmed());
  }
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
  dir_sums_ = AxesSums{};
  dir_lon_ = lon_->value();
  dir_lat_ = lat_->value();
  dir_event_planets_ = false;
  const QSignalBlocker block(directions_action_);
  directions_action_->setChecked(true);
  recompute();
}

void MainWindow::preset_extras(bool real, bool hamburg, bool apogee) {
  extras_->setChecked(real);
  hamburg_->setChecked(hamburg);
  apogee_show_->setChecked(apogee);
  // the capture hook's real switch names the whole group of
  // astronomically real bodies, not only the CH QU XE trio the panel
  // checkbox names. A false clears them so a later replay starts fresh
  for (int slot : {body::kChiron, body::kCeres, body::kPallas, body::kJuno, body::kVesta, body::kQuaoar,
                   body::kHalley, body::kPholus, body::kDamokles, body::kNessus, body::kXena}) {
    included_[static_cast<std::size_t>(slot)] = real;
  }
}

void MainWindow::preset_houses(int haw) {
  if (haw >= 1 && haw <= houses_->count()) {
    houses_->setCurrentIndex(haw - 1);
  }
}

void MainWindow::preset_chart(const AafRecord& r, bool parallax, bool true_node) {
  parallax_->setChecked(parallax);
  true_node_->setChecked(true_node);
  apply_record(r);
}

void MainWindow::apply_record(const AafRecord& r, bool claim_slot) {
  record_ = r;
  // a change to a radix chart drops the combin memory, its origin lines
  // must not stay on other drawings
  combin_name1_.clear();
  combin_moment1_.clear();
  combin_name2_.clear();
  combin_moment2_.clear();
  combin_note_.clear();
  combin_list_.clear();
  combin_na_.clear();
  prog_event_note_.clear();
  prog_radix_note_.clear();
  transit_place_.reset();
  // whatever becomes current also lives in a slot like his SATZ arrays,
  // a reactivated SOLAR snapshot leaves the radix slots untouched
  if (claim_slot) {
    active_is_solar_ = false;
    if (active_slot_ < 0) {
      active_slot_ = 0;
    }
    slots_[static_cast<std::size_t>(active_slot_)] = r;
    update_slot_actions();
    update_solar_actions();
  }
  // the record's clock runs on its zone and summer time. Where its julian
  // date disagrees with the clock fields the date wins like the loader
  // rule, the panel then shows that moment in UT
  set_local_time(false, false);
  set_panel_calendar(r.calendar);
  const CalendarDate local{r.day, r.month, r.year, static_cast<double>(r.hour), r.minute + r.second / 60.0};
  const double zone_hours = aaf_zone_hours(r.zone);
  const double dst = aaf_dst_hours(r.dst);
  const double from_fields = julian_day(local, r.calendar) - (zone_hours + dst) / kHoursPerDay;
  const bool jd_rules = r.jd > 0.0 && std::abs(from_fields - r.jd) * kSecondsPerDay > 30.0;
  CalendarDate shown = local;
  if (jd_rules) {
    shown = calendar_date(r.jd, r.calendar);
  }
  int seconds = jd_rules ? static_cast<int>((shown.hour * 60.0 + shown.minute) * 60.0 + 0.5)
                         : (r.hour * 60 + r.minute) * 60 + r.second;
  if (seconds >= kSecondsPerDay) {
    seconds = kSecondsPerDay - 1;
  }
  set_dst(jd_rules ? 0.0 : dst);
  const QSignalBlocker b1(date_);
  const QSignalBlocker b2(time_);
  const QSignalBlocker b3(zone_);
  const QSignalBlocker b4(lon_);
  const QSignalBlocker b5(lat_);
  set_panel_day(shown.day, shown.month, shown.year);
  time_->setTime(QTime(seconds / 3600, (seconds / 60) % 60, seconds % 60));
  zone_->setValue(jd_rules ? 0.0 : zone_hours);
  lon_->setValue(r.longitude());
  lat_->setValue(r.latitude());
  sync_coord_boxes();
  if (refresh_sommer_effect_) refresh_sommer_effect_();
  recompute();
  refresh_record_label();
}

void MainWindow::refresh_record_label() {
  // the panel name fields mirror the record without firing edits back
  {
    const QSignalBlocker bg(given_);
    const QSignalBlocker bs(surname_);
    const QSignalBlocker bp(place_field_);
    given_->setText(QString::fromStdString(record_.given).trimmed());
    surname_->setText(QString::fromStdString(record_.surname).trimmed());
    place_field_->setText(QString::fromStdString(record_.place).trimmed());
  }
  const CalendarDate d = panel_day();
  //RR vC, the historical count of his record list
  const QString year = d.year > 0 ? QString::number(d.year) : QString("%1 vC").arg(1 - d.year);
  record_label_ = QString("%1 %2   %3.%4.%5")
                      .arg(QString::fromStdString(record_.surname), QString::fromStdString(record_.given))
                      .arg(d.day, 2, 10, QChar('0'))
                      .arg(d.month, 2, 10, QChar('0'))
                      .arg(year);
  //RR JULIANISCH, his main screen marks the calendar of the record
  if (panel_calendar_ == Calendar::kJulian) {
    record_label_ += tr("  JULIANISCH");
  }
  banner_->set_record(record_label_.trimmed());
}

// ported from aspar, his MAXIMALER Teiler box under the orb banner, then
// the sheet. The divisor holds for this sheet only, nasp& comes back after
void MainWindow::open_aspektarium() {
  if (!last_chart_) {
    return;
  }
  // o1$ = " ORBES selbst definiert !" or " ORBES nach HORCOM- Zählung !"
  // + "   ORBIS - Faktor = " + STR$(orb)
  const QString banner = (aspect_settings_.equal_probability ? tr(" ORBES selbst definiert !")
                                                             : tr(" ORBES nach HORCOM- Zählung !")) +
                         tr("   ORBIS - Faktor = %1").arg(aspect_settings_.orb);
  QStringList answers{tr("  8  "), tr(" 12 ")};
  if (!aspect_settings_.equal_probability) {
    answers << tr(" 16 ");
  }
  // re& = @alertbox(2,"","MAXIMALER " + tl$ + " ?","","",2,...), the default is 12
  const int re = ChoiceDialog::ask(this, tr("AUSWAHL"), {banner, QString(), tr("MAXIMALER Teiler ?")}, answers, 1);
  if (re < 0) {
    return;
  }
  static constexpr int kDivisors[3] = {8, 12, 16};
  const int nasp = kDivisors[re];
  const Chart& chart = *last_chart_;
  const ChartSettings s = current_settings();
  const auto build = [&]() {
    AspectSettings scan_settings = shown_aspect_settings();
    scan_settings.divisors = nasp;
    const AspectResult scan = scan_aspects(chart, s, scan_settings);
    AspektariumInput in;
    in.chart = &chart;
    in.settings = s;
    in.aspects = &scan;
    in.orbs = aspect_settings_;
    in.orbs.divisors = nasp;
    in.weights = aspect_settings_.weight;
    in.emphasis = shown_emphasis();
    const WheelOptions dress = radix_wheel_options(chart, s);
    in.hist_colors = dress.hist_colors;
    in.hist_fill = dress.hist_fill;
    // plein2 stamps nodes and Black Moon like the wheel, ryt! drops the
    // histogram inset
    in.invert_nodes = dress.invert_nodes;
    in.invert_apogee = dress.invert_apogee;
    in.rhythm = konsta_.ryt;
    AspektariumText text;
    // TRIM$(horgt$) + "es Aspektarium  | " + sol$(od,ze)
    text.title = tr("%1es Aspektarium  | %2").arg(horgt_text().trimmed(), rhythm_chart_label()).toStdString();
    text.name = classic_sheet_text().name;
    text.date_label = (tr("Datum") + ":").toStdString();
    text.date = datum3_text(calendar_date(chart.jd_ut, s.calendar)).toStdString();
    text.heads = {tr("Tei-").toStdString(), tr("ler").toStdString(), tr("Win-").toStdString(),
                  tr("kel").toStdString(), tr("Or-").toStdString(), tr("bis").toStdString()};
    text.factor1 = tr("Orbis-").toStdString();
    text.factor2 = tr("Faktor = ").toStdString();
    text.legend = {tr("Diagramm :").toStdString(), tr("Unten:auf volle").toStdString(),
                   tr("Grade gerundet").toStdString(), tr("Darüber Teiler").toStdString(),
                   tr("Oberhalb Diago.").toStdString(), tr("Istwerte").toStdString()};
    text.name_label = tr("Name:").toStdString();
    text.weights1 = tr("Planeten-").toStdString();
    text.weights2 = tr("Gewichtung:").toStdString();
    text.extras = tr("Zusatz-Planeten:").toStdString();
    return build_aspektarium(in, text);
  };
  AspektariumDialog dialog(build(), this);
  mark_output(&dialog, menu_item::kAspektarium);
  // einzel_plan_wahl, muuu& 54
  connect(&dialog, &AspektariumDialog::right_clicked, &dialog, [&]() {
    if (single_planet_choice(3)) {
      dialog.set_sheet(build());
    }
  });
  dialog.exec();
}

// the AAF box on the panel record like CASE 125 of eingabe, all fields
// including the AAF extras. Zurück zum HORCOM-Format takes the edits into
// the panel, Datensatz ÄNDERN leads to OK = Speichern into the AAF file
void MainWindow::edit_record() {
  AafMaskDialog dialog(panel_record(), AafMaskDialog::Mode::kShow, data_dir_, this);
  dialog.set_aaf_file(data_file_.isEmpty() ? aaf_folder(data_file_, data_dir_)
                                           : aaf_twin_path(std::filesystem::path(data_file_.toStdWString())));
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  if (dialog.saved_file()) {
    const std::filesystem::path dat = dat_twin_path(*dialog.saved_file());
    if (std::filesystem::exists(dat)) {
      bind_data_file(QString::fromStdWString(dat.wstring()));
    }
  }
  apply_record(dialog.record());
}

//RR TT MM JJJJ und WENN V.CHR., 'V' EINGEBEN, the text field reads
// both the vC marker and a signed astronomical year
CalendarDate MainWindow::panel_day() const {
  const QString raw = date_->text().trimmed();
  QString head = raw;
  bool bc = false;
  const qsizetype v = raw.indexOf('v', 0, Qt::CaseInsensitive);
  if (v > 0) {
    bc = true;
    head = raw.left(v).trimmed();
  }
  const QStringList f = head.split('.', Qt::SkipEmptyParts);
  if (f.size() != 3) {
    return {};
  }
  int year = f[2].trimmed().toInt();
  if (bc && year > 0) {
    // his V turns the historical year into the astronomical count
    year = 1 - year;
  }
  return {f[0].trimmed().toInt(), f[1].trimmed().toInt(), year, 0.0, 0.0};
}

// a date is real when it survives the round trip through the Julian day
// in its own calendar, the Julian leap day of 1700 included
bool MainWindow::panel_day_valid() const {
  const CalendarDate d = panel_day();
  if (d.day < 1 || d.month < 1 || d.month > 12) {
    return false;
  }
  const CalendarDate back = calendar_date(julian_day(d, panel_calendar_), panel_calendar_);
  return back.day == d.day && back.month == d.month && back.year == d.year;
}

void MainWindow::set_panel_day(int day, int month, int astro_year) {
  const QSignalBlocker block(date_);
  if (astro_year > 0) {
    date_->setText(QString::asprintf("%02d.%02d.%04d", day, month, astro_year));
  } else {
    //RR vC like in his record list
    date_->setText(QString::asprintf("%02d.%02d.%d vC", day, month, 1 - astro_year));
  }
}

QDate MainWindow::panel_qdate() const {
  if (!panel_day_valid()) {
    return QDate::currentDate();
  }
  const CalendarDate g = calendar_date(julian_day(panel_day(), panel_calendar_), Calendar::kAuto);
  return QDate(qdate_year(g.year), g.month, g.day);
}

void MainWindow::set_dst(double hours) {
  dst_hours_ = hours;
  {
    const QSignalBlocker b1(sommerzeit_);
    const QSignalBlocker b2(double_dst_);
    sommerzeit_->setChecked(hours > 0.0);
    double_dst_->setChecked(hours >= 2.0);
    double_dst_->setEnabled(hours > 0.0 && clock_kind_ == ClockKind::kZone);
  }
  if (refresh_sommer_effect_) refresh_sommer_effect_();
}

void MainWindow::set_panel_calendar(Calendar cal) {
  panel_calendar_ = cal;
  const QSignalBlocker b(julian_);
  julian_->setChecked(cal == Calendar::kJulian);
}

// ported from the ORTSZEIT branch of zuo, a historic local clock follows
// the longitude, before 1810 as true local time, until 1890 by answer
void MainWindow::set_local_time(bool on, bool ask) {
  ClockKind kind = ClockKind::kZone;
  if (on) {
    kind = ClockKind::kMeanLocal;
    const LocalTimeRule rule = local_time_rule(panel_day().year);
    if (rule == LocalTimeRule::kTrue) {
      kind = ClockKind::kTrueLocal;
      if (ask) {
        //RR WAHRE Ortszeit wird in MITTLERE Ortszeit umgerechnet !
        QMessageBox::information(this, "HORCOM", tr("WAHRE Ortszeit wird in MITTLERE Ortszeit umgerechnet !"));
      }
    } else if (rule == LocalTimeRule::kAsk && ask) {
      const int es = ChoiceDialog::ask(
          this, tr("ORTSZEIT"),
          {tr("Soll wirklich WAHRE Ortszeit ( = LTT ) eingegeben werden? "),
           tr("Oder soll es sich um MITTLERE Ortszeit ( = LMT ) handeln ?"),
           tr("Meist war zu diesem Datum bereits MITTLERE Ortszeit üblich !")},
          {tr(" WAHRE Ortszeit = LTT = WOZ"), tr(" MITTLERE Ortszeit = LMT = MOZ")}, 1);
      kind = es == 0 ? ClockKind::kTrueLocal : ClockKind::kMeanLocal;
    }
  }
  clock_kind_ = kind;
  {
    const QSignalBlocker b1(local_time_);
    const QSignalBlocker b2(zone_);
    local_time_->setChecked(on);
    //RR zeitzon(ze) = 0, somz(ze) = 0, the local clock knows no zone
    zone_->setEnabled(!on);
    sommerzeit_->setEnabled(!on);
    if (on) {
      zone_->setValue(lon_->value() / kDegPerHour);
    }
  }
  if (on) {
    set_dst(0.0);
  } else {
    set_dst(dst_hours_);
  }
}

// the record carries the person, the panel rules the moment and the
// coordinates, the clock stays civil with the zone beside it
AafRecord MainWindow::panel_record() const {
  AafRecord r = record_;
  // the panel is the source of truth for a save, a mid-typed place is
  // picked up here without needing a commit first
  r.place = place_field_->text().trimmed().toStdString();
  const CalendarDate d = panel_day();
  const QTime t = time_->time();
  r.day = d.day;
  r.month = d.month;
  r.year = d.year;
  r.calendar = panel_calendar_;
  r.hour = t.hour();
  r.minute = t.minute();
  r.second = t.second();
  // a local clock stores its longitude zone, a true local clock keeps
  // the equation of time in the julian date only
  r.zone = aaf_zone(clock_kind_ == ClockKind::kZone ? zone_->value() : lon_->value() / kDegPerHour);
  // korr_sommz, the code his table reads back to the same shift
  if (aaf_dst_hours(r.dst) != dst_hours_ || r.dst.empty()) {
    r.dst = dst_hours_ >= 2.0 ? "2" : (dst_hours_ >= 1.0 ? "1" : (dst_hours_ > 0.0 ? "h" : "0"));
  }
  r.set_latitude(lat_->value());
  r.set_longitude(lon_->value());
  r.jd = julian_day(current_input().date_ut, current_settings().calendar);
  return r;
}

// ported from a22dat and the EINTRAGEN branch of a2fdat. The record
// goes into the bound Daten-Datei, a same named record raises his
// overwrite question, and where the AAF twin exists it is the pilot,
// the record lands there too and the DAT is rebuilt from it
void MainWindow::save_record() {
  store_record(false);
}

// ported from a22dat, the record into the working file. With an AAF twin
// the AAF file is the pilot, the matching AAF record moves to the end and
// the DAT file follows it. His same name rule held only at radix level,
// IF ex! && od = 1, a derived chart on the panel carries the person's
// name with its own moment and joins the file beside the birth record
bool MainWindow::store_record(bool from_entry) {
  if (data_file_.isEmpty()) {
    data_file_io();
    return false;
  }
  const std::filesystem::path dat(data_file_.toStdWString());
  auto records = read_chart_file(dat);
  if (!records) {
    QMessageBox::warning(this, "HORCOM", tr("Die Sammlung ließ sich nicht lesen, nichts geschrieben."));
    return false;
  }
  const AafRecord current = panel_record();
  const ChartRecord entry = dat_from_record(current);
  const QString wanted = QString::fromStdString(entry.name).trimmed();
  // od = 1, a birth chart in its RADIX slot, not a SOLAR row, the COMBIN
  // or a result promoted to a RADIX of its own
  const bool radix = !active_is_solar_ && combin_name1_.empty() &&
                     (active_slot_ < 0 || radix_labels_[static_cast<std::size_t>(active_slot_)].isEmpty());
  // IF TRIM$(UPPER$(naa$(j&))) = TRIM$(UPPER$(et$(1))), only with a date
  bool exists = false;
  if (entry.day > 0 && radix) {
    for (const ChartRecord& r : *records) {
      if (QString::fromStdString(r.name).trimmed().compare(wanted, Qt::CaseInsensitive) == 0) {
        exists = true;
        break;
      }
    }
  }
  const std::filesystem::path twin = aaf_twin_path(dat);
  const bool twin_exists = std::filesystem::exists(twin);
  const QString label = QFileInfo(data_file_).fileName();
  if (exists && twin_exists && from_entry) {
    //RR DATENSATZ GLEICHEN NAMENS bereits VORHANDEN !
    ChoiceDialog::ask(this, "HORCOM",
                      {tr("DATENSATZ GLEICHEN NAMENS bereits VORHANDEN !"), tr("VERÄNDERUNGEN im AAF-FILE vornehmen !")},
                      {tr("EXIT")});
    return false;
  }
  if (exists && !twin_exists) {
    //RR DATENSATZ GLEICHEN NAMENS in der DATEI ÜBERSCHREIBEN ?
    const int es = ChoiceDialog::ask(this, tr("DATEI : %1").arg(label),
                                     {tr("DATENSATZ GLEICHEN NAMENS in der DATEI ÜBERSCHREIBEN ?")},
                                     {tr("ÜBERSCHREIBEN"), tr("Datensatz ZUSÄTZLICH SPEICHERN"), tr("ABBRUCH")});
    if (es < 0 || es == 2) {
      return false;
    }
    if (es == 0) {
      remove_records_by_name(*records, entry.name);
    }
  }
  if (twin_exists) {
    //RR Die parallele HORCOM-Datei GLEICHEN NAMENS dient als Pilot
    auto aaf_records = read_aaf(twin);
    if (!aaf_records) {
      QMessageBox::warning(this, "HORCOM", tr("Speichern fehlgeschlagen."));
      return false;
    }
    if (exists) {
      // aaf_ident, aaf_satz_loesch and aaf_satz_add, the old record
      // gives way to the new one at the end of the file
      if (const auto k = aaf_ident(*aaf_records, entry.name)) {
        aaf_records->erase(aaf_records->begin() + static_cast<std::ptrdiff_t>(*k));
      }
    }
    aaf_records->push_back(current);
    if (!write_aaf(twin, *aaf_records)) {
      QMessageBox::warning(this, "HORCOM", tr("Speichern fehlgeschlagen."));
      return false;
    }
    records->clear();
    records->reserve(aaf_records->size());
    for (const AafRecord& a : *aaf_records) {
      records->push_back(dat_from_record(a));
    }
  } else {
    records->push_back(entry);
  }
  if (!write_chart_file(dat, *records)) {
    QMessageBox::warning(this, "HORCOM", tr("Speichern fehlgeschlagen."));
    return false;
  }
  bind_data_file(data_file_);
  //RR DATEI x : n BYTE = m SÄTZE
  std::error_code ec;
  const auto bytes = std::filesystem::file_size(dat, ec);
  QMessageBox::information(this, "HORCOM",
                           tr("DATEI %1 : %2 BYTE = %3 SÄTZE").arg(label).arg(ec ? 0 : bytes).arg(data_count_));
  //RR NEUE DATENSÄTZE UPDATEN, the record joins the dataset of its file
  stat_update_record(dat, entry);
  return true;
}

// every wheel list passes through here, the screen gets the record
// corners of his HOROSKOP GRAPHIK screen and the centred sheet
void MainWindow::show_full_sheet(DisplayList dl) {
  full_sheet_ = true;
  wheel_->set_plain_list(std::move(dl));
}

void MainWindow::show_wheel(DisplayList dl) {
  full_sheet_ = false;
  ClassicSheetText t = classic_sheet_text();
  if (uhr_slot_ >= 0 && active_slot_ == uhr_slot_ && !active_is_solar_) {
    //RR UHR
    t.name = "UHR";
  }
  add_corner_text(dl, t, 8.0, kScreenSheetWidth / 2.0, kScreenSheetWidth - 8.0);
  wheel_->set_display_list(std::move(dl));
}

// the corner texts of his HOROSKOP GRAPHIK screen, the export sheet
// carries them so the old print comes back out of the new program
// the record corners of the sheet, the panel wheel and the chart only
// preview of the record chooser share them
ClassicSheetText MainWindow::sheet_text_for(const AafRecord& r, const ChartInput& in, const Chart* chart,
                                            const ChartSettings& s, Calendar shown_calendar) const {
  ClassicSheetText t;
  t.name = QString("%1 %2")
               .arg(QString::fromStdString(r.surname), QString::fromStdString(r.given))
               .trimmed()
               .toStdString();
  t.place = r.place;
  if (s.heliocentric) {
    t.mode = tr("Heliozentrisch").toStdString();
  } else {
    //RR Topozentrisch
    t.mode = (s.topocentric_parallax ? tr("Topozentrisch") : tr("Geozentrisch")).toStdString();
  }
  // the corner labels of his sheet, the English build translates them
  t.name_label = tr("Name:").toStdString();
  t.place_label = tr("Ort:").toStdString();
  t.len_header = tr("Länge:").toStdString();
  t.houses_header = tr("Häusersp.").toStdString();
  t.mirror_label = tr("Spiegelung:").toStdString();
  // the headings over the histogram columns of the A4 sheet
  t.quality_heading = tr("Kard-Fix-Ver").toStdString();
  t.element_heading = tr("Elemente").toStdString();
  if (chart != nullptr) {
    const double stz_h = norm_deg(chart->armc_deg) / kDegPerHour;
    const int stz_s = static_cast<int>(stz_h * 3600.0 + 0.5);
    t.stz = (tr("STZ") + QString::asprintf(":%2dh %2dm %2ds", stz_s / 3600, (stz_s / 60) % 60,
                                           stz_s % 60))
                .toStdString();
    // day_w$, the WOCHENTAG of the chart form
    static constexpr const char* kWeekday[7] = {QT_TR_NOOP("Sonntag"),    QT_TR_NOOP("Montag"),
                                                QT_TR_NOOP("Dienstag"),   QT_TR_NOOP("Mittwoch"),
                                                QT_TR_NOOP("Donnerstag"), QT_TR_NOOP("Freitag"),
                                                QT_TR_NOOP("Samstag")};
    const int wd = static_cast<int>(std::fmod(chart->jd_ut + 1.5, 7.0));
    if (wd >= 0 && wd < 7) {
      t.weekday = tr(kWeekday[wd]).toStdString();
    }
  }
  const auto coord = [](const char* tag, double v, char pos, char neg) {
    const char hemi = v < 0.0 ? neg : pos;
    const double a = std::abs(v);
    const int d = static_cast<int>(a);
    return QString::asprintf("%s %d\xC2\xB0 %4.1f'%c", tag, d, (a - d) * 60.0, hemi).toStdString();
  };
  // Lä: and Br:, the place line of the form
  t.lon = coord(tr("Lä:").toUtf8().constData(), in.lon_deg_east, 'E', 'W');
  t.lat = coord(tr("Br:").toUtf8().constData(), in.lat_deg, 'N', 'S');
  // the date line reads in the record's calendar, his jul$ flag beside it
  const CalendarDate shown = calendar_date(julian_day(in.date_ut, s.calendar), shown_calendar);
  QString flag;
  if (shown_calendar == Calendar::kJulian) {
    flag = " (JULIAN.)";
  } else if (shown_calendar == Calendar::kGregorian) {
    flag = " (GREGOR.)";
  }
  t.date = (tr("Datum") + QString::asprintf(":%2d.%2d.%04d", shown.day, shown.month, shown.year) + flag)
               .toStdString();
  int sec = static_cast<int>((in.date_ut.hour * 60.0 + in.date_ut.minute) * 60.0 + 0.5);
  if (sec >= kSecondsPerDay) {
    sec = kSecondsPerDay - 1;
  }
  t.ut = QString::asprintf("UT:%3dh %2dm %2ds", sec / 3600, (sec / 60) % 60, sec % 60).toStdString();
  return t;
}

ClassicSheetText MainWindow::classic_sheet_text() const {
  ClassicSheetText t = sheet_text_for(record_, current_input(), last_chart_ ? &*last_chart_ : nullptr,
                                      current_settings(), panel_calendar_);
  // the paired sheets fill the a13aus corners. Composit reads from the
  // live partner state, combin from its own saved memory so both
  // parents plus the mid moment stay visible
  const QString hor1_name = QString("%1 %2")
                                .arg(QString::fromStdString(record_.surname),
                                     QString::fromStdString(record_.given))
                                .trimmed();
  if (composite_action_ != nullptr && composite_action_->isChecked() && partner_chart_) {
    // IF comp! = 0 && comb! = 0, the composite suppresses the STZ line of
    // the plain sheet. The first chart stands as 1 at the top left, its
    // moment stays in the date box at the lower right, the second joins
    // with name and moment at the lower left
    t.stz.clear();
    t.pair_name1 = pair_name_row(1, hor1_name, tr("Hor 1"));
    t.pair_name2 = pair_name_row(2, partner_name_, tr("Hor 2"));
    t.pair_moment2 = pair_moment_row(2, partner_input_.date_ut);
    // bes2_comp names the house method under COMPOSIT in the wheel
    // centre. a13aus writes a place only under ROBERT HAND, his residence
    // with Lä and Br, the other modes have no place of their own
    const CompositeHouses mode = composite_mode(konsta_, current_settings().houses);
    if (mode == CompositeHouses::kRobertHand) {
      if (comp_residence_) {
        ChartInput at = current_input();
        at.lon_deg_east = comp_residence_->lon;
        at.lat_deg = comp_residence_->lat;
        const ClassicSheetText where = sheet_text_for(record_, at, nullptr, current_settings(), panel_calendar_);
        t.place = comp_residence_->name;
        t.lon = where.lon;
        t.lat = where.lat;
      }
    } else {
      t.place.clear();
      t.lon.clear();
      t.lat.clear();
    }
    t.longitudes_only = true;
    t.h1_axis = mode != CompositeHouses::kSchematic;
  } else if (!combin_name1_.empty()) {
    // comb! suppresses the STZ line as well, a14auscomb puts the mean
    // on the sheet as COMBIN-UT
    t.stz.clear();
    t.pair_name1 = combin_name1_;
    t.pair_moment1 = combin_moment1_;
    t.pair_name2 = combin_name2_;
    t.pair_moment2 = combin_moment2_;
    t.pair_list = combin_list_;
    t.pair_note = combin_note_;
    // go$(0,2) = "COMBIN-ORT", Lä and Br of the mean place
    t.place = "COMBIN-ORT";
  } else if (partner_chart_ && ((compare_action_ != nullptr && compare_action_->isChecked()) ||
                                (dial_action_ != nullptr && dial_action_->isChecked()))) {
    // the double wheel and the 90 degree wheel need the second chart on
    // the sheet, else the reader cannot tell who rides outside. The tester
    // reported it for the Doppel-Kreis in round v5, the STZ and the date
    // box stay with the inner chart
    t.pair_name1 = pair_name_row(1, hor1_name, tr("Hor 1"));
    t.pair_name2 = pair_name_row(2, partner_name_, tr("Hor 2"));
    t.pair_moment2 = pair_moment_row(2, partner_input_.date_ut);
  } else if (!prog_event_note_.empty()) {
    //RR "Ereig: " + d$ und rd$ + ": " + datum3$, proho beside the wheel
    t.pair_moment1 = prog_event_note_;
    t.pair_moment2 = prog_radix_note_;
  }
  return t;
}

// ported from kon_dsp, KONSTA7P.INT follows every change of the Vorgaben
// and the end of the session. The panel switches and the orb block flow
// back into the file fields first so the file holds what the program ran
void MainWindow::persist_konsta() {
  const ChartSettings s = current_settings();
  const int haw = static_cast<int>(s.houses);
  if (haw != konsta_.haw) {
    konsta_.haw = haw;
    konsta_.haus = std::string(compute_houses(s.houses, 0.0, 0.0, 0.0).name);
  }
  konsta_.par = s.topocentric_parallax ? 1.0 : 2.0;
  konsta_.apogw = s.true_apogee;
  konsta_.moknw = s.true_node;
  konsta_.orb = aspect_settings_.orb;
  konsta_.nasp = aspect_settings_.divisors;
  konsta_.orbe_on = aspect_settings_.equal_probability;
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    konsta_.or_weight[static_cast<std::size_t>(slot)] = aspect_settings_.weight[static_cast<std::size_t>(slot)];
  }
  // orb$(1..14), his self defined orbs in degrees like STR$(x,5,2)
  for (int i = 1; i <= 14; ++i) {
    const double deg = aspect_settings_.orbe[static_cast<std::size_t>(i)] * kRadToDeg;
    if (deg > 0.0) {
      konsta_.orb_text[static_cast<std::size_t>(i)] = QString::asprintf("%5.2f", deg).toStdString();
    }
  }
  //RR hard&, the colour of the outer symbols
  konsta_.hard = outer_color_;
  konsta_.fixpunkt = fixpunkt_ >= 0.0 ? 1 : 2;
  konsta_.fixpunkt_name = fixpunkt_ >= 0.0 ? std::to_string(fixpunkt_ * kRadToDeg) : std::string();
  if (konsta_file_.empty()) {
    return;
  }
  if (!save_konsta(konsta_file_, konsta_)) {
    QMessageBox::warning(this, "HORCOM", tr("Die Vorgaben ließen sich nicht speichern."));
  }
}

void MainWindow::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  // the dock layout of the show settles one turn later
  schedule_dock_fit();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
  QMainWindow::resizeEvent(event);
  // a larger or smaller window hands its docks a share of the change
  if (isVisible()) {
    schedule_dock_fit();
  }
}

void MainWindow::schedule_dock_fit() {
  if (dock_fit_ != nullptr) {
    dock_fit_->start();
  }
}

void MainWindow::fit_body_dock() {
  if (!isVisible() || body_dock_ == nullptr || body_dock_->isFloating() || !body_dock_->isVisible()) {
    return;
  }
  // the dock around the cells, the row header, the frames and the scroll
  // bar as the layout laid them out
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  const int chrome = body_dock_->width() - bodies_->viewport()->width();
  // a column needs its cells and its head, the stretched last one too
  const auto* view = static_cast<const QAbstractItemView*>(bodies_);
  const auto need = [this, view](int c) {
    return c + 1 < bodies_->columnCount() ? bodies_->columnWidth(c)
                                          : std::max(view->sizeHintForColumn(c), bodies_->horizontalHeader()->sectionSizeHint(c));
  };
  const auto whole_columns = [this, &need](int room) {
    int cells = 0;
    for (int c = 0; c < bodies_->columnCount() && cells + need(c) <= room; ++c) {
      cells += need(c);
    }
    return cells;
  };
  // on wide windows the wheel keeps the surplus, the panel stays near a
  // third and the remaining columns scroll
  const int room = std::max(460, width() * 36 / 100) - chrome;
  const int cells = whole_columns(room);
  if (cells == bodies_->viewport()->width()) {
    return;
  }
  resizeDocks({body_dock_}, {chrome + cells}, Qt::Horizontal);
  // a narrow window gives less than asked where the wheel keeps its
  // minimum, the panel then closes on the last column that fits
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
  const int given = bodies_->viewport()->width();
  if (given < cells) {
    resizeDocks({body_dock_}, {chrome + whole_columns(given)}, Qt::Horizontal);
  }
}

void MainWindow::closeEvent(QCloseEvent* event) {
  // every way out asks like his MENU(1) = 4 of the close box, the close
  // box, the quit key and a quit of the system alike. DESKTOP ( QUIT
  // HORCOM ) asked already
  if (!quit_confirmed_ && !confirm_quit()) {
    event->ignore();
    return;
  }
  quit_confirmed_ = true;
  // his @kon_dsp before the END of the main loop
  if (!keep_konsta_file_) {
    persist_konsta();
  }
  QMainWindow::closeEvent(event);
}

// a rewrite addition. His AUFRÄUMEN / RÜCKSETZEN clears the session
// alone, this entry also returns the view settings of the ANSICHT menu and
// the VORGABEN of his profile, then starts the program anew so the window,
// the panel and the docks stand as at the first start. The language stays
// a choice of its own
void MainWindow::full_reset() {
  const int answer = ChoiceDialog::ask(
      this, tr("ALLES ZURÜCKSETZEN"),
      {tr("HORCOM auf den ZUSTAND des ERSTEN STARTS zurücksetzen ?"), QString(),
       tr("Schrift, Farben, Fenster und die VORGABEN nach Robert Rettig kehren zurück,"),
       tr("die Daten dieser Sitzung gehen verloren."),
       tr("Datensätze, Orte und eigene Dateien bleiben erhalten.")},
      {tr("ZURÜCKSETZEN und NEU STARTEN"), tr("ABBRUCH")}, 1);
  if (answer != 0) {
    return;
  }
  if (!konsta_file_.empty() && !save_konsta(konsta_file_, robert_profile())) {
    QMessageBox::warning(this, "HORCOM", tr("Die Vorgaben ließen sich nicht zurücksetzen."));
    return;
  }
  QSettings().remove(theme::kViewGroup);
  keep_konsta_file_ = true;
  quit_confirmed_ = true;
  restart_();
}

// ported from the MESSAGE of the main loop
bool MainWindow::confirm_quit() {
  // his MESSAGE "PROGRAMM   HORCOM7P   BEENDEN ?","ABBRUCH?",MB_YESNO | MB_ICONHAND
  return QMessageBox::question(this, tr("ABBRUCH?"), tr("PROGRAMM   HORCOM   BEENDEN ?"),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

// ported from desct with the muuu& = 99 branch of the main loop
void MainWindow::desktop_quit() {
  if (!confirm_quit()) {
    return;
  }
  quit_confirmed_ = true;
  close();
}

// ported from color_dial. His DLG COLOR boxes chose the background of the
// dialogs and of the passive screen, param_sp kept both in KONSTA. The
// dresses of the Ansicht menu own these colours until this entry is used,
// then HINTERGRUND-FARBEN stands checked in its FARBEN menu
void MainWindow::background_colors() {
  const QColor dial_now = theme::dialog_color(konsta_.col_dial);
  const QColor back_now = theme::passive_color(konsta_.col_backg);
  //RR " Hintergrund-FARBE bzw. MUSTER der DIALOGE ( ohne Edit-Felder ) DEFINIEREN "
  const QColor dial =
      QColorDialog::getColor(dial_now, this, tr(" Hintergrund-FARBE bzw. MUSTER der DIALOGE ( ohne Edit-Felder ) DEFINIEREN "));
  //RR " Hintergrund-FARBE für den 'PASSIVEN' HORCOM - BILDSCHIRM WÄHLEN ! "
  const QColor back =
      QColorDialog::getColor(back_now, this, tr(" Hintergrund-FARBE für den 'PASSIVEN' HORCOM - BILDSCHIRM WÄHLEN ! "));
  if (!dial.isValid() && !back.isValid()) {
    return;
  }
  konsta_.col_dial = theme::to_colorref(dial.isValid() ? dial : dial_now);
  konsta_.col_backg = theme::to_colorref(back.isValid() ? back : back_now);
  QSettings().setValue(theme::kOwnColorsKey, true);
  theme::set_own_colors(theme::from_colorref(konsta_.col_dial), theme::from_colorref(konsta_.col_backg));
  if (auto* own = findChild<QAction*>(kOwnColorsAction)) {
    own->setChecked(true);
  }
  wheel_->update();
  banner_->update();
  // his @param_sp
  persist_konsta();
}

// ported from CASE 107 of ausw_datei, the chart of the marked record in
// its own window, Space or Enter lead back to the list, F6 turns helio
void MainWindow::preview_record(const AafRecord& r) {
  QDialog box(this);
  //RR " Nur HOROSKOP Ansehen | Weiter mit LEERTASTE | BEENDEN mit 'EXIT' "
  box.setWindowTitle(tr(" Nur HOROSKOP Ansehen | Weiter mit LEERTASTE | BEENDEN mit 'EXIT' "));
  auto* v = new QVBoxLayout(&box);
  v->setContentsMargins(0, 0, 0, 0);
  auto* wheel = new WheelWidget(&box);
  v->addWidget(wheel);
  bool helio = current_settings().heliocentric;
  const auto draw = [&]() {
    ChartSettings s = current_settings();
    s.heliocentric = helio;
    if (auto dl = sheet_wheel(r, record_input(r), s)) {
      wheel->set_display_list(std::move(*dl));
    }
  };
  draw();
  // his KEYGET loop, the leading keys arrive before the wheel sees them
  LambdaFilter keys(LambdaFilter::keys([&](int key) {
    if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter) {
      box.accept();
      return true;
    }
    if (key == Qt::Key_F6) {
      helio = !helio;
      draw();
      return true;
    }
    return false;
  }));
  box.installEventFilter(&keys);
  wheel->installEventFilter(&keys);
  box.resize(760, 760);
  box.exec();
}

// the classic sheet of a record at a given moment and place, the wheel
// of the preview and of the walks
std::optional<DisplayList> MainWindow::sheet_wheel(const AafRecord& r, const ChartInput& in, const ChartSettings& s,
                                                  const std::string& centre) const {
  const Chart c = compute_chart(in, s, vsop_, eph_);
  if (!c.ok) {
    return std::nullopt;
  }
  const AspectResult a = scan_aspects(c, s, shown_aspect_settings());
  WheelOptions opt = radix_wheel_options(c, s);
  opt.center_label = centre;
  if (!centre.empty()) {
    opt.chart_label.clear();
    opt.chart_sub_label.clear();
  }
  DisplayList dl = build_wheel(c, s, a, opt);
  add_corner_text(dl, sheet_text_for(r, in, &c, s, r.calendar), 8.0, kScreenSheetWidth / 2.0, kScreenSheetWidth - 8.0);
  return dl;
}

// the export drawing, the classic sheet of the original with the body
// table down the left margin around whatever the wheel currently shows
DisplayList MainWindow::classic_export_list() const {
  DisplayList dl = wheel_->display_list();
  // his full sheets already carry their tables and corners
  if (last_chart_ && !full_sheet_) {
    add_classic_text(dl, *last_chart_, current_settings(), classic_sheet_text(),
                     last_aspects_ ? last_aspects_->mirrors : std::vector<std::pair<int, int>>{});
  }
  return dl;
}

void MainWindow::export_svg() {
  if (!last_chart_ || !last_aspects_) {
    return;
  }
  const QString path = QFileDialog::getSaveFileName(this, tr("Horoskop als SVG"), "wheel.svg", tr("SVG (*.svg)"));
  if (path.isEmpty()) {
    return;
  }
  export_svg_to(path);
}

bool MainWindow::export_svg_to(const QString& path) {
  const std::string svg = to_svg(classic_export_list(), svg_sprite_resolver());
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly)) {
    return false;
  }
  f.write(svg.data(), static_cast<qint64>(svg.size()));
  return true;
}

// builds the a11 A4 page over the current chart, the bes_big tables
// and the wheel, empty when no chart is up
DisplayList MainWindow::a4_export_list() const {
  if (!last_chart_ || !last_aspects_) {
    return {};
  }
  const ChartSettings s = current_settings();
  const MidpointResult mids = scan_midpoints(*last_chart_, s, shown_aspect_settings());
  const auto mode = static_cast<HistogramMode>(std::clamp(konsta_.elem, 1, 3));
  return a4_print_sheet(*last_chart_, s, *last_aspects_, mids, classic_sheet_text(),
                        radix_wheel_options(*last_chart_, s), sheet_histogram(*last_chart_, s), mode);
}

// the DRUCKER-GRAPHIK rows of druck_graph_ein without its BILDSCHIRM row
int MainWindow::ask_print_format() {
  return ChoiceDialog::ask(this, tr("AUSGABE als DRUCKER-GRAPHIK ?"), {},
                           {tr("DRUCKER-GRAPHIK  DIN A5 ?"), tr("DRUCKER-GRAPHIK  DIN A4 ?"),
                            tr("ABBRUCH")});
}

bool MainWindow::export_pdf_to(const QString& path, bool big) {
  const DisplayList dl = big ? a4_export_list() : classic_export_list();
  if (dl.items.empty()) {
    return false;
  }
  QPdfWriter writer(path);
  writer.setPageSize(QPageSize(QPageSize::A4));
  // DIN A4 stands upright, the A5 wheel canvas lies landscape
  writer.setPageOrientation(big ? QPageLayout::Portrait : QPageLayout::Landscape);
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
  const int format = ask_print_format();
  if (format != 0 && format != 1) {
    return;
  }
  const QString path = QFileDialog::getSaveFileName(this, tr("Horoskop als PDF"), "horoskop.pdf", tr("PDF (*.pdf)"));
  if (path.isEmpty()) {
    return;
  }
  if (!export_pdf_to(path, format == 1)) {
    QMessageBox::warning(this, "HORCOM", tr("Die PDF-Datei ließ sich nicht schreiben."));
  }
}

// DRUCKEN, his DRUCKER-GRAPHIK of the chart on the wheel through the
// same setup boxes, paper and page geometry as the a11 branches
void MainWindow::print_chart() {
  const int format = ask_print_format();
  if (format != 0 && format != 1) {
    return;
  }
  const bool a4 = format == 1;
  (void)print_graphic(a4 ? a4_export_list() : classic_export_list(), a4, page_margin::kLeftChart);
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
