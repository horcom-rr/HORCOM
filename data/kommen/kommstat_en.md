# Explanation of Statistics

*Robert Rettig, from the original HORCOM documentation (KOMMEN7P folder), English edition, his wording preserved in translation.*

COMMENTARY on "STATISTIK":

PRELIMINARY REMARK:

I strongly recommend that you read through the following text carefully once, so that whenever something is unclear you know where to look it up. This will save you precious time.

To practise the possibilities of the program, it is best to use the file "MUSTER.STA" on the SYSTEM DISKETTE.

"STATISTIK" was developed for the researching astrologer. It is certainly nothing for the astrological beginner or for uncritical people.

I am by no means inclined to overrate the value of statistical consideration in astrology. The subject is too complex for that.

But the search possibilities now provided with this program module will surely be welcomed by every astrologer, and fill a long felt gap in HORCOM.

With statistical methods in the narrower sense one can, in my opinion, only find quite fundamental facts, as for example the GAUQUELINS have done. For individual statements they are completely irrelevant.

If you were to try to test your astrological "basic convictions" statistically, in the vast majority of cases you will be all the more disappointed the larger your test collective is.

Actually, though, every independently working astrologer is also a "statistician", since he will only regard something as significant if it stands out from "chance".

That can also become visible in a single case. However, the quantification is difficult and one should not underestimate the influence of "chance".

In engineering, for example, the content of a message is expressed by a measure of its improbability. With the present program you can track down improbable constellations, and test corresponding hypotheses. That, for me, is the real purpose of such a program. Statistics in the narrower sense only becomes possible with collectives from about 400 upwards, if distributions across the Häuser or the signs are to be tested. Even then, though, "chance" must still be reckoned with, with scatter of up to about +- 10% in the relative frequency.

If you want to test distributions across the individual degrees for single factors, you need collectives from 10000 upwards, which is probably rarely possible.

The computing times would then also become quite unpleasantly long. In an individual case it can really be difficult to decide whether a result stands out from "chance", that is, whether it is "significant" or not. Quantitative procedures for estimating the significance of a finding were not introduced for the time being, since they can as a rule only be formulated for quite specific questions and are also likely to be familiar to only a few astrologers. Added to this is the fundamental difficulty of specifying "a priori probabilities" on the basis of the astronomical circumstances, without which a calculation of confidence values is not possible. Knowledge of astronomical conditions is in any case not unimportant. For example, the observation that the ascendant in our latitudes is more frequent in the signs LE, VI, LI, SC, SG than in AQ, PS, AR, TA, GM is to be expected for all collectives, since it rests on geometric conditions. The use of "STATISTIK" in the actual sense of the word therefore requires careful reflection of the user's own regarding the significance of results.

## OPERATION

The operation of "STATISTIK" should present no problems for users who already know HORCOM, since it is precisely here that the dialogue guided operation is consistently practised.

When you select the menu item "STATISTIK" in the main menu, you see a first sub-menu named "WAS WOLLEN SIE TUN ?", whose first bar "VORGABEN ÄNDERN" has the following function:

Here you can arrange in advance that your statistics file is UPDATED each time you save a DATENSATZ in a DATEN file, if a STATISTIK file of the same name already exists in the folder \STATIST. Since the data must be taken over with the correct parameters, the process is automatic, without possibility of intervention, and takes a little while, since it is recalculated as a precaution.

ATTENTION !! If in a STATISTIK file a data record is to be saved with a name that already exists, it is rejected. This is to ensure that there are not 2 or more identical records present in the STATISTIK file. If you do want to save this record, you must change the name. One letter or digit is enough.

Data records that are OVERWRITTEN in the DATEN file for correction purposes are, however, likewise newly written into the STATISTIK file if UPDATING is switched on. This only works if the names in both files match. Please also do not try to "smuggle in" new data records under the same name. That goes wrong !!

In addition, you can subsequently choose whether you want more information in small print in the output list or less in larger print. The second menu bar "AUSWERTEFÄHIGE DATEI ERSTELLEN" has the following function:

This program makes an evaluable file out of a DATEN file to be selected, which is assumed to be in the folder \SPEZIAL\... .DAT, by calculating and saving the planet and Häuser positions for each data record. With larger files this process can take somewhat long. Each data record takes up 210 BYTE on the hard disk.

Of the house cusps, AC, MC as well as cusp 2, 3, 5, 6 are stored. The EVALUABLE FILE = A.D. is stored in the folder \STATIST\ with the extension .STA and otherwise has the same name as the underlying DATEN file.

