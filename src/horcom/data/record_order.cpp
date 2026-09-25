// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/record_order.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

#include "horcom/data/statist_list.hpp"

namespace horcom {

// the vg| table of his QSORT, shared with the statistics lists. His names
// were capitals already, the keys of the port are made so
std::string collation_key(std::string_view utf8) {
  std::string out = stat_collation_key(std::string(utf8));
  for (char& c : out) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return out;
}

namespace {

std::string trim(const std::string& s) {
  const std::size_t a = s.find_first_not_of(' ');
  if (a == std::string::npos) {
    return {};
  }
  return s.substr(a, s.find_last_not_of(' ') - a + 1);
}

// the word splitter of a211_nnam, INSTR finding nothing keeps the
// whole rest like MID$(n$,1) did
std::string after_first_blank(const std::string& s) {
  const std::size_t p = s.find(' ');
  return p == std::string::npos ? s : s.substr(p + 1);
}

// ported from a211_nnam, the original split the 25 byte padded field,
// so a name without a third word keys on blanks and sorts to the top
std::string name_key(const std::string& name, RecordOrder order) {
  std::string n = collation_key(name);
  if (n.size() < 25) {
    n.resize(25, ' ');
  }
  const std::string b = after_first_blank(n);
  const std::string c = after_first_blank(b);
  switch (order) {
    case RecordOrder::kName123:
      return trim(n);
    case RecordOrder::kName23:
      return b.substr(0, 5) + c.substr(0, 5);
    case RecordOrder::kName3:
      return c.substr(0, 5);
    default:
      return {};
  }
}

// ported from a211
long date_key(const OrderKeySource& r, RecordOrder order) {
  if (order == RecordOrder::kBirthday) {
    // GEBURTSTAG
    return r.day + 31 * r.month;
  }
  // DATUM
  return std::lround(365.25 * ((r.year + r.month / 12.0 + r.day / 365.25) + 4713.0));
}

}  // namespace

std::vector<std::size_t> record_order(const std::vector<OrderKeySource>& records, RecordOrder order) {
  std::vector<std::size_t> idx(records.size());
  for (std::size_t i = 0; i < idx.size(); ++i) {
    idx[i] = i;
  }
  if (order == RecordOrder::kFile || records.empty()) {
    return idx;
  }
  if (order == RecordOrder::kBirthday || order == RecordOrder::kDate) {
    std::vector<long> key(records.size());
    for (std::size_t i = 0; i < records.size(); ++i) {
      key[i] = date_key(records[i], order);
    }
    std::stable_sort(idx.begin(), idx.end(), [&key](std::size_t a, std::size_t b) { return key[a] < key[b]; });
    return idx;
  }
  std::vector<std::string> key(records.size());
  for (std::size_t i = 0; i < records.size(); ++i) {
    key[i] = name_key(records[i].name, order);
  }
  std::stable_sort(idx.begin(), idx.end(), [&key](std::size_t a, std::size_t b) { return key[a] < key[b]; });
  return idx;
}

}  // namespace horcom
