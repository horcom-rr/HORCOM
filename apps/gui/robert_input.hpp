// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QString>
#include <QStringList>
#include <array>
#include <optional>
#include <utility>
#include <vector>

#include "horcom/time/calendar.hpp"

class QLineEdit;
class QValidator;
class QWidget;

// The small value dialogs of the original, one value per box with his
// captions, shared by every flow that asks for a degree or a clock.
namespace horcom {

/// One of his short right aligned edit fields, sized to its characters.
///
/// @param parent    the owner
/// @param chars     the field length, his eingl|
/// @param validator the input rule, owned by the caller's widget, may be
///                  empty
/// @return the field
[[nodiscard]] QLineEdit* small_box(QWidget* parent, int chars, QValidator* validator = nullptr);

/// A short field for a whole number from zero up, preset with a text.
///
/// @param parent    the owner
/// @param text      the preset text
/// @param chars     the field length
/// @param max_value the largest accepted number, zero accepts any text
/// @return the field
[[nodiscard]] QLineEdit* number_box(QWidget* parent, const QString& text, int chars, int max_value);

/// Reads a hemisphere letter field like his UPPER$(et$(3)) = "W" tests.
///
/// @param text     the field text
/// @param first    the letter of the positive side, E or N
/// @param second   the letter of the negative side, W or S
/// @param fallback the letter kept when the field holds neither
/// @return first, second or fallback
[[nodiscard]] char hemisphere_letter(const QString& text, char first, char second, char fallback);

/// The before Christ flag of his date masks, zuo took V or a minus.
///
/// @param text the flag field
/// @return true for V or -, upper or lower case
[[nodiscard]] bool before_christ_flag(const QString& text);

/// @param sign 0 Aries to 11 Pisces
/// @return the full sign name of his zod_zeich_alph list, translated
[[nodiscard]] QString sign_name(int sign);

/// Asks for a sign from his zod_zeich_alph list the way ze_pl_wa does,
/// a double click picks.
///
/// @param parent the owner window
/// @return 0 Aries to 11 Pisces, nothing after ESC
[[nodiscard]] std::optional<int> pick_sign(QWidget* parent);

/// Asks for an ecliptic position the way input_grmise_zod does, first
/// the sign from his list, then degree, minute and second inside it or
/// one decimal degree for the whole circle.
///
/// @param parent the owner window
/// @param title  the dialog title, his ti$
/// @return the longitude in radians, nothing after ESC
[[nodiscard]] std::optional<double> ask_zodiac_position(QWidget* parent, const QString& title);

/// Asks for a clock time the way zeiteing does, hours, minutes and
/// seconds in three boxes.
///
/// @param parent the owner window
/// @param title  the dialog title
/// @return the time in decimal hours, nothing after ESC
[[nodiscard]] std::optional<double> ask_clock(QWidget* parent, const QString& title);

/// Asks for a date the way a37dat does, TT MM JJJJ with his V box for
/// years before Christ, optionally a clock row under it.
///
/// @param parent     the owner window
/// @param title      the dialog title, his tida$
/// @param start      the date the fields open with, astronomical year
/// @param time_label the caption of the clock row, empty leaves it out
/// @param notes      centred lines above the fields, the texts his screen
///                   showed around the box
/// @return the date with the clock in hour and minute, nothing after ESC
[[nodiscard]] std::optional<CalendarDate> ask_date(QWidget* parent, const QString& title, const CalendarDate& start,
                                                   const QString& time_label = QString(),
                                                   const QStringList& notes = {});

/// Asks for one number the way his inputbox$ does, a prompt line over a
/// single edit field.
///
/// @param parent  the owner window
/// @param title   the dialog title
/// @param prompt  the line above the field
/// @param minimum smallest accepted value
/// @param maximum largest accepted value
/// @param start   the value the field opens with, NaN leaves it empty
/// @param decimals digits after the point, zero for whole numbers
/// @param notes   centred lines under the field, the texts his screen
///                showed around the box
/// @return the value, nothing after ESC
[[nodiscard]] std::optional<double> ask_number(QWidget* parent, const QString& title, const QString& prompt,
                                               double minimum, double maximum, double start, int decimals, const QStringList& notes = {});

/// Asks for a text the way his inputbox$ does under STRING-Eingabe !,
/// a prompt line over one field.
///
/// @param parent  the owner window
/// @param title   the dialog title, his eas$
/// @param prompt  the line above the field
/// @param start   the text the field opens with
/// @param max_len the longest text, his LEFT$ cut, zero for no limit
/// @return the text, nothing after ESC
[[nodiscard]] std::optional<QString> ask_text(QWidget* parent, const QString& title, const QString& prompt,
                                              const QString& start = {}, int max_len = 0);

/// Asks for a set of objects the way ausw_obj_m does under OBJEKTE
/// AUSWÄHLEN !, one check box per object, an empty choice asks again.
///
/// @param parent  the owner window
/// @param objects the body slots on offer, top to bottom
/// @param names   their captions, his ltz$ rows
/// @return the chosen slots, nothing after ESC
[[nodiscard]] std::optional<std::vector<int>> ask_objects(QWidget* parent, const std::vector<int>& objects,
                                                          const QStringList& names);

/// Asks for one object the way ausw_pl_hs does under EIN OBJEKT
/// AUSWÄHLEN !, one row per slot, the rows of objects that do not apply
/// read as his dashes and cannot be chosen.
///
/// @param parent the owner window
/// @param rows   the slot of each row with its caption, slot -1 for a
///               dash row
/// @param notes  lines beside the list, the texts his screen showed
///               left of the box
/// @param title  the box title, EIN OBJEKT AUSWÄHLEN ! when empty
/// @return the chosen slot, nothing after ESC
[[nodiscard]] std::optional<int> ask_object(QWidget* parent, const std::vector<std::pair<int, QString>>& rows,
                                            const QStringList& notes = {}, const QString& title = {});

/// Asks for one whole number the way numw1 does in its ZIFFERN-EINGABE
/// box, one button per value in a row, the first as the default.
///
/// @param parent the owner window
/// @param prompt the line above the buttons
/// @param from   the smallest value
/// @param to     the largest value
/// @return the value, nothing after ESC
[[nodiscard]] std::optional<int> ask_digit(QWidget* parent, const QString& prompt, int from, int to);

/// Asks for a geographic position the way labr2 does, E or W and N or S
/// with degrees, minutes and seconds.
///
/// @param parent the owner window
/// @param notes  centred lines above the fields
/// @return east longitude and north latitude in degrees, nothing after ESC
[[nodiscard]] std::optional<std::pair<double, double>> ask_geo_coordinates(QWidget* parent,
                                                                          const QStringList& notes = {});

/// Asks for ecliptic or equatorial coordinates the way
/// input_grmise_ekl_aeq does, degree minute second or decimal.
///
/// @param parent     the owner window
/// @param equatorial true asks right ascension and declination in degrees
/// @param title      the dialog title
/// @return longitude or right ascension and latitude or declination in
///         degrees, nothing after ESC
[[nodiscard]] std::optional<std::pair<double, double>> ask_sky_coordinates(QWidget* parent, bool equatorial,
                                                                          const QString& title);

/// The ENTER of rechne1, decimal to degree minute second or back.
///
/// @param fields the four fields, decimal, degree, minute, second
/// @return the fields after the conversion
[[nodiscard]] std::array<QString, 4> angle_enter(const std::array<QString, 4>& fields);

/// Runs the WINKEL(ZEIT)-UMRECHNUNG box of rechne1 until QUIT.
///
/// @param parent the owner window
void angle_converter(QWidget* parent);

}  // namespace horcom
