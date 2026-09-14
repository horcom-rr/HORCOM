# Einführender Kommentar

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P), unverändert bis auf die Formatierung.*

Als legitime(r) Benutzer(in) sind Sie berechtigt,für eigene Zwecke Sicherungs-Kopien anzulegen.

Die ORIGINAL-DISKETTEN ( oder CD ) bewahren Sie wie einen Wertgegenstand auf.

Kopieen davon können Sie ( natürlich ohne Ihr Kennwort ) an andere Intereressenten weitergeben,die dann diese als Demo - Option für die folgende Zeitspannen erproben können indem sie statt des Kennwortes die ESC-Taste drücken :

100 v.Chr. bis 100 A.D.

1500 bis 1600 1800 bis 1920 1960 bis 1970 1990 bis 2010

( Die Uhr-Funktion läuft nur mit Kennwort ! ).

VERSIONEN von HORCOM:

HORCOM5P/7P sind die derzeitigen,für den IBM-kompatiblen PC zur Verfügung stehenden Versionen.

Hardware-Voraussetzungen :

486 DX Prozessor (erweitert) oder höher. 4 MBYTE Arbeitsspeicher und 20 MBYTE oder mehr freien Speicher auf der HARD-DISK.

Ein schneller Prozessor ist nie ein Nachteil.

Software-Voraussetzung :

DOS... und WINDOWS 3.X ( oder höher )

WINDOWS 95,WINDOWS 98,WINDOWS NT,WINDOWS XP

Die geeignetste Bildschirm-Auflösung ist das VGA-Format mit 1074*768 Bildpunkten und 256 Farben oder monochrom.

Höhere Farb-Auflösungen werden von HORCOM nicht benötigt.

Hardcopys können Sie von jeder Bildschirmauflösung aus ausdrucken,die hochauflösenden Drucker-Graphiken von Horoskopen sind von der Bildschirmdarstellung unabhängig bzw. arbeiten mit der höheren Auflösung des Druckers.

Die für die Programmerstellung verwendete Software ist:

```text
   GFA-BSIC für WINDOWS 3.1 , PROFIVERSION RELEASE 4.36 von GFA-
   SYSTEM - TECHNIK Mönchen-Gladbach.
```

```text
   Daneben wurden auch direkt API-Funktionen von WINDOWS 3.1
   eingesetzt.
```

Die DLL-Dateien sind nicht von mir sondern aus WINDOWS bzw. GFA-BASIC um die Lauffähigkeit von HORCOM sicherzustellen.

```text
             Dieses Programm enthaelt Softwarecode ( Planeten
             SO,MO,ME,VE,MA,JU,SA,UR,NE )
             Copyright (c) 1991-1992 by Jeffrey Sax
             und verteilt durch Willmann-Bell, Inc.  Serie 10756
```

Danksagungen :

Herrn FRANK OSTROWSKI und den Mannen von GFA-SYSTEMTECHNIK danke ich für das GFA-BASIC,und für Support bei der PC-Version.

Den Herren A.BUNKAHLE,B.MAHL,L.RATHKE,PH.SCHIFFMANN,K.STAMER, M.H. WESEMANN,DR.H.WISGRILL,I.HAHN-ROSTOCK,J.HUBER und H.PHILIPP danke ich für nützliche Hinweise,Überlassung von Literatur sowie Hilfe beim Testen und Fehlersuchen, meist schon bei der ATARI-Version. Hr.B.MAHL hat wichtige Anregung und Unterstützung gegeben bei der Erstellung der PRIMÄR-Direktionen,der "KORREKTUR mit PRIMÄR DIRIGIERTEN ACHSEN" und den "ARABISCHEN TEILEN".

Den Herren B.MAHL und DR.H.WISGRILL danke ich für ihre ORTS-DATEIEN,in denen viel Arbeit steckt.Sie stellen sie allen HORCOM - Usern zur Verfügung.

Frau M.-L. BORKERT danke ich für wichtige Anregungen bei der vorliegenden Version für WINDOWS und für wertvolle Hilfe bei der Fehlersuche. Da ich selbst nur gelegentlich ( für den Hausgebrauch ) astrologisch arbeite,verdankt das Programm seine Reife zum guten Teil der Mitarbeit oben genannter Astrologen und noch einiger nicht genannten. Leider konnte ich bisher bei weitem nicht allen Anregungen nachgehen, da ich das Programm nur als Rentner-Hobby betreibe( maximal 4 Stunden täglich ).Aber was nicht ist kann ja zum Teil noch werden.

