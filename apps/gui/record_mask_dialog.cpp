// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "record_mask_dialog.hpp"

#include <QCheckBox>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>
#include <string>

#include "aaf_mask_dialog.hpp"
#include "auto_advance.hpp"
#include "choice_dialog.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/collection.hpp"
#include "horcom/data/place_file.hpp"
#include "place_hub.hpp"
#include "robert_input.hpp"
#include "zeitzon_dialog.hpp"

namespace horcom {

namespace {

// the fields his afg! compared by VAL, the coordinates and the date
// and clock numbers in the order of eing_box
bool numeric_field(int index) {
  switch (index) {
    case 3: case 4: case 5: case 7: case 8: case 9: case 11: case 12: case 13: case 14: case 15: case 16:
      return true;
    default:
      return false;
  }
}

}  // namespace

// ported from eing_box and eingabe
RecordMaskDialog::RecordMaskDialog(AafRecord record, const QString& title, Mode mode,
                                   std::filesystem::path data_dir, QWidget* parent)
    : QDialog(parent), base_(std::move(record)), mode_(mode), data_dir_(std::move(data_dir)) {
  //RR EINGABE- und ANZEIGE-BOX
  setWindowTitle(title);
  auto* v = new QVBoxLayout(this);
  v->setContentsMargins(16, 14, 16, 14);
  v->setSpacing(14);

  auto* row1 = new QHBoxLayout();
  row1->addWidget(new QLabel(tr("NAME,VORNAME"), this));
  name_ = new QLineEdit(this);
  row1->addWidget(name_, 3);
  row1->addSpacing(10);
  row1->addWidget(new QLabel(tr("ORT"), this));
  place_ = new QLineEdit(this);
  row1->addWidget(place_, 2);
  // his gettext writes in capitals and stops at eingl|, 25 letters for
  // the name and 20 for the place like the DAT fields
  name_->setMaxLength(kDatNameLength);
  place_->setMaxLength(kDatPlaceLength);
  for (QLineEdit* e : {name_, place_}) {
    connect(e, &QLineEdit::textEdited, this, [e](const QString& t) {
      const int pos = e->cursorPosition();
      e->setText(t.toUpper());
      e->setCursorPosition(pos);
    });
  }
  v->addLayout(row1);

  auto* row2 = new QHBoxLayout();
  row2->addWidget(new QLabel(tr("LÄNGE O/W"), this));
  lon_ew_ = number_box(this, QString(), 1, 0);
  row2->addWidget(lon_ew_);
  row2->addWidget(new QLabel(QStringLiteral("°"), this));
  lon_deg_ = number_box(this, QString(), 3, 180);
  row2->addWidget(lon_deg_);
  row2->addWidget(new QLabel(QStringLiteral("'"), this));
  lon_min_ = number_box(this, QString(), 2, 59);
  row2->addWidget(lon_min_);
  row2->addWidget(new QLabel(QStringLiteral("''"), this));
  lon_sec_ = number_box(this, QString(), 2, 59);
  row2->addWidget(lon_sec_);
  row2->addStretch(1);
  row2->addWidget(new QLabel(tr("BREITE N/S"), this));
  lat_ns_ = number_box(this, QString(), 1, 0);
  row2->addWidget(lat_ns_);
  row2->addWidget(new QLabel(QStringLiteral("°"), this));
  lat_deg_ = number_box(this, QString(), 2, 89);
  row2->addWidget(lat_deg_);
  row2->addWidget(new QLabel(QStringLiteral("'"), this));
  lat_min_ = number_box(this, QString(), 2, 59);
  row2->addWidget(lat_min_);
  row2->addWidget(new QLabel(QStringLiteral("''"), this));
  lat_sec_ = number_box(this, QString(), 2, 59);
  row2->addWidget(lat_sec_);
  v->addLayout(row2);

  auto* row3 = new QHBoxLayout();
  row3->addWidget(new QLabel(tr("WENN V.CHR.,  'V' EINGEBEN"), this));
  bc_ = number_box(this, QString(), 1, 0);
  row3->addWidget(bc_);
  row3->addStretch(1);
  row3->addWidget(new QLabel(tr("DATUM :  TT"), this));
  day_ = number_box(this, QString(), 2, 31);
  row3->addWidget(day_);
  row3->addWidget(new QLabel(QStringLiteral("MM"), this));
  month_ = number_box(this, QString(), 2, 12);
  row3->addWidget(month_);
  row3->addWidget(new QLabel(tr("JJJJ"), this));
  // with the V flag the year field counts historically
  year_ = number_box(this, QString(), 5, 99999);
  row3->addWidget(year_);
  v->addLayout(row3);

  const bool entry = mode == Mode::kEntry;
  if (entry) {
    // MEZ und SONSTIGE ZONEN-ZEITEN, the box of zeitzon opens on ticking
    auto* row3b = new QHBoxLayout();
    row3b->addStretch(1);
    zones_ = new QCheckBox(tr("MEZ und SONSTIGE ZONEN-ZEITEN"), this);
    row3b->addWidget(zones_);
    v->addLayout(row3b);
    const double east = aaf_zone_hours(base_.zone) + aaf_dst_hours(base_.dst);
    if (east != 0.0) {
      zzd_ = -east;
      summer_ = static_cast<int>(aaf_dst_hours(base_.dst));
      zones_->setChecked(true);
    }
    connect(zones_, &QCheckBox::toggled, this, &RecordMaskDialog::zone_box);
  }

  auto* row4 = new QHBoxLayout();
  clock_label_ = new QLabel(this);
  row4->addWidget(clock_label_);
  hour_ = number_box(this, QString(), 2, 23);
  row4->addWidget(hour_);
  row4->addWidget(new QLabel(QStringLiteral("mm"), this));
  minute_ = number_box(this, QString(), 2, 59);
  row4->addWidget(minute_);
  row4->addWidget(new QLabel(QStringLiteral("ss"), this));
  second_ = number_box(this, QString(), 2, 59);
  row4->addWidget(second_);
  row4->addStretch(2);
  if (entry) {
    //RR Zonenzeitdiff ZZD =
    zzd_label_ = new QLabel(this);
    row4->addWidget(zzd_label_);
    show_zzd();
  }
  v->addLayout(row4);

  if (entry) {
    auto* row4b = new QHBoxLayout();
    //RR ZEITBESTIMMUNGEN LESEN
    auto* texts = new QPushButton(tr("ZEITBESTIMMUNGEN LESEN"), this);
    connect(texts, &QPushButton::clicked, this, [this]() { read_time_determinations(this, data_dir_); });
    row4b->addWidget(texts, 0, Qt::AlignTop);
    row4b->addStretch(1);
    auto* switches = new QVBoxLayout();
    //RR ORTSZEIT (HISTORISCHE HOROSKOPE), NOCH JULIANISCH ( NACH 1582 )
    local_ = new QCheckBox(tr("ORTSZEIT (HISTORISCHE HOROSKOPE)"), this);
    julian_ = new QCheckBox(tr("NOCH JULIANISCH ( NACH 1582 )"), this);
    julian_->setChecked(base_.calendar == Calendar::kJulian);
    local_->setChecked(julian_->isChecked());
    switches->addWidget(local_);
    switches->addWidget(julian_);
    row4b->addLayout(switches);
    v->addLayout(row4b);
    // his CHECK?(121) set ortsz! too, a Julian date runs on local time
    connect(julian_, &QCheckBox::toggled, this, [this](bool on) {
      if (on) {
        local_->setChecked(true);
      }
    });
  }

  auto* row5 = new QHBoxLayout();
  row5->addStretch(1);
  row5->addWidget(new QLabel(tr("BEMERKUNG:"), this));
  comment_ = new QLineEdit(this);
  comment_->setMaxLength(kDatRemarkLength);
  row5->addWidget(comment_, 3);
  auto* ok = new QPushButton(tr("OK"), this);
  ok->setDefault(true);
  connect(ok, &QPushButton::clicked, this, &RecordMaskDialog::finish);
  row5->addWidget(ok);
  v->addLayout(row5);

  // the AAF - Format button that switches into the richer AAF box. His
  // NEU-EINGABE had none, the format question comes first there
  const bool aaf_button = !entry && !data_dir_.empty();
  if (aaf_button || entry) {
    auto* row6 = new QHBoxLayout();
    if (aaf_button) {
      aaf_ = new QPushButton(tr("AAF-Format"), this);
      connect(aaf_, &QPushButton::clicked, this, &RecordMaskDialog::open_aaf_box);
      row6->addWidget(aaf_);
    }
    if (entry) {
      // &ABBRUCH, bottom left under his ZEITBESTIMMUNGEN button
      auto* cancel = new QPushButton(tr("ABBRUCH"), this);
      connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
      row6->addWidget(cancel);
    }
    row6->addStretch(1);
    v->addLayout(row6);
  }
  fill_fields(base_);
  place_->installEventFilter(this);
  lat_sec_->installEventFilter(this);
  // eingl|(diha&,i&), the lengths that complete each field of eing_box
  chain_fields({{name_, 25}, {place_, 20}, {lon_ew_, 1}, {lon_deg_, 3}, {lon_min_, 2}, {lon_sec_, 2},
                {lat_ns_, 1}, {lat_deg_, 2}, {lat_min_, 2}, {lat_sec_, 2}, {bc_, 1}, {day_, 2},
                {month_, 2}, {year_, 4}, {hour_, 2}, {minute_, 2}, {second_, 2}, {comment_, 0}});
  resize(760, 0);
  name_->setFocus();
}

// the et$ fields of a record, his nein! cleared every one for a fresh
// entry, so a record without date shows empty boxes
void RecordMaskDialog::fill_fields(const AafRecord& r) {
  const bool fresh = r.day == 0 && r.month == 0 && r.year == 0 && r.jd <= 0.0;
  const auto num = [fresh](int value) { return fresh ? QString() : QString::number(value); };
  joined_name_ = QString::fromStdString(record_name(r));
  name_->setText(joined_name_);
  place_->setText(QString::fromStdString(r.place));
  lon_ew_->setText(QString(QChar(r.lon_ew)));
  lon_deg_->setText(num(r.lon_deg));
  lon_min_->setText(num(r.lon_min));
  lon_sec_->setText(num(r.lon_sec));
  lat_ns_->setText(QString(QChar(r.lat_ns)));
  lat_deg_->setText(num(r.lat_deg));
  lat_min_->setText(num(r.lat_min));
  lat_sec_->setText(num(r.lat_sec));
  bc_->setText(r.year > 0 || fresh ? QString() : QStringLiteral("V"));
  day_->setText(num(r.day));
  month_->setText(num(r.month));
  year_->setText(num(r.year > 0 ? r.year : 1 - r.year));
  hour_->setText(num(r.hour));
  minute_->setText(num(r.minute));
  second_->setText(num(r.second));
  comment_->setText(QString::fromStdString(r.comment));
  // his NEU-EINGABE reads UHRZEIT, else a clock off UT is a local one
  // like his UHRZEIT label, the zone and summer time read like the loader
  const bool zoned = mode_ == Mode::kEntry || aaf_zone_hours(r.zone) + aaf_dst_hours(r.dst) != 0.0;
  clock_label_->setText(zoned ? tr("UHRZEIT : hh") : tr("WZ = GMT = UT : hh"));
  place_seen_ = place_fields();
  opened_ = field_texts();
}

// his fields 1 through 18 of eing_box in their order
void RecordMaskDialog::limit_fields(int last) {
  QLineEdit* const fields[] = {name_,   place_,   lon_ew_, lon_deg_, lon_min_, lon_sec_,
                               lat_ns_, lat_deg_, lat_min_, lat_sec_, bc_,     day_,
                               month_,  year_,    hour_,   minute_,  second_,  comment_};
  for (int i = last; i < static_cast<int>(std::size(fields)); ++i) {
    fields[i]->setEnabled(false);
  }
  if (aaf_ != nullptr) {
    aaf_->hide();
  }
}

void RecordMaskDialog::bind_file(const QString& file_label, std::filesystem::path aaf_file) {
  file_label_ = file_label;
  aaf_file_ = std::move(aaf_file);
  bound_ = true;
}

QStringList RecordMaskDialog::field_texts() const {
  QStringList out;
  for (const QLineEdit* e : {name_, place_, lon_ew_, lon_deg_, lon_min_, lon_sec_, lat_ns_, lat_deg_, lat_min_,
                             lat_sec_, bc_, day_, month_, year_, hour_, minute_, second_, comment_}) {
    out << e->text().trimmed().toUpper();
  }
  return out;
}

// his afg!, a field differs from i$ as the box opened it
bool RecordMaskDialog::edited() const {
  const QStringList now = field_texts();
  for (int i = 0; i < now.size() && i < opened_.size(); ++i) {
    const bool same = numeric_field(i) ? now[i].toInt() == opened_[i].toInt() : now[i] == opened_[i];
    if (!same) {
      return true;
    }
  }
  return false;
}

// ported from the OK path of eingabe for the ANZEIGE box, an edited record
// meets his eingaaf warning when an AAF twin exists and then his eing2
// question, an untouched one is taken at once
void RecordMaskDialog::finish() {
  if (mode_ != Mode::kShow || !bound_ || !edited()) {
    accept();
    return;
  }
  std::error_code ec;
  if (std::filesystem::exists(aaf_file_, ec)) {
    //RR Änderungen nur im AAF-File vornehmen ! Sonst evtl. DATENVERLUST !!
    const int al = ChoiceDialog::ask(this, "HORCOM",
                                     {tr("Änderungen nur im AAF-File vornehmen !"), tr("Sonst evtl. DATENVERLUST !!"),
                                      tr("Dort 'Datensatz ÄNDERN' anklicken und EDITIEREN !")},
                                     {tr("ZURÜCK"), tr("ABBRUCH"), tr("TROTZDEM WEITER")}, 0);
    if (al != 2) {
      // et$(i&) = i$(i&), ZURÜCK stays in the box, ABBRUCH leaves it with
      // the record as it was fetched
      fill_fields(base_);
      if (al == 1) {
        accept();
      } else {
        name_->setFocus();
      }
      return;
    }
  }
  // his eing2 question with ds$ = " In Datei " + the file name
  const int ada = ChoiceDialog::ask(this, "HORCOM",
                                    {tr("Datensatz In Datei %1 ABSPEICHERN ?").arg(file_label_), QString(),
                                     tr("Oder Weiter EDITIEREN ?")},
                                    {tr("In Datei %1 ABSPEICHERN").arg(file_label_), tr("Weiter EDITIEREN"),
                                     tr("NUR als Datensatz ÜBERNEHMEN")},
                                    2);
  if (ada == 0) {
    save_requested_ = true;
    accept();
  } else if (ada == 2) {
    accept();
  }
}

// the menu and the question come from tabbing forward like his MENU(6)
// order test, a mouse click into the field stays quiet
bool RecordMaskDialog::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::FocusIn && watched == place_ &&
      static_cast<QFocusEvent*>(event)->reason() == Qt::TabFocusReason) {
    QTimer::singleShot(0, this, &RecordMaskDialog::place_menu);
  } else if (event->type() == QEvent::FocusOut && watched == lat_sec_ &&
             static_cast<QFocusEvent*>(event)->reason() == Qt::TabFocusReason) {
    QTimer::singleShot(0, this, &RecordMaskDialog::place_leave);
  }
  return QDialog::eventFilter(watched, event);
}

