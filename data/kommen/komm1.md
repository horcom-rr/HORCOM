# Einführender Kommentar

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P). Wortlaut unverändert, weggelassen sind technische Abschnitte zur Installation, Tastatur- und Druckersteuerung der historischen Programmfassung.*

Die für die Programmerstellung verwendete Software ist:

GFA-BSIC für WINDOWS 3.1 , PROFIVERSION RELEASE 4.36 von GFA-SYSTEM - TECHNIK Mönchen-Gladbach.

Daneben wurden auch direkt API-Funktionen von WINDOWS 3.1 eingesetzt.

Dieses Programm enthaelt Softwarecode ( Planeten SO,MO,ME,VE,MA,JU,SA,UR,NE ) Copyright (c) 1991-1992 by Jeffrey Sax und verteilt durch Willmann-Bell, Inc.  Serie 10756

Danksagungen :

Herrn FRANK OSTROWSKI und den Mannen von GFA-SYSTEMTECHNIK danke ich für das GFA-BASIC,und für Support bei der PC-Version.

Den Herren A.BUNKAHLE,B.MAHL,L.RATHKE,PH.SCHIFFMANN,K.STAMER, M.H. WESEMANN,DR.H.WISGRILL,I.HAHN-ROSTOCK,J.HUBER und H.PHILIPP danke ich für nützliche Hinweise,Überlassung von Literatur sowie Hilfe beim Testen und Fehlersuchen, meist schon bei der ATARI-Version. Hr.B.MAHL hat wichtige Anregung und Unterstützung gegeben bei der Erstellung der PRIMÄR-Direktionen,der "KORREKTUR mit PRIMÄR DIRIGIERTEN ACHSEN" und den "ARABISCHEN TEILEN".

Den Herren B.MAHL und DR.H.WISGRILL danke ich für ihre ORTS-DATEIEN,in denen viel Arbeit steckt.Sie stellen sie allen HORCOM - Usern zur Verfügung.

Frau M.-L. BORKERT danke ich für wichtige Anregungen bei der vorliegenden Version für WINDOWS und für wertvolle Hilfe bei der Fehlersuche. Da ich selbst nur gelegentlich ( für den Hausgebrauch ) astrologisch arbeite,verdankt das Programm seine Reife zum guten Teil der Mitarbeit oben genannter Astrologen und noch einiger nicht genannten. Leider konnte ich bisher bei weitem nicht allen Anregungen nachgehen, da ich das Programm nur als Rentner-Hobby betreibe( maximal 4 Stunden täglich ).Aber was nicht ist kann ja zum Teil noch werden.

Man möge mir auch nachsehen,daß ich nur solchen Anregungen nachgehen kann die von mehreren Seiten kommen und auch mir berechtigt erscheinen. Auch muß ich darauf achten,daß die Bedienung nicht durch Überladung mit Auswahlmöglichkeiten zu problematisch wird.

Einiges zum Inhalt von HORCOM :

Hinter den Menütiteln des Hauptmenüs verbergen sich oft ganze Gruppen von Programmen.Z.B. bei "STATISTIK" oder "MÜNCHNER RHYTHMENLEHERE".

ERLÄUTERUNGEN sind im Programm enthalten.Die Texte in den DIALOG-BOXEN sind als Ergänzung dazu zu betrachten !

Abruf der Erläterungen entweder aus dem Menü oder aus dem Ergebnisbildschirm mit der Taste F1 ( ruft die jeweils zuständige Erläuterung auf ).

Für den Dialog mit dem Benutzer wurde sehr viel Aufwand getrieben. Fast die Hälfte des Programm-Textes dürfte auf Erläuterungen und Dialogtexte entfallen.

Wollen Sie Erläuterungen AUSDRUCKEN klicken Sie bei "DRUCKEN" an. Es wird dann mit einem vorgegebenen Font ausgedruckt.Die Umlaute werden nur dann richtig gebracht,wenn der Drucker auf den deutschen Zeichensatz eingestellt ist.

Wollen Sie den Ausdruck selbst gestalten,können Sie auch mit WRITE bzw.WORDPAD oder dem WINDOWS-EDITOR arbeiten.Sie müssen nur die Textdateien in dem Ordner \HORCOM\KOMMEN5\.....TXT öffnen.

