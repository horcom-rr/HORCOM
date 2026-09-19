// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QApplication>
#include <QColor>
#include <QSettings>
#include <QString>
#include <algorithm>

// The visual identity of the shell in two dresses. The paper theme,
// black on white with his yellow label boxes, follows the working
// screens of the original program and is the default. The night theme
// keeps the sky of his splash screen with gold headings. The starry
// banner stays in both, the original carried it over its white screens
// the same way.
namespace horcom::theme {

//RR RGB($C0,$DC,$C0)
inline constexpr const char* kPanelGreen = "#C0DCC0";

/// the settings key of the theme choice, true is the night theme
inline constexpr const char* kDarkKey = "view/darkTheme";

/// The desk behind the wheel paper per theme.
inline QColor desk_color(bool dark) {
  return dark ? QColor(0x10, 0x17, 0x2B) : QColor(0xE7, 0xE4, 0xD8);
}

/// The frame line around the wheel paper per theme.
inline QColor paper_edge_color(bool dark) {
  return dark ? QColor(0x23, 0x2D, 0x4A) : QColor(0xB9, 0xB4, 0xA2);
}

/// One colour token of the stylesheet, both dresses side by side.
struct Token {
  const char* mark;
  const char* dark;
  const char* light;
};

/// The colour table of the shell, the dark column is the night sky
/// family, the light column black on white with the yellow boxes of
/// his list screens.
inline constexpr Token kTokens[] = {
    {"@base@", "#10172B", "#FFFFFF"},
    {"@panel@", "#182138", "#F3F0E7"},
    {"@edge@", "#2C3759", "#B9B4A2"},
    {"@ink@", "#E9E5D9", "#000000"},
    {"@field@", "#151C33", "#FFFFFF"},
    {"@grid@", "#222C4E", "#D8D4C6"},
    {"@headBg@", "#182138", "#FFFF00"},
    {"@headInk@", "#FFFF00", "#000000"},
    {"@selBg@", "#33406B", "#000000"},
    {"@selInk@", "#E9E5D9", "#FFFFFF"},
    {"@btn@", "#222C4E", "#ECE9DD"},
    {"@btnHover@", "#232E50", "#E2DECF"},
    {"@focus@", "#A6A600", "#A6A600"},
    {"@checkBg@", "#A6A600", "#FFFF00"},
    {"@checkEdge@", "#FFFF00", "#000000"},
    {"@disInk@", "#565C6E", "#A8A497"},
    {"@disField@", "#131A2E", "#F1EEE4"},
    {"@disEdge@", "#232C4A", "#D4D0C2"},
    {"@disBtn@", "#18213B", "#EFECE1"},
    {"@arrowUp@", ":/arrow-up.svg", ":/arrow-up-ink.svg"},
    {"@arrowDown@", ":/arrow-down.svg", ":/arrow-down-ink.svg"},
    {"@arrowUpDim@", ":/arrow-up-dim.svg", ":/arrow-up-faint.svg"},
    {"@arrowDownDim@", ":/arrow-down-dim.svg", ":/arrow-down-faint.svg"},
    {"@arrowLeft@", ":/arrow-left.svg", ":/arrow-left-ink.svg"},
    {"@arrowRight@", ":/arrow-right.svg", ":/arrow-right-ink.svg"},
};

/// The stylesheet template, font sizes carry @Npx@ markers so the
/// text scale of the Ansicht menu can rebuild it, colours and arrow
/// icons carry the tokens of the theme table.
inline constexpr const char* kStyleSheetTemplate = R"qss(
QMainWindow, QDialog {
  background: @base@;
}
QWidget {
  color: @ink@;
  font-family: "Courier New", monospace;
  font-weight: bold;
  font-size: @14px@;
}
QLabel {
  background: transparent;
}
QMenuBar {
  background: @base@;
  border-bottom: 1px solid @edge@;
  padding: 2px 6px;
}
QMenuBar::item {
  padding: 4px 10px;
  background: transparent;
}
QMenuBar::item:selected {
  background: @selBg@;
  color: @selInk@;
  border-radius: 4px;
}
QMenu {
  background: @panel@;
  border: 1px solid @edge@;
  padding: 4px;
}
QMenu::item {
  padding: 5px 24px 5px 14px;
  border-radius: 4px;
}
QMenu::item:selected {
  background: @selBg@;
  color: @selInk@;
}
QMenu::separator {
  height: 1px;
  background: @edge@;
  margin: 4px 8px;
}
QToolBar#bannerBar {
  background: @base@;
  border: none;
  padding: 0;
  margin: 0;
  spacing: 0;
}
QLabel#aspectsLine {
  background: @panel@;
  border: 1px solid @edge@;
  border-radius: 5px;
  padding: 6px 10px;
  font-family: "Courier New", monospace;
  font-size: @13px@;
}
QDockWidget {
  color: @headInk@;
  background: @base@;
  font-family: "Courier New", monospace;
  font-size: @12px@;
  font-weight: bold;
  letter-spacing: 2px;
  text-transform: uppercase;
}
QDockWidget::title {
  background: @headBg@;
  border: 1px solid @edge@;
  padding: 5px 10px;
  font-weight: bold;
}
QGroupBox {
  font-weight: bold;
  border: 1px solid @edge@;
  border-radius: 5px;
  margin-top: 12px;
  padding: 6px 8px 6px 8px;
}
QGroupBox::title {
  subcontrol-origin: margin;
  subcontrol-position: top left;
  left: 10px;
  padding: 0 4px;
  background: @base@;
  color: @headInk@;
  font-weight: bold;
}
QTableWidget {
  background: @field@;
  alternate-background-color: @panel@;
  gridline-color: @grid@;
  border: 1px solid @edge@;
  font-family: "Courier New", monospace;
  font-size: @13px@;
  selection-background-color: @selBg@;
  selection-color: @selInk@;
}
/* Windows Qt styles ignore the widget-level selection-color and fall
   back to the palette HighlightedText role, so on the black selection
   band of the paper theme the row text vanishes. Pinning both colours
   on the item pseudo-state paints them the same active or inactive. */
