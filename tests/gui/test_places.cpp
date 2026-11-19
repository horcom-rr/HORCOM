// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include <QAbstractButton>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QFocusEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QTableWidget>
#include <filesystem>

#include "dialog_driver.hpp"
#include "doctest.h"
#include "horcom/data/chart_file.hpp"
#include "horcom/data/countries.hpp"
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

// the text of a message box, then its OK
DialogDriver::Step note_of(QString& note) {
  return [&note](QDialog* d) {
    if (auto* box = qobject_cast<QMessageBox*>(d)) {
      note = box->text();
    }
    d->accept();
  };
}

// one standard button of a question box, its caption follows the locale
DialogDriver::Step answer(QMessageBox::StandardButton which) {
  return [which](QDialog* d) {
    auto* box = qobject_cast<QMessageBox*>(d);
    if (box == nullptr || box->button(which) == nullptr) {
      d->reject();
      return;
    }
    box->button(which)->click();
  };
}

// the lines of one of his boxes
QString box_lines(QDialog* d) {
  QString all;
  for (const QLabel* l : d->findChildren<QLabel*>()) {
    all += l->text() + "|";
  }
  return all;
}

}  // namespace

TEST_CASE("ORTS-DATEIEN HOLEN hands back the chosen place") {
  ScratchData s("horcom_places_get");
  const auto file = s.dir / "places" / "TEST.INT";
  REQUIRE(write_place_file(file, three_places()));
  std::optional<PlaceRecord> got;
  bool bold_names = true;
  {
    DialogDriver drive;
    drive.then(open_file(file))
        .then(DialogDriver::click("HOLEN"))
        .then([&bold_names](QDialog* d) {
          auto* table = d->findChild<QTableWidget*>();
          for (int r = 0; r < table->rowCount(); ++r) {
            // the place names bold, the coordinates plain
            bold_names = bold_names && table->item(r, 0)->font().bold() && !table->item(r, 1)->font().bold();
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
  CHECK(bold_names);
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
        // any number of marks may stand, the count is asked first
        .then(DialogDriver::click("1 Datensätze LÖSCHEN ?"))
        .then(DialogDriver::click("Weiter LÖSCHEN"))
        .then(mark_places({"Testdorf / D"}))
        .then(DialogDriver::click("1 Datensätze LÖSCHEN ?"))
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
  pick.set_delete_mode();
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

TEST_CASE("EINTRAGEN appends the place and reports the size of the file") {
  ScratchData s("horcom_places_enter");
  const auto file = s.dir / "places" / "EIGENE.INT";
  REQUIRE(write_place_file(file, three_places()));
  {
    // both coordinates zero is no place, nothing is asked or written
    DialogDriver drive;
    CHECK_FALSE(enter_place(nullptr, s.dir, {0.0, 0.0, "Nirgends"}));
    CHECK(drive.unexpected() == 0);
  }
  QString size;
  {
    DialogDriver drive;
    drive.then(open_file(file)).then(note_of(size));
    CHECK(enter_place(nullptr, s.dir, {7.5, 47.25, "Neudorf / CH"}));
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  // "DATEI " + da$ + " : " + LOF + " BYTE = " + a$ + " SÄTZE"
  CHECK(size == QString("DATEI EIGENE.INT : %1 BYTE = 4 SÄTZE").arg(4 * kPlaceRecordBytes));
  const auto places = read_place_file(file);
  REQUIRE(places.has_value());
  REQUIRE(places->size() == 4);
  CHECK(places->back().name == "Neudorf / CH");
  CHECK(places->back().lat == doctest::Approx(47.25));
}

TEST_CASE("the VORZUGSORT is replaced and deleted only after his questions") {
  ScratchData s("horcom_places_home");
  const auto ext = s.dir / "ort.ext";
  // the longitude on file, zero while none is stored
  const auto stored_lon = [&ext]() { return read_preferred_place(ext).value_or(PlaceRecord{}).lon; };
  QString note;
  {
    DialogDriver drive;
    drive.then(note_of(note));
    delete_preferred_place(nullptr, s.dir);
    CHECK(drive.pending() == 0);
  }
  //RR VORZUGSORT FEHLT !
  CHECK(note == "VORZUGSORT FEHLT !");
  REQUIRE(write_preferred_place(ext, {10.0, 47.0, "Altort"}));
  QString ask;
  {
    DialogDriver drive;
    drive.then([&ask](QDialog* d) {
      ask = box_lines(d);
      DialogDriver::click_exact("NEIN")(d);
    });
    CHECK_FALSE(store_preferred_place(nullptr, s.dir, {8.0, 46.0, "Neuort"}));
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR BISHERIGEN VORZUGSORT LÖSCHEN|Und AKTUELLEN ORT dafür eintragen ?
  CHECK(ask.contains("BISHERIGEN VORZUGSORT LÖSCHEN"));
  CHECK(stored_lon() == doctest::Approx(10.0));
  {
    DialogDriver drive;
    drive.then(DialogDriver::click_exact("JA")).then(note_of(note));
    CHECK(store_preferred_place(nullptr, s.dir, {8.0, 46.0, "Neuort"}));
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(note == "NEUER VORZUGSORT gespeichert !");
  CHECK(stored_lon() == doctest::Approx(8.0));
  {
    DialogDriver drive;
    drive.then(answer(QMessageBox::No));
    delete_preferred_place(nullptr, s.dir);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  CHECK(std::filesystem::exists(ext));
  {
    DialogDriver drive;
    drive.then(answer(QMessageBox::Yes)).then(note_of(note));
    delete_preferred_place(nullptr, s.dir);
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR VORZUGSORT GELÖSCHT !
  CHECK(note == "VORZUGSORT GELÖSCHT !");
  CHECK_FALSE(std::filesystem::exists(ext));
}

TEST_CASE("ORTS-DATEIEN confirms a BIGFILE and refuses a file of another kind") {
  ScratchData s("horcom_places_bigfile");
  std::filesystem::copy_file(HORCOM_TEST_DATA_DIR "/landnima.int", s.dir / "landnima.int");
  const auto nima = load_nima_countries(s.dir / "landnima.int");
  REQUIRE(nima.has_value());
  const QString land = QString::fromStdString(nima_country_name(*nima, "GM"));
  REQUIRE_FALSE(land.isEmpty());
  // a BIGFILE of one country and letter group, CC_A_K
  const auto big = s.dir / "places" / "GM_A_K.INT";
  REQUIRE(write_place_file(big, three_places()));
  QString confirm;
  QString note;
  {
    DialogDriver drive;
    drive.then(open_file(big))
        .then(DialogDriver::click("KORRIGIEREN ?"))  // the file choice again
        .then(open_file(big))
        .then([&confirm](QDialog* d) {
          confirm = box_lines(d);
          DialogDriver::click_exact("OK")(d);
        })
        .then(DialogDriver::click("TRIMMEN"))
        .then(note_of(note));
    CHECK_FALSE(place_file_hub(nullptr, s.dir).has_value());
    INFO(drive.titles().join(" | ").toStdString());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR Sie haben folgende Orts-Datei gewählt :
  CHECK(confirm.contains("Sie haben folgende Orts-Datei gewählt :"));
  CHECK(confirm.contains(land + " , Buchstaben-Gruppe  A .... K"));
  // a file without empty places stays as it was
  CHECK(note == "Nichts zu bereinigen, 3 Datensätze.");
  const auto other = s.dir / "places" / "LISTE.TXT";
  REQUIRE(write_place_file(other, three_places()));
  {
    DialogDriver drive;
    drive.then(open_file(other)).then(note_of(note));
    CHECK_FALSE(place_file_hub(nullptr, s.dir).has_value());
    CHECK(drive.pending() == 0);
    CHECK(drive.unexpected() == 0);
  }
  //RR KEINE oder UNGÜLTIGE Datei !
  CHECK(note == "KEINE oder UNGÜLTIGE Datei !");
}
