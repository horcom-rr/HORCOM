// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "aaf_mask_dialog.hpp"

#include <QFileDialog>
#include <QFileInfo>
#include <QFocusEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath>
#include <system_error>

#include "auto_advance.hpp"
#include "choice_dialog.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/collection.hpp"
#include "horcom/data/countries.hpp"
#include "horcom/data/place_file.hpp"
#include "horcom/time/local_time.hpp"
#include "kommen_dialog.hpp"
#include "place_hub.hpp"
#include "robert_input.hpp"
#include "theme.hpp"
#include "zeitzon_dialog.hpp"
#include "zone_dialog.hpp"

namespace horcom {

namespace {

// his test day around the calendar reform of October 1582, jd > 2299151
// makes a forced Gregorian date one the reform covers anyway
constexpr double kReformCheckJd = 2299151.0;

// the listbox of hor_art, laender and sommerzeit. The rows before first
// stand as captions, his select& > 1 left the header row out
std::optional<int> pick_row(QWidget* parent, const QString& title, const QStringList& rows, int first) {
  QDialog d(parent);
  d.setWindowTitle(title);
  auto* v = new QVBoxLayout(&d);
  auto* list = new QListWidget(&d);
  list->setFont(theme::mono_font(10));
  for (int i = 0; i < rows.size(); ++i) {
    auto* item = new QListWidgetItem(" " + rows[i], list);
    if (i < first) {
      item->setFlags(Qt::NoItemFlags);
    }
  }
  list->setCurrentRow(first);
  v->addWidget(list, 1);
  auto* buttons = new QHBoxLayout();
  auto* ok = new QPushButton(AafMaskDialog::tr("OK"), &d);
  ok->setDefault(true);
  auto* cancel = new QPushButton(AafMaskDialog::tr("ABBRUCH"), &d);
  buttons->addWidget(ok);
  buttons->addStretch(1);
  buttons->addWidget(cancel);
  v->addLayout(buttons);
  QObject::connect(ok, &QPushButton::clicked, &d, &QDialog::accept);
  QObject::connect(cancel, &QPushButton::clicked, &d, &QDialog::reject);
  QObject::connect(list, &QListWidget::itemDoubleClicked, &d, &QDialog::accept);
  d.resize(440, 360);
  if (d.exec() != QDialog::Accepted || list->currentRow() < first) {
    return std::nullopt;
  }
  return list->currentRow();
}

bool entered(QEvent* event) {
  const Qt::FocusReason reason = static_cast<QFocusEvent*>(event)->reason();
  return reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason || reason == Qt::MouseFocusReason;
}

double field_number(const QLineEdit* e) {
  //RR Eingabe mit Komma statt Punkt
  QString t = e->text().trimmed();
  t.replace(',', '.');
  return QLocale::c().toDouble(t);
}

}  // namespace

AafMaskDialog::AafMaskDialog(AafRecord record, std::filesystem::path data_dir, QWidget* parent)
    : AafMaskDialog(std::move(record), Mode::kEdit, std::move(data_dir), parent) {}

// ported from aaf_box
AafMaskDialog::AafMaskDialog(AafRecord record, Mode mode, std::filesystem::path data_dir, QWidget* parent)
    : QDialog(parent), base_(std::move(record)), data_dir_(std::move(data_dir)), kommen_(data_dir_ / "kommen") {
  //RR AAF - Eingabe - und Anzeige - Box | Erläuterung mit F1
  setWindowTitle(tr("AAF- Eingabe- und Anzeige- Box   (Erläuterung mit F1)"));
  auto* v = new QVBoxLayout(this);
  // his ERASE ed$() before a NEU-EINGABE, a fresh record shows empty
  // fields and only the N and E his loop filled in
  const bool fresh = base_.day == 0 && base_.month == 0 && base_.year == 0 && base_.jd <= 0.0;
  const auto num = [fresh](int value) { return fresh ? QString() : QString::number(value); };

  auto* a = new QGroupBox(tr("Standarddaten AAF-A"), this);
  auto* ag = new QGridLayout(a);
  ag->addWidget(new QLabel(tr("Nachname"), a), 0, 0);
  surname_ = new QLineEdit(QString::fromStdString(base_.surname), a);
  ag->addWidget(surname_, 0, 1, 1, 3);
  ag->addWidget(new QLabel(tr("Vorname"), a), 0, 4);
  given_ = new QLineEdit(QString::fromStdString(base_.given), a);
  ag->addWidget(given_, 0, 5, 1, 3);
  ag->addWidget(new QLabel(tr("Horoskopart / Geschlecht"), a), 1, 0);
  sex_ = number_box(a, QString::fromStdString(base_.sex), 2, 0);
  ag->addWidget(sex_, 1, 1);
  ag->addWidget(new QLabel(tr("Datum  TT MM JJJJ"), a), 2, 0);
  auto* datum = new QHBoxLayout();
  day_ = number_box(a, num(base_.day), 2, 31);
  month_ = number_box(a, num(base_.month), 2, 12);
  // ed$(5), the astronomical year with the g or j that forces the
  // Gregorian or Julian calendar, a free text field
  QString year_text = num(base_.year);
  if (!fresh && base_.calendar == Calendar::kGregorian) {
    year_text += "g";
  } else if (!fresh && base_.calendar == Calendar::kJulian) {
    year_text += "j";
  }
  year_ = number_box(a, year_text, 6, 0);
  datum->addWidget(day_);
  datum->addWidget(month_);
  datum->addWidget(year_);
  // his kal$ box beside the year
  calendar_note_ = new QLabel(a);
  datum->addWidget(calendar_note_);
  datum->addStretch(1);
  ag->addLayout(datum, 2, 1, 1, 7);
  ag->addWidget(new QLabel(tr("Zeit  hh mm ss"), a), 3, 0);
  auto* zeit = new QHBoxLayout();
  hour_ = number_box(a, num(base_.hour), 2, 23);
  minute_ = number_box(a, num(base_.minute), 2, 59);
  second_ = number_box(a, num(base_.second), 2, 59);
  zeit->addWidget(hour_);
  zeit->addWidget(minute_);
  zeit->addWidget(second_);
  zeit->addStretch(1);
  ag->addLayout(zeit, 3, 1, 1, 7);
  ag->addWidget(new QLabel(tr("Ortsname"), a), 4, 0);
  place_ = new QLineEdit(QString::fromStdString(base_.place), a);
  ag->addWidget(place_, 4, 1, 1, 5);
  ag->addWidget(new QLabel(tr("Land z.B. 'D'"), a), 4, 6);
  country_ = number_box(a, QString::fromStdString(base_.country), 4, 0);
  ag->addWidget(country_, 4, 7);
  v->addWidget(a);

  auto* b = new QGroupBox(tr("Standarddaten AAF-B"), this);
  auto* bg = new QGridLayout(b);
  // Juldatum, a typed value takes priority over date and time
  bg->addWidget(new QLabel(tr("Juldatum"), b), 0, 0);
  jd_seed_ = base_.jd;
  jd_ = new QLineEdit(base_.jd > 0.0 ? QString::number(base_.jd, 'f', 5) : QString(), b);
  jd_->setFixedWidth(160);
  bg->addWidget(jd_, 0, 1);
  bg->addWidget(new QLabel(tr("Breite  N/S grd min sek"), b), 1, 0);
  auto* breite = new QHBoxLayout();
  const bool placed = !fresh || base_.latitude() != 0.0 || base_.longitude() != 0.0;
  const auto coord = [placed](int value) { return placed ? QString::number(value) : QString(); };
  lat_ns_ = number_box(b, QChar(base_.lat_ns), 1, 0);
  lat_deg_ = number_box(b, coord(base_.lat_deg), 2, 89);
  lat_min_ = number_box(b, coord(base_.lat_min), 2, 59);
  lat_sec_ = number_box(b, coord(base_.lat_sec), 2, 59);
  for (QLineEdit* e : {lat_ns_, lat_deg_, lat_min_, lat_sec_}) {
    breite->addWidget(e);
  }
  breite->addStretch(1);
  bg->addLayout(breite, 1, 1);
  bg->addWidget(new QLabel(tr("Länge  E/W grd min sek"), b), 2, 0);
  auto* laenge = new QHBoxLayout();
  lon_ew_ = number_box(b, QChar(base_.lon_ew), 1, 0);
  lon_deg_ = number_box(b, coord(base_.lon_deg), 3, 180);
  lon_min_ = number_box(b, coord(base_.lon_min), 2, 59);
  lon_sec_ = number_box(b, coord(base_.lon_sec), 2, 59);
  for (QLineEdit* e : {lon_ew_, lon_deg_, lon_min_, lon_sec_}) {
    laenge->addWidget(e);
  }
  laenge->addStretch(1);
  bg->addLayout(laenge, 2, 1);
  bg->addWidget(new QLabel(tr("Zone (ZZD)"), b), 3, 0);
  auto* zonrow = new QHBoxLayout();
  // the AAF zone string, like 01hE00:00, east leads with its letter
  zone_ = new QLineEdit(QString::fromStdString(base_.zone), b);
  zone_->setFixedWidth(140);
  zonrow->addWidget(zone_);
  zonrow->addWidget(new QLabel(tr("Sommerzeit"), b));
  dst_ = number_box(b, QString::fromStdString(base_.dst), 1, 0);
  zonrow->addWidget(dst_);
  zonrow->addStretch(1);
  bg->addLayout(zonrow, 3, 1);
  v->addWidget(b);

  auto* c = new QGroupBox(tr("Zusatzdaten AAF-C"), this);
  auto* cg = new QGridLayout(c);
  cg->addWidget(new QLabel(tr("COM (alle Zeichen außer '#')"), c), 0, 0, 1, 2);
  com_ = new QPlainTextEdit(QString::fromStdString(base_.comment), c);
  com_->setFixedHeight(60);
  cg->addWidget(com_, 1, 0, 1, 2);
  const auto add_line = [&](int row, const QString& label, const std::string& value) {
    cg->addWidget(new QLabel(label, c), row, 0);
    auto* e = new QLineEdit(QString::fromStdString(value), c);
    cg->addWidget(e, row, 1);
    return e;
  };
  via_ = add_line(2, "VIA", base_.via);
  src_ = add_line(3, "SRC", base_.source);
  gzq_ = add_line(4, "GZQ", base_.quality);
  znam_ = add_line(5, "ZNAM", base_.zone_name);
  cword_ = add_line(6, "CWORD", base_.catchword);
  attrb_ = add_line(7, "ATTRB", base_.attributes);
  v->addWidget(c);

  // the button row of his box, 167 AAF-Help, 172 Datensatz ÄNDERN, 168
  // or 171 in the middle and 169 EXIT, 173 below
  auto* row = new QHBoxLayout();
  auto* help = new QPushButton(tr("AAF-Help"), this);
  change_ = new QPushButton(tr("Datensatz ÄNDERN"), this);
  save_ = new QPushButton(tr("OK = Speichern"), this);
  back_ = new QPushButton(tr("Zurück zum HORCOM-Format"), this);
  auto* exit = new QPushButton(tr("EXIT"), this);
  row->addWidget(help);
  row->addWidget(change_);
  row->addStretch(1);
  row->addWidget(save_);
  row->addWidget(back_);
  row->addStretch(1);
  row->addWidget(exit);
  v->addLayout(row);
  auto* row2 = new QHBoxLayout();
  auto* texts = new QPushButton(tr("ZEITBESTIMMUNGEN lesen"), this);
  row2->addWidget(texts);
  row2->addStretch(1);
  v->addLayout(row2);
  for (QPushButton* p : {help, change_, back_, exit, texts}) {
    p->setAutoDefault(false);
  }
  connect(help, &QPushButton::clicked, this, [this]() {
    KommenDialog dialog(kommen_, "aaf_komm", english_edition(), this);
    dialog.exec();
  });
  connect(save_, &QPushButton::clicked, this, &AafMaskDialog::save);
  connect(back_, &QPushButton::clicked, this, &QDialog::accept);
  connect(exit, &QPushButton::clicked, this, &QDialog::reject);
  connect(texts, &QPushButton::clicked, this, [this]() { read_time_determinations(this, data_dir_); });
  // CASE 172, the record opens for changes, the julian date, the zone,
  // the summer time and the zone name are entered afresh
  connect(change_, &QPushButton::clicked, this, [this]() {
    for (QLineEdit* e : {jd_, zone_, dst_, znam_}) {
      e->clear();
    }
    jd_seed_ = 0.0;
    jd_open_ = false;
    set_mode(Mode::kEdit);
    surname_->setFocus();
  });
  connect(year_, &QLineEdit::textChanged, this, [this](const QString&) { show_calendar_note(); });
  // ed$(12) = UPPER$(ed$(12)), ed$(16) = UPPER$(ed$(16))
  for (QLineEdit* e : {lat_ns_, lon_ew_}) {
    connect(e, &QLineEdit::textEdited, this, [e](const QString& t) { e->setText(t.toUpper()); });
  }
  for (QLineEdit* e : {place_, zone_, sex_, country_, dst_, jd_, year_, lat_ns_, lon_ew_}) {
    e->installEventFilter(this);
  }
  // gettextaaf advances only behind the name fields, his dial_i& > 108
  chain_fields({{day_, 2}, {month_, 2}, {year_, 4}, {hour_, 2}, {minute_, 2}, {second_, 2}, {place_, 0}});
  chain_fields({{lat_ns_, 1}, {lat_deg_, 2}, {lat_min_, 2}, {lat_sec_, 2}, {lon_ew_, 1}, {lon_deg_, 3},
                {lon_min_, 2}, {lon_sec_, 2}, {zone_, 0}});
  show_calendar_note();
  set_mode(mode);
  resize(900, 0);
  surname_->setFocus();
}

void AafMaskDialog::set_aaf_file(std::filesystem::path aaf) {
  aaf_file_ = std::move(aaf);
}

// IF aafein! = -1 OR aafaend! = -1 shows OK = Speichern, else the two
// buttons of the display, the julian date stays grey there
void AafMaskDialog::set_mode(Mode mode) {
  mode_ = mode;
  const bool show = mode == Mode::kShow;
  save_->setVisible(!show);
  back_->setVisible(show);
  change_->setVisible(show);
  jd_->setEnabled(!show);
  save_->setDefault(!show);
  back_->setDefault(show);
}

// his MENU(6) loop, entering a field by key or mouse opens its box. The
// focus that comes back after a box stays quiet
bool AafMaskDialog::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::FocusIn && entered(event) && !data_dir_.empty()) {
    if (watched == place_) {
      QTimer::singleShot(0, this, &AafMaskDialog::place_menu);
    } else if (watched == zone_) {
      if (preferred_new_) {
        // IF vorz_neu!, @ortp(gl,gg,go$)
        preferred_new_ = false;
        const AafRecord r = record();
        write_preferred_place(data_dir_ / "ort.ext",
                              {r.longitude(), r.latitude(), QString::fromStdString(r.place).toUpper().toStdString()});
      }
      QTimer::singleShot(0, this, &AafMaskDialog::zone_menu);
    } else if (watched == sex_) {
      QTimer::singleShot(0, this, &AafMaskDialog::kind_menu);
    } else if (watched == country_) {
      QTimer::singleShot(0, this, &AafMaskDialog::country_menu);
    } else if (watched == dst_) {
      QTimer::singleShot(0, this, &AafMaskDialog::summer_menu);
    }
  }
  if (event->type() == QEvent::FocusIn && entered(event) && watched == jd_) {
    QTimer::singleShot(0, this, &AafMaskDialog::jd_priority);
  }
  if (event->type() == QEvent::FocusOut && entered(event)) {
    if (watched == jd_) {
      QTimer::singleShot(0, this, &AafMaskDialog::jd_taken);
    } else if (watched == year_) {
      QTimer::singleShot(0, this, &AafMaskDialog::year_check);
    } else if (watched == lat_ns_) {
      QTimer::singleShot(0, this, [this]() { letter_check(lat_ns_, 'N', 'S'); });
    } else if (watched == lon_ew_) {
      QTimer::singleShot(0, this, [this]() { letter_check(lon_ew_, 'E', 'W'); });
    }
  }
  return QDialog::eventFilter(watched, event);
}

