# Erläuterung Ephemeride

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P), unverändert bis auf die Formatierung.*

## VORBEMERKUNG

Für die grossen Planeten ME,VE,ERDE (SONNE) MA,JU,SA,UR,NE, wird in HORCOM eine abgekürzte Version der neuesten analytischen Theorie VSOP87 des BUREAU DES LONGITUDES,PARIS ( P.BRETAGNON ) verwendet.Diese wurde nun durch JEAN MEEUS ("ASTRONOMISCHE ALGORITHMEN" , WILLMANN BELL INC,JOHANN AMBROSIUS BARTH, ISBN 3-335-00318-7) einem größeren Kreis zugänglich gemacht.

Die Zahlen für die Planeten-Formeln sind auf Diskette erhältlich,die direkt von dem Magnetband des INSTITUT DES LONGITUDES übernommen wurden,sodaß die Möglichkeit von Übertragungsfehlern weitgehend ausgeschlossen ist. Es handelt sich immerhin,allein für diese Planeten,um ca. 8500 Zahlen ( der abgekürzten Theorie ).

Das Einlesen dieser Zahlen von Diskette erfolgt bei Programmstart und erfordert einige Sekunden Zeit.

Für das vorliegende Programm wurden diese Dateien auf GFA-BASIC umgesetzt.

```text
             Dieses Programm enthaelt Softwarecode
             Copyright (c) 1991-1992 by Jeffrey Sax
             und verteilt durch Willmann-Bell, Inc.
                         Serie 10756
```

Die Routinen stellen,bei relativer Kompaktheit,einen sehr hohen Stand an Präzision dar,der besser ist als viele vorliegende gedruckte Ephemeriden,die in der Regel nur maximal 150 Jahre umfassen und die Planetenpositionen oft nur minutengenau darstellen.

Diese Routinen stellen dagegen im Zeitraum 2000 v.Chr. bis 6000 n.Chr. die Sonne und Venus mit Fehler < 1",die übrigen genannten Planeten mit ca. < 4" Die genannten Daten beziehen sich auf EPHEMERIDENZEIT bzw. DYNAMISCHE ZEIT die neuerdings durch Atomuhren definiert wird.

Diese dynamische Zeit läuft völlig gleichmäßig ab,während die UT ( = GMT ) einer Langzeit-Drift sowie unregelmäßigen Schwankungen unterliegt. Für Daten vor -2000 ist bei SO,ME,VE,MA,JU,SA,UR,NE mit langsam ansteigenden den Fehlern zu rechnen,die aber irrelevant sind,da in diesem Datums-Bereich ohnehin der Zusammenhang zwischen der UT ( = GMT ) und der Ephemeridenzeit mehr und mehr verloren geht.

Für PLUTO existiert leider keine entsprechend präzise analytische Theorie. Hier wurde durch numerische Integration im Bereich 18.05.602 v.Chr. bis A.D 12.06.2201 eine genaue Ephemeride von mir berechnet.Darüber hinaus werden analytische Näherungsformeln von J.CHAPRONT ( BUREAU DES LONGITUDES ) verwendet.Deren Fehler kann in fernen historischen Zeiten ( v.Chr. ) auf mehrere Minuten anwachsen.

Bei der Integration wurden die Störungen durch die großen Planeten berücksichtigt.Die Störungen durch die tausende von Asteroiden und den Objekten im "KUIPER-Gürtel" mussten natürlich außer Betracht bleiben.

Ab HORCOM7P wurde auch für den Planetoiden QUAOAR eine Ephemeride berechnet, für den gleichen Zeitraum wie für Pluto und mit der gleichen Rechengenauigkeit.

QUAOAR verläuft außerhalb der Plutobahn,hat aber im Gegensatz zu Pluto eine geringe Exzentrizität und Bahnneigung,also wie ein echter großer Planet. Er hat allerdings nur etwa den halben Durchmesser von Pluto. Als SYMBOL für QUAOAR wurde einstweilen ein "Q" verwendet,solange noch kein allgemein gebräuchliches Symbol existiert.

Ab 10.Februar 2006 wurde auch der Planetoid XENA im gleichen Zeitraum von ca. 2800 Jahren numerisch integriert.Er ist größer als Pluto und verläuft mit stark exzentrischer und stark geneigter Bahn großenteils außerhalb der Plutobahn.

Als SYMBOL für XENA wird einstweilen ein "X" verwendet.

Die charakteristischen genäherten Bahndaten für PLUTO,QUAOAR und XENA sind:

```text
   Name         Kürzel    a        e     i (Grad)     T (Jahre)
   PLUTO         PL     39.65    0.25   17.15°       250
   QUAOAR        QU     43.25    0.035   7.99°       284
   XENA          XE     67.66    0.44   44.18°       557
```

Dabei sind:

```text
              a = Große Halbachse e = Exzentrizität
              i = Inklination     T = Umlaufszeit
___________________________________________________________________________
```

Für den Zusammenhang zwischen UT = GMT und Ephemeridenzeit = ET wurden die neuesten Unterlagen herangezogen,die nur bis einige hundert Jahre v. Chr. diesen Zusammenhang einigermassen gewährleisten.Für die Zukunft kann dieser Zusammenhang ebenfalls nur vermutet bzw. extrapoliert werden. Insofern ist die zeitliche Spannweite dieses Programms auch für den historisch forschenden Astrologen bzw. den "Zukunfts-Seher" für die praktisch infrage kommenden Daten völlig ausreichend.

Siehe dazu auch weiter unten bei "PLANETEN-KOORDINATEN".

