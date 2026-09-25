// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QKeyEvent>
#include <QLabel>
#include <QRegularExpression>
#include <QTableWidget>
#include <cmath>
#include <filesystem>
#include <initializer_list>
#include <optional>
#include <utility>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/chart/rhythm.hpp"
#include "horcom/core/angle.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/render/rhythm_panel.hpp"
#include "probe.hpp"
#include "wheel_widget.hpp"
#include "zodiac_cells.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a synthetic morning birth east of Greenwich, no real person
AafRecord morning_birth() {
  AafRecord r;
  r.surname = "TESTFALL";
  r.day = 10;
  r.month = 5;
  r.year = 1970;
  r.hour = 7;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lat_min = 10;
  r.lon_deg = 11;
  r.lon_min = 35;
  return r;
}

void press_space(QDialog* d) {
  QKeyEvent e(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
  QApplication::sendEvent(d, &e);
}

const DisplayList* canvas(QDialog* d) {
  const auto* w = d->findChild<WheelWidget*>();
  return w != nullptr ? &w->display_list() : nullptr;
}

QStringList texts(const DisplayList& dl) {
  QStringList out;
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kText) {
      out << QString::fromStdString(p.text);
    }
  }
  return out;
}

QStringList table_column(QDialog* d, int column) {
  QStringList out;
  const auto* t = d->findChild<QTableWidget*>();
  for (int r = 0; t != nullptr && r < t->rowCount(); ++r) {
    const QTableWidgetItem* item = t->item(r, column);
    out << (item != nullptr ? item->text() : QString());
  }
  return out;
}

}  // namespace

TEST_CASE("MÜNCHNER RHYTHMENLEHRE walks his graph phase by phase") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  QStringList first;
  QStringList titles;
  int arcs = 0;
  double name_x = 0.0;
  int phases = 0;
  int first_item = 0;
  DialogDriver drive;
  drive.then(DialogDriver::click("GRAPHIK"))
      .then([&titles](QDialog* d) {
        // the a17sol lines stand above his box
        for (const QLabel* l : d->findChildren<QLabel*>()) {
          titles << l->text();
        }
        DialogDriver::click("LINKS")(d);
      })
      .then(DialogDriver::click("DATUM"))
      .then(DialogDriver::click("EINS"))
      .then(DialogDriver::click("JAHR"))
      .then(DialogDriver::click("SIEBEN"))
      .then(DialogDriver::click("KEIN SONDERPUNKT"))
      .then([&](QDialog* d) {
        // the first phase screen offers the HARDCOPY like the others
        first_item = output_item(d);
        const DisplayList* dl = canvas(d);
        REQUIRE(dl != nullptr);
        first = texts(*dl);
        for (const Primitive& p : dl->items) {
          if (p.kind == Primitive::Kind::kLine && p.color == 0xFF0000 && p.width == 2.0) {
            ++arcs;
          }
          if (p.kind == Primitive::Kind::kText && p.text == "Name:") {
            name_x = p.x1;
          }
        }
        // twelve phases, the twelfth Space closes the screen
        for (int i = 0; i < 12 && d->isVisible(); ++i) {
          press_space(d);
          ++phases;
        }
      });
  MainWindowProbe::rhythm(*w);
  INFO(drive.titles().join(" | ").toStdString());
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  const QString sol = titles.join(QChar(0x0A));
  CHECK(sol.contains(" Auslösungen nach W.DÖBEREINER"));
  CHECK(sol.contains("  RADIX "));
  CHECK(sol.contains("RICHTUNGSSINN MARKIEREN !"));
  const QString all = first.join(QChar(0x0A));
  CHECK(all.contains("Auslösung nach DÖBEREINER:"));
  CHECK(all.contains("Periode: 7 Jahre: LINKS"));
  CHECK(all.contains("WEITER mit Leertaste"));
  // the first phase opens with the ascendant on the day of birth
  CHECK(first.contains("10. 5.70 D"));
  // a1795 arcs the first house in red
  CHECK(arcs > 0);
  // his naf& block moved beside the strip
  CHECK(name_x == doctest::Approx(222.0));
  CHECK(phases == 12);
  CHECK(first_item == menu_item::kRhythm);
}

