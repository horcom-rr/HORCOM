// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "zodiac_cells.hpp"

#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QStyle>
#include <QTableWidgetItem>
#include <algorithm>
#include <cmath>

#include "horcom/chart/signs.hpp"
#include "horcom/render/wheel.hpp"
#include "painter.hpp"
#include "theme.hpp"

namespace horcom {

// his coordinate screen paints only the sign glyph of a position in
// its element colour, the numbers stay ink
// his zeich_col, Feuer, Erde, Luft, Wasser. Wasser asked for
// RGB(0,255,255), pure cyan drowns on a modern white panel, so it rides
// slightly darker
QColor element_color(int sign, bool bright) {
  static const QColor kOriginal[4] = {QColor(0xFF, 0x00, 0x00), QColor(0x80, 0x80, 0x00),
                                      QColor(0x00, 0x80, 0x80), QColor(0x00, 0xC8, 0xC8)};
  // the night dress needs lighter shades, a rewrite addition
  static const QColor kBright[4] = {QColor(0xFF, 0x8A, 0x70), QColor(0xD9, 0xB8, 0x4D),
                                    QColor(0x6F, 0xD0, 0xDC), QColor(0x00, 0xE8, 0xE8)};
  return (bright ? kBright : kOriginal)[((sign % kSignCount) + kSignCount) % 4];
}

// ported from grze with grze_0
QString zodiac_text(double rad, ZodiacForm form) {
  const ZodiacSplit z = split_zodiac(rad, form == ZodiacForm::kGz2);
  const char* tag = kSignTag[z.sign];
  switch (form) {
    case ZodiacForm::kGz0: return QString::asprintf("%2d\xC2\xB0%s%2d'", z.deg, tag, z.min);
    case ZodiacForm::kGz1: return QString::asprintf("%2d\xC2\xB0 %s %2d'", z.deg, tag, z.min);
    case ZodiacForm::kGz2: return QString::asprintf("%2d\xC2\xB0%s %2d'%2d\"", z.deg, tag, z.min, z.sec);
    case ZodiacForm::kGz8: return QString::asprintf("%2d %s %2d", z.deg, tag, z.min);
  }
  return {};
}

// ported from grzemise
QTableWidgetItem* zodiac_item(double rad, bool seconds, bool degree_mark) {
  const ZodiacSplit z = split_zodiac(rad, seconds);
  QString text = QString::asprintf(degree_mark ? "%2d\xC2\xB0 " : "%2d ", z.deg) + QString::fromUtf8(sign_glyph(z.sign));
  text += seconds ? QString::asprintf(" %02d'%02d\"", z.min, z.sec) : QString::asprintf(" %02d'", z.min);
  auto* item = new QTableWidgetItem(text);
  item->setData(kSignRole, z.sign + 1);
  return item;
}

void set_body_sprite(QTableWidgetItem* item, int slot, const QColor& ink) {
  if (slot <= 0) {
    return;
  }
  const QImage img = glyph_sprite(QString::fromUtf8(body_glyph(slot)), to_rgb(ink));
  if (!img.isNull()) {
    item->setIcon(QIcon(QPixmap::fromImage(img)));
  }
}

namespace {

// the side of the sprite square in a text rect of the given height
int sprite_side(const QFontMetrics& fm, int text_height) {
  return std::min(text_height - 2, fm.height() + 2);
}

// draws his sprite one to one on the device pixels of the cell, false
// when the glyph has no sprite
bool draw_sprite(QPainter* painter, const QString& glyph, const QColor& tint, const QRect& cell) {
  const qreal dpr = painter->device() != nullptr ? painter->device()->devicePixelRatio() : 1.0;
  const QSize px(static_cast<int>(std::lround(cell.width() * dpr)), static_cast<int>(std::lround(cell.height() * dpr)));
  QImage img = glyph_sprite_fitted(glyph, to_rgb(tint), px);
  if (img.isNull()) {
    return false;
  }
  img.setDevicePixelRatio(dpr);
  painter->drawImage(cell.topLeft(), img);
  return true;
}

// one unwrapped line of text in a cell, its height and the margins
// around it. The tag of a sprite cell stays hidden under the sprites, the
// tag wrapped in the narrow column would ask for two rows
struct LineCell {
  int height = 0;
  int margins = 0;
};

LineCell line_cell(const QStyleOptionViewItem& option) {
  QStyleOptionViewItem opt = option;
  const QString line = QStringLiteral("M");
  opt.text = line;
  opt.features &= ~QStyleOptionViewItem::WrapText;
  const QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
  const QSize one = style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), opt.widget);
  return {one.height(), std::max(0, one.width() - QFontMetrics(opt.font).horizontalAdvance(line))};
}

