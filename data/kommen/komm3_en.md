# Ephemeris Explanation

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

## PRELIMINARY REMARK

For the major planets ME, VE, EARTH (SUN) MA, JU, SA, UR, NE, HORCOM uses an abbreviated version of the latest analytical theory VSOP87 of the BUREAU DES LONGITUDES, PARIS ( P.BRETAGNON ). This has now been made accessible to a wider circle by JEAN MEEUS ("ASTRONOMISCHE ALGORITHMEN", WILLMANN BELL INC, JOHANN AMBROSIUS BARTH, ISBN 3-335-00318-7).

The numbers for the planetary formulae are available on diskette, taken directly from the magnetic tape of the INSTITUT DES LONGITUDES, so that the possibility of transcription errors is largely ruled out. For these planets alone, this amounts after all to about 8500 numbers ( of the abbreviated theory ).

These numbers are read in from diskette at program start and this requires a few seconds of time.

For the present program these files were converted to GFA-BASIC.

```text
             This program contains software code
             Copyright (c) 1991-1992 by Jeffrey Sax
             and distributed by Willmann-Bell, Inc.
                         Series 10756
```

Despite their relative compactness, the routines represent a very high level of precision, which is better than many available printed ephemerides, which as a rule cover at most 150 years and often give the planetary positions only to the nearest minute.

These routines, by contrast, give, in the period 2000 BC to AD 6000, the Sun and Venus with an error < 1", the other planets mentioned with about < 4". The data given refer to EPHEMERIS TIME, or DYNAMICAL TIME, which is nowadays defined by atomic clocks.

This dynamical time runs completely uniformly, whereas UT ( = GMT ) is subject to a long-term drift as well as to irregular fluctuations. For dates before -2000, slowly increasing errors are to be expected for SO, ME, VE, MA, JU, SA, UR, NE, but these are irrelevant, since in this date range the relationship between UT ( = GMT ) and ephemeris time is in any case increasingly lost.

For PLUTO, unfortunately, no correspondingly precise analytical theory exists. Here I have computed an accurate ephemeris by numerical integration over the range 18.05.602 BC to AD 12.06.2201. Beyond that, analytical approximation formulae by J.CHAPRONT ( BUREAU DES LONGITUDES ) are used. Their error can grow to several minutes in remote historical times ( BC ).

In the integration, the perturbations by the major planets were taken into account. The perturbations by the thousands of asteroids and by the objects in the "KUIPER belt" naturally had to be left out of consideration.

From HORCOM7P on, an ephemeris was also computed for the planetoid QUAOAR, for the same period as for Pluto and with the same computational accuracy.

QUAOAR runs outside Pluto's orbit, but, unlike Pluto, has a low eccentricity and orbital inclination, that is, like a true major planet. However, it has only about half the diameter of Pluto. As the SYMBOL for QUAOAR a "Q" was used for the time being, as long as no generally accepted symbol yet exists.

From 10 February 2006 on, the planetoid XENA was also numerically integrated over the same period of about 2800 years. It is larger than Pluto and, with a strongly eccentric and strongly inclined orbit, runs for the greater part outside Pluto's orbit.

As the SYMBOL for XENA an "X" is used for the time being.

The characteristic approximate orbital data for PLUTO, QUAOAR and XENA are:

```text
   Name          Abbr.    a        e     i (deg)      T (years)
   PLUTO         PL     39.65    0.25   17.15°       250
   QUAOAR        QU     43.25    0.035   7.99°       284
   XENA          XE     67.66    0.44   44.18°       557
```

Here:

```text
              a = semi-major axis   e = eccentricity
              i = inclination       T = orbital period
___________________________________________________________________________
```

For the relationship between UT = GMT and ephemeris time = ET, the latest documents were used, which guarantee this relationship reasonably well only back to a few hundred years BC. For the future, this relationship can likewise only be surmised or extrapolated. In this respect, the temporal span of this program is completely sufficient, both for the historically researching astrologer and for the "future-seer", for the dates that come into practical consideration.