VORGABEN EPHEMERIDE ÄNDERN :Dient zur Eingabe der Parameter für die Ephemeridenberechnung und zur Wahl der ZUSATZ-Planeten.

Die bereits VOREINGESTELLTEN Werte sind an dem jeweils DICK UMRANDETEN Kästchen zu ersehen.

Im Einzelnen:

- Bei den LÄNGEN kann zwischen den WAHREN = GEOMETRISCHEN und den APPARENTEN Werten gewählt werden,wobei im letzteren Fall noch unterschieden wird zwischen Berücksichtigung der LICHT-LAUFZEIT allein ( = astrometrische Koordinaten ),sowie zusätzlich der "JÄHRLICHEN ABERRATION" . Für den Mond können alle diese Unterschiede vernachlässigt werden.

Abkürzungen :

- W = Wahre ( =Geometrische ) Position.
- A1 = Apparente Position,wenn nur die LICHT-LAUFZEIT berücksichtigt ist. Hier ist der Abstand des Planeten maßgebend ( = "astrometrische" Position).
- A2 = Apparente Position,wenn die JÄHRLICHE ABERRATION zugefügt ist. Hier ist auch die Umlaufgeschwindigkeit relativ zum Beobachter maßgebend. Diese wird i.a. nur bei Fixstern-Positionen verwendet und ist daher bei diesem Programm automatisch eingeschaltet um evtl. Vergleiche zu ermöglichen.

All diese Unterscheidungen spielen sich unterhalb einer Bogenminute ab, sind aber interessant,wenn man mit gedruckten Ephemeriden vergleichen will. Leider ist bei astrologischen Eph. nicht immer angegeben,welche Werte gemeint sind und welche Zeit (UT oder ET) zugrunde liegt (siehe unten !).In jedem Fall erfordert ein korrekter Vergleich einige astronomische Grundkenntnisse,die nur wenigen Astrologen geläufig sind.Die Genauigkeits-Fanatiker mögen sich also bitte erst vergewissern,daß Sie nicht " Äpfel mit Birnen" vergleichen,bevor sie ungenaue Zahlen feststellen ! Bei älteren Ephemeriden ist übrigens mit Fehlern bei Pluto und Neptun bis zu ca. 0.5' zu rechnen,da vor 1978 für die Masse des Pluto ein viel zu hoher Wert zugrundegelegt wurde.

Bei Solaren ist die Wahl dieser Berechnungs-Modalitäten kaum von Belang. Bei Direktionen kann sie sich in etwas unterschiedlichen Zeitpunkten auswirken. Was nun "richtig" ist,soll hier nicht entschieden werden.Meist wird die mit APPARENT1,bzw. A1 bezeichnete Version angewendet. Wichtig ist natürlich daß man bei Solaren usw. immer mit den gleichen Parametern rechnet wie bei der Radix.

- Beim MONDKNOTEN kann zwischen dem üblichen MITTLEREN Wert und dem WAHREN (="oskulierenden" bzw. momentanen) Wert gewählt werden. Der Mittelwert kann sehr präzise angegeben werden der wahre Wert kann einen Fehler von wenigen Bogenminuten aufweisen.

- "PARALLAXE" Dient zur Wahl der Planeten-Berechnung mit Berücksichtigung der Parallaxe oder ohne.Bei LUNAREN kann der Unterschied mehr als 1h ausmachen ! Mit Parallaxe heißt : Die Koordinaten werden TOPOZENTRISCH bestimmt.,d.h. für den jeweiligen Ort des Beobachters auf der Erdoberfläche,der natürlich auch zeitabhängig ist infolge der Erdrotation. Bezeichnung durch "P" oder durch entspr. Text. Die Parallaxe wird nun für alle Planeten berechnet. Bei den äusseren Planeten NE,PL liegt der Effekt unterhalb der Rechengenauigkeit.Bei Venus kann er bis 34" betragen,bei Mars bis 24",bei Jupiter bis 2.2". Es kann wohl keinem Zweifel unterliegen,daß die topozentrischen Positionen, d.h. MIT Parallaxe,die astrologisch "richtigeren" sind.

- "ZUSATZ-PLANETEN" erlaubt die Einbeziehung der "ASTEROIDEN" CHIRON, CERES,PALLAS,JUNO,VESTA .Zeitliche Gültigkeit,siehe unten.

Der "TRANS-PLUTO" ( oft auch ISIS genannt ) ist ebenfalls unter ZUSATZ-PLANETEN anzuwählen (s.unten), wie auch der GLÜCKSPUNKT.

Die 8 "HAMBURGER" Planeten können ebenfalls gewählt werden ( siehe unten ).

Ab HORCOM7P wurden noch zusätzliche 7 Objekte aufgenommen:

Astrologisch von Interesse können wohl vor allem diejenigen Asteroiden sein,die,ähnlich wie CHIRON einen großen mittleren Abstand = a und große Exzentrizität = e haben,sodaß sie unter den tausenden von Asteroiden unterscheidbar sein könnten. Oft werden diese Objekte als "KENTAUREN" bezeichnet.Einige haben auch entsprechende Namen.

Insgesamt sind ab HORCOM7P 4 solcher Objekte ( einschließlich CHIRON ) aufgenommen,zusätzlich noch der Komet HALLEY,der eine noch extremere Bahncharakteristik aufweist. In der folgenden Tabelle sind Näherungswerte einiger charakteristischer heliozentrischen Bahndaten aufgeführt.Dabei ist i die Bahn-Neigung gegen die Ekliptik,T die ungefähre Umlaufszeit.

Im Fall des Kometen HALLEY ist die Bahnneigung fast 180 Grad,sodass dieser heliozentrisch retrograd zu den übrigen Planeten verläuft.

