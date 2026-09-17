# Erläuterung Ein-Ausgabe

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P). Wortlaut unverändert, weggelassen sind technische Abschnitte zur Installation, Tastatur- und Druckersteuerung der historischen Programmfassung.*

************************************************************************ VORBEMERKUNG :

Das folgende bezieht sich auf das Datei-Handling von HORCOM,wie es seit langen Jahren vorhanden ist.

Hinsichtlich Handhabung von AAF-DATEIEN innerhalb HORCOM7P,siehe den entsprechenden ANHANG unten !!

Die AAF-Eingabebox erscheint zusätzlich durch Drücken des BUTTONS "AAF-Format" links unten in der HORCOM-Eingabebox.

Der Erläuterungstext,speziell für das AAF-Datei-Handling erscheint innerhalb der AAF-Eingabebox durch Drücken der Taste F1.

*************************************************************************

Die Kolonne EIN-AUSGABE dient,neben der EIN - und AUSGABE der Daten, auch der STEUERUNG des PROGRAMM-ABLAUFS.Dies verlangt bei einem komplexen Programm,das für Experimente offen sein soll,in jedem Fall etwas Übung ! Der Autor hat inzwischen ca. 12000 Arbeitsstunden in HORCOM investiert.Etwa 50 Stunden wird auch der "User" aufwenden müssen um einigermaßen auszuschöpfen was HORCOM bietet.

## DATEI EIN/AUSGABE

ACHTUNG !!

Hier können Sie Datensätze aus der DEMO-Datei oder von Ihnen selbst angelegten Dateien holen ( siehe unten ) oder in andere Dateien abspeichern oder nicht mehr benötigte Datensätze löschen.

Die Daten einiger Prominenter sind in der \DEMO.DAT auf Ihrer Programm-Disk gespeichert (ohne Gewähr!) Diese Daten können Sie verwenden um sich mit der Programm-Bedienung vertraut zu machen.Die MC-Werte dieser Daten wurden oft dem Buch 'CIRCELS' entnommen (JAN CAMPHERBEEG AMSTERDAM, ISBN 906378044) und mit dem Menü-Punkt KORREKTUR in UT umgesetzt.Bei Großstädten wurde dabei immer die Ortsmitte angenommen,was in der Regel nur genähert gilt.

Das Anwählen eines Datensatzes zum Holen oder Überschreiben geschieht mit der Maus,indem die betreffende Zeile markiert wird.

Aus der Datei-Liste können Sie so bis zu 5 Datensätze anklicken und drücken dann "WAHL-ENDE".Doppelklick wirkt wie "WAHL-ENDE".

Eine Daten-Datei bedarf ab und zu der Pflege.

Das LÖSCHEN nicht mehr benötigter Datensätze ist möglich.Sie können für einen Löschvorgang bis zu 10 zu löschende Datensätze markieren.

Die Datei DEMO.DAT befindet sich in dem Ordner \HORCOM\SPEZIAL in dem auch alle sonstigen neu angelegten DATEN-Dateien abgelegt werden.

## NEU-EINGABE

Zur erstmaligen Eingabe eines Daten-Satzes "VON HAND".

Wenn nichts Anderes gefragt ist,immer GROß-oder KLEIN-BUCHSTABEN,bei EIGEN-NAMEN (auch Orts-Namen) eingeben.Es wird alles in Großbuchstaben umgesetzt. Ziffern OHNE PUNKT oder KOMMA! innnerhalb der vorgegebenen Eingabefelder eingeben.

Abgeschickt wird ein Datensatz durch Anklicken des 'OK' - Feldes rechts unten.Solange dies nicht geschehen ist,können die RADIX-Datensätze wie folgt editiert werden : Zwischen den einzelnen Editfeldern kann mit der TABULATOR-Taste,teilweise auch den senkrechten Pfeiltasten hin und hergesprungen werden.Innerhalb der Felder kann mit den wagerechten Pfeiltasten und mit BACKSPACE oder DEL ( ENTF ) editiert werden.

Das Weiterschalten von Datenfeld zu Datenfeld kann immer mit der TABULA-TOR-Taste geschehen,in Gegenrichtung mit SHIFT+TAB.