On this see also further below under "PLANETARY COORDINATES".

CHANGE EPHEMERIS DEFAULTS: serves for entering the parameters for the ephemeris computation and for choosing the ADDITIONAL planets.

The values already PRESET can be recognized by the respective THICKLY FRAMED box.

In detail:

- For the LONGITUDES one can choose between the TRUE = GEOMETRIC and the APPARENT values, whereby in the latter case a further distinction is made between taking into account the LIGHT-TRAVEL TIME alone ( = astrometric coordinates ) and additionally the "ANNUAL ABERRATION". For the Moon all these differences can be neglected.

Abbreviations:

- W = True ( = Geometric ) position.
- A1 = Apparent position, when only the LIGHT-TRAVEL TIME is taken into account. Here the distance of the planet is decisive ( = "astrometric" position).
- A2 = Apparent position, when the ANNUAL ABERRATION is added. Here the orbital velocity relative to the observer is also decisive. This is generally used only for fixed-star positions and is therefore switched on automatically in this program in order to allow possible comparisons.

All these distinctions play out below one arc minute, but they are interesting if one wants to compare with printed ephemerides. Unfortunately, in astrological ephemerides it is not always stated which values are meant and which time (UT or ET) underlies them (see below!). In any case, a correct comparison requires some basic astronomical knowledge, which few astrologers are familiar with. So may the accuracy fanatics please first make sure that they are not comparing "apples with pears" before they find inaccurate numbers! With older ephemerides, incidentally, errors of up to about 0.5' are to be expected for Pluto and Neptune, since before 1978 a much too high value was assumed for the mass of Pluto.

For solar returns the choice of these calculation modalities is of hardly any importance. For directions it can have an effect in somewhat different points in time. What is now "correct" shall not be decided here. Mostly the version designated APPARENT1, or A1, is applied. It is of course important that for solar returns etc. one always computes with the same parameters as for the radix.

- For the LUNAR NODE one can choose between the usual MEAN value and the TRUE ( = "osculating", or instantaneous) value. The mean value can be given very precisely, the true value can have an error of a few arc minutes.

- "PARALLAX" serves for choosing the planetary computation with or without taking parallax into account. For lunar returns the difference can amount to more than 1h! With parallax means: the coordinates are determined TOPOCENTRICALLY, that is, for the respective location of the observer on the Earth's surface, which is of course also time-dependent as a consequence of the Earth's rotation. Designated by "P" or by corresponding text. Parallax is now computed for all planets. For the outer planets NE, PL the effect lies below the computational accuracy. For Venus it can amount to up to 34", for Mars up to 24", for Jupiter up to 2.2". There can surely be no doubt that the topocentric positions, that is, WITH parallax, are the astrologically "more correct" ones.

- "ADDITIONAL PLANETS" allows the inclusion of the "ASTEROIDS" CHIRON, CERES, PALLAS, JUNO, VESTA. Temporal validity, see below.

The "TRANS-PLUTO" ( often also called ISIS ) is likewise to be selected under ADDITIONAL PLANETS (see below), as is the PART OF FORTUNE.

The 8 "HAMBURG" planets can likewise be chosen ( see below ).

From HORCOM7P on, an additional 7 objects have been included:

Of astrological interest can probably be, above all, those asteroids which, similar to CHIRON, have a large mean distance = a and a large eccentricity = e, so that among the thousands of asteroids they could be distinguishable. Often these objects are designated "CENTAURS". Some also have corresponding names.

In total, from HORCOM7P on, 4 such objects ( including CHIRON ) are included, plus the comet HALLEY, which has an even more extreme orbital characteristic. In the following table, approximate values of some characteristic heliocentric orbital data are listed. Here i is the orbital inclination against the ecliptic, T the approximate orbital period.

In the case of the comet HALLEY the orbital inclination is almost 180 degrees, so that heliocentrically it runs retrograde to the other planets.