// his ort_loe, the place and its coordinates emptied
void AafMaskDialog::clear_place() {
  for (QLineEdit* e : {place_, country_, lat_ns_, lat_deg_, lat_min_, lat_sec_, lon_ew_, lon_deg_, lon_min_, lon_sec_}) {
    e->clear();
  }
}

// ported from aaf_orteingabe, the EREIGNIS-ORT menu of the AAF box
void AafMaskDialog::place_menu() {
  const std::filesystem::path ext = data_dir_ / "ort.ext";
  const auto take = [this](const QString& name, double lon, double lat) {
    // make_ort_horc_aaf
    place_->setText(name.toUpper());
    const auto dms = [](double value, QLineEdit* d, QLineEdit* m, QLineEdit* s) {
      const Dms x = split_dms(value);
      d->setText(QString::number(x.deg));
      m->setText(QString::number(x.min));
      s->setText(QString::number(x.sec));
    };
    lat_ns_->setText(lat < 0.0 ? "S" : "N");
    dms(lat, lat_deg_, lat_min_, lat_sec_);
    lon_ew_->setText(lon < 0.0 ? "W" : "E");
    dms(lon, lon_deg_, lon_min_, lon_sec_);
  };
  for (;;) {
    // vv$ = "VORZUGSORT ", EREIGNIS-ORT EINGEBEN !
    const int es = ChoiceDialog::ask(this, tr("EREIGNIS-ORT EINGEBEN !"), {},
                                     {tr(" NEUER ORT oder ORT EDITIEREN ( auch Länge u. Breite ! ) "), tr(" VORZUGSORT "),
                                      tr("ORTS-DATEIEN  HOLEN"), tr("VORZUGSORT LÖSCHEN"), tr("UNDO = NICHT ÄNDERN ! ")});
    switch (es) {
      case 0:
        clear_place();
        place_->setFocus();
        return;
      case 1: {
        if (const auto home = read_preferred_place(ext)) {
          take(QString::fromStdString(home->name), home->lon, home->lat);
          return;
        }
        //RR Vorzugsort NOCH LEER ! HIER als 'NEUER ORT' eingeben !
        const int k = ChoiceDialog::ask(this, "HORCOM", {tr("Vorzugsort NOCH LEER ! "), tr("HIER als 'NEUER ORT' eingeben !")},
                                        {tr("OK"), tr("LEER LASSEN")});
        preferred_new_ = k == 0;
        continue;
      }
      case 2:
        clear_place();
        if (const auto got = place_file_hub(this, data_dir_)) {
          take(QString::fromStdString(got->name), got->lon, got->lat);
        }
        return;
      case 3: {
        //RR Vorzugsort WIRKLICH LÖSCHEN ?
        const int h = ChoiceDialog::ask(this, "HORCOM", {tr("Vorzugsort WIRKLICH LÖSCHEN ? ")},
                                        {tr("LÖSCHEN"), tr("NICHT Löschen")});
        if (h == 0) {
          std::error_code ec;
          std::filesystem::remove(ext, ec);
        }
        continue;
      }
      default:
        return;
    }
  }
}

