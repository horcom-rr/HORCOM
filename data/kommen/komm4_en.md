# Explanation of Chart Types

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

## CHANGE CHART DEFAULTS

Here the parameters for the chart displays are to be set. Among other things, the "MUNDANE CHART" mode can also be set here. This is a TOPOCENTRIC-EQUATORIAL type of display:

In the "MUNDANE" display, the respective directly visible HALF DAY or NIGHT arc in the HORIZON system is set in proportion to 90 deg AR each. With this projection the Placidus houses each have 30 deg on the equator. This type of display has particular significance for the MUNICH RHYTHM THEORY of W. DÖBEREINER, which as far as I know E.R. DOSTAL was the first to point out. The equator as reference system should yield more precise triggerings, especially at higher geographical and ecliptical latitudes. The house system of Placidus is obligatory here. Some details are presented, for example, in the manual for the ASTRO pocket calculator "HORUS" (author E.R. DOSTAL). See also under: "Die Mundane Hausposition", published by AKIMOTO, Nov. 1982.

The mundane display has a close relationship to the PRIMARY directions as described by E.C. KÜHR ("Berechnung der Ereigniszeiten").

How far other applications of the MUNDANE version are meaningful is left open.

With regard to the aspects within the M.R. I have also evaluated these ecliptically in the mundane display and, in order to avoid confusion, have not drawn them in. This point seems to need clarification.

For now, in MUNDANE mode, PROGRESSION and DAILY CHART have still been left accessible.

The remaining items under "CHANGE CHART DEFAULTS" are self-explanatory. The last three items allow the colour choice of the zodiac ring and of the aspect lines. That requires some practice and some experiments. With 16 colours the finer colour nuances are not possible! Hatched areas need less colour when printing hardcopies. Your own settings of COLOURS, HATCHING, LINE COLOURS and STYLES are saved. The HORCOM STANDARD settings are fixed and can always easily be reactivated.

If you have not yet defined your own colours, you should always choose HORCOM standard, otherwise the colour areas may possibly be black!

By means of the Page-Up key or "R" you can reach the preceding dialogs, in case you have overlooked something.

****************************************************************************

## CHART GRAPHIC

Draws GEOCENTRIC or HELIOCENTRIC charts, depending on the preselected parameters. Among other things, the START of the chart, at the far left, can be chosen freely.

The chart DIAGRAM can contain various EVALUATIONS, namely: An aspect distribution up to the 16th division of the circle (corresponding to 22.5 degrees), if the orb is calculated in HORCOM manner (see below), if not, only up to the 12th division.

A HALBSUMMEN list: Direct, square and semi-square aspect, where the orb is respectively 1, 1/2, 1/4 degree, if calculated in HORCOM manner, or in the corresponding gradation if the orbs are defined by yourself.

The number of Halbsummen can optionally be quickly surveyed with a COUNTER.

During ZEITWANDERN or during the rapid scanning of data records in STATISTIK the Halbsummen are additionally summed up and the respective arithmetic mean of the accumulated sums is displayed.

A weighting of the "ELEMENTS" by "points":

Until April 97 with the following fixed values:

```text
  SO MO ME VE MA JU SA UR NE PL DR DS  KLEIN-PL. AC MC DC IC
   6  6  3  3  3  2  2  1  1  1  1  1  0...       6  6  4  4   Punkte
```

Planets in the FIRST HOUSE and the BIRTH RULER are weighted DOUBLE.

A weighting by "CARDINAL-FIXED-MUTABLE = KARD/FIX/GEM" by "points" with the same valuations as for elements, however IC and DC are not counted separately here, since that would amount to an overvaluation.

Since May 97 the individual point values can be defined by yourself between the limits 0...9, or the histograms can also be removed entirely (under 'CHANGE CHART DEFAULTS').

From May 97 the histograms can additionally also take the house occupation into account. This is to be seen above the sign occupation as a separate column. Often the house occupation compensates for a deficiency in the sign occupation.