```text
   Name          Abbr.      a        e     i (deg)      T (years)
   COMET HALLEY  HL        17.94    0.97  162.24        76
   Chiron        CH        13.61    0.38    6.94        50
   Damocles      DA        11.82    0.86   61.84        41
   Nessus        NS        24.46    0.52   15.66       121
   Pholus        PH        20.23    0.57   24.70        91
```

For all these objects I computed the ephemerides by numerical integration in the ensemble with the major planets, so that their influences are fully taken into account. As starting values the latest orbital elements were used ( of course much more accurate than the guideline values given above ).

Information about the SYMBOLS used is given by the table "ADDITIONAL-PLANET COORDINATES" ( second sheet! ).

The time spans of the ephemerides computed by me and stored in HORCOM7P are summarized once more:

```text
   PLUTO, QUAOAR and XENA :
   JD 1502079.5 to 2525120.5  = 18.05.602 BC to 12.06.2201

   CHIRON:
   JD 1502279.5 to 2524460.5  = 07.01.600 BC to 21.08.2199

   ASTEROIDS CERES...VESTA and the 3 "CENTAURS" DAMOCLES, NESSUS
   PHOLUS :
   JD 2268939.5 to 2488390.5  = 08.01.1500      to 18.11.2100

   Comet HALLEY :
   JD 2305446.5 to 2469806.5  = 31.12.1599      to 31.12.2049
```

According to my calculation, the comet HALLEY will, at its next return ( 2061 ), come very close to Venus at perihelion and thereafter have considerably altered orbital data.

The chosen additional planets appear in the horoscopes and other representations. In the table "ADDITIONAL-PLANET COORD." all are shown on two sheets. In the program "ASPECTARIUM" a maximum of 5 can be processed!

The more additional planets are chosen, the higher the computation and evaluation times become ( for aspects and midpoints up to 10 times the value! ). Moreover, overlapping of the symbols in the horoscope, or clipping of graphics, can occur more often. So: do not select more additional elements than appropriate!!

The LUNAR APSIDES ( = "BLACK MOON" ) are also selectable among the additional planets. SYMBOL: 2 intersecting circles. One can choose between the MEAN value and the TRUE ( = "osculating" = instantaneous) value. Here the true value can differ from the mean by more than 20 degrees.

The "BLACK MOON" is the TRUE ( = INSTANTANEOUS) APOGÄUM = AG of the MOON's ORBIT. The TRUE value is in each case marked by an INVERTED SYMBOL, or with "W" (in the printer graphics by a rectangular frame around the symbol), the MEAN one by "M". The mean value is in each case exact, the true one a good numerical approximation with a few arc minutes maximum error. The "black Moon" is not a planet but an "axis". An aspect with angle W to the B.M. is thus equivalent to W+180 to the opposite pole. An inclusion of the opposite pole with its own symbol, as is indeed the case for the lunar nodes, has not been carried out for the time being. It is therefore advisable, when judging aspects etc. to this factor, also to use the horoscope diagram, in which the axis is entered as a dashed line.

ATTENTION! For the black Moon different definitions are in use. In HORCOM, for the TRUE value, the "osculating" apside of the lunar orbit is understood by it. These values correspond, with deviations of a few arc minutes, for example to those of the "ROSICRUCIAN EPHEMERIS", which in turn corresponds to the ephemeris DE200/LE200 of the US NAVAL OBSERVATORY. As regards the MEAN value of the lunar apsides, there is uniformity.

## "CHANGE EPHEMERIS DEFAULTS"

With this the parameters mentioned above can be preset. From HORCOM5P on, a FIXED POINT ( = fixed ecliptic longitude ) can also be preset under "CHANGE EPHEMERIS DEFAULTS". The fixed point has the symbol "F" in red color and the short designation FP. It appears, when preselected, in most outputs. With it you can, for example, study the influence of particular zodiac degrees. The FIXED POINT is treated identically geocentrically and heliocentrically, thus corresponds to an infinitely distant planet without ecliptic latitude, which only takes part in the precession, since it is fixed in the zodiac. Strictly speaking, geocentrically, the nutation of the Earth's axis would also have to be added, but this would complicate the matter unduly. This effect lies below about 30 arcsec.