// ported from hor_art, ed$(2) = LEFT$(hoa$(select& - 1),2)
void AafMaskDialog::kind_menu() {
  const QStringList rows = {tr("Für Radix-Datensätze :"),  tr("F od. W = Weiblich"), tr("M       = Männlich"),
                            tr("E       = Ereignis"),      tr("L       = Land"),
                            tr("O       = Organisation / Firma"), tr("*       = Ohne Belang")};
  if (const auto row = pick_row(this, tr("Auswahl Horoskop-Art"), rows, 1)) {
    sex_->setText(rows[*row].left(2).trimmed());
    day_->setFocus();
  }
}

// ported from laender, ed$(10) = LEFT$(land$(select& - 1),3)
void AafMaskDialog::country_menu() {
  const auto countries = load_german_countries(data_dir_ / "laender.int");
  if (!countries || countries->empty()) {
    return;
  }
  QStringList rows;
  for (const GermanCountry& g : *countries) {
    rows << QString("%1 %2").arg(QString::fromStdString(g.abbrev), -3).arg(QString::fromStdString(g.name));
  }
  if (const auto row = pick_row(this, tr("Auswahl Länder-Kürzel"), rows, 0)) {
    country_->setText(QString::fromStdString((*countries)[static_cast<std::size_t>(*row)].abbrev));
  }
}

