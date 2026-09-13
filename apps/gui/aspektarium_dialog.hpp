// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"

class QComboBox;
class QLabel;
class QTableWidget;

namespace horcom {

/// The Aspektarium of the original aspar, the triangular aspect matrix.
/// Exact separations stand above the diagonal, the named aspect below
/// it, the per body hit counts on the diagonal, beside it the divisor
/// table with angle and orb and under it his planet weights.
class AspektariumDialog : public QDialog {
  Q_OBJECT

 public:
  /// @param chart  the computed chart the matrix reads
  /// @param s      its settings, decide the mode word of the title
  /// @param a      orb configuration, the divisor limit moves with the
  ///               dialog's own selector like his MAXIMALER TEILER box
  /// @param record the record label for the title line
  AspektariumDialog(const Chart& chart, const ChartSettings& s, const AspectSettings& a, const QString& record, QWidget* parent = nullptr);

 private:
  void rebuild();

  Chart chart_;
  ChartSettings s_;
  AspectSettings base_;
  QComboBox* divisors_ = nullptr;
  QTableWidget* matrix_ = nullptr;
  QTableWidget* legend_ = nullptr;
  QLabel* weights_ = nullptr;
};

}  // namespace horcom
