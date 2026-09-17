// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "horcom/time/calendar.hpp"

// The chart collections of the original, SPEZIAL *.DAT. Fixed 128 byte
// ASCII records with no header and no index, right aligned numbers and
// left aligned text, the clock always Universal Time. Robert Rettig
// documented the truncation himself,
//RR HORCOM speichert z.B. die Zeit immer nur als GMT = UT , Bemerkungen
//RR werden nach 51 Zeichen abgeschnitten.Das stammt noch aus der Zeit
//RR als Speicher-Platz knapp war.Heute kann man dies vergessen.
namespace horcom {

/// the record size of the original FIELD layout
inline constexpr std::size_t kChartRecordBytes = 128;

/// One stored chart, text fields already UTF-8.
struct ChartRecord {
  int day = 0;
  int month = 0;
  int year = 0;        // astronomical count, negative means BC
  double hour = 0.0;   // UT
  double minute = 0.0; // carries seconds as a fraction
  double lon = 0.0;    // decimal degrees, east positive
  double lat = 0.0;    // decimal degrees, north positive
  std::string name;    // 25 bytes in the file
  std::string place;   // 20 bytes
  std::string remark;  // 51 bytes, may start with the calendar flag

  /// @return the calendar the remark flag requests, the original jul$
  [[nodiscard]] Calendar calendar() const;

  /// @return day, month, year, hour and minute as a CalendarDate
  [[nodiscard]] CalendarDate date() const { return {day, month, year, hour, minute}; }
};

/// Decodes one 128 byte record.
///
/// @param bytes exactly kChartRecordBytes raw bytes
/// @return the record with trimmed, UTF-8 text
[[nodiscard]] ChartRecord decode_chart_record(std::string_view bytes);

/// Encodes a record into the original byte layout.
///
/// Numbers are right aligned like RSET STR$, text is left aligned and
/// space padded like LSET, overlong text is truncated exactly like the
/// original fields did.
///
/// @param r the record
/// @return kChartRecordBytes bytes
[[nodiscard]] std::string encode_chart_record(const ChartRecord& r);

/// Reads a whole chart file.
///
/// @param path a SPEZIAL style .DAT file
/// @return all records, or std::nullopt when the file cannot be read or
///         its size is not a multiple of the record size
[[nodiscard]] std::optional<std::vector<ChartRecord>> read_chart_file(const std::filesystem::path& path);

/// Writes a whole chart file in the original format.
///
/// @param path    destination
/// @param records the records
/// @return true on success
bool write_chart_file(const std::filesystem::path& path, const std::vector<ChartRecord>& records);

/// The Datei TRIMMEN pass, ported from a2f_tr_dat.
///
/// Records with day zero and records whose longitude and latitude are
/// both zero fall out, every text field comes back trimmed. The
/// original replaced the file only when the pass removed something,
/// callers keep that rule.
///
/// @param records edited in place
void trim_records(std::vector<ChartRecord>& records);

/// The Datensätze LÖSCHEN pass, ported from a2f_tr_dat in delete mode.
///
/// The doomed records fall out and the trim drop rules run alongside
/// exactly like the original single pass did.
///
/// @param records edited in place
/// @param doomed  zero based indices into records before the pass
void delete_records(std::vector<ChartRecord>& records, const std::vector<std::size_t>& doomed);

/// The Datei MINIMIEREN pass, records sharing name and birth clock
/// collapse onto their first copy.
///
/// @param records edited in place, order kept
void minimize_records(std::vector<ChartRecord>& records);

/// Removes every record wearing the given name, the overwrite half of
/// AKTUELLEN Datensatz EINTRAGEN, ported from a22ueberschrb.
///
/// The original compared the names verbatim after its list pass had
/// already uppercased them all, the port compares case blind for the
/// same effect.
///
/// @param records edited in place
/// @param name    the name to overwrite, surrounding blanks ignored
/// @return how many records fell out
std::size_t remove_records_by_name(std::vector<ChartRecord>& records, const std::string& name);

}  // namespace horcom