Man möge mir auch nachsehen,daß ich nur solchen Anregungen nachgehen kann die von mehreren Seiten kommen und auch mir berechtigt erscheinen. Auch muß ich darauf achten,daß die Bedienung nicht durch Überladung mit Auswahlmöglichkeiten zu problematisch wird.

## VORBEMERKUNGEN

* Eine gewisse Übung mit der Bedienung von WINDOWS wird vorausgesetzt.

```text
   Zumindest sollten Sie das WINDOWS-LERNPROGRAMM ( unter "HILFE"
   erreichbar) einigemale durchgehen.
   HORCOM verwendet nicht alle Bedien-Elemente und nur sehr einfache
   DIALOG - Boxen.Aber wie man ein Fenster schließt,wieder öffnet,in
   der Größe und Lage verändert,sollten Sie beherrschen.Ebenso die
   Bedienung der SCROLLBALKEN.
   Auch die Bedienung des DATEI-MANAGERS,der SYSTEM-STEUERUNG und des
   WINDOWS-SETUP aus der HAUPTGRUPPE von WINDOWS 3.X ist immer wieder
   erforderlich auch für den fortgeschrittenen Umgang mit HORCOM.
   Gelegentlich auch der DRUCK-MANAGER.
```

```text
   Das Programm WINDOWS\CALC (ULATOR) = Taschenrechner ist aus jedem
   Ergebnisbildschirm von HORCOM direkt über die Funktionstaste F3 direkt
   erreichbar.
   Mit der Funktionstaste F1 erreichen Sie immer die zuständige
   Erläuterung.
   Die Funktionstaste F8 schaltet die DRUCKER-OPTION EIN bzw. AUS.
```

* Wer HORCOM schon vom ATARI kennt,wird sicher kaum Schwierigkeiten

```text
   haben.
   Insbesondere habe ich die grundsätzliche Art der Bedienung
   beibehalten,nämlich,daß der Ausgabebildschirm im VGA-Format immer
   den ganzen Bildschirm einnimmt und man beim Verlassen eines
   Ergebnisses i.a. automatisch ins Hauptmenü mit seiner Parameter-
   Liste gelangt.
   Falls bei den HORCOM – Ausgaben-Bildschirmen unten die TASKLEISTE
   sichtbar ist bitte unter SYSTEMSTEUERUNG > TASKLEISTE..  die
   Einstellung „TASKLEISTE IMMER IM VORDERGRUND“ deaktivieren.
```

```text
   Was man als Umsteiger vom ATARI vor allem sich angewöhnen muß ist
   die Benutzung der TABULATOR-Taste,mit der man immer in den
   DIALOGEN von einem "ITEM" zum anderen hüpfen kann,in Gegenrichtung
   mit SHIFT+TAB.
   Falls ein DIALOG erscheint und nichts geschieht,kann es nicht
   schaden auf TAB zu drücken ( linke Seite der Tastatur mit 2
   gegenläufigen Pfeilen bezeichnete Taste ).
   Meist kann man auch mit den vertikalen PFEIL-Tasten springen.
   Beim EDITIEREN muß man sich erheblich umgewöhnen.
   Ferner ist es gut zu wissen,daß man die Titelzeile eines MENÜS
   auch mit der Funtionstaste F10 oder mit ALT erreicht und mit den
   Pfeiltasten abgreifen kann,falls der Mauszeiger aus irgend einem
   Grund verschwunden oder eingeengt ist.
```

* DATEN- Dateien vom ATARI ST können auf den PC wie folgt

```text
   übertragen werden.
   -Die Ordner-Namen und Systemdatei-Namen der ATARI-Version wurden
   im wesentlichen beibehalten.-
```

```text
   1. Eine ATARI-Diskette ( Double Density ) auf dem PC mit 720 KBYTE
      formatieren.
```