Die Positionen der großen Planeten ME,VE,ERDE ( SO ),MA,JU,SA,UR,NE werden nach den von JEAN MEEUS zur Verfügung gestellten analytischen Formeln berechnet:

JEAN MEEUS ( "ASTRONOMISCHE ALGORITHMEN" , JOHANN AMBROSIUS BARTH- VERLAG ISBN 3-335-00318-7 )

Diese Routinen sind für 8 Jahrtausende ( 2000.0 + - 4000 Jahre ) so genau,wie man es sich als Astrologe nur wünschen kann.Sie entstammen einer abgekürzten Theorie VSOP87 des INSTITUT DES LONGITUDES ( PARIS, P.BRETAGNON G.FRANCOU ).Dank an die Astronomen !

Auf Grund dieser Daten werden auch die MOMENTANWERTE ( auch "WAHRE WERTE" genannt ) des MONDKNOTENS und MOND-APOGÄUMS ( = SCHWARZER MOND ) innerhalb HORCOM berechnet.

SOLARE,SEPTARE,LUNARE,PLANETARE und PERSONARE ( P.ORBAN,I.ZINNEL ) sind enthalten,ebenso wie einige der meist verwendeten DIREKTIONEN und natürlich TRANSITE,die in unterschiedlichen Darstellungsarten angezeigt werden können.

Die "Multiplen Direktionen" von ST.LEHRIEDER sind ebenfalls enthalten. ( siehe Erläuterung 4 ).

Einen großen Raum nimmt die "MÜNCHNER RHYTHMENLEHRE" meines astrologischen Lehrers W.DÖBEREINER ein,die ich hoch schätze.Im folgenden auch als M.R. abgekürzt ! Ihm verdanke ich den Zugang zur Astrologie. ( siehe Erläuterung 6 )

Auch "MUNDAN"-Horoskope ( = topozentrisch-äquatoriale Horoskope in einem Horizont-System )können für die M.R. herangezogen werden. Das hierbei benutzte Koordinatensystem mit Zeitmessung=Raummessung auf dem Äquator ist dem Sinn der M.R. besser angepaßt als die Arbeit mit dem geozentrischen,ekliptikalen Horoskop.Darauf machte E.R.DOSTAL erstmals aufmerksam ( siehe Erläuterung 4 ).

Außerdem wurde eine weitere Auswertung in Form einer GRAD-DATUM-Liste für die M.R. hinzugefügt.

Daneben sind HALBSUMMEN verwendet,etwa in der Art der Schule R.EBERTIN bzw.der "HAMBURGER SCHULE" (Auch als eigene Graphik). Auch die Programme SEKUNDÄR- u. SONNENBOGEN-DIREKTION sind durch Anregungen aus Büchern von R.EBERTIN entstanden.

Neben den normalen Sekundärdirektionen können auch "Dynamogramme" nach KRAFFT-GOERNER erstellt werden.Diese Variante ist mir von Hrn. JOHANN HUBER ans Herz gelegt worden ( siehe Erläuterung 7 ).

Die PRIMÄR-DIREKTIONEN sind nun nach dem Buch von E.C.KÜHR "BERECHNUNG DER EREIGNISZEITEN" programmiert.Alle Planeten, Zwischenhäuser und Kardinalpunkte können als Signifikatoren und Promissoren verwendet werden.

Die Promissoren können auch MIT BREITE berechnet werden ( siehe Erläuterung 7 ).

In der ORBIS-Frage habe ich mich von J.ADDEY u. H.J.WALTER anregen lassen,für die DIREKTIONEN neben E.C.KüHR von R.EBERTIN und C.O.E.CARTER.

Die KLEINPLANETEN Chiron,Ceres,Pallas,Juno,Vesta sind enthalten. (siehe auch Erläuterung 3).

Die 8 hypothetischen "Planeten" der HAMBURGER SCHULE können ebenfalls gewählt werden (siehe auch Erläuterung 3).

Der hypothetische TRANS-PLUTO ( auch Isis genannt ) nach den von EMILE SEVIN berechneten und auch N.F.MICHELSEN in seiner Ephemeride genannten vorläufigen Bahnelementen ist ebenfalls aufgenommen (siehe auch Erläuterung 3).

