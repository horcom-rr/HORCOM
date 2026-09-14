# Erläuterung Horoskope

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P), unverändert bis auf die Formatierung.*

## VORGABEN HOROSKOP ÄNDERN

Hier sind die Parameter für die Horoskop-Darstellungen einzustellen. U.a. kann hier auch der Modus "MUNDAN-HOROSKOP" eingestellt werden.Dies ist eine TOPOZENTRISCH-ÄQUATORIALE Darstellungsart :

In der "MUNDANEN"-Darstellung wird der jeweilige direkt zu sehende HALBE TAGbzw. NACHT-Bogen im HORIZONT-System ins Verhältnis zu je 90° AR gesetzt. Die Placidus-Häuser haben bei dieser Projection jeweils 30° auf dem Äquator. Diese Darstellungsart hat besondere Bedeutung für die MÜNCHNER RHYTHMENLEHRE von W.DÖBEREINER,worauf m.W. zuerst E.R.DOSTAL aufmerksam gemacht hat.Der Äquator als Bezugssystem sollte genauere Auslösungen ergeben,insbesondere bei höheren geogr. und eklipt. Breiten.Das Häusersystem von Placidus ist hierbei obligatorisch.Einige Einzelheiten sind z.B. in dem Handbuch zum ASTRO-Taschenrechner "HORUS" dargestellt ( Verfasser E.R.DOSTAL ). Siehe auch bei : "Die Mundane Hausposition" ,herausgegeben von AKIMOTO, Nov. 1982.

Die Mundan-Darstellung hat engen Bezug zu den PRIMÄR-Direktionen,wie sie bei E.C.KÜHR ( "Berechnung der Ereigniszeiten" ) beschrieben sind.

Wieweit sonstige Anwendungen der MUNDAN-Version sinnvoll sind,sei dahingestellt.

Hinsichtlich der Aspekte innerhalb der M.R. habe ich diese auch in der Mundan-Darstellung ekliptikal ausgewertet und,um Verwirrungen zu vermeiden, nicht eingezeichnet.Dieser Punkt scheint klärungsbedürftig.

Zunächst sind im MUNDAN-Modus PROGRESSION und TAGES-HOROSKOP noch zugänglich belassen worden.

Die übrigen Punkte unter "VORGABEN HOROSKOP ÄNDERN" sind selbst erklärend. Die drei letzten Punkte erlauben die Farbwahl des Tierkreis-Ringes und der Aspektlinien.Das verlangt einige Übung und einige Experimente. Bei 16 Farben sind die feineren Farbnuancen nicht möglich ! Schraffierte Flächen brauchen weniger Farbe beim Ausdruck von Hardcopys. Ihre eigenen Einstellungen der FARBEN,SCHRAFFUR,LINIEN-FARBEN und STILE werden gespeichert.Die HORCOM-STANDARD-Einstellungen sind fest vorgegeben und können immer leicht reaktiviert werden.

Falls Sie noch keine eigenen Farben festgelegt haben,sollten Sie immer HORCOM-Standard wählen,wenn nicht sind die Farbflächen möglicherweise schwarz !

Mittels der Bild-nach-oben-Taste oder "R" können die vorhergehenden Dialoge erreichen,falls Sie etwas übersehen haben.

****************************************************************************

## HOROSKOP-GRAPHIK

Zeichnet GEOZENTRISCHE bzw. HELIOZENTRISCHE Horoskope,je nach den vorgewählten Parametern.U.a. kann der ANFANG des Horoskops,links außen,beliebig gewählt werden.

Wahlweise kann das HOROSKOP auf den BILDSCHIRM oder als DRUCKER-GRAPHIK auf den Drucker ausgegeben werden ( falls die DRUCKER-OPTION von HORCOM aktiv ist ).In diesem Fall sehen Sie am Bildschirm nichts und müssen warten bis der Drucker die Graphik ausgibt.Das dauert etwa eine Minute ( je nach Prozessor ) da das Bild erst für den Drucker umgerechnet werden muß ! Die Farb-Schraffuren des Bildschirm-Ausgabe werden von den meisten Drucker-Treibern nicht reproduziert,bzw. ignoriert.

Als DRUCKER-GRAPHIK ist auch eine DINA4 - Ausgabe vorgesehen die immer auch eine HALBSUMMEN-Liste enthält.Da in HORCOM-Dateien immer nur die UT gespeichert ist wird bei dieser Version,die zur Weitergabe an Laien gedacht ist, noch die ZONENZEIT,bzw. SOMMERZEIT abgefragt sodaß bei eventuellen DATUMS-Überschneidungen auch das "bürgerliche" Datum zusätzlich angegeben werden kann.

Das Horoskop-DIAGRAMM kann verschiedene AUSWERTUNGEN enthalten,nämlich : Eine Aspekt-Verteilung bis zur 16.Kreisteilung (entspr. 22.5 Grad),falls der Orbis nach HORCOM-Manier ( siehe unten ) berechnet wird,wenn nicht nur bis zur 12. Teilung.

Eine HALBSUMMEN-LISTE: Direkt,Quadrat und Halbquadrat-Aspekt,wobei der Orbis respektive 1,1/2,1/4 Grad beträgt,wenn nach HORCOM-Manier gerechnet wird,wenn die Orbes selbst festgelegt werden,in der entsprechenden Stufung.

