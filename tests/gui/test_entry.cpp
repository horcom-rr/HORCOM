// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QTableWidget>
#include <QTimer>
#include <filesystem>

#include <QPushButton>

#include "aaf_mask_dialog.hpp"
#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/collection.hpp"
#include "probe.hpp"
#include "record_mask_dialog.hpp"
#include "zeitzon_dialog.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a scratch folder per test, never the shipped data
struct Scratch {
  std::filesystem::path dir;
  explicit Scratch(const char* name) : dir(std::filesystem::temp_directory_path() / name) {
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
  }
  ~Scratch() {
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
  }
  [[nodiscard]] QString path(const char* file) const { return QString::fromStdWString((dir / file).wstring()); }
};

// the mask fields in his order, name to comment, a synthetic birth
QStringList mask_values(const QString& name, const QString& day) {
  return {name, "TESTORT", "E", "11", "34", "0", "N", "48", "8", "0", "", day, "7", "1970", "12", "30", "0", ""};
}

// fills the mask, runs the extra clicks, then presses OK
DialogDriver::Step mask(const QStringList& values, std::function<void(QDialog*)> extra = {}) {
  return [values, extra](QDialog* d) {
    const QList<QLineEdit*> edits = d->findChildren<QLineEdit*>();
    for (int i = 0; i < values.size() && i < edits.size(); ++i) {
      edits[i]->setText(values[i]);
    }
    if (extra) {
      extra(d);
    }
    DialogDriver::click("OK")(d);
  };
}

// the save picker of a2dat, the file typed in
DialogDriver::Step pick_file(const QString& path) {
  return [path](QDialog* d) {
    auto* fd = qobject_cast<QFileDialog*>(d);
    if (fd == nullptr) {
      d->reject();
      return;
    }
    // selectFile of an existing file only highlights it once the folder
    // model has loaded, the name typed into the edit is taken at once
    const QFileInfo info(path);
    fd->setDirectory(info.absolutePath());
    if (auto* name = fd->findChild<QLineEdit*>("fileNameEdit")) {
      name->setText(info.fileName());
    } else {
      fd->selectFile(path);
    }
    // QFileDialog hides accept, the QDialog slot dispatches to it
    static_cast<QDialog*>(fd)->accept();
  };
}

// reads the message text before closing the box
DialogDriver::Step read_box(QString& text) {
  return [&text](QDialog* d) {
    if (auto* box = qobject_cast<QMessageBox*>(d)) {
      text = box->text();
    } else {
      for (const QLabel* l : d->findChildren<QLabel*>()) {
        text += l->text() + "|";
      }
    }
    d->accept();
  };
}

ChartRecord stored(const char* name) {
  ChartRecord c;
  c.name = name;
  c.place = "TESTORT";
  c.day = 3;
  c.month = 3;
  c.year = 1960;
  c.hour = 8.0;
  c.lon = 11.5;
  c.lat = 48.1;
  return c;
}

}  // namespace

