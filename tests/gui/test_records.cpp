// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPushButton>
#include <filesystem>

#include "auto_advance.hpp"
#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/core/constants.hpp"
#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/konsta.hpp"
#include "probe.hpp"
#include "record_list_dialog.hpp"
#include "record_mask_dialog.hpp"
#include "wheel_widget.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// synthetic births, no real persons
AafRecord person(const char* surname, int day) {
  AafRecord r;
  r.surname = surname;
  r.day = day;
  r.month = 4;
  r.year = 1975;
  r.hour = 9;
  r.zone = "00hE00:00";
  r.lat_deg = 48;
  r.lon_deg = 11;
  return r;
}

QPushButton* button(QDialog& d, const QString& caption) {
  for (QPushButton* b : d.findChildren<QPushButton*>()) {
    if (b->text() == caption) {
      return b;
    }
  }
  return nullptr;
}

}  // namespace

TEST_CASE("NUR HOROSKOP ZEIGEN browses without taking a record") {
  const std::vector<AafRecord> records = {person("ERSTER", 1), person("ZWEITER", 2)};
  RecordListDialog list(records, {0, 1}, "TEST.DAT", 5, RecordListDialog::Mode::kFetch);
  std::vector<std::string> shown;
  list.set_preview([&shown](const AafRecord& r) { shown.push_back(r.surname); });
  QPushButton* show = button(list, "NUR HOROSKOP ZEIGEN");
  REQUIRE(show != nullptr);
  {
    // nothing marked yet, his fanz asks for a single click
    DialogDriver drive;
    drive.then(DialogDriver::click("OK"));
    show->click();
    CHECK(drive.pending() == 0);
    CHECK(shown.empty());
  }
  auto* rows = list.findChild<QListWidget*>();
  REQUIRE(rows != nullptr);
  emit rows->itemClicked(rows->item(1));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("OK = WEITER"));
    show->click();
    CHECK(drive.pending() == 0);
  }
  REQUIRE(shown.size() == 1);
  CHECK(shown[0] == "ZWEITER");
  // the list is a browser now, no record can be taken
  CHECK_FALSE(button(list, "WAHL - ENDE")->isEnabled());
  CHECK_FALSE(button(list, "DRUCKEN")->isEnabled());
  emit rows->itemClicked(rows->item(0));
  show->click();
  REQUIRE(shown.size() == 2);
  CHECK(shown[1] == "ERSTER");
  CHECK(list.picked().empty());
}

TEST_CASE("the chart only window draws the record and Space leads back") {
  auto w = MainWindowProbe::make();
  bool drawn = false;
  QString title;
  {
    DialogDriver drive;
    drive.then([&drawn, &title](QDialog* d) {
      title = d->windowTitle();
      if (auto* wheel = d->findChild<WheelWidget*>()) {
        drawn = !wheel->display_list().items.empty();
      }
      QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
      QApplication::sendEvent(d, &space);
    });
    MainWindowProbe::preview(*w, person("VORSCHAU", 5));
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(title == " Nur HOROSKOP Ansehen | Weiter mit LEERTASTE | BEENDEN mit 'EXIT' ");
  CHECK(drawn);
  // the panel record stays untouched
  CHECK(MainWindowProbe::record(*w).surname != "VORSCHAU");
}

TEST_CASE("AUFRÄUMEN asks before it empties the slots") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, person("BLEIBT", 3));
  REQUIRE(MainWindowProbe::slot(*w, 0).has_value());
  {
    DialogDriver drive;
    drive.then([](QDialog* d) {
      auto* box = qobject_cast<QMessageBox*>(d);
      REQUIRE(box != nullptr);
      //RR DATEN und GESPEICHERTE BILDER dieser Sitzung LÖSCHEN ?
      CHECK(box->text() == "DATEN und GESPEICHERTE BILDER dieser Sitzung LÖSCHEN ?");
      CHECK(box->windowTitle() == "RÜCKSETZEN ?");
      box->button(QMessageBox::Cancel)->click();
    });
    MainWindowProbe::clear_slots(*w);
  }
  CHECK(MainWindowProbe::slot(*w, 0).has_value());
  {
    DialogDriver drive;
    drive.then([](QDialog* d) { qobject_cast<QMessageBox*>(d)->button(QMessageBox::Ok)->click(); });
    MainWindowProbe::clear_slots(*w);
  }
  CHECK_FALSE(MainWindowProbe::slot(*w, 0).has_value());
}

