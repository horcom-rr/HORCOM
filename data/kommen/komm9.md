# Erläuterung Diverses

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P), unverändert bis auf die Formatierung.*

## KORREKTUR

Dient vor allem der Korrektur der Geburtszeit.Es wird die normale Eingabe-Routine verwendet,wobei mindestens die Ortskoordinaten und das Tages-Datum einzugeben sind.Man kann dann entweder mit der STERNZEIT,dem MC,AC,mit ZWISCHENHÄUSERN,mit SONNE oder MOND korrigieren.

Das Ergebnis ÜBERSCHREIBT die ursprüngliche Eingabe !! und kann unmittelbar weiter verwendet werden., Mit "KORREKTUR" kann man auch bei einem gegebenen Horoskop mit unbekannter Geburtszeit und bekanntem AC od. MC die UT wie folgt ermitteln: Mit "NEU-EINGABE" Geogr.Koord.,Datum und irgendeine UT eingeben und mit AC oder MC abändern.Man braucht nur entweder AC oder MC.

Für den Praktiker ist dies eines der wichtigsten Programme.

Die KORREKTUR mit "PRIMÄR DIRIGIERTEN ACHSEN" ist eine HILFS-ROUTINE für ein bewährtes Korrekturverfahren.Es arbeitet nach der "Gleichung" :

```text
           Ein TäGLICHER STERNZEIT-FORTSCHRITT = 1 LEBENSJAHR
```

Es gelten die Beziehungen :,

```text
    1 JAHR  = 0.98565 GRAD          ARMC   = 0 h  3 m  56.56 s  STERNZEIT
    1 MONAT = 4.928   BOGEN-MIN      "     = 0 h  0 m  19.71 s       "
    1 TAG   = 9.7     BOGEN-SEK.     "     = 0 h  0 m   0.65 s       "
```

```text
    1 GRAD  Sternzeit ( = ARMC )  entspricht   4 Zeitminuten Korrektur am Radix.
```

Die RADIX-PLANETEN bleiben dabei normalerweise UNVERÄNDERT.Man kann aber auch mit den EREIGNIS-Planeten experimentieren.

Im übrigen werden nur die HÄUSER nach der Progression vorwärts ="DIREKT" oder rückwärts = "KONVERS" gedreht = "DIRIGIERT".

Die rechts oben angezeigte Sternzeit bezieht sich nur auf das Häuser-System! Man gibt also markante EREIGNISDATEN ein und beobachtet wie in dem betreffenden "progressiven Horoskop" die Hauptachsen Aspekte mit Planeten bilden. M.E. sind beide Richtungen zu beachten,wie dies ja auch bei den "klassischen" Primärdirektionen üblich ist.Offenbar ist die "Richtung" fiktiv und nur der Abstand relevant.

Als BEZUGS-ORT kommt primär der Geburtsort in Betracht,nach dem Prinzip daß Direktionen sich immer auf die RADIX beziehen sollten ( siehe z.B. bei KÜHR ). Aber das sollte kein Dogma sein.

Versuchsweise kann man das gedrehte Achsenkreuz noch durch kleinere Variationen verändern und sich anschauen wie sich das Bild ändert.Die Werte dieser probeweisen Verschiebungen können wahlweise in einem Summenspeicher aufsummiert werden.

Die Verschiebungen einzeln oder der Mittelwert ihrer Summe können ins Radix-Horoskop übernommen werden.Dazu bitte genau die Dialogtexte beachten. Man verschiebt die RADIX-STERNZEIT,so daß die Hauptachsen für die betrachteten Ereignisse markante Winkel zu wichtigen RADIX-oder(und) EREIGNIS-Faktoren bilden.

Siehe z.B. dazu das Buch von SCHUBERT-WELLER : "DIE ASTROLOGISCHE GEBURTSZEIT- KORREKTUR".

Bei Variationen wird die Änderung der Sternzeit in Grad eingegeben Zweckmäßig dürfte folgendes Verfahren sein:

Zuerst verschaffe man sich einen Überblick für einige markante Ereignisse ohne irgendwelche Versuche mit Variation zu machen.

Macht man daraufhin Versuche mit Verschiebungen kann man entweder jede Verschiebung einzeln ins Radix übernehmen oder man summiert verschiedene Variationen auf und korrigiert das Radix-Horoskop mit der mittleren Summe dieser Variationen.