QStringList RecordMaskDialog::place_fields() const {
  return {place_->text(),   lon_ew_->text(), lon_deg_->text(), lon_min_->text(), lon_sec_->text(),
          lat_ns_->text(),  lat_deg_->text(), lat_min_->text(), lat_sec_->text()};
}

void RecordMaskDialog::take_place(const QString& name, double lon, double lat) {
  place_->setText(name.toUpper().left(kDatPlaceLength));
  const auto dms = [](double value, QLineEdit* d, QLineEdit* m, QLineEdit* s) {
    const Dms x = split_dms(value);
    d->setText(QString::number(x.deg));
    m->setText(QString::number(x.min));
    s->setText(QString::number(x.sec));
  };
  lon_ew_->setText(lon < 0.0 ? "W" : "E");
  dms(lon, lon_deg_, lon_min_, lon_sec_);
  lat_ns_->setText(lat < 0.0 ? "S" : "N");
  dms(lat, lat_deg_, lat_min_, lat_sec_);
}

// ported from CASE 102 of eingabe, the ORT menu
void RecordMaskDialog::place_menu() {
  if (neu_) {
    return;
  }
  const std::filesystem::path ext = data_dir_ / "ort.ext";
  const bool have = !data_dir_.empty() && std::filesystem::exists(ext);
  // vv$ = "VORZUGSORT ", ue$(0) = er$ + "EINGEBEN !", his er$ was empty here
  const int es = ChoiceDialog::ask(
      this, tr("EINGEBEN !"), {},
      {tr(" NEUER ORT oder ORT EDITIEREN "), have ? tr(" VORZUGSORT ") : tr(" VORZUGSORT  LEER ! NEU EINGEBEN !"),
       tr("ORTS-DATEIEN : HOLEN - EINTRAGEN - LÖSCHEN"),
       have ? tr("VORZUGSORT LÖSCHEN") : tr("VORZUGSORT LÖSCHEN  ( ist LEER ! )"), tr("UNDO = NICHT ÄNDERN ! ")});
  switch (es) {
    case 0:
      vorz_ = false;
      holda_ = false;
      place_seen_ = place_fields();
      neu_ = place_->text().trimmed().isEmpty();
      place_->setFocus();
      return;
    case 1: {
      holda_ = false;
      neu_ = false;
      vorz_ = true;
      place_seen_ = place_fields();
      if (!have) {
        if (mode_ == Mode::kEntry) {
          //RR VORZUGSORT ERSTMALIG von HAND EINGEBEN !
          QMessageBox::information(this, "HORCOM", tr("VORZUGSORT ERSTMALIG von HAND EINGEBEN !"));
          place_->setFocus();
        } else {
          //RR VORZUGSORT ERSTMALIG unter 'NEUEINGABE' von HAND EINGEBEN !
          QMessageBox::information(this, "HORCOM", tr("VORZUGSORT ERSTMALIG unter 'NEUEINGABE' von HAND EINGEBEN !"));
          reject();
        }
        return;
      }
      if (const auto home = read_preferred_place(ext)) {
        take_place(QString::fromStdString(home->name), home->lon, home->lat);
      }
      day_->setFocus();
      return;
    }
    case 2:
      vorz_ = false;
      neu_ = false;
      holda_ = true;
      place_seen_ = place_fields();
      if (const auto got = place_file_hub(this, data_dir_)) {
        take_place(QString::fromStdString(got->name), got->lon, got->lat);
        day_->setFocus();
      } else {
        place_->setFocus();
      }
      return;
    case 3:
      delete_preferred_place(this, data_dir_);
      name_->setFocus();
      return;
    case 4:
      vorz_ = false;
      holda_ = false;
      neu_ = false;
      name_->setFocus();
      return;
    default:
      place_->setFocus();
      return;
  }
}

