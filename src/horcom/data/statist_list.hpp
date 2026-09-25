// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "horcom/data/statist.hpp"
#include "horcom/data/statist_eval.hpp"

// The output list of stat2. Every match of every condition becomes one
// entry like his so$, cc% and mz% arrays. The label carries the record's
// name and the condition it met, under UND the chain of conditions.
namespace horcom {

/// How a condition joins the ones before it, the rows of his od_un box.
enum class StatJoin { kFirst, kOr, kAndInclusive, kAndExclusive };

/// One line of the output list.
struct StatEntry {
  /// cc%, the record index into the dataset
  int record = 0;
  /// mz|, the body behind the value, 0 for a derived value
  int slot = 0;
  /// the tested longitude or the aspect separation in radians
  double value = 0.0;
  /// the condition that added the entry, one based
  int condition = 0;
  /// so$, the name with the condition number or the UND chain
  std::string label;
  /// su&, the entry ends the longest UND chain of the last condition
  bool complete = false;
};

/// The label width of bed_erf_1, name plus the chain.
inline constexpr std::size_t kStatLabelWidth = 28;

/// The entry list of one evaluation, his so$ and m$ bookkeeping.
class StatList {
 public:
  /// Starts over like his regg, every record carries its bare name.
  ///
  /// @param set the dataset whose records the entries point to
  explicit StatList(const StatSet& set);

  /// Adds the matches of one condition, bed_erf_2 with bed_erf_1.
  ///
  /// @param r         the matches of the condition
  /// @param condition its number, one based
  /// @param join      how it joins the conditions before
  void add(const StatEvalResult& r, int condition, StatJoin join);

  /// Marks the entries that end the longest UND chain, his usuch.
  ///
  /// @param condition the number of the last condition
  /// @return the entries marked, his zdmu&
  int mark_complete(int condition);

  /// @return the entries in list order
  [[nodiscard]] const std::vector<StatEntry>& entries() const { return entries_; }
  /// @return the entries for sorting
  [[nodiscard]] std::vector<StatEntry>& entries() { return entries_; }
  /// @return his zdms&, the entries counted before the first EXKLUSIV cut
  [[nodiscard]] int before_exclusive() const { return before_exclusive_; }
  /// @return true once an UND joined, his odu$ = "UND  "
  [[nodiscard]] bool chained() const { return chained_; }
  /// @return true once an UND EXKLUSIV cut the list, his odex!
  [[nodiscard]] bool exclusive() const { return exclusive_; }

 private:
  std::vector<std::string> names_;
  std::vector<std::string> last_;
  std::vector<StatEntry> entries_;
  int before_exclusive_ = 0;
  bool chained_ = false;
  bool exclusive_ = false;
};

/// The order of list_ausg.
enum class StatSort {
  kByValue,  ///< OHNE EINSCHRÄNKUNG, ascending by the value
  kNone,     ///< SO / MO / AC and ALLE PLANETEN keep their order
  kByName,   ///< every other list by its label
};

/// Sorts a list like list_ausg.
///
/// @param entries the entries
/// @param sort    the order
void sort_stat_entries(std::vector<StatEntry>& entries, StatSort sort);

/// The collation of his vg| table, Ä, Ö and Ü sort as A, O and U, ß as S.
///
/// @param s a label in UTF-8
/// @return the key the byte order sorts by
[[nodiscard]] std::string stat_collation_key(const std::string& s);

/// The name of a record padded to his n$ field.
///
/// @param name the stored name
/// @return the name cut or padded to kStatNameWidth characters
[[nodiscard]] std::string stat_padded_name(const std::string& name);

/// His list_zeil_loe, an extra body standing outside its ephemeris file
/// at the date of the record.
///
/// @param r    the record
/// @param slot the body slot of the entry
/// @return true when the value must not be shown
[[nodiscard]] bool stat_out_of_range(const StatRecord& r, int slot);

}  // namespace horcom
