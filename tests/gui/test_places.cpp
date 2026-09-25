// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QFocusEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QTableWidget>
#include <filesystem>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/place_file.hpp"
#include "place_dialog.hpp"
#include "place_hub.hpp"
#include "probe.hpp"
#include "record_mask_dialog.hpp"

using namespace horcom;
using horcom::test::DialogDriver;

namespace {

// a scratch data folder per test, never the shipped data
struct ScratchData {
  std::filesystem::path dir;
  explicit ScratchData(const char* name) : dir(std::filesystem::temp_directory_path() / name) {
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir / "places");
  }
  ~ScratchData() {
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
  }
};

std::vector<PlaceRecord> three_places() {
  return {{11.5, 48.1, "Testdorf / D"}, {13.4, 52.5, "Anderort / D"}, {9.2, 48.8, "Zielstadt / D"}};
}

DialogDriver::Step open_file(const std::filesystem::path& file) {
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

// marks rows of the place table by name, then presses WAHL - ENDE
DialogDriver::Step mark_places(const QStringList& names) {
  return [names](QDialog* d) {
    auto* table = d->findChild<QTableWidget*>();
    if (table == nullptr) {
      d->reject();
      return;
    }
    for (int r = 0; r < table->rowCount(); ++r) {
      if (names.contains(table->item(r, 0)->text())) {
        table->selectRow(r);
      }
    }
    DialogDriver::click("WAHL - ENDE")(d);
  };
}

void tab_into(QWidget* w, QEvent::Type type) {
  QFocusEvent ev(type, Qt::TabFocusReason);
  QApplication::sendEvent(w, &ev);
  QApplication::processEvents();
}

}  // namespace

TEST_CASE("ORTS-DATEIEN HOLEN hands back the chosen place") {
  ScratchData s("horcom_places_get");
  const auto file = s.dir / "places" / "TEST.INT";
  REQUIRE(write_place_file(file, three_places()));
  std::optional<PlaceRecord> got;
  {
    DialogDriver drive;
    drive.then(open_file(file))
        .then(DialogDriver::click("HOLEN"))
        .then([](QDialog* d) {
          auto* table = d->findChild<QTableWidget*>();
          for (int r = 0; r < table->rowCount(); ++r) {
            if (table->item(r, 0)->text() == "Zielstadt / D") {
              table->selectRow(r);
            }
          }
          DialogDriver::click("OK")(d);
        });
    got = place_file_hub(nullptr, s.dir);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
    //RR ue$(0) = "DATEI : " + name
    REQUIRE(drive.titles().size() == 3);
    CHECK(drive.titles()[1] == "DATEI : TEST.INT");
  }
  REQUIRE(got.has_value());
  CHECK(got->name == "Zielstadt / D");
  CHECK(got->lon == doctest::Approx(9.2));
}