In the STATISTICS files the fixed point is not stored, since here the input of such a point is possible anyway, whereby an already defined fixed point may be overwritten.

In the MUNICH RHYTHM THEORY a fixed point can likewise be entered, which, however, has a special meaning there. It is now likewise designated there as a red "F", but need not be identical with the "normal" ( other ) fixed point, rather it is defined and stored independently of it. It is also named "SP".

Otherwise the fixed point is treated like a planet in all evaluations. In "DYNAMOGRAMS" it is counted like one of the inner planets ( ME to MA ).

The orb factor can be set under "CHANGE HOROSCOPE DEFAULTS".

ATTENTION!!

The values of exactly 0° and 360° can NOT be chosen as a fixed point for technical ( arithmetic ) reasons, or would then be partly ineffective.

## PLANETARY COORDINATES ( helio- or geocentric )

The program outputs the planetary coordinates for the currently activated data record.

(The subsequent "VARY TIME" allows the stepwise variation of time in order to make detailed studies. The routine resembles that in "TIME WANDERING", see Explanation 9 regarding CORRECTION. The data record can here be overwritten with the varied time. So one can also use this routine for correcting.)

From all TABLE outputs, by pressing the FUNCTION KEY F2, one can view the respective horoscope in between and go back again with 'Esc'.

Conversion formulae were largely taken from the following books, which are a good first basis for the self-programmer, provided he has solid basic mathematical knowledge.

- "ASTRONAMICAL FORMULAE for CALCULATORS" by J.MEEUS / 1985 ( not a textbook )
- "ASTRONOMISCHE ALGORITHMEN" by J.MEEUS / 1992 ( not a textbook, but a treasure trove of astronomical formulae, which represents the latest state for the self-programmer and contains almost all desirable formulae. See above! )
- "GRUNDLAGEN DER EPHEMERIDENRECHNUNG" by O.MONTENBRUCK / 1985 ( TEXTBOOK ).
- "EINFÜHRUNG IN DIE HIMMELSMECHANIK UND EPHEMERIDENRECHNUNG" A.GUTHMANN B.I. WISSENSCHAFTSVERLAG ( TEXTBOOK ).

Literature references can be found in MONTENBRUCK. The original literature, however, requires considerable prior knowledge of spherical astronomy and celestial mechanics for its understanding.

For the MOON, PERTURBATION TERMS > 1 arcsec amplitude were taken into account. Despite the effort toward simplicity, the planetary computation has become quite extensive and needs some computing time.

The planetary computation is always carried out in ephemeris time, whereas the sidereal time, house computation etc. proceed from UT.

For the relationship between EPHEMERIS TIME (= ET) and UT, the data to be found in the American Ephemeris.. for the years 1620 to 1998 were used, outside this interval the latest approximation formulae ( F.R.STEPHEN SON, L.V.MORISSON, Phil.Trans.R.Soc.Lond. A 313, 47-70 / 1984 ). In remote historical times the use of these newer data can lead to considerable deviations in the position of the Moon compared to the values computed with the "Improved Lunar Ephemeris", since these still rest on an older approximation formula ( 1948 ). For the Sun the corresponding differences are about an order of magnitude smaller.

The difference between ET and UT grows, for example, up to the year AD 1 to about 2.7 h!!! (not linearly). The relationship is reasonably secured only back to 390 BC. For older dates, extrapolation is used. One may then no longer count on precise results.

For the future too, UT can only be determined approximately from the uniformly running ET. Per century the difference amounts to about 1-2 min.

These questions are only relevant for HISTORICAL horoscopes. But precisely here the computer also opens up new possibilities.