ATTENTION ! The folder \STATIST is normally managed completely by the program. So please do not load or remove anything into it yourself. That could lead to a crash.

The following, however, is to be observed:

If you delete a DATEN file (extension .DAT, folder SPEZIAL) to which a STATISTIK file (extension .STA, folder STATIST) also exists, it is advisable to also delete the STATISTIK file of the same name concerned via the menu item "STATISTIK-DATEI LÖSCHEN" (that is, not via the DESKTOP).

If, for example, the DATEN file \HORCOM\SPEZIAL\MUSTER.DAT is deleted via the desktop, do not forget to likewise delete \MUSTER.STA via the mentioned menu item. Otherwise it can, if a DATEN file is to be created again under the same name, come to nonsensical outputs in "STATISTIK".

Please make sure that you have set the parameters for planet calculation according to your habits, otherwise you have to repeat the whole thing again. These are likewise stored in the folder \STATIST and restored again during the evaluation. So do not be surprised if after a statistics evaluation the ephemeris parameters are possibly changed. The calculation results are stored as 4-byte integer numbers. That is enough to recover the values far better than 1 arcsecond of accuracy. Likewise the NAMES are stored, but not the REMARKS of the DATEN files.

The third menu bar "AUSWERTUNG" allows you to apply versatile search operations in your A.D.

The operation of the program is completely and step by step DIALOGUE controlled.

Whole numbers you can enter via special boxes with the mouse. If you want to enter decimal numbers, you can, after pressing any key, enter the decimal value via the keyboard with a decimal POINT and continue with "RETURN". Sometimes this mode is the more convenient one. As the first thing you see a selection box for the choice of an OBJECT to be searched for.

Once you have chosen this, a second selection box appears with various search "KRITERIEN" and after that a third box with which you can enter additional OR respectively AND conditions or directly choose the OUTPUT LIST. The greatest possible versatility was aimed at, so that the operation too already requires a little practice. Some input possibilities are present twice, in order to be reachable from various objects. For example, the Häuser (cusps) H2, H3, H5, H6 can be chosen directly, since they are stored directly, while the remaining houses are only selectable under "HAUS NR.". "HERR v. HAUS.." too is accessible in two ways, in order to also be selectable from other objects (for example Halbsummen, Aspekte). If you want to work further with one of the data records of the output list, simply click on the line concerned. It is then first asked whether you want to take over the data record for further investigations. Whatever you answer here, in every case the HOROSCOPE GRAPHIC is displayed. If you answer with "JA", it is treated further like a normal RADIX data record. However, any remarks are then missing. This can also be used for the QUICK SCANNING OF MANY HOROSCOPES by clicking on the line concerned !

Here an ASPEKTE and HALBSUMMEN counter can also be switched on, as it is also applicable with ZEITWANDERN.

If a click is made in a complete list, the counter can optionally also count through the entire file automatically.

The clearest are simple questions or search operations, such as for example: Which data records have the sun in the sign Gemini, or: In which does the ruler of house 1 stand in house 10, or: In which data records does the Halbsumme SO-JU lie on the MC or on any other factor. The normal kind of OUTPUT is then a list of the data records that fulfil the search criterion, sorting being done by name.

At the top left of the screen the chosen criterion is listed once more in a small rectangle, and in a box described somewhere the number of the found data records that fulfil the search criterion.

If you search by NAMES, please use the search criterion NAME (LETTER SEQUENCE). Then the program searches all NAMES for the letter sequence concerned. If, for example, you search for all names that contain JOSEPH, JOSEF or JOSEFA, simply enter "JOSE" or "jose" or "jos". ATTENTION ! BLANK SPACES too count as a letter !

If you want to see the list of ALL NAMES of a file, enter a blank space at "NAME.." (press the space bar once).

If you are searching for a QUITE SPECIFIC DATA RECORD, search at "NAME.." and enter a part of the name of which you are sure, since every letter matters.

With "ASPEKT" as the search object a distinction is made between SINGLE aspect and searching through ALL aspects up to a certain divisor. For example, with a preselected maximum divisor of 4, a search is made for conjunction, opposition, trine and square (it is to be noted that here, with a large maximum divisor, long computing times result).

In this case the orbis, as everywhere else in HORCOM, is calculated proportionally to the basic aspect, whereby the ORBIS FACTORS are set with "VORGABEN HOROSKOP ÄNDERN". See also the remarks in ERLÄUTERUNG 4 on this.

