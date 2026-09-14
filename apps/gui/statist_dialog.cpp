// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "statist_dialog.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <set>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

constexpr const char* kObjectName[14] = {"Sonne", "Mond",   "Merkur", "Venus", "Mars", "Jupiter", "Saturn",
                                         "Uranus", "Neptun", "Pluto",  "Knoten", "Südknoten", "AC", "MC"};

constexpr const char* kSignTag[12] = {"AR", "TA", "GM", "CN", "LE", "VI", "LI", "SC", "SG", "CP", "AQ", "PS"};

// operand combos encode kind and value in one item id
constexpr int kCuspBase = 100;
constexpr int kRulerBase = 200;

QString slot_label(int slot) {
  const std::string_view tag = body::kTag[static_cast<std::size_t>(slot)];
  return QString::fromUtf8(tag.data(), static_cast<int>(tag.size()));
}

}  // namespace

StatistDialog::StatistDialog(const AspectSettings& aspects, QWidget* parent) : QDialog(parent), aspects_(aspects) {
  setWindowTitle(tr("Statistik"));
  auto* v = new QVBoxLayout(this);
  auto* top = new QHBoxLayout();
  object_ = new QComboBox(this);
  for (int i = 0; i < 14; ++i) {
    object_->addItem(kObjectName[i], i + 1);
  }
  count_ = new QLabel(this);
  top->addWidget(new QLabel(tr("Objekt"), this));
  top->addWidget(object_);
  top->addWidget(count_, 1);
  table_ = new QTableWidget(0, 3, this);
  table_->setHorizontalHeaderLabels({tr("Name"), tr("Datum"), tr("Ort")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->verticalHeader()->setDefaultSectionSize(20);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  distribution_ = new QLabel(this);
  distribution_->setObjectName("aspectsLine");
  distribution_->setWordWrap(true);

  //RR AUSWERTUNG NACH VORGEWÄHLTEN SUCH-BEDINGUNGEN
  auto* eval = new QGridLayout();
  eval_object_ = new QComboBox(this);
  eval_object_->addItem(tr("Planet/Punkt"), static_cast<int>(StatObject::kBody));
  eval_object_->addItem(tr("Lichter (so mo ac)"), static_cast<int>(StatObject::kLights));
  eval_object_->addItem(tr("Alle Planeten"), static_cast<int>(StatObject::kAllBodies));
  eval_object_->addItem(tr("Herr von Haus"), static_cast<int>(StatObject::kHouseRuler));
  eval_object_->addItem(tr("Halbsumme"), static_cast<int>(StatObject::kMidpoint));
  eval_object_->addItem(tr("Aspekt"), static_cast<int>(StatObject::kAspect));
  eval_object_->addItem(tr("Aspekt zur Halbsumme"), static_cast<int>(StatObject::kMidpointAspect));
  eval_object_->addItem(tr("Spiegelpunkt"), static_cast<int>(StatObject::kMirror));
  eval_object_->addItem(tr("Name"), static_cast<int>(StatObject::kName));
  eval_object_->addItem(tr("Arabischer Punkt"), static_cast<int>(StatObject::kArabicPart));
  op_a_ = new QComboBox(this);
  op_b_ = new QComboBox(this);
  op_c_ = new QComboBox(this);
  mirror_ = new QComboBox(this);
  //RR die spg Wahl
  mirror_->addItem("0° AR", static_cast<int>(MirrorAxis::kAriesLibra));
  mirror_->addItem("0° CN", static_cast<int>(MirrorAxis::kCancerCapricorn));
  mirror_->addItem(tr("beide"), static_cast<int>(MirrorAxis::kBoth));
  mirror_->setCurrentIndex(2);
  name_ = new QLineEdit(this);
  name_->setPlaceholderText(tr("Namensteil"));
  house_ = new QSpinBox(this);
  house_->setRange(1, 12);
  eval->addWidget(new QLabel(tr("Objekt"), this), 0, 0);
  eval->addWidget(eval_object_, 0, 1);
  eval->addWidget(op_a_, 0, 2);
  eval->addWidget(op_b_, 0, 3);
  eval->addWidget(op_c_, 0, 4);
  eval->addWidget(house_, 0, 5);
  eval->addWidget(mirror_, 0, 6);
  eval->addWidget(name_, 0, 7);

  window_ = new QComboBox(this);
  //RR bei GRAD / in ZEICHEN / in HAUS / bei PL / 0-360
  window_->addItem(tr("bei Grad"), static_cast<int>(StatWindow::kAtDegree));
  window_->addItem(tr("in Zeichen"), static_cast<int>(StatWindow::kInSign));
  window_->addItem(tr("in Haus"), static_cast<int>(StatWindow::kInHouse));
  window_->addItem(tr("bei Planet/Punkt"), static_cast<int>(StatWindow::kNearBody));
  window_->addItem(QString::fromUtf8("0-360°"), static_cast<int>(StatWindow::kAnywhere));
  window_->setCurrentIndex(4);
  degree_ = new QDoubleSpinBox(this);
  degree_->setRange(0.0, 360.0);
  degree_->setDecimals(1);
  degree_->setSuffix(QString::fromUtf8("°"));
  orb_ = new QDoubleSpinBox(this);
  orb_->setRange(0.0, 10.0);
  orb_->setDecimals(1);
  orb_->setValue(1.0);
  orb_->setPrefix(QString::fromUtf8("± "));
  orb_->setSuffix(QString::fromUtf8("°"));
  sign_ = new QComboBox(this);
  for (int i = 0; i < 12; ++i) {
    sign_->addItem(kSignTag[i], i + 1);
  }
  house_pct_ = new QSpinBox(this);
  house_pct_->setRange(0, 10);
  house_pct_->setSuffix(tr("% Orbis"));
  near_ = new QComboBox(this);
  asp_low_ = new QSpinBox(this);
  asp_low_->setRange(1, 16);
  asp_low_->setPrefix(tr("Teiler "));
  asp_high_ = new QSpinBox(this);
  asp_high_->setRange(1, 16);
  asp_high_->setValue(12);
  asp_high_->setPrefix(tr("bis "));
  asp_orb_ = new QDoubleSpinBox(this);
  asp_orb_->setRange(0.0, 12.0);
  asp_orb_->setDecimals(1);
  asp_orb_->setSpecialValueText(tr("Orbis-Tabelle"));
  asp_orb_->setSuffix(QString::fromUtf8("°"));
  eval->setColumnStretch(8, 1);
  eval->addWidget(new QLabel(tr("Bedingung"), this), 1, 0);
  eval->addWidget(window_, 1, 1);
  eval->addWidget(degree_, 1, 2);
  eval->addWidget(sign_, 1, 2);
  eval->addWidget(near_, 1, 2);
  eval->addWidget(orb_, 1, 3);
  eval->addWidget(house_pct_, 1, 3);
  eval->addWidget(asp_low_, 1, 4);
  eval->addWidget(asp_high_, 1, 5);
  eval->addWidget(asp_orb_, 1, 6);

  und_ = new QCheckBox(tr("UND mit voriger Bedingung"), this);
  und_->setEnabled(false);
  apply_ = new QPushButton(tr("Bedingung anwenden"), this);
  auto* reset = new QPushButton(tr("Zurücksetzen"), this);
  //RR der Zähler kann wahlweise auch automatisch die gesamte Datei durchzählen
  auto* count_all = new QPushButton(tr("Datei durchzählen"), this);
  eval_count_ = new QLabel(this);
  auto* actions = new QHBoxLayout();
  actions->addWidget(und_);
  actions->addWidget(apply_);
  actions->addWidget(reset);
  actions->addWidget(count_all);
  actions->addWidget(eval_count_, 1);
  eval_dist_ = new QLabel(this);
  eval_dist_->setObjectName("aspectsLine");
  eval_dist_->setWordWrap(true);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  v->addLayout(top);
  v->addWidget(table_, 1);
  v->addWidget(distribution_);
  v->addLayout(eval);
  v->addLayout(actions);
  v->addWidget(eval_dist_);
  v->addWidget(buttons);

  connect(object_, &QComboBox::currentIndexChanged, this, [this](int) { refresh_distribution(); });
  connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { accept_row(row); });
  connect(buttons, &QDialogButtonBox::accepted, this, [this]() { accept_row(table_->currentRow()); });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(eval_object_, &QComboBox::currentIndexChanged, this, [this](int) { update_eval_fields(); });
  connect(window_, &QComboBox::currentIndexChanged, this, [this](int) { update_eval_fields(); });
  connect(apply_, &QPushButton::clicked, this, &StatistDialog::apply_condition);
  connect(reset, &QPushButton::clicked, this, &StatistDialog::reset_conditions);
  connect(count_all, &QPushButton::clicked, this, &StatistDialog::count_file);
  update_eval_fields();
  resize(760, 720);
}

bool StatistDialog::load(const QString& sta) {
  const auto set = load_statistics(std::filesystem::path(sta.toStdWString()));
  if (!set) {
    return false;
  }
  load_set(*set);
  return true;
}

void StatistDialog::load_set(StatSet set) {
  set_ = std::move(set);
  table_->setRowCount(0);
  for (std::size_t i = 0; i < set_.records.size(); ++i) {
    const StatRecord& r = set_.records[i];
    const int row = table_->rowCount();
    table_->insertRow(row);
    auto* item = new QTableWidgetItem(QString::fromStdString(r.name));
    item->setData(Qt::UserRole, static_cast<qulonglong>(i));
    table_->setItem(row, 0, item);
    table_->setItem(row, 1, new QTableWidgetItem(QString::asprintf("%02d.%02d.%d", r.day, r.month, r.year)));
    table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(r.place)));
  }
  count_->setText(tr("%1 Datensätze").arg(set_.records.size()));
  if (table_->rowCount() > 0) {
    table_->selectRow(0);
  }
  table_->resizeColumnToContents(0);
  table_->resizeColumnToContents(1);
  for (QComboBox* combo : {op_a_, op_b_, op_c_, near_}) {
    fill_operand(combo);
  }
  reset_conditions();
  refresh_distribution();
}

