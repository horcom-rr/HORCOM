# Notes on Solar, Septar and more

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

SOLAR, SEPTAR, LUNAR, PLANETAR, PERSONAR, DAILY HOROSCOPE, PROGRESSED HOROSCOPE

## SOLAR, SEPTAR

Determines the moment for a SOLAR horoscope, its special form the SEPTAR, or a LUNAR.

After you have selected the SOLAR.. program and entered the desired year or date for the Solar.., you can select the remaining menu items, as far as they make sense, because the new moment now applies. The relevant output is marked by the heading SOLAR.... Any location coordinates that differ from the birthplace are prompted for. The NUMBER shown before "SOLAR" is to be understood as follows: NO. 0 corresponds to the RADIX, NO. 1 corresponds to the 1st birthday and so on.

Negative numbers correspond to past anniversaries.

CAUTION!

This numbering, which was also introduced for LUNARS and PLANETARS on 26.09.02, differs from the numbering for SEPTARS (see the following topic). There the 1st SEPTAR equals the RADIX! This is inherent in the nature of the method.

Besides the normal Solar you can also create "SEPTARS", as they are used in the MÜNCHNER RHYTHMENLEHRE of W. DÖBEREINER. With the programs you can handle ALL SEPTAR VERSIONS. Besides the SEPTAR NO., the PERIOD/HOUSE and the DIRECTION (RIGHT = CLOCKWISE) can also be freely chosen.

For example, in the 7-part rhythm the Septar No. 3 applies to the years of life 14 to 21, if from age 14 the time unit month is used. (With the time unit "year" this corresponds, in the 7-part rhythm, to the span 14 to 98 years.)

You can also, for example, run through SOLARS in a one-month rhythm. It may also be interesting to run through a LUNAR in a 1/12-month rhythm: enter time unit MONTH, PERIOD 0.08333 (under OTHER) and DATE output.

SEPTARS are, by the textbook, created for the birthplace. But you can also experiment with other places.

After you have calculated a SEPTAR moment, you can study the triggerings with the MÜNCHNER RHYTHMENLEHRE program.

## LUNAR

Determines correspondingly the moment for a Lunar horoscope (the Moon has the same ecliptic longitude as at birth. See also Note 3 PARALLAXE). For this you are prompted to enter a date for the period of interest. Then usually the Lunar immediately preceding it, within the next roughly 28 days, is determined. If not, choose a date pushed forward accordingly.

Besides this, the entry of a NUMBER in the future or the past is possible. This is intended to support the TERTIARY directions after TROINSKY (suggestion by Mr. PH. SCHIFFMANN). See the above remark under SOLAR for this.

Lunars and Solars are NOT calculated here by interpolation within a day, but CONSIDERABLY MORE PRECISELY. This is particularly significant with LUNARS.

So if you check with (linear) interpolation, you will get somewhat different results, which however is not to be blamed on this program, but on the inaccuracy of linear interpolation.

The exact iteration does, however, take a few seconds of computing time. The absolute accuracy of the determined times is better than 0.4 min. See also Note 3.

## PLANETARS

Planetars can now (from HORCOM3C) also be calculated. The calculation is geocentric, which, because of possible retrograde motions, is considerably more elaborate than for SOLARS and LUNARS. Depending on the processor and clock frequency of the PC, the computing time is typically about 1 second to half a minute, or even somewhat more.

Thanks to the very accurate ephemeris formulas of BRETAGNON now available, the time resolution is far better than 1 time minute, at least at some distance from turning points. If you hit turning points while searching, the calculation routine fails, which however will rarely be the case. If the computing time exceeds 1 minute, the attempt is to be regarded as failed. Then you can exit with the ESC key (stay on the key for a longer time until "ABBRUCH" appears) and try again with a new search date.

The interpolation error corresponds to at most a few time seconds. In any case, if you compute back to the radix as a PLANETAR, the correct minute always results, and often the second as well.