// ported from sommerzeit, ed$(21) = LEFT$(soz$(select&),1), then his
// hor_add with the summer shift renews the julian date
void AafMaskDialog::summer_menu() {
  const QStringList rows = {tr("0 = Standardzeit"),           tr("1 = Einfache Sommerzeit"),
                            tr("2 = Doppelte Sommerzeit"),    tr("w = Kriegszeiten  = '1'"),
                            tr("h = Halbe    Sommerzeit"),    tr("m = Bestimmter Zeitmeridian"),
                            tr("L = Ortszeit"),               tr("* = Ortszeit oder ohne Belang")};
  const auto row = pick_row(this, tr("Auswahl Sommerzeiten"), rows, 0);
  if (!row) {
    return;
  }
  dst_->setText(rows[*row].left(1));
  if (mode_ != Mode::kShow && !jd_open_ && !zone_->text().trimmed().isEmpty()) {
    AafRecord r = record();
    r.jd = 0.0;
    jd_seed_ = aaf_moment_jd_ut(r);
    jd_->setText(QString::number(jd_seed_, 'f', 5));
  }
}

// ported from zeitzon_nam_aaf. LMT follows from the longitude alone, the
// equation of time is part of it. LTT converts before 1810 on its own and
// asks until 1890 like zuo, a zone row takes its difference and its name
void AafMaskDialog::zone_menu() {
  ZoneDialog dialog(data_dir_ / "zonnamen.int", this);
  if (!dialog.loaded()) {
    //RR ZEITZONEN-Datei fehlt !
    QMessageBox::warning(this, "HORCOM", tr("ZEITZONEN-Datei fehlt !"));
    return;
  }
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const ZoneEntry& z = dialog.chosen();
  if (z.is_local_time()) {
    const AafRecord r = record();
    if (r.lon_deg == 0 && r.lon_min == 0 && r.lon_sec == 0) {
      //RR Geogr. Länge ist = 0 ! FEHLT eventuell ?
      QMessageBox::information(this, "HORCOM", tr("Geogr. Länge ist = 0 ! FEHLT eventuell ?"));
      zone_->clear();
      lon_deg_->setFocus();
      return;
    }
    // zzd = -gl * 24 / 360 leads to UT, the panel wants hours east
    double east = r.longitude() / kDegPerHour;
    QString name = QStringLiteral("LMT");
    if (z.abbrev == "LTT") {
      const CalendarDate local{r.day, r.month, r.year, static_cast<double>(r.hour), r.minute + r.second / 60.0};
      const double eot_hours = equation_of_time_days(julian_day(local, r.calendar)) * kHoursPerDay;
      switch (local_time_rule(r.year)) {
        case LocalTimeRule::kTrue:
          //RR WAHRE Ortszeit wird in MITTLERE Ortszeit umgerechnet !
          QMessageBox::information(this, "HORCOM", tr("WAHRE Ortszeit wird in MITTLERE Ortszeit umgerechnet !"));
          east += eot_hours;
          break;
        case LocalTimeRule::kAsk: {
          const int es = ChoiceDialog::ask(
              this, "HORCOM",
              {tr("Soll wirklich WAHRE Ortszeit ( = LTT ) eingegeben werden? "),
               tr("Oder soll es sich um MITTLERE Ortszeit ( = LMT ) handeln ?"),
               tr("Meist war zu diesem Datum bereits MITTLERE Ortszeit üblich !")},
              {tr(" WAHRE Ortszeit = LTT = WOZ"), tr(" MITTLERE Ortszeit = LMT = MOZ")}, 1);
          if (es == 0) {
            east += eot_hours;
            name = QStringLiteral("LTT");
          }
          break;
        }
        case LocalTimeRule::kMean:
          // his branch from 1890 left the zone name empty
          name.clear();
          break;
      }
    }
    zone_->setText(QString::fromStdString(aaf_zone(east)));
    dst_->setText(QStringLiteral("*"));
    znam_->setText(name);
  } else if (z.to_ut_hours) {
    zone_->setText(QString::fromStdString(aaf_zone(-*z.to_ut_hours)));
    dst_->clear();
    znam_->setText(QString::fromStdString(z.abbrev));
  } else {
    return;
  }
  // make_juld, the julian date follows the new zone
  if (mode_ != Mode::kShow && !jd_open_) {
    AafRecord r = record();
    r.jd = 0.0;
    jd_seed_ = aaf_moment_jd_ut(r);
    jd_->setText(QString::number(jd_seed_, 'f', 5));
  }
  com_->setFocus();
}