With regard to the ORB, HORCOM normally proceeds as follows:

```text
          TARGET angle - ORB < ACTUAL angle < TARGET angle + ORB,
               ORB = BASE ANGLE/30
```

BASE ANGLE is here the angle of a series that results from dividing 360 degrees by a consecutive whole number.

The series of DIVISORS and corresponding BASE ANGLES up to the 16th division is accordingly:

```text
DIVISOR    :   1       2       3       4       5        6        7       8
BASE ANGLE : 360     180     120      90      72       60       51.43    45
DIVISOR    :   9      10      11      12      13       14       15      16
BASE ANGLE :  40      36      32.73   30      27.69    25.71    24      22.5
```

For example, the following holds:

ANGLE 135 degrees has BASE ANGLE 45 degrees, so ORB = 45/30 = 1.5 degrees

```text
        360 (Conj.)               360                  = 360/30 =12
        120 (Trine )              120                  = 120/30 = 4
        150 (Quincunx)             30                  =  30/30 = 1
         90 (Square)               90                  =  90/30 = 3
         30 (Semi-sext.)           30                  =  30/30 = 1
```

This procedure was, as far as I know, recommended by J. ADDEY and is also used by H.J. WALTER ("ENTSCHLÜSSELTE ASPEKT-FIGUREN") for the "WAVE MODEL", although there with a larger orb factor. A particular aspect is assigned to each planet.

The thoughts of LUTZ RATHKE on this subject have also recently strengthened me in these ideas. His aspect theory is very interesting and can only be recommended to classical astrologers too for their attention. He finds in part different assignments to the planets than H.J. WALTHER. Some aspects occur as MULTIPLES in several aspect series, the conjunction in all, the opposition in the 45, 30, 60, 90, 180 series and so on. This makes the differing "STRENGTH" of the aspects plausible.

Through the proportionality of the orb to the base angle it is also ensured that "WEAK" aspects have a SMALL orb, "STRONG" ones a LARGER ORB. Also a GEOMETRIC "EQUAL PROBABILITY" of all aspects is sufficiently plausible if one regards these as "series" or as a "wave model" after H.J. WALTHER for a particular divisor. For example, the sextile still occurs in the trine and the opposition, so that it seems justified to give, for example, the "pure" sextile a smaller orb than the trine or the opposition.

If one considers only individual aspects, each one would have the same geometric probability for a given orb. But if one were to give all aspects the same orb, these would no longer be distinguishable at higher divisors.

This too seems to me to speak for the manner of orb choice described above.

Another sensible concept would be to require the SAME PROBABILITY = FREQUENCY for all aspects.

This is best done EMPIRICALLY, since the astronomical a-priori probability is, geocentrically, a rather complicated topic.

By means of the ASPECT COUNTER in "ZEITWANDERN" the following orbs were determined which, between 1930 and 2050 GEOCENTRICALLY, averaged over about 2000 points in time, exhibit roughly the same probability:

```text
Divisor     :   1    2    3    4    5    6    7    8    9    10    11    12
Base angle  :  360  180  120  90   72   60   51.4  45   40   36    32.7  30
Orb         :  5.4  6.8  3.1  3.0  1.6  2.6  1.0  1.5   1.0  1.5   0.6   1.0
```

HELIOCENTRICALLY the following result in the same way:

```text
Divisor     :   1    2    3    4    5    6    7    8    9    10    11    12
Base angle  :  360  180  120  90   72   60   51.4  45   40   36    32.7  30
Orb         :  6.51 5.36 3.18 3.0 1.58 2.01 1.02  1.51  1.0 1.53   0.58 1.50
```

After the above "WAVE MODEL" = HORCOM normal setting, the following series results:

```text
Divisor     :   1    2    3    4    5    6    7    8    9    10    11    12
Base angle  :  360  180  120  90   72   60   51.4  45   40   36    32.7  30
Orb         :   12   6    4    3   2.4  2.0  1.71  1.5 1.33  1.2   1.09  1.0
```