Die Anzahl der Halbsummen kann wahlweise mit einem ZÄHLER schnell überschaut werden.

Bei ZEITWANDERN oder beim schnellen Durchmustern von Datensätzen in STATISTIK werden die Halbsummen zusätzlich aufsummiert und das jeweilige arithmetische Mittel der aufgelaufenen Summen angezeigt.

Eine Gewichtung der "ELEMENTE" nach "Punkten" :

Bis April 97 mit den folgenden Festwerten :

```text
  SO MO ME VE MA JU SA UR NE PL DR DS  KLEIN-PL. AC MC DC IC
   6  6  3  3  3  2  2  1  1  1  1  1  0...       6  6  4  4   Punkte
Planeten im ERSTEN HAUS und der GEBURTSHERRSCHER werden DOPPELT gewichtet.
```

Eine Gewichtung nach "KARDINAL-FIX-GEMEINSCHAFTLICH = KARD/FIX/GEM" nach "Punkten" mit den gleichen Bewertungen wie bei Elementen,allerdings werden IC und DC hier nicht extra gezählt,da dies einer Überbewertung gleichkommen würde.

Seit Mai 97 können die einzelnen Puntwerte selbst zwischen den Grenzen 0...9 festgelegt bzw. die Histogramme auch ganz entfernt werden ( bei 'VORGABEN HOROSKOPE ÄNDERN' ).

Die Histogramme können ab Mai 97 zusätzlich auch die Häuserbesetzung berücksichtigen.Diese ist oberhalb der Zeichenbesetzung als getrennte Säule zu sehen.Oft gleicht die Häuserbesetzung einen Mangel in der Zeichenbesetzung aus.

Hinsichtlich des ORBIS verfährt HORCOM normalerweise wie folgt :

```text
          SOLL-Winkel - ORBIS < IST-Winkel < SOLL-Winkel + ORBIS,
               ORBIS = GRUND-WINKEL/30
```

GRUND-WiNKEL ist dabei der Winkel einer Reihe,der sich durch Teilung von 360 Grad durch eine fortlaufende ganze Zahl ergibt.

Die Reihe der TEILER und entspr. GRUND-WINKEL bis zur 16.Teilung ist demnach:

```text
TEILER     :   1       2       3       4       5        6        7       8
GRUNDWINKEL: 360     180     120      90      72       60       51.43    45
TEILER     :   9      10      11      12      13       14       15      16
GRUNDWINKEL:  40      36      32.73   30      27.69    25.71    24      22.5
```

Z.B. gilt:

WINKEL 135 Grad hat GRUND-WINKEL 45 Grad,also ORBIS = 45/30 = 1.5 Grad

```text
        360 (Konj.)               360                  = 360/30 =12
        120 (Trig. )              120                  = 120/30 = 4
        150 (Quinkunx)             30                  =  30/30 = 1
         90 (Quadrat)              90                  =  90/30 = 3
         30 (Halbsext.)            30                  =  30/30 = 1
```

Dieses Verfahren wurde m.W. von J.ADDEY empfohlen und ist auch bei H.J. WALTER ("ENTSCHLÜSSELTE ASPEKT-FIGUREN") für das "WELLENMODELL" angewendet, allerdings dort mit größerem Orbis-Faktor.Es wird jedem Planeten ein bestimmter Aspekt zugeordnet.

Auch die Gedanken von LUTZ RATHKE zu diesem Thema haben mich neuerdings in diesen Vorstellungen bestärkt.Seine Aspekt-Lehre ist sehr interessant und kann auch klassischen Astrologen zur Kenntnisnahme nur empfohlen werden. Er findet z.T. andere Zuordnungen zu den Planeten als H.J.WALTHER. Manche Aspekte kommen als VIELFACHE in mehreren Aspekt-Reihen vor,die Konjunktion in allen,die Opposition in 45er,30er,60er,90er,180er Reihe usw.Dies macht die unterschiedliche "STÄRKE" der Aspekte plausibel.

Durch die Proportionalität des Orbis zum Grund-Winkel ist auch gewährleistet, daß "SCHWACHE" Aspekte einen KLEINEN,"STARKE" einen GRÖßEREN ORBIS haben. Auch eine GEOMETRISCHE "GLEICHWAHRSCHEINLICHKEIT" aller Aspekte ist hinreichend plausibel,wenn man diese als "Reihen" bzw. als "Wellenmodell" nach H.J.WALTHER für einen bestimmten Teiler betrachtet.Z.B. kommt das Sextil noch im Trigon und der Opposition vor,sodaß es berechtigt scheint, z.B. dem "reinen" Sextil einen kleineren Orbis zu geben als dem Trigon oder der Opposition.

Betrachtet man nur einzelne Aspekte,so hätte jeder für einen gegebenen Orbis die gleiche geometrische Wahrscheinlichkeit.Würde man aber alle Aspekte mit gleichem Orbis versehen,so wären diese bei höheren Teilern nicht mehr unterscheidbar.