// ported from CASE 111 of eingabe, leaving the coordinates
void RecordMaskDialog::place_leave() {
  if (data_dir_.empty()) {
    return;
  }
  const AafRecord r = record();
  PlaceRecord here{r.longitude(), r.latitude(), r.place};
  // IF vorz! && EXIST(hrc$ + "\ORT.EXT") = FALSE, the first preferred
  // place typed by hand
  if (vorz_ && !std::filesystem::exists(data_dir_ / "ort.ext")) {
    if (write_preferred_place(data_dir_ / "ort.ext", here)) {
      //RR NEUER VORZUGSORT gespeichert !
      QMessageBox::information(this, "HORCOM", tr("NEUER VORZUGSORT gespeichert !"));
    }
    return;
  }
  const bool changed = place_fields() != place_seen_;
  if (place_->text().trimmed().isEmpty() || vorz_ || holda_ || !(changed || neu_)) {
    return;
  }
  //RR ORT ABSPEICHERN ?
  const int osp = ChoiceDialog::ask(this, "HORCOM", {tr("ORT ABSPEICHERN ?")},
                                    {tr("JA"), tr(" NEIN "), tr("Aktuellen Ort als VORZUGSORT speichern")}, 1);
  if (osp == 0) {
    enter_place(this, data_dir_, here);
  } else if (osp == 2) {
    store_preferred_place(this, data_dir_, here);
  }
  // @eingfl_l
  neu_ = false;
  vorz_ = false;
  holda_ = false;
}