void StatistDialog::fill_operand(QComboBox* combo) const {
  combo->clear();
  for (int slot = 1; slot <= 14; ++slot) {
    combo->addItem(slot_label(slot), slot);
  }
  for (int e = 1; e <= 22; ++e) {
    const int nk = set_.params.nk[static_cast<std::size_t>(e)];
    if (nk > 18) {
      combo->addItem(slot_label(nk), nk);
    }
  }
  const bool helio = !set_.records.empty() && set_.records[0].heliocentric();
  if (!helio) {
    for (int h = 1; h <= 12; ++h) {
      combo->addItem(tr("Spitze H%1").arg(h), kCuspBase + h);
    }
    for (int h = 1; h <= 12; ++h) {
      combo->addItem(tr("Herr H%1").arg(h), kRulerBase + h);
    }
  }
}

StatOperand StatistDialog::operand_from(const QComboBox* combo) const {
  StatOperand op;
  const int id = combo->currentData().toInt();
  if (id >= kRulerBase) {
    op.kind = StatOperand::Kind::kRuler;
    op.house = id - kRulerBase;
  } else if (id >= kCuspBase) {
    op.kind = StatOperand::Kind::kCusp;
    op.house = id - kCuspBase;
  } else {
    op.body = id;
  }
  return op;
}