Dieses Verfahren verlangt viel Erfahrung.Man muß hier schon wissen,was man tut und muß nicht nur quantitative,sondern auch inhaltliche Überlegungen anstellen.Das ist also etwas für alte Hasen und kaum etwas für Anfänger. Man verschiebt die RADIX-STERNZEIT,so daß die Hauptachsen für die betrachteten Ereignisse markante Winkel zu wichtigen RADIX-oder(und) EREIGNIS-Faktoren bilden.

## ZEIT-WANDERN

Ist ebenfalls für Korrektur-Überlegungen geeignet.Man kann damit mit VER-SCHIEDENEN ZEIT-EINHEITEN ein gegebenes RADIX abwandeln.Die volle Auswertung unterbleibt dabei (nur Hauptaspekte !).

Das Programm läuft automatisch.Ablauf und Steuerung wie bei TRANSIT-GRAPHIK. Zum Schluß kann die volle Auswertung angehängt werden.Man muß erst ein wenig üben um den Wert dieses Programms zu erkennen !

Das Ergebnis ÜBERSCHREIBT wahlweise sofort die ursprüngl. Eingabe. Anstelle dieses Programms kann man auch in analoger Weise mit "PLANETEN-KOOR-DINATEN" arbeiten., Wahlweise kann ein ASPEKTE-Zähler und,wenn bei Horoskopen die HALBSUMMEN-Liste aktiv ist ( unter "VORGABEN HOROSKOP ÄNDERN" zu wählen ) auch ein HALBSUMMEN-Zähler aktiviert werden.

Der ASPEKTE-Zähler wurde für und in Zusammenarbeit mit Herrn Dr. SIEGFRIED SCHIEMENZ für dessen statistische Untersuchungen an HELIOZENTRISCHEN Horoskopen erstellt.

Er zählt auch "GROßE ( = geschlossene ) TRIGONE" und Dreifach-Konjunktionen, die bei SCHIEMENZ "TRIGA" oder "TROIKA" genannt werden.

============================================================

Ablauf bei "Zeitwandern" innerhalb 1 Sekunde nach Signalton UMSTEUERBAR mit TASTATUR bzw. MAUS Stop / Go : LEERTASTE

Groß- oder Kleinschreibung !

---

Zeitmaß LINEAR :

Transite,Multiple Direktionen :

```text
  D  =  1 Tag
  H  =  1 Stunde
  M  =  1 Minute
```

---

Zeitmaß Progressiv ( 1 TAG <> 1 JAHR ):

Sekundär- und Sonnenbogen-Direktionen :

```text
  J  =  1 Jahr   ( 1 Tag )           "
  M  =  1 Monat  ( 2 Stunden )       "
  D  =  1 Tag    ( 3 Min. 56.6 SEK.)"
```

---

Richtung :

```text
  V  =  Vorwärts
  R  =  Rückwärts
```

---

Intervall:

```text
   +  =  Verdoppelung
   -  =  Halbierung
```

---

Wartezeit :

```text
  Linke  Maustaste = + 2 Sekunden
  Rechte Maustaste = - 1 Sekunde
```

---

```text
                WEITER jeweils mit LEERTASTE
```

============================================================

## UHR

Gibt ein Horoskop der aktuellen Tageszeit aus.Die Zeit wird von der System-Uhr übernommen.Die ZZD ( Sommerzeit usw. ) wird abgefragt.

Beim Aktivieren des Menü-Bildschirmes wird dann in der untersten Zeile des Kontrollfeldes,die aktuelle UT,AC,MC und STERNZEIT angezeigt.

Lässt man das UHR-Horoskop stehen wird es alle 5 Sekunden nue gezeichnet.

ACHTUNG!

Befand sich WINDOWS XP im RUHEZUSTAND und HORCOM wird neu gestartet stimmt die Uhrzeit nicht mehr es sei denn WINDOWS XP wird neu gestartet.

Die laufende Uhr kann auch als Datensatz übernommen werden um weitere Auswertungen vornehmen zu können.Dieser Datensatz wird alle 15 Sekunden aktualisiert,im Menü jedoch erst wenn dieses jeweils benutzt wird. Der vorliegende Menüpunkt "UHR" ist,solange diese als Datensatz behandelt wird,nicht mehr zugänglich.

Ergebnisse,wie SOLARE usw. werden nach 30 Sek. gelöscht,da ja die Uhr weiter läuft.