For the square all three series give the same value.

The absolute values are chosen arbitrarily.

Under "CHANGE CHART DEFAULTS", however, a FACTOR (10 to 300%) can be introduced in front of the "NORMAL" orb, which enters into ALL orbs. This is especially sensible when in certain cases one obtains too many aspects and threatens to lose the overview. Then one can use it to reduce ALL orbs, and so on.

As a consequence of LONG LASTING aspects between slow-moving planets, however, the probabilities during such phases may indeed be temporarily considerably larger or smaller.

For scientific investigations it will therefore be necessary to clarify such conditions for the investigation period, whereby the Zeitwandern with aspect counter can be useful in each case!

For the MIRRORING at the axis 0 AR-LI or 0 CN-CP the orb was assumed throughout, also for M.R., to be 2 degrees, for the DIRECT HALBSUMMEN to be 1 degree, if the orb choice is left to HORCOM.

The "free choice" of the orbs of the base angles I have, since January 94, now likewise made possible at the repeated request of customers. If you wish to make use of this possibility, which you should only do after some practice with HORCOM, the following applies:

* As default before a FIRST choice of your own, those mentioned above apply.

* Only the divisors from 1 to 12 are then counted through, that is, all base angles from the CONJUNCTION up to the SEMI-SEXTILE.

* All aspects are valued EXCLUSIVELY, that is, a trine is not also valued as a BI-SEXTILE, an OPPOSITION not as a BI-SQUARE and so on, as is the case with the "WAVE" concept described above, to which I personally lean more, but which is not customary among most astrologers.

* You can now enter under "CHANGE CHART DEFAULTS" the orbs that seem right to you. The orbs I consider appropriate for this method of counting are listed as default values, but you can also define values that deviate from them. One should, especially when larger divisors (>6) are also entered, not use substantially larger orbs than suggested. With considerably too large orbs an error message follows, since otherwise an unmanageable jumble of aspects would result in the output graphics.

For aspects that you fundamentally do not observe, you can enter ZERO. * For the M.R. of W. DÖBEREINER, 4 degrees apply for square, trine and opposition and 6 degrees for conjunction as guide values (see the literature on M.R.). Practising adherents of this school should accordingly enter these angles. The sextile is mostly not observed in the M.R., but can be added in HORCOM.

* The general orb factor and any entered planet factors always enter in, if they deviate from 1 or 100%! (see below) * With regard to the compound aspects such as 3*45 degrees or 5*30 degrees (= 1 1/2 square or quincunx), the orb of the responsible base aspect is taken as basis, as also in the "WAVE" concept above. If you are, for example, in love with the quincunx and want to see it more often, you can enter a larger orb at the base angle 30 degrees. Promptly you will come across your beloved aspect at every turn. Whether that leads to "insights" may be doubted.

* Also for MIRRORING at the cardinal axes and for the HALBSUMMEN the base values can be changed.

The PLANET ORBS can be weighted individually.

(Under "CHANGE CHART DEFAULTS".) The normal value is "100 %" each. Any percentages can be entered (at your own responsibility), where for example 200% means that the planet in question occurs with double weight, that is, on average also twice as often. The value 200 % thus corresponds to a valuation factor of 2.

Correspondingly, for example, 50% means a valuation factor of 1/2. It will then occur with only half the probability.

If one deletes the individual defaults, then all planets are again valued with the same weight 100 %, corresponding to the factor 1.

So if you wish to "forget" a planet in terms of aspects, then enter 0%. It is then no longer counted.

The general orb factor mentioned above and the factors assigned to the individual planets always appear as a product, which of course only becomes noticeable if at least one of the two factors differs from 1 or 100%. If, for example, both factors are 50%, then the resulting factor is 25%, and so on.