With the input of SINGLE aspects the entered orbis is only possibly provided for SO, MO, AC with the factor 1.5, if this is preselected. The conjunction is, by the way, also designated as "Aspekt 360 Grad" ! Within the SEARCH OBJECT "ASPEKT" you can also search for aspects between single planets/houses and HALBSUMMEN. This should be important for the followers of the school of R.EBERTIN, or for the "HAMBURGERS" (and interests me myself as well). The remaining users are welcome to be annoyed by an additional query.

You can also use "STATISTIK" to list the PLANETS or other objects of the whole statistics file. For this you choose in the selection box for the search criterion "OHNE EINSCHRÄNKUNG (0....360 GRAD)". You then receive all data records, ordered by the ecliptic longitude of the preselected object.

The same you achieve, though with alphabetical sorting, by entering a blank at "NAME" and going straight to "AUSGABE". With single evaluations (without an OR or AND condition) a distribution across signs respectively HÄUSER is generally also given on the left in the picture, depending on which criterion is asked for.

With the search object "PLANETEN/HÄUSER..." there is, besides the single factors, also the possibility "ALLE PLANETEN". This is meant to serve for studying the occupation of the degrees. Here one may only enter an orbis of at most about 3 degrees, if not, an error message threatens !!. Larger orbes also make no sense, since for example already at 10 degrees almost all data records would fulfil the condition. In practice one should not go beyond an orbis of 2 degrees.

This possibility is intended above all for single evaluations and only in this case contains the full information. The planet which fulfils the condition stands as a symbol before the longitude of this planet.

It is clear that this evaluation requires a multiple of the computing time compared with a single factor !

As an intermediate stage there is also the possibility of querying SO, MO, AC together. Here, besides a degree input, the search criterion "IM ZEICHEN" can also be used.

Basically this is an OR combination (see below). For these objects sorting was dispensed with, since it would be either memory or computing time intensive.

With these latter objects it can also happen that the following message comes: "ZU VIELE, ODER ZU UNSCHARFE BEDINGUNGEN", after which you find yourself back in the menu. This has its reason in the fact that the maximum number of found data records was limited to 1.5 times the total number of data records.

For meaningful results the number of found data records must of course be only a fraction of the total number. If this message comes, it therefore means that your question is by far too imprecisely formulated. The operation becomes more difficult (and the program creation too became more difficult) when several criteria are to be investigated at the same time as an "OR combination" or "AND combination", such as for example:

Which data records have the sun in trine with a planet OR in the 9th house OR in aspect with Jupiter or in Sagittarius. That would be a kind of question as it would arise in the astrological way of viewing of LUTZ RATHKE with regard to a JUPITER dominance.

In the "MÜNCHNER RHYTHMENLEHRE" of W.DÖBEREINER, on the other hand, for example the following question could be tested:

In which data records do "hard" aspects or mirror points between SO and UR occur, OR an occupation of the degrees 22 GEMINI OR 22 SAGITTARIUS.

In the sense of the KOSMOBIOLOGIE of R.EBERTIN it might perhaps be interesting to test whether for "success" the HALBSUMME SO-MC-JU OR SO-AC-JU is significant.

Please note that "AND" combinations very quickly lead to your no longer finding anything, unless you had very large files. With a collective of a few hundred data records, for example, the search for:

Sun in Capricorn "AND" moon in Capricorn "AND" ascendant in Capricorn will hardly deliver a corresponding data record any more.

"OR" combinations, on the other hand, expand the number of found data records.

Put differently, with "AND" conditions one searches for rare (= interesting) constellations, while the "OR" combination delivers more and more banal results the more possibilities are allowed. ATTENTION ! Please do not confuse the AND with a +. The AND is a purely logical concept !

The operation of a simple "OR" query already requires that one has to click up to 12 times, since in some cases an orbis too must be entered, which necessitates an additional query.

However, during the "AND" respectively "OR" queries the file remains in RAM memory. Only when a change is to be made to another FILE must one enter again via the menu.

If you have already entered and listed some conditions and want to add yet another one, you can bring this about by entering "W" or "w", without having to enter via the menu again ! This is almost always advisable in order to approach a question STEP BY STEP. After the ending of an OUTPUT it is each time asked whether you want to evaluate further NEW OBJECTS with the loaded file. That then works directly, without new loading. Please do not confuse this with the above "W", which is only intended for new search conditions with one and the same object. With too many conditions it can happen that the provided memory space is no longer sufficient. That will, however, only happen with questions that are no longer meaningful anyway (I hope).

