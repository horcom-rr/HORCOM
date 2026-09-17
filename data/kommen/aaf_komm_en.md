# Explanation of AAF Input and Output

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

## COMMENT concerning AAF = Astrological Exchange Format

It is the achievement and the initiative of MARTIN GARMS to have created this file standard, in order to make possible the exchange of files between different astrology programs. For this he deserves the thanks and recognition of astrologers and programmers.

The website of M.GARMS is, by the way:

http://www.sternwerkstatt.de

The format may at first seem somewhat complicated to some. But one will have to accept that this is founded in the nature of the matter. Thinking about the details, in particular about the reckoning of time, is spared to no one who wants to work exactly. It was also the aim, in designing this format, to satisfy as far as possible all demands, including and especially those of the researching astrologers.

## HANDLING of AAF files within HORCOM7P

The present text appears, within the AAF input box, when you press the F1 key!

The basic concept consists in the fact that, as before, the file handling of HORCOM stands in the foreground, but that one can, when it is expedient, switch at any time in the Radix level into the AAF format in order to have available the greater scope of information of the AAF format.

It is therefore a kind of "double-entry bookkeeping" for the Radix records, which has proven to be not entirely simple.

For the rest, the handling of the HORCOM input is essentially unchanged. Only, instead of "O" for East, an "E" is now used here too, as is customary in the AAF format.

You can now, if there is a need for it, also write longer comments etc. for your already existing HORCOM data files into the respective associated AAF file, and access them again at any time.

If a new AAF file is inserted into the folder ...\AAFDATEN, then the associated HORCOM file is to be created with the corresponding menu item "AAF-DATEI <> HORCOM-DATEI".

If, for example, a new AAF file named ..\AAFDATEN\NEUDAT.AAF has been found, then a HORCOM file named ...\HORCOM\SPEZIAL\NEUDAT.DAT is formed.

The parallel HORCOM file OF THE SAME NAME serves as the pilot.

The reverse conversion from HORCOM into AAF format should only make sense when data exchange with AAF-compatible programs is intended.

If AAF files are present to which a HORCOM file of the same name corresponds, then a button named "AAF-Format" appears at the bottom left in the HORCOM input box. When you click there, the richer AAF box appears, in which you can also make changes.

Changes via the HORCOM input should then be omitted, and one should switch to the AAF input instead.

Additional information is then indeed partly lost in the HORCOM file, but can be viewed again at any time via the AAF box. HORCOM, for example, always stores the time only as GMT = UT, remarks are cut off after 51 characters. That still dates from the time when storage space was scarce. Today one can forget about this.

ATTENTION! With the umlauts there are again and again mistranslations, which hardly seem to be avoidable, since the files may have been created with different code tables. In the AAF format, commas are important separators! If a comma "arises" at the wrong place, e.g. because a comma is "translated" instead of an umlaut, the record in question can no longer be correctly interpreted. You recognize this by the fact that at the date 1.1.-4712 12h appears. Or (and) the name is garbled. Against this I could provide only limited remedy.

With files that you have created yourself within HORCOM, this will hopefully not be the case, provided you always have the same code table set on your computer.

For numbers there is no such problem.

With "NEU-EINGABE" you are asked in which format you want to enter. The AAF format offers, among other things, the possibility of entering extensive comments, source references, remarks on data quality and search terms. This is of great advantage both for the researching and for the advising astrologer, because he can survey all data at once. Regarding the details that are to be observed during input, see at the end of this comment.

If you want to inform yourself about the extensive details of the AAF standard, open the help file ..\HORCOM\AAFHELP\AAF.HLP with the button "AAF-HELP".

Within this file you can read all the explanatory texts by MARTIN GARMS. There are not a few of them.

Nevertheless, you should not shy away from the effort if you want to work with the format.

Attention! When one views AAF files with a browser on the Internet, these are at first present in HTM format. Please then take care, when downloading, that you save the AAF file in .TXT format, or right away with the extension .AAF, that is, not as HTM files.

In case you forget that, however, provision is made that the "tags" of the htm files are eliminated (I hope).

TXT files that are encountered in the folder \AAFDATEN are automatically given the extension .AAF. So copy only files with the extension .AAF or .TXT into this folder, and ONLY SUCH files as are actually AAF files.

So do NOT bring in ANY OTHER TXT or HTM files!!

ATTENTION! Use ONLY pure TXT format and not, say, RTF format (= Rich Text Format) or even WORD format. That does not work, because in that case formatting code is stored along with it!!

---

From the AAF format, only the first three groups are used in HORCOM. These are:

- AAF-A: The "civil" starting data for a horoscope
- AAF-B: The calendrically and geographically exact horoscope data
- AAF-C: Various text information such as comment, source etc.

The possibilities of the further groups, insofar as they concern the evaluation of computation results, can within HORCOM7P mostly be handled by the module "STATISTIK" (under "EPHEMERIDE"), by producing "evaluable files" from the pure data files. That goes very quickly.

The storage of results themselves, which is likewise provided for in the AAF format, appears today no longer very interesting, since such results are each computable within seconds.

