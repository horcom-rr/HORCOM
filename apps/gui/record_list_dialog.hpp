// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QDialog>
#include <cstddef>
#include <functional>
#include <vector>

#include "horcom/data/aaf.hpp"

class QKeyEvent;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QHBoxLayout;
class QPushButton;

namespace horcom {

/// The record chooser of the original ausw_datei, one monospace row
/// per record under his fixed column header, records picked by click
/// in order until WAHL - ENDE.
class RecordListDialog : public QDialog {
  Q_OBJECT

 public:
  /// How the chooser behaves.
  enum class Mode {
    kSingle,  ///< one record, a double click takes it straight away
    kFetch,   ///< up to max_pick records for the free slots
    kDelete,  ///< up to ten records marked for LÖSCHEN
  };

  /// @param records    the collection in file order
  /// @param order      the display permutation from record_order
  /// @param file_label the file name for the title line
  /// @param max_pick   how many records may be taken
  /// @param mode       single pick, fetch or delete
  /// @param parent     the owner
  RecordListDialog(const std::vector<AafRecord>& records, const std::vector<std::size_t>& order,
                   const QString& file_label, int max_pick, Mode mode, QWidget* parent = nullptr);

  /// @return indices into the record vector in the order they were picked
  [[nodiscard]] const std::vector<std::size_t>& picked() const { return picked_; }

  /// Adds his NUR HOROSKOP ZEIGEN button, the marked record is shown
  /// without being taken, the list turns into a browser from then on.
  ///
  /// @param preview shows the chart of one record
  void set_preview(std::function<void(const AafRecord&)> preview);

 protected:
  void keyPressEvent(QKeyEvent* event) override;

 private:
  void toggle(QListWidgetItem* item);
  void double_click(QListWidgetItem* item);
  void exit_list();
  void search(const QString& text);
  void show_marked();

  std::vector<AafRecord> records_;
  std::vector<std::size_t> picked_;
  int max_pick_;
  Mode mode_;
  QListWidget* list_ = nullptr;
  QLineEdit* search_ = nullptr;
  QHBoxLayout* buttons_ = nullptr;
  QPushButton* done_ = nullptr;
  QPushButton* print_ = nullptr;
  std::function<void(const AafRecord&)> preview_;
  // his muster!, once browsing the list only shows charts
  bool browsing_ = false;
  // his ds&, the row of the last single click
  int marked_row_ = -1;
};

/// One chooser row exactly like the a2111 line, name, date, UT clock,
/// coordinates and place in his fixed widths.
///
/// @param r the record
/// @return the monospace row
[[nodiscard]] QString record_list_line(const AafRecord& r);

}  // namespace horcom