TEST_CASE("the AUSLÖSUNGS-TABELLE lists the walk in years and months") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  QStringList when;
  QStringList art;
  DialogDriver drive;
  drive.then(DialogDriver::click("AUSLÖSUNGS"))
      .then(DialogDriver::click("RECHTS"))
      .then(DialogDriver::click("LEBENSJAHR"))
      .then(DialogDriver::click("EINS"))
      .then(DialogDriver::click("MONAT"))
      .then(DialogDriver::click("SONSTIGE"))
      .then(DialogDriver::fill({"7"}, "OK"))
      .then(DialogDriver::click("KEIN SONDERPUNKT"))
      .then([&](QDialog* d) {
        when = table_column(d, 0);
        art = table_column(d, 2);
        d->reject();
      });
  MainWindowProbe::rhythm(*w);
  INFO(drive.titles().join(" | ").toStdString());
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  REQUIRE(!when.isEmpty());
  // the rightward walk begins in the twelfth house
  CHECK(when.front() == "PHASE 1 = HS 12");
  // seven months a house, the whole walk ends before the eighth year
  const QRegularExpression age(QStringLiteral("^\\s*-?(\\d+)\\s+(\\d+\\.\\d)$"));
  int ages = 0;
  for (const QString& t : when) {
    const auto m = age.match(t);
    if (m.hasMatch()) {
      ++ages;
      CHECK(m.captured(1).toInt() <= 7);
      CHECK(m.captured(2).toDouble() < 12.0);
    }
  }
  CHECK(ages > 12);
  // his a175 table without extras named AC and MC as well
  CHECK(art.contains("D"));
  CHECK(art.contains("P"));
}

TEST_CASE("the GRAD-DATUM-LISTE dates every half degree in walk order") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  QStringList degree;
  QStringList mark;
  QStringList pair_sprites;
  bool pair_framed = false;
  bool sprite_column = false;
  DialogDriver drive;
  drive.then(DialogDriver::click("LINKS"))
      .then(DialogDriver::click("EINS"))
      .then(DialogDriver::click("JAHR"))
      .then(DialogDriver::click("SIEBEN"))
      .then([&](QDialog* d) {
        degree = table_column(d, 0);
        mark = table_column(d, 1);
        if (const auto* t = d->findChild<QTableWidget*>()) {
          sprite_column = dynamic_cast<SpriteRowDelegate*>(t->itemDelegateForColumn(1)) != nullptr;
          for (int r = 0; r < t->rowCount(); ++r) {
            const QTableWidgetItem* item = t->item(r, 1);
            if (item != nullptr && item->text() == "MA-NE") {
              pair_sprites = item->data(kSpritesRole).toStringList();
              pair_framed = item->data(kFrameRole).toBool();
            }
          }
        }
        d->reject();
      });
  MainWindowProbe::degree_date_list(*w);
  INFO(drive.titles().join(" | ").toStdString());
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  int phases = 0;
  int rows = 0;
  for (const QString& t : degree) {
    if (t.startsWith("PHASE")) {
      ++phases;
    } else {
      ++rows;
    }
  }
  CHECK(phases == 12);
  CHECK(rows == 720);
  // 4.5 degrees Aries carries Mars and Neptune
  CHECK(mark.contains("MA-NE"));
  // a174g draws the pair as his two sprites in a box
  CHECK(sprite_column);
  CHECK(pair_sprites == QStringList{QString::fromUtf8(body_glyph(body::kMars)), QString::fromUtf8(body_glyph(body::kNeptune))});
  CHECK(pair_framed);
}