```text
   2. Die betreffenden Dateien auf dem ATARI ST darauf kopieren und
      an die entsprechende Stelle im HORCOM - Ordner des PC kopieren:
      DATEN-Dateien ( Endung .DAT ) in den Ordner \HORCOM\SPEZIAL
      ORTS -DATEIEN mit der Endung .INT  in den Ordner HORCOM
      \SPEZ_ORT.
      STATISTIK-Dateien  müßen neu erstellt werden,was mit einem
      modernen Prozessor recht schnell geht.Diese Dateien werden im
      Ordner  \HORCOM\STATIST. abgelegt.
```

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

GRUNDSÄTZLICHES zur BEDIENUNG:

Die BEDIENUNG Ihres Computers auf der WINDOWS-Ebene sollten Sie einigermaßen beherrschen ! -Mit allen Tricks sind nur wenige vertraut zu denen auch ich mich nicht zählen kann.

Sie müssen sich von vornherein darüber klar sein,daß eine Menge Kommunikation zwischen Ihnen und HORCOM nötig ist,damit genau das erfolgt,was Sie wünschen.

Diese Kommunikation erfolgt über folgende Elemente:

```text
  1. Das HAUPT-MENÜ
  2. Die DIALOGE und Untermenüs
  3. Die FUNKTIONSTASTEN F1,F3,F5,F8,F9
  4. Die Bildüberschriften
  5. Gelegentliche TASTATUR-Eingaben,wenn EINGABEN in DIALOGEN
     verlangt werden,oder während des Ablaufs Eingriffe vorgenommen
     werden sollen.
```

Der Programm-START benötigt einige Sekunden,je nachdem wie schnell Ihr Computer ist.Mit heute üblichen Ausrüstungen gibt es kaum noch Wartezeiten.

Nach Starten des Programms erscheint zunächst oben im Bild die Aufforderung Ihr KENNWORT ( verdeckt OHNE RETURN !) einzugeben. Falls Sie das Kennwort noch nicht kennen drücken Sie dort die ESC-Taste.

Dabei können Sie alles in folgenden Zeitspannen erproben :

```text
   100 v.Chr bis 100 A.D.
   1500 bis 1600
   1800 bis 1920
```

Nun erscheint nach einigem Warten der Titel ...HORCOM5P/7P HAUPTMENÜ.... Darunter die Zeile :

EINFÜHRUNG | EIN-AUSGABE| EPHEMERIDE | HOROSKOPE | AUSWERTUNG | DIVERSES

Drücken Sie nun die RECHTE Maustaste,so wird ein Text sichtbar,der die grundlegende Bedienungsweise von HORCOM darstellt,den Sie lesen sollten.

Wenn Sie eine der obigen Gruppen anklicken,entrollt sich jeweils eine Liste von Programm-Titeln.

Schauen Sie sich die einzelnen Titel anfangs gut an,damit Sie sich einprägen wo was zu finden ist.