Since in the present program the time is always given as UT = GMT = GZ, outside the year 1900 ( where UT and ET approximately coincide ) noticeable differences from printed ephemerides, which often refer to ephemeris time, will arise for SO and MO. So if you COMPARE, first determine UT from ET (see below) and make sure what you are comparing! The GEOCENTRIC positions refer to the TRUE equinox (with nutation), the HELIOCENTRIC ones to the MEAN equinox, each of the current date.

For Sun and Moon the accuracy corresponds to a time resolution of less than 20 time SECONDS. That is sufficiently accurate for the production of solar and lunar returns.

The PARALLELS you take from the column DECLIN. of the table. "VEL." gives the INSTANTANEOUS change of the ecliptic longitude in '/day. For determining these values the formulae from J.MEEUS were likewise used. It is to be noted that this is the instantaneous velocity and not the daily longitude progress as it can be taken from the ephemeris tables!

The GEOCENTRIC MAXIMUM velocities of the planets in POSITIVE and NEGATIVE direction in ARC MINUTES / DAY were determined empirically ( statistically ) as follows:

```text
PLANET          MAXIMUM POSITIVE     MINIMUM or MAXIMUM NEGATIVE
                      min/d                   min/d

SUN                   61.188                  57.186
MOON                 922.28                  709.18

MERCURY              132.08                  -81.74
VENUS                 75.48                  -37.59
MARS                  47.47                  -23.91
JUPITER               14.53                   -8.19
SATURN                 7.82                   -4.95
URANUS                 3.68                   -2.51
NEPTUNE                2.27                   -1.70
PLUTO                  2.38                   -1.75

LUNAR NODE TRUE        2.334                 -15.768
APOGÄUM TRUE         367.33                 -213.24

CHIRON                 6.71                   -4.11
TRANSPLUTO             0.850                  -0.719
CERES                 26.94                  -14.58
PALLAS                35.29                  -21.99
JUNO                  34.96                  -16.59
VESTA                 32.25                  -16.45

CUPIDO                 1.600                  -1.265
HADES                  1.321                  -1.014
ZEUS                   1.094                  -0.897
KRONOS                 1.007                  -0.809
APOLLON                0.920                  -0.761
ADMETOS                0.892                  -0.713
VULKANUS               0.853                  -0.695
POSEIDON               0.773                  -0.640
```

TRANSPLUTO and the last 8 ( "HAMBURG" ) are HYPOTHETICAL!

Column A ( = Acceleratio ) contains the sign of the acceleration. In the case of a turning point one can thereby recognize whether the planet becomes direct (A positive) or retrograde (A negative).

The column "ENTF." gives the mutual distance between Earth and planet, measured in AU ( 1AU = mean distance Sun-Earth ). For the major planets ME, VE ......... NE, PL, optionally the relative values, referred to the mean, in % of the mean can also be listed.

Besides the positions in the ecliptic and equatorial reference system, the ecliptic positions of the mean northern or southern planetary NODES are also given.

The "SUN NODE" (= line of intersection between equator SO and ecliptic) was computed according to "GEOZENTRISCHE PLANETENKNOTEN" by DR.TH.LANDSCHEIDT. In geocentric consideration these are points in space, in heliocentric ones directions!

In the last column the mean PLANETARY APSIDES are plotted (also geocentric). At the top the PERIHELION, at the bottom the APHELION. These are the major axes of the orbital ellipses ( mean values ). In geocentric consideration these are points in space, in heliocentric ones directions! The values refer to the vernal point ( = node longitude + distance perihelion-node ).

These values are not used in astrology. If at all, then at most for the Sun (Earth). I have included them here because they are already present heliocentrically in the planetary computation and can be represented geocentrically too with little effort.

For a heliocentric astrology the apsides would most probably have considerable relevance, for the geocentric one this is less evident. The same applies to the planetary nodes.

The instantaneous ( true ) values of the APSIDES can, for the outer planets SA, UR, NE, PL, deviate by several degrees from the mean. I would therefore recommend limiting any studies on the astrological relevance of these elements to MO, SO, VE, MA, JU.

