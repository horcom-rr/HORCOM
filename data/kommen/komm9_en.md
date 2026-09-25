# Explanation Miscellaneous

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

## KORREKTUR

Serves above all for correcting the birth time. The normal input routine is used, whereby at least the place coordinates and the calendar date must be entered. One can then correct either with the SIDEREAL TIME, the MC, AC, with INTERMEDIATE HOUSES, with SUN or MOON.

The result OVERWRITES the original input !! and can be used further immediately. With "KORREKTUR" one can also, for a given horoscope with unknown birth time and known AC or MC, determine the UT as follows: with "NEU-EINGABE" enter geographic coordinates, date and any UT, and modify with AC or MC. One needs only either AC or MC.

For the practitioner this is one of the most important programs.

The KORREKTUR with "PRIMÄR DIRIGIERTEN ACHSEN" is an AUXILIARY ROUTINE for a proven correction method. It works according to the "equation":

> A DAILY SIDEREAL-TIME PROGRESS = 1 YEAR OF LIFE

The following relationships hold:

```text
    1 YEAR  = 0.98565 DEGREES        ARMC   = 0 h  3 m  56.56 s  SIDEREAL TIME
    1 MONTH = 4.928   ARCMIN          "     = 0 h  0 m  19.71 s        "
    1 DAY   = 9.7     ARCSEC          "     = 0 h  0 m   0.65 s        "
```

1 DEGREE of sidereal time ( = ARMC ) corresponds to 4 time-minutes of correction at the RADIX.

The RADIX PLANETS normally remain UNCHANGED. But one can also experiment with the EVENT planets.

Otherwise only the HOUSES are turned = "DIRIGIERT" according to the progression forward = "DIREKT" or backward = "KONVERS".

The sidereal time shown at the top right refers only to the house system ! One thus enters distinctive EVENT DATES and observes how, in the "progressive horoscope" concerned, the main axes form aspects with planets. In my opinion both directions are to be considered, as is customary with the "classical" primary directions too. Apparently the "direction" is fictitious and only the distance is relevant.

As REFERENCE PLACE the place of birth comes primarily into consideration, according to the principle that directions should always refer to the RADIX ( see e.g. in KÜHR ). But that should not be a dogma.

As an experiment one can further alter the turned axis cross by smaller variations and look at how the picture changes. The values of these trial shifts can optionally be summed up in a sum store.

The shifts individually or the mean value of their sum can be taken over into the radix horoscope. For this please observe the dialog texts exactly. One shifts the RADIX SIDEREAL TIME so that the main axes form distinctive angles to important RADIX or (and) EVENT factors for the events considered.

See on this e.g. the book by SCHUBERT-WELLER: "DIE ASTROLOGISCHE GEBURTSZEIT-KORREKTUR".

With variations the change of sidereal time is entered in degrees. The following procedure should be expedient:

First one should gain an overview for a few distinctive events without making any attempts with variation.

If one then makes attempts with shifts, one can either take over each shift individually into the radix or one sums up various variations and corrects the radix horoscope with the mean sum of these variations.

This procedure demands much experience. Here one must already know what one is doing, and must make not only quantitative but also substantive considerations. This is thus something for old hands and hardly something for beginners. One shifts the RADIX SIDEREAL TIME so that the main axes form distinctive angles to important RADIX or (and) EVENT factors for the events considered.

## ZEIT-WANDERN

Is likewise suitable for correction considerations. With it one can modify a given RADIX with VARIOUS TIME UNITS. The full evaluation is omitted here (only main aspects !).

The program runs automatically. Procedure and control as with TRANSIT-GRAPHIK. At the end the full evaluation can be appended. One must practice a little first to recognize the value of this program !

The result optionally OVERWRITES the original input immediately. Instead of this program one can also work analogously with "PLANETEN-KOORDINATEN". Optionally an ASPECTS counter and, if the MIDPOINTS list is active for horoscopes ( to be selected under "VORGABEN HOROSKOP ÄNDERN" ), a MIDPOINTS counter as well can be activated.

