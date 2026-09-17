# Short Manual

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

## SHORT INSTRUCTIONS for getting started quickly with HORCOM

## Operating HORCOM in WINDOWS

The first line in the main menu is merely a title line, which also contains the date of the version.

The actually important MENU LINE shows the top-level headings:

ÜBER HORCOM EIN-AUSG. EPHEMERIDE HOROSKOPE AUSWERTUNG DIVERSES

When you click one of the top-level headings, a list of program titles unrolls, which you should first of all look at all of, so that you begin to sense what all there is in HORCOM. Especially instructive is also to go through the menu items VORGABEN...ÄNDERN once, in order to see what all can be influenced. With the defaults for HOROSKOPE you should in particular set the colors for the horoscope ring and the aspect lines to your taste, or for the time being leave the HORCOM standard settings (recommended for the beginning).

## First attempts

Click in the main menu in the menu line on EIN-AUSG. The menu items unroll that are needed for the input and output and the activation of the entered records.

Click right on the first line "DATEN-DATEI EIN-AUSGABE". Now the WINDOWS FILESELECT BOX appears, in which you at first see only the files MUSTER.DAT and MERIDIAN.DAT.

The data file MERIDIAN.DAT is based on examples from earlier issues of the MERIDIAN, with the corresponding file MERIDIAN.AAF in the folder ..\AAFDATEN.

If you fetch a record from MERIDIAN.DAT, then a button named "AAF-FORMAT" appears at the bottom left in the HORCOM display and input window. With it you can view and possibly edit the corresponding AAF record.

This also applies to every valid AAF file that is found in the folder ...\AAFDATEN.

Now comes a query as to how you want the data sorted. Here at first simply press the RETURN key, which in each case makes the choice with the thick border (here ALPHABETICAL).

Now a list unfolds, e.g. with the names of historical personalities. Now click one or up to 5 names once, and then click in the box "WAHLENDE". Here do not perform a double click, since it possibly involves a multiple selection. Thereupon the DISPLAY BOX appears with the corresponding data. On a double click you leave the list! Active, or valid, is in each case the record chosen last. It is marked by a preceding check mark.

By clicking another of the selected records, this one is selected as active.

Now you can do something with the active record, e.g. make a horoscope.

You are now back in the main menu and click in the menu bar the top-level heading "HOROSKOPE", and once the list of programs has unrolled, the line "HOROSKOP-GRAPHIK". Now a horoscope is drawn, which you can look at as long as you like.

The text window can be searched by terms.

The comments 1, 2, 3, 9 and the one for "STATISTIK" are quite extensive. In the menu, the comments always refer to the themes standing above them respectively.

You should skim through all the explanations at least once in order to know roughly what all there is to observe.

Very useful is it also to go through all the menu items named "VORGABEN....ÄNDERN" point by point once, and thus to get to know which parameters are settable and how. During the first hours with HORCOM, also press the function key F1 from the result screens now and then, so that you gain routine with the setting possibilities.

Please also read, in the first period, the texts in the DIALOGS exactly. They likewise often have the character of explanations.

Programs that have a * in front of the name are to be used independently of the valid record. They are mostly astronomical auxiliary routines.

A special position is taken by "STATISTIK", an extensive module that serves for the statistical investigation of larger files.

After these necessary intermediate remarks, you can now do something new, e.g. look at a SOLAR.

For this you go into the category "AUSWERTUNG" and then into the line "SOLAR-SEPTAR-LUNAR-PLANETARE-PERSONARE" and press RETURN, whereby "SOLAR-HOROSKOP" is chosen.

After entering place and time via corresponding queries, after some computation time (it is computed very exactly with iteration) the SOLAR appears directly.

Now you are no longer in the "RADIX LEVEL" but in the "SOLAR LEVEL", from which not all programs are any longer meaningful and are therefore partly switched inactive. The check mark in the category "EIN-AUSG." in the menu now also no longer stands under "RADIX-DATEN" but under "SOLAR.....-DATEN". So that you again have all programs available, you go back into the RADIX LEVEL by clicking, under RADIX-DATEN, the record that interests you. It then again has the check mark placed before it. Now look, in all peace, at the program titles under EPHEMERIDE, HOROSKOPE, AUSWERTUNG and DIVERSES exactly, so that you see what top-level headings there are.

After clicking these top-level headings, often groups of subprograms are opened, which in turn can partly run in different modes of presentation, e.g. as a table or horoscope or as a linear graphic with different parameters.

The parameter settings (defaults) are mostly accessible in the first line (except in the column EIN-AUSGAB). If you want to know exactly here, the reading of the explanations is not spared you. For the first attempts you can leave the preset settings (= with a thick border) here, i.e. press RETURN.

Now you can perhaps enter a new record, e.g. your birth data, by hand, by clicking, under EIN-AUSG., the 2nd line NEU-EINGABE ...

Now the INPUT BOX appears.

As of HORCOM6P there are two input possibilities. Firstly the "conventional" input box of HORCOM or, optionally, the more extensive one for the AAF file format. About that everything is in the file AAF_KOMM.TXT, which also appears when one presses the F1 key in the AAF input box.

