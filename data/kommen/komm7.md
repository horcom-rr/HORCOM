# Erläuterung Direktionen

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P), unverändert bis auf die Formatierung.*

Allgemeine Vorbemerkungen:

Unter "AUSLÖSUNG" wird im folgenden ein Aspekt zwischen dem laufenden und jeweils einem Radix-Faktor verstanden.Dieser Aspekt ist immer ein Vielfaches eines vorgegebenen GRUND-WINKELS.

Bei allen Direktionen wird automatisch mit MAXIMALER Genauigkeit gerechnet. Die Programme SEKUNDÄR-DIR. ,TRANSITE u. MUNDAN-ASPEKTE brauchen relativ viel Rechenzeit.Wundern Sie sich also nicht,wenn die Füllung des Tabellen-Formulars etwas dauert.Sie können diese Programme auch vorzeitig mit ESC verlassen ( Evtl. mehrmals drücken ! ).

Die Möglichkeiten der Programme sind jetzt so vielfältig,daß Sie einige Stunden aufwenden müssen um die für Sie wichtigen und ergiebigen Verfahren und Einstellungen zu erproben.So ist zu beachten,daß bei Vorwahl des Glücks-Punktes oder des Mondes,des wahren Mondknotens oder des wahren schwarzen Mondes die Auswertungen i.a. besonders zeitaufwendig sind,wegen der schnellen Bewegung dieser Faktoren,bzw. langsamen Vorschubs !!

Die Zeit-Skalierung der LINEAR-GRAPHIK sollte optimal gewählt werden.Das verlangt Übung ( s.unten ),die ich Ihnen nicht ersparen kann. Als GRUND-ASPEKT können Sie jeden Winkel wählen,der sich durch ganzzahlige Teilung von 360 Grad ergibt,solange der Wert von 15 Grad (Teiler 24) nicht unterschritten wird.

360 GRAD entspricht der Konjunktion als alleinigem Aspekt.Im übrigen ist die Konjunktion in allen Aspektreihen enthalten.

Man kann aus allen TABELLEN-ausgaben heraus,durch Drücken der FUNKTIONS-TASTE F2 das jeweilige Horoskop zwischendurch ansehen und mit 'Esc' wieder zurückgehen.

In den Tabellen bedeuten:

```text
LJ  = Lebensjahr                MO = Monat mit Dezimalen
PRO = Progressiver  Planet     STA = Feststehender-Planet
( Falls keine Richtung beachtet wird,einfach PL1 u. PL2 )
TB  = Tägl.Bewegung oder Geschw. des laufenden Planeten in Einheiten von
      "/Tag (Jahr).
      Negatives Vorzeichen bedeutet Rückläufigkeit.
      Je geringer TB ,desto länger die Wirkungs-Dauer.
```

ASP = Aspekt zwischen progressivem und Radix-Planet., Die Zwischenhäuser 2,3,5,6 sind einfach als Ziffern angegeben.

Bei SEKUNDÄR-DIR.,SONNENBOGEN-DIR. und TRANSIT erfolgt die Interpolation über 3 Stützwerte,also nicht linear (Kurve 2.Ordnung).Dies erhöht meist die Rechengenauigkeit,insbesondere in der Nähe von Umkehrpunkten,bedingt aber,daß eine gewisse Zeit vergeht,bevor in der Tabelle Werte erscheinen ! Die angegebenen TB-Werte (=tägliche Bewegung) sind jeweils MITTELWERTE des jeweiligen Interpolations-Intervalls,also in der Nähe von Umkehrpunkten nur sehr grobe Näherungen.Die TB-Werte in der Tabelle "PLANETEN-KOORDINATEN,sind demgegenüber genauere MOMENTAN-Werte.

## VORGABEN DIREKTIONEN ÄNDERN

Hiermit können u.a. auch die ZWISCHENHÄUSER hinzugewählt werden,ebenso auch, teilweise,die Punkte 0 AR,0 CN,0 LI und 0 CP = KARDINALPUNKTE berücksichtigt werden,die zwar keine individuelle,aber evtl. "mundane" Bedeutung haben könnten.

