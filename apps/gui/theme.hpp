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

inline constexpr const char* kSky = "#10172B";
inline constexpr const char* kPanel = "#182138";
inline constexpr const char* kPanelEdge = "#2C3759";
inline constexpr const char* kInk = "#E9E5D9";
inline constexpr const char* kInkDim = "#A7A498";
inline constexpr const char* kGold = "#FFFF00";
inline constexpr const char* kGoldDim = "#A6A600";
inline constexpr const char* kTeal = "#5FC0C0";
inline constexpr const char* kFire = "#E85D4E";
//RR RGB($C0,$DC,$C0)
inline constexpr const char* kPanelGreen = "#C0DCC0";

/// The stylesheet template, font sizes carry @Npx@ markers so the
/// text scale of the Ansicht menu can rebuild it.
inline constexpr const char* kStyleSheetTemplate = R"qss(
QMainWindow, QDialog {
  background: #10172B;
}
QWidget {
  color: #E9E5D9;
  font-family: "Courier New", monospace;
  font-weight: bold;
  font-size: @14px@;
}
QLabel {
  background: transparent;
}
QMenuBar {
  background: #10172B;
  border-bottom: 1px solid #2C3759;
  padding: 2px 6px;
}
QMenuBar::item {
  padding: 4px 10px;
  background: transparent;
}
QMenuBar::item:selected {
  background: #2C3759;
  border-radius: 4px;
}
QMenu {
  background: #182138;
  border: 1px solid #2C3759;
  padding: 4px;
}
QMenu::item {
  padding: 5px 24px 5px 14px;
  border-radius: 4px;
}
QMenu::item:selected {
  background: #2C3759;
}
QMenu::separator {
  height: 1px;
  background: #2C3759;
  margin: 4px 8px;
}
QToolBar#bannerBar {
  background: #10172B;
  border: none;
  padding: 0;
  margin: 0;
  spacing: 0;
}
QLabel#aspectsLine {
  background: #182138;
  border: 1px solid #2C3759;
  border-radius: 5px;
  padding: 6px 10px;
  font-family: "Courier New", monospace;
  font-size: @13px@;
}
QDockWidget {
  color: #FFFF00;
  background: #10172B;
  font-family: "Courier New", monospace;
  font-size: @12px@;
  letter-spacing: 2px;
  text-transform: uppercase;
}
QDockWidget::title {
  background: #182138;
  border: 1px solid #2C3759;
  padding: 5px 10px;
}
QTableWidget {
  background: #151C33;
  alternate-background-color: #182138;
  gridline-color: #222C4E;
  border: 1px solid #2C3759;
  font-family: "Courier New", monospace;
  font-size: @13px@;
  selection-background-color: #33406B;
  selection-color: #E9E5D9;
}
QHeaderView {
  background: #151C33;
}
QAbstractScrollArea::corner {
  background: #151C33;
}
QHeaderView::section {
  background: #182138;
  color: #FFFF00;
  border: none;
  border-bottom: 1px solid #2C3759;
  border-right: 1px solid #222C4E;
  padding: 4px 8px;
  font-family: "Courier New", monospace;
  font-size: @11px@;
  letter-spacing: 1px;
}
QTableCornerButton::section {
  background: #182138;
  border: none;
}
QComboBox, QDateEdit, QTimeEdit, QSpinBox, QDoubleSpinBox, QLineEdit {
  background: #151C33;
  border: 1px solid #2C3759;
  border-radius: 5px;
  padding: 4px 8px;
  min-height: 20px;
  selection-background-color: #33406B;
}
QComboBox:focus, QDateEdit:focus, QTimeEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QLineEdit:focus {
  border-color: #A6A600;
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
  border-left: 1px solid #2C3759;
  border-bottom: 1px solid #2C3759;
  border-top-right-radius: 5px;
  background: #182138;
}
QSpinBox::down-button, QDoubleSpinBox::down-button, QTimeEdit::down-button {
  subcontrol-origin: border;
  subcontrol-position: bottom right;
  width: 18px;
  border-left: 1px solid #2C3759;
  border-bottom-right-radius: 5px;
  background: #182138;
}
QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover, QTimeEdit::up-button:hover,
QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover, QTimeEdit::down-button:hover {
  background: #232E50;
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
  background: #131A2E;
  border-color: #232C4A;
}
QCheckBox:disabled, QLabel:disabled, QPushButton:disabled {
  color: #565C6E;
}
QPushButton:disabled {
  background: #18213B;
  border-color: #232C4A;
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
  background: #131A2E;
  border-color: #232C4A;
}
QCalendarWidget QWidget#qt_calendar_navigationbar {
  background: #182138;
  border: 1px solid #2C3759;
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
  background: #2C3759;
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
  background: #182138;
}
QCalendarWidget QAbstractItemView {
  background: #151C33;
  alternate-background-color: #182138;
  selection-background-color: #33406B;
  selection-color: #E9E5D9;
  outline: none;
}
QComboBox QAbstractItemView {
  background: #182138;
  border: 1px solid #2C3759;
  selection-background-color: #33406B;
}
QCheckBox {
  spacing: 8px;
  padding: 2px 0;
}
QCheckBox::indicator {
  width: 15px;
  height: 15px;
  border: 1px solid #2C3759;
  border-radius: 4px;
  background: #151C33;
}
QCheckBox::indicator:checked {
  background: #A6A600;
  border-color: #FFFF00;
}
QScrollBar:vertical {
  background: transparent;
  width: 10px;
  margin: 0;
}
QScrollBar::handle:vertical {
  background: #2C3759;
  border-radius: 5px;
  min-height: 30px;
}
QScrollBar::handle:vertical:hover {
  background: #33406B;
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
  background: #2C3759;
  border-radius: 5px;
  min-width: 30px;
}
QScrollBar::handle:horizontal:hover {
  background: #33406B;
}
QListWidget {
  background: #151C33;
  border: 1px solid #2C3759;
  font-family: "Courier New", monospace;
}
QToolTip {
  background: #182138;
  color: #E9E5D9;
  border: 1px solid #A6A600;
}
QMessageBox {
  background: #182138;
}
QPushButton {
  background: #222C4E;
  border: 1px solid #2C3759;
  border-radius: 5px;
  padding: 5px 16px;
}
QPushButton:hover {
  border-color: #A6A600;
}
QPushButton:pressed {
  background: #2C3759;
}
QToolButton {
  background: #222C4E;
  border: 1px solid #2C3759;
  border-radius: 5px;
  padding: 4px 10px;
}
QToolButton:hover {
  border-color: #A6A600;
}
QToolButton:pressed {
  background: #2C3759;
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
  for (const int base : {14, 13, 12, 11}) {
    qss.replace(QString("@%1px@").arg(base), QString("%1px").arg(std::max(7, base * p / kTextScaleNormal)));
  }
  return qss;
}

}  // namespace horcom::theme
