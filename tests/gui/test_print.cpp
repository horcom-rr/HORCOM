// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QCheckBox>
#include <QHeaderView>
#include <QImage>
#include <QLabel>
#include <QRegularExpression>
#include <QTableWidget>
#include <filesystem>
#include <numeric>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "probe.hpp"
#include "theme.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a synthetic birth with the ascendant in Virgo and Jupiter in the
// first house, no real person
AafRecord birth() {
  AafRecord r;
  r.surname = "TESTFALL";
  r.day = 13;
  r.month = 10;
  r.year = 1992;
  r.hour = 3;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 19;
  return r;
}

int total(const std::array<int, 5>& columns) {
  return std::accumulate(columns.begin() + 1, columns.end(), 0);
}

bool has_text(const DisplayList& dl, const std::string& t) {
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kText && p.text == t) {
      return true;
    }
  }
  return false;
}

void plain_weights(Konsta& k) {
  k.pn.fill(0);
  for (int i = 1; i <= 14; ++i) {
    k.pn[static_cast<std::size_t>(i)] = 1;
  }
  k.haus1_dop = false;
  k.gebherr_dop = false;
}

}  // namespace

TEST_CASE("the A4 page counts with his point weights and doubling switches") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  plain_weights(k);
  // the Virgo ascendant makes Chiron the ruler of the NEU, it counts with
  // the ZUSATZ weight of the point table
  k.pn[15] = 1;
  const int plain = total(MainWindowProbe::histogram(*w).element_sign);
  // GebHerr doppelt
  k.gebherr_dop = true;
  CHECK(total(MainWindowProbe::histogram(*w).element_sign) == plain + 1);
  // 1.Haus doppelt, Jupiter stands in the first house
  k.gebherr_dop = false;
  k.haus1_dop = true;
  CHECK(total(MainWindowProbe::histogram(*w).element_sign) == plain + 1);
  // a point weight of five on the Sun adds four in its element
  k.haus1_dop = false;
  k.pn[1] = 5;
  const Histogram weighted = MainWindowProbe::histogram(*w);
  CHECK(total(weighted.element_sign) == plain + 4);

  // the numbers above the columns are the counts of the sheet
  k.elem = static_cast<int>(HistogramMode::kSigns);
  const DisplayList one = MainWindowProbe::a4(*w);
  CHECK(has_text(one, "Elemente"));
  char buf[8];
  std::snprintf(buf, sizeof(buf), "%2d", weighted.element_sign[3]);
  CHECK(has_text(one, buf));
  // HISTOGRAMME WEGLASSEN leaves both blocks out
  k.elem = static_cast<int>(HistogramMode::kNone);
  const DisplayList none = MainWindowProbe::a4(*w);
  CHECK_FALSE(has_text(none, "Elemente"));
  CHECK_FALSE(has_text(none, "Kard-Fix-Ver"));
}

TEST_CASE("the Histogramme window keeps its switches for the session") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  plain_weights(k);
  k.haus1_dop = true;
  bool haus1_shown = false;
  DialogDriver drive;
  drive.then([&haus1_shown](QDialog* d) {
    for (QCheckBox* box : d->findChildren<QCheckBox*>()) {
      if (box->text().contains("1. HAUS")) {
        haus1_shown = box->isChecked();
        box->setChecked(false);
      }
      if (box->text().contains("GEBURTSHERRSCHER")) {
        box->setChecked(true);
      }
    }
    d->reject();
  });
  MainWindowProbe::histogram_view(*w);
  CHECK(drive.unexpected() == 0);
  CHECK(haus1_shown);
  CHECK_FALSE(k.haus1_dop);
  CHECK(k.gebherr_dop);
}

