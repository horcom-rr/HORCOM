// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QCheckBox>
#include <numeric>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "probe.hpp"

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
  const int plain = total(MainWindowProbe::histogram(*w).element_sign);
  // GebHerr doppelt, the Virgo ascendant makes Mercury the ruler
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