If the matter with these orb factors is too complicated for you, simply skip the relevant prompts. All factors are then 1 or 100% and you need not concern yourself with it further.

The aspect valuation proposed by HORCOM has proved itself well in my practice and yields in the chart manageable and interpretable figures.

The ASPECT LINES that you wish to see in the CHART you can specify individually up to the 12th division. If you have marked a line, then it appears next to the selection box as it looks in the chart. In addition, in the relevant row of the selection box the symbol *---* appears as a mark that the aspect in question appears in the chart. If you also wish to select compound aspects, it is advisable beforehand to choose at least 12 as the maximum divisor. For example, the QUINCUNX = 5*30 requires the semi-sextile as base angle and therefore the divisor 12.

A CONJUNCTION within the given orb is indicated by a small circle.

If you have a monochrome monitor it will be somewhat difficult to tell the various lines apart. Here the colour monitor is an advantage. If the "BLACK MOON" is chosen, then the apsidal line of the lunar orbit is drawn in as a dashed line, with the symbol of the Black Moon at the APOGEE. AG = longitude of the apogee = "Black Moon".

The BIRTH RULER is highlighted inversely on the screen. In the PRINTER GRAPHICS it is highlighted in colour, GREEN for SO, MO, ME, VE, JU and RED for MA, SA, UR, NE, PL.

The PLANET LONGITUDES, scaled to degree/min or /SEC, are entered on the left in the CHART DIAGRAM. Behind that stands the DAILY MOTION "TB". Negative sign = retrogression.

The abbreviations for LONGITUDE:

* (W) = TRUE
* (A1) = APPARENT1 (with light travel time)
* A2 = APPARENT2 (LTT + ANNUAL aberration).
* P = WITH PARALLAX.

For the LUNAR NODE and APOGEE the following mean:

M = mean value, W = true value. Normally the ROUNDED DEGREE VALUE is written into the chart. Overlaps with the symbols cannot always be ruled out in this. Under DEFAULTS this degree value can also be eliminated.

Detailed information (for example "parallels") is given by PLANET COORDINATES... Below the longitude table the HOUSE CUSPS are entered. In detail, see HOUSE TABLE.

When working at the SCREEN it is possible to select INDIVIDUAL PLANETS in the CHART, so that ONLY THESE are visible. Or individual planets can only be marked RED:

For this, from the SCREEN CHART graphic press the RIGHT MOUSE BUTTON and then select accordingly!

A possibly preselected FIXED POINT can only be influenced via CHANGE EPHEM. DEFAULTS!

***************************************************************************

## ASPEKTARIUM

Breaks down the aspects with divisor 1 up to a maximum of 16 individually. The maximum divisor is selectable in three steps: 8 / 12 / 16, 16 only if the orb counting is left to HORCOM.

Above the diagonal the actual angular distances of the planets are entered in degree/min, insofar as aspects are found.

The aspects CONJ., OPP., TRINE, SQUARE are marked here by small rectangles, at the lower left in each case, the further aspects by a small triangle.

Below the diagonal is the actual aspektarium:

The MAIN aspects are entered with the well-known SYMBOLS, the aspects with divisor 5 to 16 are entered, possibly as rounded, TARGET degree values. The DIVISOR is entered small ABOVE. With regard to the ORB, see above. The numbers in the diagonal give the NUMBER OF ASPECTS for the factor standing above, with which the strongly aspected planets immediately catch the eye. With more than 5 additional factors, aspects are also counted here of which one partner no longer finds room in the graphic. This program also runs heliocentrically.

*****************************************************************************

## HALBSUMMEN GRAPHIC

This program is intended for adherents of the R. EBERTIN school and for the HAMBURG school. The Halbsummen are each assigned to the central factor. The letters on the axis mean:

D = Direct, Q = Square, H = 45 degrees, V = 22.5 degrees.

The normal orb here is respectively: 1, 1/2, 1/4, 1/8 degree.

