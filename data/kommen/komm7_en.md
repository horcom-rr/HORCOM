# Explanation of Directions

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

General preliminary remarks:

In what follows, "TRIGGERING" means an aspect between the moving factor and one Radix factor in each case. This aspect is always a multiple of a specified BASE ANGLE.

For all directions the calculation is done automatically with MAXIMUM accuracy. The programs SEKUNDÄR-DIR., TRANSITE and MUNDAN-ASPEKTE need relatively much computing time. So do not be surprised if the filling of the table form takes a while. You can also leave these programs early with ESC (possibly press several times!).

The capabilities of the programs are now so varied that you will have to spend several hours to test out the methods and settings that are important and productive for you. Thus you should note that when the Part of Fortune, or the Moon, the true Moon's node or the true Black Moon is preselected, the evaluations are generally particularly time consuming, because of the fast movement of these factors, or the slow advance respectively!!

The time scaling of the LINEAR GRAPHIC should be chosen optimally. That takes practice (see below), which I cannot spare you. As a BASE ASPECT you can choose any angle that results from an integer division of 360 degrees, as long as the value of 15 degrees (divisor 24) is not gone below.

360 DEGREES corresponds to the conjunction as the sole aspect. Otherwise the conjunction is contained in all aspect series.

From all TABLE outputs you can, by pressing the FUNCTION KEY F2, view the respective horoscope in between and go back again with 'Esc'.

In the tables the following mean:

- LJ = Year of life
- MO = Month with decimals
- PRO = Progressive planet
- STA = Fixed planet
  (If no direction is taken into account, simply PL1 and PL2)
- TB = Daily movement or speed of the moving planet in units of "/day (year). A negative sign means retrograde motion. The smaller TB, the longer the duration of the effect.

ASP = Aspect between progressive and Radix planet. The intermediate houses 2, 3, 5, 6 are simply given as digits.

For SEKUNDÄR-DIR., SONNENBOGEN-DIR. and TRANSIT the interpolation is done over 3 supporting values, so not linearly (curve of 2nd order). This mostly increases the computational accuracy, particularly near turning points, but requires that a certain time passes before values appear in the table! The stated TB values (= daily movement) are in each case MEAN VALUES of the respective interpolation interval, so near turning points only very rough approximations. The TB values in the table "PLANETEN-KOORDINATEN" are by contrast more accurate INSTANTANEOUS values.

## VORGABEN DIREKTIONEN ÄNDERN

With this, among other things, the INTERMEDIATE HOUSES can also be added, and likewise, in part, the points 0 AR, 0 CN, 0 LI and 0 CP = CARDINAL POINTS can be taken into account, which admittedly have no individual, but possibly "mundane" meaning could have.

## SEKUNDÄR-DIREKTION

The SEKUNDÄR-DIREKTION, also called progression, is based on the equation:

- 1 day after birth = 1 year of life (the measure is the ecliptic longitude).

The program determines the triggerings by secondary directions, as they are used for example in the R. EBERTIN school as LIFE DIAGRAMS "LDP". The output can be chosen as a TABLE, as HOROSCOPE GRAPHIC (on this see also the remarks below, under "TRANSITE") or LINEAR GRAPHIC.

With the LINEAR GRAPHIC a period of 5, 10, 20, 40, 80 or 160 years can be preselected, by choosing the START and END year in steps of 5!! This also applies to the SONNEN and MOND BOGEN direction.

The time measure is MEAN SOLAR TIME (= UT).

ATTENTION! Regarding AC and MC the following is to be noted:

The house system runs around once by about 360 degrees in one day (or progressed "year"). This is only visible in a true way in the HOROSCOPE display if you choose "STRENG PROPORTIONALE..." in the selection box. The measures "JAHR", "MONAT", "TAG" are in this case not to be understood as "real" in calendar terms, but symbolically. The length of the tropical year of tja = 365.24219878 days serves as the conversion factor.

In the TABLE display and the LINEAR GRAPHIC, progress is made in whole days and the MC apparently advances uniformly only at the speed of 0.98565 degrees per day ("year"). In between it is interpolated. If in the HOROSCOPE GRAPHIC you choose the variant HÄUSER-DREHUNG according to 1 DAY = 1 YEAR, then the relationships are correspondingly. However, in this case the houses are calculated specifically exactly according to the sidereal time progress. What now makes more astrological sense may be left open. R. EBERTIN handles the matter in his LIFE DIAGRAMS the way this happens in the LINEAR GRAPHIC or the TABLE display.