The ASPECTS counter was created for and in cooperation with Dr. SIEGFRIED SCHIEMENZ for his statistical investigations on HELIOCENTRIC horoscopes.

It also counts "GREAT ( = closed ) TRIGONS" and triple conjunctions, which SCHIEMENZ calls "TRIGA" or "TROIKA".

---

**Procedure during "Zeitwandern": within 1 second after the signal tone, redirectable via KEYBOARD or MOUSE. Stop / Go : SPACEBAR**

Upper- or lowercase !

Time measure LINEAR :

Transits, Multiple Directions :

```text
  D  =  1 Day
  H  =  1 Hour
  M  =  1 Minute
```

Time measure Progressive ( 1 DAY <> 1 YEAR ):

Secondary and Solar-arc Directions :

```text
  J  =  1 Year   ( 1 Day )           "
  M  =  1 Month  ( 2 Hours )         "
  D  =  1 Day    ( 3 min. 56.6 SEC.)"
```

Direction :

```text
  V  =  Forward
  R  =  Backward
```

Interval:

```text
   +  =  Doubling
   -  =  Halving
```

Waiting time :

```text
  Left  mouse button = + 2 seconds
  Right mouse button = - 1 second
```

CONTINUE each time with SPACEBAR

---

## UHR

Outputs a horoscope of the current time of day. The time is taken from the system clock. The ZZD ( summer time etc. ) is queried.

When activating the menu screen, the current UT, AC, MC and SIDEREAL TIME are then shown in the bottom line of the control field.

If the UHR horoscope is left standing, it is redrawn every 5 seconds.

The running clock can also be taken over as a data record in order to be able to carry out further evaluations. This data record is updated every 15 seconds, in the menu however only when the latter is actually used. The present menu item "UHR" is no longer accessible as long as this is treated as a data record.

Results such as SOLARE etc. are deleted after 30 sec., since the clock keeps running.

## AR-DE aus EL-EB  und  EL-EB aus AR-DE

Serve for converting ecliptic coordinates into equatorial coordinates and vice versa.

"LT aus UT" determines local time from UT; "UT aus LT" has the reverse function.

The abbreviations are:

AR = Right ascension, DE = Declination, EL = Ecliptic longitude, EB = Ecliptic latitude, MC = Ecliptic longitude of the MC, AC = Ecliptic longitude of the AC, UT = Universal Time = GREENWICH time.

LT = Local Time = local time ( not zone time ! ).

The angles are referred to the TRUE equinox ( with nutation ). The time must be entered, at least approximately, in order to take the effects of precession and nutation into account.

## AUFGANG..MERIDIAN-DURCHGANG..UNTERGANG

Allows the calculation of the RISINGS, MERIDIAN TRANSITS and SETTINGS of the planets for a given date and place.

Optionally a list can be created for a particular planet with running date, or for all planets with fixed date.

It only makes sense to choose REAL astronomical objects, so under no circumstances hypothetical planets.

A distinction can be made between APPARENT and TRUE coordinates. In astronomy the APPARENT coordinates are taken as the basis exclusively, which also contain corrections for the refraction of light in the atmosphere, which depend among other things on air pressure. Astrologically, on the other hand, precisely the TRUE values might be more meaningful.

The time differences between the two calculation methods can amount to several time-minutes.

With the APPARENT values the times for the Sun are always referred to the upper edge of the solar disk; with the choice of the TRUE values always to the center of the respective object.

In the output table, for the preselected object the 3 points in time are calculated, as well as the associated longitude, latitude, AR, declination and sidereal time. The accuracy for TRUE values is to be estimated at a few time-seconds for SO, ME, VE, MA, and at < 1 time-minute for the outer planets. With the choice of the APPARENT values only minute-accurate values are given because of the imponderables of the corrections for light refraction. Especially with the Moon, with its large interpolation interval, it can now and then happen that a field remains empty for the date in question. Then you must look at the preceding date or at the following one.

Please note !! :

At the ascendant one can expect an object at "RISING" only if it currently has ecliptic latitude 0.