QTableView::item:selected,
QTableView::item:selected:!active {
  background: @selBg@;
  color: @selInk@;
}
QHeaderView {
  background: @field@;
}
QAbstractScrollArea::corner {
  background: @field@;
}
QHeaderView::section {
  background: @headBg@;
  color: @headInk@;
  border: none;
  border-bottom: 1px solid @edge@;
  border-right: 1px solid @grid@;
  padding: 4px 8px;
  font-family: "Courier New", monospace;
  font-size: @11px@;
  letter-spacing: 1px;
}
QTableCornerButton::section {
  background: @headBg@;
  border: none;
}
QComboBox, QDateEdit, QTimeEdit, QSpinBox, QDoubleSpinBox, QLineEdit {
  background: @field@;
  border: 1px solid @edge@;
  border-radius: 5px;
  padding: 4px 8px;
  min-height: 20px;
  selection-background-color: @selBg@;
  selection-color: @selInk@;
}
QComboBox:focus, QDateEdit:focus, QTimeEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QLineEdit:focus {
  border-color: @focus@;
}
QComboBox::drop-down, QDateEdit::drop-down {
  border: none;
  width: 22px;
}
QComboBox::down-arrow, QDateEdit::down-arrow {
  image: url(@arrowDown@);
  width: 10px;
  height: 7px;
}
QSpinBox, QDoubleSpinBox, QTimeEdit {
  padding-right: 22px;
}
QSpinBox::up-button, QDoubleSpinBox::up-button, QTimeEdit::up-button {
  subcontrol-origin: border;
  subcontrol-position: top right;
  width: 18px;
  border-left: 1px solid @edge@;
  border-bottom: 1px solid @edge@;
  border-top-right-radius: 5px;
  background: @panel@;
}
QSpinBox::down-button, QDoubleSpinBox::down-button, QTimeEdit::down-button {
  subcontrol-origin: border;
  subcontrol-position: bottom right;
  width: 18px;
  border-left: 1px solid @edge@;
  border-bottom-right-radius: 5px;
  background: @panel@;
}
QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover, QTimeEdit::up-button:hover,
QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover, QTimeEdit::down-button:hover {
  background: @btnHover@;
}
QSpinBox::up-arrow, QDoubleSpinBox::up-arrow, QTimeEdit::up-arrow {
  image: url(@arrowUp@);
  width: 10px;
  height: 7px;
}
QSpinBox::down-arrow, QDoubleSpinBox::down-arrow, QTimeEdit::down-arrow {
  image: url(@arrowDown@);
  width: 10px;
  height: 7px;
}
QComboBox:disabled, QDateEdit:disabled, QTimeEdit:disabled, QSpinBox:disabled,
QDoubleSpinBox:disabled, QLineEdit:disabled {
  color: @disInk@;
  background: @disField@;
  border-color: @disEdge@;
}
QCheckBox:disabled, QLabel:disabled, QPushButton:disabled {
  color: @disInk@;
}
QPushButton:disabled {
  background: @disBtn@;
  border-color: @disEdge@;
}
QComboBox::down-arrow:disabled, QDateEdit::down-arrow:disabled {
  image: url(@arrowDownDim@);
}
QSpinBox::up-arrow:disabled, QDoubleSpinBox::up-arrow:disabled, QTimeEdit::up-arrow:disabled {
  image: url(@arrowUpDim@);
}
QSpinBox::down-arrow:disabled, QDoubleSpinBox::down-arrow:disabled, QTimeEdit::down-arrow:disabled {
  image: url(@arrowDownDim@);
}
QSpinBox::up-button:disabled, QDoubleSpinBox::up-button:disabled, QTimeEdit::up-button:disabled,
QSpinBox::down-button:disabled, QDoubleSpinBox::down-button:disabled, QTimeEdit::down-button:disabled {
  background: @disField@;
  border-color: @disEdge@;
}
QCalendarWidget QWidget#qt_calendar_navigationbar {
  background: @panel@;
  border: 1px solid @edge@;
  border-bottom: none;
}
QCalendarWidget QToolButton {
  background: transparent;
  border: none;
  border-radius: 4px;
  padding: 4px 8px;
  color: @ink@;
}
QCalendarWidget QToolButton:hover {
  background: @btnHover@;
}
QCalendarWidget QToolButton::menu-indicator {
  image: none;
}
QCalendarWidget QToolButton#qt_calendar_prevmonth {
  qproperty-icon: url(@arrowLeft@);
}
QCalendarWidget QToolButton#qt_calendar_nextmonth {
  qproperty-icon: url(@arrowRight@);
}
QCalendarWidget QSpinBox {
  padding: 1px 18px 1px 6px;
  min-height: 0;
  border-radius: 3px;
}
QCalendarWidget QSpinBox::up-button, QCalendarWidget QSpinBox::down-button {
  width: 14px;
  border-top-right-radius: 3px;
  border-bottom-right-radius: 3px;
}
QCalendarWidget QMenu {
  background: @panel@;
}
QCalendarWidget QAbstractItemView {
  background: @field@;
  alternate-background-color: @panel@;
  selection-background-color: @selBg@;
  selection-color: @selInk@;
  outline: none;
}
QComboBox QAbstractItemView {
  background: @panel@;
  border: 1px solid @edge@;
  selection-background-color: @selBg@;
  selection-color: @selInk@;
}
QCheckBox {
  spacing: 8px;
  padding: 2px 0;
}
QCheckBox::indicator, QAbstractItemView::indicator {
  width: 15px;
  height: 15px;
  border: 1px solid @edge@;
  border-radius: 4px;
  background: @field@;
}
QCheckBox::indicator:checked, QAbstractItemView::indicator:checked {
  background: @checkBg@;
  border-color: @checkEdge@;
}
QScrollBar:vertical {
  background: transparent;
  width: 10px;
  margin: 0;
}
QScrollBar::handle:vertical {
  background: @edge@;
  border-radius: 5px;
  min-height: 30px;
}
QScrollBar::handle:vertical:hover {
  background: @selBg@;
}
QScrollBar::add-line, QScrollBar::sub-line {
  height: 0;
  width: 0;
}
QScrollBar::add-page, QScrollBar::sub-page {
  background: transparent;
}
QScrollBar:horizontal {
  background: transparent;
  height: 10px;
}
QScrollBar::handle:horizontal {
  background: @edge@;
  border-radius: 5px;
  min-width: 30px;
}
QScrollBar::handle:horizontal:hover {
  background: @selBg@;
}
QListWidget {
  background: @field@;
  border: 1px solid @edge@;
  font-family: "Courier New", monospace;
}
/* the record chooser needs picks to stand out at a glance, his yellow
   label box against black is the strongest paper theme highlight, and
   at night the same yellow pops on the sky blue */