What follows here refers to the HORCOM input box.

When nothing else is asked, always enter CAPITAL or SMALL letters, with PROPER NAMES (also place names). Digits WITHOUT PERIOD or COMMA! within the given input fields.

A record is submitted by clicking the OK field at the bottom right. As long as this has not happened, the RADIX records can be edited as follows: Between the individual edit fields one can jump back and forth with the TABULATOR key, partly also with the vertical arrow keys. Within the fields one can edit with the horizontal arrow keys and with BACKSPACE or DEL (ENTF). The advancing from data field to data field can always happen with the TABULATOR key, in the opposite direction with SHIFT+TAB or the arrow keys.

The jumping onward from one field to the next happens, in the case of new entry, either automatically, when the field is filled, or optionally with TABSTOP, depending on how it was determined at VORGABEN EIN-AUSGABE ÄNDERN!

ATTENTION! With a DOUBLE CLICK in one of the edit fields you can at any time switch back and forth directly during editing between the TABSTOP and the automatic version.

With that, I hope, everyone will find his own habit.

When CHANGING an ALREADY EXISTING RECORD, first go with the cursor up into the first DATE field and answer the query "ORT SPEICHERN ?" with NO, then go back into the first field (NAME..) and begin with the overwriting.

If you want to overwrite already filled fields, always first delete these fields completely with "ENTF" or the BACKSPACE key, unless the field in question is highlighted in color, then it is deleted automatically. If you want to see the corresponding SELECTION BOX at the PLACE input, then with already filled-in fields that one wants to modify or overwrite it is possibly necessary to perform a DOUBLE CLICK in the first field (NAME, VORNAME). Likewise, when the mouse stops only in the last filled-in field!!

At the place you can define a PREFERRED PLACE, mostly probably your place of residence.

Places that you might need again later you can enter into PLACE FILES and fetch again when needed.

PLACE files for most European countries are located on the system disc.

All PLACE files are located in the folder "\HORCOM\SPEZ_ORT. If, at longitude and latitude, instead of "E" or "N" one simply moves on, then this is nevertheless interpreted as "E" and "N". Only "W" or "S" one really has to enter (delete "E" or "N" beforehand). To a large extent one can therefore enter with the right hand in the numeric keypad.

For dates before Christ you enter, in the field provided for that, which you otherwise simply pass over, either "V" or "-" (minus), and at jjjj the NORMAL year number used by the historians. In the case of the NEGATIVE year numbers that may then occur in results, however, it is a matter of the ARITHMETIC counting: e.g. "the year 63 before Christ" corresponds to the year -62!! the year zero thus to the year 1 before Christ!! See also Expl.3 at "KALENDER". Otherwise, in the case of dates before Christ, the letters VC are placed after the date. The TIME OF DAY is to be entered as ZONE TIME! With (almost) all OUTPUTS, however, GREENWICH time GZ = GMT = UT is displayed, since only this is also stored.

The UT is determined, when zone time is present, as follows:

```text
UT = GMT = ZONE TIME + ZZD
ZZD = ZONE TIME DIFFERENCE
```

When newly entering data, a dialog appears for setting the zone time, if the relevant CHECKBOX in the input box was clicked. It shows at the top left an edit field and two tables in which, behind the place name, the ZZD values are to be seen. If you want to choose one of the more distant zone times, look in the table for a city in the sought region and click it. In the edit field this value then appears, and you can take it over with "OK". For MEZ (= Central European Time) the ZZD = -1 h. If you want to choose this, you do not need the tables but simply click "OK".

If SUMMER TIME applies, then click in the relevant CHECKBOX (click in the small square! in which a cross then appears)..

For geographical longitudes east of Greenwich the ZZD is generally negative, west of it positive.

Common abbreviations for the nearer zone times are:

```text
MEZ  = Central European Time                   ZZD = -1
WEZ  = Western European Time                   ZZD = 0
     = UT = GMT
OEZ  = Eastern European Time                   ZZD = -2
DSZ  = German SUMMER Time                       ZZD = -2
DDSZ = Double German SUMMER Time                ZZD = -3
```

If you want to enter LOCAL TIME, which is necessary with HISTORICAL horoscopes, mark the relevant CHECKBOX and enter the given local time in the time fields.

Please do not confuse LOCAL TIME with ZONE TIME. In HORCOM, local time is always used in the original sense.

A further CHECKBOX under LOCAL TIME serves to note when, after 15.10.1582, the JULIAN calendar was still used. If yes, under BEMERKG. the character sequence "(JULIAN.)" is stored, which is then, when fetching from the data file, queried and displayed again, but only insofar as these records were created with the present version of HORCOM, which with the MUSTER file was not yet the case. In the data file, namely, always only the UT is stored.

If files in the AAF format (Astrological Exchange Format, which Mr. MARTIN GARMS has thankfully established) are available to you, then you can convert such files with the corresponding menu item in the last column of the main menu into HORCOM files or vice versa. As of version HORCOM6P you can, and should then also, edit in the AAF format yourself.

Robert Rettig, Eichenau, 21.09.2008