```text
   Name         Kürzel      a        e     i (Grad)     T (Jahre)
   KOMET HALLEY  HL        17.94    0.97  162.24        76
   Chiron        CH        13.61    0.38    6.94        50
   Damocles      DA        11.82    0.86   61.84        41
   Nessus        NS        24.46    0.52   15.66       121
   Pholus        PH        20.23    0.57   24.70        91
```

Für alle diese Objekte wurden die Ephemeriden durch numerische Integration im Ensemble mit den großen Planeten von mir errechnet, sodass deren Einflüsse voll berücksichtigt sind.Als Startwerte wurden die neuesten Bahnelemente verwendet (Natürlich viel genauer als die oben genannten Richtwerte ).

Über die verwendeten SYMBOLE gibt die Tabelle "ZUSATZ-PLANETEN-KOORDINATEN" ( Zweites Blatt ! ) Aufschluss.

Die Zeitspannen der von mir errechneten und in HORCOM7P gespeicherten Ephemeriden sind nochmals zusammengefasst :

```text
   PLUTO,QUAOAR und XENA :
   JD 1502079.5 bis 2525120.5  = 18.05.602 v.Chr. bis 12.06.2201

   CHIRON:
   JD 1502279.5 bis 2524460.5  = 07.01.600 v.Chr. bis 21.08.2199

   ASTEROIDEN CERES...VESTA und die 3 "KENTAUREN" DAMOKLES,NESSUS
   PHOLUS :
   JD 2268939.5 bis 2488390.5  = 08.01.1500       bis 18.11.2100

   Komet HALLEY :
   JD 2305446.5 bis 2469806.5  = 31.12.1599       bis 31.12.2049
```

Der Komet HALLEY wird nach meiner Rechnung bei der nächsten Wiederkehr ( 2061 ) im Perihel der Venus sehr nahe kommen und danach erheblich veränderte Bahndaten haben.

Die gewählten Zusatz-Planeten erscheinen in den Horoskopen und sonstigen Darstellungen.In der Tabelle "ZUSATZ-PLANETEN-KOORD." sind alle auf zwei Blättern dargestellt. Beim Programm "ASPEKTARIUM" können max.5 verarbeitet werden !

Je mehr Zusatz-Planeten gewählt werden,desto höher werden die Rechen-und Auswerte-Zeiten ( Bei Aspekten und Halbsummen bis zum 10-fachen Wert ! ). Außerdem kann es öfter zu Überdeckungen der Symbole im Horoskop,oder zur Beschneidung von Graphiken kommen. Also : Nicht mehr Zusatz-Elemente auswählen,als angemessen !!

Auch die MOND-APSIDEN ( = "SCHWARZER MOND" ) sind unter den Zusatz-Planeten anwählbar. SYMBOL : 2 sich schneidende Kreise.Es kann zwischen dem MITTLEREN Wert und dem WAHREN (="oskulierenden" = momentanen) Wert gewählt werden.Der wahre Wert kann sich hier vom mittleren um mehr als 20 Grad unterscheiden.

Der "SCHWARZE MOND" Ist das WAHRE (=MOMENTANE) APOGÄUM = AG der MOND-BAHN. Der WAHRE Wert ist jeweils durch INVERTIERTES SYMBOL,bzw. mit "W" gekennzeichnet (bei den Drucker-Graphiken durch einen Rechteckrahmen um das Symbol),der MITTLERE durch "M".Der mittlere Wert ist jeweils exakt,der wahre eine gute numerische Näherung mit einigen Bogenminuten max.Fehler. Der "schwarze Mond" ist kein Planet sondern eine "Achse". Ein Aspekt mit Winkel W zum S.M. ist also gleichbedeutend mit W+180 zum Gegenpol.Eine Einbeziehung des Gegenpols mit eigenem Symbol,wie dies ja bei den Mondknoten der Fall ist,wurde einstweilen nicht vorgenommen.Es ist daher ratsam,bei der Beurteilung von Aspekten usw. zu diesem Faktor,das Horoskop-Diagramm mit zu verwenden,in dem die Achse als gestrichelte Linie eingetragen ist.

ACHTUNG! Für den schwarzen Mond sind unterschiedliche Definitionen in Gebrauch.In HORCOM wird für den WAHREN Wert darunter die "oskulierende" Apside der Mondbahn verstanden.Diese Werte entsprechen,mit Abweichungen von wenigen Bogenminuten,z.B denen der "ROSICRUCIAN EPHEMERIS",die wiederum auf der Ephemeride DE200/LE200 des US NAVAL OBSERVATORY entspricht. Was den MITTELWERT der Mondapsiden anbelangt besteht Einheitlichkeit.

## "VORGABE EPHEMERIDE ÄNDERN"

Hiermit können die oben erwähnten Parameter vorgegeben werden. Ab HORCOM5P kann auch ein FIXPUNKT ( = feste ekliptikale Länge ) unter "VORGABE EPHEMERIDE ÄNDERN" vorgegeben werden .Der Fixpunkt hat das Symbol "F" in roter Farbe und die Kurzbezeichnung FP.Er erscheint,wenn vorgewählt,in den meisten Ausgaben.Sie können damit z.B. den Einfluß bestimmter Tierkreis-Grade studieren.Der FIXPUNKT wird geozentrisch und heliozentrisch gleich behandelt,entspricht also einem unendlich weit entfernten Planeten ohne ekliptikale Breite,der nur die Präzession mitmacht,da er im Tierkreis festliegt. Streng genommen müßte,geozentrisch,auch die Nutation der Erdachse hinzugefügt werden,was aber die Sache über Gebühr komplizieren würde.Dieser Effekt liegt unterhalb ca. 30 arcsek.