The consideration of "TRUE SOLAR TIME", which is preferred by some astrologers, was dispensed with, since this complicated the program control and made the operation too confusing.

According to my experience so far, the SEKUNDÄR directions are also not so accurately due that the consideration of this difference would carry weight. The smaller the base aspect (>= 15 degrees), the higher the number of triggerings. The MOON can be added as a moving "planet" (many triggerings!).

With the TABLE you can choose between the display in YEARS OF LIFE/MON. decimal and the date specification.

The printed days are to be regarded only as a reference point. If a moving planet is approximately stationary and at the same time within 1/4 DEGREE orb of a Radix planet, then "STATION" is printed in the TABLE.

Such effect points will possibly be active over several years.

For TRANSITE, SEKUNDÄR, SONNENBOGEN directions and ecliptic SYMBOLIC directions the MIDPOINTS of the planets can also be displayed. This leads to a multiple of triggerings (for example several per day for transits).

If this option was chosen, it is automatically switched to "LARGE" symbols, since otherwise the midpoints, in smaller symbols slanted over and behind one another with "/" as separator line, would not be representable. If you have activated this option you should choose only small time sections, since otherwise very many screens are needed, which may overload the memory. Then HORCOM quits without comment!! and one has to press RESET and restart WINDOWS!! With WINDOWS 95 this does not happen as easily as with WINDOWS 3.1

## DYNAMOGRAMM

The Dynamogramm is a special form of the secondary direction with which Mr. JOHANN HUBER (Munich) acquainted me. The method originally comes from KRAFFT and was brought to Mr. Huber by F.G. GOERNER already in the 1950s.

In this method the aspects of the moving planets to the Radix planets and those of the moving planets among each other are graphically summed in both time directions, that is progressive and regressive, whereby the direction equation 1 day = 1 year applies.

For the planets and AC, MC, effect widths and amplitudes are defined. If a factor is within its effect width in aspect to a second one, then its energetic effect is evaluated as an "arc" over the corresponding time interval in which the moving factor lies within the effect width.

The aspects are, depending on character, rated "positive" or "negative" and all amplitudes of a particular point in time are added or subtracted respectively. Among the factors a distinction is made between the "existential situation" and the "psychic basic mood".

The sum of both is called the "resulting energy". The method only becomes vivid when one follows the emergence of the arcs on the screen. In the past cosine arcs were used. In the present form a period of about 5 years is considered. The years of life for the beginning are entered beforehand.

The method was formerly handled graphically with enormous expenditure of time. Mr. Huber, after a computer model had been created by me, still studied with it whether the amplitudes and effect widths used earlier should be retained and confirmed this.

The program is very computation intensive. The relative amplitudes and the orbs can best be observed in action on the screen and can also convey something of the qualities involved. The numbers above the arcs give the planet, the aspect divisor and the multiple of the base aspect.

All arcs are added up or subtracted respectively and yield, for the time span of about 5 years, continuous curves.

Instead of cosine arcs, Gaussian curves (bell curves) can also be chosen (added by me). These form a continuously fading effect and even out the unevennesses in the curves. The orb at which the amplitude has faded to 1/e is then assumed to be 2/3 of the orb for cosine curves.

Mr. J. HUBER is of the opinion that the cosine curves are to be preferred. The details of the method Mr. Huber would like to keep to himself for the time being.

In the analysis of the aspects, a period of + - 25 days (= years) before and after the beginning year of life is evaluated in each case, in order to also capture the slow moving planets within the effect width. Over this period of 50 days (years) a mean value is formed in each case in order to estimate whether one rates considerably too positively or too negatively. In tendency there seems to be rather a somewhat too negative assessment. This mean value (below in the output graphic) is added up within a HORCOM session. If it becomes ever more positive, the assessment is too positive, in the opposite case too negative. A completely balanced result cannot of course be expected. The amplitude scale is arbitrarily fixed, and indeed in such a way that for the majority of cases the curves remain within the area of the screen. But they can also now and then exceed this area. A normalization to the screen dimensions has not proven expedient, since then the relative size of the deflections is easily judged incorrectly. The curves can therefore also leave the screen area.

