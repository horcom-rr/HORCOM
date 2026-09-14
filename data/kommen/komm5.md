# Erläuterung Solar, Septar ...

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P), unverändert bis auf die Formatierung.*

SOLAR,SEPTAR,LUNAR,PLANETAR,PERSONAR,TAGES-HOROSKOP,PROGRESSIV-HOROSKOP

## SOLAR , SEPTAR

Ermittelt den Zeitpunkt für ein SOLAR-Horoskop,dessen Sonderform SEPTAR oder eines LUNAR .

Nachdem Sie das SOLAR..-Programm angewählt haben und das gewünschte Jahr, bzw. Datum für das Solar.. eingegeben haben,können Sie die übrigen Menü-Punkte,soweit sie sinnvoll sind,anwählen,da jetzt der neue Zeitpunkt gilt. Die betreffende Ausgabe ist durch die Überschrift SOLAR.... gekennzeichnet. Evtl.gegenüber Geburts-Ort geänderte Orts-Koordinaten werden abgefragt. Die NUMMER,die vor "SOLAR" angezeigt wird ist folgendermaßen zu verstehen: NR. 0 entspricht der RADIX NR. 1 entspricht dem 1. Geburtstag usw.

Negative Nummern entsprechen den vergangenen Jahrestagen.

ACHTUNG !

Diese Numerierung,die ab 26.09.02 auch für LUNARE und PLANETARE eingeführt wurde,unterscheidet sich von der Numerierung bei SEPTAREN ( siehe folgendes Thema ).Dort ist das 1. SEPTAR gleich der RADIX ! Das ist im Wesen des Verfahrens begründet.

Neben dem normalen Solar können Sie auch "SEPTARE" erstellen,wie sie in der MÜNCHNER RHYTHMENLEHRE von W.DÖBEREINER üblich sind.Mit den Programmen können Sie ALLE SEPTAR-VERSIONEN behandeln.Neben der SEPTAR-NR. kann auch die PERIODE/HAUS und die RICHTUNG ( RECHTS = UHRZEIGER-SINN ) frei gewählt werden.