In den STATISTIK-Dateien wird der Fixpunkt nicht gespeichert,da hier ohnehin die Eingabe eines solchen Punktes möglich ist,wobei ein evtl. bereits definierter Fixpunkt überschrieben werden kann.

In der MÜNCHNER RHYTHMENLEHRE kann ebenfalls ein Fixpunkt eingegeben werden,der allerdings dort eine spezielle Bedeutung hat.Er ist dort jetzt ebenfalls als rotes "F" bezeichnet,muß aber nicht identisch sein mit dem "normalen "( sonstigen ) Fixpunkt,sondern wird unabhängig davon definiert und gespeichert.Er ist auch mit "SP" benannt.

Ansonsten wird in allen Auswertungen der Fixpunkt wie ein Planet behandelt. Bei "DYNAMOGRAMMEN" wird er wie einer der inneren Planeten ( ME bis MA ) gewertet.

Der Orbis-Faktor läßt sich unter "VORGABEN HOROSKOP ÄNDERN" einstellen.

ACHTUNG !!

Die Werte von exakt 0° und 360° können aus technischen ( arithmetischen) Gründen NICHT als Fixpunkt gewählt werden bzw. wären dann teilweise unwirksam.

## PLANETEN-KOORDINATEN ( helio- oder geozentrisch )

Das Programm gibt die Planeten-Koordinaten für den jeweils aktivierten Datensatz aus.

(Das anschliessende "ZEIT VARIIEREN" erlaubt die schrittweise Veränderung der Zeit um Detailstudien zu machen.Die Routine ähnelt der bei "ZEITWANDERN", siehe Erläuterung 9 betr. KORREKTUR.Der Datensatz kann hierbei mit der variierten Zeit überschrieben werden.Man kann diese Routine also auch zum Korrigieren verwenden ).

Man kann aus allen TABELLEN-ausgaben heraus,durch Drücken der FUNKTIONS-TASTE F2 das jeweilige Horoskop zwischendurch ansehen und mit 'Esc' wieder zurückgehen.

Umrechnungs-Formeln wurden großenteils folgenden Büchern entnommen,die für den Selbst-Programmierer eine gute erste Grundlage sind,vorausgesetzt er hat solide mathematische Grund-Kenntnisse.

- "ASTRONAMICAL FORMULAE for CALCULATORS" von J.MEEUS / 1985 ( Kein Lehrbuch )
- "ASTRONOMISCHE ALGORITHMEN" von J.MEEUS / 1992 ( Kein Lehrbuch,aber eine Fundgrube astronomischer Formeln,die den neuesten Stand für den Selbstprogrammierer darstellt,und fast alle wünschenswerten Formeln enthält.Siehe oben ! )
- "GRUNDLAGEN DER EPHEMERIDENRECHNUNG" von O.MONTENBRUCK / 1985 ( LEHRBUCH ).
- "EINFÜHRUNG IN DIE HIMMELSMECHANIK UND EPHEMERIDENRECHNUNG" A.GUTHMANN B.I. WISSENSCHAFTSVERLAG ( LEHRBUCH ).

Literatur-Hinweise sind bei MONTENBRUCK zu finden.Die Original-Literatur verlangt zum Verständnis allerdings erhebliche Vorkenntnisse in der sphärischen Astronomie und der Himmels-Mechanik.

Für den MOND wurden STÖRUNGSGLIEDER > 1 arcsec Amplitude berücksichtigt. Trotz Bemühung um Einfachheit ist die Planeten -Berechnung recht umfangreich geworden und braucht einige Rechenzeit.

Die Planetenberechnung erfolgt immer in Ephemeridenzeit,während die Sternzeit Häuserberechnung usw. von der UT ausgeht.

Für den Zusammenhang zwischen EPHEMERIDENZEIT (=ET) und UT wurden die in der American Ephemeris.. zu findenden Daten für die Jahre 1620 bis 1998 verwendet,ausserhalb dieses Intervalls die neuesten Näherungsformeln ( F.R.STEPHEN SON,L.V.MORISSON, Phil.Trans.R.Soc.Lond. A 313,47-70 / 1984 ) Bei fernen historischen Zeiten kann die Verwendung dieser neueren Daten zu erheblichen Abweichungen bei der Position des Mondes gegenüber den mit der "Improved Lunar Ephemeris" berechneten Werten führen,da diese noch auf einer älteren Näherungsformel ( 1948 ) beruhen.Bei der Sonne sind die entsprechenden Unterschiede etwa eine Grössenordnung geringer.

Der Unterschied zwischen ET und UT wächst z.B. bis zum Jahr 1 n.Chr. auf ca. 2.7 h !!! (nicht linear).Einigermassen gesichert ist der Zusammenhang nur bis 390 v.Chr. Bei älteren Daten wird extrapoliert.Man darf also dann nicht mehr mit präzisen Ergebnissen rechnen.

Auch für die Zukunft kann die UT nur genähert aus der gleichförmig ablaufenden ET ermittelt werden.Pro Jahrhundert beträgt der Unterschied ca.1-2 Min.

Diese Fragen sind nur bei HISTORISCHEN Horoskopen relevant.Aber gerade hier eröffnet der Computer auch neue Möglichkeiten.