Mercury too is to be regarded with reservation because of its considerable orbital inclination.

The reference to the vernal point is probably the more appropriate choice for geocentric consideration, but only at low orbital inclination.

The entry "MOON PHASE" ( in the table header ) is the ecliptic longitude difference MOON-SUN. 0 to 180 degrees corresponds to a waxing Moon, 180 to 360 degrees to a waning Moon.

After it there is also a % figure. The opposition ( = FULL MOON ) corresponds to the value 100%, the conjunction ( = NEW MOON ) to the value 0%. With this the Moon phases can be seen approximately, in a somewhat more vivid way.

If you want to determine the exact times of FULL MOON or NEW MOON, you can use the program "ECLIPSES..." under "MISCELLANEOUS" ( Explanation 9 ).

## ADDITIONAL-PLANET COORDINATES ( helio- or geocentric )

Further regarding ADDITIONAL-PLANET COORD.:

For the minor planets (asteroids) no analytical planetary theory exists. Here I have computed an ephemeris for CERES, PALLAS, JUNO and VESTA by numerical integration over the range AD 08.01.1500 to AD 18.11.2100.

The osculating initial values were taken from the "AHNERT" volume 97. The integration was carried out strictly as a 14-body problem. The SYMBOLS used are the same as in the ephemeris of the PAUL C.R. ARENDS VERLAG 8219-RIMSTING.

The accuracy corresponds to that of the major planets ME to NE. Spot checks with observational values yielded very good agreement.

For CHIRON, which seen over longer periods has a "chaotic" orbit, a numerically integrated ephemeris was likewise computed over the range 07.01.600 BC to AD 21.08.2199, with inclusion of the deflections by JU, SA, UR, NE, PL.

The masses of ME, VE, TE, MA were added to the Sun. This simplifies the integration without noticeably impairing the accuracy. As osculating orbital elements the values determined by MARSDEN ( as of 1979 ) were used. These are given about as accurately as those used for CE, PA, JN and VS.

The computed values can also be used for planetary returns.

The symbols of the minor planets disappear when the given time frame is exceeded, even when they are preselected.

The hypothetical TRANS-PLUTO ( = ISIS ), after the orbital elements of E.SEVIN, is likewise categorized under ADDITIONAL PLANETS. The longitude values correspond to the ephemeris of NEIL F.MICHELSEN.

In this ephemeris the astronomical premises are also described quite thoroughly: "HAWKINS ENTERPRISING PUBLIKATIONS DALLAS/TEXAS 1978". The ephemeris of TH.LANDSCHEIDT gives values that deviate somewhat from these. Apparently he used somewhat different "orbital elements". PUBLISHER: "F.BRANDAU PASSAU / 1984." About this very questionable "planet" the following is to be said:

- So far TP has only been computed but never observed.
- In the orbital elements, the orbital inclination and the node longitude are undetermined. For the orbital inclination, however, values between 0 and about 40 degrees are discussed among various astronomers, for the mean anomaly widely divergent values, up to mutual opposition. Right ascension, declination, latitude and node longitude therefore remain open in the tables.
- It is therefore not sensible to work to the minute with the available values, since the planet, if it really exists, could deviate by several degrees or even stand in approximate opposition to it. Best to keep your hands off it!!

For the "HAMBURG planets", which by astronomical judgment can hardly deserve this name, circular orbits are generally assumed, that is, of the 7 orbital elements needed in total only 2 are used, for example major orbital axis and mean anomaly ( = eccentric anomaly = true anomaly = mean longitude = true longitude ).

The "orbital elements" are now computed such that there is as much agreement as possible with the printed ephemeris of RUTH BRUMMUND ( publisher: WITTE-VERLAG Hamburg ). This ephemeris can probably count as the "official" one of the Hamburg School.

Precision, by the way, should not be expected here. If someone claims to obtain day-exact triggerings with these "TRANS-NEPTUNIANS", he only shows that he has not yet informed himself about the fundamentals, or is not clear about the role of chance.

