// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#define DOCTEST_CONFIG_IMPLEMENT
#include <QApplication>
#include <QSettings>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string>
#include <system_error>
#include <thread>

#include "doctest.h"
#include "kommen_dialog.hpp"
#include "layout_check.hpp"
#include "theme.hpp"

namespace {

// the slowest case runs some four seconds here and four times that on the
// CI runners, HORCOM_CASE_LIMIT_S overrides the limit and 0 switches it off
constexpr int kCaseLimitSeconds = 60;

/// Ends the run when a test case outlasts its limit.
///
/// @note A dialog a test fails to answer waits forever in its own event
/// loop. Without the watchdog ctest waits out its whole timeout and names
/// no case, with it the run stops at once and names the case that hangs.
class Watchdog final : public doctest::IReporter {
 public:
  /// Starts the watching thread.
  ///
  /// @param options the options of the run, unused
  explicit Watchdog(const doctest::ContextOptions& options) {
    static_cast<void>(options);
    bool ok = false;
    const int limit = qEnvironmentVariableIntValue("HORCOM_CASE_LIMIT_S", &ok);
    limit_ = std::chrono::seconds(ok ? limit : kCaseLimitSeconds);
    if (limit_.count() > 0) {
      thread_ = std::thread([this]() { watch(); });
    }
  }

  ~Watchdog() override {
    {
      const std::lock_guard lock(mutex_);
      stop_ = true;
    }
    wake_.notify_all();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  Watchdog(const Watchdog&) = delete;
  Watchdog& operator=(const Watchdog&) = delete;

  void report_query(const doctest::QueryData&) override {}
  void test_run_start() override {}
  void test_run_end(const doctest::TestRunStats&) override {}
  void test_case_start(const doctest::TestCaseData& tc) override { arm(tc.m_name); }
  void test_case_reenter(const doctest::TestCaseData& tc) override { arm(tc.m_name); }
  void test_case_end(const doctest::CurrentTestCaseStats&) override { arm(nullptr); }
  void test_case_exception(const doctest::TestCaseException&) override {}
  void subcase_start(const doctest::SubcaseSignature&) override {}
  void subcase_end() override {}
  void log_assert(const doctest::AssertData&) override {}
  void log_message(const doctest::MessageData&) override {}
  void test_case_skipped(const doctest::TestCaseData&) override {}

 private:
  using Clock = std::chrono::steady_clock;

  void arm(const char* name) {
    {
      const std::lock_guard lock(mutex_);
      name_ = name;
      since_ = Clock::now();
    }
    wake_.notify_all();
  }

  void watch() {
    std::unique_lock lock(mutex_);
    while (!stop_) {
      if (name_ == nullptr) {
        wake_.wait(lock);
        continue;
      }
      const Clock::time_point deadline = since_ + limit_;
      wake_.wait_until(lock, deadline);
      if (!stop_ && name_ != nullptr && Clock::now() >= since_ + limit_) {
        std::cout.flush();
        std::fprintf(stderr, "\n[watchdog] \"%s\" still runs after %lld s, a dialog waits unanswered\n", name_,
                     static_cast<long long>(limit_.count()));
        std::fflush(stderr);
        std::_Exit(EXIT_FAILURE);
      }
    }
  }

  std::chrono::seconds limit_{};
  std::mutex mutex_;
  std::condition_variable wake_;
  const char* name_ = nullptr;
  Clock::time_point since_;
  bool stop_ = false;
  std::thread thread_;
};

REGISTER_LISTENER("watchdog", 1, Watchdog);

}  // namespace

int main(int argc, char** argv) {
  // the suites run headless however they are started, a dialog a test
  // fails to answer must never wait on the desktop of whoever runs them
  if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
  }
  // each process keeps its settings and scratch files in a folder of its
  // own, ctest runs the shards of the suite side by side. The folder sits
  // in the system temp in the form the system names it, the 8.3 form of
  // the CI runners included
  const std::filesystem::path scratch =
      std::filesystem::temp_directory_path() / ("horcom_gui_tests_" + std::to_string(QCoreApplication::applicationPid()));
  std::error_code ec;
  std::filesystem::remove_all(scratch, ec);
  std::filesystem::create_directories(scratch);
  const QByteArray scratch_env = QString::fromStdWString(scratch.wstring()).toLocal8Bit();
  for (const char* name : {"TMP", "TEMP", "TMPDIR"}) {
    qputenv(name, scratch_env);
  }
  // a settings home of its own, the tests never touch the user's choices
  // and every run starts from empty settings
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QString::fromStdWString(scratch.wstring()));
  QApplication app(argc, argv);
  QApplication::setApplicationName("horcom-gui-tests");
  QApplication::setOrganizationName("horcom-gui-tests");
  // the German edition whatever the locale of the machine, the English
  // tests switch it themselves
  app.setProperty(horcom::kEnglishEditionProperty, false);
  // HORCOM_SHOTS captures and checks every dialog the flows open,
  // HORCOM_THEME dresses them in the style sheet of the program so the
  // sweep measures the cells as the program shows them, a number there
  // is the text scale in percent
  horcom::test::LayoutSweep sweep;
  if (qEnvironmentVariableIsSet("HORCOM_SHOTS")) {
    app.installEventFilter(&sweep);
  }
  if (qEnvironmentVariableIsSet("HORCOM_THEME")) {
    bool ok = false;
    const int percent = qEnvironmentVariableIntValue("HORCOM_THEME", &ok);
    horcom::theme::apply(ok && percent > 0 ? percent : horcom::theme::kTextScaleNormal, false);
  }
  doctest::Context context(argc, argv);
  const int result = context.run();
  std::filesystem::remove_all(scratch, ec);
  return result;
}