Da im vorliegenden Programm immer die UT = GMT = GZ als Zeit angegeben ist, werden sich ausserhalb des Jahres 1900 ( wo UT und ET ca. übereinstimmen ) bei SO und MO merkliche Unterschiede zu gedruckten Ephemeriden ergeben,die sich oft auf Ephemeridenzeit beziehen.Wenn Sie also VERGLEICHEN,erst die UT aus der ET ermitteln (s.unten) und vergewissern Sie sich,was Sie vergleichen ! Die GEOZENTRISCHEN Positionen beziehen sich auf das WAHRE Äquinoktium (mit Nutation),die HELIOZENTRISCHEN auf das MITTLERE Äquinoktium,jeweils des aktuellen Datums.

Für Sonne und Mond entspricht die Genauigkeit einer Zeit-Auflösung von weniger als 20 Zeit-SEKUNDEN.Das ist für die Erstellung von Solaren und Lunaren hinreichend genau.

Die PARALLELEN entnehmen Sie der Spalte DEKLIN. der Tabelle. "VEL." gibt die MOMENTANE Änderung der ekl.Länge in '/Tag . Für die Ermittlung dieser Werte wurden ebenfalls die Formeln aus J.MEEUS herangezogen.Es ist zu beachten,daß es sich um die momentane Geschwindigkeit handelt und nicht um den täglichen Längen-Fortschritt,wie er aus den Ephemeriden-Tabellen entnommen werden kann !

Die GEOZENTRISCHEN MAXIMALEN Geschwindigkeiten der Paneten in POSITIVER und NEGATIVER Richtung in BOGENMINUTEN / TAG wurden wie folgt empirisch ( statistisch ) ermittelt:

```text
PLANET          MAXIMAL POSITIV      MINIMAL bzw. MAXIMAL NEGATIV
                      min/d                   min/d

SONNE                 61.188                  57.186
MOND                 922.28                  709.18

MERKUR               132.08                  -81.74
VENUS                 75.48                  -37.59
MARS                  47.47                  -23.91
JUPITER               14.53                   -8.19
SATURN                 7.82                   -4.95
URANUS                 3.68                   -2.51
NEPTUN                 2.27                   -1.70
PLUTO                  2.38                   -1.75

MONDKNOTEN WAHR        2.334                 -15.768
APOGÄUM WAHR         367.33                 -213.24

CHIRON                 6.71                   -4.11
TRANSPLUTO             0.850                  -0.719
CERES                 26.94                  -14.58
PALLAS                35.29                  -21.99
JUNO                  34.96                  -16.59
VESTA                 32.25                  -16.45

CUPIDO                 1.600                  -1.265
HADES                  1.321                  -1.014
ZEUS                   1.094                  -0.897
KRONOS                 1.007                  -0.809
APOLLON                0.920                  -0.761
ADMETOS                0.892                  -0.713
VULKANUS               0.853                  -0.695
POSEIDON               0.773                  -0.640
```

TRANSPLUTO und die letzten 8 ( "HAMBURGER" ) sind HYPOTHETHISCH !

Spalte A ( = Acceleratio ) enthält das Vorzeichen der Beschleunigung.Im Fall. eines Umkehrpunktes kann man hiermit erkennen ob der Planet direktläufig wird (A positiv) oder rückläufig (A negativ).

Die Spalte "ENTF." gibt die gegenseitige Entfernung zwischen Erde und Planet, gemessen in AE an ( 1AE = Mittlere Entfernung Sonne-Erde ). Für die großen Planeten ME,VE.........NE,PL können wahlweise auch die relativen Werte,bezogen auf den Mittelwert,in % des Mittelwerts aufgelistet werden.

Neben den Positionen im ekliptikalen und äquatorialen Bezugs-System,sind auch die ekl.Positionen der mittleren nördlichen oder südl. Planeten-KNOTEN angegeben.

Der "SONNENKNOTEN" (=Schnitt-Linie zw. Äquator SO u. Ekliptik) wurde nach "GEOZENTRISCHE PLANETENKNOTEN" von DR.TH.LANDSCHEIDT berechnet. In geozentrischer Betrachtung handelt es sich um Raumpunkte,in heliozentrischer um Richtungen !

In der letzten Spalte sind die mittleren PLANETEN-APSIDEN aufgetragen (auch geozentrisch) .Oben das PERIHEL unten das APHEL.Es sind dies die großen Achsen der Bahnellipsen ( Mittelwerte ).In geozentrischer Betrachtung handelt es sich um Raumpunkte,in heliozentrischer um Richtungen ! Die Werte beziehen sich auf den Frühlingspunkt ( = Knotenlänge + Abstand Perihel-Knoten ).

Diese Werte werden in der Astrologie nicht verwendet.Wenn doch,dann höchstens für die Sonne (Erde).Ich habe sie hier mit aufgenommen,da sie bei der Planetenberechnung heliozentrisch bereits vorhanden sind und mit geringem Aufwand auch geozentrisch darstellbar sind.

Für eine heliozentrische Astrologie hätten die Apsiden höchstwahrscheinlich erhebliche Relevanz,für die geozentrische ist dies weniger einsichtig.Entsprechendes gilt für die Planeten-Knoten.

Die momentanen ( wahren ) Werte der APSIDEN können bei den äusseren Planeten SA,UR,NE,PL um mehrere Grade vom Mittelwert abweichen. Ich würde daher empfehlen,eventuelle Studien zur astrologischen Relevanz dieser Elemente auf MO,SO,VE,MA,JU zu beschränken.

Der Merkur ist wegen seiner beträchtlichen Bahnneigung ebenfalls mit Reserve zu betrachten.

Der Bezug auf den Frühlingspunkt ist für die geozentrische Betrachtung wohl die angemessenere Wahl,allerdings nur bei geringer Bahnneigung.

Die Angabe "MONDPHASE" ( im Tabellen-Kopf ) ist die ekliptikale Längendifferenz MOND-SONNE. 0 bis 180 Grad entspricht zunehmendem,180 bis 360 Grad abnehmendem Mond.