With this method one can make clear the active (= "positive") or passive phases (= "negative") in life. Positive and negative, however, should not be understood in an evaluative way, but rather like exhaling and inhaling.

| "Positive": | "Negative": |
| --- | --- |
| Animus | Anima |
| Bemächtigungsformen | Bemühungsformen |
| aktiv, zugreifend | passiv, nachgebend |
| kämpferisch | defensiv |
| genießend | sparsam |
| frei | gehemmt |
| Einsatz, Risiko | Sinnsuche |

For prognoses the method should in my opinion only be drawn upon if it is combined with others, for example with solars and transits. Success is also not always to be seen from it, but perhaps whether this comes easily or demands special effort.

Also a high with an introverted type is to be judged differently than with an extraverted one. A low will be more noticeable with an extraverted type than with an introverted one.

It would interest Mr. Huber and me what experiences you have with this method. Please communicate it to him or me as the case may be. According to my tests with the method so far I take a skeptical, but not rejecting attitude toward it.

PRELIMINARY REMARK on PRIMÄR directions:

Of the following SONNENBOGEN PRIMÄR SYMBOLIC directions only one (if any at all) can be "valid", unless one holds the geometry to be an irrelevant matter!

In my view all such directions, including the Primär directions in the narrower sense, are to be regarded as "symbolic" directions, since they evaluate structures of the RADIX. That they in part coincide with real astronomical movements does not change that. In the end it is only a matter of the "right" TIME MEASURE.

The multitude of directions is meant to serve study purposes and does NOT spring from my equal appreciation of all these methods. The existence of several methods, all of which are in use, shows that this is one of the unclarified fields of astrology. An experimental clarification would probably turn out to be quite difficult. The decision whether now the "MUNDAN" geometry is the right one as opposed to the direct equatorial or ecliptic geometry should however be possible, since the triggering points in time are often very strongly different. The decision between SONNENBOGEN EKLIPTIK.SYMBOL. and AR SYSTEM seems to me by contrast quite difficult and will probably long remain a "matter of taste". The often asserted day exact precision of directional triggerings would probably hold up to a statistical examination for no method.

## SONNEN-BOGEN-DIREKTION

Calculates the corresponding triggerings.

The display is largely analogous to that for Sekundär-Direktion. The equation here is:

- 1 ecliptic daily advance of the SUN = 1 year of life

The parameter "Daily movement" is omitted here, since all planets are "pushed forward" with the solar arc.

## MOND-BOGEN-DIREKTION

Here the same applies correspondingly as for the solar arc:

- 1 ecliptic daily advance of the MOON = 1 year of life

This yields, compared to the solar arc, 12 to 14 times as many triggerings, so possibly a kind of "slow motion".

Both for SONNEN and MOND BOGEN DIR. one can work with HOROSCOPE as well as with LINEAR GRAPHIC or with tables. For the MOND BOGEN with LINEAR GRAPHIC, use only the 5 year period!

## PRIMÄR-DIREKTION

Calculates the primary directions according to E.C.KÜHR ("BERECHNUNG DER EREIGNIS-ZEITEN").

In the output table the following mean:

SIG = Signifikator, PRO = Promissor. Signifikator can be all planets, AC, MC and (optionally) the intermediate houses 2, 3 or 5, 6 (8, 9 or 11, 12 correspond to the complementary angles to these).

With the Promissors the aspects (= multiples of the base angle) are always also given, unless one preselects the conjunction (= 360 degrees) as the base angle.

The TIME KEY in YEARS/DEGREE can be entered arbitrarily. The NAIBOD key corresponds unambiguously to the method.

For CORRECTION purposes one has the ARC printed, for already corrected data the time.

Here the calculation is ALWAYS done with PLACIDUS houses, since these correspond optimally to the sense of the method.

Furthermore the Promissors can be evaluated WITH or WITHOUT LATITUDE. Thereby the latitude of the ASPECT POSITIONS of the planets is calculated according to BIANCHINI (see KÜHR). This point seems to me, whichever calculation method is used, very problematic. KÜHR too therefore worked for the Promissors WITHOUT latitude!!, for the Signifikators always WITH LATITUDE, which appears inconsistent. It can therefore also be calculated for BOTH factors WITHOUT LATITUDE. May everyone make his own experiences with it.

