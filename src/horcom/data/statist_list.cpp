// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "horcom/data/statist_list.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "horcom/chart/bodies.hpp"
#include "horcom/time/calendar.hpp"

namespace horcom {

namespace {

// the characters of a UTF-8 string, one entry per code point
std::vector<std::string> code_points(const std::string& s) {
  std::vector<std::string> out;
  for (std::size_t i = 0; i < s.size();) {
    const auto c = static_cast<unsigned char>(s[i]);
    std::size_t n = 1;
    if (c >= 0xF0) {
      n = 4;
    } else if (c >= 0xE0) {
      n = 3;
    } else if (c >= 0xC0) {
      n = 2;
    }
    out.push_back(s.substr(i, n));
    i += n;
  }
  return out;
}

std::string join(const std::vector<std::string>& cps, std::size_t from, std::size_t count) {
  std::string out;
  for (std::size_t i = from; i < cps.size() && i < from + count; ++i) {
    out += cps[i];
  }
  return out;
}

// GFA's LEFT$ and RIGHT$ on characters
std::string left(const std::string& s, std::size_t n) {
  return join(code_points(s), 0, n);
}

std::string right(const std::string& s, std::size_t n) {
  const std::vector<std::string> cps = code_points(s);
  const std::size_t from = cps.size() > n ? cps.size() - n : 0;
  return join(cps, from, n);
}

std::size_t length(const std::string& s) {
  return code_points(s).size();
}

// GFA's VAL, the leading number of a string, blanks before it skipped
int val(const std::string& s) {
  return std::atoi(s.c_str());
}

// STR$(n,3), right aligned in three places
std::string str3(int n) {
  char buf[16];
  std::snprintf(buf, sizeof(buf), "%3d", n);
  return buf;
}

// his uindo, the position of the first chain mark, 28 without one
std::size_t uindo(const std::string& label) {
  const std::vector<std::string> cps = code_points(label);
  for (std::size_t i = 0; i < cps.size(); ++i) {
    if (cps[i] == "u") {
      const std::size_t pos = i + 1;
      return pos > 2 && pos < kStatLabelWidth ? pos : kStatLabelWidth;
    }
  }
  return kStatLabelWidth;
}

// the condition number that ends a label, after its last chain mark
int last_condition(const std::string& label) {
  const std::size_t u = label.rfind('u');
  return val(u == std::string::npos ? label.substr(label.size() > 3 ? label.size() - 3 : 0) : label.substr(u + 1));
}

}  // namespace

// the n$ field of stat2, LSET into 25 places
std::string stat_padded_name(const std::string& name) {
  std::string out = left(name, kStatNameWidth);
  for (std::size_t n = length(out); n < kStatNameWidth; ++n) {
    out += ' ';
  }
  return out;
}

StatList::StatList(const StatSet& set) {
  // ported from regg, m$(i&) = n$(i&)
  for (const StatRecord& r : set.records) {
    names_.push_back(stat_padded_name(r.name));
  }
  last_ = names_;
}

// ported from bed_erf_1 and bed_erf_2
void StatList::add(const StatEvalResult& r, int condition, StatJoin join) {
  if (join == StatJoin::kAndExclusive && !exclusive_) {
    // IF zdms& = 0 : zdms& = zdm&, then CLR zdm&, the list starts over
    before_exclusive_ = static_cast<int>(entries_.size());
  }
  if (join == StatJoin::kAndExclusive) {
    entries_.clear();
    exclusive_ = true;
  }
  const bool chain = join == StatJoin::kAndInclusive || join == StatJoin::kAndExclusive;
  chained_ = chained_ || chain;
  for (const StatMatch& m : r.matches) {
    const auto rec = static_cast<std::size_t>(m.record);
    StatEntry e{m.record, m.slot, m.value, condition, {}, false};
    if (!chain) {
      // so$(zdm&) = n$ + STR$(anzb&,3)
      e.label = names_[rec] + str3(condition);
    } else {
      // ll& = @uindo(m$(i&)), r$ = RIGHT$(m$(i&),28 - ll& + 2)
      const std::string& was = last_[rec];
      const std::size_t ll = uindo(was);
      const std::string t = right(was, kStatLabelWidth - ll + 2) + "u" + std::to_string(condition);
      if (val(left(t, 2)) != 0) {
        // a$ = LEFT$(n$,28 - t& - 1), so$(zdm&) = a$ + " " + t$
        const std::size_t keep = kStatLabelWidth > length(t) + 1 ? kStatLabelWidth - length(t) - 1 : 0;
        e.label = left(names_[rec], keep) + " " + t;
      } else if (join == StatJoin::kAndInclusive) {
        e.label = names_[rec] + str3(condition);
      } else {
        // DEC zdm&, EXKLUSIV keeps only the chains
        continue;
      }
    }
    last_[rec] = e.label;
    entries_.push_back(std::move(e));
  }
}

// ported from usuch. He compared VAL(RIGHT$(d$)), the last digit alone,
// with the condition number, from the tenth condition on no chain ever
// counted as complete, the port reads the whole number after the mark
int StatList::mark_complete(int condition) {
  for (StatEntry& e : entries_) {
    e.complete = false;
  }
  if (!chained_) {
    return 0;
  }
  std::size_t ll = kStatLabelWidth;
  for (const StatEntry& e : entries_) {
    ll = std::min(ll, uindo(e.label));
  }
  int marked = 0;
  for (StatEntry& e : entries_) {
    if (uindo(e.label) == ll && ll < kStatLabelWidth && last_condition(e.label) == condition) {
      e.complete = true;
      ++marked;
    }
  }
  return marked;
}

std::string stat_collation_key(const std::string& s) {
  // x$ ="ÄÖÜß", y$ = "AOUS"
  std::string out;
  for (const std::string& cp : code_points(s)) {
    if (cp == "\xC3\x84" || cp == "\xC3\xA4") {
      out += 'A';
    } else if (cp == "\xC3\x96" || cp == "\xC3\xB6") {
      out += 'O';
    } else if (cp == "\xC3\x9C" || cp == "\xC3\xBC") {
      out += 'U';
    } else if (cp == "\xC3\x9F") {
      out += 'S';
    } else {
      out += cp;
    }
  }
  return out;
}

// ported from the QSORT calls of list_ausg
void sort_stat_entries(std::vector<StatEntry>& entries, StatSort sort) {
  if (sort == StatSort::kByValue) {
    std::stable_sort(entries.begin(), entries.end(),
                     [](const StatEntry& a, const StatEntry& b) { return a.value < b.value; });
  } else if (sort == StatSort::kByName) {
    std::stable_sort(entries.begin(), entries.end(), [](const StatEntry& a, const StatEntry& b) {
      return stat_collation_key(a.label) < stat_collation_key(b.label);
    });
  }
}

// ported from list_zeil_loe
bool stat_out_of_range(const StatRecord& r, int slot) {
  if (slot < body::kApogee || r.el[static_cast<std::size_t>(slot)] <= 0.0) {
    return false;
  }
  struct Range {
    int slot;
    double from;
    double to;
  };
  // the coverage of his ephemeris files in julian days
  static constexpr Range kRanges[] = {
      {body::kChiron, 1502279.5, 2524460.5}, {body::kCeres, 2268939.5, 2488390.5},
      {body::kPallas, 2268939.5, 2488390.5}, {body::kJuno, 2268939.5, 2488390.5},
      {body::kVesta, 2268939.5, 2488390.5},  {body::kQuaoar, 1502079.5, 2525120.5},
      {body::kHalley, 2305446.5, 2469806.5}, {body::kPholus, 2268939.5, 2488390.5},
      {body::kDamokles, 2268939.5, 2488390.5}, {body::kNessus, 2268939.5, 2488390.5},
      {body::kXena, 1502079.5, 2525120.5}};
  // ta = as%(m%,1), mo, ja, ho, mi, @juld
  const double jd = julian_day({r.day, r.month, r.year, static_cast<double>(r.hour), r.minute});
  for (const Range& g : kRanges) {
    if (g.slot == slot) {
      return jd < g.from || jd > g.to;
    }
  }
  return false;
}

}  // namespace horcom