Das Weiterspringen von einem Feld zum nächsten geschieht bei Neueingabe automatisch,wenn das Feld gefüllt ist und unter "VORGABEN EIN-AUSGABE ÄNDERN" entsprechend verfügt wurde.

Mit der Maus kann der Cursor ebenfalls positioniert werden.

ACHTUNG ! Mit einem DOPPELKLICK in eines der Editierfelder können Sie jederzeit direkt beim Editieren zwischen der TABSTOP- und der automatischen Version hin- und herschalten.

Damit,hoffe ich,wird jeder zu seiner Gewohnheit finden.Bei der Eingabe "Feld für Feld" wird durch einen Signalton angezeigt wenn das Feld gefüllt ist !

Beim ÄNDERN eines BEREITS BESTEHENDEN DATENSATZES mit dem Cursor zuerst bis in das erste DATUMS-Feld gehen und die Abfrage "ORT SPEICHERN ?" mit NEIN beantworten,dann in das erste Feld ( NAME.. ) zurückgehen und mit dem Überschreiben beginnen.

Wollen Sie bereits gefüllte Felder überschreiben,immer vorher mit "ENTF" oder RÜCKTASTE diese Felder vollständig löschen,es sei denn diese sind farbig markiert !!

Falls Sie bei der ORTS-Eingabe die entsprechende AUSWAHLBOX sehen wollen ist es bei bereits ausgefüllten Feldern,die man abändern bzw. überschreiben möchte,evtl. notwendig im ersten Feld ( NAME,VORNAME ) einen DOPPELKLICK auszuführen.Ebenso,wenn die Maus nur im letzten ausgefüllten Feld stehen bleibt !!

## VORGABEN EIN-AUSGABE ÄNDERN

Beim Ort können Sie einen VORZUGS-ORT festlegen.

Orte,die Sie eventuell später wieder brauchen,können Sie in ORTS-DATEIEN eingeben und bei Bedarf wieder holen.

ORTS-Dateien für DEUTSCHLAND,SCHWEIZ,ÖSTERREICH,EUROPA und WELT befinden sich auf der System-Disc.Sie werden von den Herren BRUNO MAHL und DR.HELMUT WISGRILL allen HORCOM-Usern dankenswerterweise zur Verfügung gestellt.

Alle ORTS-Dateien befinden sich in dem Ordner '\HORCOM\SPEZ_ORT

Seit 2003 sind nun auch sehr viel UMFANGREICHERE Ortsdateien im HORCOM-FORMAT verfügbar,die für WESTEUROPA und USA über meine Website heruntergeladen werden können.Sie benötigen 17.5 MByte Speicher,entsprechend 486000 Orten.

Für (fast) ALLE LÄNDER der ERDE können sie auf CD für EURO 25.- ( ins Ausland EURO 30.- ) bezogen werden.Diese benötigen 79 MByte Speicher, entsprechend ca. 2.2 Millionen Orten.Sie sind in einem Ordner namens ..\SPEZ_ORT\BIGFILES zusammengefasst.

Diese Ortsdateien sind durch den zweistelligen Code von NIMA ( = NATIONAL IMAGERY and MAPPING AGENCY ) bezeichnet,wobei die größeren Länder noch mittels Buchstabengruppen unterteilt sind.Die Code-Tabelle wird jeweils eingeblendet.

Für die meisten Länder sind hier ALLE bewohnten Orte vorhanden,was den Nachteil hat,dass Namen vielfach vorkommen können.

In einigen Fällen,wie z.B. für Russland und China ist die Anzahl der Orte so groß,dass nur die nennenswerten Verwaltungeinheiten aufgeführt werden konnten.Es sind trotzdem noch einige Tausende.

Wird bei Länge u.Breite statt 'O' bzw 'N' einfach weitergegangen,so wird dies trotzdem als 'O' u. 'N' interpretiert.Nur 'W' bzw. 'S' muß man wirklich eingeben ( vorher 'O' bzw. 'N' löschen ).Großenteils kann man daher mit der rechten Hand im Ziffernblock eingeben.