## SEKUNDÄR-DIREKTION

Die SEKUNDÄR-DIREKTION -auch Progression genannt- beruht auf der Gleichung:

```text
  1 Tag nach der Geburt = 1 Lebensjahr  ( Maß ist die eklipt. Länge ).
Das Programm ermittelt die Auslösungen nach Sekundär-Direktionen,wie sie z.B.
in der R.EBERTIN-Schule als LEBENS-DIAGRAMME "LDP" verwendet werden.
Die Ausgabe kann als TABELLE,als HOROSKOP-GRAPHIK ( siehe dazu auch die Aus-
führungen unten,bei "TRANSITE) oder LINEAR-GRAPHIK gewählt werden.
```

Bei der LINEAR-GRAPHIK kann ein Zeitraum von 5,10,20,40,80 oder 160 Jahren vorgewählt werden,durch Wahl des ANFANGS- u. END- Jahres in 5er Schritten !! Dies gilt auch für die SONNEN- und MOND-BOGEN-Direktion.

Zeitmaß ist die MITTLERE SONNENZEIT ( = UT ).

ACHTUNG! Hinsichtlich AC und MC ist folgendes zu beachten:

Das Häusersystem läuft in einem Tag ( bzw. progr. "Jahr") einmal um ca. 360 Grad.Dies ist nur in der HOROSKOP-Darstellung in echter Weise ersichtlich, wenn Sie bei der Auswahlbox "STRENG PROPORTIONALE...wählen. Die Maße "JAHR","MONAT","TAG" sind in diesem Fall kalendermäßig nicht "echt" sondern symbolisch zu verstehen.Als Umrechnungsfaktor dient die Länge des tropischen Jahres von tja = 365.24219878 Tagen.

Bei der TABELLEN-Darstellung und der LINEARGRAPHIK wird in ganzen Tagen fortgeschritten und das MC wandert scheinbar nur mit der Geschwindigkeit von 0.98565 Grad pro Tag ( "Jahr" ) gleichmäßig weiter.Dazwischen ist interpoliert. Wählen Sie bei der HOROSKOP-GRAPHIK die Variante HÄUSER-DREHUNG nach 1 TAG = 1 JAHR so liegen die Verhältnisse entsprechend.Allerdings werden in diesem Fall die Häuser eigens genau nach dem Sternzeitfortschritt berechnet. Was nun astrologisch mehr Sinn macht,sei dahingestellt.R. EBERTIN handhabt in seinen LEBENSDIAGRAMMEN die Sache so wie dies in der LINEAR-GRAPHIK oder der TABELLEN-Darstellung geschieht.

Auf die Berücksichtigung der "WAHREN SONNENZEIT",die von manchen Astrologen bevorzugt wird,wurde verzichtet,da dies die Programmsteuerung komplizierte und die Bedienung zu unübersichtlich machte.

Nach meiner bisherigen Erfahrung sind auch die SEKUNDäR-Direktionen nicht so genau fällig,daß die Beachtung dieses Unterschiedes ins Gewicht fallen würde. Je kleiner der Grund-Aspekt (>=15 Grad),desto höher die Zahl der Auslösungen. Der MOND kann als laufender "Planet " hinzugewählt werden ( Viele Auslösungen!).

Bei der TABELLE kann zwischen der Darstellung in LEBENSJAHREN/MON. dezimal und der Datums-Angabe gewählt werden.

Die ausgedruckten Tage sind nur als Anhaltspunkt zu werten. Wenn ein laufender Planet annähernd stationär ist und gleichzeitig in 1/4 GRAD Orbis zu einem Radix-Planeten,so wird in der TABELLE "STATION" ausgedruckt.

Derartige Wirkungspunkte werden evtl. über mehrere Jahre wirksam sein.