The program ADDITIONAL-PLANET COORD. does not switch on this mode for the horoscope representations. This is only possible via CHANGE EPHEMERIS DEFAULTS...!

## HELIOCENTRIC VERSION ON/OFF

Switches between these versions.

You achieve the same from the main menu with the key combination ALT + "H".

## STATISTICS

For this extensive module see the separate "Explanation STATISTICS".

## DEGREE LIST

Lists the planetary positions, the houses (HS 2, 3, 5, 6.) and their DIRECT midpoints, together with a graphic that makes the occupation density of the degrees vivid. This program also runs heliocentrically. For the midpoints, always the value nearest to the two factors is chosen. The opposition point to it is of course also in the direct midpoint, but not listed.

## FIXED-STAR POSITIONS

Gives the coordinates of 62 objects of the fixed-star sky. The influences of proper motion and precession are taken into account for the individual stars; for the star clusters, which are marked by ***, the GALACTIC CENTER and the APEX, only the precession.

The column "D(LJ)" gives the approximate DISTANCE in light years. The max. longitude error corresponds, within our century, to one time second. It can increase per century by at most one time second (= 15 sec RA). Incidentally, the galactic center and the apex (= direction of motion of the solar system) are by far not as precisely definable as the impression is often given. I have used the most commonly cited values, but one does well to reckon with about 1 degree of uncertainty, especially for the apex! The entry, for example, 2deg/120 * for star clusters means 2 degrees extent/120 stars.

If one of the objects has an ASPECT with the planets of the current data record, the relevant row of the table is INVERTED and the relevant planet is entered in the column "ASPECTS". Counted are: conjunction with 2 degrees normal orb, opposition with 1 degree, trine with 2/3 and square with 0.5 degrees orb.

The orb can be influenced via "HOROSCOPE DEFAULTS..". See also Expl.4! The same applies to the following program "ARABIC PARTS"! In the column "QUALITY" the character of the object concerned is represented with 1 to 3 planet symbols, roughly as it can be found in R.EBERTIN "Die Bedeutung der Fixsterne".

The formulations, also for APEX and GALACTIC CENTER (introduced into astrology by TH.LANDSCHEIDT), are to be understood only as a non-binding working hypothesis!!

In general an "influence" of the fixed stars seems to me especially questionable, since with today's telescopes every degree of the sky appears covered with objects that would hardly be distinguishable. The old astrologers could not yet know this.

If these could nevertheless be statistically demonstrated, one would rather have to attribute "qualities" to particular directions in space. Incidentally, only ecliptic longitudes are ever considered anyway, even for objects that stand almost at the zenith.

## ARABIC PARTS

The program computes for the current data record a list of 37 "ARABIC PARTS" such as Part of Fortune etc. "Classical" are in part those that use planets up to SA. The others are "later creations". The respective formula is always given along with it. Some were named to me by acquaintances. I myself have hardly worked with them yet.

The main aspects with radix factors are displayed as in the preceding program. The values can also be SORTED by ecliptic longitude (SPACE key). Insofar as house cusps are used, H12 for example means the cusp of the 12th house. Hv9, by contrast, means lord of house 9.

About the value of these points (also called "sensitive points") let everyone form his own opinion.

In my opinion, when interpreting, one should consider the FORMULA in relation to the radix and let the name count only as an association aid, even if one may occasionally register a "bull's eye" with the classical names too. The FORMULA, by tradition, differs between day and night birth. From the present version on, besides this traditional version either only the "day version" or only the "night version" can be chosen. The current list is intended only as a "BASIS FOR DISCUSSION". In due course I will perhaps take into account in an update the suggestions that have reached me.

You can overwrite this list completely with YOUR OWN DEFINITIONS, which are stored in the files ARABTEI1 and ARABTEI2 in the folder INTERN.