TEST_CASE("NEU-EINGABE saves one record into a new Daten-Datei") {
  Scratch s("horcom_entry_new");
  auto w = MainWindowProbe::make();
  const QString file = s.path("NEUTEST.DAT");
  QString note;
  {
    DialogDriver drive;
    drive.then(mask(mask_values("testfall neu", "7")))
        .then(DialogDriver::click("ABSPEICHERN"))
        .then(pick_file(file))
        .then(read_box(note));
    MainWindowProbe::new_entry(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    //RR NEU-EINGABE von DATENSÄTZEN  :   |  RADIX NR.1
    REQUIRE(!drive.titles().isEmpty());
    CHECK(drive.titles().front() == "NEU-EINGABE von DATENSÄTZEN  :   |  RADIX NR.1");
  }
  CHECK(note.contains("NEUE DATEN-DATEI NEUTEST.DAT !"));
  const auto records = read_chart_file(std::filesystem::path(file.toStdWString()));
  REQUIRE(records.has_value());
  REQUIRE(records->size() == 1);
  // the DAT name in capitals like aaf_horcom2, the clock UT without a zone
  CHECK(QString::fromStdString((*records)[0].name).trimmed() == "TESTFALL NEU");
  CHECK((*records)[0].day == 7);
  CHECK((*records)[0].hour == doctest::Approx(12.0));
  CHECK((*records)[0].minute == doctest::Approx(30.0));
  // the same file by identity, a temp path may carry 8.3 short names the
  // file dialog hands back in long form
  std::error_code ec;
  CHECK(std::filesystem::equivalent(std::filesystem::path(MainWindowProbe::data_file(*w).toStdWString()),
                                    std::filesystem::path(file.toStdWString()), ec));
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
}

TEST_CASE("NEU-EINGABE appends to a bound file and reports his byte count") {
  Scratch s("horcom_entry_append");
  const QString file = s.path("ALT.DAT");
  REQUIRE(write_chart_file(std::filesystem::path(file.toStdWString()), {stored("ALTFALL")}));
  auto w = MainWindowProbe::make();
  MainWindowProbe::bind(*w, file);
  QString note;
  {
    DialogDriver drive;
    drive.then(mask(mask_values("ZWEITFALL", "9")))
        .then(DialogDriver::click("ABSPEICHERN"))
        .then(pick_file(file))
        .then(read_box(note));
    MainWindowProbe::new_entry(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR DATEI x : n BYTE = m SÄTZE
  CHECK(note == "DATEI ALT.DAT : 256 BYTE = 2 SÄTZE");
  const auto records = read_chart_file(std::filesystem::path(file.toStdWString()));
  REQUIRE(records.has_value());
  REQUIRE(records->size() == 2);
  CHECK(QString::fromStdString((*records)[1].name).trimmed() == "ZWEITFALL");
}

TEST_CASE("a duplicate beside an AAF twin only points to the AAF file") {
  Scratch s("horcom_entry_twin");
  const QString file = s.path("PAAR.DAT");
  const std::filesystem::path dat(file.toStdWString());
  REQUIRE(write_chart_file(dat, {stored("DOPPELFALL")}));
  AafRecord twin;
  twin.surname = "DOPPELFALL";
  twin.given = "*";
  twin.day = 3;
  twin.month = 3;
  twin.year = 1960;
  twin.hour = 8;
  REQUIRE(write_aaf(aaf_twin_path(dat), {twin}));
  auto w = MainWindowProbe::make();
  QString box;
  {
    DialogDriver drive;
    drive.then(mask(mask_values("DOPPELFALL", "7")))
        .then(DialogDriver::click("ABSPEICHERN"))
        .then(pick_file(file))
        .then(read_box(box));
    MainWindowProbe::new_entry(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  INFO("box: " << box.toStdString());
  //RR DATENSATZ GLEICHEN NAMENS bereits VORHANDEN !
  CHECK(box.contains("DATENSATZ GLEICHEN NAMENS bereits VORHANDEN !"));
  CHECK(box.contains("VERÄNDERUNGEN im AAF-FILE vornehmen !"));
  const auto records = read_chart_file(dat);
  REQUIRE(records.has_value());
  CHECK(records->size() == 1);
  const auto aaf = read_aaf(aaf_twin_path(dat));
  REQUIRE(aaf.has_value());
  CHECK(aaf->size() == 1);
}

TEST_CASE("the hub save of a same named record replaces its AAF twin") {
  Scratch s("horcom_entry_hub");
  const QString file = s.path("HUB.DAT");
  const std::filesystem::path dat(file.toStdWString());
  // aaf_horcom2 drops the star of a one word name, the DAT holds HUBFALL
  REQUIRE(write_chart_file(dat, {stored("HUBFALL"), stored("ANDERER FALL")}));
  AafRecord a;
  a.surname = "HUBFALL";
  a.given = "*";
  a.day = 3;
  a.month = 3;
  a.year = 1960;
  AafRecord b = a;
  b.surname = "ANDERER";
  b.given = "FALL";
  REQUIRE(write_aaf(aaf_twin_path(dat), {a, b}));
  auto w = MainWindowProbe::make();
  MainWindowProbe::bind(*w, file);
  // the loader reads his star as an empty field
  AafRecord now = a;
  now.given.clear();
  now.day = 4;
  now.hour = 6;
  MainWindowProbe::apply(*w, now);
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("OK"));
    MainWindowProbe::save(*w);
    CHECK(drive.unexpected() == 0);
  }
  // aaf_ident found HUBFALL, the new record moved to the end and the DAT
  // followed the AAF pilot
  const auto aaf = read_aaf(aaf_twin_path(dat));
  REQUIRE(aaf.has_value());
  REQUIRE(aaf->size() == 2);
  CHECK((*aaf)[0].surname == "ANDERER");
  CHECK((*aaf)[1].surname == "HUBFALL");
  CHECK((*aaf)[1].day == 4);
  const auto records = read_chart_file(dat);
  REQUIRE(records.has_value());
  REQUIRE(records->size() == 2);
  CHECK(QString::fromStdString((*records)[1].name).trimmed() == "HUBFALL");
}

TEST_CASE("a derived chart never overwrites the birth record of its person") {
  Scratch s("horcom_entry_derived");
  const QString file = s.path("GEBURT.DAT");
  const std::filesystem::path dat(file.toStdWString());
  REQUIRE(write_chart_file(dat, {stored("SOLARFALL")}));
  AafRecord birth;
  birth.surname = "SOLARFALL";
  birth.day = 3;
  birth.month = 3;
  birth.year = 1960;
  birth.hour = 8;
  birth.zone = kUtZoneText;
  birth.lat_deg = 48;
  birth.lon_deg = 11;
  REQUIRE(write_aaf(aaf_twin_path(dat), {birth}));
  auto w = MainWindowProbe::make();
  MainWindowProbe::bind(*w, file);
  MainWindowProbe::apply(*w, birth);
  // the SOLAR row carries the person's name with its own moment
  MainWindowProbe::moment(*w, julian_day({3, 3, 2000, 12.0, 0.0}), false);
  MainWindowProbe::store_solar(*w, "SOLAR 2000");
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("OK"));
    MainWindowProbe::save(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // the first port erased the birth record of the AAF pilot and wrote the
  // SOLAR moment in its place, his IF ex! && od = 1 appends it
  const auto aaf = read_aaf(aaf_twin_path(dat));
  REQUIRE(aaf.has_value());
  REQUIRE(aaf->size() == 2);
  CHECK((*aaf)[0].year == 1960);
  CHECK((*aaf)[1].surname == "SOLARFALL");
  const auto records = read_chart_file(dat);
  REQUIRE(records.has_value());
  REQUIRE(records->size() == 2);
  CHECK((*records)[0].year == 1960);
  CHECK((*records)[1].year == 2000);
}

TEST_CASE("a derived chart without AAF twin joins the file without the overwrite question") {
  Scratch s("horcom_entry_derived_plain");
  const QString file = s.path("GEBURT.DAT");
  const std::filesystem::path dat(file.toStdWString());
  REQUIRE(write_chart_file(dat, {stored("SOLARFALL")}));
  auto w = MainWindowProbe::make();
  MainWindowProbe::bind(*w, file);
  AafRecord birth = aaf_from_chart_record(stored("SOLARFALL"));
  MainWindowProbe::apply(*w, birth);
  MainWindowProbe::moment(*w, julian_day({3, 3, 2000, 12.0, 0.0}), false);
  MainWindowProbe::store_solar(*w, "SOLAR 2000");
  QStringList seen;
  {
    DialogDriver drive;
    drive.then([&seen](QDialog* d) {
      seen << d->windowTitle();
      DialogDriver::click("OK")(d);
    });
    MainWindowProbe::save(*w);
    CHECK(drive.unexpected() == 0);
  }
  // the first port offered DATENSATZ GLEICHEN NAMENS ... ÜBERSCHREIBEN ?
  CHECK(seen == QStringList{"HORCOM"});
  const auto records = read_chart_file(dat);
  REQUIRE(records.has_value());
  REQUIRE(records->size() == 2);
  CHECK((*records)[0].year == 1960);
}

TEST_CASE("a one word name finds its own record in the file") {
  Scratch s("horcom_entry_oneword");
  const QString file = s.path("EINZEL.DAT");
  const std::filesystem::path dat(file.toStdWString());
  REQUIRE(write_chart_file(dat, {stored("EINWORT")}));
  auto w = MainWindowProbe::make();
  MainWindowProbe::bind(*w, file);
  const auto loaded = read_chart_file(dat);
  REQUIRE(loaded.has_value());
  MainWindowProbe::apply(*w, aaf_from_chart_record(loaded->front()));
  QString question;
  {
    DialogDriver drive;
    drive
        .then([&question](QDialog* d) {
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            question += l->text();
          }
          DialogDriver::click("ÜBERSCHREIBEN")(d);
        })
        .then(DialogDriver::click("OK"));
    MainWindowProbe::save(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // the first port stored EINWORT * beside EINWORT without asking
  CHECK(question.contains("DATENSATZ GLEICHEN NAMENS"));
  const auto records = read_chart_file(dat);
  REQUIRE(records.has_value());
  REQUIRE(records->size() == 1);
  CHECK(QString::fromStdString((*records)[0].name).trimmed() == "EINWORT");
}

TEST_CASE("LMT from the zone list is mean local time without a question") {
  auto w = MainWindowProbe::make();
  AafRecord r = aaf_from_chart_record(stored("ALTFALL"));
  r.year = 1792;
  MainWindowProbe::apply(*w, r);
  {
    DialogDriver drive;
    drive.then([](QDialog* d) {
      auto* table = d->findChild<QTableWidget*>();
      for (int row = 0; table != nullptr && row < table->rowCount(); ++row) {
        if (table->item(row, 1)->text() == "LMT") {
          emit table->cellDoubleClicked(row, 0);
          return;
        }
      }
      d->reject();
    });
    MainWindowProbe::pick_zone(*w);
    CHECK(drive.pending() == 0);
    // the first port treated LMT like LTT, before 1810 with the equation of
    // time and his WAHRE Ortszeit box
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::clock(*w) == ClockKind::kMeanLocal);
  // the zone box shows two decimals, the clock itself runs on the exact
  // longitude and the record stores 46 minutes for 11.5 degrees east
  CHECK(MainWindowProbe::zone(*w)->value() == doctest::Approx(11.5 / 15.0).epsilon(0.01));
  CHECK(MainWindowProbe::record(*w).zone == "00hE46:00");
}

TEST_CASE("a new Daten-Datei needs a loaded record like a2dat") {
  Scratch s("horcom_entry_newfile");
  auto w = MainWindowProbe::make();
  QString note;
  {
    DialogDriver drive;
    drive.then(pick_file(s.path("LEER"))).then(read_box(note));
    MainWindowProbe::data_file_io(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR KEIN Datensatz!  ERST EINGEBEN !
  CHECK(note == "KEIN Datensatz!  ERST EINGEBEN !");
  CHECK_FALSE(std::filesystem::exists(s.dir / "LEER.DAT"));
  CHECK(MainWindowProbe::data_file(*w).isEmpty());
  MainWindowProbe::apply(*w, aaf_from_chart_record(stored("ERSTFALL")));
  {
    DialogDriver drive;
    drive.then(pick_file(s.path("ERSTE"))).then(read_box(note));
    MainWindowProbe::data_file_io(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // the typed name gets the extension of the file kind
  CHECK(note == "NEUE DATEN-DATEI ERSTE.DAT !");
  const auto records = read_chart_file(s.dir / "ERSTE.DAT");
  REQUIRE(records.has_value());
  CHECK(records->size() == 1);
}

TEST_CASE("LÖSCHEN with the AAF records kept leaves the deletion in place") {
  Scratch s("horcom_entry_delete_kept");
  const QString file = s.path("DREI.DAT");
  const std::filesystem::path dat(file.toStdWString());
  std::vector<ChartRecord> recs = {stored("ERSTER FALL"), stored("ZWEITER FALL"), stored("DRITTER FALL")};
  REQUIRE(write_chart_file(dat, recs));
  std::vector<AafRecord> twin;
  for (const ChartRecord& c : recs) {
    twin.push_back(aaf_from_chart_record(c));
  }
  REQUIRE(write_aaf(aaf_twin_path(dat), twin));
  auto w = MainWindowProbe::make();
  MainWindowProbe::bind(*w, file);
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("BEIBEHALTEN"))
        .then([](QDialog* d) {
          auto* list = d->findChild<QListWidget*>();
          for (int r = 0; list != nullptr && r < list->count(); ++r) {
            if (list->item(r)->text().contains("ZWEITER")) {
              list->item(r)->setSelected(true);
              emit list->itemClicked(list->item(r));
            }
          }
          DialogDriver::click("WAHL - ENDE")(d);
        })
        .then(DialogDriver::click("LÖSCHEN ?"))
        .then(DialogDriver::click("LÖSCHEN Beenden"));
    MainWindowProbe::delete_from_file(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // his aaf_horcom2 rebuilt the DAT from the untouched AAF and brought
  // ZWEITER FALL back, three records
  const auto left = read_chart_file(dat);
  REQUIRE(left.has_value());
  CHECK(left->size() == 2);
  const auto aaf = read_aaf(aaf_twin_path(dat));
  REQUIRE(aaf.has_value());
  CHECK(aaf->size() == 3);
}

namespace {

QPushButton* button_of(QDialog& d, const QString& caption) {
  for (QPushButton* b : d.findChildren<QPushButton*>()) {
    if (b->text() == caption) {
      return b;
    }
  }
  return nullptr;
}

// the AAF box fields in creation order, name to zone and summer time
void fill_aaf(AafMaskDialog& box, const QStringList& values) {
  const QList<QLineEdit*> edits = box.findChildren<QLineEdit*>();
  for (int i = 0; i < values.size() && i < edits.size(); ++i) {
    if (!values[i].isNull()) {
      edits[i]->setText(values[i]);
    }
  }
}

}  // namespace

TEST_CASE("OK = Speichern writes the AAF file and rebuilds its DAT twin") {
  Scratch s("horcom_aaf_save");
  AafMaskDialog box(AafRecord{}, AafMaskDialog::Mode::kEntry, s.dir);
  box.set_aaf_file(s.dir / "NEU.AAF");
  //   surname, given, kind, day, month, year, hour, min, sec, place, land,
  //   jd, N/S, lat, min, sec, E/W, lon, min, sec, zone, summer
  fill_aaf(box, {"Speicherfall", "Anna", "w", "7", "7", "1970", "12", "30", "0", "Testort", "D", "", "N", "48",
                 "0", "0", "E", "11", "0", "0", "01hE00:00", "0"});
  QPushButton* save = button_of(box, "OK = Speichern");
  REQUIRE(save != nullptr);
  QString note;
  {
    DialogDriver drive;
    drive.then(pick_file(s.path("NEU.AAF"))).then(read_box(note));
    save->click();
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // the first port saved nothing behind his OK = Speichern
  //RR Neue AAF-Datei ... !
  CHECK(note == "Neue AAF-Datei NEU.AAF !");
  CHECK(box.result() == QDialog::Accepted);
  REQUIRE(box.saved_file().has_value());
  const auto aaf = read_aaf(s.dir / "NEU.AAF");
  REQUIRE(aaf.has_value());
  REQUIRE(aaf->size() == 1);
  CHECK((*aaf)[0].surname == "Speicherfall");
  CHECK((*aaf)[0].jd > 0.0);
  const auto dat = read_chart_file(dat_twin_path(s.dir / "NEU.AAF"));
  REQUIRE(dat.has_value());
  REQUIRE(dat->size() == 1);
  CHECK(QString::fromStdString((*dat)[0].name).trimmed() == "SPEICHERFALL ANNA");
  // 12:30 MEZ is 11:30 UT
  CHECK((*dat)[0].hour == doctest::Approx(11.0));

  // the same name again asks his overwrite question
  AafRecord again = (*aaf)[0];
  again.hour = 14;
  again.jd = 0.0;
  AafMaskDialog second(again, AafMaskDialog::Mode::kEdit, s.dir);
  second.set_aaf_file(s.dir / "NEU.AAF");
  QString question;
  {
    DialogDriver drive;
    drive.then(pick_file(s.path("NEU.AAF"))).then([&question](QDialog* d) {
      for (const QLabel* l : d->findChildren<QLabel*>()) {
        question += l->text() + "|";
      }
      DialogDriver::click("ÜBERSCHREIBEN")(d);
    });
    button_of(second, "OK = Speichern")->click();
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR Datensatz gleichen Namens bereits vorhanden !
  CHECK(question.contains("Datensatz gleichen Namens"));
  const auto replaced = read_aaf(s.dir / "NEU.AAF");
  REQUIRE(replaced.has_value());
  REQUIRE(replaced->size() == 1);
  CHECK((*replaced)[0].hour == 14);
  const auto rebuilt = read_chart_file(dat_twin_path(s.dir / "NEU.AAF"));
  REQUIRE(rebuilt.has_value());
  CHECK((*rebuilt)[0].hour == doctest::Approx(13.0));
}

TEST_CASE("OK = Speichern without a zone keeps the AAF box open") {
  Scratch s("horcom_aaf_nozone");
  AafMaskDialog box(AafRecord{}, AafMaskDialog::Mode::kEntry, s.dir);
  fill_aaf(box, {"Zonenlos", "", "", "7", "7", "1970", "12", "30", "0"});
  QString note;
  {
    DialogDriver drive;
    drive.then(read_box(note));
    button_of(box, "OK = Speichern")->click();
    CHECK(drive.pending() == 0);
  }
  //RR Das wichtige Feld 'Zone' FEHLT noch !!
  CHECK(note == " Das wichtige Feld 'Zone' FEHLT noch !!");
  CHECK(box.result() != QDialog::Accepted);
  CHECK_FALSE(box.saved_file().has_value());
}

TEST_CASE("a fresh AAF entry opens without the before Christ year") {
  AafMaskDialog box(AafRecord{}, AafMaskDialog::Mode::kEntry, HORCOM_TEST_DATA_DIR);
  const QList<QLineEdit*> edits = box.findChildren<QLineEdit*>();
  REQUIRE(edits.size() > 5);
  CHECK(edits[5]->text().isEmpty());
  edits[3]->setText("7");
  edits[4]->setText("7");
  edits[5]->setText("1970");
  // the first port preset year 0 as 1 with the V flag, 1970 became -1969
  CHECK(box.record().year == 1970);
  // his astronomical year with the kal$ note beside it
  AafRecord old;
  old.day = 15;
  old.month = 3;
  old.year = -43;
  AafMaskDialog antique(old, AafMaskDialog::Mode::kShow, HORCOM_TEST_DATA_DIR);
  CHECK(antique.findChildren<QLineEdit*>()[5]->text() == "-43");
  bool note = false;
  for (const QLabel* l : antique.findChildren<QLabel*>()) {
    note = note || l->text() == " = 44 Vor Christus";
  }
  CHECK(note);
  CHECK(antique.record().year == -43);
}

TEST_CASE("DATEIEN VERKETTEN trims like VERKETTEN BEENDEN") {
  Scratch s("horcom_entry_chain");
  ChartRecord empty = stored("OHNE ORT");
  empty.lon = 0.0;
  empty.lat = 0.0;
  REQUIRE(write_chart_file(s.dir / "A.DAT", {stored("ERSTER FALL"), empty, stored("ZWEITER FALL")}));
  auto w = MainWindowProbe::make();
  QString note;
  {
    DialogDriver drive;
    drive.then(pick_file(s.path("A.DAT"))).then(pick_file(s.path("KETTE"))).then(read_box(note));
    MainWindowProbe::chain_files(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(note == "2 Datensätze verkettet.");
  const auto chain = read_aaf(s.dir / "KETTE.AAF");
  REQUIRE(chain.has_value());
  REQUIRE(chain->size() == 2);
  CHECK((*chain)[1].surname == "ZWEITER");
}

TEST_CASE("DATUM FEHLT keeps the typed fields and NUR ÜBERNEHMEN writes nothing") {
  auto w = MainWindowProbe::make();
  QString kept;
  {
    DialogDriver drive;
    drive.then(mask(mask_values("OHNE DATUM", "")))
        .then(DialogDriver::click("OK"))
        .then([&kept](QDialog* d) {
          kept = d->findChildren<QLineEdit*>().front()->text();
          mask(mask_values("OHNE DATUM", "5"))(d);
        })
        .then(DialogDriver::click("NUR als Datensatz"));
    MainWindowProbe::new_entry(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    REQUIRE(drive.titles().size() == 4);
  }
  CHECK(kept == "OHNE DATUM");
  CHECK(MainWindowProbe::data_file(*w).isEmpty());
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  CHECK(MainWindowProbe::slot(*w, 0)->surname == "OHNE DATUM");
  CHECK(MainWindowProbe::slot(*w, 0)->day == 5);
}

TEST_CASE("Weiter EDITIEREN returns to the mask with the entry") {
  auto w = MainWindowProbe::make();
  QString again;
  {
    DialogDriver drive;
    drive.then(mask(mask_values("EDITFALL", "5")))
        .then(DialogDriver::click("Weiter EDITIEREN"))
        .then([&again](QDialog* d) {
          again = d->findChildren<QLineEdit*>().front()->text();
          DialogDriver::click("ABBRUCH")(d);
        });
    MainWindowProbe::new_entry(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(again == "EDITFALL");
  CHECK_FALSE(MainWindowProbe::slot(*w, 0).has_value());
}

TEST_CASE("the zone box of the entry mask turns the clock into UT") {
  auto w = MainWindowProbe::make();
  QString zzd;
  {
    DialogDriver drive;
    // the zone box opens from the mask, the tick waits for the step to
    // return so the driver sees the nested box, the mask then comes back
    drive.then([](QDialog* d) {
          const QList<QLineEdit*> edits = d->findChildren<QLineEdit*>();
          const QStringList values = mask_values("ZONENFALL", "7");
          for (int i = 0; i < values.size() && i < edits.size(); ++i) {
            edits[i]->setText(values[i]);
          }
          for (QCheckBox* c : d->findChildren<QCheckBox*>()) {
            if (c->text().startsWith("MEZ")) {
              QTimer::singleShot(0, c, [c]() { c->click(); });
            }
          }
        })
        .then([](QDialog* d) {
          // MEZ is the preset, DSZ adds the summer hour
          for (QCheckBox* c : d->findChildren<QCheckBox*>()) {
            if (c->text().startsWith("EINFACHE")) {
              c->click();
            }
          }
          DialogDriver::click("OK")(d);
        })
        .then([&zzd](QDialog* d) {
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            if (l->text().startsWith("Zonenzeitdiff")) {
              zzd = l->text();
            }
          }
          DialogDriver::click("OK")(d);
        })
        .then(DialogDriver::click("NUR als Datensatz"));
    MainWindowProbe::new_entry(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR Zonenzeitdiff ZZD =
  CHECK(zzd == "Zonenzeitdiff ZZD = -2");
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  const AafRecord r = *MainWindowProbe::slot(*w, 0);
  CHECK(r.zone == "01hE00:00");
  CHECK(r.dst == "1");
  // 12:30 MESZ is 10:30 UT
  const CalendarDate ut = calendar_date(aaf_moment_jd_ut(r), Calendar::kAuto);
  CHECK(ut.hour == doctest::Approx(10.0));
  CHECK(ut.minute == doctest::Approx(30.0).epsilon(0.001));
}

TEST_CASE("ORTSZEIT in the entry mask takes the longitude as zone") {
  auto w = MainWindowProbe::make();
  QStringList values = mask_values("ORTSFALL", "7");
  values[13] = "1850";
  {
    DialogDriver drive;
    drive.then(mask(values,
                    [](QDialog* d) {
                      for (QCheckBox* c : d->findChildren<QCheckBox*>()) {
                        if (c->text().startsWith("ORTSZEIT")) {
                          c->click();
                        }
                      }
                    }))
        .then(DialogDriver::click("MITTLERE"))
        .then(DialogDriver::click("NUR als Datensatz"));
    MainWindowProbe::new_entry(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  const AafRecord r = *MainWindowProbe::slot(*w, 0);
  // 11 degrees 34 minutes east is 46 minutes 16 seconds of mean local time
  const CalendarDate ut = calendar_date(aaf_moment_jd_ut(r), Calendar::kAuto);
  const double minutes = ut.hour * 60.0 + ut.minute;
  CHECK(minutes == doctest::Approx(12.5 * 60.0 - (11.0 + 34.0 / 60.0) * 4.0).epsilon(0.0001));
}

TEST_CASE("the zone box adds the summer hours to any chosen zone") {
  ZeitzonDialog box(HORCOM_TEST_DATA_DIR);
  CHECK(box.zzd() == doctest::Approx(-1.0));
  const QList<QListWidget*> lists = box.findChildren<QListWidget*>();
  REQUIRE(lists.size() == 2);
  QListWidget* europa = lists[0];
  QListWidgetItem* athen = nullptr;
  for (int i = 0; i < europa->count(); ++i) {
    if (europa->item(i)->text().startsWith("Athen")) {
      athen = europa->item(i);
    }
  }
  REQUIRE(athen != nullptr);
  emit europa->itemClicked(athen);
  CHECK(box.zzd() == doctest::Approx(-2.0));
  QCheckBox* dsz = nullptr;
  QCheckBox* ddsz = nullptr;
  for (QCheckBox* c : box.findChildren<QCheckBox*>()) {
    if (c->text().startsWith("EINFACHE")) {
      dsz = c;
    } else if (c->text().startsWith("DOPPELTE")) {
      ddsz = c;
    }
  }
  REQUIRE(dsz != nullptr);
  REQUIRE(ddsz != nullptr);
  // his flat -2 held only for MEZ, Athens with DSZ is three hours ahead
  dsz->click();
  CHECK(box.zzd() == doctest::Approx(-3.0));
  CHECK(box.summer() == 1);
  ddsz->click();
  CHECK_FALSE(dsz->isChecked());
  CHECK(box.zzd() == doctest::Approx(-4.0));
  CHECK(box.summer() == 2);
  ddsz->click();
  CHECK(box.zzd() == doctest::Approx(-2.0));
  CHECK(box.summer() == 0);
}
