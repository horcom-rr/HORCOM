// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

// STATISTIK, the stat hub over the datasets of STATIST7, the writer of
// stat1, the deletion of stat3, the start of stat2 and the NEUE
// DATENSÄTZE UPDATEN branch of a22dat.

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMessageBox>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

#include "choice_dialog.hpp"
#include "horcom/core/constants.hpp"
#include "main_window_statist.hpp"

namespace horcom {

namespace {

// his hrc$ + "\STATIST7"
std::filesystem::path statist_dir(const std::filesystem::path& data) {
  return data / "statist7";
}

QString upper_ext(const std::filesystem::path& p) {
  return QString::fromStdWString(p.extension().wstring()).toUpper();
}

// the stored names of his house systems carry trailing blanks
QString house_label(std::string_view name) {
  return QString::fromUtf8(name.data(), static_cast<qsizetype>(name.size())).trimmed();
}

}  // namespace

QString stat_file_label(const std::filesystem::path& p) {
  return QString::fromStdWString(p.filename().wstring());
}

StatProgress::StatProgress(QWidget* parent, const QString& title, const QStringList& lines)
    : QWidget(parent, Qt::Dialog) {
  setWindowTitle(title);
  setWindowModality(Qt::ApplicationModal);
  setFocusPolicy(Qt::StrongFocus);
  auto* v = new QVBoxLayout(this);
  v->setContentsMargins(24, 18, 24, 18);
  v->setSpacing(14);
  for (const QString& line : lines) {
    v->addWidget(new QLabel(line, this), 0, Qt::AlignHCenter);
  }
  box_ = new QLabel(this);
  box_->setObjectName("statistPercent");
  // RGBCOLOR vg% = RGB(255,255,0), hg% = RGB(0,0,255), the yellow box
  // framed blue
  box_->setStyleSheet("QLabel { background: #ffff00; color: #000000; border: 2px solid #0000ff; padding: 10px 24px; }");
  box_->setAlignment(Qt::AlignCenter);
  v->addWidget(box_);
  v->addWidget(new QLabel(MainWindow::tr(" ABBRUCH mit 'ESC'"), this), 0, Qt::AlignHCenter);
}

void StatProgress::set_text(const QString& text) {
  box_->setText(text);
}

bool StatProgress::take_cancel() {
  const bool c = cancel_;
  cancel_ = false;
  return c;
}

void StatProgress::keyPressEvent(QKeyEvent* e) {
  // his INKEY$ = CHR$(27) of the WHILE loop
  if (e->key() == Qt::Key_Escape) {
    cancel_ = true;
    return;
  }
  QWidget::keyPressEvent(e);
}

void StatProgress::closeEvent(QCloseEvent* e) {
  // the close button stops like ESC, the window stays for the question
  cancel_ = true;
  e->ignore();
}

// ported from stat, the hub with its five rows
void MainWindow::statistics_hub() {
  std::error_code ec;
  const std::filesystem::path dir = statist_dir(data_dir_);
  // IF FGATTR(hrc$ + "\STATIST7") < 0 : MKDIR hrc$ + "\STATIST7"
  std::filesystem::create_directories(dir, ec);
  bool any = false;
  for (const auto& e : std::filesystem::directory_iterator(dir, ec)) {
    any = any || upper_ext(e.path()) == ".STA";
  }
  // ue$(0) = "     * WAS " + wol$ + " TUN ? *      "
  const int es = ChoiceDialog::ask(this, tr("     * WAS Wollen Sie  TUN ? *      "), {},
                                   {tr("VORGABEN STATISTIK ÄNDERN"), tr("AUSWERTEFÄHIGE DATEI ERSTELLEN ?"),
                                    tr("AUSWERTUNG STARTEN"), tr("AUSWERTEFÄHIGE DATEI LÖSCHEN ?"), tr("ABBRUCH")},
                                   any ? 2 : 1);
  switch (es) {
    case 0:
      statistics_defaults();
      break;
    case 1:
      create_statistics();
      break;
    case 2:
      run_statistics();
      break;
    case 3:
      delete_statistics();
      break;
    default:
      break;
  }
  // stend with kon_dhol, the panel returns to its own parameters
  recompute();
}

// ported from the VORGABEN STATISTIK row of stat
void MainWindow::statistics_defaults() {
  // "STATISTIK-DATEI","","NEUE DATENSÄTZE UPDATEN ?"
  const int b = ChoiceDialog::ask(this, tr("AUSWAHL"),
                                  {QString(), tr("STATISTIK-DATEI"), QString(), tr("NEUE DATENSÄTZE UPDATEN ?")},
                                  {tr("JA"), tr(" NEIN "), tr("ABBRUCH")}, konsta_.stats ? 0 : 1);
  if (b < 0 || b == 2) {
    return;
  }
  bool small = konsta_.slist;
  // IF hrg! = 0, the helio list knows only GROß
  if (!current_settings().heliocentric) {
    const int c = ChoiceDialog::ask(this, tr("AUSWAHL"),
                                    {tr("STATISTIK - AUSGABETABELLE :"), QString(), tr("MEHR DETAILS in KLEIN-SCHRIFT ?"),
                                     tr("WENIGER DETAILS in GROßSCHRIFT ?")},
                                    {tr("KLEIN"), tr("GROß"), tr("Zurück zum HAUPT - MENÜ")}, konsta_.slist ? 0 : 1);
    // his GOTO stend skipped kon_dsp, the first answer went with it
    if (c < 0 || c == 2) {
      return;
    }
    small = c == 0;
  }
  konsta_.stats = b == 0;
  konsta_.slist = small;
  // @kon_dsp
  persist_konsta();
}

// ported from stat1, AUSWERTEFÄHIGE DATEI ERSTELLEN
void MainWindow::create_statistics() {
  const ChartSettings s = current_settings();
  const QString helio_note = tr(" Die Datei wird HELIOZENTRISCH ANGELEGT !");
  QStringList info{QString(), tr("PARAMETER RICHTIG GESETZT ?")};
  if (s.heliocentric) {
    info.prepend(helio_note);
  }
  if (ChoiceDialog::ask(this, tr("AUSWAHL"), info, {tr("JA"), tr(" NEIN ")}, 0) != 0) {
    return;
  }
  // FILESELECT hrc$ + "\SPEZIAL\*.DAT"
  const QString src = QFileDialog::getOpenFileName(this, tr("AUSWERTEFÄHIGE DATEI ERSTELLEN"),
                                                   QString::fromStdWString((data_dir_ / "spezial").wstring()),
                                                   tr("HORCOM Daten-Dateien (*.DAT *.dat)"));
  if (src.isEmpty()) {
    return;
  }
  const auto records = read_chart_file(std::filesystem::path(src.toStdWString()));
  if (!records) {
    // fanz("DATEI " + datr$ + " ?")
    QMessageBox::information(this, tr(" HINWEIS "), tr("DATEI %1 ?").arg(src));
    return;
  }
  std::error_code ec;
  const std::filesystem::path dir = statist_dir(data_dir_);
  std::filesystem::create_directories(dir, ec);
  // daa$, the name of the DAT, the .PAR, .STA and .STH beside each other
  const QString stem = QFileInfo(src).completeBaseName().toUpper();
  const std::filesystem::path sta = find_ignoring_case(dir, std::filesystem::path((stem + ".STA").toStdWString()));
  if (std::filesystem::exists(par_path(sta), ec)) {
    // "STATISTIK-DATEI",ss$,"EXISTIERT SCHON !","NEU ANLEGEN ?"
    QStringList exists_info{tr("STATISTIK-DATEI"), QString::fromStdWString(par_path(sta).wstring()),
                            tr("EXISTIERT SCHON !"), tr("NEU ANLEGEN ?")};
    if (s.heliocentric) {
      exists_info.prepend(helio_note);
    }
    if (ChoiceDialog::ask(this, tr("AUSWAHL"), exists_info, {tr("ABBRUCH"), tr(" NEU ANLEGEN ")}, 0) != 1) {
      return;
    }
  }

  // the progress screen of stat10
  QStringList lines;
  if (s.heliocentric) {
    lines << helio_note;
  }
  // " STATISTIK-DATEI : " + UPPER$(daa$) + ".STA"
  lines << tr(" STATISTIK-DATEI : ") + stem + ".STA";
  StatProgress progress(this, tr("AUSWERTEFÄHIGE DATEI ERSTELLEN"), lines);
  progress.show();
  progress.setFocus();

  StatSet set;
  set.params = stat_params(s, konsta_);
  bool named = false;
  const std::size_t lf = std::max<std::size_t>(1, records->size());
  std::size_t done = 0;
  std::size_t refused = 0;
  for (const ChartRecord& r : *records) {
    ++done;
    // IF mo > 0 && ta > 0
    if (r.month <= 0 || r.day <= 0) {
      continue;
    }
    ChartSettings rs = s;
    rs.calendar = r.calendar();
    ChartInput in;
    in.date_ut = r.date();
    in.lon_deg_east = r.lon;
    in.lat_deg = r.lat;
    const Chart c = compute_chart(in, rs, vsop_, eph_);
    // Placidus and Koch refuse a place beyond the polar circles, his
    // stat1 had no maxbreit guard, the port counts the record out
    if (!c.ok) {
      ++refused;
      continue;
    }
    // haus$, the name of the house system the file was computed with
    if (!named && !house_label(c.houses.name).isEmpty()) {
      set.params.haus = house_label(c.houses.name).toStdString();
      named = true;
    }
    set.records.push_back(stat_record(r, c, s.heliocentric));
    // IF FRAC(laf& / 2) = 0, " nnn % Der STATISTIK-DATEI berechnet !"
    if (set.records.size() % 2 == 0 || done == records->size()) {
      const long pct = std::lround(kPercent * static_cast<double>(done) / static_cast<double>(lf));
      progress.set_text(QString(" %1").arg(pct, 5) + tr(" % Der STATISTIK-DATEI berechnet !"));
    }
    QApplication::processEvents();
    if (progress.take_cancel()) {
      // "STATISTIK-DATEI","",de$,"WIRKLICH VERWERFEN ?"
      const int rs2 = ChoiceDialog::ask(&progress, tr("AUSWAHL"),
                                        {tr("STATISTIK-DATEI"), QString(), QString::fromStdWString(sta.wstring()),
                                         tr("WIRKLICH VERWERFEN ?")},
                                        {tr("JA"), tr(" NEIN ")}, 0);
      if (rs2 == 0) {
        // his VERWERFEN killed only the .PAR, the torn .STA stayed
        remove_statistics(sta);
        return;
      }
    }
  }
  progress.hide();
  if (refused > 0) {
    // the words of his maxbreit, Geog. Breite zu groß !
    QMessageBox::information(this, tr(" HINWEIS "),
                             tr("%1 DATENSÄTZE NICHT AUFGENOMMEN : Geog. Breite zu groß für die HÄUSER-METHODE !")
                                 .arg(static_cast<qulonglong>(refused)));
  }
  if (set.records.empty() || !save_statistics(sta, set)) {
    QMessageBox::warning(this, "HORCOM", tr("Die Statistik-Datei ließ sich nicht schreiben."));
  }
}

// ported from stat3, AUSWERTEFÄHIGE DATEI LÖSCHEN
void MainWindow::delete_statistics() {
  // FILESELECT hrc$ + "\STATIST7\*.STA"
  const QString path = QFileDialog::getOpenFileName(this, tr("AUSWERTEFÄHIGE DATEI LÖSCHEN"),
                                                    QString::fromStdWString(statist_dir(data_dir_).wstring()),
                                                    tr("STATISTIK (*.STA *.sta)"));
  if (path.isEmpty()) {
    return;
  }
  const std::filesystem::path sta(path.toStdWString());
  if (!std::filesystem::exists(sta)) {
    QMessageBox::information(this, tr(" HINWEIS "), tr("DATEI %1 ?").arg(path));
    return;
  }
  // "",datr$,"","WIRKLICH LÖSCHEN ?" with NEIN as the default
  if (ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), path, QString(), tr("WIRKLICH LÖSCHEN ?")},
                        {tr(" NEIN "), tr("JA")}, 0) != 1) {
    return;
  }
  remove_statistics(sta);
}

