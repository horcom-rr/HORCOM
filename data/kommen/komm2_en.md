# Explanation of Input-Output

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

## PRELIMINARY NOTE

The following refers to the file handling of HORCOM as it has existed for many years.

Regarding the handling of AAF FILES within HORCOM7P, see the corresponding APPENDIX below !!

The AAF input box appears additionally by pressing the BUTTON "AAF-Format" at the lower left of the HORCOM input box.

The explanatory text, specifically for AAF file handling, appears within the AAF input box by pressing the F1 key.

The INPUT-OUTPUT column serves, besides the INPUT and OUTPUT of data, also to CONTROL the PROGRAM FLOW. In a complex program that is meant to be open for experiments, this in any case requires some practice ! The author has by now invested about 12000 working hours in HORCOM. The "User" too will have to spend about 50 hours in order to reasonably exhaust what HORCOM offers.

## DATEI EIN/AUSGABE

ATTENTION !!

Here you can fetch data records from the DEMO file or from files you created yourself ( see below ), or save them to other files, or delete data records no longer needed.

The data of some prominent people are stored in \DEMO.DAT on your program disk (no guarantee!). You can use these data to familiarise yourself with operating the program. The MC values of these data were often taken from the book 'CIRCELS' (JAN CAMPHERBEEG AMSTERDAM, ISBN 906378044) and converted into UT with the menu item KORREKTUR. For large cities the city centre was always assumed here, which as a rule holds only approximately.

Selecting a data record for fetching or overwriting is done with the mouse, by marking the relevant line.

From the file list you can thus click up to 5 data records and then press "WAHL-ENDE". A double click acts like "WAHL-ENDE".

A data file needs maintenance now and then.

DELETING data records no longer needed is possible. For one deletion operation you can mark up to 10 data records to be deleted.

The file DEMO.DAT is located in the folder \HORCOM\SPEZIAL, in which all other newly created DATA files are stored as well.

## NEU-EINGABE

For the first-time input of a data record "BY HAND".

Unless something else is asked, always enter UPPER or LOWER CASE LETTERS for PROPER NAMES (place names too). Everything is converted to upper case. Enter digits WITHOUT A POINT or COMMA! within the given input fields.

A data record is submitted by clicking the 'OK' field at the lower right. As long as this has not happened, the RADIX data records can be edited as follows: Between the individual edit fields you can jump back and forth with the TAB key, and in part also with the vertical arrow keys. Within the fields you can edit with the horizontal arrow keys and with BACKSPACE or DEL ( ENTF ).

Advancing from data field to data field can always be done with the TAB key, in the opposite direction with SHIFT+TAB.

Jumping from one field to the next happens automatically during new input, when the field is filled and this has been set accordingly under "VORGABEN EIN-AUSGABE ÄNDERN".

The cursor can also be positioned with the mouse.

ATTENTION ! With a DOUBLE CLICK into one of the edit fields you can, at any time and directly while editing, switch back and forth between the TABSTOP version and the automatic version.

With this, I hope, everyone will find their own habit. During "field by field" input a signal tone indicates when the field is filled !

When CHANGING an ALREADY EXISTING DATA RECORD, first move the cursor into the first DATE field and answer the query "ORT SPEICHERN ?" with NO, then go back to the first field ( NAME.. ) and begin overwriting.

If you want to overwrite already filled fields, always first delete these fields completely with "ENTF" or the BACKSPACE key, unless they are marked in colour !!

If, during PLACE input, you want to see the corresponding SELECTION BOX, then with already filled fields that you want to change or overwrite it may be necessary to perform a DOUBLE CLICK in the first field ( NAME, FIRST NAME ). Likewise if the mouse stays only in the last filled field !!

## VORGABEN EIN-AUSGABE ÄNDERN

For the place you can set a PREFERRED PLACE.

Places that you may need again later you can enter into PLACE FILES and fetch again as needed.

PLACE files for GERMANY, SWITZERLAND, AUSTRIA, EUROPE and the WORLD are on the system disc. They are kindly made available to all HORCOM users by Mr BRUNO MAHL and DR. HELMUT WISGRILL.

All PLACE files are located in the folder '\HORCOM\SPEZ_ORT

