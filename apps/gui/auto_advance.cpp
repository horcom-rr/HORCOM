// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "auto_advance.hpp"

#include <QEvent>
#include <QLineEdit>

namespace horcom {

namespace {

// his profile starts with tabstop& = 0, the automatic advance
bool g_auto_advance = true;

// the property his mousestop!(diha&,m&) became, set once a field is
// entered with the mouse
constexpr const char* kMouseStop = "horcomMouseStop";

// marks a field the user clicked into, it then keeps the focus when full
class MouseStop : public QObject {
 public:
  using QObject::QObject;

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override {
    if (event->type() == QEvent::MouseButtonPress) {
      watched->setProperty(kMouseStop, true);
    }
    return false;
  }
};

}  // namespace

void set_auto_advance(bool on) {
  g_auto_advance = on;
}

bool auto_advance() {
  return g_auto_advance;
}

// ported from gettext and gettextaaf
void chain_fields(const std::vector<std::pair<QLineEdit*, int>>& fields) {
  if (!g_auto_advance || fields.size() < 2) {
    return;
  }
  auto* stop = new MouseStop(fields.front().first);
  for (std::size_t i = 0; i + 1 < fields.size(); ++i) {
    QLineEdit* field = fields[i].first;
    QLineEdit* next = fields[i + 1].first;
    const int length = fields[i].second;
    field->installEventFilter(stop);
    //RR IF LEN(fl$) = e| ... ~SetFocus(DLGITEM(diha&,dial_i& + 1))
    QObject::connect(field, &QLineEdit::textEdited, next, [field, next, length](const QString& text) {
      if (length > 0 && text.size() >= length && !field->property(kMouseStop).toBool()) {
        next->setFocus(Qt::TabFocusReason);
        next->selectAll();
      }
    });
  }
}

}  // namespace horcom