// ported from CASE 125 of eingabe, the AAF - Format button. The AAF box
// opens in his display mode, Zurück zum HORCOM-Format or a save there
// brings the AAF record back into this box like his a2113 with eing_box,
// EXIT leaves the EINGABE with the record as it stood like his expr!
void RecordMaskDialog::open_aaf_box() {
  AafMaskDialog dialog(record(), AafMaskDialog::Mode::kShow, data_dir_, this);
  dialog.set_aaf_file(aaf_file_);
  if (dialog.exec() != QDialog::Accepted) {
    accept();
    return;
  }
  if (dialog.saved_file()) {
    aaf_saved_ = dialog.saved_file();
  }
  base_ = dialog.record();
  fill_fields(base_);
  name_->setFocus();
}

// ported from CASE 119 of eingabe, the zone box opens only on ticking
// with an hour filled in, a cancelled box leaves the switch off
void RecordMaskDialog::zone_box(bool on) {
  if (on && !hour_->text().trimmed().isEmpty()) {
    ZeitzonDialog dialog(data_dir_, this);
    if (dialog.exec() == QDialog::Accepted) {
      zzd_ = dialog.zzd();
      summer_ = dialog.summer();
      show_zzd();
      return;
    }
  }
  zzd_ = 0.0;
  summer_ = 0;
  const QSignalBlocker b(zones_);
  zones_->setChecked(false);
  show_zzd();
}