TEST_CASE("a SATZ row of the menu opens the EINGABE- und ANZEIGE-BOX like a4") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::put_slot(*w, 0, person("ERSTFALL", 3));
  MainWindowProbe::put_slot(*w, 1, person("ZWEITFALL", 5));
  {
    DialogDriver drive;
    bool mask = false;
    drive.then([&mask](QDialog* d) {
      mask = qobject_cast<RecordMaskDialog*>(d) != nullptr;
      d->reject();
    });
    // a4 with direkt and eingabe, the box of the clicked slot
    MainWindowProbe::slot_action(*w, 1)->trigger();
    CHECK(drive.pending() == 0);
    CHECK(mask);
  }
  // the slot is the chart whatever the box answered
  CHECK(MainWindowProbe::active_slot(*w) == 1);
  CHECK(MainWindowProbe::record(*w).surname == "ZWEITFALL");
  {
    // inside the paired sessions his direkt went on into a12, a13 or a14,
    // the SATZ click there opens no box
    DialogDriver drive;
    MainWindowProbe::session_click(*w, 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::active_slot(*w) == 0);
}

TEST_CASE("a DOPPEL-DATEN row names its pair and brings it back") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, person("INNENPERSON", 3));
  MainWindowProbe::remember_double(*w, 2, person("AUSSENPERSON", 7));
  QAction* row = MainWindowProbe::double_action(*w, 2);
  REQUIRE(row != nullptr);
  //RR LEFT$(na$(od0,ze0),10) + "-" + LEFT$(na$(od2,zf2),10)
  CHECK(row->text() == "DOPPEL-KREIS: INNENPERSO-AUSSENPERS ");
  // another chart in the panel, the row still recalls its own pair
  MainWindowProbe::apply(*w, person("DRITTE", 9));
  row->trigger();
  CHECK(MainWindowProbe::record(*w).surname == "INNENPERSON");
  CHECK(MainWindowProbe::compare_on(*w));
  CHECK(MainWindowProbe::partner_name(*w) == "AUSSENPERSON");
}

namespace {

DialogDriver::Step open_path(const std::filesystem::path& file) {
  return [file](QDialog* d) {
    auto* fd = qobject_cast<QFileDialog*>(d);
    if (fd == nullptr) {
      d->reject();
      return;
    }
    const QFileInfo info(QString::fromStdWString(file.wstring()));
    fd->setDirectory(info.absolutePath());
    if (auto* name = fd->findChild<QLineEdit*>("fileNameEdit")) {
      name->setText(info.fileName());
    }
    static_cast<QDialog*>(fd)->accept();
  };
}

}  // namespace

TEST_CASE("Wie umwandeln imports an AAF file into its DAT twin") {
  const auto dir = std::filesystem::temp_directory_path() / "horcom_convert_in";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  const auto aaf = dir / "PAAR.AAF";
  REQUIRE(write_aaf(aaf, {person("ERSTER", 1), person("ZWEITER", 2)}));
  auto w = MainWindowProbe::make();
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("AAF - in HORCOM"))
        .then(open_path(aaf))
        .then(DialogDriver::click("OK"));
    MainWindowProbe::aaf_convert(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const auto dat = read_chart_file(dat_twin_path(aaf));
  REQUIRE(dat.has_value());
  CHECK(dat->size() == 2);
  std::filesystem::remove_all(dir);
}