TEST_CASE("the Sonderpunkt through a date lands on the walk degree and persists") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  k.fixpunkt_rh.clear();
  k.lpktg = false;
  QStringList special_rows;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSLÖSUNGS"))
        .then(DialogDriver::click("LINKS"))
        .then(DialogDriver::click("DATUM"))
        .then(DialogDriver::click("EINS"))
        .then(DialogDriver::click("JAHR"))
        .then(DialogDriver::click("SIEBEN"))
        .then([&special_rows](QDialog* d) {
          for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            special_rows << b->text();
          }
          DialogDriver::click("INDIREKT")(d);
        })
        // ten and a half years after the birth, mid house two
        .then(DialogDriver::fill({"", "10", "11", "1980", "7", "0"}, "OK"))
        .then([](QDialog* d) { d->reject(); });
    MainWindowProbe::rhythm(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(special_rows.join(QChar(0x0A)).contains("KEIN SONDERPUNKT"));
  REQUIRE(!k.fixpunkt_rh.empty());
  CHECK_FALSE(k.lpktg);
  const Chart& c = MainWindowProbe::chart(*w);
  RhythmOptions opt;
  opt.phase_years = 7.0;
  RhythmClock clock;
  clock.base_jd = c.jd_ut;
  clock.tja = c.ta.tropical_year_days;
  const double years = rhythm_years(clock, julian_day({10, 11, 1980, 7.0, 0.0}));
  CHECK(years == doctest::Approx(10.5).epsilon(0.01));
  const double expected = degree_at_age(c, opt, years) * kRadToDeg;
  CHECK(std::stod(k.fixpunkt_rh) == doctest::Approx(expected).epsilon(1e-4));

  // the next run offers the kept point first
  QStringList kept;
  DialogDriver again;
  again.then(DialogDriver::click("AUSLÖSUNGS"))
      .then(DialogDriver::click("LINKS"))
      .then(DialogDriver::click("DATUM"))
      .then(DialogDriver::click("EINS"))
      .then(DialogDriver::click("JAHR"))
      .then(DialogDriver::click("SIEBEN"))
      .then([&kept](QDialog* d) {
        for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
          kept << b->text();
        }
        DialogDriver::click("BEIBEHALTEN")(d);
      })
      .then([](QDialog* d) {
        const auto* t = d->findChild<QTableWidget*>();
        bool sp = false;
        for (int r = 0; t != nullptr && r < t->rowCount(); ++r) {
          sp = sp || (t->item(r, 1) != nullptr && t->item(r, 1)->text() == "SP");
        }
        CHECK(sp);
        d->reject();
      });
  MainWindowProbe::rhythm(*w);
  CHECK(again.pending() == 0);
  CHECK(kept.join(QChar(0x0A)).contains("BEIBEHALTEN"));
}

TEST_CASE("his ABBRUCH of the period ends the run") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  DialogDriver drive;
  //RR a17eing12 carried CASE 8 twice, ABBRUCH went on with the old period
  drive.then(DialogDriver::click("AUSLÖSUNGS"))
      .then(DialogDriver::click("LINKS"))
      .then(DialogDriver::click("DATUM"))
      .then(DialogDriver::click("EINS"))
      .then(DialogDriver::click("JAHR"))
      .then(DialogDriver::click("ABBRUCH"));
  MainWindowProbe::rhythm(*w);
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
}

