// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "banner.hpp"

#include <QIcon>
#include <QLinearGradient>
#include <QPainter>

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

  // the night sky
  QLinearGradient sky(0, 0, 0, height());
  sky.setColorAt(0.0, QColor(0x06, 0x0A, 0x16));
  sky.setColorAt(0.7, QColor(0x10, 0x18, 0x30));
  sky.setColorAt(1.0, QColor(0x0A, 0x0F, 0x1E));
  p.fillRect(rect(), sky);
  for (const Star& s : kStars) {
    QColor c = s.warm == 1 ? QColor(0xFF, 0xE9, 0xB0) : s.warm == 2 ? QColor(0xCF, 0xE4, 0xFF) : QColor(0xFF, 0xFF, 0xFF);
    c.setAlphaF(0.55 + 0.35 * (s.r > 1.0));
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawEllipse(QPointF(s.x * width(), s.y * height()), s.r, s.r);
  }

  // the logo and the name
  const QPixmap logo = QIcon(":/logo.svg").pixmap(40, 40);
  p.drawPixmap(14, (height() - 40) / 2, logo);
  QFont title("Cascadia Mono");
  title.setStyleHint(QFont::Monospace);
  title.setPixelSize(20);
  title.setBold(true);
  title.setLetterSpacing(QFont::AbsoluteSpacing, 7.0);
  p.setFont(title);
  p.setPen(QColor(0xD4, 0xA9, 0x4A));
  p.drawText(QRect(66, 0, 260, height()), Qt::AlignVCenter | Qt::AlignLeft, "HORCOM");

  // the record, spoken on the green of his main menu panel
  if (!record_.isEmpty()) {
    QFont rec("Cascadia Mono");
    rec.setStyleHint(QFont::Monospace);
    rec.setPixelSize(12);
    p.setFont(rec);
    const QFontMetrics fm(rec);
    const int w = fm.horizontalAdvance(record_) + 24;
    // left of the true centre so the box stays clear of the epoch line
    const QRect box((width() - w) / 2 - 60, (height() - 24) / 2, w, 24);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0xC0, 0xDC, 0xC0));
    p.drawRoundedRect(box, 5, 5);
    p.setPen(QColor(0x1C, 0x2B, 0x1C));
    p.drawText(box, Qt::AlignCenter, record_);
  }

  // the epoch data on the right
  QFont mono("Cascadia Mono");
  mono.setStyleHint(QFont::Monospace);
  mono.setPixelSize(12);
  p.setFont(mono);
  p.setPen(QColor(0xE9, 0xE5, 0xD9));
  p.drawText(QRect(0, 0, width() - 16, height()), Qt::AlignVCenter | Qt::AlignRight, info_);

  p.setPen(QColor(0x23, 0x2D, 0x4A));
  p.drawLine(0, height() - 1, width(), height() - 1);
}

}  // namespace horcom