TEST_CASE("Wie umwandeln exports a DAT file and renames the pair on request") {
  const auto dir = std::filesystem::temp_directory_path() / "horcom_convert_out";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  const auto dat = dir / "ALT.DAT";
  ChartRecord c;
  c.name = "EXPORTFALL";
  c.day = 3;
  c.month = 3;
  c.year = 1960;
  c.hour = 8.0;
  c.lon = 11.5;
  c.lat = 48.1;
  c.remark = "(JULIAN.) ALTER VERMERK";
  REQUIRE(write_chart_file(dat, {c}));
  // an original AAF of the same name already exists
  REQUIRE(write_aaf(aaf_twin_path(dat), {person("ORIGINAL", 1)}));
  auto w = MainWindowProbe::make();
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("HORCOM - in AAF"))
        .then(DialogDriver::click("TROTZDEM WEITER"))
        .then(open_path(dat))
        .then(DialogDriver::click("JA = Name"))
        .then(DialogDriver::fill({"NEUNAME"}, "OK"))
        .then(DialogDriver::click("OK"));
    MainWindowProbe::aaf_convert(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // the original AAF stays untouched, the pair moved to the new name
  const auto original = read_aaf(aaf_twin_path(dat));
  REQUIRE(original.has_value());
  CHECK((*original)[0].surname == "ORIGINAL");
  CHECK_FALSE(std::filesystem::exists(dat));
  const auto moved = read_chart_file(dir / "NEUNAME.DAT");
  REQUIRE(moved.has_value());
  const auto out = read_aaf(dir / "NEUNAME.AAF");
  REQUIRE(out.has_value());
  REQUIRE(out->size() == 1);
  CHECK((*out)[0].surname == "EXPORTFALL");
  // the calendar flag becomes the j of the year, the remark keeps the rest
  CHECK((*out)[0].calendar == Calendar::kJulian);
  CHECK((*out)[0].comment == "ALTER VERMERK");
  //RR LSET goo$ = "NICHT GENANNT !"
  CHECK((*out)[0].place == "NICHT GENANNT !");
  std::filesystem::remove_all(dir);
}

TEST_CASE("the direction defaults reach konsta.int when the walk ends like kon_dsp") {
  auto w = MainWindowProbe::make();
  const std::filesystem::path file = MainWindowProbe::konsta_file(*w);
  std::filesystem::remove(file);
  MainWindowProbe::konsta(*w).zwhd = false;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("ZWISCHENHÄUSERN")).then(DialogDriver::click("MIT")).then(DialogDriver::click("EXIT"));
    MainWindowProbe::vorgaben_direktionen(*w);
    CHECK(drive.pending() == 0);
  }
  const auto saved = load_konsta(file);
  REQUIRE(saved.has_value());
  CHECK(saved->zwhd);
  std::filesystem::remove(file);
}