Dahinter steht noch eine %-Angabe.Die Opposition ( = VOLLMOND ) entspricht dem Wert 100%,die Konjunktion ( = NEUMOND ) dem Wert 0%.Damit sind,in etwas anschaulicherer Weise,die Mondphasen genähert zu ersehen.

Wollen Sie die exakten Zeiten von VOLLMOND oder NEUMOND ermitteln,können Sie das Programm "FINSTERNISSE..." unter "DIVERSES" benutzen ( Erläut. 9 ).

## ZUSATZ-PLANETEN-KOORDINATEN ( helio- oder geozentrisch )

Weiteres betr. ZUSATZ-PLANETEN-KOORD. :

Für die kleinen Planeten (Asteroiden) existiert keine analytische Planeten-Theorie.Hier wurde im Bereich 08.01.1500 A.D bis 18.11.2100 A.D. durch numerische Integration eine Ephemeride für CERES,PALLAS,JUNO und VESTA von mir berechnet.

Die oskulierenden Anfangswerte wurden aus dem"AHNERT" Jahrgang 97 entnommen.Die Integration geschah streng als 14-Körperproblem. Die verwendeten SYMBOLE sind die gleichen wie in der Ephemeride des PAUL C.R. ARENDS VERLAGES 8219-RIMSTING.

Die Genauigkeit entspricht der der großen Planeten ME bis Ne.Stichproben mit Beobachtungswerten ergaben sehr gute Übereinstimmung.

Für CHIRON,der über längere Zeiträume gesehen eine "chaotische" Bahn hat, wurde im Bereich 07.01.600 v.Chr. bis 21.08.2199 A.D. ebenfalls eine numerisch integrierte Ephemeride berechnet mit Einbeziehung der Ablenkungen durch JU,SA,UR,NE,PL.

Die Massen von ME,VE,TE,MA wurden zur Sonne addiert.Das vereinfacht die Integration ohne die Genauigkeit nennenswert zu beeinträchtigen. Als oskulierende Bahnelemente wurden die von MARSDEN ermittelten Werte ( Stand 1979 ) verwendet.Diese sind etwa so genau angegeben als die für CE,PA,JN u. VS. verwendeten.

Die errechneten Werte können auch für Planetare verwendet werden.

Die Symbole der Kleinplaneten verschwinden wenn der vorgegebene Zeitrahmen überschritten ist,auch dann wenn sie vorgewählt sind.

Der hypothetische TRANS-PLUTO ( = ISIS ),nach den Bahn-Elementen von E.SEVIN ist ebenfalls unter ZUSATZ-PLANETEN rubriziert.Die Längen-Werte entsprechen der Ephemeride von NEIL F.MICHELSEN.

In dieser Ephemeride sind auch die astronomischen Voraussetzungen recht ausführlich beschrieben :"HAWKINS ENTERPRISING PUBLIKATIONS DALLAS/TEXAS 1978" Die Ephemeride von TH.LANDSCHEIDT gibt davon etwas abweichende Werte.Offenbar hat er etwas andere "Bahn-Elemente" verwendet.HERAUSGEBER :"F.BRANDAU PASSAU / 1984." Zu diesem sehr fragwürdigen "Planeten" ist folgendes zu sagen:

- Bisher ist TP nur berechnet aber nie beobachtet worden.
- Bei den Bahnelementen ist die Bahn-Neigung und die Knotenlänge unbestimmt. Für die Bahn-Neigung werden indes Werte zwischen 0 und ca.40 Grad unter verschiedenen Astronomen diskutiert,für die mittlere Anomalie weit auseinanderliegende Werte,bis zur gegenseitigen Opposition. Rektaszension,Deklination,Breite und Knotenlänge bleiben in den Tabellen deshalb offen.
- Es ist daher nicht sinnvoll,mit vorliegenden Werten minutengenau zu arbeiten,da der Planet,falls er wirklich existiert,um mehrere Grade abweichen oder gar in ungefährer Opposition dazu stehen könnte.Am besten läßt man die Finger davon !!

Für die "HAMBURGER Planeten",die nach astronomischem Ermessen kaum diesen Namen verdienen können,werden allgemein Kreisbahnen vorausgesetzt,d.h. von den insgesamt nötigen 7 Bahnelementen werden nur 2 verwendet,z.B. große Bahnachse und mittlere Anomalie ( = exzentrische Anomalie = wahre Anomalie = mittlere Länge = wahre Länge ).

Die "Bahnelemente" sind jetzt so berechnet,daß möglichst Übereinstimmung mit der gedruckten Ephemeride von RUTH BRUMMUND ( Verlag: WITTE-VERLAG Hamburg ) besteht.Diese Ephemeride kann wohl als die "offizielle" der Hamburger Schule gelten.

Präzision sollte man hier im übrigen nicht erwarten.Falls jemand behauptet mit diesen "TRANSNEPTUNERN" taggenaue Auslösungen zu erhalten,zeigt er nur, daß er sich noch nicht über die Grundlagen informiert hat,bzw sich über die Rolle des Zufalls nicht im Klaren ist.

Die Prog. ZUSATZ-PLANETEN-KOORD. schalten nicht diesen Modus für die Horoskop-Darstellungen ein.Dies geht nur über VORGABEN EPHEMERIDE... !

## HELIOZENTRISCHE VERSION EIN/AUS

Schaltet zwischen diesen Versionen um.

Das Gleiche bewirken Sie aus dem Hauptmenü heraus mit der Tastenkombination ALT + "H" .

## STATISTIK