For the calculation of the latitude of aspect positions KÜHR did not recommend the BIANCHINI method, but one, quite complicated and hardly automatable, which for the rest also does not make sense to me.

All these obscurities and also some more, for example that the projection onto the equator is always done under the pole of the Signifikator, make this direction method appear to me as particularly problematic. The nimbus that surrounds it could rest on the conclusion: What is (was) laborious to calculate must also be good.

A special aspect of it, namely the so called "MUNDAN HOROSCOPE", I would like to exempt from this skeptical assessment, since this rests on an unambiguous, simple ratio relationship (PLACIDIAN FUNDAMENTAL PROPORTION).

## SYMBOLISCHE DIREKTION ÄQUATORIAL

Here you can now apply 2 methods that I have introduced, also for my own experiments.

(The calculation is ALWAYS done with PLACIDUS houses, since these correspond best to the sense of the methods).

1. The EQUATORIAL DISTANCES in the "MUNDAN" geometry, as it is also applicable with the M.R. They correspond to the PRIMÄR directions, without distinction of Promissor and Signifikator, only according to the "PTOLEM. FUNDAMENTAL PROP." The aspects are measured DIRECTLY ON THE EQUATOR, which seems to me appropriate here. This is also handled thus with the following "AR SYSTEM". The angles are here therefore not spatial but primarily TIME sections. This method is not customary so far, but seems to me, if one wants to use PRIMÄR directions at all, the "natural" method that avoids the mentioned uncertainties (see above).
2. The "AR SYSTEM" of C.O.E.CARTER (see SYMBOLISCHE DIREKTIONEN Urania, Blaue Reihe 3). I have, on the basis of my considerations, modified it somewhat: With the planets the AR distance is evaluated, as CARTER describes this. With AC and intermediate houses the corresponding AO is evaluated. With the MC this is of course identical with the ARMC. My experiences so far with this system are rather better than with the "classical" primary directions corresponding to the presentation in KÜHR.

The AR or AO distance is converted into time according to a selectable key. The natural KEY (= YEARS/DEGREE) for the conversion of the ARC into TIME is in both cases the NAIBOD key. The equation here is:

- 1 DAILY SIDEREAL TIME ADVANCE = 1 YEAR

## SYMBOLISCHE DIREKTION EKLIPTIKAL

Here, in place of the AR, the ECLIPTIC arc DISTANCE applies. This is surely the simplest of all direction types, but in my opinion not less interesting for that.

Here one can also work with other house systems. With this program the mutual ANGULAR DISTANCE of the horoscope factors in the ecliptic can of course also be output directly. AR SYSTEM and SYMB. DIR. may also be applied to SOLAR, LUNAR and TAG-HOR. SYMB.DIR EKLIPT. also runs HELIOCENTRICALLY.

On the repeated wish of users a distinction is made between "DIRECT" = "D" and "CONVERSE" = "K", although I hardly consider this justifiable. The sense of direction is fixed such that the Dir. is rated as "D" if the SECOND planet in ZODIAC SEQUENCE "catches up" with the first. This corresponds to the PRIMÄR directions (KÜHR), whereby PL1 corresponds to the SIGNIFIKATOR.

## TRANSITE

Works analogously to SEKUNDÄR-DIR., only that here the days are counted directly. It also runs HELIOCENTRICALLY and can also be applied to SOLAR, LUNAR and TAG-HOR. The computational effort or time requirement is relatively large. With the TABLE display the following is to be noted:

The UT of the transit in H, MIN is displayed, with the outer planets the H with decimal parts (between the planet symbols, in small print). Near turning points these specifications are only guide values. With INTERMEDIATE HOUSES as RADIX factors, for example +3 is given if the respective aspect "runs into the 3rd house" positively in the zodiac sense, so from the 2nd into the 3rd house. By contrast -3 means that the aspect passes from the 3rd into the 2nd house! The same applies incidentally correspondingly also with SEKUNDÄR directions. After the date the daily movement of the moving factor is displayed in angular minutes. If the sign is negative, then the aspect runs from the larger to the smaller angle, if not, the other way around. Displayed is the, within the framework of the computational accuracy, exact aspect.