Für TRANSITE,SEKUNDÄR-,SONNENBOGEN-Direktionen und ekliptikale SYMBOLISCHE Direktionen können auch die HALBSUMMEN der Planeten angezeigt werden. Dies führt zu einem Vielfachen an Auslösungen (z.B. mehrere pro Tag bei Transiten ).

Falls diese Option gewählt wurde,wird automatisch auf "GROßE" Symbole umgeschaltet,da sonst die Halbsummen,in kleineren Symbolen schräg überund hintereinander mit "/" als Trennstrich,nicht darstellbar wären. Falls Sie diese Option aktiviert haben sollten Sie nur kleine Zeitabschnitte wählen,da sonst sehr viele Bildschirme benötigt werden,die evtl. den Speicher überfordern.Dann steigt HORCOM ohne Kommentar aus !! und man muß RESET betätigen und WINDOWS neu starten !! Mit WINDOWS 95 passiert dies nicht so leicht als mit WINDOWS 3.1

## DYNAMOGRAMM

Das Dynamogramm ist eine Sonderform der Sekundärdirektion mit der mich Herr JOHANN HUBER ( München ) vertraut machte.Das Verfahren stammt ursprünglich von KRAFFT und wurde Herrn Huber durch F.G. GOERNER schon in den 50er Jahren nahe gebracht.

Bei diesem Verfahren werden die Aspekte der laufenden Planeten zu den Radix-Planeten und die der laufenden Planeten untereinander in beiden Zeitrichtungen,das heißt progressiv und regressiv,graphisch summiert wobei die Direktions-Gleichung 1 Tag = 1 Jahr gilt.

Für die Planeten und AC,MC werden Wirkungsbreiten und Amplituden festgelegt. Befindet sich ein Faktor innerhalb seiner Wirkungsbreite im Aspekt zu einem zweiten,so wird seine energetische Wirkung als "Bogen" über dem entsprechenden Zeitintervall,in dem der laufende Faktor innerhalb der Wirkungsbreite liegt,ausgewertet.

Die Aspekte werden,je nach Charakter,"positiv" oder "negativ" bewertet und alle Amplituden eines bestimmten Zeitpunktes addiert bzw. subtrahiert. Bei den Faktoren wird unterschieden zwischen der "existentiellen Situation" und der "seelischen Grundstimmung".

Die Summe beider wird als "resultierende Energie" bezeichnet. Das Verfahren wird erst anschaulich wenn man die Entstehung der Bögen am Bildschirm verfolgt.Es wurden früher Cosinus-Bögen verwendet.In der vorliegenden Form wird ein Zeitraum von ca. 5 Jahren betrachtet.Die Lebensjahre für den Beginn werden zuvor eingegeben.

Das Verfahren wurde früher graphisch mit riesigem Zeitaufwand gehandhabt. Herr Huber hat,nachdem ein Computermodell von mir erstellt war,damit noch studiert ob die früher verwendeten Amplituden und Wirkungsbreiten beibehalten werden sollten und hat dies bestätigt.

Das Programm ist sehr rechenintensiv.Die relativen Amplituden und die Orbes können am besten in Aktion am Bildschirm beobachtet werden und können auch etwas von den beteiligten Qualitäten vermitteln.Die Zahlen über den Bögen geben den Planeten, den Aspekt-Teiler und das Vielfache des Grundaspektes an.

Alle Bögen werden aufaddiert bzw subtrahiert und ergeben für die Zeitspanne von ca. 5 Jahren durchlaufende Kurven.

Anstelle von Cosinus-Bögen können auch Gauß-Kurven ( Glockenkurven ) gewählt werden ( von mir hinzugefügt ).Diese bilden eine kontinuierlich ausklingende Wirkung nach und gleichen die Unebenheiten in den Kurven aus.Der Orbis bei dem die Ampltude auf 1/e abgeklungen ist wird dann zu 2/3 des Orbis bei Cosinus-Kurven angenommen.

Herr J.HUBER ist der Meinung daß die Cosinus-Kurven vorzuziehen sind. Die Einzelheiten des Verfahrens möchte Herr Huber einstweilen für sich behalten.

