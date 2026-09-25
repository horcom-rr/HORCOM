// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QWidget>

namespace horcom {

/// The head of the window, a night sky with the logo and the chart data,
/// the homage to the himmel banner of his splash screen and website.
class Banner : public QWidget {
  Q_OBJECT

 public:
  /// Builds the banner at its fixed height, empty until the lines are set.
  ///
  /// @param parent the owning widget
  explicit Banner(QWidget* parent = nullptr);

  /// Sets the right hand information line, JD, delta T, ARMC, houses.
  void set_info(const QString& info);

  /// Sets the centre line, the loaded record or a notice. Shown in the
  /// green of his main menu panel when not empty.
  void set_record(const QString& record);
  /// @return the record line shown now
  [[nodiscard]] const QString& record() const { return record_; }

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  QString info_;
  QString record_;
};

}  // namespace horcom