void StatistDialog::update_eval_fields() {
  const auto obj = static_cast<StatObject>(eval_object_->currentData().toInt());
  const bool aspect = obj == StatObject::kAspect || obj == StatObject::kMidpointAspect;
  const bool windowed = !aspect && obj != StatObject::kName;
  op_a_->setVisible(obj == StatObject::kBody || obj == StatObject::kMidpoint || aspect ||
                    obj == StatObject::kMirror || obj == StatObject::kArabicPart);
  op_b_->setVisible(obj == StatObject::kMidpoint || aspect || obj == StatObject::kArabicPart);
  op_c_->setVisible(obj == StatObject::kMidpointAspect || obj == StatObject::kArabicPart);
  house_->setVisible(obj == StatObject::kHouseRuler);
  mirror_->setVisible(obj == StatObject::kMirror);
  name_->setVisible(obj == StatObject::kName);
  window_->setVisible(windowed);
  const auto win = static_cast<StatWindow>(window_->currentData().toInt());
  degree_->setVisible(windowed && win == StatWindow::kAtDegree);
  sign_->setVisible(windowed && win == StatWindow::kInSign);
  near_->setVisible(windowed && win == StatWindow::kNearBody);
  orb_->setVisible(windowed && (win == StatWindow::kAtDegree || win == StatWindow::kInSign || win == StatWindow::kNearBody));
  house_pct_->setVisible(windowed && win == StatWindow::kInHouse);
  asp_low_->setVisible(aspect);
  asp_high_->setVisible(aspect);
  asp_orb_->setVisible(aspect);
  // the house window carries its own house number field
  if (windowed && win == StatWindow::kInHouse && obj != StatObject::kHouseRuler) {
    house_->setVisible(true);
  }
}