If several conditions apply to one and the same data record in an OR combination, this record also appears several times in the result list. The output screen contains, with composite search criteria, an additional information box on the left in the picture, in which the chosen criteria are noted once more, though without orbes (except with "ASPEKT" as the object), for the sake of better clarity. Mostly one will also choose 0 for these orbes.

At most 12 conditions (possibly in small print) can be noted. The AND conditions are limited to at most 8.

However, with 12 OR conditions one will already find almost all data records, that is, obtain a meaningless result. In practice you should work with considerably fewer conditions. With 8 AND conditions you will as a rule find nothing, unless you are searching for something already known, which is after all not the purpose of this program.

ATTENTION ! By clicking on the RIGHT mouse button the information box appears additionally horizontally in the right half of the screen and disappears again on clicking once more. Before you continue, you must make this displayed box disappear again !

The conditions are consecutively numbered in these boxes. These numbers are entered after the name as well, so that you can see which condition is fulfilled by which data record. With "AND" conditions this labelling needs more space and can shorten the name somewhat.

OR as well as AND conditions can also be used together. An AND condition therefore always refers to ALL data records that satisfy the previously entered conditions. It is therefore sensible to enter an AND condition as the last one. Otherwise the thing would also very soon no longer be possible to see through. This is also sensible insofar as an AND condition restricts the number of found data records.

SO PLEASE: ALWAYS ENTER AND CONDITIONS AS THE LAST !!

The data records which fulfil an OR condition have ONE digit at the end, namely the No. of the OR condition.

PLEASE NOTE ! : Data records that fulfil AND conditions are recognisable by the fact that after the name they contain at least 2 digits and a "u". For example "2u3u4" means that the conditions 1 OR (2 AND 3 AND 4) are fulfilled.

The data record or records that fulfil an AND condition are marked by INVERSION !

The data records which fulfil only single conditions also still appear in the output list too, if you have not chosen "UND EXKLUSIV" in the selection box (see below).

Two kinds of display with the AND combination can now be chosen:

UND INKLUSIV:

Here the AND combinations are displayed each time with all preceding OR respectively AND conditions. The "hit" or hits of the last AND condition are shown by inversion.

If you see the display "3u4" here, for example, this means that the following is fulfilled:

((1 or 2 or 3) and 4).

With several OR as well as AND conditions one already has to study a bit. The thing then very quickly becomes confusing.

UND EXKLUSIV:

Here you see in the output list only the data records that fulfil the last AND condition as well as any preceding OR respectively AND conditions.

This kind of display is suited above all for the tracking down of quite specific constellations. With this kind of display it will, however, more often happen that the message comes: "KEIN DATENSATZ ERFÜLLT ALLE BEDINGUNGEN", with which you no longer see any output at all, but come back into the object selection.

ATTENTION ! With senseless inputs such as for example SO in CP AND SO in CN one cannot expect any meaningful outputs ! It is practically impossible to exclude all senseless inputs by built-in blocks. In the most favourable case you will, with senseless inputs, be thrown back into the menu. Nonsensical displays or crashes are, however, likewise possible !!

With attempts at complex query conditions it must be well considered whether conditions are not connected among one another. Thus AC and MC are connected by law with the remaining houses, or ME, VE cannot move far away from the sun etc., UR, NE, PL are only to be expected in certain signs etc.

Unfortunately it is likely to be hardly possible to specify an "a priori probability" for every question, however fine and important this would be. If one could indeed do it, enormous computing times would also result. With complex questions that one puts to "SPEZIAL" collectives it is therefore very important that one puts the same question to a collective with "AVERAGE" cases, before one derives new "laws" in the intoxication of discovery.

In occupying oneself with OR conditions one will find that already with quite elementary questions one arrives at washed-out statements. If you search, for example, for a "Libra emphasis" in about the following form: SO in LI OR MO in LI OR AC in LI OR SO in house 7, you will perhaps establish that almost half of all data records are "Libra emphasised", which surely can no longer be very significant.

Experiments of this kind may serve to give up, little by little, all too uncritical ways of viewing, and to find or confirm the really interesting facts.

Precisely the occupation with such questions can, as I believe, help one to acquire a critical standard, which among astrologers is not to be taken for granted.

I wish that this program may in this direction have an educational effect and contribute to the seriousness of astrology.

I regard the efforts to make astrology into a "science" not exactly with approval. But within esotericism too the laws of logic and mathematics must apply, both of which are most profoundly "esoteric".