The orb can possibly be changed under "CHANGE CHART DEF.". If the last Halbsumme in a scale is framed, this means that at the given orb not all values had room anymore.

After pressing the space bar the tables (scales) are sorted, and indeed in such a way that multiples of 45 degrees are sorted in next to one another and thus "PLANETARY PICTURES" become apparent. The sort criterion in degrees is to be read off at the lower end of the scale in each case. If LA is the absolute longitude of the central factor in degrees, then this number is therefore LA*8, reduced to 360 degrees. In this it is to be noted that the orb, referred to these numbers, is likewise enlarged by the factor 8!!

If two Halbsummen axes lie less than 1 degree base orb apart, then the numbers in question are drawn in RED!

The number of Halbsummen can optionally be quickly surveyed with a counter.

***************************************************************************

## MULTIPLE DIRECTIONS after STEPHAN A. LEHRIEDER

Have in part a connection with the HARMONICS, only that here also and precisely NON-integer multipliers are used, namely the LIFE AGE in decimal specification: the number of days between birth and event divided by the length of a tropical year (365.2422) yields LJ = life years.

There are so far the following variants:

MULTI 1:

The chart factors (planets and houses) are multiplied by LJ according to their position in the respective SIGN (0 to 30 degrees), added to the respective ecliptic longitude and reduced to 360 degrees.

MULTI 2:

The chart factors are multiplied by LJ according to their position in the ZODIAC (0 to 360 degrees), added to the respective ecliptic longitude and reduced to 360 degrees.

MULTI 3:

The chart factors (planets and houses) are multiplied by LJ according to their position in the respective SIGN, all added to the same PRESELECTED chart factor (ecl. longitude) and reduced to 360 degrees.

MULTI-0-EAST:

The chart factors (planets and houses) are multiplied by LJ according to their position in the respective SIGN, all added to the ZERO POINT of the signs ruled by them, whereby according to LEHRIEDER the following assignments apply: SA rules in AQ, JU in PS, MA in AR, VE in TA, ME in GM. The remaining planets correspondingly to the signs they rule. CHIRON is valued as ruler of Virgo!

MULTI-0-WEST:

The chart factors (planets and houses) are multiplied by LJ according to their position in the respective SIGN, all added to the ZERO POINT of the signs ruled by them, whereby according to LEHRIEDER the following assignments apply: ME rules in VI, VE in LI, MA in SC, JU in SG, SA in CP. The remaining planets correspondingly to the signs they rule. For AC, 0 deg AR is given, for MC 0 deg CP, for DR 0 deg CN, for AG 0 deg CN and for the PART OF FORTUNE 0 deg AR.

CHIRON is valued as ruler of Virgo!

Since August 1998 a further version of the Multiples has been introduced on a trial basis within HORCOM5P/7P, at the suggestion of and in consultation with Mr. Lehrieder, which are designated MULTI-ARC.

Here the ecliptic distances of the planets from one another, referred to a reference point (as with MULTI 3), are evaluated. In effect this version resembles MULTI 3, only that here it is not the position in the sign that is multiplied by the life years, but the position in the zodiac. For the reference point 0 Aries this is identical with MULTI 2. There are as yet no experiences with it. Researchers to the front!

The ascending (mean) LUNAR NODE is treated like a planet in the Multiples and the descending one is DEFINED as +180 deg to it! The HOUSES are normally converted like planets. In this the arrangement of the houses may possibly change! The PART OF FORTUNE too is converted like a planet!

Optionally in HORCOM, on the basis of the new MC, with the corresponding sidereal time a regular house system (depending on preselection) can also be calculated. In this the PART OF FORTUNE is, if applicable, also recalculated on the basis of the new AC.

Incidentally, it is important to know that HORCOM has always taken into account the NUTATION of the Earth's axis in the house calculation. This amounts to maximum amplitudes of the order of up to about 30". When comparing with other calculations one should bear this in mind.