Den LINKEN DAUMEN reserviert der geübte HORCOM-User für die Leertaste ! Bei Daten v.Chr. geben Sie in dem dafür vorgesehenen Feld,das Sie sonst einfach übergehen,entweder 'V' oder '-' (minus) ein und bei jjjj die NORMALE, von den Historikern verwendete Jahreszahl.Bei den bei Ergebnissen dann oft vorkommenden NEGATIVEN Jahreszahlen handelt es sich allerdings um die ARITHMETISCHE Zählweise : Z.B. entspricht "Jahr 63 v.Chr" dem Jahr -62 !! , das Jahr Null also dem Jahr 1 vor Chr !!

Siehe auch Erläuterung 3 bei "KALENDER".

Ansonsten sind bei Daten v.Chr. hinter dem Datum die Buchstaben VC oder V gesetzt.

Die UHRZEIT ist als ZONENZEIT einzugeben ! Bei ( fast ) allen AUSGABEN wird allerdings GREENWICH-Zeit GZ = GMT = UT angezeigt,da nur diese auch gespeichert wird.

Die UT wird bei vorliegender Zonenzeit wie folgt ermittelt :

```text
                 UT = GMT = ZONEN-ZEIT + ZZD
                 ZZD = ZONENZEIT-DIFFERENZ
```

Bei der NEU-Eingabe von Daten erscheint ein Dialog zum Einstellen der Zonenzeit,falls in der Eingabebox die betr. CHECKBOX angeklickt wurde.Sie zeigt links oben ein Editierfeld und zwei Tabellen in welchen hinter dem ORTS-Namen die ZZD-Werte zu ersehen sind.Wollen Sie eine der entfernteren Zonenzeiten wählen,suchen Sie in der Tabelle nach einer Stadt in dem gesuchten ten Landstrich und klicken diesen an.In dem Editierfeld erscheint dann dieser Wert und Sie können ihn mit "OK" übernehmen.

Für MEZ ( = mitteleuropäische Zeit) ist die ZZD = -1 h .Wollen Sie diese wählen,brauchen Sie die Tabellen nicht sondern klicken auf "OK".

Gilt SOMMERZEIT,so klicken Sie in die betreffende CHECKBOX.

Für geographische Längen östlich Greenwich ist die ZZD i.a. negativ,westlich davon positiv.

Gebräuchliche Abkürzungen für die näherliegenden Zonenzeiten sind:

```text
       MEZ  = MITTEL-europäische Zeit          ZZD = -1
       WEZ  = WEST - europäische Zeit          ZZD = 0
            = UT = GMT
       OEZ  = OST -  europäische Zeit          ZZD = -2
       DSZ  = Deutsche SOMMER-Zeit             ZZD = -2
       DDSZ = Doppelte Deutsche SOMMER-Zeit    ZZD = -3
```

Wollen Sie ORTS-Zeit eingeben,was bei HISTORISCHEN Horoskopen nötig ist, geben Sie in den Zeit-Feldern die angegebene Ortszeit ein und markieren die betreffende CHECKBOX "ORTSZEIT".

Bitte nicht ORTSZEIT mit ZONENZEIT verwechseln.In HORCOM wird Ortszeit immer im ursprünglichen Sinn gebraucht.

Vor 1810 ( Deutschland ) bezog sich die Ortszeit jeweils auf den WAHREN Sonnenstand.

Da die UT auf einem rechnerischen MITTLEREN Sonnenstand beruht,ist für die Umsetzung von WAHRER Ortszeit in MITTLERE Ortszeit,für Daten vor 1810,noch die ZEITGLEICHUNG zu berücksichtigen.Diese Korrektur kann bis etwa +,- eine viertel Stunde betragen.

Sie wird nun ( ab Ausgaben 2005 ) für Daten < 1810 automatisch berücksichtigt.

Für Daten ab 1810 ( Deutschland ) aufwärts ist zumindest zu vermuten,daß

```text
die MITTLERE Ortszeit gemeint ist,kurz    LMT = Local Mean Time
                                      oder MOZ = Mittlere Ortszeit.

Entsprechend                              LTT = Local True Time .
                                      oder WOZ = WAHRE Ortszeit.
```

In Frankreich ist das Stichdatum 1816.In vielen anderen europäischen Ländern erst 1884 also relativ kurz vor Einführung der ZONENZEIT 1890.

Für Daten zwischen 1810 und 1890 kann daher gewählt werden,welche Ortszeit eingegeben werden soll.