void RecordMaskDialog::show_zzd() {
  if (zzd_label_ == nullptr) {
    return;
  }
  // "Zonenzeitdiff ZZD = " + STR$(zeitzon)
  zzd_label_->setText(zzd_ != 0.0 ? tr("Zonenzeitdiff ZZD = %1").arg(zzd_) : QString());
}

bool RecordMaskDialog::local_time() const {
  return local_ != nullptr && local_->isChecked();
}

void RecordMaskDialog::preset_local_time(bool on) {
  if (local_ != nullptr) {
    local_->setChecked(on);
  }
}

AafRecord RecordMaskDialog::record() const {
  AafRecord r = base_;
  const QString name = name_->text().trimmed();
  if (name != joined_name_) {
    // an edited name lands whole in the first field like the 25 byte
    // DAT name did, the split into surname and given name is gone
    r.surname = name.toStdString();
    r.given.clear();
  }
  r.place = place_->text().trimmed().toStdString();
  r.lon_ew = hemisphere_letter(lon_ew_->text(), 'E', 'W', base_.lon_ew);
  r.lon_deg = lon_deg_->text().toInt();
  r.lon_min = lon_min_->text().toInt();
  r.lon_sec = lon_sec_->text().toInt();
  r.lat_ns = hemisphere_letter(lat_ns_->text(), 'N', 'S', base_.lat_ns);
  r.lat_deg = lat_deg_->text().toInt();
  r.lat_min = lat_min_->text().toInt();
  r.lat_sec = lat_sec_->text().toInt();
  r.day = day_->text().toInt();
  r.month = month_->text().toInt();
  const int year = year_->text().toInt();
  // his V or minus turns the historical year into the astronomical count
  r.year = before_christ_flag(bc_->text()) ? 1 - year : year;
  r.hour = hour_->text().toInt();
  r.minute = minute_->text().toInt();
  r.second = second_->text().toInt();
  r.comment = comment_->text().trimmed().toStdString();
  if (mode_ == Mode::kEntry) {
    // zeitzon = zeitzon(ze), somz = somz(ze), a local clock knows no zone
    const bool local = local_time();
    r.zone = aaf_zone(local ? 0.0 : -zzd_ - summer_);
    r.dst = local ? "0" : std::to_string(summer_);
    r.calendar = julian_->isChecked() ? Calendar::kJulian : Calendar::kAuto;
  }
  // edits invalidate a stored julian date, the clock fields rule again
  if (r.day != base_.day || r.month != base_.month || r.year != base_.year || r.hour != base_.hour ||
      r.minute != base_.minute || r.second != base_.second) {
    r.jd = 0.0;
  }
  return r;
}

}  // namespace horcom