TEST_CASE("an own degree joins grade.int unless it is known") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  const std::filesystem::path dir = std::filesystem::temp_directory_path() / "horcom_rhythm_grade";
  std::filesystem::create_directories(dir);
  std::filesystem::remove(dir / "grade.int");
  MainWindowProbe::set_data_dir(*w, dir);
  {
    DialogDriver drive;
    drive.then(DialogDriver::fill({"13"}, "OK"))
        .then(DialogDriver::click("OK"))   // Punkt ist NICHT von W.DÖBEREINER
        .then(DialogDriver::pick_row(4))   // MARS
        .then(DialogDriver::pick_row(6))   // SATURN
        .then(DialogDriver::click("OK"));  // PLANETEN : MA - SA
    MainWindowProbe::rhythm_define_degree(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const std::vector<CustomDegree> own = read_degrees(dir / "grade.int");
  REQUIRE(own.size() == 1);
  CHECK(own[0].degree == doctest::Approx(13.0));
  CHECK(own[0].p == body::kMars);
  CHECK(own[0].q == body::kSaturn);
  // a published degree answers BEREITS VORHANDEN
  QString note;
  {
    DialogDriver known;
    // a point under a German Windows once read 175
    known.then(DialogDriver::fill({"17.5"}, "OK")).then([&note](QDialog* d) {
      for (const QLabel* l : d->findChildren<QLabel*>()) {
        note += l->text();
      }
      d->accept();
    });
    MainWindowProbe::rhythm_define_degree(*w);
    CHECK(known.pending() == 0);
    INFO(known.titles().join(" | ").toStdString());
    INFO(note.toStdString());
    CHECK(note.contains("BEREITS VORHANDEN"));
  }
  // and all of them go on JA
  DialogDriver gone;
  gone.then(DialogDriver::click("JA"));
  MainWindowProbe::rhythm_delete_degrees(*w);
  CHECK_FALSE(std::filesystem::exists(dir / "grade.int"));
  std::filesystem::remove_all(dir);
}

TEST_CASE("SEPTAR walks his chain and the walk dates it from the offset") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  Konsta& k = MainWindowProbe::konsta(*w);
  k.fixpunkt_rh = "123.5000";
  k.lpktg = true;
  QString unit_default;
  QString message;
  QStringList special_rows;
  QStringList special_info;
  QStringList unit_info;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("JA"))  // EREIGNIS-Ort = GEBURTS-Ort ?
        .then([&unit_default, &unit_info](QDialog* d) {
          //RR Bei 'SEPTAREN' i,a. 'MONAT'
          for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            if (b->hasFocus() || b->property("default").toBool()) {
              unit_default = b->text();
            }
          }
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            unit_info << l->text();
          }
          DialogDriver::click("MONAT")(d);
        })
        .then(DialogDriver::click("SIEBEN"))
        .then(DialogDriver::fill({"30"}, "OK"))
        .then([&special_rows, &special_info](QDialog* d) {
          for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
            if (b->isEnabled()) {
              special_rows << b->text();
            }
          }
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            special_info << l->text();
          }
          DialogDriver::click("BEIBEHALTEN")(d);
        })
        .then([&message](QDialog* d) {
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            message += l->text();
          }
          d->accept();
        });
    MainWindowProbe::septar(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(unit_default == "MONAT");
  // sol$(2,ze) = "SEPTAR" heads the unit box, the numbered Septar the
  // SONDERPUNKT box, not the radix on screen
  CHECK(unit_info.join(QChar(0x0A)).contains("  SEPTAR "));
  const QString info = special_info.join(QChar(0x0A));
  CHECK(info.contains("  5.SEPTAR "));
  CHECK_FALSE(info.contains("  RADIX "));
  CHECK(info.contains("28 Bis 35  LEBENS-Jahre"));
  // the Septar box offers no date definition
  CHECK_FALSE(special_rows.join(QChar(0x0A)).contains("INDIREKT"));
  CHECK(message.contains("SEPTAR NR. 5 Gilt bei der Periode von 7 und der Zeit-Einheit MONAT"));
  CHECK(message.contains("Für die LEBENS-Jahre von  28 bis  35"));
  // the fifth Septar is the solar return of 1974, his red F on slot zero
  const Chart& c = MainWindowProbe::chart(*w);
  CHECK(MainWindowProbe::day(*w).year == 1974);
  REQUIRE(c.b[body::kFixpunkt].present);
  CHECK(c.b[body::kFixpunkt].el * kRadToDeg == doctest::Approx(123.5));

  // the walk of the Septar starts 28 years after the birth
  QStringList dates;
  QStringList points;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AUSLÖSUNGS"))
        .then(DialogDriver::click("LINKS"))
        .then(DialogDriver::click("DATUM"))
        .then(DialogDriver::click("EINS"))
        .then(DialogDriver::click("MONAT"))
        .then(DialogDriver::click("SIEBEN"))
        .then(DialogDriver::click("BEIBEHALTEN"))
        .then([&](QDialog* d) {
          dates = table_column(d, 0);
          points = table_column(d, 1);
          d->reject();
        });
    MainWindowProbe::rhythm(*w);
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const int ac = static_cast<int>(points.indexOf("AC"));
  REQUIRE(ac >= 0);
  CHECK(dates[ac] == "10. 5.98");
  CHECK(points.contains("SP"));
}

