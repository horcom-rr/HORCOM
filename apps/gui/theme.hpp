// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

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

/// The application stylesheet.
inline constexpr const char* kStyleSheet = R"qss(
QMainWindow, QDialog {
  background: #0A0F1E;
}
QWidget {
  color: #E9E5D9;
  font-size: 13px;
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
  font-size: 12px;
}
QDockWidget {
  color: #D4A94A;
  font-family: "Cascadia Mono", Consolas, monospace;
  font-size: 11px;
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
  font-size: 12px;
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
  font-size: 10px;
  letter-spacing: 1px;
}
QTableCornerButton::section {
  background: #141B2E;
  border: none;
}
QComboBox, QDateEdit, QTimeEdit, QDoubleSpinBox, QLineEdit {
  background: #10162A;
  border: 1px solid #232D4A;
  border-radius: 5px;
  padding: 4px 8px;
  min-height: 20px;
  selection-background-color: #2C3A63;
}
QComboBox:focus, QDateEdit:focus, QTimeEdit:focus, QDoubleSpinBox:focus, QLineEdit:focus {
  border-color: #8F7433;
}
QComboBox::drop-down, QDateEdit::drop-down {
  border: none;
  width: 22px;
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

}  // namespace horcom::theme
