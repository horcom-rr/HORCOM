// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1970s to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/gfa_stream.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace horcom {

GfaReader::GfaReader(std::string_view text) {
  std::size_t i = 0;
  const auto skip_separators = [&]() {
    while (i < text.size() && (text[i] == ',' || text[i] == '\r' || text[i] == '\n' || text[i] == ' ')) {
      ++i;
    }
  };
  skip_separators();
  while (i < text.size()) {
    std::string tok;
    if (text[i] == '"') {
      ++i;
      while (i < text.size() && text[i] != '"') {
        tok += text[i];
        ++i;
      }
      if (i < text.size()) {
        ++i;  // closing quote
      }
    } else {
      while (i < text.size() && text[i] != ',' && text[i] != '\r' && text[i] != '\n') {
        tok += text[i];
        ++i;
      }
    }
    tokens_.push_back(std::move(tok));
    skip_separators();
  }
}

std::optional<std::string> GfaReader::next() {
  if (pos_ >= tokens_.size()) {
    return std::nullopt;
  }
  return tokens_[pos_++];
}

double GfaReader::next_number() {
  const auto t = next();
  if (!t) {
    return 0.0;
  }
  return std::strtod(t->c_str(), nullptr);
}

std::string GfaReader::next_string() {
  const auto t = next();
  return t ? *t : std::string();
}

std::string gfa_write_line(const std::vector<std::string>& fields) {
  std::string out;
  for (std::size_t i = 0; i < fields.size(); ++i) {
    if (i > 0) {
      out += ',';
    }
    out += fields[i];
  }
  return out;
}

std::string gfa_number(double v) {
  if (v == std::floor(v) && std::abs(v) < 1e15) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(v));
    return buf;
  }
  char buf[40];
  std::snprintf(buf, sizeof(buf), "%.15g", v);
  return buf;
}

std::string gfa_quoted(std::string_view s) {
  std::string out = "\"";
  out += s;
  out += '"';
  return out;
}

}  // namespace horcom
