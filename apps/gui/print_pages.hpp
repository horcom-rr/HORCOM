// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <QRectF>
#include <QString>
#include <QStringList>
#include <string>

class QPainter;
class QPrinter;
class QWidget;

// The printer side of the original, the setup screen before every print,
// the page geometry his StretchBlt and VIEWPORT calls used, the HARDCOPY
// box and the text list pages. Shared by the main window and the list
// dialogs.
namespace horcom {

struct DisplayList;

/// his muuu& numbers of the menu entries, the HARDCOPY gate of
/// start_hardc and the F9 capture read them
namespace menu_item {
inline constexpr int kPlanetCoordinates = 37;
inline constexpr int kExtraCoordinates = 38;
inline constexpr int kStatistics = 40;
inline constexpr int kDegreeList = 42;
inline constexpr int kFixedStars = 43;
inline constexpr int kArabicParts = 44;
inline constexpr int kIngresses = 45;
inline constexpr int kChartGraphic = 53;
inline constexpr int kAspektarium = 54;
inline constexpr int kMidpointGraphic = 55;
inline constexpr int kMulti = 56;
inline constexpr int kComposite = 57;
inline constexpr int kCombin = 58;
inline constexpr int kDoubleWheel = 59;
inline constexpr int kReturns = 64;
inline constexpr int kDayChart = 65;
inline constexpr int kRhythm = 67;
inline constexpr int kSecondary = 69;
inline constexpr int kArcDirection = 70;
inline constexpr int kPrimary = 71;
inline constexpr int kSymbolicEquatorial = 72;
inline constexpr int kSymbolicEcliptic = 73;
inline constexpr int kTransits = 74;
inline constexpr int kMundane = 75;
inline constexpr int kHouseTable = 80;
inline constexpr int kTimeWander = 83;
inline constexpr int kClock = 84;
inline constexpr int kRiseSet = 89;
inline constexpr int kEclipses = 90;
inline constexpr int kPlaceWander = 97;
}  // namespace menu_item

/// the divisors of the printable width and height that gave his page
/// margins, and the factor of the full page HARDCOPY
namespace page_margin {
inline constexpr double kLeft = 20.0;       ///< gdxp& / 20 of the HARDCOPY and most graphics
inline constexpr double kLeftChart = 15.0;  ///< gdxp& / 15 of the a11 chart graphic
inline constexpr double kTop = 90.0;        ///< gdyp& / 90
inline constexpr double kFull = 90.0;       ///< both margins of the full page HARDCOPY
inline constexpr double kDoubleTop = 28.0;  ///< gdyp& / 28 above each picture of the double print
inline constexpr double kLinearTop = 20.0;  ///< gdyp& / 20 of the linear graphic on DIN A4
inline constexpr double kFullFactor = 1.4;  ///< the full page HARDCOPY and the A4 linear graphic
}  // namespace page_margin

/// The pages druck_einr_anz tells the operator to set up, each with its
/// paper and orientation.
enum class PrintPage {
  kHardcopy,  ///< the HARDCOPY of an output, A5 portrait or A4 landscape by the format
  kGraphicA5, ///< DRUCKER-GRAPHIK DIN A5, and the lists printed as A5 pages
  kGraphicA4, ///< DRUCKER-GRAPHIK DIN A4, portrait
  kLinearA4,  ///< the LINEAR-GRAPHIK on DIN A4, landscape
  kDouble,    ///< the F9 DOPPEL-AUSDRUCK, two pictures on DIN A4 portrait
  kList,      ///< the text lists of datei_pr and lese_text_pr
};

/// Whether start_hardc offers the HARDCOPY after an output of this menu
/// entry. The converters, the defaults, the explanations and the tools
/// of DIVERSES get none unless a composite, combin or double chart is up.
///
/// @param item      his muuu& number of the output
/// @param pair_view a composite, combin or double chart is on screen
/// @return true when the box may come
[[nodiscard]] bool hardcopy_item(int item, bool pair_view);

/// Prepares the printer the way druck_einr_anz did, DRUCKER BEREIT ?,
/// his setup box with the paper and the orientation, then the system
/// print dialog.
///
/// @param parent    the window the boxes belong to
/// @param printer   the printer, its orientation follows the page
/// @param page      the kind of page
/// @param half_page his halbs& = 1, the HARDCOPY on half a page
/// @return true when the operator started the print
[[nodiscard]] bool prepare_printer(QWidget* parent, QPrinter& printer, PrintPage page, bool half_page);

/// dr_fehl, the printer gave up or the operator cancelled.
///
/// @param parent the window the box belongs to
void printer_failed(QWidget* parent);

/// The rectangle his StretchBlt and VIEWPORT calls mapped the 640 by
/// 459 screen into, 0.8 points per screen unit.
///
/// @param printer  the prepared printer
/// @param left_div the printable width divided by it gives the left margin
/// @param top_div  the printable height divided by it gives the top margin
/// @param factor   1 for his half pages, 1.4 for the full page
/// @return the rectangle in device pixels
[[nodiscard]] QRectF robert_page_rect(const QPrinter& printer, double left_div, double top_div, double factor);

/// The rectangle of the a11 DRUCKER-GRAPHIK DIN A4, his 640 by 980 page
/// at 0.75 points per unit from a fourteenth of the width and an
/// eightieth of the height.
///
/// @param printer the prepared printer
/// @return the rectangle in device pixels
[[nodiscard]] QRectF robert_a4_rect(const QPrinter& printer);

/// Marks a window as an output of the menu entry, the HARDCOPY and the
/// F9 capture of the output windows read it.
///
/// @param output the output window
/// @param item   his muuu& number of the entry
void mark_output(QWidget* output, int item);

/// The menu entry an output window was marked with.
///
/// @param w the window
/// @return his muuu& number, zero when the window is no output
[[nodiscard]] int output_item(const QWidget* w);

/// Paints a page into the rectangle of robert_page_rect with the moment
/// of printing behind the credit.
///
/// @param p      the painter on the printer
/// @param page   the page, stamped on a copy
/// @param target the rectangle of robert_page_rect
void paint_robert_page(QPainter& p, const DisplayList& page, const QRectF& target);

/// Paints the picture of an output window like hardcopy stretched his
/// window bitmap, a sheet canvas as its display list and every other
/// output as its grabbed picture with the credit under it.
///
/// @param p      the painter on the printer
/// @param output the output window
/// @param target the rectangle of robert_page_rect
void paint_output(QPainter& p, QWidget* output, const QRectF& target);

/// The display list of the sheet canvas an output window carries.
///
/// @param output the output window
/// @return the page, empty when the output holds no canvas
[[nodiscard]] DisplayList output_page(QWidget* output);

/// Prints rows of text the way datei_pr and lese_text_pr did, 53 rows
/// a page in a bold fixed pitch font a sixtieth of the page high, the
/// page number centred above them.
///
/// @param printer the prepared printer
/// @param rows    the rows in their order
/// @return false when the printer refused a page
[[nodiscard]] bool print_text_rows(QPrinter& printer, const QStringList& rows);

/// The character pitch of the text lists, his ninetieth of the width
/// or narrower until the longest row fits between the margins.
///
/// @param page_width the printable width
/// @param longest    the characters of the longest row
/// @return the width of one character
[[nodiscard]] double list_pitch(double page_width, qsizetype longest);

/// Breaks a text into the rows of his text files, each paragraph
/// wrapped at the word before the column limit.
///
/// @param text    the text, paragraphs separated by line breaks
/// @param columns the widest row
/// @return the rows in their order
[[nodiscard]] QStringList wrap_text_rows(const QString& text, int columns);

/// the width of his KOMMEN7P text files, the rows lese_text_pr printed
inline constexpr int kTextColumns = 80;

/// The moment of printing, his datumakt$ and tim$.
///
/// @return DD.MM.YYYY HH:MM
[[nodiscard]] std::string print_stamp();

/// Sends every print into a PDF file instead of the system dialog, the
/// hook of the tests and the capture rig. An empty path restores the
/// dialog.
///
/// @param path the PDF file the next prints write
void set_print_file(const QString& path);

/// The HARDCOPY box of start_hardc, JA and NEIN with NEIN focused at
/// the lower right of the output. ESC, PgUp and PgDn answer NEIN, F1
/// opens his ERLÄUTERUNG 2 through the horcomHelp property.
///
/// @param output the output window the box sits on
/// @return true for JA
[[nodiscard]] bool ask_hardcopy(QWidget* output);

}  // namespace horcom