// the ink of a cell, the bright shades on the black selection band of
// the paper theme
QColor cell_ink(const QStyleOptionViewItem& opt) {
  const bool selected = (opt.state & QStyle::State_Selected) != 0;
  return theme::dark_now() || !selected ? theme::ink_now() : QColor(0xFF, 0xFF, 0xFF);
}

}  // namespace

QSize ZodiacDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  const int stored = index.data(kSignRole).toInt();
  if (stored <= 0) {
    return size;
  }
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  const QString glyph = QString::fromUtf8(sign_glyph(stored - 1));
  if (!opt.text.contains(glyph) || glyph_sprite(glyph, 0).isNull()) {
    return size;
  }
  // the sprite and the blank behind it take the place of the glyph the
  // base measured, at the largest side paint ever gives it
  const QFontMetrics fm(opt.font);
  const int sprite = sprite_side(fm, fm.height() + 4) + fm.horizontalAdvance(' ');
  size.rwidth() += std::max(0, sprite - fm.horizontalAdvance(glyph));
  return size;
}

void ZodiacDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  const int stored = index.data(kSignRole).toInt();
  if (stored <= 0) {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  const QString text = opt.text;
  const QString glyph = QString::fromUtf8(sign_glyph(stored - 1));
  const qsizetype at = text.indexOf(glyph);
  if (at < 0) {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }
  opt.text.clear();
  QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
  const bool dark = theme::dark_now();
  const bool selected = (opt.state & QStyle::State_Selected) != 0;
  const QColor ink = cell_ink(opt);
  QRect r = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
  painter->save();
  painter->setFont(opt.font);
  const QFontMetrics fm(opt.font);
  int x = r.x();
  const int y = r.y() + (r.height() + fm.ascent() - fm.descent()) / 2;
  const QString pre = text.left(at);
  const QString post = text.mid(at + glyph.size());
  painter->setPen(ink);
  painter->drawText(x, y, pre);
  x += fm.horizontalAdvance(pre);
  const QColor col = element_color(stored - 1, dark || selected);
  // his coordinate screen put the fat sprite into the cell, tinted
  // by zeich_col, the font glyph stays as the fallback
  const int side = sprite_side(fm, r.height());
  if (draw_sprite(painter, glyph, col, QRect(x, r.y() + (r.height() - side) / 2, side, side))) {
    x += side + fm.horizontalAdvance(' ');
  } else {
    painter->setPen(col);
    painter->drawText(x, y, glyph);
    x += fm.horizontalAdvance(glyph);
  }
  painter->setPen(ink);
  painter->drawText(x, y, post);
  painter->restore();
}

QSize SpriteRowDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  const QStringList glyphs = index.data(kSpritesRole).toStringList();
  if (glyphs.isEmpty()) {
    return size;
  }
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  // the sprites stand one text line high with a pixel of frame each side
  const LineCell line = line_cell(opt);
  const int side = sprite_side(QFontMetrics(opt.font), line.height);
  size = QSize(line.margins + static_cast<int>(glyphs.size()) * side + 2, line.height);
  return size;
}

// ported from a174g, the pair side by side and boxn around it
void SpriteRowDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  const QStringList glyphs = index.data(kSpritesRole).toStringList();
  const bool drawable = !glyphs.isEmpty() && std::all_of(glyphs.begin(), glyphs.end(), [](const QString& g) {
    return !glyph_sprite(g, 0).isNull();
  });
  if (!drawable) {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  opt.text.clear();
  QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
  const QRect r = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
  // a red mark of the cell tints the sprites like his plan_col!
  const QVariant fg = index.data(Qt::ForegroundRole);
  const QColor tint = fg.isValid() ? fg.value<QBrush>().color() : cell_ink(opt);
  // the side the width was measured with, a lower row shrinks it
  const int side = sprite_side(QFontMetrics(opt.font), std::min(r.height(), line_cell(opt).height));
  const bool frame = index.data(kFrameRole).toBool();
  const int top = r.y() + (r.height() - side) / 2;
  int x = r.x() + (frame ? 1 : 0);
  painter->save();
  for (const QString& g : glyphs) {
    draw_sprite(painter, g, tint, QRect(x, top, side, side));
    x += side;
  }
  if (frame) {
    // a hairline on whole pixels like his boxn
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(QPen(tint, 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(QRect(r.x(), top - 1, x - r.x(), side + 1));
  }
  painter->restore();
}

}  // namespace horcom