Achtung ! Wenn der Computer im standby - Modus ist läuft die HORCOM-Uhr nicht weiter ! Danach HORCOM verlassen und wieder neu starten.

## AR-DE aus EL-EB  und  EL-EB aus AR-DE

Dienen der Umwandlung eklipt. Koordinaten in äquat. Koordinaten und umgekehrt.

LT aus UT ermittelt Orts-Zeit aus UT "UT aus LT" hat die umgekehrte Funktion.

Die Abkürzungen sind:

AR = Rektascension DE = Deklination EL = Eklipt.Länge EB = Ekl.Breite MC = Ekl.Länge des MC AC = Ekl.Länge des AC UT = Universal Time = GREENWICH - Zeit.

```text
LT = Local Time     = ORTS - Zeit ( Nicht Zonenzeit ! ).,
```

Die Winkel sind auf das WAHRE Äquinoktium bezogen (mit Nutation). Die Zeit muß,mindestens genähert,eingegeben werden um die Effekte der Präzession und Nutation zu berücksichtigen.

## AUFGANG..MERIDIAN-DURCHGANG..UNTERGANG

Erlaubt die Berechnung der AUFGÄNGE,MERIDIAN-DURCHGÄNGE und UNTERGÄNGE der Planeten für vorgegebenes Datum und Ort.

Wahlweise kann eine Liste für einen bestimmten Planeten bei laufendem Datum oder für alle Planeten bei festem Datum erstellt werden.

Es ist nur sinnvoll ECHTE astronomische Objekte zu wählen,also keinesfalls hypothetische Planeten.

Es kann zwischen SCHEINBAREN und WAHREN Koordinaten unterschieden werden. In der Astronomie werden ausschliesslich die SCHEINBAREN Koordinaten zugrundegelegt,die auch Korrekturen für die Lichtbrechung in der Atmosphäre enthalten,die u.a. vom Luftdruck abhängen.Astrologisch könnten hingegen gerade die WAHREN Werte sinngemäßer sein.

Die Zeit-Unterschiede zwischen beiden Berechnungsarten können einige Zeitminuten betragen.

Bei den SCHEINBAREN Werten werden für die Sonne die Zeiten immer auf den oberen Rand der Sonnenscheibe bezogen,bei der Wahl der WAHREN Werte immer auf den Mittelpunkt des jeweiligen Objekts.

In der Ausgabe-Tabelle werden für das vorgewählte Objekt,die 3 Zeitpunkte berechnet,sowie die zugehörigen Länge, Breite,AR,Deklination und Sternzeit. Die Genauigkeit ist bei WAHREN Werten für SO,ME,VE,MA auf wenige Zeitsekunden zu veranschlagen,bei den äusseren Planeten auf <1 Zeitminute. Bei Wahl der SCHEINBAREN Werte sind nur minutengenaue Werte angegeben wegen der Unwägbarkeiten der Korrekturen für die Lichtbrechung.Besonders beim Mond, mit seinem großen Interpolations-Intervall kann es hin und wieder vorkommen daß bei dem betreffenden Datum ein Feld leerbleibt.Dann müssen Sie beim vorhergehenden Datum bzw. beim nachfolgenden nachsehen.

mit den Tasten Bild nach unten,der Leertaste oder der linken Maustaste kann zu späteren Daten weitergegangen werden,mit "-" bzw. "R" oder der "Bild nach oben"-Taste geht man in der Liste rückwärts.

Bitte beachten !! :

Am Aszendenten kann man ein Objekt beim "AUFGANG" nur dann erwarten wenn es gerade die ekliptikale Breite 0 hat.

Das ist nur bei der Sonne immer der Fall.

Entsprechendes gilt für den Meridian-Durchgang und Untergang.

## FINSTERNISSE...

Damit können die Zeitpunkte von SONNEN- und MONDFINSERNISSEN berechnet werden.

Daneben werden die Zeitpunkte von NEUMOND und VOLLMOND ebenfalls aufgelistet, bei denen keine Finsternisse auftreten.

Es wird geozentrisch gerechnet,d.h.die PARALLAXE bleibt ausser Betracht. Man gibt ein Datum in der Nähe des interessierenden Zeitraumes ein.Dann wird eine Liste der NEUMOND- ( LINKS ) und VOLLMOND-Zeitpunkte ( RECHTS )für je 1 bis 2 Mondumläufe vor diesem Zeitpunkt und ca. 30 Mondumläufe danach ausgegeben. Die Sonnenfinsternisse ( LINKS ) bzw. die Mondfinsternisse ( RECHTS ) sind durch Inversion kenntlich gemacht.