Siehe zu diesem umfangreichen Modul die eigene "Erläuterung STATISTIK".

## GRAD-LISTE

Listet die Planeten-Positionen,die Häuser (HS 2,3,5,6.) und deren DIREKTE Halbsummen auf,samt einer Graphik,die die Besetzungs-Dichte der Grade anschaulich macht.Dieses Programm läuft auch heliozentrisch. Bei den Halbsummen ist immer der den beiden Faktoren nächst benachbarte Wert gewählt.Die Oppositions-Stelle dazu ist natürlich auch in der direkten Halbsumme,aber nicht aufgelistet.

## FIXSTERN-POSITIONEN

Gibt die Koordinaten von 62 Objekten des Fixstern-Himmels.Die Einflüsse der Eigenbewegung und der Präzession sind bei den Einzelsternen berücksichtigt, bei den Stern-Haufen,die durch *** gekennzeichnet sind,dem GALAKTISCHEN ZENTRUM und dem APEX,nur die Präzession.

Die Spalte "D(LJ)" gibt die ungefähre DISTANZ in Lichtjahren. Der max. Längen-Fehler entspricht,innerhalb unseres Jahrhunderts,einer Zeit-Sekunde.Er kann pro Jahrhundert um max.eine Zeitsekunde (=15 sec AR zunehmen). übrigens sind das galaktische Zentrum und der Apex (= Richtung der Bewegung des Sonnen-Systems) bei weitem nicht so genau definierbar wie oft der Eindruck erweckt wird.Ich habe die meist genannten Werte verwendet,man tut aber gut daran,dabei mit ca. 1 Grad Unsicherheit zu rechnen,besonders beim Apex ! Die Angabe z.B. 2Grad/120 * bei Sternhaufen bedeutet 2 Grad Ausdehnung/120 Sterne.

Weist eines der Objekte einen ASPEKT mit den Planeten des aktuellen Daten-Satzes auf,so wird die betreffende Zeile der Tabelle INVERTIERT und der betr. Planet in der Spalte "ASPEKTE" eingetragen.Gezählt werden: Konjunktion mit 2 Grad Normal-Orbis,Opposition mit 1 Grad,Trigon mit 2/3 und Quadrat mit 0.5 Grad Orbis.

Der Orbis kann über "VORGABEN HOROSKOP.." beeinflußt werden.Siehe auch Erl.4 ! Entsprechendes gilt auch für das folgende Prog. "ARABISCHE TEILE" ! In der Spalte "QUALITÄT" ist der Charakter des betreffenden Objekts mit 1 bis 3 Planeten-Symbolen dargestellt,wie er in etwa bei R.EBERTIN "Die Bedeutung der Fixsterne" zu finden ist.

Die Formulierungen,auch für APEX und GALAKTISCHES ZENTRUM (von TH.LANDSCHEIDT in die Astrologie eingeführt) sind nur als unverbindliche Arbeitshypothese auffassen !!

Überhaupt scheint mir ein "Einfluß" der Fixsterne besonders fragwürdig,da mit heutigen Fernrohren jeder Grad des Himmels mit Objekten bedeckt erscheint, die kaum auseinander zu halten wären.Die alten Astrologen konnten dies noch nicht wissen.

Falls sich diese trotzdem statistisch belegen ließen,müßte man eher bestimmten Raumrichtungen "Qualitäten" zuschreiben.Übrigens werden ja auch immer nur ekliptikale Längen in Betracht gezogen,auch bei Objekten die fast im Zenith stehen.

## ARABISCHE TEILE

Das Programm berechnet für den aktuellen Datensatz eine Liste von 37 "ARABISCHEN TEILEN" wie Glücks-Punkt usw."Klassisch" sind zum Teil diejenigen, die Planeten bis SA benutzen.Die anderen sind "Nachschöpfungen".Die jeweilige Formel ist immer mit angegeben.Einige sind mir von Bekannten genannt worden.Ich selbst habe damit noch kaum gearbeitet.

Die Haupt-Aspekte mit Radix-Faktoren werden,wie im vorhergehen Programm angezeigt.Die Werte können auch nach ekl.Länge SORTIERT werden (LEER-Taste). Soweit Häuserspitzen verwendet werden,bedeutet z.B. H12 die Spitze 12.Haus. Hv9 bedeutet dagegen Herr von Haus9.

Über den Wert dieser Punkte (auch "sensitive Punkte" genannt) möge sich jeder seine eigene Meinung bilden.

M.E. sollte man bei der Deutung die FORMEL in Bezug auf das Radix betrachten und den Namen nur als Assoziations-Hilfe gelten lassen,auch wenn man mit den klassischen Namen auch einmal "Volltreffer" registrieren mag. Die FORMEL ist,nach Tradition,zwischen Tag-und Nachtgeburt unterschiedlich. Ab vorliegender Version kann außer dieser traditionellen Version auch entweder nur die "Tag-Version" oder nur die "Nacht-Version" gewählt werden. Die derzeitige Liste ist nur als "DISKUSSIONS-GRUNDLAGE" gedacht.Zu gegebener Zeit werde ich vielleicht die mir zugegangenen Anregungen in einem Update berücksichtigen.

Sie können diese Liste vollständig mit EIGENEN DEFINITIONEN überschreiben, die in den Dateien ARABTEI1 und ARABTEI2 im Ordner INTERN abgelegt werden.

BRUNO MAHL hat neuerdings ein Buch über seine Untersuchungen an den A.T. veröffentlicht : " Die verborgene Macht der arabischen Punkte im Horoskop" ISDN 3-200-00071-6 Dort sind auch Quellen-Angaben zu finden.