// ported from stat2, AUSWERTUNG STARTEN
void MainWindow::run_statistics() {
  // FILESELECT hrc$ + "\STATIST7\*.STA"
  const QString path = QFileDialog::getOpenFileName(this, tr("AUSWERTUNG STARTEN"),
                                                    QString::fromStdWString(statist_dir(data_dir_).wstring()),
                                                    tr("STATISTIK (*.STA *.sta)"));
  if (path.isEmpty()) {
    return;
  }
  const std::filesystem::path sta(path.toStdWString());
  if (!std::filesystem::exists(sta)) {
    QMessageBox::information(this, tr(" HINWEIS "), tr("DATEI %1 ?").arg(path));
    return;
  }
  // his FRAC(LOF(#26) / 24 <> 0) took FRAC of a comparison and never
  // fired, the port checks the record size of the .STH
  std::error_code ec;
  if (const auto sth = existing_sth_path(sta); sth && std::filesystem::file_size(*sth, ec) % kSthRecordBytes != 0) {
    QMessageBox::information(this, tr(" HINWEIS "), tr("Wegen FORMAT-ÄNDERUNG bitte STATISTIK-Datei neu erstellen !"));
    return;
  }
  auto set = load_statistics(sta);
  if (!set) {
    // CATCH, "Das FORMAT hat sich GEÄNDERT !"
    ChoiceDialog::ask(this, tr("!! ACHTUNG !!"),
                      {tr("Das FORMAT hat sich GEÄNDERT !"), tr("Bitte LÖSCHEN Sie %1").arg(path),
                       tr("Via STATISTIK -Menü : 'AUSWERTEFÄHIGE DATEI LÖSCHEN'"), tr("Und ERZEUGEN Sie es NEU !")},
                      {tr(" OK ")}, 0);
    return;
  }
  // his laf& > 15975 refusal guarded the DIM budget of GFA, the port
  // takes a dataset of any size
  bool helio = current_settings().heliocentric;
  const bool file_helio = stat_heliocentric(*set);
  if (file_helio != helio) {
    // ALERT 3,"    Diese Datei ist HELIOZENTRISCH angelegt !",1," WEITER HELIOZENTRISCH |ABRUCH"
    const QString line = file_helio ? tr("    Diese Datei ist HELIOZENTRISCH angelegt !")
                                    : tr("    Diese Datei ist GEOZENTRISCH angelegt !");
    const QString go = file_helio ? tr(" WEITER HELIOZENTRISCH ") : tr(" WEITER GEOZENTRISCH ");
    if (ChoiceDialog::ask(this, tr("!! ACHTUNG !!"), {line}, {go, tr("ABRUCH")}, 0) != 0) {
      return;
    }
    // hrg! = -1 or 0, the program follows the file
    helio_->setChecked(file_helio);
    helio = file_helio;
  }
  StatSession ss;
  ss.sta = sta;
  ss.set = std::move(*set);
  ss.helio = helio;
  // IF hrg!, slist! = 0
  ss.small = konsta_.slist && !helio;
  ss.restart();
  stat_counter_mode_ = 0;
  for (;;) {
    ++ss.conditions;
    if (!stat_condition(ss)) {
      return;
    }
    if (ss.conditions == kStatMostConditions) {
      // fanz("MAXIMALE ANZAHL BEDINGUNGEN ERREICHT ")
      QMessageBox::information(this, tr(" HINWEIS "), tr("MAXIMALE ANZAHL BEDINGUNGEN ERREICHT "));
    }
    // IF suc& = 5, GOTO stat3, the list at once
    if (ss.suc != kStatSucAll) {
      const int j = stat_join(ss);
      if (j < 0) {
        return;
      }
      if (j >= 1 && j <= 3) {
        continue;
      }
    }
    const int next = stat_list(ss);
    if (next == 2) {
      continue;
    }
    if (next == 1) {
      //RR EINSPR. BEI WIEDERHOLUNG
      ss.restart();
      continue;
    }
    return;
  }
}

