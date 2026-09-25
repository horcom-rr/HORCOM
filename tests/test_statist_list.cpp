// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#include "doctest.h"
#include "horcom/chart/bodies.hpp"
#include "horcom/core/constants.hpp"
#include "horcom/data/statist_list.hpp"

using namespace horcom;

namespace {

// three invented charts, no real people
StatSet sample() {
  StatSet set;
  const char* names[3] = {"ANNA MUSTER", "BERND BEISPIEL", "ÄGIDIUS DEMO"};
  for (int i = 0; i < 3; ++i) {
    StatRecord r;
    r.name = names[i];
    r.day = 1;
    r.month = 1;
    r.year = 1970 + i;
    r.el[body::kSun] = (10.0 + 100.0 * i) * kDegToRad;
    r.el[body::kMoon] = (20.0 + 100.0 * i) * kDegToRad;
    set.records.push_back(r);
  }
  return set;
}

StatEvalResult matches(std::initializer_list<int> records, int slot = body::kSun) {
  StatEvalResult r;
  for (const int i : records) {
    r.matches.push_back({i, slot, 0.1 * (i + 1)});
  }
  return r;
}

}  // namespace

TEST_CASE("bed_erf_1 labels the first and the ODER conditions with their number") {
  StatList list(sample());
  list.add(matches({0, 2}), 1, StatJoin::kFirst);
  list.add(matches({1}), 2, StatJoin::kOr);
  REQUIRE(list.entries().size() == 3);
  //RR so$ = n$ + STR$(anzb&,3), the name padded to 25 places
  CHECK(list.entries()[0].label == "ANNA MUSTER                1");
  CHECK(list.entries()[2].label == "BERND BEISPIEL             2");
  CHECK(list.entries()[0].label.size() == kStatLabelWidth);
  CHECK(list.mark_complete(2) == 0);
}

TEST_CASE("UND INKLUSIV chains the conditions a record met and keeps the earlier entries") {
  StatList list(sample());
  list.add(matches({0, 1}), 1, StatJoin::kFirst);
  list.add(matches({0}), 2, StatJoin::kAndInclusive);
  REQUIRE(list.entries().size() == 3);
  //RR a$ = LEFT$(n$,28 - t& - 1) + " " + t$, t$ = " 1u2"
  CHECK(list.entries()[2].label == stat_padded_name("ANNA MUSTER").substr(0, 23) + "  1u2");
  list.add(matches({0}), 3, StatJoin::kAndInclusive);
  CHECK(list.entries()[3].label == stat_padded_name("ANNA MUSTER").substr(0, 21) + "  1u2u3");
  CHECK(list.entries()[3].label.size() == kStatLabelWidth);
  // the entry that ends the longest chain is complete, usuch
  CHECK(list.mark_complete(3) == 1);
  CHECK(list.entries()[3].complete);
  CHECK_FALSE(list.entries()[2].complete);
}

TEST_CASE("UND EXKLUSIV cuts the list down to the chains") {
  StatList list(sample());
  list.add(matches({0, 1, 2}), 1, StatJoin::kFirst);
  list.add(matches({2}), 2, StatJoin::kAndExclusive);
  //RR zdms& = zdm&, CLR zdm&
  CHECK(list.before_exclusive() == 3);
  REQUIRE(list.entries().size() == 1);
  CHECK(list.entries()[0].record == 2);
  CHECK(list.exclusive());
}

TEST_CASE("usuch sees the complete chains from the tenth condition on") {
  // his VAL(RIGHT$(d$)) read the last digit alone
  StatList list(sample());
  list.add(matches({0}), 1, StatJoin::kFirst);
  for (int n = 2; n <= 10; ++n) {
    list.add(matches({0}), n, StatJoin::kAndInclusive);
  }
  CHECK(list.mark_complete(10) == 1);
  CHECK(list.entries().back().complete);
}

TEST_CASE("the list sorts by his umlaut folding or by value") {
  StatList list(sample());
  list.add(matches({2, 1, 0}), 1, StatJoin::kFirst);
  std::vector<StatEntry> byname = list.entries();
  sort_stat_entries(byname, StatSort::kByName);
  //RR Ä sorts as A, ÄGIDIUS before ANNA
  CHECK(byname[0].record == 2);
  CHECK(byname[1].record == 0);
  std::vector<StatEntry> byvalue = list.entries();
  sort_stat_entries(byvalue, StatSort::kByValue);
  CHECK(byvalue[0].record == 0);
  CHECK(byvalue[2].record == 2);
  CHECK(stat_collation_key("M\xC3\x9CLLER") == "MULLER");
}

TEST_CASE("list_zeil_loe masks an extra outside its ephemeris file") {
  StatRecord r;
  r.day = 1;
  r.month = 1;
  r.year = 1400;
  r.el[body::kCeres] = 1.0;
  r.el[body::kChiron] = 1.0;
  //RR CE from 2268939.5, Chiron from 1502279.5
  CHECK(stat_out_of_range(r, body::kCeres));
  CHECK_FALSE(stat_out_of_range(r, body::kChiron));
  CHECK_FALSE(stat_out_of_range(r, body::kSun));
}

TEST_CASE("a lone blank finds every name shorter than the field, the alphabetical list of KOMMSTAT") {
  StatSet set = sample();
  set.records[1].name = "ABCDEFGHIJKLMNOPQRSTUVWXY";
  StatQuery q;
  q.object = StatObject::kName;
  q.name = " ";
  std::vector<double> mask;
  const StatEvalResult r = evaluate_statistics(set, q, AspectSettings{}, mask);
  REQUIRE(r.matches.size() == 2);
  CHECK(r.matches[0].record == 0);
  CHECK(r.matches[1].record == 2);
}
