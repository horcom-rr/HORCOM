// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QColor>
#include <QStyledItemDelegate>

class QTableWidgetItem;

// The zodiac cells of the result tables, his coordinate screen painted
// the sign sprite of a position in its element colour between the ink
// numbers.
namespace horcom {

/// The item role that carries the sign index plus one for the delegate.
inline constexpr int kSignRole = Qt::UserRole + 7;

/// The item role that lists the glyphs a SpriteRowDelegate cell draws as
/// his sprites side by side in place of its text.
inline constexpr int kSpritesRole = Qt::UserRole + 8;

/// The item role that frames the sprites of a cell like his boxn.
inline constexpr int kFrameRole = Qt::UserRole + 9;

/// His zeich_col, the element colour of a sign.
///
/// @param sign   0 for Aries through 11 for Pisces
/// @param bright the lighter shades of the night dress and selections
/// @return the colour
[[nodiscard]] QColor element_color(int sign, bool bright);

/// The text forms of his grze with the sign tags of zei$.
enum class ZodiacForm {
  kGz0,  ///< gz0$, "%2d°SS%2d'", the minutes rounded
  kGz1,  ///< gz1$, "%2d° SS %2d'", the minutes rounded
  kGz2,  ///< gz2$, "%2d°SS %2d'%2d\"", whole minutes and rounded seconds
  kGz8,  ///< gz8$, "%2d SS %2d", the minutes rounded
};

/// A longitude in one of his grze text forms.
///
/// @param rad  the longitude in radians
/// @param form the form
/// @return the text, the rounded last unit carries into the next sign
///         like his grze_0
[[nodiscard]] QString zodiac_text(double rad, ZodiacForm form);

/// A table cell like his grzemise, degree, sign glyph, minutes and on
/// demand seconds.
///
/// @param rad         the longitude in radians
/// @param seconds     true for minutes and rounded seconds, false for
///                    rounded minutes only
/// @param degree_mark true writes the degree sign behind the degree
/// @return the item, the sign rides kSignRole, the rounded last unit
///         carries into the next sign like his grze_0
[[nodiscard]] QTableWidgetItem* zodiac_item(double rad, bool seconds = true, bool degree_mark = false);

/// Puts his symbbmp sprite of a body before the text of a table cell,
/// where his screens drew the sprite and the tables write the tag.
///
/// @param item the cell
/// @param slot the body slot, zero or a slot without a sprite leaves it
/// @param ink  the tint, the colour of the cell text
void set_body_sprite(QTableWidgetItem* item, int slot, const QColor& ink);

/// Paints the zodiac cells of the main tables, only the sign glyph in
/// his element colour like the original coordinate screen.
class ZodiacDelegate final : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

  /// The room of a cell with the sprite in place of the font glyph, so a
  /// column sized to its contents holds the whole position.
  ///
  /// @param option the style of the view
  /// @param index  the cell
  /// @return the size the cell paints into
  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

/// Paints the cells whose kSpritesRole lists glyphs as his sprites in the
/// colour of the cell text, side by side and framed on kFrameRole like
/// the planet pair of a174g. The text stays in the item for copying and
/// the tests, a cell without the role paints as usual.
class SpriteRowDelegate final : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

  /// The room of the sprites and their frame.
  ///
  /// @param option the style of the view
  /// @param index  the cell
  /// @return the size the cell paints into
  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

}  // namespace horcom