// the JULDATUM question of aaf_box, asked on entering the empty field
void AafMaskDialog::jd_priority() {
  if (mode_ == Mode::kShow || jd_open_ || field_number(jd_) > 0.0) {
    return;
  }
  //RR Ein HIER eingegebenes JULDATUM hat PRIORITÄT !
  const int ju = ChoiceDialog::ask(this, "HORCOM", {tr("Ein HIER eingegebenes JULDATUM hat PRIORITÄT !")},
                                   {tr("JA = JULDATUM EINGEBEN"), tr("ABBRUCH")}, 1);
  if (ju == 0) {
    jd_open_ = true;
    jd_->clear();
    jd_->setFocus();
  } else {
    jd_->clear();
    lat_ns_->setFocus();
  }
}

// leaving the julian date, jd = VAL(ed$(11)) with priority, @dat writes
// the date and time back as UT with the zone 00hW00:00 and the name GMT
void AafMaskDialog::jd_taken() {
  if (!jd_open_) {
    return;
  }
  const double jd = field_number(jd_);
  if (jd <= 0.0) {
    jd_open_ = false;
    jd_->clear();
    return;
  }
  const AafRecord r = record();
  const CalendarDate d = calendar_date(jd, r.calendar);
  int seconds = static_cast<int>(std::lround((d.hour * 60.0 + d.minute) * 60.0));
  if (seconds >= kSecondsPerDay) {
    seconds = kSecondsPerDay - 1;
  }
  const QString suffix = r.calendar == Calendar::kGregorian ? "g" : (r.calendar == Calendar::kJulian ? "j" : "");
  day_->setText(QString::number(d.day));
  month_->setText(QString::number(d.month));
  year_->setText(QString::number(d.year) + suffix);
  hour_->setText(QString::number(seconds / 3600));
  minute_->setText(QString::number((seconds / 60) % 60));
  second_->setText(QString::number(seconds % 60));
  zone_->setText(QString::fromLatin1(kUtZoneText));
  dst_->setText(QStringLiteral("*"));
  znam_->setText(QStringLiteral("GMT"));
}

