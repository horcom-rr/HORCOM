// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

// The sort modes of the record chooser, ported from a210, a211 and
// a211_nnam. The original built key arrays and QSORTed a permutation,
// the port returns the permutation directly.
namespace horcom {

/// The choices of his SORTIER-MODUS box plus the plain file order.
enum class RecordOrder {
  kFile,      ///< the order on disk
  kName123,   ///< ALPHABETISCH 1., 2. und 3. NAME
  kName23,    ///< ALPHABETISCH 2. und 3. NAME
  kName3,     ///< ALPHABETISCH nur 3. NAME
  kBirthday,  ///< GEBURTSTAG, day inside the year
  kDate,      ///< DATUM, the full date
};

/// What the key builder needs of one record.
struct OrderKeySource {
  std::string name;  ///< the full name, words separated by blanks
  int day = 0;
  int month = 0;
  int year = 0;
};

/// The sort key of a name under his QSORT collation table vg|, capitals
/// with Ä, Ö and Ü sorting as A, O and U and ß as S.
///
/// @param utf8 the name
/// @return the key, compared bytewise
[[nodiscard]] std::string collation_key(std::string_view utf8);

/// Builds the display permutation for the chooser list.
///
/// The name modes key on the words of the name like a211_nnam, the
/// second and third mode truncate each word key to five characters like
/// the original did. The keys run through collation_key like his QSORT
/// WITH vg|.
///
/// @param records the key sources in file order
/// @param order   the chosen mode
/// @return indices into records in display order
[[nodiscard]] std::vector<std::size_t> record_order(const std::vector<OrderKeySource>& records, RecordOrder order);

}  // namespace horcom