Eine weitere CHECKBOX unter ORTSZEIT dient dazu zu vermerken,wenn nach dem 15.10.1582 noch der JULIANISCHE Kalender benutzt wurde.Wenn ja wird unter BEMERKG. die Zeichenfolge "(JULIAN.)" gespeichert,die dann beim Holen aus der Daten-Datei wieder abgefragt und angezeigt wird,aber nur soweit diese Datensätze mit der vorliegenden Version von HORCOM erstellt wurden,was bei der DEMO-Datei noch nicht der Fall war.In der Daten-Datei wird nämlich immer nur die UT gespeichert.

Eine umfangreiche Dokumentation der Zeit-Bestimmungen für Europa auf Diskette wurde dankenswerterweise von Herrn B.MAHL erstellt und kann bei der Eingabe durch Anklicken des BUTTONS "ZEITBESTIMMUNGEN LESEN" gelesen werden.

Nach den Zeit-Feldern wandert der CURSOR zunächst in das untere Feld,in dem Sie kurze KOMMENTARE,bis 50 ZEICHEN eingeben können.

Falls Sie in keiner der CHECKBOXEN angeklickt haben,wird die eingegebene Zeit als UT = GREENWICH-ZEIT interpretiert !

Wollen Sie während der Eingabe DEZIMAL-GRADE in GRAD/MIN/SEK umrechnen, drücken Sie die Funktionstaste F5 ( oder ALT + R ) womit eine Hilfsroutine zugänglich wird,die Sie mit dem QUIT wieder verlassen.

Die Abfrage,ob ein Datensatz abgespeichert werden soll,kommt bei Datensätzen aus der Datei nur,wenn NEU eingegeben wurde oder ein aus der Datei geholter Datensatz geändert wurde.

Die Eingaben werden nicht daraufhin geprüft ob sie sinnvoll sind.Das müssen Sie selbst tun.Unsinnige Eingaben führen zu nichts.Schlimmstenfalls zum Absturz des Programms.

Die Eingabe-Box dient auch als Anzeigebox,die für RADIX-Datensätze immer sofort editierbar (=veränderbar) ist.Diese Editierbarkeit hat zur Kehrseite, daß immer im 'OK'-Feld quittiert werden muß.

In der ANZEIGE-BOX ist die Zeit immer als UT = GMT = WEZ angegeben !

Die Anzeigeboxen mit RADIX-Daten werden immer in der oberen Bildschirmhälfte angezeigt,die der ERGEBNIS-Datensätze (SOLAR,LUNAR,DOPPEL-HOROSKOPE usw.) in der unteren.

Die SONSTIGEN EINGABEN,während der Programm-Bedienung,sind ebenfalls über DIALOG-BOXEN formatiert.Diese Dialoge zwingen Sie,Schritt für Schritt dem Programm das nötige mitzuteilen.ICONS brauchen Sie dabei nicht zu enträtseln. Alles ist immer im Klartext zu lesen,mit gelegentlichen Zugeständnissen an das Computer-Englisch,das meist kürzer ist als entsprechende deutsche Ausdrücke.

Lesen Sie,besonders zu Anfang die DIALOG-TEXTE immer genau,bevor Sie klicken.

GLEICHZEITIG können bis zu 5 EINGABE-SÄTZE eingegeben sein. Dies wird gelegentlich als zu wenig empfunden.

Wenn Sie viele Horoskope durchmustern wollen ohne die Eingabe zu füllen merken Sie in der Dateiliste den Datensatz nur mit Einfachklick an und klicken dann auf "NUR HOROSKOP ANSEHEN".

Wollen Sie einen Datensatz übernehmen müssen Sie direckt nach dem Aufrufen der Datei entweder einen Doppelklick auf die betreffende Zeile ausführen oder nach Markieren des oder der Datensätze "WAHL-ENDE" anklicken !

Sie können auch,wenn Sie viele Horoskope durchmustern wollen,eine "auswertefähige Datei" mit dem Modul "STATISTIK" erstellen ( in der Rubrik "EPHEMERIDE" ).Hier können Sie dann mit unterschiedlichen Kriterien und Bedingungen die Ausgabeliste erstellen und jeweils durch Anmerken eines Datensatzes das jeweilige Horoskop ansehen.