The AAF format is therefore, for the time being and certainly also in future, used within HORCOM only as a RADIX file!

---

The writing of AAF records can in principle be done with a text editor. This presupposes perfect knowledge of the syntax rules! There will probably be few who can do that without errors!

Considerably safer for the later readability is it, therefore, to accomplish the input with the AAF input box via "NEU-EINGABE".

Within the AAF input box, records that have been fetched from an AAF file can also be changed by hand and stored back.

For the changing of a record, the following applies, which is listed for NEUEINGABE.

---

For NEU-EINGABE with the AAF input box (only HORCOM7P) you fill in the edit fields as follows:

1. "Name"

2. "Vorname"

3. "Horoskopart" you fill using the given list selection.

4. "Datum"

    For dates before Christ you enter the ASTRONOMICAL year number WITH A MINUS SIGN: The HISTORICAL year 1 before Christ corresponds to the year 0 in ASTRONOMICAL counting.

    Example:

    ```text
    ASTRONOMICAL : -500
    HISTORICAL   :  501 before Christ
    ```

    Year numbers with a negative sign are therefore ALWAYS to be understood in ASTRONOMICAL counting! The associated HISTORICAL year number is displayed behind the ASTRONOMICAL year number in a "passive" field.

    If the JULIAN calendar is still to be used AFTER 4.10.1582, which will not seldom be necessary, a "j" is appended to the year number.

    Correspondingly, the AAF format also offers the possibility of using the GREGORIAN calendar BEFORE 15.10.1582. Then a "g" is appended. This will practically come into consideration less. In HORCOM this is NOT provided for.

5. "Zeit" enter as the respective "civil time", that is, either ZONE TIME (for more recent dates) or LOCAL TIME for dates before the introduction of zone time.

    With LOCAL TIME, strictly speaking, a further distinction is to be made between:

    MEAN local time LMT = Local Mean Time, for dates roughly after 1810, which rest on an astronomically defined MEAN SOLAR TIME, and the

    TRUE local time LTT = Local True Time, for dates before that, which rest on the TRUE POSITION OF THE SUN and is present in any case before the mentioned date.

    The difference between the two is calculated with the so-called EQUATION OF TIME, about which you do NOT have to worry. It can amount to up to about a quarter of an hour +- (maxima in February and November). Normally, for dates BEFORE 1810 the LTT is (after a query) converted into LMT, since today's astronomical time reckoning presupposes this. Answer the corresponding query accordingly with 'OK'. The query was left in so that you remain aware of the problem of the matter.

6. "Ortsname"

7. "Land" (= the vehicle registration code for the nation) you enter mostly via the given list.

8. "Juldatum" you NORMALLY leave EMPTY (input is, however, NOT deactivated), it is normally filled in automatically later.

    The Juldatum is the astronomically exact time specification for the ephemeris computation and normally need not interest the user, unless he needs it for astronomical considerations. The stated Juldatum refers to UT = GMT! The ephemeris time ET is determined internally by the program. The difference ET - UT amounts, for example up to A.D. 0, to over two hours! But the user does NOT have to worry about that!

    The Juldatum has priority over other time specifications. If a JULDATUM is entered primarily, then the following is set automatically:

    ```text
    ZNAM       = GMT
    ZONE(ZZD)  = 00hW00 or 00hE00 or "*"
    SOMMERZEIT = "*"
    ```

    The date and time specifications then refer to UT = GMT. ATTENTION! If you enter a Juldatum PRIMARILY, you do NOT need to fill in date and time! The fields are computed and filled in automatically.

9. "Breite" and "Länge", that is, the geographical location coordinates.

    "Länge" MUST ALWAYS BE FILLED IN when LOCAL TIME is present, since it then enters directly into the time!!

10. "Zone ( ZZD )" you normally fill using the given list selection. The field 13. = "ZNAM" is then filled automatically. The SORTING of the zone names is ALPHABETICAL. The LOCAL TIMES LMT and LTT respectively you will accordingly find roughly in the middle of the list.

11. "Sommerzeit" you fill using the given list selection.

    Please ALWAYS FILL IN this field when the time is to be correct and ZONE TIME is present that does not already contain summer time!! If afterwards LOCAL TIME LMT or LTT is noted at "ZNAM", then a "*" is automatically entered in this field!!

12. "COM","VIA","SRC","GZQ" you fill as needed and according to the rules of the AAF.

13. "ZNAM" is normally filled automatically (see under "Zone").

14. "CWORD","ATTRB" you fill as needed and according to the rules of the AAF.

After that you press "Speichern" and choose the file into which the record is to be saved. In doing so you can also enter a NEW NAME for the file, which then contains the current record as the first one.

If you want to save several records for ONE subject, you must additionally enter a characteristic (e.g. an appended digit), for example at "Zuname"!!

As SEARCH CRITERION, in order to find in each case the correctly assigned AAF record, the following serve: Zuname + Vorname.

ATTENTION!

If you delete a HORCOM record, an associated AAF record is deleted along with it.