Since 2003, much more EXTENSIVE place files in the HORCOM FORMAT are now also available, which can be downloaded for WESTERN EUROPE and the USA via my website. They require 17.5 MByte of storage, corresponding to 486000 places.

For (almost) ALL COUNTRIES of the EARTH they can be obtained on CD for EURO 25.- ( abroad EURO 30.- ). These require 79 MByte of storage, corresponding to about 2.2 million places. They are gathered in a folder named ..\SPEZ_ORT\BIGFILES.

These place files are designated by the two-letter code of NIMA ( = NATIONAL IMAGERY and MAPPING AGENCY ), with the larger countries further subdivided by means of letter groups. The code table is shown each time.

For most countries ALL inhabited places are present here, which has the disadvantage that names can occur many times.

In some cases, such as for Russia and China, the number of places is so large that only the noteworthy administrative units could be listed. Even so there are still several thousand.

If, for longitude and latitude, you simply move on instead of entering 'O' or 'N', this is nevertheless interpreted as 'O' and 'N'. Only 'W' or 'S' must really be entered ( first delete 'O' or 'N' ). For the most part you can therefore enter with the right hand on the numeric keypad.

The experienced HORCOM user reserves the LEFT THUMB for the space bar ! For dates B.C. enter, in the field provided for this which you otherwise simply skip, either 'V' or '-' (minus), and for jjjj the NORMAL year number used by historians. The NEGATIVE year numbers that then often occur in results, however, are the ARITHMETICAL counting: For example "the year 63 B.C." corresponds to the year -62 !!, so the year zero corresponds to the year 1 before Christ !!

See also Explanation 3 under "KALENDER".

Otherwise, for dates B.C., the letters VC or V are placed after the date.

The TIME OF DAY is to be entered as ZONE TIME ! In ( almost ) all OUTPUTS, however, GREENWICH time GZ = GMT = UT is shown, since only this is also stored.

Given the zone time, the UT is determined as follows:

```text
                 UT = GMT = ZONE TIME + ZZD
                 ZZD = ZONE TIME DIFFERENCE
```

During the NEW input of data a dialog appears for setting the zone time, if the relevant CHECKBOX was clicked in the input box. It shows at the upper left an edit field and two tables in which the ZZD values can be seen after the PLACE name. If you want to choose one of the more distant zone times, look in the table for a city in the desired region and click it. This value then appears in the edit field and you can accept it with "OK".

For MEZ ( = Central European Time) the ZZD = -1 h. If you want to choose it, you do not need the tables but click "OK".

If SUMMER TIME applies, click the relevant CHECKBOX.

For geographic longitudes east of Greenwich the ZZD is generally negative, west of it positive.

Common abbreviations for the nearer zone times are:

```text
       MEZ  = CENTRAL European Time            ZZD = -1
       WEZ  = WEST European Time               ZZD = 0
            = UT = GMT
       OEZ  = EAST European Time               ZZD = -2
       DSZ  = German SUMMER Time               ZZD = -2
       DDSZ = Double German SUMMER Time        ZZD = -3
```

If you want to enter LOCAL time, which is necessary for HISTORICAL horoscopes, enter the given local time in the time fields and mark the relevant CHECKBOX "ORTSZEIT".

Please do not confuse LOCAL TIME with ZONE TIME. In HORCOM local time is always used in its original sense.

Before 1810 ( Germany ) local time referred in each case to the TRUE position of the sun.

Since UT is based on a computed MEAN position of the sun, the EQUATION OF TIME must also be taken into account, for dates before 1810, when converting TRUE local time into MEAN local time. This correction can amount to about +,- a quarter of an hour.

It is now ( from the 2005 editions on ) automatically taken into account for dates < 1810.

For dates from 1810 ( Germany ) onward it may at least be assumed that

```text
the MEAN local time is meant, in short    LMT = Local Mean Time
                                     or   MOZ = Mittlere Ortszeit.

Correspondingly                           LTT = Local True Time .
                                     or   WOZ = WAHRE Ortszeit.
```

In France the cut-off date is 1816. In many other European countries only 1884, that is relatively shortly before the introduction of ZONE TIME in 1890.

For dates between 1810 and 1890 it can therefore be chosen which local time is to be entered.

