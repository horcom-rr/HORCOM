// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QLabel>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <array>
#include <filesystem>
#include <memory>
#include <vector>

#include "horcom/data/statist.hpp"
#include "horcom/data/statist_eval.hpp"
#include "horcom/data/statist_list.hpp"
#include "main_window.hpp"

class QCloseEvent;
class QKeyEvent;

// The state the STATISTIK flows share, the hub and the datasets in
// main_window_statist.cpp, the conditions of stat2 in
// main_window_statist_cond.cpp and the output list in
// main_window_statist_list.cpp.
namespace horcom {

/// his anzb& limit, MAXIMALE ANZAHL BEDINGUNGEN
inline constexpr int kStatMostConditions = 12;
/// his obj&, the rows of obj_wahl
inline constexpr int kStatObjBody = 1;
inline constexpr int kStatObjRuler = 2;
inline constexpr int kStatObjMidpoint = 3;
inline constexpr int kStatObjAspect = 4;
inline constexpr int kStatObjMirror = 5;
inline constexpr int kStatObjName = 6;
inline constexpr int kStatObjArabic = 7;
/// his suc&, the rows of such_wo
inline constexpr int kStatSucDegree = 1;
inline constexpr int kStatSucSign = 2;
inline constexpr int kStatSucHouse = 3;
inline constexpr int kStatSucBody = 4;
inline constexpr int kStatSucAll = 5;

/// @param p a path
/// @return his trim_wind$, the file name without the folder
[[nodiscard]] QString stat_file_label(const std::filesystem::path& p);

/// The progress screen of stat10, his yellow box framed blue with the
/// running line. A modal window that locks the menus, ESC and the close
/// button ask to stop.
class StatProgress : public QWidget {
 public:
  /// @param parent the owner window
  /// @param title  the window title
  /// @param lines  the centred lines above the box
  StatProgress(QWidget* parent, const QString& title, const QStringList& lines);

  /// @param text the line inside the yellow box
  void set_text(const QString& text);

  /// @return true once ESC or the close button asked to stop, the request
  ///         clears on reading
  [[nodiscard]] bool take_cancel();

 protected:
  void keyPressEvent(QKeyEvent* e) override;
  void closeEvent(QCloseEvent* e) override;

 private:
  QLabel* box_ = nullptr;
  bool cancel_ = false;
};

/// His m&() pick of ausw_pl_hs.
struct MainWindow::StatPick {
  enum class Kind { kBody, kHouse, kRuler, kLights, kAll };
  Kind kind = Kind::kBody;
  /// the body slot, the stored cusps H2 H3 H5 H6 at their list slots
  int slot = 0;
  /// HAUS NR. or HERR v. HAUS NR.
  int house = 0;
  /// pl$(m&)
  QString tag;

  /// @return the operand of the evaluation the pick stands for
  [[nodiscard]] StatOperand operand() const {
    StatOperand op;
    if (kind == Kind::kHouse) {
      op.kind = StatOperand::Kind::kCusp;
      op.house = house;
    } else if (kind == Kind::kRuler) {
      op.kind = StatOperand::Kind::kRuler;
      op.house = house;
    } else if (stat_cusp_slot(slot)) {
      op.kind = StatOperand::Kind::kCusp;
      op.house = kStatCuspHouse[static_cast<std::size_t>(slot - kStatCuspSlotFirst)];
    } else {
      op.body = slot;
    }
    return op;
  }
};

/// The state of one visit of AUSWERTUNG STARTEN.
struct MainWindow::StatSession {
  std::filesystem::path sta;
  StatSet set;
  bool helio = false;
  bool small = false;
  std::vector<double> mask;
  std::unique_ptr<StatList> list;
  /// anzb&, the conditions so far
  int conditions = 0;
  /// the join of the next condition, his odu$ and odex!
  StatJoin join = StatJoin::kFirst;
  QString odu;
  /// 'W' in the list asked for another condition
  bool weit = false;
  /// obja$ and suca$ of every condition
  QStringList objects;
  QStringList windows;
  /// what list_ausg reads of the last condition
  int obj = 0;
  int suc = 0;
  bool groups = false;
  bool angle_mc = false;
  QString object_line;
  QString window_line;
  QString object_tag;
  std::array<int, 13> sums{};
  int framed = 0;
  /// the counters of the viewed charts, his halbsz% and aspz%
  WanderCounts counts;

  /// regg with stend1, a new evaluation of the same file
  void restart() {
    list = std::make_unique<StatList>(set);
    mask.assign(set.records.size(), 0.0);
    conditions = 0;
    join = StatJoin::kFirst;
    odu.clear();
    weit = false;
    objects.clear();
    windows.clear();
  }

  /// @return inf_box3, the conditions so far
  [[nodiscard]] QStringList condition_lines() const {
    QStringList out;
    for (int i = 0; i < objects.size(); ++i) {
      // LEFT$(STR$(i&,2) + obja$(i&) + suca$(i&),40)
      out << (QString::asprintf("%2d", i + 1) + objects[i] + windows.value(i)).left(40);
    }
    return out;
  }
};

}  // namespace horcom
