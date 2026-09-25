// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QWidget>

#include "horcom/render/wheel.hpp"

namespace horcom {

/// Paints the wheel display list, one more backend beside the SVG
/// writer. The mouse wheel zooms around the cursor, a left drag pans
/// the zoomed sheet and a double click resets the view.
class WheelWidget : public QWidget {
  Q_OBJECT

 public:
  /// Builds an empty canvas.
  ///
  /// @param parent the owning widget
  explicit WheelWidget(QWidget* parent = nullptr);

  /// Replaces the drawing and repaints. The screen shows the wheel
  /// centred on a square sheet, the classic sheet stays for export.
  void set_display_list(DisplayList dl);

  /// Replaces the drawing with a plain canvas, drawn as delivered.
  /// The graph views use this, the wheel centring would push their
  /// axes off the paper.
  void set_plain_list(DisplayList dl);

  /// Shows what another canvas shows in the same way, a centred wheel as
  /// a centred wheel and a full sheet as delivered, the walk windows
  /// follow the main wheel with it.
  ///
  /// @param other the canvas to copy
  void show_like(const WheelWidget& other);

  /// @return the classic sheet drawing, the print and PDF paths read it
  [[nodiscard]] const DisplayList& display_list() const { return classic_; }

  /// @return the sheet as the last paint laid it out, the wheel centred
  ///         on the paper with its corner notes moved to the edges, the
  ///         delivered list before the first paint
  [[nodiscard]] const DisplayList& shown_list() const { return dl_.items.empty() ? classic_ : dl_; }

  /// A widget point on the plain canvas, his screen coordinates.
  ///
  /// @param at the point in widget pixels
  /// @return the point on the virtual canvas
  [[nodiscard]] QPointF to_canvas(const QPointF& at) const;

  /// A canvas point in widget pixels, the inverse of to_canvas.
  ///
  /// @param at the point on the virtual canvas
  /// @return the point in widget pixels
  [[nodiscard]] QPointF from_canvas(const QPointF& at) const;

 signals:
  /// The right mouse button went down on the sheet, his MOUSEK = 2.
  void right_clicked();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

 private:
  /// The scale and offset that fit a canvas into the widget.
  struct Fit {
    double scale = 1.0;
    double x = 0.0;
    double y = 0.0;
  };
  /// @param d the list, its own size or the wheel canvas when it has none
  /// @return the aspect preserving fit inside the paper margin
  [[nodiscard]] Fit fit_of(const DisplayList& d) const;

  DisplayList classic_;
  DisplayList dl_;       // the centred screen view of classic_
  double sheet_w_ = 0.0;  // the sheet width dl_ was built for
  bool plain_ = false;    // draw classic_ as delivered, no centring
  double zoom_ = 1.0;
  QPointF pan_;
  QPointF drag_start_;
  QPointF pan_start_;
  bool dragging_ = false;
};

}  // namespace horcom
