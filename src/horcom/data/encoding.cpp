// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/encoding.hpp"

#include <array>
#include <cstdint>

namespace horcom {

namespace {

// code points of Windows 1252 bytes 0x80 to 0x9F, 0 marks undefined
constexpr std::array<std::uint16_t, 32> kHighControls = {
    0x20AC, 0,      0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0,      0x017D, 0,
    0,      0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0,      0x017E, 0x0178};

std::uint32_t cp1252_code_point(unsigned char b) {
  if (b < 0x80) {
    return b;
  }
  if (b < 0xA0) {
    const std::uint16_t cp = kHighControls[b - 0x80];
    return cp != 0 ? cp : b;
  }
  return b;  // 0xA0 to 0xFF match Latin-1
}

// the German letters of the Atari ST set that his texts use, the rest
// of 0x80 to 0x9F falls back to Windows 1252
std::uint32_t atari_code_point(unsigned char b) {
  switch (b) {
    case 0x81: return 0x00FC;  // ü
    case 0x83: return 0x00E2;  // â
    case 0x84: return 0x00E4;  // ä
    case 0x8E: return 0x00C4;  // Ä
    case 0x94: return 0x00F6;  // ö
    case 0x99: return 0x00D6;  // Ö
    case 0x9A: return 0x00DC;  // Ü
    case 0x9E: return 0x00DF;  // ß
    default: return cp1252_code_point(b);
  }
}

void append_utf8(std::string& out, std::uint32_t cp) {
  if (cp < 0x80) {
    out += static_cast<char>(cp);
  } else if (cp < 0x800) {
    out += static_cast<char>(0xC0 | (cp >> 6));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  } else {
    out += static_cast<char>(0xE0 | (cp >> 12));
    out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  }
}

}  // namespace

std::string cp1252_to_utf8(std::string_view in) {
  std::string out;
  out.reserve(in.size());
  for (const char ch : in) {
    append_utf8(out, cp1252_code_point(static_cast<unsigned char>(ch)));
  }
  return out;
}

std::string atari_cp1252_to_utf8(std::string_view in) {
  std::string out;
  out.reserve(in.size());
  for (const char ch : in) {
    append_utf8(out, atari_code_point(static_cast<unsigned char>(ch)));
  }
  return out;
}

bool looks_like_utf8(std::string_view text) {
  std::size_t i = 0;
  while (i < text.size()) {
    const auto b = static_cast<unsigned char>(text[i]);
    std::size_t follow = 0;
    if (b < 0x80) {
      follow = 0;
    } else if ((b & 0xE0) == 0xC0) {
      follow = 1;
    } else if ((b & 0xF0) == 0xE0) {
      follow = 2;
    } else if ((b & 0xF8) == 0xF0) {
      follow = 3;
    } else {
      return false;
    }
    for (std::size_t k = 1; k <= follow; ++k) {
      if (i + k >= text.size() || (static_cast<unsigned char>(text[i + k]) & 0xC0) != 0x80) {
        return false;
      }
    }
    i += follow + 1;
  }
  return true;
}

std::string utf8_to_cp1252(std::string_view in) {
  std::string out;
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size();) {
    const auto b0 = static_cast<unsigned char>(in[i]);
    std::uint32_t cp = 0;
    std::size_t len = 1;
    if (b0 < 0x80) {
      cp = b0;
    } else if ((b0 & 0xE0) == 0xC0 && i + 1 < in.size()) {
      cp = static_cast<std::uint32_t>(b0 & 0x1F) << 6 | (static_cast<unsigned char>(in[i + 1]) & 0x3F);
      len = 2;
    } else if ((b0 & 0xF0) == 0xE0 && i + 2 < in.size()) {
      cp = static_cast<std::uint32_t>(b0 & 0x0F) << 12 |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(in[i + 1]) & 0x3F) << 6) |
           (static_cast<unsigned char>(in[i + 2]) & 0x3F);
      len = 3;
    } else {
      cp = '?';
      len = 1;
    }
    i += len;
    if (cp < 0x80 || (cp >= 0xA0 && cp <= 0xFF)) {
      out += static_cast<char>(cp);
      continue;
    }
    char mapped = '?';
    for (std::size_t k = 0; k < kHighControls.size(); ++k) {
      if (kHighControls[k] == cp) {
        mapped = static_cast<char>(0x80 + k);
        break;
      }
    }
    out += mapped;
  }
  return out;
}

}  // namespace horcom
