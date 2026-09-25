// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "horcom/data/aaf.hpp"
#include "horcom/data/chart_file.hpp"

// The Daten-Datei pair of the original, a SPEZIAL .DAT collection and
// its AAF twin. The AAF file is the pilot, aaf_horcom2 rebuilds the DAT
// from it, horcom_aaf3 lifts DAT records into the exchange form, and
// aaf_ident finds the AAF record a DAT name stands for.
namespace horcom {

/// Surname and given name joined by one blank like TRIM$(aaf$(1) + " " +
/// aaf$(2)) of aaf_horcom2. Both parts are trimmed and a lone star, the
/// empty field of the format, counts as no given name.
///
/// @param r the record
/// @return the joined name, not uppercased
[[nodiscard]] std::string record_name(const AafRecord& r);

/// A DAT text field like UPPER$(LEFT$(x,n)) over Windows 1252. The small
/// letters of the code page become capitals, ß has none and stays.
///
/// @param utf8   the text
/// @param length the field length in bytes, npos keeps the whole text
/// @return the capitals, cut to the field, UTF-8 again
[[nodiscard]] std::string dat_field(std::string_view utf8, std::size_t length = std::string::npos);

/// One 128 byte record from the exchange form, ported from aaf_horcom2.
/// The clock becomes UT in the record's own calendar, the calendar flag
/// rides in front of the remark, the nation follows the place.
///
/// @param r the AAF record
/// @return the DAT record
[[nodiscard]] ChartRecord chart_record_from_aaf(const AafRecord& r);

/// A DAT record lifted into the exchange form, the clock already UT. The
/// name splits at its first blank like horcom_aaf3, a single word keeps
/// no given name.
///
/// @param c the DAT record
/// @return the AAF record with the zone 00hE00:00
[[nodiscard]] AafRecord aaf_from_chart_record(const ChartRecord& c);

/// A DAT record as the exporter horcom_aaf3 wrote it. The place falls back
/// to NICHT GENANNT ! and keeps 17 letters before any star, the calendar
/// flag leaves the remark for the year letter, the zone name is GMT.
///
/// @param c the DAT record
/// @return the AAF record
[[nodiscard]] AafRecord aaf_export_record(const ChartRecord& c);

/// The AAF record a DAT name stands for, ported from aaf_ident. The names
/// meet in 25 capitals. His second rule, a surname inside the DAT name
/// with a given name of one letter like his star, could hit a longer
/// name first, the port looks for the exact name through the whole file
/// before it falls back on that rule.
///
/// @param aaf      the AAF records
/// @param dat_name the name field of the DAT record
/// @return the index of the record, nothing when none matches
[[nodiscard]] std::optional<std::size_t> aaf_ident(const std::vector<AafRecord>& aaf, std::string_view dat_name);

/// The Datensatz gleichen Namens test of the AAF box before a save,
/// ported from CASE 168 of aaf_box. The exact surname and given name win,
/// else his test runs, both names inside the head of the #A93 line. Like
/// his loop the last hit counts.
///
/// @param aaf the records of the AAF file
/// @param r   the record about to be saved
/// @return the index of the record of the same name, nothing when none
[[nodiscard]] std::optional<std::size_t> aaf_same_name(const std::vector<AafRecord>& aaf, const AafRecord& r);

/// Whether DATEIEN VERKETTEN carries a record into the chain file. His
/// a200dat took a record with day and month, the trim pass at VERKETTEN
/// BEENDEN dropped every record without place coordinates.
///
/// @param r the record
/// @return true when the record joins the chain
[[nodiscard]] bool chain_keeps(const AafRecord& r);

/// Rebuilds the DAT twin from its AAF pilot, ported from aaf_horcom2.
///
/// @param dat the DAT file, replaced in one step
/// @param aaf the records of the AAF file
/// @return true on success
bool write_dat_from_aaf(const std::filesystem::path& dat, const std::vector<AafRecord>& aaf);

}  // namespace horcom