QListWidget::item:selected {
  background: @checkBg@;
  color: #000000;
}
QListWidget::item:selected:!active {
  background: @checkBg@;
  color: #000000;
}
QListWidget::item:hover {
  background: @grid@;
}
QTextBrowser, QTextEdit, QPlainTextEdit {
  background: @field@;
  color: @ink@;
  border: 1px solid @edge@;
  selection-background-color: @selBg@;
  selection-color: @selInk@;
}
QToolTip {
  background: @panel@;
  color: @ink@;
  border: 1px solid @focus@;
}
QMessageBox {
  background: @panel@;
}
QPushButton {
  background: @btn@;
  border: 1px solid @edge@;
  border-radius: 5px;
  padding: 5px 16px;
}
QPushButton:hover {
  border-color: @focus@;
}
QPushButton:pressed {
  background: @btnHover@;
}
QToolButton {
  background: @btn@;
  border: 1px solid @edge@;
  border-radius: 5px;
  padding: 4px 10px;
}
QToolButton:hover {
  border-color: @focus@;
}
QToolButton:pressed {
  background: @btnHover@;
}
)qss";

/// The text scale of the Ansicht menu, percent of the design size.
inline constexpr int kTextScaleMin = 70;
inline constexpr int kTextScaleMax = 180;
inline constexpr int kTextScaleStep = 10;
inline constexpr int kTextScaleNormal = 100;
/// the settings key the scale survives under between runs
inline constexpr const char* kTextScaleKey = "view/textScale";