// ported from the checks on MENU(6) = 119 of aaf_box, a forced calendar
// the reform already covers loses its letter
void AafMaskDialog::year_check() {
  if (mode_ == Mode::kShow) {
    return;
  }
  QString text = year_->text().trimmed();
  const bool gregorian = text.endsWith('g', Qt::CaseInsensitive);
  const bool julian = text.endsWith('j', Qt::CaseInsensitive);
  if (!(gregorian && !gregorian_checked_) && !(julian && !julian_checked_)) {
    return;
  }
  text.chop(1);
  const CalendarDate d{day_->text().toInt(), month_->text().toInt(), text.toInt(), 0.0, 0.0};
  if (gregorian) {
    gregorian_checked_ = true;
    if (julian_day(d, Calendar::kGregorian) > kReformCheckJd) {
      //RR Der GREGORIANISCHE Kalender ist OHNEHIN GÜLTIG !
      QMessageBox::information(this, "HORCOM", tr("Der GREGORIANISCHE Kalender ist OHNEHIN GÜLTIG !"));
      year_->setText(text);
    }
  } else {
    julian_checked_ = true;
    if (julian_day(d, Calendar::kJulian) < kReformCheckJd) {
      //RR Der JULIANISCHE Kalender ist OHNEHIN GÜLTIG !
      QMessageBox::information(this, "HORCOM", tr("Der JULIANISCHE Kalender ist OHNEHIN GÜLTIG !"));
      year_->setText(text);
    }
  }
}