namespace {

// equal houses of thirty degrees from 10 Aries, the Sun in the first
// house and Mars, the ruler of the first phase, where the case puts it
Chart strip_chart(double mars_deg) {
  Chart c;
  c.ok = true;
  for (const auto& [slot, deg] : std::initializer_list<std::pair<int, double>>{{body::kSun, 15.0}, {body::kMars, mars_deg}}) {
    BodyState& b = c.b[static_cast<std::size_t>(slot)];
    b.present = true;
    b.valid = true;
    b.el = deg * kDegToRad;
  }
  c.houses.ok = true;
  for (int k = 1; k <= 12; ++k) {
    c.houses.cusp[static_cast<std::size_t>(k)] = (10.0 + (k - 1) * 30.0) * kDegToRad;
  }
  c.houses.cusp[13] = c.houses.cusp[1];
  c.jd_ut = julian_day({10, 5, 1970, 7, 0.0});
  return c;
}

// the ink of the strip sprite of a body left of the time axis
std::optional<Rgb> strip_ink(const DisplayList& dl, int slot) {
  for (const Primitive& p : dl.items) {
    if (p.kind == Primitive::Kind::kGlyph && p.x1 == doctest::Approx(87.0) && p.text == rhythm_glyph(slot)) {
      return p.color;
    }
  }
  return std::nullopt;
}

}  // namespace

TEST_CASE("the strip inverts a phase ruler unless it stands in the phase as well") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, morning_birth());
  RhythmOptions opt;
  opt.phase_years = 7.0;
  opt.leftward = true;
  opt.begin_house = 1;
  // Mars rules Aries on the first cusp from the fourth house, P alone
  const std::optional<Rgb> ruler = strip_ink(MainWindowProbe::rhythm_screen(*w, strip_chart(105.0), opt, 1), body::kMars);
  REQUIRE(ruler.has_value());
  CHECK(*ruler == 0xFFFFFF);
  //RR Der (die) PHASEN-HERRSCHER ist (sind) INVERS dargestellt,falls er nicht gleichzeitig direkt angetroffen wird.
  const std::optional<Rgb> direct = strip_ink(MainWindowProbe::rhythm_screen(*w, strip_chart(25.0), opt, 1), body::kMars);
  REQUIRE(direct.has_value());
  CHECK(*direct == 0x000000);
}

TEST_CASE("the Rhythmenlehre runs geocentric like CLR hrg!") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::houses(*w, 1);
  MainWindowProbe::apply(*w, morning_birth());
  MainWindowProbe::helio(*w, true);
  QStringList points;
  DialogDriver drive;
  drive.then(DialogDriver::click("AUSLÖSUNGS"))
      .then(DialogDriver::click("LINKS"))
      .then(DialogDriver::click("DATUM"))
      .then(DialogDriver::click("EINS"))
      .then(DialogDriver::click("JAHR"))
      .then(DialogDriver::click("SIEBEN"))
      .then(DialogDriver::click("KEIN SONDERPUNKT"))
      .then([&points](QDialog* d) {
        points = table_column(d, 1);
        d->reject();
      });
  MainWindowProbe::rhythm(*w);
  INFO(drive.titles().join(" | ").toStdString());
  CHECK(drive.pending() == 0);
  CHECK(drive.unexpected() == 0);
  CHECK_FALSE(MainWindowProbe::settings(*w).heliocentric);
  // the geocentric Sun rules and triggers again
  CHECK(points.contains("SO"));
}