Ein Datensatz ist für weitere Menü-Punkte solange gültig,als nicht ein anderer Datensatz angewählt wird,auf eine Ergebnis-Ebene übergegangen ( z.B. SOLAR ),neu eingegeben oder rückgesetzt wird.

Der AKTUELLE = GÜLTIGE Datensatz ist im MENÜ durch ein HÄKCHEN markiert !!

Es kann also unter EIN/AUSGABE nur 1 HÄkchen gesetzt sein,bei den Ergebnissen unter HOROSKOPE,AUSWERTUNG dagegen mehrere,falls AUSGABE-BILDER vorübergehend GESPEICHERT wurden !!

Für den Anfänger wichtig ist,sich über den Begriff "EBENE" klar zu sein. Nach der Eingabe ist man in der RADIX-EBENE.Nach einem SOLAR in der SOLAR-EBENE usw.

Das AKTUALISIEREN bereits eingegebener RADIX- oder berechneter ERGEBNIS-DATEN-SÄTZE geschieht einfach durch ANKLICKEN mit der Maus ! Unter RADIX-DATEN können Sie die einzelnen RADIX-SÄtze AKTUALISIEREN = ANKLICKEN,die dann das Häkchen haben.

Sind Sie auf die "SOLAR-EBENE" oder "LUNAR-EBENE" usw. übergegangen, werden die Daten ebenfalls gespeichert und können unter der Zeile SOLAR...- DATEN wieder AKTIVIERT werden,ganz analog wie bei RADIX-DATEN.Gespeichert wird nur ENTWEDER SOLAR oder SEPTAR oder LUNAR oder TAG-HOR.

Sie können auf die RADIX-Ebene zurückgehen,indem Sie unter RADIX-DATEN anklicken.

INAKTIVIERT geschriebene MENÜ-Punkte sind NICHT ansprechbar. INAKTIVIERT geschriebene Programme sind ebenfalls NICHT zugänglich.Wollen Sie solche Programme benutzen,müssen Sie meist in die RADIX-Ebene zurückgehen,in der alle Programme ansprechbar sind,falls mindestens ein Datensatz eingegeben ist !

Wird ein Datensatz AKTIVIERT,so sind evtl.zugehörige gespeicherte BILD-SCHIRME ebenfalls mit HÄKCHEN markiert (unter HOROSKOP.. oder AUSWERTUNG).

Das Wechseln zwischen ERGEBNIS-Ebene und RADIX-Ebene bedarf einiger Gewöhnung,gewährleistet aber auch,maximale VIELSEITIGKEIT.Programme, die aus der RADIX-Ebene herausführen,sind mit ^ markiert.

Von der ERGEBNIS-Ebene aus,sind NICHT MEHR ALLE PROGRAMME SINNVOLL also auch nicht zugänglich.

Mit "ERGEBNIS als RADIX" (unter "DIVERSES") können Sie,wenn Sie es sich zutrauen,trotzdem alle Programme verwenden,z.B.ein Solar eines Lunars machen.Aber Vorsicht damit ! Das ist etwas für Geübte.

Die DOPPEL-Horoskope COMPOSIT,COMBIN u.DOPPELKREIS bilden ebenfalls eine ERGEBNIS-Ebene,von der aus aber nur für COMBIN weitere Programme ansprechbar sind.

Aktivieren unter ERGEBNIS-DATEN,Zurückholen unter dem entspr.Programm in Rubrik HOROSKOPE bzw. AUSWERTUNG ,wenn dort ein HÄKCHEN ist ! Diese Bild-Speicherung ist nur während einer HORCOM-Sitzung gegeben und nur dann wenn die Rechenzeit 15 Sekunden übersteigt.

Achten Sie bei den DOPPEL-Horoskopen darauf,daß immer die 2 Datensätze schon eingegeben sind,bevor Sie das betr. Programm anwählen:Sonst wird die Eingabeprozedur unübersichtlich !

Eingegebene RADIX-Datensätze,einschließlich Kommentar,können Sie über DATEI EIN-AUS auf Diskette abspeichern und,nach Bedarf,von dort wieder holen,wobei Sie beim Suchen nach ALPHABET,GEBURTSTAG oder DATUM SORTIEREN können.

Eine Daten-Datei bedarf ab und zu der Pflege.

Das LÖSCHEN nicht mehr benötigter Datensätze ist möglich.Pro Löschvorgang können Sie jeweils bis zu 10 Datensätze markieren.