/// Builds the stylesheet at a text scale in one of the two themes.
///
/// @param percent one hundred is the design size, clamped to the range
/// @param dark true for the night theme, false for black on white
/// @return the sheet with every font size scaled and colour resolved
inline QString stylesheet(int percent, bool dark) {
  const int p = std::clamp(percent, kTextScaleMin, kTextScaleMax);
  QString qss = QString::fromUtf8(kStyleSheetTemplate);
  for (const int base : {14, 13, 12, 11}) {
    qss.replace(QString("@%1px@").arg(base), QString("%1px").arg(std::max(7, base * p / kTextScaleNormal)));
  }
  for (const Token& t : kTokens) {
    qss.replace(QString::fromLatin1(t.mark), QString::fromLatin1(dark ? t.dark : t.light));
  }
  return qss;
}

/// The remembered theme, black on white unless the night theme was
/// chosen in the Ansicht menu.
inline bool dark_theme() {
  return QSettings().value(kDarkKey, false).toBool();
}

/// Applies scale and theme to the whole shell and remembers the theme
/// on the application object for the painting widgets.
inline void apply(int percent, bool dark) {
  qApp->setProperty("horcomDark", dark);
  qApp->setStyleSheet(stylesheet(percent, dark));
}

/// @return the active theme as the painting widgets need it per frame
inline bool dark_now() {
  return qApp->property("horcomDark").toBool();
}

/// A heading of the info lines, gold text at night, his yellow label
/// box on paper.
///
/// @param text the heading, already translated
/// @return the rich text span for a QLabel
inline QString heading_span(const QString& text) {
  // Die Titel der Abteilungen tragen jetzt fett und etwas größer, wie der
  // Tester es sich für seine Kurs-Kärtchen wünscht. Dunkler Modus lässt
  // die gelbe Schrift auf schwarzem Grund stehen, der weiße Modus hält
  // die gelbe Bandfarbe des Originals
  return dark_now()
             ? QString("<span style='color:#FFFF00;font-weight:bold;font-size:110%'>%1</span>").arg(text)
             : QString("<span style='background-color:#FFFF00;color:#000000;font-weight:bold;font-size:110%'>&nbsp;%1&nbsp;</span>").arg(text);
}

}  // namespace horcom::theme