TEST_CASE("a full field hands on the focus like AUTOMATISCH WEITERSCHALTEN") {
  set_auto_advance(true);
  RecordMaskDialog mask(AafRecord{}, "NEU-EINGABE", RecordMaskDialog::Mode::kEntry);
  mask.show();
  QApplication::processEvents();
  const QList<QLineEdit*> edits = mask.findChildren<QLineEdit*>();
  QLineEdit* day = edits[11];
  QLineEdit* month = edits[12];
  QLineEdit* year = edits[13];
  QLineEdit* hour = edits[14];
  day->setFocus();
  day->setText("12");
  emit day->textEdited("12");
  CHECK(mask.focusWidget() == month);
  // a year completes with four digits like his eingl|
  year->setFocus();
  year->setText("197");
  emit year->textEdited("197");
  CHECK(mask.focusWidget() == year);
  year->setText("1970");
  emit year->textEdited("1970");
  CHECK(mask.focusWidget() == hour);
  // a field entered with the mouse keeps the focus, his mousestop!
  QMouseEvent click(QEvent::MouseButtonPress, QPointF(2, 2), QPointF(2, 2), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  QApplication::sendEvent(month, &click);
  month->setFocus();
  month->setText("07");
  emit month->textEdited("07");
  CHECK(mask.focusWidget() == month);
  mask.hide();
}

TEST_CASE("VORGABEN EIN-AUSGABE ÄNDERN walks his three topics") {
  auto w = MainWindowProbe::make();
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("EINGABE-MODUS"))
        .then(DialogDriver::click("MIT TABSTOP"))
        .then(DialogDriver::click("NEIN"))
        .then(DialogDriver::click("GANZ-SEITE"));
    MainWindowProbe::vorgaben_ein_ausgabe(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(MainWindowProbe::konsta(*w).tabstop == 1);
  CHECK(MainWindowProbe::konsta(*w).prenbl == 0);
  CHECK(MainWindowProbe::konsta(*w).halbs == 0);
  CHECK_FALSE(auto_advance());
  set_auto_advance(true);
}

TEST_CASE("a double click past the pick limit takes no further record") {
  const std::vector<AafRecord> records = {person("ERSTER", 1), person("ZWEITER", 2)};
  RecordListDialog list(records, {0, 1}, "TEST.DAT", 1, RecordListDialog::Mode::kFetch);
  auto* rows = list.findChild<QListWidget*>();
  REQUIRE(rows != nullptr);
  rows->item(0)->setSelected(true);
  emit rows->itemClicked(rows->item(0));
  emit rows->itemDoubleClicked(rows->item(1));
  // the first port added the second record past the one free slot
  CHECK(list.picked() == std::vector<std::size_t>{0});
  CHECK(list.result() == QDialog::Accepted);
}

TEST_CASE("the chart browser takes no record on a double click") {
  const std::vector<AafRecord> records = {person("ERSTER", 1), person("ZWEITER", 2)};
  RecordListDialog list(records, {0, 1}, "TEST.DAT", 5, RecordListDialog::Mode::kFetch);
  list.set_preview([](const AafRecord&) {});
  auto* rows = list.findChild<QListWidget*>();
  REQUIRE(rows != nullptr);
  emit rows->itemClicked(rows->item(0));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("OK = WEITER"));
    button(list, "NUR HOROSKOP ZEIGEN")->click();
    CHECK(drive.pending() == 0);
  }
  emit rows->itemDoubleClicked(rows->item(1));
  CHECK(list.picked().empty());
  CHECK(list.result() != QDialog::Accepted);
  // EXIT in the browser asks his Zurück in EINGABE ?, JA takes records again
  {
    DialogDriver drive;
    drive.then(DialogDriver::click(" JA "));
    button(list, "EXIT")->click();
    CHECK(drive.pending() == 0);
  }
  CHECK(button(list, "WAHL - ENDE")->isEnabled());
  CHECK(list.result() != QDialog::Accepted);
}

TEST_CASE("ESC asks before the chooser leaves the EIN-AUSGABE") {
  const std::vector<AafRecord> records = {person("ERSTER", 1)};
  RecordListDialog list(records, {0}, "TEST.DAT", 5, RecordListDialog::Mode::kFetch);
  int finished = 0;
  QObject::connect(&list, &QDialog::finished, [&finished]() { ++finished; });
  QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
  QString title;
  {
    DialogDriver drive;
    drive.then([&title](QDialog* d) {
      for (const QLabel* l : d->findChildren<QLabel*>()) {
        title += l->text();
      }
      DialogDriver::click("NEIN")(d);
    });
    QApplication::sendEvent(&list, &esc);
    CHECK(drive.pending() == 0);
  }
  //RR EIN-AUSGABE verlassen ?
  CHECK(title == "EIN-AUSGABE verlassen ?");
  CHECK(finished == 0);
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("JA"));
    QApplication::sendEvent(&list, &esc);
    CHECK(drive.pending() == 0);
  }
  CHECK(finished == 1);
}