void StatistDialog::apply_condition() {
  if (set_.records.empty()) {
    return;
  }
  StatQuery q;
  q.object = static_cast<StatObject>(eval_object_->currentData().toInt());
  q.a = operand_from(op_a_);
  q.b = operand_from(op_b_);
  q.c = operand_from(op_c_);
  if (q.object == StatObject::kHouseRuler) {
    q.a.house = house_->value();
  }
  const bool aspect = q.object == StatObject::kAspect || q.object == StatObject::kMidpointAspect;
  q.window = (aspect || q.object == StatObject::kName) ? StatWindow::kAnywhere
                                                       : static_cast<StatWindow>(window_->currentData().toInt());
  q.degree = kDegToRad * degree_->value();
  q.orb = q.window == StatWindow::kInSign ? orb_->value() : kDegToRad * orb_->value();
  q.sign = sign_->currentData().toInt();
  q.house = house_->value();
  q.house_orb_pct = house_pct_->value();
  q.near_body = operand_from(near_);
  q.asp_low = asp_low_->value();
  q.asp_high = asp_high_->value();
  q.asp_orb = kDegToRad * asp_orb_->value();
  q.mirror = static_cast<MirrorAxis>(mirror_->currentData().toInt());
  q.name = name_->text().toUpper().toStdString();
  q.combine_and = und_->isChecked();
  const StatEvalResult res = evaluate_statistics(set_, q, aspects_, mask_);
  ++conditions_;
  und_->setEnabled(true);

  std::set<int> hit;
  for (const StatMatch& m : res.matches) {
    hit.insert(m.record);
  }
  for (int row = 0; row < table_->rowCount(); ++row) {
    const int idx = static_cast<int>(table_->item(row, 0)->data(Qt::UserRole).toULongLong());
    table_->setRowHidden(row, hit.find(idx) == hit.end());
  }
  eval_count_->setText(tr("%1 Treffer in %2 von %3 Datensätzen")
                           .arg(res.matches.size())
                           .arg(hit.size())
                           .arg(set_.records.size()));
  if (res.distribution[0] > 0) {
    const bool houses = q.window == StatWindow::kInHouse;
    QString text = houses ? tr("<span style='color:#D4A94A'>HÄUSER</span>&nbsp; ")
                          : tr("<span style='color:#D4A94A'>ZEICHEN</span>&nbsp; ");
    for (int i = 1; i <= 12; ++i) {
      if (i > 1) {
        text += "  ";
      }
      const QString label = houses ? QString::number(i) : QString(kSignTag[i - 1]);
      text += QString("%1 %2").arg(label).arg(res.distribution[static_cast<std::size_t>(i)]);
    }
    eval_dist_->setText(text);
  } else {
    eval_dist_->clear();
  }
}

