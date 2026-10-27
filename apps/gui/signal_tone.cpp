// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "signal_tone.hpp"

#include <QApplication>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QSettings>

#include <cmath>
#include <optional>
#include <vector>

#include "horcom/core/constants.hpp"

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmsystem.h>
#endif

namespace horcom::tone {

namespace {

// the ping lasts this long, a key held down repeats faster and had cut
// every ping into a crackle, the next may start once it rang out
constexpr int kPingMs = 90;
constexpr int kRearmMs = kPingMs + 60;

QElapsedTimer& last_ping() {
  static QElapsedTimer timer;
  return timer;
}

std::uint64_t& count() {
  static std::uint64_t n = 0;
  return n;
}

// the switch as last read or set, the settings are asked once
std::optional<bool>& cached() {
  static std::optional<bool> on;
  return on;
}

// the test binaries draw offscreen and must stay quiet on the desk
bool headless() {
  const QString platform = QGuiApplication::platformName();
  return platform.startsWith(QLatin1String("offscreen")) || platform.startsWith(QLatin1String("minimal"));
}

#ifdef Q_OS_WIN
// a short sine with soft edges so neither start nor end clicks
constexpr int kSampleRate = 22050;
constexpr double kPingHz = 880.0;
constexpr int kFadeMs = 15;
constexpr double kPingLevel = 0.2;
// 16 bit mono samples
constexpr int kBytesPerSample = 2;
constexpr int kBitsPerSample = 8 * kBytesPerSample;
// the RIFF, fmt and data chunk heads before the samples
constexpr std::uint32_t kWaveHeadBytes = 44;
constexpr std::uint32_t kRiffHeadBytes = 8;
constexpr std::uint32_t kFmtChunkBytes = 16;
constexpr std::uint16_t kPcmFormat = 1;
constexpr std::uint16_t kMono = 1;

void put_u32(std::vector<char>& out, std::uint32_t v) {
  for (int i = 0; i < 4; ++i) {
    out.push_back(static_cast<char>((v >> (8 * i)) & 0xFF));
  }
}

void put_u16(std::vector<char>& out, std::uint16_t v) {
  out.push_back(static_cast<char>(v & 0xFF));
  out.push_back(static_cast<char>((v >> 8) & 0xFF));
}

// the wave of the ping, built once and never freed, an asynchronous play
// still reads it while the program closes
const std::vector<char>& ping_wave() {
  static const std::vector<char>* wave = []() {
    constexpr int kSamples = kSampleRate * kPingMs / 1000;
    constexpr int kFade = kSampleRate * kFadeMs / 1000;
    constexpr std::uint32_t kDataBytes = kSamples * kBytesPerSample;
    auto* out = new std::vector<char>();
    out->reserve(kWaveHeadBytes + kDataBytes);
    out->insert(out->end(), {'R', 'I', 'F', 'F'});
    put_u32(*out, kWaveHeadBytes - kRiffHeadBytes + kDataBytes);
    out->insert(out->end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    put_u32(*out, kFmtChunkBytes);
    put_u16(*out, kPcmFormat);
    put_u16(*out, kMono);
    put_u32(*out, kSampleRate);
    put_u32(*out, kSampleRate * kBytesPerSample);
    put_u16(*out, kBytesPerSample);
    put_u16(*out, kBitsPerSample);
    out->insert(out->end(), {'d', 'a', 't', 'a'});
    put_u32(*out, kDataBytes);
    for (int i = 0; i < kSamples; ++i) {
      // a raised cosine at both ends
      double envelope = 1.0;
      if (i < kFade) {
        envelope = 0.5 - 0.5 * std::cos(kPi * i / kFade);
      } else if (i >= kSamples - kFade) {
        envelope = 0.5 - 0.5 * std::cos(kPi * (kSamples - 1 - i) / kFade);
      }
      const double v = kPingLevel * envelope * std::sin(2.0 * kPi * kPingHz * i / kSampleRate);
      put_u16(*out, static_cast<std::uint16_t>(static_cast<std::int16_t>(std::lround(v * 32767.0))));
    }
    return out;
  }();
  return *wave;
}
#endif

}  // namespace

void ring() {
  if (!enabled()) {
    return;
  }
  QElapsedTimer& last = last_ping();
  if (last.isValid() && last.elapsed() < kRearmMs) {
    return;
  }
  last.start();
  ++count();
  if (headless()) {
    return;
  }
#ifdef Q_OS_WIN
  PlaySoundW(reinterpret_cast<LPCWSTR>(ping_wave().data()), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
#else
  QApplication::beep();
#endif
}

bool enabled() {
  std::optional<bool>& on = cached();
  if (!on) {
    on = QSettings().value(kEnabledKey, true).toBool();
  }
  return *on;
}

void set_enabled(bool on) {
  cached() = on;
  QSettings().setValue(kEnabledKey, on);
}

std::uint64_t rung() { return count(); }

}  // namespace horcom::tone
