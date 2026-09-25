// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <QStringList>
#include <vector>

#include "horcom/chart/transit_search.hpp"

class QLabel;
class QTableWidget;

namespace horcom {

/// How the a18 tables dress their rows, the answers and switches the
/// flow collected before the sweep.
struct A18Display {
  /// the header lines of a18kopf, his di$ with the Grund-Aspekt first
  QStringList heading;
  /// his plinv, 1 inverts MA SA UR NE PL, 2 colours the aspects, 3 both
  int plinv = 3;
  /// EINZELNE LAUFENDE PLANETEN ROT MARKIEREN
  std::vector<int> marked;
  /// the node and apogee switches decide the time resolution, the
  /// calendar the dates
  ChartSettings settings;
  /// the w4d of the run, the sort question drops its aspect sort at 360
  double base_angle_deg = kDefaultBaseAngleDeg;
  /// his klsyt, the small symbols of VORGABEN DIREKTIONEN
  bool small_symbols = false;
  /// the Ereignis-Ort line of a18tab under the table, empty without one
  QString footer;
};

/// The result table of the TRANSITE and MUNDAN-ASPEKTE runs, the table
/// output of a18tab and a181. Every row is a found moment, a picked row
/// hands its moment back so the shell can show the sky of that hour.
class TransitListDialog : public QDialog {
  Q_OBJECT

 public:
  /// A transit list, running bodies over the radix points.
  ///
  /// @param events  the sweep result
  /// @param display header and dressing
  /// @param parent  the owning widget
  TransitListDialog(std::vector<TransitEvent> events, A18Display display, QWidget* parent = nullptr);

  /// A MUNDAN-ASPEKTE list, the running bodies among themselves.
  ///
  /// @param aspects the sweep result
  /// @param display header and dressing
  /// @param parent  the owning widget
  TransitListDialog(std::vector<MundaneAspect> aspects, A18Display display, QWidget* parent = nullptr);

  /// @return the picked moment, zero when none was chosen
  [[nodiscard]] double chosen_jd() const { return chosen_jd_; }

  /// @return the number of listed moments
  [[nodiscard]] int row_count() const;

  /// @param row a table row
  /// @return the texts of that row, the capture and the tests read them
  [[nodiscard]] QStringList row_texts(int row) const;

  /// Orders the list like the answers of LISTE SORTIEREN.
  ///
  /// @param mode 0 by time, 1 by aspects, 2 keeps the sweep order
  void sort_rows(int mode);

  /// Shows the list like dirend, LISTE SORTIEREN ? over the screen before
  /// the first page, then the table in the chosen order. The button of
  /// the table sorts again.
  ///
  /// @return the result of exec
  int open_sorted();

 private:
  /// One listed moment in the common shape of both tables.
  struct Row {
    double jd_ut = 0.0;
    std::size_t order = 0;
    int first = 0;
    int second = 0;
    QString second_text;
    QString positions;
    double angle_deg = 0.0;
    bool retrograde = false;
    bool station = false;
    bool minutes = false;
    bool hours = false;
  };

  void build(bool mundane);
  void fill();
  void accept_row(int row);

  A18Display display_;
  bool mundane_ = false;
  std::vector<Row> rows_;
  double chosen_jd_ = 0.0;
  QTableWidget* table_ = nullptr;
  QLabel* count_ = nullptr;
};

}  // namespace horcom