A further CHECKBOX under ORTSZEIT serves to note when, after 15.10.1582, the JULIAN calendar was still used. If so, the character string "(JULIAN.)" is stored under BEMERKG., which is then queried again and displayed when fetching from the data file, but only insofar as these data records were created with the present version of HORCOM, which was not yet the case with the DEMO file. For in the data file only the UT is ever stored.

An extensive documentation of the time determinations for Europe on diskette was kindly produced by Mr B.MAHL and can be read during input by clicking the BUTTON "ZEITBESTIMMUNGEN LESEN".

After the time fields the CURSOR first moves into the lower field, in which you can enter short COMMENTS of up to 50 CHARACTERS.

If you have not clicked any of the CHECKBOXES, the entered time is interpreted as UT = GREENWICH TIME !

If you want to convert DECIMAL DEGREES into DEGREES/MIN/SEC during input, press the function key F5 ( or ALT + R ), which makes a helper routine accessible that you leave again with QUIT.

The query whether a data record should be saved comes, for data records from the file, only if it was entered anew or if a data record fetched from the file was changed.

The inputs are not checked as to whether they make sense. You must do that yourself. Nonsensical inputs lead to nothing. In the worst case to a crash of the program.

The input box also serves as a display box, which for RADIX data records is always immediately editable (=changeable). This editability has as its downside that confirmation must always be given in the 'OK' field.

In the DISPLAY BOX the time is always given as UT = GMT = WEZ !

The display boxes with RADIX data are always shown in the upper half of the screen, those of the RESULT data records (SOLAR, LUNAR, DOUBLE HOROSCOPES etc.) in the lower.

The OTHER INPUTS, during the operation of the program, are likewise formatted via DIALOG BOXES. These dialogs force you, step by step, to tell the program what is needed. You do not have to decipher ICONS in the process. Everything can always be read in plain text, with occasional concessions to computer English, which is usually shorter than corresponding German expressions.

Read the DIALOG TEXTS, especially at the beginning, always carefully before you click.

SIMULTANEOUSLY up to 5 INPUT RECORDS can be entered. This is occasionally felt to be too few.

If you want to scan through many horoscopes without filling the input, mark the data record in the file list only with a single click and then click "NUR HOROSKOP ANSEHEN".

If you want to take over a data record, you must, directly after calling up the file, either perform a double click on the relevant line or, after marking the record or records, click "WAHL-ENDE" !

You can also, if you want to scan through many horoscopes, create an "analysable file" with the module "STATISTIK" ( in the section "EPHEMERIDE" ). Here you can then create the output list with various criteria and conditions and, by marking a data record in each case, view the respective horoscope.

A data record remains valid for further menu items as long as no other data record is selected, no switch to a result level ( e.g. SOLAR ) is made, and nothing is newly entered or reset.

The CURRENT = VALID data record is marked in the MENU by a CHECK MARK !!

So under EIN/AUSGABE only 1 check mark can be set, whereas for the results under HOROSKOPE, AUSWERTUNG several can be, if OUTPUT IMAGES were temporarily STORED !!

Important for the beginner is to be clear about the concept of "LEVEL". After input one is in the RADIX LEVEL. After a SOLAR in the SOLAR LEVEL etc.

The UPDATING of already entered RADIX or computed RESULT data records is done simply by CLICKING with the mouse ! Under RADIX-DATEN you can UPDATE = CLICK the individual RADIX records, which then have the check mark.

If you have switched to the "SOLAR LEVEL" or "LUNAR LEVEL" etc., the data are likewise stored and can be ACTIVATED again under the line SOLAR...-DATEN, quite analogously to RADIX-DATEN. Only EITHER SOLAR or SEPTAR or LUNAR or TAG-HOR is stored.

You can go back to the RADIX level by clicking under RADIX-DATEN.

Menu items shown as INACTIVATED are NOT selectable. Programs shown as INACTIVATED are likewise NOT accessible. If you want to use such programs, you must usually go back to the RADIX level, in which all programs are selectable, provided at least one data record has been entered !

When a data record is ACTIVATED, any associated stored SCREENS are also marked with a check mark (under HOROSKOP.. or AUSWERTUNG).

