# Introductory Commentary

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

The software used to create the program is:

GFA-BSIC for WINDOWS 3.1 , PROFESSIONAL VERSION RELEASE 4.36 from GFA-SYSTEM - TECHNIK Mönchen-Gladbach.

In addition, API functions of WINDOWS 3.1 were also used directly.

This program contains software code ( planets SO,MO,ME,VE,MA,JU,SA,UR,NE ) Copyright (c) 1991-1992 by Jeffrey Sax and distributed by Willmann-Bell, Inc.  Series 10756

Acknowledgements:

I thank Mr. FRANK OSTROWSKI and the men of GFA-SYSTEMTECHNIK for GFA-BASIC, and for support with the PC version.

I thank Messrs. A.BUNKAHLE, B.MAHL, L.RATHKE, PH.SCHIFFMANN, K.STAMER, M.H. WESEMANN, DR.H.WISGRILL, I.HAHN-ROSTOCK, J.HUBER and H.PHILIPP for useful hints, for making literature available, as well as for help with testing and troubleshooting, mostly already with the ATARI version. Mr. B.MAHL gave important suggestions and support in creating the PRIMARY directions, the "CORRECTION with PRIMARY DIRECTED AXES" and the "ARABIC PARTS".

I thank Messrs. B.MAHL and DR.H.WISGRILL for their LOCATION FILES, into which much work has gone. They make them available to all HORCOM users.

I thank Mrs. M.-L. BORKERT for important suggestions on the present version for WINDOWS and for valuable help with troubleshooting. Since I myself work astrologically only occasionally ( for home use ), the program owes its maturity in good part to the collaboration of the astrologers named above and of several not named. Unfortunately I have so far been able to follow up on by far not all suggestions, since I run the program only as a retiree's hobby ( at most 4 hours daily ). But what is not yet can still partly come to be.

May it also be forgiven me that I can only follow up on suggestions that come from several quarters and that also seem justified to me. I also have to make sure that operation does not become too problematic through overloading with choices.

Some notes on the content of HORCOM:

Behind the menu titles of the main menu there are often whole groups of programs. For example under "STATISTIK" or "MÜNCHNER RHYTHMENLEHERE".

ERLÄUTERUNGEN (explanations) are contained in the program. The texts in the DIALOG BOXES are to be regarded as a supplement to them!

The explanations are called up either from the menu or from the result screen with the F1 key ( which calls up the respective relevant explanation ).

A great deal of effort went into the dialog with the user. Almost half of the program text is probably taken up by explanations and dialog texts.

If you want to PRINT explanations, click on "DRUCKEN". It is then printed with a preset font. The umlauts are rendered correctly only if the printer is set to the German character set.

If you want to design the printout yourself, you can also work with WRITE or WORDPAD or the WINDOWS EDITOR. You only need to open the text files in the folder \HORCOM\KOMMEN5\.....TXT.

The positions of the major planets ME,VE,EARTH ( SO ),MA,JU,SA,UR,NE are calculated according to the analytical formulas provided by JEAN MEEUS:

JEAN MEEUS ( "ASTRONOMISCHE ALGORITHMEN", JOHANN AMBROSIUS BARTH-VERLAG ISBN 3-335-00318-7 )

These routines are, for 8 millennia ( 2000.0 + - 4000 years ), as accurate as one could only wish for as an astrologer. They stem from an abridged theory VSOP87 of the INSTITUT DES LONGITUDES ( PARIS, P.BRETAGNON G.FRANCOU ). Thanks to the astronomers!

On the basis of this data, the INSTANTANEOUS VALUES ( also called "TRUE VALUES" ) of the LUNAR NODE and LUNAR APOGEE ( = BLACK MOON ) are also calculated within HORCOM.

SOLARE, SEPTARE, LUNARE, PLANETARE and PERSONARE ( P.ORBAN,I.ZINNEL ) are included, as are some of the most commonly used DIRECTIONS and of course TRANSITS, which can be displayed in various presentation modes.

The "Multiple Directions" of ST.LEHRIEDER are likewise included. ( see Explanation 4 ).