Auch dies scheint mir für die oben beschriebene Manier der Orbis-Wahl zu sprechen.

Ein anderes sinnvolles Konzept wäre,für alle Aspekte die GLEICHE WAHRSCHEIN-LICHKEIT = HÄUFIGKEIT zu fordern.

Dies ist am besten EMPIRISCH zu machen,da die astronomische A-Priori-Wahrscheinlichkeit geozentrisch ein recht kompliziertes Thema ist.

Mittels des ASPEKTE-ZÄHLERS in "ZEITWANDERN" wurden folgende Orbes ermittelt, die zwischen 1930 und 2050 GEOZENTRISCH,im Mittel über ca. 2000 Zeitpunkte etwa die gleiche Wahrscheinlichkeit aufweisen :

```text
Teiler      :   1    2    3    4    5    6    7    8    9    10    11    12
```

```text
Grundwinkel :  360  180  120  90   72   60   51.4  45   40   36    32.7  30
```

```text
Orbis       :  5.4  6.8  3.1  3.0  1.6  2.6  1.0  1.5   1.0  1.5   0.6   1.0
```

HELIOZENTRISCH ergeben sich auf die gleiche Weise :

```text
Teiler      :   1    2    3    4    5    6    7    8    9    10    11    12
```

```text
Grundwinkel :  360  180  120  90   72   60   51.4  45   40   36    32.7  30
```

```text
Orbis       :  6.51 5.36 3.18 3.0 1.58 2.01 1.02  1.51  1.0 1.53   0.58 1.50
```

Nach dem obigen "WELLENMODELL" = HORCOM - Normaleinstellung ergibt sich folgende Reihe:

```text
Teiler      :   1    2    3    4    5    6    7    8    9    10    11    12
```

```text
Grundwinkel :  360  180  120  90   72   60   51.4  45   40   36    32.7  30
```

```text
Orbis       :   12   6    4    3   2.4  2.0  1.71  1.5 1.33  1.2   1.09  1.0
```

Für das Quadrat ergeben alle drei Reihen den gleichen Wert.

Die Absolutwerte sind willkürlich gewählt.

Unter VORGABEN HOROSKOP ÄNDERN" kann aber ein FAKTOR (10 bis 300%) vor dem "NORMAL"-Orbis eingeführt werden,der in ALLE Orbes eingeht.Dies ist vor allem dann sinnvoll,wenn man in bestimmten Fällen zu viele Aspekte erhält und den Überblick zu verlieren droht.Dann kann man damit ALLE Orbes verkleinern usw.

Infolge von LANG ANDAUERNDEN Aspekten zwischen langsam laufenden Planeten können allerdings die Wahrscheinlichkeiten während solcher Phasen durchaus zeitweise erheblich größer oder kleiner sein.

Für wissenschaftliche Untersuchungen wird es daher notwendig sein,solche Verhältnisse für den Untersuchungszeitraum abzuklären,wobei das Zeitwandern mit Aspekte-Zähler jeweils nützlich sein kann !

Für die SPIEGELUNG an der Achse 0 AR-LI oder 0 CN-CP wurde der Orbis durchweg,auch bei M.R., zu 2 Grad angenommen,für die DIREKTEN HALBSUMMEN zu 1 Grad,falls die Orbis-Wahl HORCOM überlassen wird.

Die "Freie Wahl" der Orbes der Grund-Winkel habe ich seit Januar 94 auf mehrfachen Wunsch von Kunden-Seite nun ebenfalls möglich gemacht. Falls Sie von dieser Möglichkeit Gebrauch machen wollen,was Sie nur nach einiger Praxis mit HORCOM tun sollten,gilt folgendes:

*Als Vorgabe vor einer ERSTEN eigenen Festlegung sind die oben genannten

*Es werden dann nur die Teiler von 1 bis 12 durchgezählt,also alle Grundwinkel von der KONJUNKTION bis zum HALBSEXTIL.

*Alle Aspekte werden EXCLUSIV gewertet,d.h. ein Trigon wird nicht auch als BISEXTIL,eine OPPOSITION nicht als BIQUADRAT gewertet usw.,wie es bei oben geschildertem "WELLEN"-Konzept der Fall ist,dem ich persönlich mehr zuneige, das aber bei den meisten Astrologen nicht üblich ist.

*Sie können nun unter "VORGABEN HOROSKOP ÄNDERN" die Ihnen richtig erscheinenden Orbes eintragen.Die von mir bei dieser Zählweise für angemessen erachteten Orbes,sind als Vorgabe-Werte mit aufgeführt,Sie können aber auch davon abweichende Werte festlegen.Man sollte,besonders wenn auch grössere Teiler ( >6 ) eingetragen werden,keine wesentlich grösseren Orbes verwenden als vorgeschlagen.Bei erheblich zu großen Orbes erfolgt Fehlermeldung,da sich sonst ein unüberschaubarer Wust von Aspekten in den Ausgabegraphiken ergeben würde.

Für Aspekte,die Sie grundsätzlich nicht beachten,können Sie NULL eingeben. *Für die M.R. von W.DÖBEREINER gelten für Quadrat,Trigon und Opposition 4 Grad für Konjunktion 6 Grad als Richt-Werte (s.Literatur zur M.R.). Praktizierende Anhänger dieser Schule sollten demnach diese Winkel eintragen.Das Sextil wird in der M.R. meist nicht beachtet,kann aber in HORCOM hinzugewählt werden.