When entering the search date, please bear in mind that the search is made in the past. The respective orbital period in years or days is noted.

Besides a search date, a No. of the planetar in the future or the past can also be specified.

The NO. refers to the RADIX. NO. 0 is identical to the RADIX.

With ambiguous PLANETARS, the 3 or more moments are each assigned to the same No. With NEPTUN, not infrequently, up to 5 moments can also be found. With very eccentric orbits, such as with the asteroids DAMOKLES, NESSUS or COMET HALLEY, there can be up to 9 moments, at which point the process is aborted here.

The same applies to PLUTO, QUAOAR and XENA.

You have to practice a little. Normally the first direct-motion moment below the entered search date is shown.

Afterwards you can then search for ambiguous, normally 2 additional moments, one retrograde and one again direct-motion, and so on.

This is skipped if it is clear from the speed (daily motion) of the point found first that no ambiguity is to be expected.

Afterwards the next direct-motion moment in the past can then be sought, and so on.

No astrological experience exists regarding PLANETARS, since the available ephemerides have so far been too patchy or too inaccurate. A new field for researchers. One should assume that PLANETARS say something about the following orbit in each case. The possible ambiguity is problematic. For human horoscopes the planets ME, VE, MA, JU, SA and UR come into consideration. For historical research possibly also NE and PL.

Mr. PHILIP SCHIFFMANN contributed decisively to the creation of the "PLANETARS".

## PERSONARS

Personars were discovered as an astrological method by PETER ORBAN and INGRID ZINNEL.

They are created for the exact transits of the Sun over the radix planets, within the FIRST year of life.

So you can make Personars for all preselected "real" planets and the Moon.

To my knowledge, Personars are interpreted as partial aspects of the personality, according to the characteristic of the chosen planet. In HORCOM the names are abbreviated as follows:

For example for Mars "MARS-PERS" and so on.

## DAILY HOROSCOPE, abbreviated TAG-HOR

Calculates, for any date and place, a horoscope with the TRUE local solar time of the radix (the distance AR MC minus AR SO is the same as in the radix).

CAUTION!: With calculation INCLUDING parallax, deviations can occur, in particular when the daily horoscope is calculated for a place that differs from the birthplace!

## PROGRESSED HOROSCOPE, abbrev. PROG-HOR

Calculates directly a progressed horoscope for a specific event date. The equation for this:

"1 DAY = 1 YEAR"

As the conversion constant the tropical year of 365.242199 days is used.

For the time of day, or the house system, the following 4 versions can be chosen:

1. UT = RADIX-UT.
2. True solar time as with the radix (distance AR Sun minus ARMC constant). The TRUE SOLAR TIME is determined here, as also with the DAILY HOROSCOPE, quite precisely by iteration, which costs some computing time. The calculation corresponds to the one for the DAILY HOROSCOPE.
3. House system of the progressed horoscope rotated by the SIDEREAL-TIME ADVANCE relative to the radix. The MC moves, for example at an age of 50 years, about 50 DEGREES further compared to the RADIX. Accordingly the time of day is calculated precisely from the corresponding new sidereal time. The houses here thus move about a factor of 365 more slowly than in the following calculation type.
4. The PLANETS and HOUSES are calculated, including the time of day, according to the time computed strictly proportionally from the number of tropical years elapsed: 1 year corresponds exactly to 1 Julian day down to small fractions of minutes.

Astronomically correct and unambiguous is only version 4 to calculate. With versions 1), 2) and 3) the date has now (from 26.11.98) been placed such that a difference of at most half a day from the correct value according to 4) can exist. Which of the offered possibilities makes the most sense I do not dare to decide. I lack the experience here. According to v. H. KLÖCKLER, version 1) would be the usual one. He does, however, generally not think much of it. Normally one makes a progressed horoscope only for round birthdays and for the birthplace. The progressed horoscope is then meant to say something about the coming year of life.