Bei der Analyse der Aspekte wird jeweils ein Zeitraum von + - 25 Tagen ( = Jahren ) vor und nach dem Beginn-Lebensjahr ausgewertet um auch die langsam laufenden Planeten innerhalb der Wirkungsbreite zu erfassen. Über diesen Zeitraum von 50 Tagen ( Jahren ) wird jeweils ein Mittelwert gebildet um abzuschätzen ob man erheblich zu positiv oder zu negativ bewertet.In der Tendenz scheint eher eine etwas zu negative Beurteilung vorzuliegen.Dieser Mittelwert ( unten in der Augabe - Graphik ) wird innerhalb einer HORCOM-Sitzung aufaddiert.Falls er immer positiver wird,ist die Beurteilung zu positiv im gegenteiligen Fall zu negativ. Ein völlig ausgeglichenes Ergebnis kann naturgemäß nicht erwartet werden. Der Amplituden-Maßstab ist willkürlich festgelegt und zwar so,daß für die Mehrzahl der Fälle die Kurven im Bereich des Bildschirms bleiben.Sie können aber auch hin und wieder diesen Bereich überschreiten.Eine Normierung auf die Bildschim-Maße hat sich nicht als zweckmäßig erwiesen,da dann leicht die relative Größe der Ausschläge unzutreffend beurteilt wird.Die Kurven können daher auch den Bildschirm-Bereich verlassen.

Man kann mit diesem Verfahren die aktiven ( = "positiv" ) bzw. passiven Phasen ( = "negativ" ) im Leben deutlich machen.Positiv bzw. negativ sollte aber nicht wertend verstanden sein,sondern eher wie ausatmen bzw. einatmen.

```text
"Positiv" :                             "Negativ" :
```

```text
Animus                                    Anima
Bemächtigungsformen                       Bemühungsformen
aktiv,zugreifend                          passiv,nachgebend
kämpferisch                               defensiv
genießend                                 sparsam
frei                                      gehemmt
Einsatz,Risiko                            Sinnsuche
```

Für Prognosen sollten das Verfahren m.E. nur herangezogen werden wenn es mit anderen kombiniert wird,z.B. mit Solaren und Transiten. Auch Erfolg ist daraus nicht immer zu ersehen,wohl aber vielleicht ob dieser leicht zufällt oder besondere Mühe abverlangt.

Auch ist ein Hoch bei einem introvertierten Typus anders zu bewerten als bei einem extravertierten.Ein Tief wird bei einem extravertierten Typ spürbarer sein als bei einem introvertierten.

Es würde Herrn Huber und mich interessieren welche Erfahrungen Sie mit diesem Verfahren machen.Bitte teilen Sie es gegebenenfalls ihm oder mir mit. Nach meinen bisherigen Tests mit dem Verfahren nehme ich eine zwar skeptische, aber nicht ablehnende Haltung dazu ein.

VORBEMERKUNG zu PRIMÄR-Direktionen :

Von den folgenden SONNENBOGEN- PRIMÄR- SYMBOLISCHEN Direktionen kann nur eine ( wenn überhaupt ) "gültig" sein,es sei denn man hält die Geometrie für eine belanglose Angelegenheit !

Meines Erachtens sind alle derartigen Direktionen,auch die Primär-Direktionen im engeren Sinn,als "symbolische" Direktionen zu betrachten,da sie Strukturen der RADIX auswerten.Daß sie sich z.T. mit echten astronomischen Bewegungen decken,ändert daran nichts.Es geht letztlich nur um das "richtige" ZEITMAß.

