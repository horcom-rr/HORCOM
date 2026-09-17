// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "aspektarium_dialog.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <cmath>
#include <vector>

#include "horcom/chart/bodies.hpp"
#include "theme.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"

namespace horcom {

namespace {

// the aspect glyphs behind his asps& bitmaps, the DEFAULT branch of
// aspar prints divisor and rounded angle instead
QString symbol_text(int nm, double w_deg) {
  switch (nm) {
    case 1: return QString::fromUtf8("\xE2\x98\x8C");
    case 2: return QString::fromUtf8("\xE2\x98\x8D");
    case 3: return QString::fromUtf8("\xE2\x96\xB3");
    case 4: return QString::fromUtf8("\xE2\x96\xA1");
    case 5: return QStringLiteral("Q");
    case 6: return QString::fromUtf8("\xE2\x9A\xB9");
    case 8: return QString::fromUtf8("\xE2\x88\xA0");
    case 12: return QString::fromUtf8("\xE2\x9A\xBA");
    case 17: return QStringLiteral("bQ");
    case 18: return QString::fromUtf8("\xE2\x9A\xBB");
    case 19: return QString::fromUtf8("\xE2\x9A\xBC");
    default: return QString::fromUtf8("%1 %2\xC2\xB0").arg(nm).arg(qRound(w_deg));
  }
}

}  // namespace

AspektariumDialog::AspektariumDialog(const Chart& chart, const ChartSettings& s, const AspectSettings& a, const QString& record, QWidget* parent)
    : QDialog(parent), chart_(chart), s_(s), base_(a) {
  // his horgt ladder names the mode
  QString mode = tr("Geozentrisches");
  if (s_.heliocentric) {
    mode = tr("Heliozentrisches");
  } else if (s_.topocentric_parallax) {
    mode = tr("Topozentrisches");
  }
  QString title = mode + " " + tr("Aspektarium");
  if (!record.isEmpty()) {
    title += "  |  " + record;
  }
  setWindowTitle(title);

  auto* v = new QVBoxLayout(this);
  auto* top = new QHBoxLayout();
  //RR MAXIMALER TEILER ?
  top->addWidget(new QLabel(tr("Maximaler Teiler"), this));
  divisors_ = new QComboBox(this);
  divisors_->addItem("8", 8);
  divisors_->addItem("12", 12);
  // sixteen only outside the equal probability orbs like his alertbox
  if (!base_.equal_probability) {
    divisors_->addItem("16", 16);
  }
  const int preset = divisors_->findData(base_.divisors);
  divisors_->setCurrentIndex(preset >= 0 ? preset : 1);
  top->addWidget(divisors_);
  //RR Orbis-Faktor
  auto* orb_label = new QLabel(tr("Orbis-Faktor = %1%").arg(qRound(100.0 * base_.orb)), this);
  top->addWidget(orb_label, 1, Qt::AlignRight);
  v->addLayout(top);

  auto* middle = new QHBoxLayout();
  matrix_ = new QTableWidget(this);
  matrix_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  matrix_->setSelectionMode(QAbstractItemView::NoSelection);
  matrix_->verticalHeader()->setDefaultSectionSize(24);
  matrix_->horizontalHeader()->setDefaultSectionSize(58);
  middle->addWidget(matrix_, 1);
  legend_ = new QTableWidget(this);
  legend_->setColumnCount(3);
  //RR Teiler Winkel Orbis
  legend_->setHorizontalHeaderLabels({tr("Teiler"), tr("Winkel"), tr("Orbis")});
  legend_->verticalHeader()->setVisible(false);
  legend_->verticalHeader()->setDefaultSectionSize(20);
  legend_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  legend_->setSelectionMode(QAbstractItemView::NoSelection);
  legend_->setColumnWidth(0, 56);
  legend_->setColumnWidth(1, 88);
  legend_->setColumnWidth(2, 80);
  legend_->setFixedWidth(248);
  middle->addWidget(legend_);
  v->addLayout(middle, 1);

  //RR Planeten-Gewichtung:
  weights_ = new QLabel(this);
  weights_->setObjectName("aspectsLine");
  weights_->setWordWrap(true);
  v->addWidget(weights_);

  connect(divisors_, &QComboBox::currentIndexChanged, this, [this](int) { rebuild(); });
  rebuild();
  resize(1120, 640);
}

void AspektariumDialog::rebuild() {
  AspectSettings a = base_;
  a.divisors = divisors_->currentData().toInt();
  const AspectResult res = scan_aspects(chart_, s_, a);

  // the visible slots in the original order, the earth wears TE on the
  // moon's row in the hrg mode
  const bool helio = !chart_.b[body::kSun].present && chart_.b[body::kMoon].present;
  std::vector<int> shown;
  QStringList tags;
  for (int slot = 0; slot < body::kSlotCount; ++slot) {
    const BodyState& b = chart_.b[static_cast<std::size_t>(slot)];
    if (!b.present || !b.valid) {
      continue;
    }
    shown.push_back(slot);
    const std::string_view tag = (helio && slot == body::kMoon) ? body::kEarthName
                                                                : body::kName[static_cast<std::size_t>(slot)];
    tags << QString::fromUtf8(tag.data(), static_cast<int>(tag.size()));
  }
  const int n = static_cast<int>(shown.size());
  matrix_->clear();
  matrix_->setRowCount(n);
  matrix_->setColumnCount(n);
  matrix_->setHorizontalHeaderLabels(tags);
  matrix_->setVerticalHeaderLabels(tags);
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      const std::size_t o = static_cast<std::size_t>(shown[static_cast<std::size_t>(i < j ? i : j)]);
      const std::size_t m = static_cast<std::size_t>(shown[static_cast<std::size_t>(i < j ? j : i)]);
      auto* item = new QTableWidgetItem();
      item->setTextAlignment(Qt::AlignCenter);
      if (i == j) {
        // the diagonal carries the per body hit count az&
        item->setText(QString::number(res.az[o]));
        item->setForeground(QColor(0xD4, 0xA9, 0x4A));
      } else if (double w = res.asp[o][m]; i < j) {
        //RR nur diskrete Aspekte
        if (w > kEps) {
          double di = std::abs(chart_.b[o].el - chart_.b[m].el);
          if (di > kPi && di < kTwoPi) {
            di = kTwoPi - di;
          }
          const double deg = di * kRadToDeg;
          item->setText(QString::fromUtf8("%1\xC2\xB0%2'")
                            .arg(static_cast<int>(deg))
                            .arg(qRound(60.0 * (deg - static_cast<int>(deg))), 2, 10, QChar('0')));
        }
      } else if (w > 0.36) {
        // aspar folds to the near side, then his noise guard
        if (w > kPi && w < kTwoPi) {
          w = kTwoPi - w;
        }
        item->setText(symbol_text(aspect_symbol(w, a.divisors), w * kRadToDeg));
      }
      matrix_->setItem(i, j, item);
    }
  }

  legend_->setRowCount(a.divisors);
  for (int t = 1; t <= a.divisors; ++t) {
    const double pn = kTwoPi / t;
    //RR ORBES nach HORCOM- Zählung, oder selbst definiert
    const double dd = a.equal_probability ? a.orb * a.orbe[static_cast<std::size_t>(t)] : a.orb * pn / 30.0;
    legend_->setItem(t - 1, 0, new QTableWidgetItem(QString::number(t)));
    legend_->setItem(t - 1, 1, new QTableWidgetItem(QString::fromUtf8("%1\xC2\xB0").arg(360.0 / t, 0, 'f', 1)));
    legend_->setItem(t - 1, 2, new QTableWidgetItem(QString::fromUtf8("%1\xC2\xB0").arg(dd * kRadToDeg, 0, 'f', 1)));
  }

  QString wtext = theme::heading_span(tr("Planeten-Gewichtung")) + "&nbsp; ";
  for (int i = 0; i < n; ++i) {
    const int slot = shown[static_cast<std::size_t>(i)];
    if (slot > body::kMc) {
      break;
    }
    wtext += QString("%1 %2&nbsp; ").arg(tags[i]).arg(base_.weight[static_cast<std::size_t>(slot)]);
  }
  if (n > 0 && shown.back() > body::kMc) {
    //RR Zusatz-Planeten:
    wtext += tr("Zusatz %1%").arg(base_.weight[body::kApogee]);
  }
  weights_->setText(wtext);
}

}  // namespace horcom