*Der allgemeine Orbis-Faktor und eventuell eingegebene Planeten-Faktoren gehen immer ein,wenn sie von 1 bzw. 100% abweichen ! ( siehe unten ) *Hinsichtlich der zusammengesetzten Aspekte wie 3*45 Grad oder 5*30 Grad ( = 1 1/2-Quadrat bzw. Quinkunx ) wird der Orbis des zuständigen Grund-Aspekts zugrunde gelegt,wie auch beim obigen "WELLEN"-Konzept. Falls Sie z.B in die Quinkunx verliebt sind und diese öfter mal sehen wollen,können Sie beim Grundwinkel 30 Grad einen grösseren Orbis eintragen. Prompt werden Sie auf Schritt und Tritt auf Ihren geliebten Aspekt stoßen. Ob das zu "Erkenntnissen" führt,darf bezweifelt werden.

*Auch für SPIEGELUNG an den Kardinal-Achsen und die HALBSUMMEN können die Grund-Werte verändert werden.

Die PLANETEN-ORBES können einzeln gewichtet werden.

( Bei "VORGABEN HOROSKOP ÄNDERN" ).Der Normalwert beträgt jeweils " 100 % ". Es können ( auf eigene Verantwortung ) beliebige Prozentsätze eingegeben, wobei z.B. 200% bedeutet,daß der betreffende Planet mit doppeltem Gewicht, also auch im Mittel doppelt so häufig auftritt.Der Wert 200 % entspricht also einem Bewertungs-Faktor 2.

Entsprechend bedeutet z.B. 50% einen Bewertungs-Faktor 1/2 .Er wird dann nur mit der halben Wahrscheinlichkeit auftreten.

Löscht man die Einzel-Vorgaben,so werden wieder alle Planeten mit dem gleichen Gewicht 100 % gewertet,entsprechend dem Faktor 1.

Wollen Sie also einen Planeten aspektmäßig "vergessen",so geben Sie 0% ein. Er wird dann nicht mehr gezählt.

Der oben erwähnte allgemeine Orbis-Faktor und die den einzelnen Planeten zugeordneten Faktoren treten immer als Produkt in Erscheinung,was sich natürlich nur bemerkbar macht,wenn zumindest einer der beiden Faktoren von 1 bzw. 100% verschieden ist.Sind z.B. beide Faktoren 50% ,so ist der resultierende Faktor 25% usw.

Ist Ihnen die Sache mit diesen Orbis-Faktoren zu kompliziert,übergehen sie einfach die betreffenden Abfragen.Alle Faktoren sind dann 1 bzw. 100% und sie brauchen sich weiter darum nicht zu kümmern.

Die von HORCOM vorgeschlagene Aspekt-Bewertung hat sich in meiner Praxis gut bewährt und ergibt im Horoskop überschaubare und deutungsfähige Figuren.

Die ASPEKT-LINIEN,die Sie im HOROSKOP sehen wollen können Sie bis zur

12. Teilung einzeln vorgeben.Haben Sie eine Linie markiert,so erscheint

diese neben der Auswahl-Box,wie sie im Horoskop aussieht.Ausserdem erscheint in der betreffenden Zeile der Auswahlbox das Symbol *---* als Kennzeichen,daß der betreffende Aspekt im Horoskop erscheint. Wollen Sie auch zusammengesetzte Aspekte wählen,ist es ratsam vorher als maximalen Teiler mindestens die 12 wählen.Z.B. erfordert die QUINKUNX =5*30 das Halbsextil als Grundwinkel und daher den Teiler 12.

Eine KONJUNKTION,innerhalb des gegebenen Orbis wird durch einen kleinen Kreis angezeigt.

Haben Sie einen Monochrom-Monitor wird es etwas schwierig sein,die verschiedenen Linien auseinanderzuhalten.Hier ist der Farbmonitor von Vorteil. Wird der "SCHWARZE MOND" gewählt,so wird die Apsidenlinie der Mondbahn als gestrichelte Linie eingezeichnet,mit dem Symbol des Schw.Mondes am APOGÄUM. AG = Länge des Apogäums = "Schwarzen Mondes".

Der GEBURTSHERRSCHER wird am Bildschirm invers hervorgehoben. Bei den DRUCKER-GRAPHIKEN wird er farblich hervorgehoben,GRÜN bei SO,MO,ME, VE,JU und ROT bei MA,SA,UR,NE,PL.

Die auf Grad/Min bzw. /SEK skalierten PLANETEN-LÄNGEN sind links im HORO-SKOPDIAGRAMM eingetragen.Dahinter steht die TÄGL.BEWEGUNG TB" .Negatives Vorzeichen = Rückläufigkeit.

Die Abkürzungen bei LÄNGE:

(W)= WAHRE (A1)=APPARENTE1 (mit Licht-Laufzeit)

A2 = APPARENTE2 (LLZ+JÄHRL.Aberration).

P = MIT PARALLAXE .

