// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "place_hub.hpp"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCoreApplication>
#include <QWidget>
#include <system_error>
#include <vector>

#include "choice_dialog.hpp"
#include "horcom/data/countries.hpp"
#include "place_dialog.hpp"

namespace horcom {

namespace {

// the translation context of the hub, his captions are German
struct PlaceHub {
  Q_DECLARE_TR_FUNCTIONS(PlaceHub)
};

QString qpath(const std::filesystem::path& p) { return QString::fromStdWString(p.wstring()); }

// his make_resdat, RRESERVE.INT keeps the file before a destroying pass
void make_reserve(const std::filesystem::path& file) {
  std::error_code ec;
  std::filesystem::copy_file(file, file.parent_path() / "RRESERVE.INT",
                             std::filesystem::copy_options::overwrite_existing, ec);
}

// his fanz after a22ort, the size of the file in bytes and records
void report_size(QWidget* parent, const std::filesystem::path& file) {
  std::error_code ec;
  const auto bytes = std::filesystem::file_size(file, ec);
  const auto n = ec ? 0 : bytes / kPlaceRecordBytes;
  // "DATEI " + da$ + " : " + LOF + " BYTE = " + a$ + " SÄTZE"
  QMessageBox::information(parent, "HORCOM",
                           PlaceHub::tr("DATEI %1 : %2 BYTE = %3 SÄTZE")
                               .arg(QString::fromStdWString(file.filename().wstring()))
                               .arg(ec ? 0 : bytes)
                               .arg(n));
}

// writes the edited list back only when the pass removed something, his
// NAME of the scratch file ran only for lf1& - lf2& > 0
bool rewrite_if_shorter(QWidget* parent, const std::filesystem::path& file, const std::vector<PlaceRecord>& places,
                        std::size_t before) {
  if (places.size() >= before) {
    return true;
  }
  if (!write_place_file(file, places)) {
    QMessageBox::warning(parent, "HORCOM", PlaceHub::tr("Die Orts-Datei ließ sich nicht schreiben."));
    return false;
  }
  return true;
}

// ported from the LÖSCHEN branch of a2fort with a2f_weiter
void delete_loop(QWidget* parent, const std::filesystem::path& data_dir, const std::filesystem::path& file) {
  const QString name = QString::fromStdWString(file.filename().wstring());
  for (;;) {
    PlaceDialog pick(data_dir / "places", data_dir / "landnima.int", parent);
    pick.lock_file(file);
    pick.set_delete_mode(10);
    if (pick.exec() != QDialog::Accepted) {
      return;
    }
    auto places = read_place_file(file);
    if (!places) {
      QMessageBox::warning(parent, "HORCOM", PlaceHub::tr("Die Orts-Datei ließ sich nicht lesen."));
      return;
    }
    make_reserve(file);
    const std::size_t before = places->size();
    delete_places(*places, pick.marked());
    if (!rewrite_if_shorter(parent, file, *places, before)) {
      return;
    }
    //RR WEITER in der DATEI ... LÖSCHEN ?
    const int more = ChoiceDialog::ask(parent, "HORCOM", {PlaceHub::tr("WEITER in der DATEI"), name, PlaceHub::tr("LÖSCHEN ?")},
                                       {PlaceHub::tr("LÖSCHEN Beenden"), PlaceHub::tr("Weiter LÖSCHEN")});
    if (more != 1) {
      return;
    }
  }
}

// ported from the TRIMMEN branch of a2fort
void trim_file(QWidget* parent, const std::filesystem::path& file) {
  auto places = read_place_file(file);
  if (!places) {
    QMessageBox::warning(parent, "HORCOM", PlaceHub::tr("Die Orts-Datei ließ sich nicht lesen."));
    return;
  }
  make_reserve(file);
  const std::size_t before = places->size();
  trim_places(*places);
  if (places->size() == before) {
    QMessageBox::information(parent, "HORCOM", PlaceHub::tr("Nichts zu bereinigen, %1 Datensätze.").arg(before));
    return;
  }
  if (rewrite_if_shorter(parent, file, *places, before)) {
    QMessageBox::information(parent, "HORCOM", PlaceHub::tr("%1 von %2 Datensätzen bleiben.").arg(places->size()).arg(before));
  }
}

// ported from a2ortn, a BIGFILE named CC_A_K is confirmed with its
// country and letter group before the list opens
bool confirm_bigfile(QWidget* parent, const std::filesystem::path& data_dir, const QString& file_name) {
  if (file_name.size() < 6 || file_name[2] != '_' || file_name[4] != '_') {
    return true;
  }
  QString land = file_name.left(2);
  if (const auto nima = load_nima_countries(data_dir / "landnima.int")) {
    const std::string country = nima_country_name(*nima, file_name.left(2).toStdString());
    if (!country.empty()) {
      land = QString::fromStdString(country);
    }
  }
  // rt$ = RIGHT$(datro$,7), bchst$ = LEFT$(rt$,3)
  const QString group = file_name.right(7).left(3);
  const QString letters = group.left(1) + " .... " + group.right(1);
  //RR Sie haben folgende Orts-Datei gewählt :
  const int al = ChoiceDialog::ask(parent, "HORCOM",
                                   {PlaceHub::tr("Sie haben folgende Orts-Datei gewählt :"), QString(),
                                    PlaceHub::tr("%1 , Buchstaben-Gruppe  %2").arg(land, letters)},
                                   {PlaceHub::tr("OK"), PlaceHub::tr("KORRIGIEREN ?")});
  return al == 0;
}

}  // namespace

// ported from a2ort and a2fort
std::optional<PlaceRecord> place_file_hub(QWidget* parent, const std::filesystem::path& data_dir) {
  std::filesystem::path file;
  for (;;) {
    // FILESELECT dr$ + "\*.INT"
    const QString path = QFileDialog::getOpenFileName(parent, PlaceHub::tr("ORTS-DATEIEN"), qpath(data_dir / "places"),
                                                      PlaceHub::tr("Orts-Dateien (*.INT *.int)"));
    if (path.isEmpty()) {
      return std::nullopt;
    }
    if (!path.endsWith("INT", Qt::CaseInsensitive)) {
      //RR KEINE oder UNGÜLTIGE Datei !
      QMessageBox::information(parent, "HORCOM", PlaceHub::tr("KEINE oder UNGÜLTIGE Datei !"));
      return std::nullopt;
    }
    if (confirm_bigfile(parent, data_dir, QFileInfo(path).fileName())) {
      file = std::filesystem::path(path.toStdWString());
      break;
    }
  }
  // ue$(0) = "DATEI : " + @trim_wind$(da$)
  const int es = ChoiceDialog::ask(parent, PlaceHub::tr("DATEI : %1").arg(QString::fromStdWString(file.filename().wstring())), {},
                                   {PlaceHub::tr("Datensatz  HOLEN     ?"), PlaceHub::tr("Datensätze LÖSCHEN ?"), PlaceHub::tr("Datei TRIMMEN ?"),
                                    PlaceHub::tr("ABBRUCH")});
  switch (es) {
    case 0: {
      PlaceDialog pick(data_dir / "places", data_dir / "landnima.int", parent);
      pick.lock_file(file);
      if (pick.exec() != QDialog::Accepted) {
        return std::nullopt;
      }
      PlaceRecord p = pick.chosen();
      p.name = pick.chosen_name().toStdString();
      return p;
    }
    case 1:
      delete_loop(parent, data_dir, file);
      return std::nullopt;
    case 2:
      trim_file(parent, file);
      return std::nullopt;
    default:
      return std::nullopt;
  }
}

// ported from the EINTRAGEN path of a2ort into a22ort
bool enter_place(QWidget* parent, const std::filesystem::path& data_dir, const PlaceRecord& place) {
  // his a22ort wanted both coordinates non zero, a place on the zero
  // meridian or the equator is a real place, only both zero is empty
  if (place.lon == 0.0 && place.lat == 0.0) {
    return false;
  }
  const QString path = QFileDialog::getSaveFileName(parent, PlaceHub::tr("Ort in Orts-Datei eintragen"),
                                                    qpath(data_dir / "places" / "eigene.int"),
                                                    PlaceHub::tr("Orts-Dateien (*.INT *.int)"), nullptr,
                                                    QFileDialog::DontConfirmOverwrite);
  if (path.isEmpty()) {
    return false;
  }
  const std::filesystem::path file(path.toStdWString());
  if (!append_place(file, place)) {
    QMessageBox::warning(parent, "HORCOM", PlaceHub::tr("Die Orts-Datei ließ sich nicht schreiben."));
    return false;
  }
  report_size(parent, file);
  return true;
}

// ported from the VORZUGSORT branches of eingabe with ortp
bool store_preferred_place(QWidget* parent, const std::filesystem::path& data_dir, const PlaceRecord& place) {
  const std::filesystem::path ext = data_dir / "ort.ext";
  if (std::filesystem::exists(ext)) {
    //RR BISHERIGEN VORZUGSORT LÖSCHEN|Und AKTUELLEN ORT dafür eintragen ?
    const int vz = ChoiceDialog::ask(parent, "HORCOM",
                                     {PlaceHub::tr("BISHERIGEN VORZUGSORT LÖSCHEN"), PlaceHub::tr("Und AKTUELLEN ORT dafür eintragen ? ")},
                                     {PlaceHub::tr("NEIN"), PlaceHub::tr("JA")});
    if (vz != 1) {
      return false;
    }
  }
  if (!write_preferred_place(ext, place)) {
    QMessageBox::warning(parent, "HORCOM", PlaceHub::tr("Die Datei ort.ext ließ sich nicht schreiben."));
    return false;
  }
  //RR NEUER VORZUGSORT gespeichert !
  QMessageBox::information(parent, "HORCOM", PlaceHub::tr("NEUER VORZUGSORT gespeichert !"));
  return true;
}

// ported from CASE 4 of the ORT menu in eingabe
void delete_preferred_place(QWidget* parent, const std::filesystem::path& data_dir) {
  const std::filesystem::path ext = data_dir / "ort.ext";
  if (!std::filesystem::exists(ext)) {
    //RR VORZUGSORT FEHLT !
    QMessageBox::information(parent, "HORCOM", PlaceHub::tr("VORZUGSORT FEHLT !"));
    return;
  }
  // MESSAGE vv$ + "WIRKLICH " + l$ + " ?","DATEI " + h$ + " " + l$
  const auto answer = QMessageBox::question(parent, PlaceHub::tr("DATEI %1 LÖSCHEN").arg(qpath(ext)),
                                            PlaceHub::tr("VORZUGSORT WIRKLICH LÖSCHEN ?"), QMessageBox::Yes | QMessageBox::No);
  if (answer != QMessageBox::Yes) {
    return;
  }
  std::error_code ec;
  std::filesystem::remove(ext, ec);
  //RR VORZUGSORT GELÖSCHT !
  QMessageBox::information(parent, "HORCOM", PlaceHub::tr("VORZUGSORT GELÖSCHT !"));
}

}  // namespace horcom