mit den Tasten "+" ,der Leertaste oder der linken Maustaste kann zu späteren Daten weitergegangen werden,mit "-" bzw. "R" oder der rechten Maustaste geht man im Datum rückwärts.

In der Ausgabeliste sind nach Datum und Zeit noch LINKS die entsprechende SONNEN-Position und RECHTS die MOND-Position eingetragen.

Die Zeitpunkte des Neumondes bzw. Vollmondes sind i.a. nicht identisch mit den evtl. ausserdem vorliegenden Sonnen- bzw. Mondfinsternissen.Daher sind beide extra aufgelistet.

Die Zeitpunkte der Finsternisse betreffen jeweils die maximale Phase und können einen maximalen Fehler von < 1 Zeitminute aufweisen.

Mondfinsternisse können i.a. nur im KERNSCHATTEN der Erde beobachtet werden, dafür aber von jedem Punkt der Erde.

Sonnenfinsternisse können jeweils nur in begrenzten Bereichen der Erdoberfläche optimal beobachtet werden.Die Einzelheiten aufzuzeigen ist relativ kompliziert.Es ist mit den Buchstaben "N" bzw. "S" lediglich bezeichnet ob die Finsternis auf der NORD- oder SÜD-Halbkugel zu sehen ist. Bei den mit "EX"=EXTERN bezeichneten Sonnenfinsternissen trifft der Kernschattenkegel des Mondes nicht die Erdoberfläche.Diese Finsternisse sind in der Regel "PARTIELL",in selteneren Fällen aber auch "RINGFÖRMIG"="RF" oder sogar "TOTAL"="TOT".Die mit "ZT" = ZENTRAL bezeichneten Sonnenfinsternisse sind ( am Ort ihres maximalen Auftretens ) TOTAL oder RINGFÖRMIG. Hinsichtlich der astrologischen Relevanz von Finsternissen kann man vermuten, daß ihre "Wirkung" deutlicher sein sollte als ein blosser Neumond bzw. Vollmond,da im Falle der Finsternis sowohl ekl. Länge als auch Breite fast zusammenfallen,bzw der Mond nahe seiner Knotenlinie steht.

Die tatsächliche Beobachtbarkeit,im Falle der Sonnenfinsternis,dürfte demgegenüber weniger bedeutsam sein.Von den meisten Finsternissen als Phänomen nimmt man heute ohnehin kaum Notiz,es sei denn,die Beobachtungsbedingungen sind ungewöhnlich günstig.

Wahlweise können auch die ASPEKTE zwischen SO bzw. MO mit dem jeweils gültigen Datensatz ( Radix,Solar usw. ) betrachtet werden.Sie sind jeweils unterhalb der Zeile NEUMOND ( SONNEN-FINSTERNIS ) bzw. VOLLMOND ( MOND-FINSTERNIS ) eingetragen.Die betreffenden Zeilen sind durch einen nach rechts weisenden Pfeil markiert.

Bei der Wahl höherer Teiler ( kleiner Aspekte ) oder nicht ganzzahliger Winkel sind auch die jeweiligen Teiler,bzw. Vielfache hinter dem Aspekt eingetragen.

Man sollte sich mit dem Lesen der Tabellen ohne Aspekte vertraut gemacht haben,bevor man diese hinzunimmt,da sonst Verwirrung entstehen könnte.

## DATEIEN VERKETTEN

Mit diesem Progr. können Sie mehrere Dateien,verschiedenen Namens,miteinander verketten.

Die entstehende Datei heißt für DATEN-Dateien zunächst "\AA_MUDAT.DAT" im Ordner \HORCOM\SPEZIAL\ Sie können diese verkettete Datei auch jederzeit umbenennen und wieder löschen. Das Ganze erfordert etwas Übung.

ALLGEMEINES BETREFFEND DATEI-HANDLING :

Für das selbständige Datei-Handling ist es unbedingt erforderlich,das Kopieren und Umbenennen der Dateien,nach der Betriebs-Anleitung des PC zu beherrschen,also den DATEI-MANAGER von WINDOWS !

DATEN-Dateien sind dabei in dem Ordner \HORCOM\SPEZIAL\ unterzubringen.Den Namen können Sie in der FILESELECT-BOX bei "AUSWAHL" selbst festlegen. Er muß immer die "EXTENSION" .DAT haben.