Z.B.gilt im 7er - Rhythmus das Septar Nr.3 für die Lebensjahre 14-21,wenn ab 14 die Zeit-Einheit Monat verwendet wird.( Bei der Zeiteinheit "Jahr" entspricht dies im 7-er Rhythmus der Spanne 14-98 Jahre.

Man kann auch z.B. SOLARE im Ein-Monats-Rhythmus durchlaufen. Interessant mag auch sein,ein LUNAR im 1/12-Monats-Rhythmus zu durchlaufen: Eingeben Zeit-Einheit MONAT,PERIODE 0.08333 (bei SONSTIGE ) und DATUM-Ausgabe.

SEPTARE werden SCHULGEMÄß für den Geburtsort erstellt.Sie können aber auch mit anderen Orten experimentieren.

Nachdem Sie einen SEPTAR-Zeitpunkt berechnet haben,können Sie mit dem Programm MÜNCHNER RHYTHMENLEHRE die Auslösungen studieren.

## LUNAR

Ermittelt entsprechend den Zeitpunkt für ein Lunar-Horoskop (Mond hat die gleiche ekl.Länge wie bei der Geburt.Siehe auch Erl.3 PARALLAXE ) Dazu wird zur Eingabe eines Datums für den interessierenden Zeitraum aufgefordert.Es wird dann meist das unmittelbar vorangehende Lunar,innerhalb der nächsten ca. 28 Tage ermittelt.Wenn nicht,ein entsprechend vorgeschobenes Datum wählen.

Daneben ist die Eingabe einer NUMMER in der Zukunft oder Vergangenheit möglich.Dies ist zur Unterstützung der TERTIÄR-Direktionen nach TROINSKY gedacht ( Vorschlag von Herrn PH.SCHIFFMANN ).Siehe dazu obige Bemerkung unter SOLAR.

Lunare und Solare werden hier NICHT durch Interpolation innerhalb eines Tages berechnet,sondern ERHEBLICH GENAUER.Dies ist bei LUNAREN besonders bedeutsam.

Falls Sie also mit (linearer) Interpolation nachrechnen,werden Sie etwas andere Ergebnisse bekommen,was aber nicht etwa diesem Programm anzulasten ist,sondern der Ungenauigkeit der lin. Interpolation.

Die genaue Iteration braucht allerdings einige Sekunden Rechenzeit. Die abs. Genauigkeit der ermittelten Zeiten ist besser als 0.4 Min.Siehe auch Erläuterung 3.

## PLANETARE

Planetare können nun ( ab HORCOM3C ) auch errechnet werden.Die Rechnung ist geozentrisch,wegen der möglichen Rückläufigkeiten,erheblich aufwendiger als bei SOLAREN und LUNAREN.Je nach Prozessor und Taktfrequenz des PC beträgt	die Rechenzeit typischerweise ca. 1 Sekunde bis eine halbe Minute oder auch etwas mehr.

Dank der nun vorliegenden sehr genauen Ephemeriden-Formeln von BRETAGNON ist die Zeit-Auflösung weit besser als 1 Zeitminute,zumindest bei einiger Entfernung von Umkehr-Punkten.Stößt man beim Suchen auf Umkehrpunkte,so versagt die Rechenroutine,was allerdings selten der Fall sein wird. Falls die Rechenzeit über 1 Minute hinausgeht,ist der Versuch als mißglückt zu betrachten.Dann kann man mit der ESC-Taste aussteigen ( längere Zeit auf der Taste bleiben,bis "ABBRUCH" erscheint ) und mit einem neuen Suchdatum nochmals probieren.

Der Interpolations-Fehler entspricht höchstens einigen Zeitsekunden. Jedenfalls ergibt sich,wenn man auf die Radix als PLANETAR zurückrechnet immer die korrekte Minute,oft auch die Sekunde.

Bei Eingabe des Such-Datums berücksichtigen Sie bitte,daß in der Vergangenheit gesucht wird.Die jeweilige Umlaufszeit in Jahren bzw. Tagen ist angemerkt.

Außer einem Such-Datum kann auch eine Nr. des Planetars in der Zukunft oder der Vergangenheit vorgegeben werden.

Die NR. bezieht sich auf die RADIX. NR. 0 ist identisch mit der RADIX.

Bei mehrdeutigen PLANETAREN werden jeweils die 3 oder mehr Zeitpunkte der gleichen Nr. zugeordnet.Bei NEPTUN können auch,nicht selten,bis zu 5 Zeitpunkten gefunden werden.Bei sehr exzentrischen Bahnen,wie z.B. bei den Asteroiden DAMOKLES,NESSUS oder KOMET HALLEY können es bis zu 9 Zeitpunkten sein,wobei hier abgebrochen wird.

Das gleiche gilt für PLUTO,QUAOAR und XENA.

Man muß ein wenig üben.Es wird normalerweise der erste direktläufige Zeitpunkt unterhalb des eingegebenen Suchdatums angezeigt.

Anschließend kann dann noch nach mehrdeutigen,normalerweise 2 zusätzlichen Zeitpunkten,einem rückläufigen und einem wieder direktläufigen,gesucht werden usw.

Dies wird übergangen,wenn aus der Geschwindigkeit ( tägl. Bewegung ) des zuerst gefundenen Punktes hervorgeht,daß keine Mehrdeutigkeit zu erwarten ist.

Anschließend kann dann der nächste direktläufige Zeitpunkt in der Vergangenheit aufgesucht werden usw.

Über PLANETARE liegen keine astrologische Erfahrungen vor,da die erhältlichen Ephemeriden bisher zu lückenhaft,bzw zu ungenau waren.Ein neues Feld für Forscher.Man sollte annehmen daß PLANETARE jeweils für den darauffolgenden Umlauf etwas aussagen.Die eventuelle Mehrdeutigkeit ist problematisch. Für menschliche Horoskope kommen die Planeten ME,VE,MA,JU,SA und UR infrage. Für historische Forschungen evtl. auch NE und PL.

Herr PHILIP SCHIFFMANN hat bei der Entstehung der "PLANETARE" maßgeblich mitgewirkt.

## PERSONARE

Personare wurden von PETER ORBAN und INGRID ZINNEL als astrologische Methode entdeckt.

Sie werden für die exakten Übergänge der Sonne über die Radix-Planeten,innerhalb des ERSTEN Lebensjahres,erstellt.

Sie können also für alle vorgewählten "echten" Planeten und den Mond Personare machen.

Nach meiner Kenntnis werden Personare als Teilaspekte der Persönlichkeit interpretiert,je nach der Charakteristik des gewählten Planeten. In HORCOM werden die Namen wie folgt abgekürzt :

Z.B. für den Mars "MARS-PERS" usw.

## TAGES-HOROSKOP ,abgekürzt  TAG-HOR

Berechnet für beliebiges Datum und Ort ein Horoskop mit der WAHREN Orts-Sonnenzeit des Radix ( Der Abstand AR MC- AR SO ist der gleiche wie im Radix).

ACHTUNG! : Bei Rechnung MIT Parallaxe können Abweichungen auftreten,insbesondere wenn das Tageshoroskop für einen vom Geburtsort abweichenden Ort berechnet wird !

## PROGRESSIV-HOROSKOP ,abgek. PROG-HOR

Berechnet direkt ein Progressiv-Horoskop für ein bestimmtes Ereignis-Datum. Gleichung dabei :

```text
                    " 1 TAG = 1 JAHR "
Als Umrechnungs-Konstante wird das tropische Jahr von 365.242199 Tagen
eingesetzt.
Für die Uhrzeit,bzw. das Häusersystem können folgende 4 Versionen gewählt
werden :
```

```text
   1) UT = RADIX-UT,
   2) Wahre Sonnenzeit wie bei Radix  ( Abstand AR Sonne - ARMC konstant ).
      Die WAHRE SONNENZEIT wird hier,wie auch beim TAGES-HOROSKOP,recht genau
      durch Iteration ermittelt,was etwas Rechenzeit kostet.Die Berechnung
      entspricht der beim TAGES_HOROSKOP.
   3) Häusersystem des Progressiv-Horoskops um den STERNZEIT-FORTSCHRITT
      gegenüber dem Radix gedreht.Das MC wandert z.B. bei einem Alter von 50
      Jahren gegenüber dem RADIX um ungefähr 50 GRAD weiter.Dementsprechend
      wird die Uhrzeit genau aus der entsprechenden neuen Sternzeit berechnet.
      Die Häuser wandern hier also um etwa den Faktor 365 langsamer als bei
      der folgenden Berechnungsart.
   4) Die PLANETEN und HÄUSER werden,einschließlich der Tageszeit,nach der
      streng proportional aus der Anzahl der abgelaufenen tropischen Jahre
      errechneten Zeit berechnet:
           1 Jahr entspricht genau 1 julianischem Tag bis zu kleinen
             Bruchteilen von Minuten.
  Astronomisch korrekt und eindeutig ist nur die Version 4 zu berechnen.
  Bei den Versionen 1),2),und 3) wurde nun ( ab 26.11.98 ) das Datum so gelegt,
  daß zu dem korrekten Wert nach 4) maximal ein halber Tag Unterschied bestehen
  kann.
  Welche von den angebotenen Möglichkeiten am meisten Sinn macht,wage ich
  nicht zu entscheiden.Mir fehlt hier die Erfahrung.Nach v.H.KLÖCKLER wäre die
  Version 1) die übliche.Er hält allerdings generell nicht viel davon.
  Normalerweise macht man ein Progressiv-Horoskop nur für runde Geburtstage
  und für den Geburtsort.
  Das Progressiv-Horoskop soll dann etwas über das kommende Lebensjahr
  aussagen.
```