TEST_CASE("a minus before the year counts before Christ like zuo") {
  AafRecord r = person("ANTIKE", 15);
  RecordMaskDialog mask(r, "ANZEIGE", RecordMaskDialog::Mode::kShow);
  const QList<QLineEdit*> edits = mask.findChildren<QLineEdit*>();
  edits[10]->setText("-");
  edits[13]->setText("44");
  // the first port read only a V and kept 44
  CHECK(mask.record().year == -43);
  edits[10]->setText("v");
  CHECK(mask.record().year == -43);
}

TEST_CASE("a clock off UT reads UHRZEIT whatever the zone looks like") {
  AafRecord r = person("HALBZONE", 15);
  r.zone = "00hE30:00";
  RecordMaskDialog mask(r, "ANZEIGE", RecordMaskDialog::Mode::kShow);
  QStringList labels;
  for (const QLabel* l : mask.findChildren<QLabel*>()) {
    labels << l->text();
  }
  // atof read 00hE30:00 as zero, the first port labelled it WZ = GMT = UT
  CHECK(labels.contains("UHRZEIT : hh"));
  CHECK_FALSE(labels.contains("WZ = GMT = UT : hh"));
  RecordMaskDialog plain(person("UT", 15), "ANZEIGE", RecordMaskDialog::Mode::kShow);
  QStringList plain_labels;
  for (const QLabel* l : plain.findChildren<QLabel*>()) {
    plain_labels << l->text();
  }
  CHECK(plain_labels.contains("WZ = GMT = UT : hh"));
}

TEST_CASE("an edited ANZEIGE box asks his ABSPEICHERN question") {
  const auto dir = std::filesystem::temp_directory_path() / "horcom_mask_edit";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  {
    RecordMaskDialog mask(person("EDITFALL", 15), "ANZEIGE", RecordMaskDialog::Mode::kShow);
    mask.bind_file("X.DAT", dir / "X.AAF");
    // an untouched record is taken at once
    button(mask, "OK")->click();
    CHECK(mask.result() == QDialog::Accepted);
  }
  {
    RecordMaskDialog mask(person("EDITFALL", 15), "ANZEIGE", RecordMaskDialog::Mode::kShow);
    mask.bind_file("X.DAT", dir / "X.AAF");
    mask.findChildren<QLineEdit*>()[11]->setText("16");
    QString question;
    DialogDriver drive;
    drive.then([&question](QDialog* d) {
      for (const QLabel* l : d->findChildren<QLabel*>()) {
        question += l->text() + "|";
      }
      DialogDriver::click("NUR als Datensatz")(d);
    });
    button(mask, "OK")->click();
    CHECK(drive.pending() == 0);
    CHECK(question.contains("Datensatz In Datei X.DAT ABSPEICHERN ?"));
    CHECK(mask.result() == QDialog::Accepted);
    CHECK_FALSE(mask.save_requested());
    CHECK(mask.record().day == 16);
  }
  REQUIRE(write_aaf(dir / "X.AAF", {person("EDITFALL", 15)}));
  {
    RecordMaskDialog mask(person("EDITFALL", 15), "ANZEIGE", RecordMaskDialog::Mode::kShow);
    mask.bind_file("X.DAT", dir / "X.AAF");
    mask.findChildren<QLineEdit*>()[11]->setText("16");
    QString warning;
    DialogDriver drive;
    drive
        .then([&warning](QDialog* d) {
          for (const QLabel* l : d->findChildren<QLabel*>()) {
            warning += l->text() + "|";
          }
          DialogDriver::click("TROTZDEM WEITER")(d);
        })
        .then(DialogDriver::click("In Datei"));
    button(mask, "OK")->click();
    CHECK(drive.pending() == 0);
    //RR Änderungen nur im AAF-File vornehmen !
    CHECK(warning.contains("Änderungen nur im AAF-File vornehmen !"));
    CHECK(mask.save_requested());
  }
  {
    // ZURÜCK sets the fields back and stays in the box
    RecordMaskDialog mask(person("EDITFALL", 15), "ANZEIGE", RecordMaskDialog::Mode::kShow);
    mask.bind_file("X.DAT", dir / "X.AAF");
    mask.findChildren<QLineEdit*>()[11]->setText("16");
    DialogDriver drive;
    drive.then(DialogDriver::click("ZURÜCK"));
    button(mask, "OK")->click();
    CHECK(drive.pending() == 0);
    CHECK(mask.result() != QDialog::Accepted);
    CHECK(mask.findChildren<QLineEdit*>()[11]->text() == "15");
  }
  std::filesystem::remove_all(dir);
}

