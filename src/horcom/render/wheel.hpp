// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "horcom/chart/aspects.hpp"
#include "horcom/chart/chart.hpp"
#include "horcom/chart/histogram.hpp"

// The chart wheel as a backend neutral display list. The geometry is the
// original's, a virtual canvas of 640 by 480 with the wheel centre at
// 430, 224 and the scale km 0.95, the ring radii 90, 128, 152, 165 and
// 182, the polar mapping w = nb(lambda + pi - fza) with the ascendant on
// the left, and his glyph de clumping from plentz. Only the flood fill of
// the sign band becomes explicit annular sectors, a change the analysis
// already demanded for any modern renderer.
namespace horcom {

/// The virtual canvas and wheel geometry of the original, shared with the
/// tests and every backend.
inline constexpr double kCanvasWidth = 640.0;
/// the screen sheet is a little narrower, the wheel centred fills it
inline constexpr double kScreenSheetWidth = 560.0;
inline constexpr double kCanvasHeight = 480.0;
//RR Horoskop-Mitte
inline constexpr double kWheelCenterX = 430.0;
inline constexpr double kWheelCenterY = 224.0;
inline constexpr double kWheelScale = 0.95;  // the original km
inline constexpr double kGlyphRingRadius = 128.0;

/// Colours as packed 0xRRGGBB like the original RGB() calls.
using Rgb = unsigned;

/// the paper of the classic sheet, the cutouts under the glyphs and the
/// canvas of the wheel wear it, a rewrite addition shared by the screen,
/// the SVG export and the painter so all three match
inline constexpr Rgb kPaperColor = 0xFCFAF4;

/// his RGB(0,0,0) ink and RGB(255,0,0) marking red of the sheets
inline constexpr Rgb kInkColor = 0x000000;
inline constexpr Rgb kMarkRed = 0xFF0000;

/// the size of a planet or sign sprite on the sheets, his 11 pixel cells
inline constexpr double kSpriteSize = 11.0;

/// the side a sprite covers against its glyph size, his 16 pixel bitmap
/// cell around the drawing
inline constexpr double kSpriteBox = 1.1;

/// the credit line drad2 stamped on every output, the rewrite names the
/// original author first and drops the historical 7P like the logo
inline constexpr std::string_view kCreditLine = "HORCOM \xC2\xB7 Robert Rettig \xC2\xB7 \xC2\xA9 Dominik Schwimmbeck";

/// One drawing primitive on the virtual 640 by 480 canvas.
struct Primitive {
  /// kRect fills an axis aligned box, x1 y1 the centre, r1 and r2 the
  /// half extents, the white sprite ground of his SRCCOPY stamps
  enum class Kind { kCircle, kLine, kSector, kGlyph, kText, kDot, kRect };
  enum class Style { kSolid, kDashed, kDotted, kDashDot };
  /// what the item anchors to, the wheel itself, the corner notes of
  /// the screen sheet, or the credit line
  enum class Anchor { kSheet, kCorner, kCredit };
  Kind kind = Kind::kLine;
  double x1 = 0.0;   // centre for circles, sectors, glyphs and dots
  double y1 = 0.0;
  double x2 = 0.0;   // line end
  double y2 = 0.0;
  double r1 = 0.0;   // radius, inner radius for sectors
  double r2 = 0.0;   // outer radius for sectors
  double a1 = 0.0;   // sector start angle on the canvas, radians
  double a2 = 0.0;   // sector end angle
  double size = 0.0; // text height for glyphs and text
  Rgb color = 0x000000;
  Rgb fill = 0xFFFFFF;
  Style style = Style::kSolid;
  double width = 1.0;
  /// text grows rightward from x1 instead of centring on it, the text
  /// column of the original screens
  bool align_left = false;
  /// text ends at x1, the flush right blocks of the sheet corners
  bool align_right = false;
  Anchor anchor = Anchor::kSheet;
  std::string text;  // glyph character or label
  /// the text reads upward from x1 y1 like his escapement 900 font of
  /// texts, the vertical boxes of the statistics list
  bool vertical = false;
  /// the room of one character like his FONT WIDTH, a wider face narrows
  /// to it and a narrower one keeps its own advance, zero keeps the
  /// natural width of the face
  double pitch = 0.0;
};

/// The advance of the fixed pitch sheet face against its height, Courier
/// New and its metric twin Liberation Mono.
inline constexpr double kMonoAdvance = 0.6;

/// The advance one character of a text steps, his FONT WIDTH as the
/// ceiling over the natural advance of the sheet face.
///
/// @param p a text primitive
/// @return the advance in canvas units
[[nodiscard]] inline double text_advance(const Primitive& p) {
  const double natural = kMonoAdvance * p.size;
  return p.pitch > 0.0 ? std::min(p.pitch, natural) : natural;
}

/// The character width his textg gave a text height on the screen, the
/// FONT WIDTH of text, textc and their kin.
///
/// @param size the text height in screen units
/// @return the width of one character, zero for the heights his table
///         left to the printer
[[nodiscard]] double font_pitch(double size);

/// The half side of the dark square under a sprite as a share of the
/// sprite size. His SRCINVERT stamping inverted the whole bitmap cell,
/// so the square is the sprite box and his drawing reaches within one
/// cell pixel of its edge.
inline constexpr double kInvertPatchShare = kSpriteBox / 2.0;

/// The dark square of an inverted sprite, his putbm SRCINVERT.
///
/// @param x    the centre of the sprite
/// @param y    the centre of the sprite
/// @param size the sprite size
/// @return the filled box
[[nodiscard]] inline Primitive inverted_patch(double x, double y, double size) {
  Primitive patch;
  patch.kind = Primitive::Kind::kRect;
  patch.x1 = x;
  patch.y1 = y;
  patch.r1 = size * kInvertPatchShare;
  patch.r2 = patch.r1;
  patch.fill = kInkColor;
  return patch;
}

/// The ink of an inverted sprite on its dark patch.
inline constexpr Rgb kInvertedInk = 0xFFFFFF;

/// One row of the ASPEKT-LINIEN screen of avh, the chord an aspect
/// family draws with.
struct ChordLine {
  bool on = false;
  Rgb color = 0x000000;
  Primitive::Style style = Primitive::Style::kSolid;
};

/// the rows 0 to 19 of his aspli tables, rows 2 to 19 carry a family
inline constexpr int kChordRows = 20;

/// The chord rows indexed like aspli, row 2 the opposition up to row 19
/// the quincunx.
using ChordTable = std::array<ChordLine, kChordRows>;

/// HORCOM's own chord of a row, the colour and DEFLINE style aspz1 hands
/// aspz1_2 when no own settings are active.
///
/// @param row the aspli row, 2 to 19
/// @return the switched on standard chord, off for rows outside 2 to 19
[[nodiscard]] ChordLine standard_chord(int row);

/// The pen of a DEFLINE style number, 0 solid, 1 dash, 2 dot, 3 dash dot,
/// the Windows pen styles his aspst values name.
///
/// @param aspst the stored style
/// @return the matching primitive style, solid for anything unknown
[[nodiscard]] Primitive::Style chord_line_style(int aspst);

/// The aspli row a hit draws on, the SWITCH of aspz1. The first and last
/// multiple of a divisor take the base row, the compound multiples the
/// rows 13 to 19. The Rhythmenlehre leaves the eighth and twelfth
/// divisors out.
///
/// @param n      the divisor of the hit
/// @param m      its multiple
/// @param rhythm the ryt mode of the Münchner Rhythmenlehre
/// @return the row, 0 when aspz1 draws no chord
[[nodiscard]] int chord_row(int n, int m, bool rhythm);

/// The bar colours of asphist per divisor 1 to 16, index 0 unused.
///
/// @param own the chords of the EIGENE EINSTELLUNGEN, nullptr for his
///            standard colours. An own row without colour turns grey
///            like asphi1
/// @return the colours, the divisors above twelve white
[[nodiscard]] std::array<Rgb, 17> aspect_hist_colors(const ChordTable* own);

/// How the sign band and the histogram columns are filled, the farbs!,
/// farbp! and weiss! switches of avh.
enum class RingFill {
  kShaded,  ///< SCHRAFFIERT, the hatch of deffi as it rendered on screen
  kSolid,   ///< FARBIG PUR, a solid brush in the colour
  kWhite    ///< WEIß or the NUR SYMBOLE modes, no fill at all
};

/// The colour a sign band sector shows, the hatch of deffi rendered as
/// the shade his screen showed, shared by the wheel and the colour rows
/// of hor_farb.
///
/// @param element 1 fire, 2 earth, 3 air, 4 water, it picks the hatch
///                pattern 5, 1, 7 or 3
/// @param color   the colour of the brush
/// @param fill    how the band is filled
/// @return the colour on the paper
[[nodiscard]] Rgb ring_fill_color(int element, Rgb color, RingFill fill);

/// His RGB() value, the Windows COLORREF with red in the low byte.
///
/// @param v the stored value
/// @return the same colour packed as 0xRRGGBB
[[nodiscard]] constexpr Rgb rgb_of_colorref(int v) {
  const auto u = static_cast<Rgb>(v);
  return ((u & 0xFFu) << 16) | (u & 0xFF00u) | ((u >> 16) & 0xFFu);
}

struct Konsta;

/// The dress a settings profile gives every wheel, the ring and
/// histogram modes of avh9, the lines of the ASPEKT-LINIEN screen and the
/// outer colour of hard&.
struct WheelDress {
  /// the chords of aspz1_2, the own rows under selbst_cl_st! and the
  /// switched standard rows otherwise
  ChordTable chords{};
  /// the asphist bars, his own colours under selbst_cl_st!
  std::array<Rgb, 17> hist_colors{};
  /// the element colours of fill_color, index 1 fire to 4 water
  std::array<Rgb, 5> ring_colors{};
  /// eigfarb!, the own cols% dress the band
  bool own_colors = false;
  /// zein fills the band only without weiss! and nursymb&
  RingFill ring_fill = RingFill::kShaded;
  /// pboxn fills the histogram columns unless weiss!
  RingFill hist_fill = RingFill::kShaded;
  /// nursymb& = 1, the sign glyphs wear their element colour
  bool colored_signs = false;
  /// hard&, 1 red, 2 black, 3 blue
  int outer_color = 2;
};

/// The dress of a settings profile.
///
/// @param k the settings, his KONSTA
/// @return the colours, lines and fills every wheel of that profile wears
[[nodiscard]] WheelDress konsta_dress(const Konsta& k);

/// The dress of Robert Rettig's own profile, the defaults of every
/// WheelOptions so the command line, the SVG export and the program draw
/// the same wheel.
///
/// @return the dress of robert_profile, computed once
[[nodiscard]] const WheelDress& profile_dress();

/// A ready to draw wheel.
struct DisplayList {
  double width = 640.0;
  double height = 480.0;
  std::vector<Primitive> items;
};

/// the a20 transit screen shrinks the wheel to make room for the outer
/// ring, the original km there
inline constexpr double kTransitWheelScale = 0.85;

/// primhorg draws the directed axes wheel slightly smaller
inline constexpr double kDirectedWheelScale = 0.89;

/// the a12 comparison screen shrinks the wheel like the transit screen,
/// so the second ring and its house band stay inside the sheet, the
/// original km there
inline constexpr double kDoubleWheelScale = 0.8;

/// multi1 and the harmonics draw their double wheel a shade wider, the
/// original km of both entries
inline constexpr double kMultiWheelScale = 0.82;

/// Options of the wheel builder.
struct WheelOptions {
  /// draw the aspect chords of a scan result
  bool aspect_lines = true;
  /// the centre label, TRANSIT with the moment on the transit wheel or
  /// the running UHR of the clock chart
  std::string center_label;
  /// his sol$ of bes2, RADIX, n.SOLAR, COMPOSIT and the like, centred on
  /// the wheel, below the sun mark in the hrg mode
  std::string chart_label;
  /// the line under it, the house method of bes2_comp or Mundan
  std::string chart_sub_label;
  /// the chart data block of bes11, written down the left margin the
  /// original kept free of the wheel
  std::vector<std::string> info_lines;
  /// overrides the wheel scale when positive, primhorg draws the
  /// directed axes at 0.89
  double scale = 0.0;
  /// the hrg mode, the moon slot carries the earth and wears its glyph
  bool heliocentric = false;
  /// the 90 degree circle of a12, three sign sectors, AC and MC as the
  /// only axes since DC and IC land on top of them
  bool dial = false;
  /// a composite with real houses, MITTLERE STZ or ROBERT HAND, horg11
  /// names cusp one H1 and gives the AC midpoint a thick axis of its own
  bool composite_axes = false;
  /// the aspect chords per aspli row, colour, style and whether the row
  /// draws at all
  ChordTable chords = profile_dress().chords;
  /// the ryt mode of the Rhythmenlehre, aspz1 skips the eighth and
  /// twelfth divisors
  bool rhythm = false;
  /// draw the blue dashed node axis like the original Mondknotenlinie,
  /// asp1 draws it with the chords only
  bool node_axis = true;
  /// the apog! line of asp1, a black dashed diameter through the Black
  /// Moon on the aspect circle
  bool apogee_axis = false;
  /// the right mouse selection of the original chart screen, per slot,
  /// 0 draws normally, 1 marks the glyph red, -1 hides the body
  std::array<int, body::kSlotCount> emphasis{};
  /// the birth ruler slot, drawn inverted on a dark patch like the
  /// putbm SRCINVERT stamping of rulers and nodes, negative for none
  int ruler_slot = -1;
  /// the second inverted ruler of geb_herr, the ruler of a whole sign
  /// intercepted in the first house, negative for none
  int ruler_slot2 = -1;
  /// his moknw flag, only the true node draws inverted
  bool invert_nodes = true;
  /// his apogw flag, the true apogee draws Lilith inverted
  bool invert_apogee = false;
  /// print the degree within sign under each glyph, pziff 1 and 2
  bool degree_numbers = true;
  /// the R of a retrograde body beside its glyph, pziff 1 only
  bool retro_marks = true;
  /// the KLEIN-SYMBOLE of klsy, the body glyphs shrink to plsyver 5
  /// instead of 7 and plentz11 staggers them closer
  bool small_symbols = false;
  /// BEGINN HOROSKOP, begz& of horbeg, 1 the AC on the left, 2 the MC,
  /// 3 zero Aries, 4 zero Libra, 5 the longitude in begin_lon
  int begin = 1;
  /// the BELIEBIGER GRAD of begz 5, radians
  double begin_lon = 0.0;
  /// the element colours of the sign band and the histograms, index 1
  /// fire to 4 water, index 0 unused. The default is the cols% set of
  /// his own KONSTA, red, green, cyan and blue
  std::array<Rgb, 5> ring_colors = profile_dress().ring_colors;
  /// eigfarb!, the own colours rule the band. The 90 degree circle then
  /// paints its third sector in the water colour like fill_color under
  /// dop 4, the standard colours keep air there
  bool own_colors = profile_dress().own_colors;
  /// how the sign band is filled
  RingFill ring_fill = profile_dress().ring_fill;
  /// how the histogram columns of the A4 sheet are filled
  RingFill hist_fill = profile_dress().hist_fill;
  /// the ZEICHEN-SYMBOLE FARBIG modes of nursymb 1, the sign glyphs
  /// wear the colour of their element
  bool colored_signs = profile_dress().colored_signs;
  /// the counted divisors of asp1, nasp&, the rows of the histogram
  int divisors = 12;
  /// the bars of the aspect histogram asphist
  std::array<Rgb, 17> hist_colors = profile_dress().hist_colors;
  /// the colour of the outer symbols on double and transit wheels, the
  /// original hard&, 1 red, 2 black, 3 blue, his own profile holds red
  int outer_color = profile_dress().outer_color;
  /// whether the outer ring wears outer_color, his transit, ZEIT-WANDERN
  /// and DOPPELKREIS rings do, MULTI and HARMONICS stay black
  bool outer_tint = true;
  /// the running factors of EINZELNE LAUFENDE PLANETEN ROT MARKIEREN, his
  /// plw& under plan_col!, their outer glyphs turn red before outer_color
  /// is asked
  std::vector<int> outer_marked;
  /// drgrph!, the running AC and MC of zeitwi ride on the outer ring as
  /// his AC and MC sprites with their degree like the planets of
  /// plein1(212), the comparison ring of a12 keeps them away
  bool outer_axes = false;
  /// the rulers of a Rhythmenlehre phase, plinv stamps them once more
  /// with SRCINVERT so each flips its inversion
  std::vector<int> flip_inverted;
  /// the house of the running Rhythmenlehre phase, a1795 draws a red
  /// arc of width two over it at radius 154, zero for none
  int phase_house = 0;
};

/// Builds the display list of one chart wheel.
///
/// @param chart   the computed chart
/// @param s       chart settings, decides which slots appear
/// @param aspects the aspect scan whose hits become the chords, may be a
///                default constructed result when aspect_lines is off
/// @param opt     drawing options
/// @return primitives on the virtual canvas, in paint order
[[nodiscard]] DisplayList build_wheel(const Chart& chart, const ChartSettings& s, const AspectResult& aspects, const WheelOptions& opt = {});

/// Builds the transit double wheel of the original a20 screen. The radix
/// sits inside with its glyphs, houses and aspect chords, the running sky
/// rides outside with glyphs at 212 and tick markers on the sign ring,
/// everything at the smaller a20 scale.
///
/// @param radix   the birth chart, rules the houses and the rotation
/// @param transit the chart of the transit moment, same settings
/// @param s       chart settings, decides which slots appear in both rings
/// @param radix_aspects the radix scan whose hits draw as chords
/// @param opt     drawing options, transit_label prints in the centre
/// @return primitives on the virtual canvas, in paint order
[[nodiscard]] DisplayList build_transit_wheel(const Chart& radix, const Chart& transit, const ChartSettings& s, const AspectResult& radix_aspects, const WheelOptions& opt = {});

/// Builds the double wheel of the original a12 comparison screen. The
/// first chart sits inside at full scale with its houses and chords,
/// the second rides outside with glyphs at 204 and markers at 180, and
/// its house lines draw over the shared ring like the original's second
/// horg11 pass.
///
/// @param inner the first chart, rules the rotation
/// @param outer the compared chart
/// @param s     chart settings, decides which slots appear in both rings
/// @param inner_aspects the first chart's scan, drawn as chords
/// @param opt   drawing options
/// @return primitives on the virtual canvas, in paint order
[[nodiscard]] DisplayList build_double_wheel(const Chart& inner, const Chart& outer, const ChartSettings& s, const AspectResult& inner_aspects, const WheelOptions& opt = {});

/// Moves the wheel into the middle of a square sheet for the screen,
/// the corner notes and the credit line stay on the margins. The
/// classic 640 by 480 sheet with the wheel on the right remains the
/// export shape.
///
/// @param dl a display list in classic sheet coordinates
/// @param width the sheet width, the view passes its own aspect so no
/// dark gap stays between the panels and the paper
/// @return the same drawing on the wheel centred sheet
[[nodiscard]] DisplayList centered_sheet(const DisplayList& dl, double width = kScreenSheetWidth);

/// The corner texts of the classic sheet, assembled by the caller so
/// the drawing layer stays free of locale and record handling.
struct ClassicSheetText {
  std::string name;     ///< the record name
  std::string place;    ///< the record place
  std::string mode;     ///< Topozentrisch or Geozentrisch
  std::string stz;      ///< the sidereal time line
  std::string lon;      ///< the longitude line
  std::string lat;      ///< the latitude line
  std::string date;     ///< the date line
  std::string ut;       ///< the clock line
  std::string weekday;  ///< the weekday
  // the labels, the caller translates them, empty falls back to his
  // German wording
  std::string name_label;    ///< Name:
  std::string place_label;   ///< Ort:
  std::string len_header;    ///< Länge:, the mode tag joins it
  std::string houses_header; ///< Häusersp.
  std::string mirror_label;  ///< Spiegelung:
  std::string quality_heading;  ///< Kard-Fix-Ver over the quality columns of the A4 sheet
  std::string element_heading;  ///< Elemente over the element columns of the A4 sheet
  // the paired-chart corners of Composit, Combin and Doppelkreis after
  // the a13aus split, the first name replaces the Name: block, the
  // short rows stack above the bottom left corner. No row is wide
  // enough to reach the wheel, empty fields stay away
  std::string pair_name1;    ///< 1: first chart, replaces the Name: block
  std::string pair_moment1;  ///< 1: the first moment, when the sheet corner shows another
  std::string pair_name2;    ///< 2: second chart
  std::string pair_moment2;  ///< 2: the second moment
  std::string pair_note;     ///< the Combin mean
  /// a COMBIN of more than two, his names stacked above the bottom edge
  std::vector<std::string> pair_list;
  /// bes10 of the composite, a midpoint chart has no speed, the Vel.
  /// column stays away
  bool longitudes_only = false;
  /// bes111 of a composite with real houses, cusp one reads H1 and the
  /// AC midpoint takes a row of its own under it
  bool h1_axis = false;
};

/// Writes the classic screen sheet of the original around a wheel
/// list, the body table with velocities down the left margin, the
/// house summary under it, the record corners like his HOROSKOP
/// GRAPHIK screen. Existing corner notes of the list are replaced.
///
/// @param dl a wheel list in classic sheet coordinates
/// @param chart the chart the list was built from
/// @param s its settings, the length mode tags the table header
/// @param txt the corner texts
/// @param mirrors the Spiegelung pairs of spieg1, listed under the caption
void add_classic_text(DisplayList& dl, const Chart& chart, const ChartSettings& s, const ClassicSheetText& txt,
                      const std::vector<std::pair<int, int>>& mirrors = {});

/// Writes only the record corners around the wheel, the screen sheet
/// uses this since its tables live in the panels.
///
/// @param dl the wheel list the corners join
/// @param txt the corner texts
/// @param left_x the left margin of the name and place blocks
/// @param center_x the mode line rides the wheel centre
/// @param right_x the left edge of the moment block
/// @param classic_place true sets the place block at the lower left like
///        his bes1 on the 640 sheet, where the lower right lies inside
///        the wheel, false stacks it over the moment at the lower right
///        of the wider screen sheet
void add_corner_text(DisplayList& dl, const ClassicSheetText& txt, double left_x, double center_x, double right_x,
                     bool classic_place = false);

/// Stamps the moment of printing onto a page like drad2 and dradst put
/// DATE$ and TIME$ behind the credit. A page without a credit line gets
/// the dradst line centred at its foot.
///
/// @param dl    the page to stamp
/// @param stamp the date and the time, DD.MM.YYYY HH:MM
void stamp_credit(DisplayList& dl, const std::string& stamp);

/// Draws the aspect histogram of asphist, the hits per divisor as bars
/// seventy units long at the maximum, the divisor left and the count
/// right of each bar, the sum under them.
///
/// @param dl        the display list to extend
/// @param aspects   the scan, zh holds the counts
/// @param divisors  the counted divisors, one bar each
/// @param xt        his xt&, the left end of the bars
/// @param yt        his yt&, the bottom of the first heading line
/// @param colors    the bar colours per divisor
/// @param fill      WEIß leaves the bars empty
/// @param head_size the text size of the headings
/// @param first_heading false leaves out "Aspekt 360/N :", the part his
///                  Aspektarium grab cut off
/// @return his yt& after the last bar, the caller's next line hangs off it
double add_aspect_histogram(DisplayList& dl, const AspectResult& aspects, int divisors, double xt, double yt,
                            const std::array<Rgb, 17>& colors, RingFill fill, double head_size,
                            bool first_heading = true);

/// The DIN A4 print page of the original a11 in moda 3, the wheel over
/// the bes_big table boxes on a 640 by 980 portrait page. Ported from
/// bes1_big, bes_big_plan, bes_big_asp, bes_big_haus, bes_big_elem,
/// bes_big_kafige and bes_big_halbs in the order the original drew them.
///
/// @param chart     the computed chart
/// @param s         its settings
/// @param aspects   the aspect scan of the chart
/// @param midpoints the halbs1 scan for the Halbsummen box
/// @param txt       the corner texts of the sheet
/// @param opt       wheel options, the scale is set inside
/// @param hist      the counted element and quality columns
/// @param hist_mode his elem switch, signs alone, signs with the house
///                  counts stacked on top, or no histograms
/// @return the page as one display list, 640 wide and 980 tall
[[nodiscard]] DisplayList a4_print_sheet(const Chart& chart, const ChartSettings& s, const AspectResult& aspects,
                                         const MidpointResult& midpoints, const ClassicSheetText& txt,
                                         const WheelOptions& opt, const Histogram& hist, HistogramMode hist_mode);

/// The unicode glyph of a body slot, his two letter tag where none
/// exists, shared by every drawing that stamps bodies.
///
/// @param slot a body slot 0 through 40
/// @return the glyph as UTF-8 text
[[nodiscard]] const char* body_glyph(int slot);

/// The unicode glyph of a zodiac sign.
///
/// @param index 0 for Aries through 11 for Pisces
/// @return the glyph as UTF-8 text
[[nodiscard]] const char* sign_glyph(int index);

/// The glyph of an aspect family, his asps& sprites.
///
/// @param family the divisor of the reduced aspect, 1 the conjunction,
///               17 to 19 the Quintile family variants of his sprites
/// @return the glyph as UTF-8 text, empty where he had no sprite
[[nodiscard]] const char* aspect_glyph(int family);

}  // namespace horcom
