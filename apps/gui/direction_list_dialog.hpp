// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <QStringList>
#include <vector>

#include "horcom/chart/progressions.hpp"
#include "horcom/chart/symbolic.hpp"

class QLabel;
class QTableWidget;

namespace horcom {

/// The FORMAT der AUSGABE answer of a17dat.
enum class DirectionFormat {
  kDate,           ///< DATUM
  kAge,            ///< LEBENSJAHR/MONAT
  kArc,            ///< BOGEN in GRAD-MINUTE-SEKUNDE
  kArcDecimal,     ///< BOGEN in DEZIMAL-GRAD
};

/// How the direction tables dress their rows.
struct DirectionDisplay {
  /// the a18kopf lines above the table
  QStringList heading;
  /// the footer of a18tab, the latitude note of the primary
  QString footer;
  DirectionFormat format = DirectionFormat::kDate;
  /// his plinv of VORGABEN DIREKTIONEN
  int plinv = 3;
  /// EINZELNE PLANETEN ROT MARKIEREN
  std::vector<int> marked;
  /// the w4d of the run, the sort question drops its aspect sort at 360
  double base_angle_deg = kDefaultBaseAngleDeg;
  /// the calendar of the dates
  Calendar calendar = Calendar::kAuto;
  /// his klsyt, the small symbols of VORGABEN DIREKTIONEN
  bool small_symbols = false;
};

/// The result table of the direction evaluations, the a18tab and
/// aprimout output of SEKUNDÄR, SONNEN- and MOND-BOGEN, the symbolic
/// directions and the Kühr primaries.
class DirectionListDialog : public QDialog {
  Q_OBJECT

 public:
  /// The symbolic and primary list, arcs as ages.
  ///
  /// @param hits    the direction_hits result
  /// @param radix   the chart whose birth anchors the dates
  /// @param display header, format and dressing
  /// @param parent  the owning widget
  DirectionListDialog(std::vector<DirectionHit> hits, const Chart& radix, DirectionDisplay display,
                      QWidget* parent = nullptr);

  /// The secondary and arc list, dated events of life.
  ///
  /// @param events  the direction events
  /// @param radix   the birth chart
  /// @param display header, format and dressing
  /// @param parent  the owning widget
  DirectionListDialog(std::vector<DirectedEvent> events, const Chart& radix, DirectionDisplay display,
                      QWidget* parent = nullptr);

  /// @return the number of listed directions
  [[nodiscard]] int row_count() const;

  /// @param row a table row
  /// @return the texts of that row, the tests read them
  [[nodiscard]] QStringList row_texts(int row) const;

  /// Orders the list like the answers of LISTE SORTIEREN.
  ///
  /// @param mode 0 by time, 1 by aspects, 2 keeps the order of the walk
  void sort_rows(int mode);

  /// Shows the list like dirend, LISTE SORTIEREN ? over the screen before
  /// the first page, then the table in the chosen order. The button of
  /// the table sorts again.
  ///
  /// @return the result of exec
  int open_sorted();

 private:
  struct Row {
    std::size_t order = 0;
    double years = 0.0;
    double jd = 0.0;
    double arc_deg = 0.0;
    double angle_deg = 0.0;
    int directed = 0;
    QString directed_text;
    int target = 0;
    QString target_text;
    QString kind;
  };

  void build();
  void fill();

  DirectionDisplay display_;
  std::vector<Row> rows_;
  QTableWidget* table_ = nullptr;
  QLabel* count_ = nullptr;
};

}  // namespace horcom