Switching between the RESULT level and the RADIX level requires some getting used to, but also ensures maximal VERSATILITY. Programs that lead out of the RADIX level are marked with ^.

From the RESULT level, NOT ALL PROGRAMS ARE STILL MEANINGFUL, and therefore not accessible either.

With "ERGEBNIS als RADIX" (under "DIVERSES") you can, if you feel confident, nevertheless use all programs, e.g. make a solar of a lunar. But be careful with it ! That is something for the experienced.

The DOUBLE horoscopes COMPOSIT, COMBIN and DOPPELKREIS likewise form a RESULT level, from which, however, further programs are selectable only for COMBIN.

Activate under ERGEBNIS-DATEN, fetch back under the corresponding program in the section HOROSKOPE or AUSWERTUNG, when there is a check mark there ! This image storage is provided only during a HORCOM session and only when the computing time exceeds 15 seconds.

With the DOUBLE horoscopes, make sure that the 2 data records are always already entered before you select the relevant program: Otherwise the input procedure becomes confusing !

Entered RADIX data records, including comment, you can save to diskette via DATEI EIN-AUS and, as needed, fetch again from there, whereby when searching you can SORT by ALPHABET, BIRTHDAY or DATE.

A data file needs maintenance now and then.

DELETING data records no longer needed is possible. Per deletion operation you can mark up to 10 data records each time.

The SAMPLE file has the name DEMO.DAT and is located in the folder \HORCOM\SPEZIAL .

With "Datei TRIMMEN" empty data records and spaces before the name can be removed.

With "Datei MINIMIEREN" data records present multiple times with the same name and the same birth time are reduced to one each. You can create your own DATA files at will by entering a first data record by hand ("NEU-EINGABE"), then answering the query "DATENSATZ ABSPEICHERN" with "JA" and entering a name of your choice ( maximum 8 characters ) in the upper edit field of the FILESELECT BOX that then appears. UMLAUTS are to be avoided here.

Example: FAMILIE.DAT would be an admissible name.

All DATA files must be created in \HORCOM\SPEZIAL ! This happens automatically when you enter the name of your file in the FILE SELECT BOX. You can rename DATA files via the Program Manager of WINDOWS, but only the file, not the folder ( = directory ), the path or the extension (= the 3 letters after the . )

If you want to handle large amounts of data, it will be expedient to subdivide your data into SPECIAL FILES.

Enter only RADIX data into the files, since these always count as Radix data when fetched again !!!

The data records are limited for the NAME to 25 CHARACTERS, for the PLACE to 20 CHARACTERS, for REMARKS to 51 CHARACTERS.

Different files can be joined together with "DATEIEN VERKETTEN". (see Explanation 9).

Addendum regarding the format of the DATA and PLACE files used in HORCOM.. for people who want to convert other files to the HORCOM FORMAT:

The files in HORCOM ( all versions ) are RANDOM ACCESS files with the following layout ( for FIELD see e.g. GFA-BASIC ).

### 1. DATA files

```text
FIELD #1,2 AS ta$,2 AS mo$,5 AS ja$,2 AS ho$,5 AS mi$,8 AS ggl$,
           Day        Month   Year    Hour     Minute  Geog Long
e.g.      15         11      +1979    08      15.50   +011.636

          8 AS ggg$, 25 AS naa$,20 AS goo$,51 AS bem$
        Geog Lat    Name+First   Placename Remark
e.g.   +52.1264    Muster Hans  Magdeburg xxxxx....
```

The total length of a data record is 128 BYTES.

### 2. PLACE files

```text
FIELD #1,8 AS ggl$,8 AS ggg$,20 AS goo$         = 36 BYTE / data record
```

Meaning of the strings as the same-named ones above.

The individual quantities are stored as strings and, where required, after reading, converted back into integer or floating numbers with VAL(), whereby in some cases space is provided for the sign.

The stored time is UT = GMT !!

Where a + appears above, the sign + or - must always be provided.

The strings must of course be brought exactly to the above format and length with the string functions STR$(), MID$(), RSET(), LSET() etc. The last three strings are to be provided left-justified.

Please make such conversions only if you have some practice in programming.

STATISTIK files can only be managed by HORCOM, since they work with encoded integer numbers.