That is always the case only for the Sun.

The same applies to the meridian transit and setting.

## FINSTERNISSE...

With this the points in time of SONNEN- and MONDFINSTERNISSEN can be calculated.

In addition, the points in time of NEW MOON and FULL MOON are likewise listed, at which no Finsternisse occur.

The calculation is geocentric, i.e. the PARALLAXE is left out of consideration. One enters a date near the period of interest. Then a list of the NEW MOON ( LEFT ) and FULL MOON points in time ( RIGHT ) is output for 1 to 2 lunar cycles before this point and about 30 lunar cycles after it. The Sonnenfinsternisse ( LEFT ) and the Mondfinsternisse ( RIGHT ) are made recognizable by inversion.

In the output list, after date and time, LEFT the corresponding SUN position and RIGHT the MOON position are entered.

The points in time of the new moon or full moon are generally not identical with the Sonnen- or Mondfinsternisse that may also be present. Therefore both are listed separately.

The points in time of the Finsternisse each concern the maximum phase and can have a maximum error of < 1 time-minute.

Mondfinsternisse can generally be observed only in the UMBRA of the Earth, but from any point on Earth.

Sonnenfinsternisse can each be optimally observed only in limited regions of the Earth's surface. To show the details is relatively complicated. It is merely indicated with the letters "N" or "S" whether the Finsternis is visible in the NORTHERN or SOUTHERN hemisphere. In the Sonnenfinsternisse marked with "EX"=EXTERNAL, the umbral cone of the Moon does not strike the Earth's surface. These Finsternisse are as a rule "PARTIAL", in rarer cases however also "ANNULAR"="RF" or even "TOTAL"="TOT". The Sonnenfinsternisse marked with "ZT" = CENTRAL are ( at the place of their maximum occurrence ) TOTAL or ANNULAR. Regarding the astrological relevance of Finsternisse, one can suppose that their "effect" should be more marked than a mere new moon or full moon, since in the case of the Finsternis both ecliptic longitude and latitude almost coincide, or the Moon stands near its line of nodes.

The actual observability, in the case of the Sonnenfinsternis, might by contrast be less significant. Of most Finsternisse as a phenomenon one hardly takes notice today anyway, unless the observation conditions are unusually favorable.

Optionally the ASPECTS between SO or MO with the respectively valid data record ( Radix, Solar etc. ) can also be viewed. They are entered in each case below the line NEW MOON ( SONNEN-FINSTERNIS ) or FULL MOON ( MOND-FINSTERNIS ). The lines in question are marked with an arrow pointing to the right.

With the choice of higher divisors ( smaller aspects ) or non-integer angles, the respective divisors or multiples are also entered after the aspect.

One should have familiarized oneself with reading the tables without aspects before adding these, since otherwise confusion could arise.

## DATEIEN VERKETTEN

With this program you can chain together several files of different names.

The resulting file is initially called "\AA_MUDAT.DAT" in the folder \HORCOM\SPEZIAL\ for DATEN files. You can also rename this chained file at any time and delete it again. The whole thing requires some practice.

In the folder "\SPEZ_ORT" files for GERMANY, AUSTRIA and SWITZERLAND are already present. They are kindly made available free of charge to all HORCOM users by Mr. BRUNO MAHL.

Two files "\WELT.INT" and "\EUROPA.INT" are likewise already present. They come from files by Dr. H. WISGRILL that were pressed into the HORCOM format. Therefore only the first 16 letters of the place designation are visible. The last 4 characters were used for the ZZD = zone time difference ( relative to UT=GMT ). Dr. WISGRILL likewise kindly makes these data available as PD. The ZZD in the name is expedient in order to be able to infer the UT immediately. In many cases there are nevertheless uncertainties, due to summer times etc.