// his kal$ beside the year
void AafMaskDialog::show_calendar_note() {
  const QString text = year_->text().trimmed();
  QString note;
  if (text.endsWith('g', Qt::CaseInsensitive)) {
    note = tr("GREGORIANISCH , VOR dem 15.10.1582 !");
  } else if (text.endsWith('j', Qt::CaseInsensitive)) {
    note = tr("JULIANISCH , NACH dem 04.10.1582 !");
  } else if (!text.isEmpty() && text.toInt() < 1) {
    // kal$ = " = " + STR$(ABS(VAL(ed$(5))) + 1) + " Vor Christus"
    note = tr(" = %1 Vor Christus").arg(std::abs(text.toInt()) + 1);
  }
  calendar_note_->setText(note);
}

// Entweder 'N' oder 'S' ! and Entweder 'E' oder 'W' !
void AafMaskDialog::letter_check(QLineEdit* field, char first, char second) {
  if (mode_ == Mode::kShow) {
    return;
  }
  const QString t = field->text().trimmed().toUpper();
  if (t == QChar(first) || t == QChar(second)) {
    return;
  }
  QMessageBox::information(this, "HORCOM", tr("Entweder '%1' oder '%2' !").arg(QChar(first)).arg(QChar(second)));
  field->setText(QString(QChar(first)));
  field->setFocus();
}

// ported from CASE 168 of aaf_box, the FILESELECT of the AAF file with
// the file of the working Daten-Datei as default
void AafMaskDialog::save() {
  if (mode_ == Mode::kShow) {
    return;
  }
  const auto letter_ok = [](const QLineEdit* e, char first, char second) {
    const QString t = e->text().trimmed().toUpper();
    return t == QChar(first) || t == QChar(second);
  };
  if (!letter_ok(lat_ns_, 'N', 'S')) {
    letter_check(lat_ns_, 'N', 'S');
    return;
  }
  if (!letter_ok(lon_ew_, 'E', 'W')) {
    letter_check(lon_ew_, 'E', 'W');
    return;
  }
  if (zone_->text().trimmed().isEmpty()) {
    //RR Das wichtige Feld 'Zone' FEHLT noch !!
    QMessageBox::information(this, "HORCOM", tr(" Das wichtige Feld 'Zone' FEHLT noch !!"));
    zone_->setFocus();
    return;
  }
  const QString start = QString::fromStdWString((aaf_file_.empty() ? data_dir_ : aaf_file_).wstring());
  QString path = QFileDialog::getSaveFileName(this, tr("AAF-Datei wählen oder neu anlegen"), start,
                                              tr("AAF (*.AAF *.aaf)"), nullptr, QFileDialog::DontConfirmOverwrite);
  if (path.isEmpty()) {
    //RR DATEI ???
    QMessageBox::information(this, "HORCOM", tr("DATEI ???"));
    return;
  }
  if (QFileInfo(path).suffix().isEmpty()) {
    path += ".AAF";
  }
  const std::filesystem::path aaf(path.toStdWString());
  if (store(aaf)) {
    saved_ = aaf;
    accept();
  }
}

// the new file branch with aaf_horcom0, the same name question, the
// append of aaf_satz_speich and the DAT rebuilt by aaf_horcom2
bool AafMaskDialog::store(const std::filesystem::path& aaf) {
  AafRecord r = record();
  if (r.jd <= 0.0) {
    // make_juld, the julian date of the fields goes into the file
    r.jd = aaf_moment_jd_ut(r);
  }
  const QString name = QString::fromStdWString(aaf.filename().wstring());
  std::vector<AafRecord> records;
  std::error_code ec;
  const bool new_file = !std::filesystem::exists(aaf, ec);
  if (new_file) {
    //RR Neue AAF-Datei ... !
    QMessageBox::information(this, "HORCOM", tr("Neue AAF-Datei %1 !").arg(name));
    records.push_back(r);
  } else {
    const auto have = read_aaf(aaf);
    if (!have) {
      QMessageBox::warning(this, "HORCOM", tr("Die AAF-Datei ließ sich nicht lesen."));
      return false;
    }
    records = *have;
    if (const auto hit = aaf_same_name(records, r)) {
      //RR Datensatz gleichen Namens bereits vorhanden !
      const int rr = ChoiceDialog::ask(this, "HORCOM",
                                       {tr("Datensatz gleichen Namens"), tr("bereits vorhanden !"),
                                        tr("NAME ÄNDERN bzw. MARKIEREN ?"), tr("und ZUSÄTZLICH speichern ?")},
                                       {tr("Datensatz ÜBERSCHREIBEN"), tr("NAME ÄNDERN bzw. MARKIEREN und speichern"),
                                        tr("ABBRUCH")});
      if (rr == 1) {
        set_mode(Mode::kEdit);
        surname_->setFocus();
        return false;
      }
      if (rr != 0) {
        reject();
        return false;
      }
      // aaf_satz_loesch and aaf_satz_add "a", the record moves to the end
      records.erase(records.begin() + static_cast<std::ptrdiff_t>(*hit));
    }
    records.push_back(r);
  }
  if (!write_aaf(aaf, records)) {
    QMessageBox::warning(this, "HORCOM", tr("Die AAF-Datei ließ sich nicht schreiben."));
    return false;
  }
  std::filesystem::path dat = dat_twin_path(aaf);
  if (new_file && std::filesystem::exists(dat, ec)) {
    // aaf_horcom0 before a new pair, an existing DAT asks first
    for (;;) {
      //RR Existierendes File ... ERSETZEN ?
      const int e = ChoiceDialog::ask(this, "HORCOM",
                                      {tr("Existierendes File"), QString::fromStdWString(dat.wstring()), tr("ERSETZEN ?")},
                                      {tr(" JA "), tr(" NEIN "), tr("ABBRUCH")});
      if (e == 0) {
        break;
      }
      if (e != 1) {
        return true;
      }
      const auto stem = ask_text(this, tr("ANDEREN Namen EINGEBEN !"), tr("Name OHNE EXTENSION !"));
      if (!stem || stem->trimmed().isEmpty()) {
        return true;
      }
      dat = dat.parent_path() / (stem->trimmed().section('.', 0, 0).toStdWString() + L".DAT");
      if (!std::filesystem::exists(dat, ec)) {
        break;
      }
    }
  }
  if (!write_dat_from_aaf(dat, records)) {
    QMessageBox::warning(this, "HORCOM", tr("Die HORCOM-Datei ließ sich nicht schreiben."));
  }
  return true;
}

