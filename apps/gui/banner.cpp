// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "banner.hpp"

#include <QIcon>
#include <QLinearGradient>
#include <QPainter>

#include "theme.hpp"

namespace horcom {

namespace {

constexpr int kHeight = 64;

// a deterministic little universe, the same stars at every start
struct Star {
  double x;  // 0..1 of the width
  double y;  // 0..1 of the height
  double r;
  int warm;  // 0 cool white, 1 warm, 2 blue
};

constexpr Star kStars[] = {
    {0.03, 0.30, 1.0, 0}, {0.07, 0.68, 0.7, 2}, {0.11, 0.22, 1.3, 0}, {0.16, 0.55, 0.8, 1},
    {0.21, 0.80, 0.7, 0}, {0.26, 0.35, 1.1, 0}, {0.30, 0.15, 0.7, 2}, {0.34, 0.62, 0.9, 0},
    {0.38, 0.42, 1.5, 1}, {0.43, 0.75, 0.7, 0}, {0.47, 0.25, 0.9, 0}, {0.52, 0.58, 0.7, 2},
    {0.56, 0.38, 1.2, 0}, {0.61, 0.70, 0.8, 1}, {0.65, 0.18, 0.7, 0}, {0.69, 0.50, 1.0, 0},
    {0.74, 0.30, 0.7, 2}, {0.78, 0.66, 1.3, 0}, {0.82, 0.44, 0.8, 0}, {0.86, 0.22, 0.9, 1},
    {0.90, 0.58, 0.7, 0}, {0.94, 0.36, 1.1, 0}, {0.97, 0.72, 0.8, 2}, {0.50, 0.85, 0.6, 0},
};

}  // namespace

Banner::Banner(QWidget* parent) : QWidget(parent) {
  setFixedHeight(kHeight);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void Banner::set_info(const QString& info) {
  info_ = info;
  update();
}

void Banner::set_record(const QString& record) {
  record_ = record;
  update();
}

void Banner::paintEvent(QPaintEvent* /*event*/) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  // the night sky at night, plain paper on the white theme
  const bool dark = theme::dark_now();
  if (dark) {
    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor(0x0B, 0x11, 0x23));
    sky.setColorAt(0.7, QColor(0x15, 0x1E, 0x3A));
    sky.setColorAt(1.0, QColor(0x10, 0x17, 0x2B));
    p.fillRect(rect(), sky);
    for (const Star& s : kStars) {
      QColor c = s.warm == 1 ? QColor(0xFF, 0xE9, 0xB0) : s.warm == 2 ? QColor(0xCF, 0xE4, 0xFF) : QColor(0xFF, 0xFF, 0xFF);
      c.setAlphaF(0.55 + 0.35 * (s.r > 1.0));
      p.setPen(Qt::NoPen);
      p.setBrush(c);
      p.drawEllipse(QPointF(s.x * width(), s.y * height()), s.r, s.r);
    }
  } else {
    p.fillRect(rect(), QColor(0xF3, 0xF0, 0xE7));
  }

  // the logo and the name
  const QPixmap logo = QIcon(":/logo.svg").pixmap(40, 40);
  p.drawPixmap(14, (height() - 40) / 2, logo);
  QFont title = theme::mono_font();
  title.setPixelSize(20);
  title.setBold(true);
  title.setLetterSpacing(QFont::AbsoluteSpacing, 7.0);
  p.setFont(title);
  p.setPen(dark ? QColor(0xFF, 0xFF, 0x00) : QColor(0x00, 0x00, 0x00));
  p.drawText(QRect(66, 0, 260, height()), Qt::AlignVCenter | Qt::AlignLeft, "HORCOM");

  // the record, spoken on the green of his main menu panel. The name is
  // bold and a little larger so the heading at the top centre stands out
  if (!record_.isEmpty()) {
    QFont rec = theme::mono_font();
    rec.setPixelSize(15);
    rec.setBold(true);
    p.setFont(rec);
    const QFontMetrics fm(rec);
    const int w = fm.horizontalAdvance(record_) + 28;
    const int h = fm.height() + 8;
    // left of the true centre so the box stays clear of the epoch line
    const QRect box((width() - w) / 2 - 60, (height() - h) / 2, w, h);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor::fromString(theme::kPanelGreen));
    p.drawRoundedRect(box, 5, 5);
    p.setPen(QColor(0x1C, 0x2B, 0x1C));
    p.drawText(box, Qt::AlignCenter, record_);
  }

  // the epoch data on the right
  QFont mono = theme::mono_font();
  mono.setPixelSize(12);
  mono.setBold(true);
  p.setFont(mono);
  p.setPen(dark ? QColor(0xE9, 0xE5, 0xD9) : QColor(0x00, 0x00, 0x00));
  p.drawText(QRect(0, 0, width() - 16, height()), Qt::AlignVCenter | Qt::AlignRight, info_);

  p.setPen(theme::paper_edge_color(dark));
  p.drawLine(0, height() - 1, width(), height() - 1);
}

}  // namespace horcom