An extensive DOCUMENTATION of the TIME DETERMINATIONS for 28 European countries in past and present, in the extent of about 40 densely written typewriter pages, was created by Mr. BRUNO MAHL. It is also printable, saves the acquisition of a whole series of books, is directly callable while working with the PC ( of course also for other astrology programs ), and is also used in the EINGABE-BOX during first entry. It contains in particular the calendar changeover dates, summer times, special times, changeover date from local to zone time etc. Precisely for historically working astrologers this is very useful.

## AAF-DATEI <> HORCOM-DATEI

Converts an AAF file ( = Astrological Exchange Format ) into a HORCOM-readable DATEN file or vice versa. Only those data are taken from the ( often more extensive ) AAF file which fit into the fixed HORCOM format.

ZONE, LOCAL times etc. are converted into UT. Of any comment, only the first sentence is shown.

Regarding the NAME, note that the name part before the first blank in the HORCOM format is interpreted as SURNAME in the AAF format ! The AAF files converted into HORCOM DATEN files are created with the extension .DAT in the folder \HORCOM\SPEZIAL, alongside the original HORCOM DATEN files.

Attention ! The AAF files are always created or presupposed in a folder .....\AAFDATEN. The hierarchy of this folder is fixed at the first program start of HORCOM5P/7P. The formerly prescribed folder ..\ASTRODAT.AAF is dropped.

If AAF files are still located there, these should be moved to the folder ..\AAFDATEN.

With these programs it should also be possible to convert files created with other programs into HORCOM-readable files via the detour of AAF, if these support the AAF format, as does e.g. HERMES.

## WINKEL-ZEIT UMRECHNUNG

With this, conversions of decimal angles or h into ° ' " or h mi sec can be carried out.

This can also be called from any results screen with the F5 key !

## HITERGRUND-FARBEN

With this the color background of the dialogs and their background can be set.

## ORT-WANDERN

Works analogously to ZEIT-WANDERN, only that here, for a given time, the place, i.e. geographic LONGITUDE and LATITUDE, can be changed.

This can be understood as a step in the direction of ASTRO-GEOGRAPHY.

Of course you have always been able, in the input box, in any data record, to overwrite the place of birth with places from the place files as well, if you want to see how a change of place would have taken effect.

People who would like to look for a "good" Solar for their birthday can also make use of this.

If you press the function key F10 during the run, the coordinates of a nearby metropolis appear at the bottom left, provided the file WCAPITAL.INT is present in the folder \HORCOM\SPEZ_ORT.

If you then press "ENTER", the coordinates of this city are taken over for the further run.

## GROSSES JAHR

This program is intended for the researchers of the "Platonisches Jahr". The vernal point wanders, as a result of "PRECESSION", retrograde through the zodiac in about 25776 years.

Some astrologers now assume that the transit time through a sign corresponds to an "AGE". Most assume that we currently live in the "AGE OF AQUARIUS".

The starting point of this count is highly questionable, since the "valid" zodiac by definition always counts from the vernal point. But whoever wants to research these "ages" can work with this program. One enters the presumed beginning date of this age and then obtains, for the current data record, the "AGE POINT", which shifts retrograde by 360 degrees in about 25776 years due to PRECESSION. The calculation proceeds astronomically according to the strict formulas. As the beginning date for the "AGE OF AQUARIUS" one could choose e.g. the discovery of Uranus: 13.3.1781 or the storming of the Bastille 14.7.1789 etc. The preceding period of the "AGE OF PISCES" would then be the approximately 2148 years before that, etc.

The program provides the possibility from ARIES to CAPRICORN age, corresponding to the validity range of the ephemeris, with otherwise free input possibility.

## ERGEBNIS ALS RADIX

Converts a SOLAR, SEPTAR, LUNAR, TAG-HOR or COMBIN into a RADIX, so that you can work with ALL programs, e.g. make a LUNAR of a SOLAR etc. Please be careful !!

## ÄNDERUNGEN / HINWEISE / KURZANL.

Here one can read up on the changes and improvements of recent years. HINWEISE are remarks on working with HORCOM under WINDOWS. KURZANL. is a short introduction to operating HORCOM. The two latter topics can also be reached after pressing the right mouse button or F1 from the HORCOM main menu.