void AafMaskDialog::keyPressEvent(QKeyEvent* event) {
  // VK_F1 and Alt+E, the AAF help
  if (event->key() == Qt::Key_F1 || (event->key() == Qt::Key_E && event->modifiers() == Qt::AltModifier)) {
    KommenDialog dialog(kommen_, "aaf_komm", english_edition(), this);
    dialog.exec();
    return;
  }
  QDialog::keyPressEvent(event);
}

AafRecord AafMaskDialog::record() const {
  AafRecord r = base_;
  r.surname = surname_->text().trimmed().toStdString();
  r.given = given_->text().trimmed().toStdString();
  r.sex = sex_->text().trimmed().toStdString();
  r.place = place_->text().trimmed().toStdString();
  r.country = country_->text().trimmed().toUpper().toStdString();
  r.day = day_->text().toInt();
  r.month = month_->text().toInt();
  // a g or j behind the astronomical year forces the calendar
  QString year_text = year_->text().trimmed();
  if (year_text.endsWith('g', Qt::CaseInsensitive)) {
    r.calendar = Calendar::kGregorian;
    year_text.chop(1);
  } else if (year_text.endsWith('j', Qt::CaseInsensitive)) {
    r.calendar = Calendar::kJulian;
    year_text.chop(1);
  } else {
    r.calendar = Calendar::kAuto;
  }
  r.year = year_text.toInt();
  r.hour = hour_->text().toInt();
  r.minute = minute_->text().toInt();
  r.second = second_->text().toInt();
  r.lat_ns = hemisphere_letter(lat_ns_->text(), 'N', 'S', base_.lat_ns);
  r.lat_deg = lat_deg_->text().toInt();
  r.lat_min = lat_min_->text().toInt();
  r.lat_sec = lat_sec_->text().toInt();
  r.lon_ew = hemisphere_letter(lon_ew_->text(), 'E', 'W', base_.lon_ew);
  r.lon_deg = lon_deg_->text().toInt();
  r.lon_min = lon_min_->text().toInt();
  r.lon_sec = lon_sec_->text().toInt();
  r.zone = zone_->text().trimmed().toStdString();
  r.dst = dst_->text().trimmed().toStdString();
  r.comment = com_->toPlainText().trimmed().toStdString();
  r.via = via_->text().trimmed().toStdString();
  r.source = src_->text().trimmed().toStdString();
  r.quality = gzq_->text().trimmed().toStdString();
  r.zone_name = znam_->text().trimmed().toStdString();
  r.catchword = cword_->text().trimmed().toStdString();
  r.attributes = attrb_->text().trimmed().toStdString();
  // edits to the clock, the zone or the summer time invalidate a stored
  // julian date, the fields rule
  if (r.day != base_.day || r.month != base_.month || r.year != base_.year || r.calendar != base_.calendar ||
      r.hour != base_.hour || r.minute != base_.minute || r.second != base_.second || r.zone != base_.zone ||
      r.dst != base_.dst) {
    r.jd = 0.0;
  }
  // ed$(11), a typed Juldatum takes priority over the clock fields
  const double typed = field_number(jd_);
  if (typed > 0.0 && typed != jd_seed_) {
    r.jd = typed;
  }
  return r;
}

}  // namespace horcom