With MULTI-0-EAST and MULTI-0-WEST the corresponding signs are used as "ruled houses", that is, for example, Capricorn for the 10th house and so on. In the output of the MULTIPLES, the aspects between the RADIX and MULTIPLE factors are displayed on the left in the image, right next to it those among the MULTIPLES with one another, whereby a very small orb of +-0.2 deg is by the book. R stands for RADIX, M for MULTIPLE.

Below that, the HALBSUMMEN determined with the same orb to the SIGN cusps and the HOUSE cusps (for RADIX or MULTIPLE houses) are displayed.

Afterwards the birth time can be changed (ZEITWANDERN) and thereby the aspects can be studied.

This is, for example, very useful for CORRECTION of the birth time.

Working with MULTIPLES presupposes a very precise ephemeris calculation. In the current HORCOM this is given generally for SO, MO, ME....NE, for PLUTO and CHIRON only between 600 BC and 2200 AD, for CE, PA, JN, VS between 1500 AD and 2100 AD. However, one should use the latter with caution, since although they were calculated very precisely for astronomical reasons, because of the multitude (about 10000) of influencing bodies which do not enter into the calculation (not even with the professionals), over longer times deviations must be reckoned with.

Other additional planets must drop out for the MULTIPLES. Only the PART OF FORTUNE and the MEAN APOGEE of the MOON (BLACK MOON) are possible as additional factors. The analogous holds for the lunar node. For the BLACK MOON and the LUNAR NODE the (very precise) MEAN VALUES are therefore always calculated, regardless of what is set generally.

HYPOTHETICAL "planets" such as TRANSPLUTO (ISIS) or those of the Hamburg school are ruled out in principle in my opinion for this method, since second-accurate positions cannot here of course be spoken of even remotely.

ATTENTION!! It is very important to note which calculation modalities are set for the ephemeris calculation in each case. The calculation "WITH PARALLAX" yields considerably deviating values for the moon and the inner planets! The "multiple" moon with parallax can, for example, at a life age of 60 years, deviate by more than 2 signs from one calculated without parallax. The corresponding difference can amount to up to 45'. Astrologically "correct" are without doubt the values WITH parallax, since the event takes place not at the Earth's centre but at the event location on the Earth's surface. You can research how far this could be significant, but should always know what you are doing.

Mr. LEHRIEDER has, since the middle of 1998, brought out his experiences with the MULTIPLES in book form (self-published) in a very fine form: Edition BONASTRO Stephan A. Lehrieder, am Kavierlein 12, D-90765 Fürth. Price about DM 78.-

HARMONICS:

Result from a radix by multiplying all angles by an integer factor and reducing to 360 degrees. The 4th HARMONIC is, for example, essentially identical with the well-known 90-GRAD-KREIS. In contrast to the MULTIPLES described above, in the HARMONICS the multiplied positions are not added onto reference values, but stand on their own. The form of display has now been shaped analogously to the MULTIPLES. See above. The orb for the display of the aspects and Halbsummen is normally (at ORB FACTOR 1) fixed at 1 deg.

HARMONICS and ASPEKTARIUM have a close relationship: If, for example, the aspect with divisor 5 is particularly frequent, then the planets involved stand in conjunction in the 5th Harmonic, and so on.

The general points of view mentioned above under MULTIPLE apply here too.

*****************************************************************************

For COMPOSIT, COMBIN and DOPPEL-KREIS, FIRST MAKE SURE THAT TWO .. DATA RECORDS ARE PRESENT and then select the corresponding program. From there, in the DOUBLE CHART, you are then prompted one after another to activate the data records. The procedure is somewhat cumbersome, but allows the use of arbitrary data records, for example RADIX and SOLAR. The prompt for input is emphasized by the fact that the mouse arrow becomes a VERTICAL DOUBLE ARROW. As long as this is visible, every activation is fed to the respective DOUBLE program! The activation always happens via the main menu, column "INPUT-OUTPUT".

