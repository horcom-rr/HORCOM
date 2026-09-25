// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#define DOCTEST_CONFIG_IMPLEMENT
#include <QApplication>
#include <QSettings>

#include "doctest.h"
#include "kommen_dialog.hpp"
#include "layout_check.hpp"
#include "theme.hpp"

int main(int argc, char** argv) {
  // the suites run headless however they are started, a dialog a test
  // fails to answer must never wait on the desktop of whoever runs them
  if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
  }
  QApplication app(argc, argv);
  // a settings home of its own, the tests never touch the user's choices
  QApplication::setApplicationName("horcom-gui-tests");
  QApplication::setOrganizationName("horcom-gui-tests");
  // every run starts from empty settings, a choice one run left behind
  // must not steer the next
  QSettings().clear();
  // the German edition whatever the locale of the machine, the English
  // tests switch it themselves
  app.setProperty(horcom::kEnglishEditionProperty, false);
  // HORCOM_SHOTS captures and checks every dialog the flows open,
  // HORCOM_THEME dresses them in the style sheet of the program so the
  // sweep measures the cells as the program shows them
  horcom::test::LayoutSweep sweep;
  if (qEnvironmentVariableIsSet("HORCOM_SHOTS")) {
    app.installEventFilter(&sweep);
  }
  if (qEnvironmentVariableIsSet("HORCOM_THEME")) {
    horcom::theme::apply(horcom::theme::kTextScaleNormal, false);
  }
  doctest::Context context(argc, argv);
  return context.run();
}