Der Name darf max. 8 Buchstaben (ohne Umlaute) oder Ziffern haben,z.B. ..\SPEZIAL\SPORTLER.DAT für Daten von Sportlern.

Für ORTS-Dateien ist der Ordner \HORCOM\SPEZ_ORT\ vorgesehen.Die Dateinamen müssen immer die "EXTENSION" .INT haben., Bei DATEN- und ORTS-Dateien kommt in HORCOM3P IMMER die FILESELECT-BOX., Wichtig : Richten Sie für diese Dateien KEINE EIGENEN "ORDNER" ein !! Die DATEI-Namen können Sie hingegen frei wählen.

Im Ordner "\SPEZ_ORT" sind bereits Dateien für DEUTSCHLAND,ÖSTERREICH und die SCHWEIZ vorhanden.Sie werden freundlicherweise allen HORCOM-Usern von Herrn BRUNO MAHL kostenfrei zur Verfügung gestellt.

Zwei Dateien "\WELT.INT" und "\EUROPA.INT" sind ebenfalls schon vorhanden. Sie stammen aus Dateien von Hrn.Dr.H.WISGRILL die in das HORCOM-Format gepresst wurden.Es sind daher nur die ersten 16 Buchstaben der Ortsbezeichnung sichtbar.Die 4 letzten Zeichen wurden für die ZZD = Zonen-Zeitdiffereunz ( zur UT=GMT ) verwendet.Herr Dr.WISGRILL stellt ebenfalls diese Daten als PD freundlicherweise zur VerfÜgung.Die ZZD im Namen ist zweckmäßig um sofort auf die UT schließen zu können.In vielen Fällen gibt es trotzdem Unsicherheiten,durch Sommerzeiten usw.

Eine umfangreiche DOKUMENTATION der ZEIT-BESTIMMUNGEN für 28 europäische Länder in Vergangenheit und Gegenwart,im Umfang von ca. 40 dichtbeschriebenen Schreibmaschinenseiten wurde von Herrn BRUNO MAHL erstellt.Sie ist auch ausdruckbar,erspart die Anschaffung einer ganzen Reihe von Büchern,ist direkt, während der Arbeit mit dem PC ( natürlich auch für andere AstrologieProgramme ) abrufbar und wird auch in der EINGABE-BOX bei der Ersteingabe verwendet. Sie enthält insbesondere die Kalender-Umstell-Daten,Sommerzeiten,Sonderzeiten,Umstellungsdatum von Orts- in Zonenzeit usw.Gerade für historisch arbeitende Astrologen ist dies sehr nützlich.

## AAF-DATEI <> HORCOM-DATEI

Wandelt eine AAF-DATEI ( = Astrologisches Austauschformat ) in eine HORCOMlesbare DATEN-Datei um oder umgekehrt.Es werden dabei nur diejenigen Daten aus der ( oft umfangreicheren ) AAF-Datei entnommen,die in das feste HORCOM-Format passen.

ZONEN-,ORTS-Zeiten usw. werden in UT umgewandelt.Von einem eventuellen Kommentar wird nur der erste Satz angezeigt.

Hinsichtlich des NAMENS ist zu beachten,daß der Namens-Teil vor dem ersten Blank im HORCOM-Format als NACHNAME im AAF-Format interpretiert wird ! Die in HORCOM-DATEN-Dateien umgewandelten AAF-Dateien werden mit der Endung .DAT im Ordner \HORCOM\SPEZIAL angelegt,als neben den originären HORCOM-DATEN-Dateien.

Achtung ! Die AAF-Dateien werden immer in einem Ordner .....\AAFDATEN angelegt bzw. vorausgesetzt.Die Hierarchie dieses Ordners wird beim ersten Programmstart von HORCOM5P/7P festgelegt.Der früher vorgeschriebene Ordner ..\ASTRODAT.AAF entfällt.

Falls sich dort noch AAF-Dateien befinden,sollten diese in den Ordner ..\AAFDATEN verschoben werden.

Mit diesen Programmen sollte es auch möglich sein,Dateien die mit anderen Programmen,erstellt wurden über den Umweg AAF in HORCOM-lesbare Dateien umzuwandeln,falls diese,wie z.B. HERMES das AAF-Format unterstützen.

## WINKEL-ZEIT UMRECHNUNG

Damit können Umrechnungen von Dezimal-Winkeln bzw. h in ° ' " bzw. h mi sek vorgenommen werden.

