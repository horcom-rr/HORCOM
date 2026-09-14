// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QString>
#include <algorithm>

// The visual identity of the shell, the same tokens as the handbook
// theme. The night sky of his splash screen, gold for headings, his four
// element colours on the wheel, and the green of his main menu panel
// reserved for information that speaks with his voice.
namespace horcom::theme {

inline constexpr const char* kSky = "#0A0F1E";
inline constexpr const char* kPanel = "#141B2E";
inline constexpr const char* kPanelEdge = "#232D4A";
inline constexpr const char* kInk = "#E9E5D9";
inline constexpr const char* kInkDim = "#A7A498";
inline constexpr const char* kGold = "#D4A94A";
inline constexpr const char* kGoldDim = "#8F7433";
inline constexpr const char* kTeal = "#5FC0C0";
inline constexpr const char* kFire = "#E85D4E";
//RR RGB($C0,$DC,$C0)
inline constexpr const char* kPanelGreen = "#C0DCC0";

/// The stylesheet template, font sizes carry @Npx@ markers so the
/// text scale of the Ansicht menu can rebuild it.
inline constexpr const char* kStyleSheetTemplate = R"qss(
QMainWindow, QDialog {
  background: #0A0F1E;
}
QWidget {
  color: #E9E5D9;
  font-size: @13px@;
}
QLabel {
  background: transparent;
}
QMenuBar {
  background: #0A0F1E;
  border-bottom: 1px solid #232D4A;
  padding: 2px 6px;
}
QMenuBar::item {
  padding: 4px 10px;
  background: transparent;
}
QMenuBar::item:selected {
  background: #232D4A;
  border-radius: 4px;
}
QMenu {
  background: #141B2E;
  border: 1px solid #232D4A;
  padding: 4px;
}
QMenu::item {
  padding: 5px 24px 5px 14px;
  border-radius: 4px;
}
QMenu::item:selected {
  background: #232D4A;
}
QMenu::separator {
  height: 1px;
  background: #232D4A;
  margin: 4px 8px;
}
QToolBar#bannerBar {
  background: #0A0F1E;
  border: none;
  padding: 0;
  margin: 0;
  spacing: 0;
}
QLabel#aspectsLine {
  background: #141B2E;
  border: 1px solid #232D4A;
  border-radius: 5px;
  padding: 6px 10px;
  font-family: "Cascadia Mono", Consolas, monospace;
  font-size: @12px@;
}
QDockWidget {
  color: #D4A94A;
  font-family: "Cascadia Mono", Consolas, monospace;
  font-size: @11px@;
  letter-spacing: 2px;
  text-transform: uppercase;
}
QDockWidget::title {
  background: #141B2E;
  border: 1px solid #232D4A;
  padding: 5px 10px;
}
QTableWidget {
  background: #10162A;
  alternate-background-color: #141B2E;
  gridline-color: #1B2340;
  border: 1px solid #232D4A;
  font-family: "Cascadia Mono", Consolas, monospace;
  font-size: @12px@;
  selection-background-color: #2C3A63;
  selection-color: #E9E5D9;
}
QHeaderView::section {
  background: #141B2E;
  color: #D4A94A;
  border: none;
  border-bottom: 1px solid #232D4A;
  border-right: 1px solid #1B2340;
  padding: 4px 8px;
  font-family: "Cascadia Mono", Consolas, monospace;
  font-size: @10px@;
  letter-spacing: 1px;
}
QTableCornerButton::section {
  background: #141B2E;
  border: none;
}
QComboBox, QDateEdit, QTimeEdit, QSpinBox, QDoubleSpinBox, QLineEdit {
  background: #10162A;
  border: 1px solid #232D4A;
  border-radius: 5px;
  padding: 4px 8px;
  min-height: 20px;
  selection-background-color: #2C3A63;
}
QComboBox:focus, QDateEdit:focus, QTimeEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QLineEdit:focus {
  border-color: #8F7433;
}
QComboBox::drop-down, QDateEdit::drop-down {
  border: none;
  width: 22px;
}
QComboBox::down-arrow, QDateEdit::down-arrow {
  image: url(:/arrow-down.svg);
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
  border-left: 1px solid #232D4A;
  border-bottom: 1px solid #232D4A;
  border-top-right-radius: 5px;
  background: #141B2E;
}
QSpinBox::down-button, QDoubleSpinBox::down-button, QTimeEdit::down-button {
  subcontrol-origin: border;
  subcontrol-position: bottom right;
  width: 18px;
  border-left: 1px solid #232D4A;
  border-bottom-right-radius: 5px;
  background: #141B2E;
}
QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover, QTimeEdit::up-button:hover,
QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover, QTimeEdit::down-button:hover {
  background: #1B2440;
}
QSpinBox::up-arrow, QDoubleSpinBox::up-arrow, QTimeEdit::up-arrow {
  image: url(:/arrow-up.svg);
  width: 10px;
  height: 7px;
}
QSpinBox::down-arrow, QDoubleSpinBox::down-arrow, QTimeEdit::down-arrow {
  image: url(:/arrow-down.svg);
  width: 10px;
  height: 7px;
}
QComboBox:disabled, QDateEdit:disabled, QTimeEdit:disabled, QSpinBox:disabled,
QDoubleSpinBox:disabled, QLineEdit:disabled {
  color: #565C6E;
  background: #0D1322;
  border-color: #1A2238;
}
QCheckBox:disabled, QLabel:disabled, QPushButton:disabled {
  color: #565C6E;
}
QPushButton:disabled {
  background: #121930;
  border-color: #1A2238;
}
QComboBox::down-arrow:disabled, QDateEdit::down-arrow:disabled {
  image: url(:/arrow-down-dim.svg);
}
QSpinBox::up-arrow:disabled, QDoubleSpinBox::up-arrow:disabled, QTimeEdit::up-arrow:disabled {
  image: url(:/arrow-up-dim.svg);
}
QSpinBox::down-arrow:disabled, QDoubleSpinBox::down-arrow:disabled, QTimeEdit::down-arrow:disabled {
  image: url(:/arrow-down-dim.svg);
}
QSpinBox::up-button:disabled, QDoubleSpinBox::up-button:disabled, QTimeEdit::up-button:disabled,
QSpinBox::down-button:disabled, QDoubleSpinBox::down-button:disabled, QTimeEdit::down-button:disabled {
  background: #0D1322;
  border-color: #1A2238;
}
QCalendarWidget QWidget#qt_calendar_navigationbar {
  background: #141B2E;
  border: 1px solid #232D4A;
  border-bottom: none;
}
QCalendarWidget QToolButton {
  background: transparent;
  border: none;
  border-radius: 4px;
  padding: 4px 8px;
  color: #E9E5D9;
}
QCalendarWidget QToolButton:hover {
  background: #232D4A;
}
QCalendarWidget QToolButton::menu-indicator {
  image: none;
}
QCalendarWidget QToolButton#qt_calendar_prevmonth {
  qproperty-icon: url(:/arrow-left.svg);
}
QCalendarWidget QToolButton#qt_calendar_nextmonth {
  qproperty-icon: url(:/arrow-right.svg);
}
QCalendarWidget QMenu {
  background: #141B2E;
}
QCalendarWidget QAbstractItemView {
  background: #10162A;
  alternate-background-color: #141B2E;
  selection-background-color: #2C3A63;
  selection-color: #E9E5D9;
  outline: none;
}
QComboBox QAbstractItemView {
  background: #141B2E;
  border: 1px solid #232D4A;
  selection-background-color: #2C3A63;
}
QCheckBox {
  spacing: 8px;
  padding: 2px 0;
}
QCheckBox::indicator {
  width: 15px;
  height: 15px;
  border: 1px solid #232D4A;
  border-radius: 4px;
  background: #10162A;
}
QCheckBox::indicator:checked {
  background: #8F7433;
  border-color: #D4A94A;
}
QScrollBar:vertical {
  background: transparent;
  width: 10px;
  margin: 0;
}
QScrollBar::handle:vertical {
  background: #232D4A;
  border-radius: 5px;
  min-height: 30px;
}
QScrollBar::handle:vertical:hover {
  background: #2C3A63;
}
QScrollBar::add-line, QScrollBar::sub-line {
  height: 0;
}
QScrollBar:horizontal {
  background: transparent;
  height: 10px;
}
QScrollBar::handle:horizontal {
  background: #232D4A;
  border-radius: 5px;
  min-width: 30px;
}
QListWidget {
  background: #10162A;
  border: 1px solid #232D4A;
  font-family: "Cascadia Mono", Consolas, monospace;
}
QToolTip {
  background: #141B2E;
  color: #E9E5D9;
  border: 1px solid #8F7433;
}
QMessageBox {
  background: #141B2E;
}
QPushButton {
  background: #1B2340;
  border: 1px solid #232D4A;
  border-radius: 5px;
  padding: 5px 16px;
}
QPushButton:hover {
  border-color: #8F7433;
}
QPushButton:pressed {
  background: #232D4A;
}
QToolButton {
  background: #1B2340;
  border: 1px solid #232D4A;
  border-radius: 5px;
  padding: 4px 10px;
}
QToolButton:hover {
  border-color: #8F7433;
}
QToolButton:pressed {
  background: #232D4A;
}
)qss";

/// The text scale of the Ansicht menu, percent of the design size.
inline constexpr int kTextScaleMin = 70;
inline constexpr int kTextScaleMax = 180;
inline constexpr int kTextScaleStep = 10;
inline constexpr int kTextScaleNormal = 100;
/// the settings key the scale survives under between runs
inline constexpr const char* kTextScaleKey = "view/textScale";

/// Builds the stylesheet at a text scale.
///
/// @param percent one hundred is the design size, clamped to the range
/// @return the sheet with every font size scaled
inline QString stylesheet(int percent) {
  const int p = std::clamp(percent, kTextScaleMin, kTextScaleMax);
  QString qss = QString::fromUtf8(kStyleSheetTemplate);
  for (const int base : {13, 12, 11, 10}) {
    qss.replace(QString("@%1px@").arg(base), QString("%1px").arg(std::max(7, base * p / kTextScaleNormal)));
  }
  return qss;
}

}  // namespace horcom::theme