Die Vielzahl von Direktionen soll Studienzwecken dienen und entspringt NICHT meiner gleichmässigen Wertschätzung aller dieser Verfahren. Die Existenz mehrerer Verfahren,die alle in Gebrauch sind,zeigt daß es sich hier um eines der ungeklärten Gebiete der Astrologie handelt. Eine experimentelle Klärung dürfte sich recht schwierig gestalten.Die Entscheidung ob nun die "MUNDAN"-Geometrie die richtige ist gegenüber der direkten äquatorialen oder ekliptikalen Geometrie,sollte allerdings möglich sein,da die Auslöse-Zeitpunkte oft sehr stark unterschiedlich sind. Die Entscheidung zwischen SONNENBOGEN- EKLIPTIK.SYMBOL.- und AR-SYSTEM scheint mir dagegen recht schwierig und wird wohl noch lange "Geschmackssache" bleiben.Die oft behauptete taggenaue Präzision von direktionalen Auslösungen dürfte wohl bei keinem Verfahren einer statistischen Prüfung standhalten.

## SONNEN-BOGEN-DIREKTION

Berechnet die entsprechenden Auslösungen.

Die Darstellung ist weitgehend analog der bei Sekundär-Direktion. Die Gleichung ist hier:

```text
      1 Eklipt. Tages-Fortschritt der SONNE = 1 Lebensjahr "
Der Parameter "Tägl.Bewegung" entfällt hier,da alle Planeten mit dem Sonnen-
bogen "vorgeschoben" werden.
```

## MOND-BOGEN-DIREKTION

Hier gilt sinngemäß das gleiche wie beim Sonnenbogen:

```text
      1 Eklipt. Tages-Fortschritt des MONDES = 1 Lebensjahr "
Das ergibt gegenüber dem Sonnenbogen 12 bis 14 mal soviele Auslösungen,also
evtl. eine Art "Zeitlupe".
Sowohl bei SONNEN- als auch MOND-BOGEN-DIR. kann sowohl mit HOROSKOP- als
auch mit LINEAR-GRAPHIK oder mit Tabellen gearbeitet werden.
Beim MOND-BOGEN mit LINEAR-GRAPHIK,nur den 5-Jahres-Zeitraum verwenden !
```

## PRIMÄR-DIREKTION

Berechnet die Primärdirektionen nach E.C.KÜHR ( "BERECHNUNG DER EREIGNIS-ZEITEN" ).

In der Ausgabe-Tabelle bedeutet:

SIG = Signifikator PRO = Promissor Signifikator können alle Planeten,AC,MC und (wahlweise) die Zwischenhäuser 2,3 bzw. 5,6 sein ( 8,9 bzw. 11,12 entsprechen den Komplementär-Winkeln dazu ).

Bei den Promissoren sind immer auch die Aspekte (= Vielfache des Grundwinkels) angegeben,es sei denn man wählt die Konjunktion ( =360 Grad ) als Grundwinkel vor.

Der ZEIT-SCHLÜSSEL in JAHRE/GRAD kann beliebig eingegeben werden. Dem Verfahren entspricht eindeutig der NAIBOD-Schlüssel.

Für KORREKTUR-Zwecke läßt man sich den BOGEN ausdrucken,für bereits korrigierte Daten die Zeit.

Es wird hier IMMER mit PLACIDUS-Häusern gerechnet,da diese dem Sinn des Verfahrens optimal entsprechen.

Außerdem können die Promissoren MIT oder OHNE BREITE ausgewertet werden.Dabei wird die Breite der ASPEKTSTELLEN der Planeten nach BIANCHINI ( s. KÜHR ) berechnet.Dieser Punkt erscheint mir,welche Berechnungsart auch verwendet wird, als sehr ploblematisch.Auch KÜHR arbeitete deshalb für die Promissoren OHNE Breite !! ,für die Signifikatoren immer MIT BREITE,was inkonsequent erscheint. Es kann daher auch für BEIDE Faktoren OHNE BREITE gerechnet werden.Möge jeder seine Erfahrungen damit machen.

Für die Berechnung der Breite von Aspekt-Stellen hat KÜHR nicht die Methode BIANCHINI empfohlen,sondern eine,recht komplizierte und kaum automatisierbare, die mir im übrigen auch nicht einleuchtet.

Alle diese Unklarheiten und auch noch einiges mehr,z.B. daß immer unter dem Pol des Signifikators auf den Äquator projiziert wird,lassen mir diese Direktions-Methode als besonders problematisch erscheinen.Der Nimbus,der sie umgibt,könnte auf dem Schluß beruhen: Was mühsam zu berechnen ist ( war ) muß auch gut sein.