A large space is taken up by the "MÜNCHNER RHYTHMENLEHRE" of my astrological teacher W.DÖBEREINER, which I hold in high esteem. In the following also abbreviated as M.R.! To him I owe my access to astrology. ( see Explanation 6 )

"MUNDAN" horoscopes ( = topocentric-equatorial horoscopes in a horizon system ) can also be used for the M.R. The coordinate system used here, with time measurement=space measurement on the equator, is better suited to the sense of the M.R. than working with the geocentric, ecliptic horoscope. E.R.DOSTAL first drew attention to this ( see Explanation 4 ).

In addition, a further evaluation in the form of a DEGREE-DATE list was added for the M.R.

Besides this, MIDPOINTS are used, roughly in the manner of the school of R.EBERTIN or the "HAMBURGER SCHULE" ( also as a separate graphic ). The programs SECONDARY and SOLAR ARC DIRECTION also came about through suggestions from books by R.EBERTIN.

Besides the normal secondary directions, "Dynamograms" according to KRAFFT-GOERNER can also be created. This variant was recommended to me by Mr. JOHANN HUBER ( see Explanation 7 ).

The PRIMARY DIRECTIONS are now programmed according to the book by E.C.KÜHR "BERECHNUNG DER EREIGNISZEITEN". All planets, intermediate houses and cardinal points can be used as significators and promissors.

The promissors can also be calculated WITH LATITUDE ( see Explanation 7 ).

On the question of ORBIS I took inspiration from J.ADDEY and H.J.WALTER, for the DIRECTIONS besides E.C.KüHR from R.EBERTIN and C.O.E.CARTER.

The MINOR PLANETS Chiron, Ceres, Pallas, Juno, Vesta are included. ( see also Explanation 3 ).

The 8 hypothetical "planets" of the HAMBURGER SCHULE can likewise be chosen ( see also Explanation 3 ).

The hypothetical TRANS-PLUTO ( also called Isis ), according to the provisional orbital elements calculated by EMILE SEVIN and also given by N.F.MICHELSEN in his ephemeris, is likewise included ( see also Explanation 3 ).

Besides the usual MEAN LUNAR NODE, the TRUE (= INSTANTANEOUS) value is also calculated optionally ( see also Explanation 3 ).

The MEAN or INSTANTANEOUS (=TRUE) APSIDAL LINE of the Moon's orbit ( = 'BLACK MOON' ) can also be added under 'KLEINPLANETEN'. This 'Black Moon' has only recently been receiving some attention in Germany, probably because so far there is no ephemeris for the TRUE value, which can deviate from the mean by more than 20 degrees. Until now, work has apparently been done almost exclusively with the mean value.