void StatistDialog::reset_conditions() {
  mask_.assign(set_.records.size(), 0.0);
  conditions_ = 0;
  und_->setChecked(false);
  und_->setEnabled(false);
  for (int row = 0; row < table_->rowCount(); ++row) {
    table_->setRowHidden(row, false);
  }
  eval_count_->clear();
  eval_dist_->clear();
}

void StatistDialog::refresh_distribution() {
  const int object = object_->currentData().toInt();
  std::array<int, 12> bins{};
  int have = 0;
  for (const StatRecord& r : set_.records) {
    double el = 0.0;
    if (object == 13) {
      el = r.ac;
    } else if (object == 14) {
      el = r.mc;
    } else {
      el = r.el[static_cast<std::size_t>(object)];
    }
    if (el == 0.0) {
      continue;
    }
    const int sign = static_cast<int>(norm_deg(el * kRadToDeg) / kDegPerSign) % 12;
    ++bins[static_cast<std::size_t>(sign)];
    ++have;
  }
  QString text = tr("<span style='color:#D4A94A'>ZEICHEN</span>&nbsp; ");
  for (int i = 0; i < 12; ++i) {
    if (i > 0) {
      text += "  ";
    }
    text += QString("%1 %2").arg(kSignTag[i]).arg(bins[static_cast<std::size_t>(i)]);
  }
  text += tr("&nbsp; (%1 belegt)").arg(have);
  distribution_->setText(text);
}

void StatistDialog::accept_row(int row) {
  if (row < 0 || row >= table_->rowCount()) {
    return;
  }
  const auto idx = table_->item(row, 0)->data(Qt::UserRole).toULongLong();
  chosen_ = set_.records[static_cast<std::size_t>(idx)];
  accept();
}

// ported from the dataset walk of the Schiemenz counters, every stored
// chart rebuilt from its packed angles and run through the scanners
void StatistDialog::count_file() {
  if (set_.records.empty()) {
    return;
  }
  QApplication::setOverrideCursor(Qt::WaitCursor);
  long long asp = 0, triga = 0, gt = 0, mid = 0;
  ChartSettings s;
  s.enable_standard_extras();
  for (const StatRecord& r : set_.records) {
    Chart c;
    c.ok = true;
    for (int slot = 0; slot < body::kSlotCount; ++slot) {
      const double el = r.el[static_cast<std::size_t>(slot)];
      if (el != 0.0) {
        BodyState& b = c.b[static_cast<std::size_t>(slot)];
        b.present = true;
        b.valid = true;
        b.el = el;
      }
    }
    if (!r.heliocentric()) {
      const std::array<double, 14> fz = stat_houses(r);
      c.houses.ok = true;
      c.houses.cusp = fz;
      c.b[body::kAscendant] = {true, true, r.ac};
      c.b[body::kMc] = {true, true, r.mc};
    }
    const AspectResult a = scan_aspects(c, s, aspects_);
    const MidpointResult m = scan_midpoints(c, s, aspects_);
    asp += static_cast<long long>(a.hits.size());
    triga += a.triga;
    gt += a.grand_trines;
    mid += m.direct + m.square + m.semi;
  }
  QApplication::restoreOverrideCursor();
  const double n = static_cast<double>(set_.records.size());
  eval_dist_->setText(tr("Datei durchgezählt, %1 Datensätze. Aspekte %2 (Mittel %3), Triga %4, Großtrigone %5, Halbsummen %6 (Mittel %7).")
                          .arg(set_.records.size())
                          .arg(asp)
                          .arg(asp / n, 0, 'f', 2)
                          .arg(triga)
                          .arg(gt)
                          .arg(mid)
                          .arg(mid / n, 0, 'f', 2));
}

}  // namespace horcom