TEST_CASE("the Histogramme window adds the sign and the house points") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  plain_weights(MainWindowProbe::konsta(*w));
  QStringList heads;
  QStringList sums;
  DialogDriver drive;
  drive.then([&heads, &sums](QDialog* d) {
    const auto* t = d->findChild<QTableWidget*>();
    REQUIRE(t != nullptr);
    for (int c = 0; c < t->columnCount(); ++c) {
      heads << t->horizontalHeaderItem(c)->text();
    }
    // the number before each bar, sign plus house against the sum
    for (int r = 0; r < t->rowCount(); ++r) {
      const int sign = t->item(r, 1)->text().section(' ', 0, 0).toInt();
      const int house = t->item(r, 2)->text().section(' ', 0, 0).toInt();
      const int sum = t->item(r, 3)->text().section(' ', 0, 0).toInt();
      sums << QString("%1+%2=%3").arg(sign).arg(house).arg(sum);
    }
    d->reject();
  });
  MainWindowProbe::histogram_view(*w);
  CHECK(drive.pending() == 0);
  // the tester's Gesamt-Summe, TKZ + Häuser-Punkte
  CHECK(heads == QStringList{"Klasse", "Zeichen", "Häuser", "Gesamt"});
  REQUIRE(sums.size() == 7);
  for (const QString& s : sums) {
    INFO(s.toStdString());
    const QStringList parts = s.split(QRegularExpression("[+=]"));
    CHECK(parts[0].toInt() + parts[1].toInt() == parts[2].toInt());
  }
}

TEST_CASE("HOROSKOP als PNG SPEICHERN writes the sheet as a picture") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  const std::filesystem::path file = std::filesystem::temp_directory_path() / "horcom_gui_tests_sheet.png";
  std::filesystem::remove(file);
  REQUIRE(w->export_png_to(QString::fromStdWString(file.wstring())));
  const QImage image(QString::fromStdWString(file.wstring()));
  REQUIRE_FALSE(image.isNull());
  // three pixels per unit of his 640 by 480 canvas
  CHECK(image.width() == 1920);
  CHECK(image.height() == 1440);
  // the paper of the classic sheet fills the picture like the SVG, the
  // wheel drawn on it
  const QColor paper((kPaperColor >> 16) & 0xFF, (kPaperColor >> 8) & 0xFF, kPaperColor & 0xFF);
  CHECK(image.pixelColor(2, 2) == paper);
  int ink = 0;
  for (int y = 0; y < image.height(); y += 8) {
    for (int x = 0; x < image.width(); x += 8) {
      const QColor c = image.pixelColor(x, y);
      ink += (c != paper && c != QColor(Qt::white)) ? 1 : 0;
    }
  }
  CHECK(ink > 1000);
  std::filesystem::remove(file);
}

TEST_CASE("VORGABEN-ÜBERSICHT marks the captions yellow and leaves the data plain") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, birth());
  QString left;
  QString right;
  DialogDriver drive;
  drive.then([&left, &right](QDialog* d) {
    for (const QLabel* l : d->findChildren<QLabel*>()) {
      if (l->text().contains("Name :")) {
        left = l->text();
      }
      if (l->text().contains("Daten-Datei:")) {
        right = l->text();
      }
    }
    d->reject();
  });
  MainWindowProbe::vorgaben_overview(*w);
  CHECK(drive.pending() == 0);
  // the captions of the sections wear the yellow box, Name, Datum, UT,
  // HÄUSER and Ebene, the name, the date and the house system stand plain
  const ClassicSheetText sheet = MainWindowProbe::sheet(*w);
  const QString date = QString::fromStdString(sheet.date);
  const QString ut = QString::fromStdString(sheet.ut);
  CHECK(left.contains(theme::heading_span("Name :")));
  CHECK(left.contains(theme::heading_span("Datum:") + date.mid(date.indexOf(':') + 1).toHtmlEscaped()));
  CHECK(left.contains(theme::heading_span("UT:") + ut.mid(ut.indexOf(':') + 1).toHtmlEscaped()));
  CHECK(left.contains(theme::heading_span("HÄUSER :") + " PLACIDUS"));
  CHECK(left.contains(theme::heading_span("Ebene :") + " RADIX"));
  CHECK_FALSE(left.contains(theme::heading_span(QString::fromStdString(sheet.name).toHtmlEscaped())));
  CHECK_FALSE(left.contains(theme::heading_span(date.toHtmlEscaped())));
  CHECK_FALSE(right.contains(theme::heading_span("Drucker-Option AUS")));
}