The interpretation known to me so far as a SEXUALITY AXIS (APOGEE = BLACK MOON = FEMININE and PERIGEE = "PRIAP" = MASCULINE I have so far found confirmed insofar as this axis, when it stands in close aspect with radix factors such as MC,AC,SO,MO, birth ruler, seems to be effective as an ENERGY FACTOR. For example, Goethe's horoscope thereby gains greatly in persuasiveness, since the Black Moon stands in closest conjunction with MC and Sun.

Such constellations seem to be almost a hallmark of significant people. A radix factor that stands in conjunction or opposition with the Black Moon thereby gains in penetrating power, or becomes dominant.

For the gloomy interpretations of the SCH.M. I have so far been unable to find any evidence.

By the way, the values are usable as far as the Moon longitudes are, i.e. over several millennia.

For more on this, see Expl.3. Literature: L.MILLAT, R.DAUTREMONT, M.DUVAL 'LUNE NOIRE', EDITIONS TRADITIONELLES QUAI SAINT MICHEL - PARIS / 1983 ( see also Explanation 3 ).

FIXED-STAR POSITIONS as well as ARABIC PARTS can be called up as tables with ASPECT GRAPHIC. For the latter, all points can also be self-defined and saved to diskette ( see also Explanation 3 ).

HORCOM5P/7P further has the module "STATISTIK", which appears quite inconspicuous in the main menu but has a great deal to offer. With it one can investigate quite detailed questions statistically. This is a program package that is especially important for the researching astrologer.

In addition, HORCOM5P/7P has the following programs:

- AUFGANG ( rising, meridian transit and setting of the planets ).
- FINSTERNISSE ( solar and lunar eclipses, possibly with aspects to the current data set, as well as NEW MOON and FULL MOON times ).
- INGRESSE ( calculates the ingresses of Sun or Moon into the 12 zodiac signs ).

Only a few "PROHIBITIONS" were built in. Which application makes sense you should decide. No limits should be set to the desire to experiment.

You can, by the way, bypass the PROHIBITIONS by, for example, entering a SOLAR etc. as RADIX and thus, for example, making a SOLAR of the LUNAR, if it really must be. "ERGEBNIS ALS RADIX" serves this purpose. Use it only with caution.

The ample use of "DIALOG BOXES" and sub-menus ensures to a considerable degree a user guidance, without unduly restricting variability.

To have an inkling of the possibilities of HORCOM, you must have already spent many hours working with it.

Changes to the content are reserved.

No guarantee is assumed for freedom from errors. If you notice errors or operating deficiencies, I ask for an occasional notification. Errors will be eliminated in the next due UPDATE.

Constructive criticism too is welcomed, albeit through gritted teeth. Suggestions for improvement are taken into account only when they are found desirable by several users and also fit into my concept.

DEGREE FIGURES such as 123.45 are always to be understood as DECIMAL degrees. Occasionally, for reasons of space, the minutes or seconds are omitted. Thus 12 degrees 33 means: 12 degrees 33 min.

For zodiac signs and planets, either the symbols or the common abbreviations are used.

ABBREVIATIONS are:

ZODIAC:

```text
AR    TA    GM    CN    LE    VI    LI    SC    SG    CP    AQ    PS
```

PLANETS:

```text
SO  ( TE )  MO    ME    VE    MA    JU    SA    UR    NE    PL    DR
DS
```

TE = Earth | MO = Moon | DR = Ascending, DS = Descending lunar node.

12 VI 23 means 12 degrees 23 min Virgo. SO=MA-SA means: Sun in the midpoint of Mars and Saturn.

After entering the data, in general no further preparation is needed in order to click under EPHEMERIDE, HOROSKOPE, AUSWERTUNG, DIVERSES, except that for DOUBLE horoscopes 2 data sets should be present ( see also Explanation 4 ).

If a G/H stands after the program name, then this program is applicable both GEOCENTRICALLY and HELIOCENTRICALLY. Since the heliocentric version is "exotic" in astrology, a constant query was dispensed with.

The heliocentric version can be switched on or off under EPHEMERIDE, or also with the function key F6.

It is definitely of interest to astrologers too!

See on this the statistical research of Dr.SIEGFRIED SCHIEMENZ, e.g.: "Planetenstellungen und der Geist des Menschen".

The HOUSE SYSTEM is chosen under "DIVERSES".

The beginner with HORCOM should above all note the following:

HORCOM uses DATA SETS on various "LEVELS".

In any case one begins on the RADIX LEVEL, i.e. every newly entered data set, or one fetched from a DATA or STATISTIK file, is at first always valid as RADIX.

Once one has made a SOLAR, LUNAR or PLANETAR, one is on the respective SOLAR, LUNAR etc. LEVEL and not all HORCOM programs are accessible anymore ( but certainly still some ). If one now wants to do something new with the RADIX data set in question, one must first click on it again under "EIN-AUSGABE". The last SOLAR etc. data set is nevertheless retained and can, as long as the associated RADIX data set is present, be reactivated at any time, whereby one is again on the SOLAR etc. LEVEL.

The corresponding applies to the DOUBLE HOROSCOPES (COMPOSIT, COMBIN etc.). They likewise form a special level.

In table outputs, the "problematic" planets MA,SA,UR,NE,PL can be displayed inverted, which also serves to graphically loosen up the tables.

This can be switched on or off under VORGABEN DIREKTIONEN ÄNDERN.

Reading the explanations often in the first days, and also later again and again, is strongly recommended!!!

I wish the users of this program much joy with it and hope that it contributes to the clarification of open questions.