Beim MONDKNOTEN u.APOGÄUM bedeuten :

M = Mittelwert W=Wahrer Wert Normalerweise wird der GERUNDETE GRADWERT ins Horoskop eingeschrieben.überschneidungen mit den Symbolen sind dabei nicht immer auszuschliessen.Unter VORGABEN kann dieser Gradwert auch eliminiert werden.

Detail-Auskunft (z.B. "Parallelen") gibt PLANETEN-KOORDINATEN... Unter der Längen-Tabelle sind die HÄUSERGRENZEN eingetragen.Im Detail,siehe HÄUSER-TABELLE

Bei der BILDSCHIRM-Arbeit ist es möglich EINZELNE PLANETEN im HOROSKOP auszuwählen,sodaß NUR DIESE sichtbar sind.Oder einzelne Planeten können nur ROT markiert werden:

Dazu aus der BILDSCHIRM-HOROSKOP-Graphik die RECHTE MAUSTASTE drücken und dann entsprechend auswählen !

Ein eventuell vorgewählter FIXPUNKT kann nur über VORGABEN EPHEM.ÄNDERN beeinflußt werden !

***************************************************************************

## ASPEKTARIUM

Schlüsselt die Aspekte mit Teiler 1 bis max.16 im einzelnen auf. Der maximale Teiler ist in drei Stufen wählbar : 8 / 12 / 16, 16 nur dann, wenn die Orbis-Zählung HORCOM überlassen wird.

Oberhalb der Diagonalen sind die tatsächlichen Winkel-Abstände der Planeten in Grad/Min eingetragen,soweit Aspekte gefunden werden.

Die Aspekte KONJ.,OPP.,TRIGON,QUADRAT sind hier durch kleine Rechtecke, jeweils links unten,markiert,die weiteren Aspekte durch ein kleines Dreieck.

Unterhalb der Diagonalen ist das eigentliche Aspektarium :

Die HAUPT-Aspekte sind mit den bekannten SYMBOLEN eingetragen,die Aspekte mit Teiler 5 bis 16 sind,evtl.als gerundete,SOLL-Gradwerte eingetragen.Der TEILER ist DARÜBER klein eingetragen.Hinsichtlich ORBIS,siehe oben. Die Zahlen in der Diagonalen geben die ANZAHL DER ASPEKTE für den darüber stehenden Faktor an,womit die stark aspektierten Planeten sofort ins Auge fallen.Bei mehr als 5 Zusatzfaktoren werden hier auch die Aspekte gezählt, deren einer Partner in der Graphik keinen Platz mehr findet. Dieses Programm läuft auch heliozentrisch.

*****************************************************************************

## HALBSUMMEN-GRAPHIK

Dieses Programm ist für Anhänger der R.EBERTIN-Schule und für die HAMBURGER Schule gedacht.Die Halbsummen sind jeweils dem zentralen Faktor zugeordnet. Die Buchstaben auf der Achse bedeuten :

D=Direkt,Q=Quadrat,H=45 Grad,V=22.5 Grad.

Der Normal-Orbis ist dabei respektive: 1,1/2,1/4,1/8 Grad.

Der Orbis kann unter "VORG.HOR.ÄNDERN" evtl. verändert werden.Ist die letzte Halbsumme in einer Skala eingerahmt,bedeutet dies,daß beim vorgegebenen Orbis nicht mehr alle Werte Platz hatten.

Nach Drücken der Leertaste werden die Tabellen (Skalen) sortiert und zwar so,daß Vielfache von 45 Grad nebeneinander einsortiert werden und somit "PLANETENBILDER" ersichtlich werden.Das Sortier-Kriterium in Grad ist jeweils am unteren Ende der Skala abzulesen.Ist LA die absolute Länge des zentralen Faktors in Grad,so ist also diese Zahl LA*8 ,reduziert auf 360 Grad.Dabei ist zu beachten,daß der Orbis,bezogen auf diese Zahlen,ebenfalls um den Faktor 8 vergrößert ist !!

Liegen zwei Halbsummen_achsen weniger als 1 Grad Grund-Orbis auseinander so werden die betreffenden Zahlen ROT eingezeichnet !

Die Anzahl der Halbsummen kann wahlweise mit einem Zähler schnell überschaut werden.

***************************************************************************

## MULTIPLE DIREKTIONEN nach STEPHAN.A. LEHRIEDER

Haben z.Teil einen Zusammenhang mit den HARMONICS nur daß hier auch und gerade NICHT ganzzahlige Multiplikatoren verwendet werden,nämlich das LEBENSALTER in Dezimalangabe : Zahl der Tage zwischen Geburt und Ereignis dividiert durch die Länge eines tropischen Jahres ( 365,2422 ) ergibt LJ = Lebensjahre.

Es gibt bisher folgende Spielarten :

MULTI 1 :

Die Horoskopfaktoren ( Planeten und Häuser ) werden nach ihrer Stellung im jeweiligen ZEICHEN ( 0 bis 30 Grad ) mit LJ multipliziert,zur jeweiligen ekliptikalen Länge addiert und auf 360 Grad reduziert.

MULTI 2 :