Dies kann auch aus jedem Ergebnis-Bildschirm mit der Taste F5 aufgerufen werden !

## HITERGRUND-FARBEN

Hiermit kann der Farb-hintergrund der Dialoge und deren Hintergrund festgelegt werden.

## ORT-WANDERN

Funktioniert analog dem ZEIT-WANDERN nur dass hier bei gegebener Zeit der Ort d.h. geogr. LÄNGE und BREITE verändert werden können.

Dies kann als ein Schritt in Richtung ASTRO-GEOGRAPHIE verstanden werden.

Natürlich können Sie auch schon immer in der Eingabe-Box in jedem Datensatz anstelle des Geburts-Ortes auch Orte aus den Orts-Dateien überschreiben wenn Sie sehen wollen wie sich eine Ortsveränderung ausgewirkt hätte.

Auch Leute die sich ein "gutes" Solar für ihren Geburtstag suchen möchten können davon Gebrauch machen.

Wenn Sie die Funktionstaste F10 während des Ablaufs drücken erscheinen links unten die Koordinaten einer nahegelgenen Metropole,falls in dem Ordner \HORCOM\ SPEZ_ORT die Datei WCAPITAL.INT vorhanden ist.

Wenn Sie dann "ENTER" drücken werden die Koordinaten dieser Stadt für den weiteren Ablauf übernommen.

## GROSSES JAHR

Dieses Programm ist für die Erforscher des "Platonischen Jahres " gedacht. Der Frühlingspunkt wandert infolge der "PRÄZESSION" in ca. 25776 Jahren rückläufig durch den Tierkreis.

Manche Astrologen nehmen nun an,daß die Durchlaufzeit duch ein Zeichen einem "ZEITALTER" entspricht.Die meisten nehmen an,daß wir gegenwärtig im "WASSER-MANN-ZEITALTER" leben.

Der Anfangspunkt dieser Zählung ist höchst fragwürdig,da der "gültige" Tierkreis definitionsgemäß immer vom Frühlinspunkt aus zählt. Wer aber diese "Zeitalter" erforschen will kann mit diesem Programm arbeiten. Man gibt das vermutete Beginn-Datum dieses Zeitalters ein und erhält dann für den aktuellen Datensatz den "ZEITALTER-PUNKT" ,der sich durch die PRÄ-ZESSION rückläufig in ca. 25776 Jahren um 360 Grad verschiebt. Die Berechnung geht astronomisch nach den strengen Formeln. Als Beginn-Datum für das "WASSERMANN-ZEITALTER" könnte man z.B. die Entdekkung des Uranus wählen : 13.3.1781 oder die Erstürmung der Bastille 14.7. 1789 usw.Der vorhergehende Zeitraum des "FISCHE-ZEITALTERS" wären dann die ca. 2148 Jahre davor usw.

Das Programm hält die Möglichkeit vom WIDDER- bis STEINBOCK- Zeitalter bereit,entsprechend dem Gültigkeitsbereich der Ephemeride,mit ansonsten freier Eingabemöglichkeit.

## DESKTOP ( QUIT )

Führt zum Desktop bzw. dem WINDOWS-PROGRAMM-MANAGER zurück,von dem aus jede Software gestartet wird.Den gleichen Effekt hat das Schließen des HORCOM-HAUPTMENÜS.

Sind Sie mit einer HORCOM-Sitzung fertig,sollten Sie IMMER !! eine dieser beiden Möglichkeiten benutzen und danach auch WINDOWS ordnungsgemäß beenden. Ansonsten kann Müll auf der Festplatte zurückbleiben !!

## ERGEBNIS ALS RADIX

Wandelt ein SOLAR,SEPTAR,LUNAR,TAG-HOR oder COMBIN in ein RADIX um,sodaß Sie mit ALLEN Programmen arbeiten können,z.B ein LUNAR eines SOLARS machen usw. Bitte Vorsicht !!,

## ÄNDERUNGEN / HINWEISE / KURZANL.

Hier kann man die Änderungen und Verbesserungen der letzten Jahre nachlesen. HINWEISE sind Bemerkungen zum Arbeiten mit HORCOM unter WINDOWS. KURZANL. ist eine kurze Einführung in die Bedienung von HORCOM. Die beiden letzteren Themen erreicht man auch nach Drücken der rechten Maustaste oder F1 aus dem HORCOM-Hauptmenü heraus.