## INGRESSE SO - MO - MC - AC

Mit diesem Programm werden die Zeitpunkte des Eintritts von Sonne oder Mond MC oder AC in die 12 Tierkreiszeichen berechnet.

In früheren Versionen konnte man diese,allerdings erheblich mühsamer,nur mit dem Programmteil "KORREKTUR" errechnen.

Man gibt ein Ausgangs-Datum ein und erhält dann die genannten 12 Zeitpunkte. Beim Mond gibt man etwa das Datum der Mitte des interessierenden Monats ein und erhält die Ingresse des betreffenden Mond-Umlaufs.

Bei der Sonne erhält man die 12 Zeitpunkte innerhalb des eingegebenen Kalenderjahres,bei MC und AC während eines Tages.

Die Rechnung erfolgt sehr genau,mittels Iteration,was besonders für den Mond genauere Werte gibt als Interpolation.Der Rechenaufwand ist allerdings sehr groß,sodaß einige Rechenzeit resultiert.

Die Angabe der Zeitsekunde ist etwas übertrieben.Sie wurde trotzdem belassen,da die Rechengenauigkeit erheblich besser als zeitminutengenau ist. Damit ist auch gewährleistet daß beim Wiedereinsetzen der errechneten Zeiten tatsächlich die exakten Zeichengrenzen in der Koordinatentabelle auftauchen.

Mit der "+" Taste,der Leertaste oder der Bild ^ Taste kann zeitlich weitergeschaltet werden,mit "-","R" geht es zeitlich rückwärts.

Bei den Ingressen von MC und AC ist folgendes zu beachten : Die für diese Ingresse ermittelte sekundengenaue Zeit ist infolge der schnellen Bewegung dieser Objekte zu ungenau um beim Wiedereinsetzen die genaue Winkelsekunde zu treffen.Eine Zeitsekunde entspricht bereits 15 Winkelsekunden.Das ist demnach kein Rechenfehler,da es sinnlos sein dürfte in der Astrologie mit Bruchteilen von Sekunden zu rechnen.

## ET aus UT   und   UT aus ET

Sind Hilfs-Routinen für astronomische Überlegungen.Ebenso das Programm

## "DATUM aus JD ".

Ebenfalls eine astronomische Hilfs-Routine zur Ermittlung des Kalenderdatums aus dem in der Astronomie verwendeten "JULIANISCHEN DATUM",das ab dem Datum 1.1.4713 v.Chr. 12 h Tag für Tag zählt.

---

ALLGEMEINES zum KALENDER:

Das KALENDER-Programm (Formeln z.B.aus MEEUS) umfaßt den gregorianischen und Julianischen Kalender.Die Umschaltung am 4.10.1582 ist im Programm enthalten.

Die Zählung der JD (="Julianische Tage") geht bis JD=0,was dem 1.1.4713 v.Chr. 12H nach julianischem Kalender entspricht.

FRÜHERE DATEN NICHT VERWENDEN !!

Da in vielen Ländern die Umstellung sehr viel später erfolgte,ist für die Jahre 1583 bis 1890 noch eine Abfrage zur Wahl des Kalenders eingeschaltet. Der gewählte Kalender wird in der Koordinatentabelle und den Horoskopen, wenn noch julianisch zu rechnen ist mit dem Kürzel JULIAN. angemerkt. Z.B. wurde im prot. Deutschland erst am 17.2.1700 umgestellt. Ab 1890 aufwärts wird der gregorianische Kalender immer vorausgesetzt. Falls doch noch Daten im julianischen Kalender vorliegen,müssen Sie also das julianische Datum selbst gregorianisch ausdrücken,also zum julianischen Datum 10 Tage addieren.Nur dann rechnet das Programm richtig.

Bei DATEN V.CHR. geben Sie bei "V.CH." ein "V" oder ein "-" ein. Das Minus ist hier nur deshalb zusätzlich gewählt,damit man weitgehend mit der rechten Hand eingeben kann -.Bei "JAHR" geben Sie nämlich die normale,von den Historikern verwendete Jahreszahl ein,ohne Vorzeichen ! In den Tabellen-AUSGABEN ist meist die Angabe V.CHR. durch das MINUS-Zeichen ersetzt,wobei die arithmetische ( = astronomische ) Zählung angegeben ist ! Dabei entspricht z.B. das Jahr -62 dem Jahr 63 v.Chr. Das kommt daher,daß die Historiker das Jahr 0 "vergessen" haben.Das Jahr 0 entspricht dem Jahr 1 v.Chr !! Im übrigen ist "v.Chr." auch als "VC" abgekürzt.

Im Tabellenkopf ist das "Julianische Datum" JD=.. und die in "Julianischen Jahrhunderten"(=36525 Tage) gemessene Zeitkoordinate T ab Datum 31.12.1899 12H UT angegeben.(Entspricht JD=2415020.0).Dies ist eine in der Astronomie übliche Zeit-Angabe für den hier verwendeten Bezugs-Zeitpunkt. Mit dem kleinen Programm " DATUM aus JD " kann man nach Eingabe des vollen julianischen Datums",das Datum im jeweiligen Kalender ermitteln.Z.B. ist für JD=1507900.13 ( UT ) das Datum der 28.5.-584 (=585 v.Chr.) 15 h 7.2 Min (UT).

Eine umfangreiche Dokumentation zu Kalender- und Zeit-Bestimmungen wurde von Herrn B.MAHL dankenswerterweise für den Computer aufbereitet und kann über TEXTE LESEN im Ordner \HORCOM\ZEITBEST gelesen werden oder auch direkt aus der EINGABE-BOX bei NEU-EINGABE..