Die Horoskopfaktoren werden nach ihrer Stellung im TIERKREIS ( 0 bis 360 Grad ) mit LJ multipliziert,zur jeweiligen ekliptikalen Länge addiert und auf 360 Grad reduziert.

MULTI 3 :

Die Horoskopfaktoren ( Planeten und Häuser ) werden nach ihrer Stellung im jeweiligen ZEICHEN mit LJ multipliziert,alle zum gleichen VORGEWÄHLTEN Horoskopfaktor ( ekl. Länge ) addiert und auf 360 Grad reduziert.

MULTI-0-OST :

Die Horoskopfaktoren ( Planeten und Häuser ) werden nach ihrer Stellung im jeweiligen ZEICHEN mit LJ multipliziert,alle zum NULLPUNKT der von ihnen beherrschten Zeichen addiert,wobei nach LEHRIEDER folgende Zuordnungen gelten : SA herrscht in AQ,JU in PS,MA in AR,VE in TA ME in GM. Die übrigen Planeten entsprechend ihren beherrschten Zeichen. CHIRON wird als Herrscher von Jungfrau gewertet !

MULTI-0-WEST :

Die Horoskopfaktoren ( Planeten und Häuser ) werden nach ihrer Stellung im jeweiligen ZEICHEN mit LJ multipliziert,alle zum NULLPUNKT der von ihnen beherrschten Zeichen addiert,wobei nach LEHRIEDER folgende Zuordnungen gelten : ME herrscht in VI,VE in LI,MA in SC,JU in SG,SA in CP. Die übrigen Planeten entsprechend ihren beherrschten Zeichen. Für AC wird 0° AR, für MC 0° CP, für DR 0° CN, für AG 0° CN und für den GLÜCKSPUNKT 0° AR vorgegeben.

CHIRON wird als Herrscher von Jungfrau gewertet !

Seit August 1998 ist innerhalb HORCOM5P/7P eine weitere Version der Multiplen probeweise eingeführt,auf Anregung und in Absprache mit Herrn Lehrieder, die mit MULTI-ARC bezeichnet sind.

Hierbei sind die ekliptikalen Abstände der Planeten voneinander,bezogen auf einen Bezugspunkt ( wie bei MULTI 3 ) ausgewertet.Im Effekt ähnelt diese Version dem MULTI 3,nur daß hier nicht die Stellung im Zeichen mit den Lebensjahren multipliziert wird,sondern die Stellung im Tierkreis. Für den Bezugspunkt 0 Widder ist dies identisch mit MULTI 2. Erfahrungen damit gibt es noch nicht.Forscher an die Front !

Der aufsteigende ( mittlere ) MONDKNOTEN wird bei den Multiplen wie ein Planet behandelt und der absteigende zu +180° dazu DEFINIERT ! Die HÄUSER werden normalerweise wie Planeten umgerechnet.Dabei wird sich evtl. die Anordnung der Häuser verändern ! Auch der GLÜCKSPUNKT wird wie ein Planet umgerechnet !

Wahlweise kann in HORCOM,aufgrund des neuen MC,mit der zugehörigen Sternzeit auch ein reguläres Häusersystem ( je nach Vorwahl ) berechnet werden. Dabei wird gegebenenfalls auch der GLÜCKSPUNKT aufgrund des neuen AC neu berechnet.

Übrigens ist wichtig zu wissen,daß HORCOM bei der Häuserberechnung die NUTATION der Erdachse immer berücksichtigt hat.Dies macht Maximal-Amplituden von der Grössenordnung bis ca. 30" aus.Bei Vergleich mit anderen Berechnungen sollte man dies beachten.

Bei MULTI-0-OST und MULTI-0-WEST werden als "beherrschte Häuser" die entsprechenden Zeichen verwendet,also z.B. für das 10. Haus Steinbock usw. In der Ausgabe der MULTIPLEN werden links im Bild die Aspekte zwischen den RADIX- und MULTIPLEN-Faktoren dargestellt,rechts daneben zwischen den MULTIPLEN untereinander,wobei ein sehr kleiner Orbis von +- 0.2° schulgerecht ist.R steht für RADIX M für MULTIPLE.

Darunter werden die mit dem gleichen Orbis ermittelten HALBSUMMEN zu den ZEICHEN-Grenzen und den HÄUSER-Grenzen ( für RADIX- bzw. MULTIPLE Häuser ) dargestellt.

Anschliessend kann die Geburtszeit verändert werden ( ZEITWANDERN ) und dabei die Aspekte studiert werden.

Dies ist z.B. für KORREKTUR der Geburtszeit sehr nützlich.

Der ganze Programmteil "MULTIPLE" ist auf die Arbeit am Bildschirm abgestellt.

Bildschirmdokumente sind daher einstweilen vor allem als HARDCOPY gedacht. Den letzten Bildschirm,vor Verlassen des Programms kann man auch als Drucker-Graphik ( DIN A5 ) ausgeben.

Will man dabei Farben sparen,ist es zweckmäßig,unter "VORGABEN HOROSKOP ÄNDERN" die Farbwahl WEIß zu treffen !