With the COMPOSIT you enter two data records one after another, with the COMBIN there can be up to 5.

## COMPOSIT

Calculates and draws a composite chart of two partners. The "planets" are HALBSUMMEN of the two partner values.

Of the two possible values of the Halbsumme, the one closest to the two factors is used in each case.

The composite is astronomically not "real". It therefore does not for the time being cooperate with other evaluation programs.

The HOUSE SYSTEM of a COMPOSIT is problematic. In HORCOM5P/7P, therefore, three possibilities for the calculation of the houses are now provided for study purposes:

1. The mean sidereal time of the partner charts is calculated as well as the mean values of the geographical longitude and latitude, and then with these the house system in the usual way.

2. As a second possibility the method of ROBERT HAND is applied. Here the nearest Halbsumme of the two MC values is determined, the place of residence of the partners is entered and, starting from that, the house system is calculated, whereby the sidereal time is calculated back from the MC mean.

3. As a third method, simply the Halbsummen of the houses of the two partners are formed, and indeed starting from the nearest Halbsumme of the MC of the two partners. The Halbsummen of the remaining houses are chosen in such a way that a seemingly "normal" house system results. One cannot always use the nearest Halbsummen here, since this would possibly interchange the direction in the chart.

(As ecliptic obliquity the mean value of the two partners is used in every case.)

With the equal house systems, in order to avoid complete confusion, only the third method is used!

However one does it, none of the methods is entirely convincing, so that I would be inclined in every case to mark the houses in interpretation with a large question mark, including the so-called AC and MC values.

With the two former methods, house 1 is not identical with the AC Halbsumme. The nearest AC Halbsumme is therefore shown separately in the chart. It can in some cases even lie opposite house 1, since a Halbsumme is actually not a point but an AXIS. If equal houses are preselected, this is however dispensed with, since then the assignments would become too confusing.

Before one occupies oneself with interpretation of a COMPOSIT, one should have thoroughly clarified these conditions for oneself and studied them on a few examples.

*****************************************************************************

## COMBIN

(After PHILIP SCHIFFMANN, VIENNA) Calculates from 2 ...n data records a combined chart with the MEAN VALUE of the birth TIMES (Jd1+Jd2..)/n and the MEAN LOCATION COORDINATES. It is thus a "real" chart whose data are symbolically founded. With COMBIN many programs are accessible that also run for RADIX. So after COMBIN you can study DIRECTIONS and DÖBEREINER TRIGGERINGS, if you consider that meaningful. For the use of the program groups "SOLAR..." or "DAILY CHART.." you must first convert the COMBIN into a radix, with the menu item "RESULT as RADIX". That requires attention and practice. You can make COMBINS with up to 5 partners.

The relevant data records must be entered beforehand in order to then be able to click them one after another.

The display and evaluation with COMPOSIT and COMBIN is, moreover, analogous to the radix chart. Thus in both cases the HALBSUMMEN can also be printed out, which with COMPOSIT is of course PARTICULARLY QUESTIONABLE and intended ONLY FOR STUDY PURPOSES. It is, so to speak, a matter of "Halbsummen of Halbsummen".

***************************************************************************

## DOPPEL-KREIS

Serves for the direct chart COMPARISON.

Besides the (normal) 360-degree circle, a 90-GRAD-KREIS can also be displayed, as it is used in the HAMBURG and the R. EBERTIN school.

Here the cardinal, fixed and mutable signs are each in overlap in one of the three 30-degree sectors.

The main aspects are listed in a separate small ASPEKTARIUM. It is expedient to preselect a SMALLER ORB FACTOR (for example 50%). Under the aspect symbol stands the rounded current value within the preselected orb.

I stands for INNER, A for OUTER. If this aspektarium overflows, the only possibility that remains is to reduce the orb (see CHART DEFAULTS..). This program also runs HELIOCENTRICALLY.

ARBITRARY data records can be combined.