TEST_CASE("ORTS-DATEIEN LÖSCHEN keeps a reserve and asks WEITER") {
  ScratchData s("horcom_places_delete");
  const auto file = s.dir / "places" / "TEST.INT";
  REQUIRE(write_place_file(file, three_places()));
  {
    DialogDriver drive;
    drive.then(open_file(file))
        .then(DialogDriver::click("LÖSCHEN"))
        .then(mark_places({"Anderort / D"}))
        .then(DialogDriver::click("Weiter LÖSCHEN"))
        .then(mark_places({"Testdorf / D"}))
        .then(DialogDriver::click("LÖSCHEN Beenden"));
    CHECK_FALSE(place_file_hub(nullptr, s.dir).has_value());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const auto left = read_place_file(file);
  REQUIRE(left.has_value());
  REQUIRE(left->size() == 1);
  CHECK((*left)[0].name == "Zielstadt / D");
  // his make_resdat, the file before the last pass
  const auto reserve = read_place_file(s.dir / "places" / "RRESERVE.INT");
  REQUIRE(reserve.has_value());
  CHECK(reserve->size() == 2);
}

TEST_CASE("ORTS-DATEIEN TRIMMEN drops the empty places") {
  ScratchData s("horcom_places_trim");
  const auto file = s.dir / "places" / "TEST.INT";
  auto places = three_places();
  places.push_back({0.0, 0.0, "Leer"});
  REQUIRE(write_place_file(file, places));
  QString note;
  {
    DialogDriver drive;
    drive.then(open_file(file)).then(DialogDriver::click("TRIMMEN")).then([&note](QDialog* d) {
      if (auto* box = qobject_cast<QMessageBox*>(d)) {
        note = box->text();
      }
      d->accept();
    });
    CHECK_FALSE(place_file_hub(nullptr, s.dir).has_value());
    CHECK(drive.pending() == 0);
  }
  CHECK(note == "3 von 4 Datensätzen bleiben.");
  const auto left = read_place_file(file);
  REQUIRE(left.has_value());
  CHECK(left->size() == 3);
}

TEST_CASE("the ORT menu of the entry mask loads the VORZUGSORT") {
  ScratchData s("horcom_places_menu");
  REQUIRE(write_preferred_place(s.dir / "ort.ext", {10.123456, 47.654321, "Vorzugsdorf"}));
  RecordMaskDialog mask(AafRecord{}, "NEU-EINGABE", RecordMaskDialog::Mode::kEntry, s.dir);
  const QList<QLineEdit*> edits = mask.findChildren<QLineEdit*>();
  {
    DialogDriver drive;
    drive.then([](QDialog* d) {
      // a VORZUGSORT on file shows its plain row
      bool plain = false;
      for (QAbstractButton* b : d->findChildren<QAbstractButton*>()) {
        plain = plain || b->text() == " VORZUGSORT ";
      }
      CHECK(plain);
      DialogDriver::click(" VORZUGSORT ")(d);
    });
    tab_into(edits[1], QEvent::FocusIn);
    CHECK(drive.pending() == 0);
    CHECK(drive.titles() == QStringList{"EINGEBEN !"});
  }
  CHECK(edits[1]->text() == "VORZUGSDORF");
  CHECK(edits[2]->text() == "E");
  CHECK(edits[3]->text() == "10");
  CHECK(edits[4]->text() == "7");
  CHECK(edits[5]->text() == "24");
  CHECK(edits[7]->text() == "47");
  CHECK(edits[8]->text() == "39");
  CHECK(edits[9]->text() == "16");
}

TEST_CASE("a new place in the mask is offered for ORT ABSPEICHERN") {
  ScratchData s("horcom_places_store");
  RecordMaskDialog mask(AafRecord{}, "NEU-EINGABE", RecordMaskDialog::Mode::kEntry, s.dir);
  const QList<QLineEdit*> edits = mask.findChildren<QLineEdit*>();
  const QStringList values = {"", "NEUORT", "E", "8", "30", "0", "N", "47", "15", "0"};
  for (int i = 1; i < values.size(); ++i) {
    edits[i]->setText(values[i]);
  }
  QString note;
  {
    DialogDriver drive;
    drive.then(DialogDriver::click("VORZUGSORT speichern")).then([&note](QDialog* d) {
      if (auto* box = qobject_cast<QMessageBox*>(d)) {
        note = box->text();
      }
      d->accept();
    });
    tab_into(edits[9], QEvent::FocusOut);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR NEUER VORZUGSORT gespeichert !
  CHECK(note == "NEUER VORZUGSORT gespeichert !");
  const auto home = read_preferred_place(s.dir / "ort.ext");
  REQUIRE(home.has_value());
  CHECK(home->name == "NEUORT");
  CHECK(home->lon == doctest::Approx(8.5));
  CHECK(home->lat == doctest::Approx(47.25));
  // his afo still sees the place as new against i$, the next pass asks
  // again like his CASE 111
  {
    DialogDriver drive;
    drive.then(DialogDriver::click(" NEIN "));
    tab_into(edits[9], QEvent::FocusOut);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
}

TEST_CASE("the delete marks of the place list survive a new filter") {
  ScratchData s("horcom_places_marks");
  const auto file = s.dir / "places" / "TEST.INT";
  REQUIRE(write_place_file(file, three_places()));
  PlaceDialog pick(s.dir / "places", s.dir / "landnima.int");
  pick.lock_file(file);
  pick.set_delete_mode(10);
  auto* table = pick.findChild<QTableWidget*>();
  auto* filter = pick.findChild<QLineEdit*>();
  REQUIRE(table != nullptr);
  REQUIRE(filter != nullptr);
  for (int r = 0; r < table->rowCount(); ++r) {
    if (table->item(r, 0)->text() == "Anderort / D") {
      table->selectRow(r);
    }
  }
  filter->setText("Test");
  CHECK(table->rowCount() == 1);
  filter->setText("");
  // the first port rebuilt the table and dropped the mark
  bool marked = false;
  for (int r = 0; r < table->rowCount(); ++r) {
    if (table->item(r, 0)->text() == "Anderort / D") {
      marked = table->selectionModel()->isRowSelected(r, QModelIndex());
    }
  }
  CHECK(marked);
  DialogDriver::click("WAHL - ENDE")(&pick);
  CHECK(pick.marked() == std::vector<std::size_t>{1});
}

TEST_CASE("places sort with the umlauts among their vowels like his vg| table") {
  ScratchData s("horcom_places_umlaut");
  const auto file = s.dir / "places" / "UMLAUT.INT";
  REQUIRE(write_place_file(file, {{8.5, 47.4, "Zürich"}, {10.9, 47.1, "Ötztal"}, {9.9, 48.4, "Ulm"},
                                  {10.1, 47.8, "Oberdorf"}}));
  PlaceDialog pick(s.dir / "places", s.dir / "landnima.int");
  pick.lock_file(file);
  auto* table = pick.findChild<QTableWidget*>();
  REQUIRE(table != nullptr);
  REQUIRE(table->rowCount() == 4);
  // the first port put Ötztal behind Zürich
  CHECK(table->item(0, 0)->text() == "Oberdorf");
  CHECK(table->item(1, 0)->text() == "Ötztal");
  CHECK(table->item(3, 0)->text() == "Zürich");
}

TEST_CASE("Datensätze LÖSCHEN runs another round on Weiter LÖSCHEN") {
  const auto dir = std::filesystem::temp_directory_path() / "horcom_dat_delete";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  const auto dat = dir / "RUNDE.DAT";
  std::vector<ChartRecord> recs(3);
  const char* names[3] = {"ERSTER FALL", "ZWEITER FALL", "DRITTER FALL"};
  for (int i = 0; i < 3; ++i) {
    recs[static_cast<std::size_t>(i)].name = names[i];
    recs[static_cast<std::size_t>(i)].place = "TESTORT";
    recs[static_cast<std::size_t>(i)].day = 1 + i;
    recs[static_cast<std::size_t>(i)].month = 5;
    recs[static_cast<std::size_t>(i)].year = 1980;
    recs[static_cast<std::size_t>(i)].lon = 11.5;
    recs[static_cast<std::size_t>(i)].lat = 48.1;
  }
  REQUIRE(write_chart_file(dat, recs));
  auto w = MainWindowProbe::make();
  MainWindowProbe::bind(*w, QString::fromStdWString(dat.wstring()));
  const auto mark = [](const QString& name) {
    return [name](QDialog* d) {
      auto* list = d->findChild<QListWidget*>();
      for (int r = 0; list != nullptr && r < list->count(); ++r) {
        if (list->item(r)->text().contains(name)) {
          // a click selects the row before his toggle reads it
          list->item(r)->setSelected(true);
          emit list->itemClicked(list->item(r));
        }
      }
      DialogDriver::click("WAHL - ENDE")(d);
    };
  };
  {
    DialogDriver drive;
    drive.then(mark("ZWEITER"))
        .then(DialogDriver::click("LÖSCHEN ?"))
        .then(DialogDriver::click("Weiter LÖSCHEN"))
        .then(mark("ERSTER"))
        .then(DialogDriver::click("LÖSCHEN ?"))
        .then(DialogDriver::click("LÖSCHEN Beenden"));
    MainWindowProbe::delete_from_file(*w);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  const auto left = read_chart_file(dat);
  REQUIRE(left.has_value());
  REQUIRE(left->size() == 1);
  CHECK(QString::fromStdString((*left)[0].name).trimmed() == "DRITTER FALL");
  std::filesystem::remove_all(dir);
}