Besides the table display, "HOROSCOPE GRAPHIC" display can also be chosen. For it applies:

In the inner circle the horoscope under consideration and the TRANSIT planets are outside. In this display AC and MC are also shown for a preselectable place, whereby one can of course argue about the sense of the matter. In the left half of the picture the main aspects between inner circle (R) and outer circle (T) are listed with 1 DEGREE ORB (changeable via "VORGABEN HOROSKOP.. .."). The conjunctions are not listed therein, since they are directly visible from the figure.

The program runs automatically after preselection of direction and time unit, but can be steered differently during the run via the keyboard (see dialog texts). The exit is done via the ESC key.

The last SCREEN is with TRANSITE and SEKUNDÄR direction STORED retrievably if the running time exceeds 15 sec.

The output form "LINEAR GRAPHIC" enables the creation of graphical ephemerides for multiples of the preselected base angle, for example a 45 degree ephemeris according to R. EBERTIN, whereby the Radix planets are displayed as horizontal lines. The houses 2, 3, 5, 6 are designated as H2, H3, H5, H6. The time span can be preselected either for 1 month, 4 months or 16 months.

The DIRECTION of the ORDINATE can be chosen with "VORGABEN DIREKTIONEN ÄNDERN" either POSITIVE UPWARD (mathematical mode) or POSITIVE DOWNWARD. The latter facilitates the comparison for example with examples of R. EBERTIN.

The LINEAR graphics of the transits can, from 15.10.96, also be output as PRINTER GRAPHIC, if the PRINTER OPTION of HORCOM is active. With TRANSITE etc. one has to wait for the run of the program before the printer works. That can, depending on the speed of the processor, take a while. So exercise patience in such cases.

From August 98 one can also, in the finished screen output itself, mark hit lines with the mouse if the fixed line grid or no lines are specified: LEFT yields BLUE, RIGHT RED lines. If one then makes a hardcopy afterwards, this contains the hit lines drawn in this way.

If the parameter "MIT ZEICHEN" is chosen at the preselection, the sign in which the planets are located is displayed in the curves at intervals behind the planets, insofar as the assignment can be made reasonably unambiguously graphically. The sign boundaries are marked by horizontal dash dotted, blue lines.

## MUNDAN-ASPEKTE

Works with its own input. The aspects of the moving planets among each other are printed. Time requirement likewise high. The degrees given in the table are to be assigned as follows: the upper one to the left, the lower one to the right planet (cardinal points and houses are of course omitted). Regarding the hour specification the corresponding applies as with transits. Since one rarely needs this program, storage was dispensed with. For HORARY ASTROLOGY, with the table output the aspects can optionally be restricted to a particular sign in order for example to see whether the Moon or another planet runs "void of course".

Here too the output form "LINEAR GRAPHIC" can be chosen. If a program needs MORE THAN ONE SCREEN, it is switched onward screen by screen. At the end a TIME SORTING takes place, optionally it can also be sorted by ASPECTS. The sorted values are likewise switched onward screen by screen.

If you want the program to calculate and sort everything up to the end, then enter a "+" after the first screen. The program then runs, as far as the memory reaches, to the end, sorts automatically and then stops.

At "PROGRAMM BEENDEN ?" you then have to answer with "NEIN" and are in the sorted list on page 1. If you want to have this mode always from the outset, you can make a corresponding setting at "VORGABEN DIREKTIONEN ÄNDERN".

---

With all directions, transits and mundane aspects individual factors can be picked out and (or) MARKED RED (not with linear graphic).

Optionally in TABLES the symbols of the "MALEFICS" MA, SA, UR, NE, PL can be INVERTED or (and) the ASPECTS can be colored. The "HARD" aspects 0°, 90°, 180° RED, the "HARMONIC" ones 60°, 120° GREEN.

---

If the calculation is done WITH PARALLAX, then with PROGRESSIONS and TRANSITS a problem arises, since the RADIX factors refer to the PLACE and TIME of birth, the moving ones however possibly to the place of the event or the time of the event. With PROGRESSIONS it should therefore be sensible to ALWAYS choose the PLACE OF BIRTH. With TRANSITS it does not seem to lie so unambiguously. The matter is of course particularly interesting with the MOON, since here the effect can amount to over one degree or up to about two hours difference.