TEST_CASE("AUFRÄUMEN stops the running clock like areg11") {
  auto w = MainWindowProbe::make();
  MainWindowProbe::apply(*w, person("UHRFALL", 3));
  MainWindowProbe::clock_in_slot(*w, 0);
  {
    DialogDriver drive;
    drive.then([](QDialog* d) { qobject_cast<QMessageBox*>(d)->button(QMessageBox::Ok)->click(); });
    MainWindowProbe::clear_slots(*w);
  }
  // the first port emptied the slots and left the clock running
  CHECK_FALSE(MainWindowProbe::uhr_on(*w));
  CHECK(MainWindowProbe::uhr_slot(*w) == -1);
}

TEST_CASE("a slot that gives way takes its own clock and its person's derived charts") {
  auto w = MainWindowProbe::make();
  const char* names[5] = {"AFALL", "BFALL", "CFALL", "DFALL", "EFALL"};
  for (int i = 0; i < 5; ++i) {
    MainWindowProbe::put_slot(*w, i, person(names[i], 1 + i));
  }
  MainWindowProbe::activate_slot(*w, 2, person(names[2], 3));
  // the derived chart of CFALL lands in the SOLAR row of its radix, his
  // sol$(2,ze), row 3 with index 2
  MainWindowProbe::store_solar(*w, "SOLAR CFALL");
  REQUIRE(MainWindowProbe::solar_slot(*w, 2).has_value());
  MainWindowProbe::clock_in_slot(*w, 4);
  const auto claim = [&w](int zmsp) {
    DialogDriver drive;
    drive.then(DialogDriver::click("Weiter"));
    const int slot = MainWindowProbe::claim_slot(*w, zmsp);
    CHECK(drive.pending() == 0);
    return slot;
  };
  // his zeuhr = 5 test stopped the clock of the fifth slot for any slot
  CHECK(claim(0) == 0);
  CHECK(MainWindowProbe::uhr_slot(*w) == 4);
  // AFALL gives way, its column empties, the row of CFALL stays
  CHECK(MainWindowProbe::solar_slot(*w, 2).has_value());
  MainWindowProbe::clock_in_slot(*w, 2);
  CHECK(claim(2) == 2);
  // his clock in slot 3 went on writing into the new record
  CHECK(MainWindowProbe::uhr_slot(*w) == -1);
  // dats_l empties the SOLAR row of the column that gives way
  CHECK_FALSE(MainWindowProbe::solar_slot(*w, 2).has_value());
}

TEST_CASE("kon_dsp keeps the self defined orbs and the outer colour") {
  auto w = MainWindowProbe::make();
  const std::filesystem::path file = MainWindowProbe::konsta_file(*w);
  std::filesystem::remove(file);
  MainWindowProbe::aspect_settings(*w).orbe[3] = 4.25 * kDegToRad;
  MainWindowProbe::outer_color(*w) = 3;
  MainWindowProbe::persist_konsta(*w);
  const auto saved = load_konsta(file);
  REQUIRE(saved.has_value());
  CHECK(saved->orb_text[3] == " 4.25");
  CHECK(saved->hard == 3);
  std::filesystem::remove(file);
}