Richten Sie nun den Mauszeiger auf den gewünschten Programm-Titel und drücken Sie die linke Maus-Taste (="Anklicken" bzw.Markieren").Fangen Sie mit den ERLÄUTERUNGEN an.Diese sind immer verfügbar und sollten wiederholt studiert werden.Sie beziehen sich jeweils auf die DARÜBER in der betreffenden Kolonne stehenden Programme.Mit der Funtionstaste F1 erreichen Sie aus jedem Ergebnis-Bildschirm die "zuständige" Erläuterung.

Sind Menü-Titel DEAKTIVIERT geschrieben,handelt es sich entweder um bloße Überschriften oder um momentan nicht aktivierbare Programme, z.B.vor der Eingabe.

Menü-Titel mit vorangestellten * sind HILFS-ROUTINEN,die mit anderen Programmen nicht zusammenarbeiten.Sie können auch zwischendurch oder vorab angewendet werden.

Die BEENDIGUNG eines Programms wird jeweils durch einen 3-FACH-Signalton angezeigt.

Ein EINFACH-Signalton deutet an,daß es sich um ein ZWISCHEN-ERGEBNIS handelt.Daraus kommen Sie mit LEERTASTE (= BLANK) oder BILD nach UNTEN / OBEN - Taste weiter,bei Listen auch mit "R" zum vorangehenden Bildschirm.

Durch Doppelklick auf das SCHLIEßFELD links oben wird das betreffende Unterprogramm bedingungslos beendet.

Ausserdem auch über die ESC-Taste,evtl. noch nach Zwischenschritten.

Vom Endergebnis aus auch mit 2 * BLANK - Taste.

Wenn Sie dem Computer neue Anweisungen geben wollen,MÜSSEN Sie i.a. wieder ins Menü gehen,da der Menü-Bildschirm Ihre "KOMMANDO-EBENE" ist.

Normalerweise wird daher beim Verlassen eines Ergebnis-Bildschirms automatisch das HORCOM-HAUPTMENÜ sichtbar.Falls das nicht der Fall sein sollte,die TAB-Taste oder ESC-Taste drücken.

Danach ist zunächst nur die Kopfzeile zu sehen.Erst wenn Sie mit der Maus auf ein Wort der Menüzeile klicken,z.B. HOROSKOPE,wird die jeweilige Liste entrollt.

Bei dem Menü-Bildschirm handelt es sich um ein "WINDOWS"-Fenster. Ebenso bei Ergebnissen.

Im allgemeinen beginnt man mit der EINGABE DER DATEN für ORT und ZEIT:

Entweder NEU-EINGABE oder Eingabe aus der Datei,d.h.betreffende Zeile anklicken.Bitte dazu auch unbedingt ERLÄUTERUNG 2 lesen !!

Nun können Sie z.B. HOROSKOP-GRAPHIK mit der Maus in der betreffenden Zeile anklicken.Dazu den Maus-Pfeil über diese Zeile schieben und die LINKE Maustaste drücken.Die Sanduhr zeigt an daß der Computer arbeitet.

Zu eventuell nötigen Eingabe-PARAMETERN werden Sie jeweils mit "DIALOGEN" abgefragt.Sie antworten in den meisten Fällen mit der Maus, indem Sie das gewünschte "ABFRAGE-KäSTCHEN" = "BUTTON" anklicken. Die DIALOGE sind sehr einfach gehalten.In der Regel müssen Sie immer nur eine Frage beantworten bzw. eine Eingabe bedienen und zwar unbedingt.Sie kommen aus einem HORCOM-DIALOG nur heraus indem Sie ihn beantworten.

Lesen Sie zu Beginn die Dialog-Texte sorgfältig bevor Sie klicken. Für den ANFÄNGER ist es empfehlenswert,zuerst die DICK UMRANDETEN "BUTTONS" ( Kästchen ) zu bedienen.Dazu genügt,außer bei Dialogen mit Editierfeldern,oder Texten das Drücken der RETURN-Taste oder,bei HORCOM-Dialogen,meist auch die LEERTASTE.

Sonstige Antwort-Felder klicken Sie mit der Maus an.

In vielen Fällen enthält eines der Abfrage-Kästchen die Bezeichnung "ZURÜCK zum MENÜ","ABBRUCH" oder "EXIT".Diese Wege bilden "Kurzschlüsse" zum MENÜ - Bildschirm.

Oft braucht HORCOM die Eingabe von Zahlen-Werten oder Worten ( Zeichenfolgen = STRINGS ).Dies geschieht in "EDITIER-FELDERN", die durch einen blinkenden "CURSOR" auffallen.

Normalerweise können Sie hier Groß-oder Kleinbuchstaben eingeben. Bei Ziffern mit Dezimalen ( selten erforderlich ) können Sie KOMMA oder PUNKT verwenden ( hoffe ich ).

Neben den ALPHANUM. Tasten benötigen Sie für die "Bedienung" noch die RETURN- oder ENTER-Taste oder die LEER-TASTE (zum Weiterschalten),bei einigen Programmen evtl. auch die ESC-Taste (zum vorzeitigen Aussteigen), bei Tabellen-Serien auch BILD mit Pfeil nach oben oder unten oder "R" für Rückwärts oder "V" für Vorwärts.

Falls Sie an einen Bildschirm oder DIALOG geraten bei dem sich nichts tut,kann es nicht schaden,die TAB-Taste zu drücken.

Falls ein Programm hängen bleibt,hilft oft die ESC-Taste (links oben) Dann öfters drücken.Wenn das nichts hilft,STRG + PAUSE und dann HORCOM beenden.

Wenn gar nichts mehr geht,hilft (fast ) immer STRG + ALT + ENTF ,was einen Neustart des Computers bewirkt.

Bitte auch die TITEL-ZEILE jeweils lesen,da diese oft Informationen enthält.

Die Belegung der FUNKTIONS-TASTEN ist folgende:

```text
  F1  (ALT+E)  Zuständiger KOMMENTAR
  F2  (ALT+A)  Anzeigen des HOROSKOPS aus sonstigen Ausgaben heraus
  F3  (ALT+C)  WINDOWS-RECHNER
  F5  (ALT+R)  WINKEL-UMRECHNUNG DEZIMAL in GRAD/MIN/SEK oder H/MIN/SEK
  F6  (ALT+H)  Umschalten zwischen HELIO- und GEOZENTRISCHEM Modus
  F7  (ALT+F)  MSPAINT öffnen um evtl. Bilder vom BMP in GIF umzuwandeln
  F8  (ALT+Z)  Schaltet die DRUCKER-OPTION EIN oder AUS,ebenso die Tasten-
               Kombination ALT +"D"
  F9  (ALT+M)  Steuert eine DOPPEL-AUSGABE (2 HORCOM-ERGEBNIS-Bildschirme
               auf einer DINA4-Seite ausdrucken)
```

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

Für die Rubriken HOROSKOP und AUSWERTUNG gilt noch folgendes: Ist vor einem bearbeiteten Menüpunkt ein HÄKCHEN gesetzt,so ist der betreffende BILDSCHIRM GESPEICHERT.Er kann dann sofort,oder wenn später der zugehörige Datensatz wieder aktiviert ist,wahlweise ZURÜCKGEHOLT werden.

ALSO MERKE: WO unter "HOROSKOPE" oder "AUSWERTUNG" ein HÄKCHEN, IST AUCH EIN BILDSCHIRM ZU HOLEN !

Diese Bildschirm-Speicherung auf die Programme beschränkt,die besonders wichtig sind ( diese sind mit einem | bezeichnet ) und tritt auch nur dann in Aktion wenn die Rechenzeit merklich ist und wenn sie nicht unter "VORGABEN HOROSKOP ÄNDERN" abgeschaltet wurde. Es werden nur die Bildschirme gespeichert,die mehr als 15 sek zum Aufbau benötigen,was mit modernen Prozessoren schon selten ist.

Beim Verlassen von HORCOM bzw. beim Neustart werden alle in der letzten Sitzung evtl. gespeicherten Bildschirme wieder gelöscht.

Der jeweils LETZTE Ergebnis-Bildschirm kann daneben immer bei " LETZTES BILD ZEIGEN.." zurückgeholt werden,solange er eben noch nicht überschrieben ist.

Dort können auch BILDER auf DISC mit NAMEN bleibend abgespeichert werden (siehe auch Erläuterung 2).

Im unteren Teil des Menü-Bildschirmes sind wichtige AKTUELLE DATEN und PARAMETER-ZUSTÄNDE jeweils abzulesen.Diese Parameter werden meist auf Diskette GESPEICHERT in der DATEI \HORCOM\INTERN\KONSTA5P/7P.INT. Darum brauchen Sie sich aber nicht zu kümmern.

Außerdem ist der AKTUELLE Eingabesatz durch ein HÄKCHEN markiert und kann durch Anklicken angezeigt werden und ist danach AKTUELL.

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

Wenn Sie sich schnell in die Bedienung einüben wollen,ist es sehr zweckmäßig,wenn Sie zunächst 2 Datensätze eingeben und dann für beide HOROSKOP-GRAPHIK sowohl für Radix als auch Solar anwählen.Üben Sie dann das "Anklicken" unter DATEI EIN-AUS um zwischen Solar und Radix hin und herzuschalten.

Beobachten Sie dabei,welche Programme jeweils zugänglich (=aktiviert) bleiben.

Danach lesen Sie nochmals genau die Erläuterungen 1 u.2 und wagen sich an weitere Unternehmungen.

Das oftmalige Lesen der Erläuterungen in den ersten Tagen und auch später immer wieder einmal,wird dringend empfohlen !!!

Ich wünsche den Anwendern dieses Programms viel Freude damit und hoffe,daß es zur Klärung offener Fragen beiträgt.
