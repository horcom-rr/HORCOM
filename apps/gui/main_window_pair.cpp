// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// The pair charts of the HOROSKOPE menu, the COMPOSIT of a13 and the
// DOPPEL-KREIS and 90-GRAD-KREIS of a12 with their SATZ picks and his
// full sheet.

#include <QAction>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QMessageBox>

#include "banner.hpp"
#include "choice_dialog.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/harmonics.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/pair_sheet.hpp"
#include "main_window.hpp"
#include "record_mask_dialog.hpp"
#include "robert_text.hpp"
#include "wheel_widget.hpp"

namespace horcom {

namespace {

void sheet_text(DisplayList& dl, double x, double bottom, const QString& s, double size) {
  add_sheet_text(dl, x, bottom, s.toStdString(), size);
}

QString name_of(const AafRecord& r) {
  return (QString::fromStdString(r.surname).trimmed() + " " + QString::fromStdString(r.given).trimmed())
      .trimmed()
      .toUpper();
}

}  // namespace

// the SATZ clicks of his menu, every filled RADIX and SOLAR slot
std::optional<MainWindow::SlotChoice> MainWindow::pick_slot(const QString& title, const QStringList& info,
                                                             const std::vector<SlotChoice>& marked) const {
  // sol$(od,ze) = "*" + sol$(od,ze) for every click so far
  const auto stars = [&marked](bool solar, int i) {
    const auto n = std::count_if(marked.begin(), marked.end(),
                                 [&](const SlotChoice& c) { return c.solar == solar && c.index == i; });
    return QString(static_cast<int>(n), QChar('*'));
  };
  QStringList buttons;
  std::vector<SlotChoice> map;
  const int satz = static_cast<int>(slots_.size());
  for (int i = 0; i < satz; ++i) {
    if (const auto& r = slots_[static_cast<std::size_t>(i)]) {
      // SATZ1, RADIX and the name
      buttons << QString("SATZ%1: %2%3  %4").arg(i + 1).arg(stars(false, i), radix_label(i), name_of(*r));
      map.push_back({false, i});
    }
  }
  for (int i = 0; i < static_cast<int>(solar_slots_.size()); ++i) {
    if (const auto& r = solar_slots_[static_cast<std::size_t>(i)]) {
      buttons << QString("SATZ%1: %2%3  %4")
                     .arg(i + 1)
                     .arg(stars(true, i), solar_labels_[static_cast<std::size_t>(i)], name_of(*r));
      map.push_back({true, i});
    }
  }
  if (map.empty()) {
    return std::nullopt;
  }
  buttons << tr("ABBRUCH");
  const int es = ChoiceDialog::ask(const_cast<MainWindow*>(this), title, info, buttons, 0);
  if (es < 0 || es >= static_cast<int>(map.size())) {
    return std::nullopt;
  }
  return map[static_cast<std::size_t>(es)];
}

// the menu click on a SATZ entry, the slot becomes the active chart
void MainWindow::activate_slot(const SlotChoice& c) {
  // an edit still settling belongs to the slot the panel leaves
  flush_history();
  const auto i = static_cast<std::size_t>(c.index);
  if (c.solar) {
    if (solar_slots_[i]) {
      active_is_solar_ = true;
      active_solar_ = c.index;
      apply_record(*solar_slots_[i], false);
      banner_->set_record(solar_labels_[i]);
    }
  } else if (slots_[i]) {
    active_slot_ = c.index;
    active_is_solar_ = false;
    apply_record(*slots_[i]);
  }
  update_slot_actions();
  update_solar_actions();
}

// ported from a13 with a12i and a12a, the MODUS box, the two SATZ clicks
// and under ROBERT HAND the residence of the pair
void MainWindow::composite_session() {
  // CLR hrg!, a13 casts the composite geocentric whatever the panel held
  if (helio_->isChecked()) {
    helio_->setChecked(false);
  }
  const HouseSystem sys = current_settings().houses;
  if (sys == HouseSystem::kEqualAsc || sys == HouseSystem::kEqualVehlow) {
    // the equal systems haw& 6 and 7 clear comp_hand! and comp_mstz!
    konsta_.comp_mstz = false;
    konsta_.comp_hand = false;
  } else {
    // h$ = "HÄUSER-SYSTEM ", "COMPOSIT-HOROSKOP MODUS ?","WELCHES "+h$+"?"
    const int preset = konsta_.comp_mstz ? 0 : (konsta_.comp_hand ? 1 : 2);
    const int b = ChoiceDialog::ask(this, tr("AUSWAHL"), {tr("COMPOSIT-HOROSKOP MODUS ?"), tr("WELCHES HÄUSER-SYSTEM ?")},
                                    {tr("HÄUSER-SYSTEM mit MITTLERER STZ,Länge,Breite ab MC -HALBSUMME"),
                                     tr("HÄUSER-SYSTEM nach ROBERT HAND ( AUFENTHALTS-ORT )"),
                                     tr("HÄUSER-SYSTEM : SCHEMATISCHE HALBSUMMEN ab MC -HALBSUMME")},
                                    preset);
    // his box has no ABBRUCH, ESC keeps the mode it offered
    if (b >= 0) {
      konsta_.comp_mstz = b == 0;
      konsta_.comp_hand = b == 1;
    }
  }
  // @param_sp
  persist_konsta();
  // fanz("2 DATENSÄTZE NACHEINANDER ANKLICKEN ! "), da$ + p1$ + ai$ and ia$
  const QString ia = tr("COMPOSIT - HOROSKOP");
  const auto first = pick_slot(tr(" HINWEIS "), {tr("2 DATENSÄTZE NACHEINANDER ANKLICKEN ! "),
                                                 tr("Datensatz für PARTNER 1    aktivieren !"), tr("  "), ia});
  if (!first) {
    if (!slots_[0] && !slots_[1] && !slots_[2] && !slots_[3] && !slots_[4]) {
      QMessageBox::information(this, "HORCOM", tr("Zuerst Datensätze in die SATZ-Plätze holen !"));
    }
    return;
  }
  const auto second = pick_slot(tr(" HINWEIS "), {QString(), tr("Datensatz für PARTNER 2    aktivieren !"), tr("  "), ia});
  if (!second) {
    return;
  }
  const AafRecord partner =
      second->solar ? *solar_slots_[static_cast<std::size_t>(second->index)] : *slots_[static_cast<std::size_t>(second->index)];
  std::optional<EventPlace> residence;
  if (konsta_.comp_hand) {
    // @et_l, @eingabe(0,-1,1,10), ort_l1 empties the place, E and N stay
    RecordMaskDialog mask(AafRecord{}, tr("EREIGNIS-ORT  EINGEBEN !  ->  EVENTL. TAB - TASTE !  |  COMPOSIT NR.1"),
                          RecordMaskDialog::Mode::kShow, data_dir_, this);
    mask.limit_fields(10);
    if (mask.exec() != QDialog::Accepted) {
      // IF expr!, ekls = ekk and EXPROC
      return;
    }
    // @a31_o(gl,gg,gl$,gg$,na$,go$)
    const AafRecord at = mask.record();
    residence = EventPlace{at.longitude(), at.latitude(), at.place};
  }
  leave_views({});
  activate_slot(*first);
  if (!set_partner(partner)) {
    QMessageBox::warning(this, "HORCOM", tr("Der gewählte Datensatz ließ sich nicht berechnen."));
    return;
  }
  comp_residence_ = residence;
  remember_double(kDoubleComposit, partner);
  double_slots_[kDoubleComposit]->residence = residence;
  claim_wheel();
  {
    const QSignalBlocker block(composite_action_);
    composite_action_->setChecked(true);
  }
  recompute();
  // moda& = @druck_graph_ein after the picks
  chart_output(menu_item::kComposite, false);
}

// ported from a14 with a14_1, two to five SATZ clicks one after the
// other, the HOLEN ? box between them while more RADIX slots are filled
void MainWindow::combin_chart() {
  // CLR hrg!, a14 casts the combin geocentric whatever the panel held
  if (helio_->isChecked()) {
    helio_->setChecked(false);
  }
  // zesp, the highest filled RADIX slot
  int zesp = 0;
  for (std::size_t i = 0; i < slots_.size(); ++i) {
    if (slots_[i]) {
      zesp = static_cast<int>(i) + 1;
    }
  }
  // fanz(" DATENSÄTZE ( MINDESTENS 2, HÖCHSTENS 5 ) NACHEINANDER ANKLICKEN ! ")
  const QStringList info{QString(), tr(" DATENSÄTZE ( MINDESTENS 2, HÖCHSTENS 5 ) NACHEINANDER ANKLICKEN ! "), tr("  "),
                         tr("COMBIN - HOROSKOP")};
  constexpr std::size_t kMostRecords = 5;
  std::vector<SlotChoice> picked;
  while (picked.size() < kMostRecords) {
    const auto c = pick_slot(tr(" HINWEIS "), info, picked);
    if (!c) {
      if (zesp == 0 && picked.empty()) {
        QMessageBox::information(this, "HORCOM", tr("Zuerst Datensätze in die SATZ-Plätze holen !"));
      }
      return;
    }
    picked.push_back(*c);
    const int z = static_cast<int>(picked.size());
    if (z < 2) {
      continue;
    }
    if (z == static_cast<int>(kMostRecords) || z >= zesp) {
      break;
    }
    // STR$(z& + 1) + ". " + das$ + "HOLEN ?","oder","AUSGABE ?", ESC outputs
    const int b = ChoiceDialog::ask(this, tr("AUSWAHL"),
                                    {QString(), tr("%1. Datensatz HOLEN ?").arg(z + 1), tr("oder"), tr("AUSGABE ?")},
                                    {tr("HOLEN"), tr("AUSGABE")}, 0);
    if (b != 0) {
      break;
    }
  }
  std::vector<AafRecord> parts;
  std::vector<int> sets;
  for (const SlotChoice& c : picked) {
    parts.push_back(c.solar ? *solar_slots_[static_cast<std::size_t>(c.index)] : *slots_[static_cast<std::size_t>(c.index)]);
    // z$ = z$ + "," + STR$(e&(i&))
    sets.push_back(c.index + 1);
  }
  leave_views({});
  activate_slot(picked.front());
  combin_of(parts, sets);
  // od = 0, ze = 2, the DOPPEL-DATEN row COMBIN
  DoubleSlot row{parts[0], parts[1], std::nullopt, parts, sets};
  double_slots_[kDoubleCombin] = row;
  update_double_actions();
  // moda& = @druck_graph_ein after the picks
  chart_output(menu_item::kCombin, false);
}

// ported from a12 with a12i and a12a, the MODUS box and the two SATZ
// clicks for the inner and the outer circle
void MainWindow::double_wheel_session() {
  // ue$(0) = "MODUS ?", ze$(2) = mer$ + " = " + ac$
  const bool dial_now = dial_action_ != nullptr && dial_action_->isChecked();
  const int es = ChoiceDialog::ask(this, tr("MODUS ?"), {},
                                   {tr(" NORMAL-KREIS "), tr(" 90°-KREIS"), tr("Zurück zum HAUPT - MENÜ = ABBRUCH")},
                                   dial_now ? 1 : 0);
  if (es < 0 || es == 2) {
    return;
  }
  const bool dial = es == 1;
  // ia$, NORMALES DOPPEL - HOROSKOP or 90° - DOPPEL - HOROSKOP
  const QString ia = dial ? tr("90° - DOPPEL - HOROSKOP") : tr("NORMALES DOPPEL - HOROSKOP");
  // da$ + in$ + ai$, "Datensatz für INNEN-Kreis   aktivieren !"
  const auto inner = pick_slot(tr(" HINWEIS "), {QString(), tr("Datensatz für INNEN-Kreis   aktivieren !"), tr("  "), ia});
  if (!inner) {
    if (!slots_[0] && !slots_[1] && !slots_[2] && !slots_[3] && !slots_[4]) {
      QMessageBox::information(this, "HORCOM", tr("Zuerst Datensätze in die SATZ-Plätze holen !"));
    }
    return;
  }
  const auto outer = pick_slot(tr(" HINWEIS "), {QString(), tr("Datensatz für AUSSEN-Kreis   aktivieren !"), tr("  "), ia});
  if (!outer) {
    return;
  }
  const AafRecord partner =
      outer->solar ? *solar_slots_[static_cast<std::size_t>(outer->index)] : *slots_[static_cast<std::size_t>(outer->index)];
  const QString partner_label = outer->solar ? solar_labels_[static_cast<std::size_t>(outer->index)] : radix_label(outer->index);
  leave_views({});
  activate_slot(*inner);
  if (!set_partner(partner)) {
    QMessageBox::warning(this, "HORCOM", tr("Der gewählte Datensatz ließ sich nicht berechnen."));
    return;
  }
  partner_label_ = partner_label;
  remember_double(kDoubleWheel, partner);
  claim_wheel();
  {
    const QSignalBlocker b1(compare_action_);
    const QSignalBlocker b2(dial_action_);
    compare_action_->setChecked(true);
    dial_action_->setChecked(dial);
  }
  recompute();
  // IF inn! = TRUE && auu! = TRUE : moda& = @druck_graph_ein
  chart_output(menu_item::kDoubleWheel, false);
}

// ported from the drawing part of a12 with a12beschr1, a12beschr and
// a12asp, the full sheet with both coordinate columns and the grid
DisplayList MainWindow::a12_sheet(const Chart& inner, const Chart& outer, const ChartSettings& s, bool dial,
                                  DisplayList wheel) const {
  DisplayList& dl = wheel;
  // bes11 header, Ekl.Länge: or Länge: under voll!, the mode tag A1 A2 W
  const QString mode = konsta_.appa == 2 ? "A2" : (konsta_.appa == 3 ? "W" : "A1");
  PairColumnOptions col;
  col.header = ((konsta_.voll ? tr("Länge:") : tr("Ekl.Länge:")) + mode).toStdString();
  // vf!, dppel! with klpl! and more than one extra
  int extras = 0;
  for (int slot = body::kApogee; slot < body::kSlotCount; ++slot) {
    extras += inner.b[static_cast<std::size_t>(slot)].present ? 1 : 0;
  }
  col.compact = s.extra_bodies && extras > 1;
  col.parallax = s.topocentric_parallax;
  col.true_node = s.true_node;
  col.true_apogee = s.true_apogee;
  col.heliocentric = s.heliocentric;
  col.extras = s.extra_bodies;
  col.dial = dial;
  col.houses_header = (konsta_.voll ? tr("Häusersp.") : tr("Häuserspitzen")).toStdString();
  col.house_name = QString::fromUtf8(inner.houses.name.data(), static_cast<int>(inner.houses.name.size())).trimmed().toStdString();
  // xt& = 4, yt& = 24 for the inner column, 114 for the outer
  double y = add_pair_bodies(dl, inner, col, 4.0, 24.0);
  add_pair_houses(dl, inner, col, 4.0, y);
  PairColumnOptions out_col = col;
  out_col.house_name = QString::fromUtf8(outer.houses.name.data(), static_cast<int>(outer.houses.name.size())).trimmed().toStdString();
  y = add_pair_bodies(dl, outer, out_col, 114.0, 24.0);
  y = add_pair_houses(dl, outer, out_col, 114.0, y);
  // a12beschr, ADD yt&,8, then a12asp at xt& = 2
  CrossScanOptions scan;
  scan.extras = s.extra_bodies;
  scan.heliocentric = s.heliocentric;
  const std::vector<CrossAspectHit> hits = scan_aspects_between(inner, outer, shown_aspect_settings(), scan);
  CrossGridOptions grid;
  grid.outer_color = outer_color_;
  grid.more_label = tr(" MEHR ").toStdString();
  add_cross_grid(dl, hits, grid, 2.0, y + 8.0);

  // the corners of a12, the inner chart on top, the outer below
  const auto moment = [&](const Chart& c, double x, double y_date, double y_ut) {
    const CalendarDate d = calendar_date(c.jd_ut, s.calendar);
    // datum1$ = a$ + "." + b$ + "." + d$, the year five wide
    sheet_text(dl, x, y_date, tr("Datum:") + QString::asprintf("%2d.%2d.%5d", d.day, d.month, d.year), 13.0);
    // ze1$ = homi$, whole minutes
    const long minutes = std::lround((d.hour + d.minute / 60.0) * 60.0) % static_cast<long>(kMinutesPerDay);
    sheet_text(dl, x, y_ut, tr(" UT  :") + QString::asprintf("%2ldh %3ldm ", minutes / 60, minutes % 60), 13.0);
  };
  const auto place = [&](const QString& name, double lon, double lat, double x, double y_name, double y_lon, double size) {
    sheet_text(dl, x, y_name, name.left(15), size);
    // ort(10,x,y), "Lä:" + grmi$ + gl$ and "Br:" + grmi$ + gg$
    sheet_text(dl, x, y_lon, tr("Lä:") + grmi_text(std::abs(lon), 1) + (lon < 0.0 ? "W" : "E"), 11.0);
    sheet_text(dl, x, y_lon + 10.0, tr("Br:") + grmi_text(std::abs(lat), 1) + (lat < 0.0 ? "S" : "N"), 11.0);
  };
  const ClassicSheetText own = classic_sheet_text();
  sheet_text(dl, 4.0, 14.0, tr("INNEN-Kreis"), 12.0);
  sheet_text(dl, 224.0, 14.0, tr("INNEN : ") + rhythm_chart_label(), 12.0);
  sheet_text(dl, 224.0, 26.0, QString::fromStdString(own.name).left(20), 12.0);
  place(QString::fromStdString(record_.place), lon_->value(), lat_->value(), 510.0, 14.0, 24.0, 12.0);
  moment(inner, 376.0, 14.0, 24.0);
  sheet_text(dl, 114.0, 14.0, tr("AUSSEN-Kreis"), 13.0);
  sheet_text(dl, 224.0, 444.0, tr("AUSSEN: ") + partner_label_, 13.0);
  const QString partner_full = partner_record_ ? name_of(*partner_record_) : partner_name_;
  sheet_text(dl, 224.0, 456.0, partner_full.left(20), 13.0);
  place(partner_place_, partner_input_.lon_deg_east, partner_input_.lat_deg, 510.0, 440.0, 450.0, 13.0);
  moment(outer, 376.0, 448.0, 458.0);
  return wheel;
}

}  // namespace horcom