Außer dem üblichen MITTLEREN MONDKNOTEN wird auch der WAHRE (= MOMENTANE) Wert wahlweise berechnet (siehe auch Erläuterung 3).

Auch die MITTLERE oder MOMENTANE (=WAHRE) APSIDENLINIE der Mond-Bahn ( = 'SCHWARZER MOND') kann unter 'KLEINPLANETEN' hinzugewählt werden. Dieser 'Schwarze Mond' erfreut sich in Deutschland erst in jüngster Zeit einiger Beachtung,wahrscheinlich weil es bisher keine Ephemeride für den den WAHREN Wert gibt,der vom mittleren um mehr als 20 Grad abweichen kann.Bisher wird offenbar fast ausschließlich mit dem mittleren Wert gearbeitet.

Die mir bisher bekannt gewordene Deutung als SEXUALITÄTS-ACHSE (APOGÄUM = SCHWARZER MOND = WEIBLICH und PERIGÄUM = "PRIAP" = MÄNNLICH habe ich bisher insofern bestätigt gefunden,als diese Achse,wenn sie im engen Aspekt mit Radix-Faktoren wie MC,AC,SO,MO, Geburts-Herrscher steht,als ENERGIE-FAKTOR wirksam zu sein scheint.Z.B. gewinnt Goethes Horoskop damit sehr an Überzeugungskraft,da der schwarze Mond in engster Konjunktion mit MC und Sonne steht.

Derartige Konstellationen scheinen geradezu ein Merkmal für bedeutende Menschen zu sein.Ein Radix-Faktor der mit dem Schw.Mond in Konjunktion oder Opposition steht,gewinnt dadurch an Durchschlags-kraft bzw. wird dominant.

Für die düsteren Interpretationen des SCH.M. habe ich bisher keine Anhaltspunkte finden können.

Übrigens sind die Werte soweit brauchbar wie die Mond-Längen,d.h. über mehrere Jahrtausende.

Näheres dazu,siehe Erl.3.Literatur:L.MILLAT,R.DAUTREMONT,M.DUVAL 'LUNE NOIRE',EDITIONS TRADITIONELLES QUAI SAINT MICHEL - PARIS / 1983 (siehe auch Erläuterung 3).

FIXSTERN-POSITIONEN sowie ARABISCHE TEILE sind als Tabellen mit ASPEKT-GRAPHIK abrufbar.Bei den letzteren können alle Punkte auch selbst definiert und auf Diskette gespeichert werden (siehe auch Erläuterung 3).

HORCOM5P/7P verfügt ferner über das Modul "STATISTIK" ,das sich im Hauptmenü recht unscheinbar darstellt,aber eine ganze Menge zu bieten hat.Man kann damit recht detaillierte Fragen statistisch unter-suchen. Dies ist ein für den forschenden Astrologen besonders wichtiges Programm-Packet.

Daneben hat HORCOM5P/7P zusätzlich folgende Programme :

```text
AUFGANG...    ( Aufgang,Meridian-Durchgang und Untergang der
               Planeten ).
FINSTERNISSE..( Sonnen- und Mondfinsternisse evtl. mit Aspekten zum
               aktuellen Datensatz,sowie NEUMOND und VOLLMOND-
               Zeitpunkte ).
INGRESSE      ( Berechnet die Ingresse von Sonne oder Mond in die 12
                Tierkreiszeichen ).
```

"VERBOTE" wurden nur wenige eingebaut.Welche Anwendung Sinn macht, sollen Sie entscheiden.Der Experimentierlust sollen keine Grenzen gesetzt sein.

Die VERBOTE können Sie übrigens dadurch umgehen,daß Sie z.B ein SOLAR usw.als RADIX eingeben und so z.B.ein SOLAR des LUNARS chen,wenn es denn sein muß.Dazu dient "ERGEBNIS ALS RADIX". Machen Sie davon nur mit Vorsicht Gebrauch.

Der reichliche Gebrauch von "DIALOG-BOXEN" und Unter-Menüs gewährleistet in erheblichem Maß eine Bedienerführung,ohne die Variabilität unzulässig einzuschränken.

Um eine Ahnung über die Möglichkeiten von HORCOM zu haben,müssen Sie schon viele Stunden damit umgegangen sein.

Inhaltliche Änderungen bleiben vorbehalten.

Für Fehlerfreiheit wird keine Gewähr übernommen.Falls Sie Fehler oder Bedienungsmängel feststellen,bitte ich um gelegentliche Mitteilung. Fehler werden bei dem nächstfälligen UPDATE beseitigt.

Auch Konstruktive Kritik wird,wenn auch zähneknirschend,begrüßt. Verbesserungsvorschläge werden nur dann berücksichtigt wenn sie von mehreren Anwendern für wünschenswert befunden werden und auch mir ins Konzept passen.

GRAD-ANGABEN wie 123.45 sind immer als DEZIMAL-Grade zu verstehen. Gelegentlich sind aus räumlichen Gründen die Minuten oder Sekunden weggelassen.So heißt 12 Grad 33 : 12 Grad 33 Min.

Für Tierkreiszeichen und Planeten werden entweder die Symbole oder die gebräuchlichen Abkürzungen verwendet.

ABKÜRZUNGEN sind :

TIERKREIS :

```text
AR    TA    GM    CN    LE    VI    LI    SC    SG    CP    AQ    PS
```

PLANETEN :

```text
SO  ( TE )  MO    ME    VE    MA    JU    SA    UR    NE    PL    DR
DS
```

TE = Erde | MO = Mond | DR = Aufsteigender , DS = Absteigender Mondknoten.

12 VI 23 bedeutet 12 Grad 23 Min Jungfrau. SO=MA-SA heißt: Sonnne in der Halbsumme von Mars u. Saturn.

Nach Eingabe der Daten ist i.a. keine weitere Vorbereitung mehr nötig um unter EPHEMERIDE,HOROSKOPE,AUSWERTUNG,DIVERSES anzuklicken,außer daß bei DOPPEL Horoskopen 2 Datensätze vorhanden sein sollten (siehe auch Erläuterung 4).

Steht hinter dem Programm-Namen ein G/H ,so ist dieses Programm sowohl GEOZENTRISCH als HELIOZENTRISCH anwendbar.Da die heliozentrische Version in der Astrologie "exotisch" ist,wurde auf eine ständige Abfrage verzichtet.

Die heliozentrische Version kann unter EPHEMERIDE ein-oder ausgeschaltet werden oder auch mit der Funktionstaste F6.

Sie ist durchaus auch für Astrologen von Interesse !

Siehe dazu die statistischen Forschungen von Dr.SIEGFRIED SCHIEMENZ,z.B: "Planetenstellungen und der Geist des Menschen".

Das HÄUSER-SYSTEM wird unter "DIVERSES" gewählt.

Der Anfänger mit HORCOM sollte vor allem folgendes beachten:

HORCOM verwendet DATENSÄTZE in verschiedenen "EBENEN".

Man beginnt auf jeden Fall in der RADIX - EBENE,d.h. jeder neu eingegebene oder aus einer DATEN- oder STATISTIK-Datei geholte Datensatz ist zunächst immer als RADIX gültig.

Hat man nun ein SOLAR,LUNAR oder PLANETAR gemacht so befindet man sich in der betr. SOLAR-,LUNAR usw. EBENE und es sind nicht mehr alle HORCOM-Programme ( aber durchaus noch einige ) zugänglich. Will man nun etwas neues mit dem betreffenden RADIX-Datensatz machen,muß man unter "EIN-AUSGABE" diesen erst wieder anklicken. Der letzte SOLAR- usw. Datensatz bleibt trotzdem erhalten und kann,solange der zugehörige RADIX-Datensatz vorhanden ist,jederzeit wieder aktiviert werden,womit man wieder in der SOLAR- usw. EBENE ist.

Entsprechendes gilt für die DOPPEL-HOROSKOPE (COMPOSIT,COMBIN usw.). Sie bilden ebenfalls eine besondere Ebene.

Bei Tabellen-Ausgaben können die "problematischen" Planeten MA,SA,UR,NE,PL invers dargestellt,was auch der graphischen Auflockerung der Tabellen dient.

Das kann unter VORGABEN DIREKTIONEN ÄNDERN ein- oder ausgeschaltet werden.

Das oftmalige Lesen der Erläuterungen in den ersten Tagen und auch später immer wieder einmal,wird dringend empfohlen !!!

Ich wünsche den Anwendern dieses Programms viel Freude damit und hoffe,daß es zur Klärung offener Fragen beiträgt.