// ported from the STATISTIK branch of a22dat, NEUE DATENSÄTZE UPDATEN
void MainWindow::stat_update_record(const std::filesystem::path& dat, const ChartRecord& entry) {
  // IF stats! && EXIST(ss$) && EXIST(ss1$)
  if (!konsta_.stats) {
    return;
  }
  const QString stem = QString::fromStdWString(dat.stem().wstring()).toUpper();
  const std::filesystem::path sta =
      find_ignoring_case(statist_dir(data_dir_), std::filesystem::path((stem + ".STA").toStdWString()));
  if (!std::filesystem::exists(sta) || !existing_sth_path(sta)) {
    return;
  }
  // das$ + "in STATISTIK-DATEI","","ÜBERNEHMEN ?"
  if (ChoiceDialog::ask(this, tr("AUSWAHL"), {QString(), tr("Datensatz in STATISTIK-DATEI"), QString(), tr("ÜBERNEHMEN ?")},
                        {tr("JA"), tr(" NEIN ")}, 0) != 0) {
    return;
  }
  auto set = load_statistics(sta);
  if (!set) {
    return;
  }
  // @stat2parl(ss$), the record computed with the parameters of the file
  const bool helio = stat_heliocentric(*set);
  ChartSettings s = stat_chart_settings(konsta_, set->params, helio);
  s.calendar = entry.calendar();
  ChartInput in;
  in.date_ut = entry.date();
  in.lon_deg_east = entry.lon;
  in.lat_deg = entry.lat;
  const Chart c = compute_chart(in, s, vsop_, eph_);
  if (!c.ok) {
    // the words of his maxbreit, Placidus and Koch refuse the place
    QMessageBox::information(this, tr(" HINWEIS "), tr("Geog. Breite zu groß ! Datensatz NICHT in STATISTIK-DATEI"));
    return;
  }
  const StatRecord rec = stat_record(entry, c, helio);
  // his TRIM$(na$) = TRIM$(na$(od,ze)) compared the upper case field with
  // the typed name and doubled every record not typed in capitals
  //RR prüfen ob schon vorhanden
  const QString wanted = QString::fromStdString(rec.name).trimmed();
  bool replaced = false;
  for (StatRecord& r : set->records) {
    if (QString::fromStdString(r.name).trimmed().compare(wanted, Qt::CaseInsensitive) == 0) {
      // fanz("VORHANDENER DATENSATZ  " + TRIM$(na$) + " wird ÜBERSCHRIEBEN !")
      QMessageBox::information(this, tr(" HINWEIS "), tr("VORHANDENER DATENSATZ  %1 wird ÜBERSCHRIEBEN !").arg(wanted));
      r = rec;
      replaced = true;
      break;
    }
  }
  // his append put the .STH row at RECORD #26,k& with a stale k&, the
  // port appends both files together
  if (!replaced) {
    set->records.push_back(rec);
  }
  if (!save_statistics(sta, *set)) {
    return;
  }
  // a22sta_anz, "STATISTIK-DATEI : " + STR$(n * 210) + " BYTE = " + STR$(n) + " SÄTZE"
  const auto count = static_cast<long long>(set->records.size());
  QMessageBox::information(this, tr(" HINWEIS "),
                           tr("STATISTIK-DATEI : %1 BYTE = %2 SÄTZE").arg(count * static_cast<long long>(kStaRecordBytes)).arg(count));
}

}  // namespace horcom