Die Arbeit mit MULTIPLEN setzt eine sehr genaue Ephemeridenrechnung voraus. Im derzeitigen HORCOM ist dies für SO,MO,ME....NE allgemein,für PLUTO und CHIRON nur zwischen 600 v.Chr. und 2200 A.D. gegeben,bei CE,PA,JN,VS zwischen 1500 A.D. und 2100 A.D.Allerdings sollte man letztere mit Vorsicht verwenden da sie aus astronomischen Gründen zwar sehr genau berechnet wurden,aber wegen der Vielzahl ( ca. 10000 ) beeinflussender Körper,die nicht in die Rechnung eingehen ( auch bei Profis nicht ),über längere Zeiten mit Abweichungen gerechnet werden muß.

Sonstige Zusatz-Planeten müssen für die MULTIPLEN ausfallen.Nur der GLÜCKS-PUNKT und das MITTLERE APOGÄUM des MONDES ( SCHWARZER MOND ) sind als Zusatzfaktoren möglich.Analoges gilt für den Mondknoten. Für den SCHWARZEN MOND und den MONDKNOTEN werden daher immer die ( sehr genauen ) MITTELWERTE berechnet,unabhängig davon,was allgemein eingestellt ist.

HYPOTHETISCHE "Planeten" wie TRANSPLUTO ( ISIS ) oder die der Hamburger Schule entfallen m.E. prinzipiell für diese Methode,da von sekundengenauen Positionen hier natürlich auch nicht entfernt die Rede sein kann.

ACHTUNG !! Sehr wichtig ist es zu beachten,welche Berechnungs-Modalitäten für die Ephemeridenrechnung jeweils eingestellt sind.Die Berechnung "MIT PARALLAXE" ergibt für den Mond und die inneren Planeten erheblich abweichende Werte ! Der "multiple" Mond mit Parallaxe kann z.B bei Lebensalter von 60 Jahren um mehr als 2 Zeichen abweichen von einem ohne Parallaxe berechneteten.kann der entsprechende Unterschied bis zu 45' betragen. Astrologisch "richtig" sind zweifellos die Werte MIT Parallaxe,da das Ereignis nicht im Erdmittelpunkt sondern am Ereignisort auf der Erdoberfläche stattfindet. Sie können forschen wieweit dies bedeutsam sein könnte,sollten aber immer wissen was Sie tun.

Hr. LEHRIEDER hat seit Mitte 1998 seine Erfahrungen mit den MULTIPLEN in Buchform ( im Selbstverlag ) in sehr schöner Form herausgebracht : Edition BONASTRO Stephan A.Lehrieder ,am Kavierlein 12,D-90765 Fürth. Preis ca. DM 78.-

HARMONICS :

Ergeben sich aus einem Radix indem alle Winkel mit einem ganzzahligen Faktor multipliziert und auf 360 Grad reduziert werden.Die 4.HARMONIC ist z.B. im wesentlichen mit dem bekannten 90-Grad-Kreis identisch.Im Gegensatz zu den oben beschriebenen MULTIPLEN werden Bei den HARMONICS die multiplizierten Positionen nicht auf Bezugswerte aufaddiert,sondern stehen für sich. Die Darstellungsform wurde nun den MULTIPLEN analog gefaßt.Siehe oben. Der Orbis für die Anzeige der Aspekte und Halbsummen ist normalerweise ( bei ORBIS-FAKTOR 1 ) zu 1° festgelegt.

HARMONICS und ASPEKTARIUM haben eine enge Beziehung:Ist z.B. der Aspekt mit dem Teiler 5 besonders häufig,so stehen die beteiligten Planeten in der 5.Harmonic in Konjunktion usw.

Die oben bei MULTIPLE genannten allgemeinen Gesichtspunkte gelten auch hier. *****************************************************************************

Für COMPOSIT, COMBIN und DOPPEL-KREIS STELLEN SIE ZUNÄCHST SICHER,DAß ZWEI .. DATENSÄTZE VORHANDEN SIND und wählen dann das entspr. Programm an. Von dort aus wird dann beim DOPPEL-HOROSKOP zur Aktivierung der Datensätze nacheinander aufgefordert.Die Prozedur ist etwas umständlich,ermöglicht aber die Verwendung beliebiger Datensätze,z.B. RADIX u.SOLAR. Die Aufforderung zur Eingabe ist dadurch betont,daß der Maus-Pfeil zum SENKRECHTEN DOPPELPFEIL wird.Solange dieser zu sehen ist,wird jede Aktivierung dem jew. DOPPEL-Programm zugeführt ! Die Aktivierung geschieht immer über das Haupt-Menü,Spalte "EIN-AUSGABE".

Beim COMPOSIT geben Sie zwei Datensätze hintereinander ein,beim COMBIN können es bis zu 5 sein.

## COMPOSIT

Berechnet und zeichnet ein Composit-Horoskop zweier Partner. Die "Planeten" sind HALBSUMMEN der beiden Partner-Werte.

Von den beiden möglichen Werten der Halbsumme wird jeweils der den beiden Faktoren nächstliegende verwendet.

Das Composit ist astronomisch nicht "echt".Es arbeitet daher einstweilen nicht mit sonstigen Auswerte-Programmen zusammen.