Die PROBE-Datei hat den Namen DEMO.DAT und befindet sich in dem Ordner \HORCOM\SPEZIAL .

Mit "Datei TRIMMEN" können Leer-Datensätze und Leerstellen vor dem Namen beseitigt werden.

Mit "Datei MINIMIEREN" werden mehrfach vorhandene Datensätze mit gleichem Namen und gleicher Geburtszeit auf jeweils einen reduziert. Sie können beliebig eigene DATEN-Dateien anlegen,indem Sie einen ersten Datensatz von Hand ("NEU-EINGABE") eingeben,dann die Abfrage "DATENSATZ ABSPEICHERN mit "JA" beantworten und oben im Editierfeld der daraufhin erscheinenden FILESELECT-BOX einen Namen Ihrer Wahl ( maximal 8 Zeichen eintragen.Dabei sind UMLAUTE zu vermeiden.

Beispiel : FAMILIE.DAT wäre ein zulässiger Name.

Alle DATEN-Dateien müssen in \HORCOM\SPEZIAL angelegt werden !Das geschieht, wenn Sie den Namen Ihrer Datei in der FILE-SELECTBOX eintragen,automatisch. Sie können DATEN-Dateien über den Programm-Manager von WINDOWS umbenennen,jedoch nur die Datei,nicht den Ordner ( = Verzeichnis ),den Pfad oder die Extension (= die 3 Buchstaben nach dem . )

Wollen Sie mit grossen Datenmengen umgehen,wird es zweckmässig sein,Ihre Daten in SPEZIAL-DATEIEN zu unterteilen.

In die Dateien nur RADIX-Daten eingeben,da diese beim wieder holen immer als Radix-Daten gelten !!!

Die Datensätze sind beim NAMEN auf 25 ZEICHEN,beim ORT auf 20 ZEICHEN,bei BEMERKUNGEN auf 51 ZEICHEN begrenzt.

Verschiedene Dateien können zusammengefügt werden mit "DATEIEN VERKETTEN". (siehe Erläuterung 9).

Nachtrag hinsichtlich des Formates der in HORCOM.. verwendeten DATENund ORTS-DATEIEN für Leute,die andere Dateien auf das HORCOM-FORMAT umsetzen möchten :

Die Dateien in HORCOM ( alle Versionen ) sind RANDOM ACCESS - Dateien mit folgender Aufteilung ( FIELD siehe z.B. GFA-BASIC ) .

1.DATEN-Dateien :

FIELD #1,2 AS ta$,2 AS mo$,5 AS ja$,2 AS ho$,5 AS mi$,8 AS ggl$,

```text
           Tag        Monat   Jahr    Stunde   Minute  Geog Länge
z.B.      15         11      +1979    08      15.50   +011.636
```

```text
          8 AS ggg$, 25 AS naa$,20 AS goo$,51 AS bem$
```

```text
        Geog Breite Name+Vorn.   Ortsname  Bemerkung
z.B.   +52.1264    Muster Hans  Magdeburg xxxxx....
```

Die Gesamtlänge eines Datensatzes beträgt 128 BYTE.

2. ORTS-Dateien :

```text
FIELD #1,8 AS ggl$,8 AS ggg$,20 AS goo$         = 36 BYTE / Datensatz
```

Bedeutung der Strings wie die gleichnamigen oben.

Die einzelnen Größen werden als Strings gespeichert,und,wo erforderlich, nach dem Auslesen mit VAL() wieder in ganze bzw. floating Zahlen umgewandelt,wobei in einigen Fällen Platz für das Vorzeichen vorgesehen ist.

Die gespeicherte Zeit ist UT = GMT !!

Wo oben ein + steht,ist immer das Vorzeichen + oder - mit vorzusehen.

Die Strings müssen natürlich mit den Stringfunktionen STR$(), MID$() , RSET(),LSET() usw. exakt auf das obige Format und Länge gebracht werden. Die drei letzten Strings sind linksbündig vorzusehen.

Bitte machen Sie derartige Umsetzungen nur,wenn Sie einige Übung im Programmieren haben.

STATISTIK-Dateien können nur von HORCOM verwaltet werden,da sie mit verschlüsselten Integerzahlen arbeiten.