Einen Spezial-Aspekt davon,nämlich das sog. "MUNDAN-HOROSKOP" möchte ich von dieser skeptischen Bewertung ausnehmen,da dieses auf einer eindeutigen,einfachen Verhältnis-Beziehung beruht (PLACIDIANISCHE FUNDAMENTAL-PROPORTION).

## SYMBOLISCHE DIREKTION ÄQUATORIAL

Hier können Sie nun 2 Verfahren anwenden,die ich,auch für eigene Experimente, eingeführt habe.

( Es wird IMMER mit PLACIDUS-Häusern gerechnet,da diese dem Sinn der Verfah-

```text
   ren am besten entsprechen).
1.Die ÄQUATORIALEN ABSTÄNDE in der "MUNDAN"-Geometrie,wie sie bei der M.R.
  auch anwendbar ist.Sie entsprechen den PRIMÄR-Direktionen,ohne Unterschei-
  dung von Promissor und Signifikator,nur nach der "PTOLEM.FUNDAMENTAL-PROP."
  Die Aspekte sind DIREKT AUF DEM ÄQUATOR gemessen,was mir hier sachgemäß
  erscheint.Dies ist auch beim folgenden "AR-SYSTEM" so gehandhabt.Die Win-
  kel sind also hier keine Raum- sondern primär ZEIT-Abschnitte.
  Dieses Verfahren ist bisher nicht üblich,scheint mir aber,wenn man schon
  PRIMÄR-Direktionen verwenden will,das "naturgemässe Verfahren,das die
  erwähnten Unsicherheiten (s. oben ) umgeht.
2.Das "AR-SYSTEM" von  C.O.E.CARTER  (s.SYMBOLISCHE DIREKTIONEN  Urania,
  Blaue Reihe 3 ).Ich habe es,aufgrund meiner Überlegungen etwas abgewandelt:
  Bei den Planeten wird der AR-Abstand ausgewertet,wie dies CARTER schildert.
  Bei AC und Zwischenhäusern wird die entsprechende AO ausgewertet.Beim MC
  ist dies natürlich mit der ARMC identisch.
  Meine bisherigen Erfahrungen mit diesem System sind eher besser als mit
  den "klassischen" Primär-Direktionen entsprechend der Darstellung bei KÜHR .
Der AR- bzw. AO- Abstand wird nach einem wählbaren Schlüssel in Zeit umge-
rechnet.
Der naturgemäße SCHLÜSSEL (=JAHRE/GRAD ) für die Umrechnung des BOGENS in
ZEIT ist in beiden Fällen der NAIBOD-Schlüssel.Die "Gleichung ist hier:
```

```text
             1 TÄGLICHER STERNZEIT-FORTSCHRITT = 1 JAHR "
```

## SYMBOLISCHE DIREKTION  EKLIPTIKAL

Hier gilt anstelle der AR der EKLIPTIKALE Bogen-ABSTAND .Dies ist sicher die einfachste aller Direktions-Arten,aber m.E. deshalb nicht weniger interessant.

Hier kann auch mit anderen Häuser-Systemen gearbeitet werden. Mit diesem Programm kann natürlich auch direkt der gegenseitige WINKEL-ABSTAND der Horoskop-Faktoren in der Ekliptik ausgegeben werden. AR-SYSTEM und SYMB. DIR. darf auch auf SOLAR,LUNAR und TAG-HOR angewendet werden.SYMB.DIR EKLIPT. läuft auch HELIOZENTRISCH.

Auf mehrfachen Wunsch von Anwendern ist unterschieden zwischen "DIREKT"="D" und "KONVERS"= "K",obwohl ich dies kaum für begründbar halte.Der Richtungssinn ist so festgelegt,daß die Dir. als "D" gewertet wird,wenn der ZWEITE Planet in TIERKREIS-FOLGE den ersten "einholt".Dies entspricht den PRIMÄR-Direktionen ( KÜHR ),wobei PL1 dem SIGNIFIKATOR entspricht.

## TRANSITE

Arbeitet analog wie SEKUNDÄR-DIR.,nur daß hier die Tage direkt gezählt werden.Es läuft auch HELIOZENTRISCH und kann auch auf SOLAR,LUNAR und TAG-HOR angewendet werden.Der Rechen-Aufwand bzw. Zeitbedarf ist rel.groß. Bei der TABELLEN-Darstellung ist folgendes zu beachten:

Die UT des Transits in H,MIN wird angezeigt,bei den äusseren Planeten die H mit Dezimalteilen ( Zwischen den Planeten-Symbolen,in Kleinschrift). In der Nähe von Umkehrpunkten sind diese Angaben nur Richtwerte. Bei ZWISCHENHÄUSERN als RADIX-Faktoren ist z.B. +3 angegeben wenn der betreffende Aspekt im Tierkreissinn positiv "ins 3. Haus hineinläuft" also vom 2. ins 3. Haus.Dagegen bedeutet -3 daß der Aspekt vom 3. ins 2. Haus übergeht ! Dasselbe gilt übrigens sinngemäß auch bei SEKUNDÄR-Direktionen. Nach dem Datum ist die tägliche Bewegung des laufenden Faktors in Winkel-Minuten angezeigt.Ist das Vorzeichen negativ,so verläuft der Aspekt vom größeren zum kleineren Winkel,wenn nicht,umgekehrt.Angezeigt wird der,im Rahmen der Rechengenauigkeit,exakte Aspekt.

Neben der Tabellen-Darstellung kann auch "HOROSKOP-GRAPHIK"-Darstellung gewählt werden.Dafür gilt :

Im Innenkreis das betrachtete Horoskop und die TRANSIT-Planeten sind aussen. Bei dieser Darstellung werden auch AC und MC für einen vorwählbaren Ort mit dargestellt,wobei man über den Sinn der Sache natürlich streiten kann. In der linken Bildhälfte werden die Haupt-Aspekte zwischen Innenkreis (R) und Außenkreis (T) mit 1 GRAD ORBIS ( veränderbar über "VORGABEN HOROSKOP.. .. ) aufgelistet.Die Konjunktionen sind darin nicht aufgeführt,da sie aus der Figur direkt ersichtlich sind.

Das Programm läuft,nach Vorwahl von Richtung und Zeit-Einheit,automatisch ab, kann aber während des Laufs über die Tastatur umgesteuert werden (siehe Dialog-Texte).Der Ausstieg erfolgt über die ESC-Taste.

Der letzte BIDSCHIRM wird bei bei TRANSITEN und SEKUNDÄR-Direktion rückholbar GESPEICHERT falls die Laufzeit 15 Sek. überschreitet.

Die Ausgabe-Form "LINEAR-GRAPHIK" ermöglicht das Erstellen von graphischen Ephemeriden für Vielfache des vorgewählten Grundwinkels,z.B. eine 45-Grad-Ephemeride nach R.EBERTIN,wobei die Radix-Planeten als horizontale Linien dargestellt sind.Die Häuser 2,3,5,6 aind als H2,H3,H5,H6 bezeichnet. Die Zeitspanne kann entweder für 1 Monat,4 Monate oder 16 Monate vorgewählt werden.

Die RICHTUNG der ORDINATE kann mit "VORGABEN DIREKTIONEN ÄNDERN" entwerder nach OBEN POSITIV ( mathematischer Modus ) oder nach UNTEN POSITIV gewählt werden.Letzteres erleichtert den Vergleich z.B. mit Beispielen von R.EBERTIN.

Die LINEAR-Graphiken der Transite können ab 15.10.96 auch als DRUCKER-GRAPHIK ausgegeben werden,falls die DRUCKER-OPTION von HORCOM aktiv ist. Bei TRANSITEN usw. muß man den Ablauf des Programms abwarten bevor der Drucker arbeitet.Das kann,je nach Geschwindigkeit des Prozessors,etwas dauern.Also in solchen Fällen Geduld üben.

Ab August 98 kann man auch in der fertigen Bildschirm - Ausgabe selbst mit der Maus Treffer-Linien markieren wenn das feste Linien-Gitter oder keine Linien vorgegeben sind : LINKS ergibt BLAUE , RECHTS ROTE Linien.Macht man anschließend eine Hardcopy enthält diese die so eingezeichneten Trefferlinien.

Wird bei der Vorwahl der Parameter "MIT ZEICHEN" gewählt,werden in den Kurven in Abständen hinter den Planeten das Zeichen angezeigt in dem diese sich befinden soweit die Zuordnung graphisch einigermaßen eindeutig zu treffen ist.Die Zeichengrenzen werden durch waagrechte strichpunktierte, blaue Linien markiert.

## MUNDAN-ASPEKTE

Arbeitet mit eigener Eingabe.Es werden die Aspekte der laufenden Planeten untereinander ausgedruckt.Zeitbedarf ebenfalls hoch.Die in der Tabelle angegebenen Grade sind wie folgt zuzuordnen:Der obere auf den linken,der untere auf den rechten Planeten (Kardinalpunkte und Häuser entfallen natürlich ). Hinsichtlich Stunden-Angabe gilt entsprechendes wie bei Transiten. Da man dieses Programm selten braucht,wurde auf Speicherung verzichtet. Für die STUNDEN-ASTROLOGIE können bei der Tabellen-Ausgabe wahlweise die Aspekte auf ein bestimmtes Zeichen beschränkt werden um z.B. zu sehen ob der Mond oder ein anderer Planet "void of course" läuft.

Auch hier kann die Ausgabeform "LINEAR-GRAPHIK" gewählt werden. Benötigt ein Programm MEHR ALS EINEN BILDSCHIRM,so wird bildschirmweise weitergeschaltet.Zum Schluß erfolgt eine ZEIT-SORTIERUNG,wahlweise kann auch nach ASPEKTEN sortiert werden.Die sortierten Werte werden ebenfalls bildschirmweise weitergeschaltet.

Wollen Sie,daß das Programm bis zum Ende alles berechnet und sortiert,so geben Sie nach dem ersten Bildschirm ein "+" ein.Das Programm läuft dann, soweit der Speicher reicht,bis zum Ende,sortiert automatisch und bleibt dann stehen.

Bei "PROGRAMM BEENDEN ?" müssen Sie dann mit "NEIN " antworten und sind in der sortierten Liste in Blatt 1.Wollen Sie diesen Modus von vornherein immer haben,können Sie bei "VORGABEN DIREKTIONEN ÄNDERN " eine entsprechende Einstellung vornehmen.

---

Bei allen Direktionen,Transiten und Mundanaspekten können einzelne Faktoren herausgegriffen und ( oder ) ROT MARKIERT werden ( nicht bei Lineargraphik ).

Wahlweise können in TABELLEN die Symbole der "ÜBELTÄTER" MA,SA,UR,NE,PL INVERTIERT werden oder ( und ) die ASPEKTE eingefärbt werden. Die "HARTEN" Aspekte 0°,90°,180° ROT,die "HARMONISCHEN" 60°,120° GRÜN.

---

Wird MIT PARALLAXE gerechnet so ergibt sich bei PROGRESSIONEN und TRANSITEN ein Problem,da die RADIX-Faktoren sich auf GEBURTS-Ort und -Zeit beziehen, die laufenden jedoch gegebenenfalls auf den Ereignisort bzw.die Ereigniszeit. Bei PROGRESSIONEN dürfte es daher sinnvoll sein,IMMER den GEBURTSORT zu wählen.Bei TRANSITEN scheint es nicht so eindeutig zu liegen. Die Sache ist natürlich besonders beim MOND interessant,da hier der Effekt über ein Grad bzw. bis zu ca. zwei Stunden Unterschied ausmachen kann.