Das HÄUSERSYSTEM eines COMPOSIT ist problematisch.In HORCOM5P/7P sind daher für Studienzwecke jetzt drei Möglichkeiten für die Berechnung der Häuser vorgesehen :

1. Es wird die mittlere Sternzeit der Partner-Horoskope berechnet sowie die

```text
   Mittelwerte der geogr. Länge und Breite berechnet und dann damit das
   Häusersystem in üblicher Weise.
```

2. Als zweite Möglichkeit wird die Methode von ROBERT HAND angewandt.

```text
   Hierbei wird die nächstgelegene Halbsumme der beiden MC-Werte ermit-
   telt,der Aufenthaltsort der Partner eingegeben und davon ausgehend das
   Häusersystem berechnet,wobei die Sternzeit aus dem MC-Mittel zurück-
   gerechnet wird.
```

3. Als dritte Methode werden einfach die Halbsummen der Häuser der beiden

```text
   Partner gebildet und zwar ausgehend von der nächstgelegenen Halbsum-
   me des MC der beiden Partner.Die Halbsummen der übrigen Häuser werden
   so gewählt,daß ein scheinbar "normales" Häusersystem resultiert.Man kann
   hier nicht immer die nächstgelegenen Halbsummen verwenden,da dies evtl.
   die Richtung im Horoskop vertauschen würde.
```

( Als Ekliptik-Schiefe wird in jedem Fall der Mittelwert der beiden

```text
   Partner verwendet ).
Bei den äqualen Haüsersystemen wird,um völlige Verwirrung zu vermeiden,
nur die dritte Methode verwendet !
```

Wie man es auch macht,keine der Methoden ist ganz überzeugend,sodaß ich geneigt wäre,in jedem Fall die Häuser bei der Deutung mit einem großen Fragezeichen zu versehen,einschließlich der sogenannten AC und MC-Werte.

Bei den beiden ersteren Methoden ist Haus 1 nicht mit der AC-Halbsumme identisch.Die nächstgelegene AC -Halbsumme wird im Horoskop daher extra angezeigt.Sie kann durchaus in manchen Fällen dem Haus 1 auch gegenüber liegen, da eine Halbsumme eigentlich kein Punkt sondern eine ACHSE ist. Sind äquale Häuser vorgewählt wird hierauf allerdings verzichtet,da dann die Zuordnungen zu verwirrend würden.

Bevor man sich mit Deutung eines COMPOSIT beschäftigt,sollte man sich diese Verhältnisse gründlich klar gemacht und an einigen Beispielen studiert haben. *****************************************************************************

## COMBIN

(Nach PHILIP SCHIFFMANN,WIEN) Berechnet aus 2 ...n Datensätzen ein kombiniertes Horoskop mit dem MITTELWERT der Geburts-ZEITEN (Jd1+Jd2..)/n und den MITTLEREN ORTS-KOORDINATEN.Es handelt sich also um ein "echtes" Horoskop,dessen Daten symbolisch begründet sind.Mit COMBIN sind viele Programme zugänglich,die auch für RADIX laufen.So können Sie nach COMBIN DIREKTIONEN und DÖBEREINER-AUSLÖSUNGEN studieren,falls Sie das für sinnvoll halten. Für die Verwendung der Programm-Gruppen "SOLAR..." oder "TAGES-HOROSKOP.." müssen Sie Vorher das COMBIN in ein Radix umwandeln,mit dem Menüpunkt "ERGEBNIS als RADIX".Das erfordert Aufmerksamkeit und Übung. Sie können COMBINS mit bis 5 Partnern machen.

Die betreffenden Datensätze müssen vorher eingegeben sein um sie dann nacheinander anklicken zu können.

Die Darstellung und Auswertung bei COMPOSIT und COMBIN ist im übrigen analog dem Radix-Horoskop.So können in beiden Fällen die HALBSUMMEN mit ausgedruckt werden,was bei COMPOSIT natürlich BESONDERS FRAGWÜRDIG und NUR ZU STUDIENZWECKEN gedacht ist.Es handelt sich ja sozusagen um "Halbsummen von Halbsummen.

***************************************************************************

## DOPPEL-KREIS

Dient zum direkten Horoskop-VERGLEICH.

Ausser dem (normalen) 360-Grad-Kreis kann auch ein 90-Grad-Kreis dargestellt werden,wie er in der HAMBURGER- und der R.EBERTIN-Schule benutzt wird.

Hierbei sind jeweils die kardinalen,fixen und gemeinschaftlichen Zeichen in je einem der drei 30-Grad-Sektoren in Überdeckung.

Die Haupt-Aspekte werden in einem eigenen kleinen ASPEKTARIUM aufgelistet. Es ist zweckmäßig,einen KLEINEREN ORBIS-FAKTOR ( z.B. 50% ) vorzuwählen. Unter dem Aspektsymbol steht der gerundete aktuelle Wert innerhalb des vorgewählten Orbis.

I steht für INNEN ,A für AUSSEN .Wenn Dieses Aspektarium überläuft,bleibt nur die Möglichkeit,den Orbis zu reduzieren (s. VORGABEN HOROSKOP..). Dieses Programm läuft auch HELIOZENTRISCH.

Es können BELIEBIGE Datensätze kombiniert werden.