BRUNO MAHL has recently published a book about his investigations into the A.P.: " Die verborgene Macht der arabischen Punkte im Horoskop" ISDN 3-200-00071-6. Source references can also be found there.

## INGRESSES SO - MO - MC - AC

With this program the points in time of the entry of Sun or Moon, MC or AC, into the 12 zodiac signs are computed.

In earlier versions one could compute these, though considerably more laboriously, only with the program part "CORRECTION".

One enters a starting date and then obtains the 12 points in time mentioned. For the Moon one enters roughly the date of the middle of the month of interest and obtains the ingresses of the relevant lunar orbit.

For the Sun one obtains the 12 points in time within the entered calendar year, for MC and AC during one day.

The computation is carried out very accurately, by means of iteration, which especially for the Moon gives more accurate values than interpolation. The computational effort, however, is very large, so that some computing time results.

Giving the time to the second is somewhat exaggerated. It was nevertheless kept, since the computational accuracy is considerably better than to the time minute. This also ensures that when the computed times are reinserted, the exact sign boundaries actually appear in the coordinate table.

With the "+" key, the space key or the Page ^ key one can step forward in time, with "-", "R" it goes backward in time.

For the ingresses of MC and AC the following is to be noted: the time determined to the second for these ingresses is, as a consequence of the fast motion of these objects, too inaccurate to hit the exact arc second on reinsertion. One time second already corresponds to 15 arc seconds. This is accordingly not a computational error, since it should be pointless in astrology to compute with fractions of seconds.

## ET from UT   and   UT from ET

Are auxiliary routines for astronomical considerations. Likewise the program

## "DATE from JD ".

Likewise an astronomical auxiliary routine for determining the calendar date from the "JULIAN DATE" used in astronomy, which counts day by day from the date 1.1.4713 BC, 12 h.

---

GENERAL notes on the CALENDAR:

The CALENDAR program (formulae for example from MEEUS) covers the Gregorian and Julian calendar. The switchover on 4.10.1582 is contained in the program.

The counting of the JD (= "Julian days") goes down to JD=0, which corresponds to 1.1.4713 BC, 12H in the Julian calendar.

DO NOT USE EARLIER DATES!!

Since in many countries the changeover took place much later, for the years 1583 to 1890 a query for choosing the calendar is still switched in. The chosen calendar is noted in the coordinate table and the horoscopes, when computation is still to be Julian, with the abbreviation JULIAN. For example, in Protestant Germany the changeover was made only on 17.2.1700. From 1890 upward the Gregorian calendar is always assumed. If, after all, dates in the Julian calendar should still be present, you must then express the Julian date yourself in Gregorian terms, that is, add 10 days to the Julian date. Only then does the program compute correctly.

For DATES BC you enter at "V.CH." a "V" or a "-". The minus is chosen additionally here only so that one can largely enter with the right hand -. Namely, at "JAHR" you enter the normal year number used by historians, without a sign! In the table OUTPUTS the entry V.CHR. is mostly replaced by the MINUS sign, whereby the arithmetic ( = astronomical ) counting is given! Here, for example, the year -62 corresponds to the year 63 v.Chr. This comes from the fact that the historians "forgot" the year 0. The year 0 corresponds to the year 1 v.Chr!! Incidentally, "v.Chr." is also abbreviated as "VC".

In the table header the "Julian Date" JD=.. and the time coordinate T measured in "Julian centuries" (= 36525 days) from the date 31.12.1899 12H UT are given. (Corresponds to JD=2415020.0). This is a time specification customary in astronomy for the reference point in time used here. With the small program " DATE from JD " one can, after entering the full Julian date, determine the date in the respective calendar. For example, for JD=1507900.13 ( UT ) the date is 28.5.-584 (= 585 v.Chr.) 15 h 7.2 min (UT).

An extensive documentation on calendar and time determinations was, thankfully, prepared for the computer by Mr. B.MAHL and can be read via READ TEXTS in the folder \HORCOM\ZEITBEST, or also directly from the INPUT BOX at NEW ENTRY..
