# Erläuterung Ein-Ausgabe

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P), unverändert bis auf die Formatierung.*

************************************************************************ VORBEMERKUNG :

Das folgende bezieht sich auf das Datei-Handling von HORCOM,wie es seit langen Jahren vorhanden ist.

Hinsichtlich Handhabung von AAF-DATEIEN innerhalb HORCOM7P,siehe den entsprechenden ANHANG unten !!

Die AAF-Eingabebox erscheint zusätzlich durch Drücken des BUTTONS "AAF-Format" links unten in der HORCOM-Eingabebox.

Der Erläuterungstext,speziell für das AAF-Datei-Handling erscheint innerhalb der AAF-Eingabebox durch Drücken der Taste F1.

*************************************************************************

Die Kolonne EIN-AUSGABE dient,neben der EIN - und AUSGABE der Daten, auch der STEUERUNG des PROGRAMM-ABLAUFS.Dies verlangt bei einem komplexen Programm,das für Experimente offen sein soll,in jedem Fall etwas Übung ! Der Autor hat inzwischen ca. 12000 Arbeitsstunden in HORCOM investiert.Etwa 50 Stunden wird auch der "User" aufwenden müssen um einigermaßen auszuschöpfen was HORCOM bietet.

Der Dialog zwischen HORCOM und Benutzer ( oft kurz "User" genannt ), erfolgt hauptsächlich über das HORCOM-Menü,über DIALOG-BOXEN,TASTATUR-Eingaben und einige FUNKTIONS-TASTEN ( F1,F2,F3,F5,F8,F9 oben auf der Tastatur ).Eine Maus ist praktisch unverzichtbar.

ACHTUNG ! Die Tasten- und Mausfunktionen werden meist beim LOSLASSEN aktiv, mit Ausnahme der ALT- oder STRG-Taste.Es hat also keinen Sinn auf einer Taste zu bleiben,wenn eine Aktion unterbleibt,sondern man muß dann ein zweites mal betätigen.

Die Belegung der FUNKTIONS-TASTEN ist unter "EINFÜHRUNG" im Menü jederzeit zu ersehen.

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
Die Funktionstasten werden teilweise nur aus den Ausgabe-Bildschirmen
abgefragt.Wenn nicht muß man die entsprechenden Menü-Titel anwählen.
```

## DATEI EIN/AUSGABE

ACHTUNG !!

Falls Ihnen bei bestimmten Datei-Operationen eine Datei abhanden kommt oder verstümmelt wird,sehen Sie nach ob in dem Ordner ..\HORCOM\SPEZIAL eine Datei3 "RRESERVE.DAT" entstanden ist,die nun die Datensätze enthält und benennen diese mit dem Dateimanager entsprechend wieder um.

Hier können Sie Datensätze aus der DEMO-Datei oder von Ihnen selbst angelegten Dateien holen ( siehe unten ) oder in andere Dateien abspeichern oder nicht mehr benötigte Datensätze löschen.

Die Daten einiger Prominenter sind in der \DEMO.DAT auf Ihrer Programm-Disk gespeichert (ohne Gewähr!) Diese Daten können Sie verwenden um sich mit der Programm-Bedienung vertraut zu machen.Die MC-Werte dieser Daten wurden oft dem Buch 'CIRCELS' entnommen (JAN CAMPHERBEEG AMSTERDAM, ISBN

906378044) und mit dem Menü-Punkt KORREKTUR in UT umgesetzt.Bei Großstädten

wurde dabei immer die Ortsmitte angenommen,was in der Regel nur genähert gilt.

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

Hiermit können Sie Optionen hinsichtlich der Ein-Ausgabe aktivieren bzw. deaktivieren.Z.B. die DRUCKER-Option,das Format von HARDCOPYS oder die Art des Weiterschaltens in Editierfeldern.

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
```

```text
       WEZ  = WEST - europäische Zeit          ZZD = 0
            = UT = GMT
```

```text
       OEZ  = OST -  europäische Zeit          ZZD = -2
```

```text
       DSZ  = Deutsche SOMMER-Zeit             ZZD = -2
```

```text
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
```

```text
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

Falls beim Überschreiben eines Datensatzes einmal eine Datei "verschwindet", infolge eines Fehlers,lohnt es sich nachzusehen ob die Datei unter dem Namen \HORCOM\SPEZIAL\RRESERVE.DAT noch existiert,die man dann entsprechend wieder umbenennen kann.

GLEICHZEITIG können bis zu 5 EINGABE-SÄTZE eingegeben sein. Dies wird gelegentlich als zu wenig empfunden.

Wenn Sie viele Horoskope durchmustern wollen ohne die Eingabe zu füllen merken Sie in der Dateiliste den Datensatz nur mit Einfachklick an und klicken dann auf "NUR HOROSKOP ANSEHEN".

Wollen Sie einen Datensatz übernehmen müssen Sie direckt nach dem Aufrufen der Datei entweder einen Doppelklick auf die betreffende Zeile ausführen oder nach Markieren des oder der Datensätze "WAHL-ENDE" anklicken !

```text
Sie können auch,wenn Sie viele Horoskope durchmustern wollen,eine        "auswertefähige Datei" mit dem Modul "STATISTIK" erstellen ( in der Rubrik        "EPHEMERIDE" ).Hier können Sie dann mit unterschiedlichen Kriterien und      Bedingungen die Ausgabeliste erstellen und jeweils durch Anmerken eines      Datensatzes das jeweilige Horoskop ansehen.
```

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

## AUFRÄUMEN/RÜCKSETZEN

Löscht alle gespeicherte BILDSCHIRME und Datensätze und macht den entsprechenden SPEICHER FREI.

## DRUCKER OPTION EIN/AUS

Bereitet HORCOM zum Ausdrucken der Ergebnisse für die von HORCOM gesteuerten Drucker-Ausgaben vor.

Dies ist deshalb zweckmäßig,weil bei der reinen Bildschirm-Arbeit weniger Abfragen nötig sind als beim Drucken.

Schaltet man diesen Punkt ein,so erscheinen zusätzliche Dialoge,welche die verschiedenen AUSGABE- und DRUCK-MODI zu selektieren gestatten und zwar nur die von HORCOM vorgesehenen.

Das Umschalten funktioniert übrigens auch mit "ALT und D" vom Ausgabe-Bildschirm aus oder mit der Funktionstaste F8.

HARDCOPYS ( = Druckerausgabe des Ergebnis-Bildschirms "PUNKT für PUNKT") können Sie auch ohne die Aktivierung dieses Punktes mittels WORD erzeugen. Siehe weiter unten die Ausführungen zum Thema DRUCKEN.

## LETZTES BILD HOLEN / BILDER SPEICHERN

Dient zum Zurück-Holen des LETZTEN Bildschirms,soweit dieser nicht eigens gespeichert ist.Das ist bei allen Ergebnissen möglich.

über "In Datei SPEICHERN" können Sie diesen Bildschirm auf DISC speichern und später über LADEN wieder hereinholen.Das kann zweckmäßig sein,wenn man ein Programm mit großem Zeitbedarf,z.B. TRANSITE bearbeitet hat und am nächsten Tag mit der Auswertung weitermachen will.

Beim "LADEN" brauchen Sie dann nur in der FILE-SELECTBOX den Namen anklicken und 'OK' drücken.

Die Bilder werden in einem Ordner \HORCOM\BILDER\*.BMP abgelegt. Diese Bilder sind bleibend gespeichert und erfordern viel Speicherplatz. Daher sollte der Ordner \BILDER regelmäßig geleert werden.

Verwenden Sie bitte dazu den obigen Menüpunkt und nicht den Datei-Manager, da sonst die Gefahr besteht daß Sie versehentlich mal etwas anderes löschen, was wichtig ist.

Achtung !! Machen Sie von dieser Bildspeicherung nur dann Gebrauch,wenn Sie NICHT AUFLÖSUNG oder FARBTIEFE verändern.Es können nur Bilder geladen werden die mit der aktuellen Auflösung erstellt wurden.

Auch hier gilt,daß ein schneller Prozessor diese Speicherung eigentlich meist überflüssig macht.

Wollen Sie Bilder über das INTERNET versenden ist das BMP-Format der HORCOM-Ausgaben sehr aufwendig.

Hierfür empfiehlt es sich,das betreffende Bild mit "Einfügen" in MSPAINT.EXE zu übernehmen und dann im GIF-Format abzuspeichern und dann dieses zu versenden.

---

Hinsichtlich DRUCKEN ist grundsätzlich folgendes zu beachten:

Die "DRUCKER-OPTION" muß EINgeschaltet sein.Entwerd mit dem Menüpunkt "DRUCKER-OPTION EIN/AUS" oder aus den Ausgabe-Bildschirmen heraus mit "ALT + D " oder mit F8.

* HORCOM ist ein sehr stark Bildschirm-orientiertes Programm.Daher gibt

```text
   es sämtliche Ausdrucke nur entweder als "HARDCOPYS", d.h. der Bildschirm
   wird Punkt für Punkt als BITMAP-GRAPHIK kopiert oder bei einigen wichti-
   gen Ausgaben ( Horoskope ) auch als DRUCKER-GRAPHIK wobei die höhere
   Auflösung des Druckers genutzt wird.
* Allgemein ist die Drucker-Unterstützung von WINDOWS wirklich vorbildlich
   zu nennen,soweit man dies angesichts des Fehlens einer verbindlichen Nor-
   mung erwarten kann.
   Aber dem "User" bleibt es trotzdem nicht erspart Experimente zu machen
   um etwa das zu erhalten was er sich vorstellt.Man kann mühelos dabei
   viele Quadratmeter Papier und viel Tinte/Farbband verbrauchen.
   Die Ursache dafür ist die nicht vorhandene Normung und die unterschied-
   liche Qualität der Druckertreiber.
   Die besten Ergebnisse werden Sie wahrscheinlich erhalten wenn Ihr Drucker
   eine Option für "EPSON-Kompatibel" aufweist.Stellen Sie zunächst diese
   Option ein,da die Firma EPSON fast schon so etwas wie einen allgemeinenen
   Standard für Drucker etabliert hat.
```

* Ihr Drucker muß mit dem richtigen "TREIBER" installiert und angemeldet

```text
   sein.WINDOWS bietet hunderte von Treibern.Sie werden mit Sicherheit einen
   brauchbaren finden.
   Zum Einrichten : SYSTEM-STEUERUNG >> DRUCKER >> EINRICHTEN >>
   OPTIONEN  durchgehen.
```

* Falls Sie in der Regel 2 Bildschirme auf eine Druckerseite ausdrucken

```text
   wollen,ist es zweckmäßig daß Sie bei DRUCKER "EINRICHTEN" unter SEITEN-
   LÄNGE eine "BENUTZERDEFINIERTE" Seitenlänge vorgeben,die der halben
   Seitenlänge des benutzten Papierformats exakt entspricht.
   ( Das erfordert bei manchen Druckern die Einstellung mittels DIP-Schalter )
   Falls Die exakte Seitenlänge des verwendeten Drucker-Papieres nicht in
   der vorgegebenen Liste vorhanden ist, ( bei meinem Matrix-Drucker z.B.
   nicht ) sollten Sie auf jeden Fall von dieser Möglichkeit Gebrauch machen.
   Ansonsten würde der Drucker immer eine leere Halbseite mit ausgeben,oder
   Beim Ausdrucken mehrerer Kopien aus dem Tritt kommen.
```

* Es ist unter WINDOWS recht einfach,beliebige Dokumente,also auch die Aus-

```text
   gaben von HORCOM als HARDCOPY auf Matrix-,Laser- oder Tintenstrahl-Druk-
   kern auszugeben.
   Man kann dazu das WINDOWS- Programm WORD oder ein vergleichbares Textver-
   arbeitungsprogramm aktivieren und unter BEARBEITEN auf EINFÜGEN klicken,
   womit der Inhalt des jeweils VORHER AKTIVEN Fensters in das WORD- Fenster
   kopiert wird.Man kann es dann entweder direkt oder nach Hinzufügen eines
   Textes oder zweiten Ausgabe-Bildschirmes mit "DRUCKEN" zu Papier bringen.
```

```text
   Praktisch geht dies am einfachsten in folgenden Schritten:
```

```text
   1. Ausgabe-Bildschirm mit HORCOM erzeugen.
   2. WINDOWS WORD oder ein anderes Textverarbeitungsprogramm öffnen
   3. Unter BEARBEITEN im MENÜ "EINFÜGEN" anklicken.
      Sie sehen nun die Ausgabe auch im WORD-Bildschirm.
      Nun können Sie auf der unteren Blatthälfte evtl. Bemerkungen dazu
      schreiben oder auch noch einen zweiten Ergebnisbildschirm hinzufügen.
      Dazu müssen Sie das "Ikonisieren" und wieder Vergrössern der Fenster
      beherrschen: WORD nicht verlassen,sondern ikonisieren und den zweiten
      Ausgabebildschirm mit HORCOM erstellen.
      Darauf wieder WORD ( das bereits den ersten Bildschirm enthält )
      vergrössern ( rechtes nach oben weisendes Dreieck anklicken !) und
      die letzte HORCOM-Ausgabe mit "BEARBEITEN >> EINFÜGEN" ebenfalls in
      das WORD-FENSTER übernehmen.
   4. Nun im WORD-MENÜ unter DATEI "DRUCKEN" anklicken.Daraufhin erscheint
      Der PRINTER-DIALOG den Sie,wenn einmal alles über "EINRICHTEN"  und
      "OPTIONEN" richtig eingestellt ist,nur mit OK zu beantworten brauchen.
   5. Danach verlassen Sie WORD wieder mit dem Schließfeld.
```

```text
   Eventuell kann auch statt WORD ein Programm wie NETSCAPE-COMPOSER oder
   FRONTPAGE-EXPRESS verwendet werden.
```

* Daneben können Sie,wenn die DRUCKER-OPTION EINgeschaltet ist über HORCOM

```text
   eine HARDCOPY einleiten indem Sie in dem kleinen Dialog rechts unten
  "HARDCOPY ?" ,der beim Verlassen einer Ausgabe erscheint,mit JA antworten.
```

```text
   Was das Ausdrucken von mehrseitigen Listen anbelangt,bitte wie folgt vorgehen:
```

```text
   Vorher unter "VORGABEN EIN-AUSGABE ÄNDERN" das Hardcopy-Format einstellen.
   Dann am besten unter "VORGABEN DIREKTIONEN ÄNDERN" das "BIS ZUM
   ENDE LAUFEN" aktivieren.
```

```text
   Dann die gesamte Liste erst mal mit allen Seiten am Bildschirm erstellen.
   Dann in der Liste zurückblättern auf Blatt eins z.B. mit der Bildnachoben-
   Taste.Dann die Drucker-Option einschalten,mit "ALT + D".Beim Weiterblättern
   mit der Leertaste oder Bildnachunten-Taste erscheit rechts unten jeweils
   die Abfrage "HARDCOPY ?" dann diese jeweils mit JA beantworten usw.
```

* HORCOM bietet auch die Möglichkeit bis zu 2 Ausgabe-Bildschirme auf einer

```text
   DINA 4-Seite im HOCHFORMAT auszugeben.Zwei Ausgaben füllen dann eine DINA4-
   Seite.
   Dies geschieht wie folgt,wobei die Funktionstaste F9 zur STEUERUNG dient:
```

```text
   1. Jede Ausgabe,die man darstellen will,mit F9 speichern.Der Pfeil-CURSOR
      verwandelt sich,solange dieser Modus aktiv ist in ein Kreuz.
```

```text
   2. Nachdem 2 Ausgaben gespeichert sind,aus dem Menü heraus oder,bei längeren             Listen direkt,wieder F9 drücken,worauf die DOPPEL-Ausgabe veranlaßt werden
      kann wonach der Doppel-Bild-Speicher wieder gefüllt werden kann usw.
      Etwas Übung muss sein.
```

* Darüberhinaus habe ich mich bemüht,für besonders wichtige Ausgaben,wie

```text
   Horoskope,DRUCKER-GRAPHIKEN zu programmieren,die dann zugänglich sind
   wenn die DRUCKER-OPTION im HORCOM-Menü auf EIN geschaltet ist.
```

```text
   Das kann auch aus den Ausgaben heraus mit "ALT + D" oder mit F8 umgeschaltet
   werden !!
```

```text
   Der Schalt-Zustand ist im Haupt-Menü rechts unten zu ersehen.
   Die Drucker-Graphiken nutzen die HÖHERE AUFLÖSUNG des Druckers aus,
   müssen aber extra programmiert werden.
   Das Ergebnis hängt sehr von dem verwendeten TREIBER bzw. den Einstel-
   lungen des Druckers ab.Um die Lektüre Ihres Drucker-Handbuches werden Sie
   kaum herumkommen,wenn Sie optimale Ergebnisse sehen wollen.
```

```text
   Das aktivieren der DRUCKER-OPTION von HORCOM hat also zur Folge,daß neben
   den HARDCOPYS,die nun auch aus HORCOM gestartet werden können,auch
   DRUCKER- GRAPHIK für HOROSKOPE und DOPPEL-HOROSKOPE ausgegeben
   werden kann.
```

```text
   Damit ist für HOROSKOPE auch eine GANZSEITEN-Graphik möglich,die auch eine
   Halbsummenliste enthält.
   Bei dieser und nur dieser Version wird auch nochmals die ZONENZEIT abge-
   fragt um in Grenzfällen einen Datums-Wechsel zu dokumentieren.
```

Bei hochauflösenden Druckern kann es vorkommen,daß Kreise ab einem bestimmten Durchmesser nicht mehr gezeichnet werden.Das kommt davon wenn der hochwertige Drucker mit einem dummen Treiber betrieben wird,der kein "banding" beherrscht, oder der Drucker-Speicher nicht der Auflösung entspricht.Dann sollte man versuchen mit reduzierter Auflösung zu arbeiten.

```text
  Bei den HARDCOPYS sehen Sie auf dem Drucker genau das,was Sie am Bild-
  schirm sehen,wobei raffinierte Druckertreiber das Bild offenbar noch ver-
  bessern können durch "KANTEN-GLÄTTUNG",meist nur im Monochrom-Modus.
  Hardcopys funktionieren von jedem Ausgabe-Bildschirm und von jeder Bild-
  schirmauflösung aus und wohl auch mit jedem richtigen Treiber.
```

```text
  Bei der DRUCKER-GRAPHIK hingegen werden die Graphik-Elemente direkt in der
  Auflösung des Druckers zu Papier gebracht.Am Bildschirm sehen Sie dabei
  nichts,bzw. nur den CURSOR.Man sollte dazu einen schnellen Computer haben,
  da diese Ausgaben viel Rechenaufwand benötigen.Auch Arbeitsspeicher wird
  hierfür reichlich gefordert.
  Farben werden dabei nur von hochwertigen Treibern,bzw. Druckern "verstan-
  den",Schraffuren ebenfalls nicht immer.
  Gute Ergebnisse sind zu erwarten wenn der Drucker in einem "EPSON-kompa-
  tiblen" Modus läuft.Versuche sind unvermeidlich.Man kann Tage damit zubringen.
```

```text
  Probieren Sie,was Ihnen besser gefällt.
```

```text
  Ich bin oft mit Harcopys zufrieden.Hier gilt das berühmte WYSYWIG ( = what
  you see you will get ) oder besser.
```

```text
  Farbige Ausdrucke benötigen übrigens rein weisses Papier,wenn Farbtreue
  verlangt wird.Die Farbe Weiß wird nicht vom Drucker,sondern vom Papier
  geliefert !
```

---

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

************************************************************************ KOMMENTAR betreffend AAF = Astrologisches Austausch - Format.

```text
  ************************************************************************
   Es ist das Verdienst und die Initiative von MARTIN GARMS diesen
   Datei-Standard kreiert zu haben,um den Austausch von Dateien
   zwischen unterschiedlichen Astrologie-Programmen zu ermöglichen.
   Dafür gebührt ihm Dank und Anerkennung der Astrologen und der
   Programmierer.
```

```text
   Die Website von M.GARMS ist übrigens :
```

```text
                       http://www.sternwerkstatt.de
```

```text
   Das Format mag manchem zunächst etwas Kompliziert erscheinen.
   Man wird aber einsehen müssen,daß dies in der Natur der Sache
   begründet ist.
   Das Nachdenken über die Details,insbesondere der Zeitrechnung,
   bleibt keinem erspart,der exakt arbeiten will.
   Auch war es beim Entwurf dieses Formats das Ziel,möglichst allen
   Forderungen,auch und gerade der forschenden Astrologen,Genüge
   zu tun.
```

************************************************************************

HANDHABUNG von AAF-Dateien innerhalb HORCOM7P :

***********************************************************************

```text
   Der vorliegende Text erscheint,innerhalb der AAF-Eingabebox,wenn Sie
   die Taste F1 drücken !
```

***********************************************************************

Das Grundkonzept besteht darin,daß nach wie vor das Datei-Handling von HORCOM im Vordergrund steht,man aber,wenn es zweckmäßig ist,in der Radix-Ebene jederzeit in das AAF-Format wechseln kann um den größeren Informationsumfang des AAF-Formats zur Verfügung zu haben.

Es handelt sich also um eine Art "Doppelter Buchführung" für die Radix-Datensätze,die sich als nicht ganz einfach erwiesen hat.

Im Übrigen ist die Handhabung der HORCOM-Eingabe im Wesentlichen unverändert.Lediglich wird statt "O" für OST nun auch hier ein "E" verwendet, wie es im AAF-Format üblich ist.

Sie können nun,wenn dafür Bedarf ist,längere Kommentare usw. auch für Ihre bereits vorhandenen HORCOM-Datendateien in die jeweilig zugehörige AAF-Datei hineinschreiben und jederzeit wieder darauf zurückgreifen.

Wird eine neue AAF-Datei in den Ordner ...\AAFDATEN eingefügt so ist mit dem entsprechenden Menüpunkt "AAF-DATEI <> HORCOM-DATEI" die zugehörige HORCOM-Datei anzulegen.

Ist z.B eine neue AAF-Datei namens ..\AAFDATEN\NEUDAT.AAF gefunden worden, so wird eine HORCOM-Datei namens...\HORCOM\SPEZIAL\NEUDAT.DAT gebildet.

Die parallele HORCOM-Datei GLEICHEN NAMENS dient als Pilot.

Die umgekhrte Umwandlung von HORCOM in AAF-Format dürfte nur dann sinnvoll sein wenn Daten-Austausch mit AAF-kompatiblen Programmen vorgesehen ist.

Sind AAF-Dateien vorhanden,denen eine gleichnamige HORCOM-Datei entspricht so erscheint in der HORCOM- Eingabebox links unten ein Button namens "AAF-Format".Wenn Sie dort anklicken,erscheint die reichhaltigere AAF-Box in der Sie auch Änderungen vornehmen können.

Änderungen über die HORCOM-Eingabe sollten dann unterbleiben und dafür in die AAF-Eingabe gewechselt werden.

Zusatzinformationen gehen dann zwar teilweise in der HORCOM-Datei verloren, können aber jederzeit wieder über die AAF-Box eingesehen werden. HORCOM speichert z.B. die Zeit immer nur als GMT = UT , Bemerkungen werden nach 51 Zeichen abgeschnitten.Das stammt noch aus der Zeit als Speicher-Platz knapp war.Heute kann man dies vergessen.

ACHTUNG! Mit den Umlauten gibt es immer wieder Fehl-Übersetzungen,die kaum

```text
          vermeidbar zu sein scheinen,da die Dateien mit unterschiedlichen
          Code-Tabellen erstellt sein können.
          Im AAF-Format sind Kommata wichtige Trennzeichen ! Wenn an der
          falschen Stelle ein Komma "entsteht",z.B. weil statt eines Umlauts
          ein Komma "übersetzt" wird,kann der betr. Datensatz nicht mehr
          korrekt interpretiert werden.
          Sie erkennen das daran,daß beim Datum der 1.1.-4712 12h erscheint.
          Oder ( und ) es wird der Name verstümmelt.
          Dagegen konnte ich nur bedingt Abhilfe schaffen.
```

```text
          Bei Dateien,die Sie selbst innerhalb HORCOM erstellt haben,wird
          dies hoffentlich nicht der Fall sein,sofern Sie auf Ihrem Computer
          immer die gleiche Codetabelle eingestellt haben.
```

```text
          Für Zahlen gibt es keine derartige Problematik.
```

Bei "NEU-EINGABE" wird gefragt,in welchem Format Sie eingeben wollen. Das AAF-Format bietet u.a. die Möglichkeit,umfangreiche Kommentare,Quellenverweise,Bemerkungen zur Daten-Qualität und Suchbegriffe einzugeben. Dies ist,sowohl für den forschenden,als auch den beratenden Astrologen von großem Vorteil,weil er alle Daten sofort überblicken kann. Hinsichtlich der Einzelheiten,die bei der Eingabe zu beachten sind,siehe am Schluß dieses Kommentars.

Wenn Sie sich über die umfangreichen Einzelheiten des AAF-Standards informieren wollen,öffnen Sie die Hilfedatei ..\HORCOM\AAFHELP\AAF.HLP mit dem Button "AAF-HELP".

Innerhalb dieser Datei können Sie sämtliche Erläuterrungs-Texte von MARTIN GARMS lesen.Es sind nicht wenige.

Trotzdem sollten Sie die Mühe nicht scheuen,wenn Sie mit dem Format arbeiten wollen.

Achtung ! Wenn man AAF-Dateien mit einem Browser im Internet sichtet,liegen diese zunächst im HTM-Format vor.Achten Sie bitte dann beim Herunterladen darauf,daß Sie die AAF-Datei im Format .TXT speichern,oder gleich mit der Endung .AAF also nicht als HTM-Dateien.

Falls Sie das vergessen,ist allerdings dafür gesorgt,daß die "tags" der htm-Dateien eliminiert werden ( hoffe ich ).

TXT-Dateien,die im Ordner \AAFDATEN angetroffen werden,werden automatisch mit der Endung .AAF versehen.Kopieren Sie also nur Dateien mit der Endung .AAF oder .TXT in diesen Ordner und NUR SOLCHE,die tatsächlich AAF-Dateien sind.

Also KEINE SONSTIGEN TXT- oder HTM-Dateien hineinbringen!!

********************************************************************* ACHTUNG ! NUR REINES TXT-Format verwenden und nicht etwa RTF-Format ( = Rich Text Format ) oder gar WORD-Format.Das funktioniert nicht,da hierbei Formatierungs-Code mitgespeichert wird !!

*********************************************************************

Wenn Sie HORCOM7P erstmalig auf Ihrem Computer starten,werden Sie gefragt, wo Sie den Ordner,namens \AAFDATEN für die AAF-Dateien anlegen wollen. HORCOM7P sieht dafür 2 Möglichkeiten vor :

```text
  1.In dem Grundverzeichnis,das auch den Ordner \WINDOWS enthält.
```

```text
  2.Im Ordner HORCOM.
```

Falls Sie nur mit HORCOM Astrologie betreiben,ist die zweite Möglichkeit zu empfehlen,damit Sie immer wissen,wo sich die AAF-Dateien befinden.

Achtung ! HORCOM7P merkt sich den Pfad für die AAF-Dateien nur dann,wenn Sie beim ERSTEN Start die Festlegung treffen und HORCOM7P den Ordner entsprechend plaziert ! Also bitte diese Festlegung DORT treffen und nicht mit dem Explorer hinterher den Ordner \AAFDATEN verschieben und auf KEINEN FALL diesen nachträglich umbenennen !! Dies ist ein weiterer Grund den Ordner \AAFDATEN im Ordner \HORCOM zu plazieren und dort zu belassen.

Eine AAF-Datei,die Sie irgendwo erhalten,kopieren Sie immer in diesen Ordner ....\AAFDATEN.

Fallls diese Datei statt .AAF die Endung .TXT hat,wird die "richtige" Endung automatisch von HORCOM generiert.Es MUß sich aber in jedem Fall um eine echte AAF-Datei handeln,zu erkennen an den Zeilen-Anfängen: #A93: ,#B93: usw.

*******************************************************************

Aus dem AAF-Format werden nur die drei ersten Gruppen in HORCOM verwertet. Es sind dies :

```text
  AAF-A: Die "bürgerlichen" Ausgangsdaten für ein Horoskop
  AAF-B: Die kalendarisch und geografisch exakten Horoskopdaten
  AAF-C: Verschiedene Textinformationen wie Kommentar, Quelle etc.
```

Die Möglichkeiten der weiteren Gruppen,soweit sie Auswertung von Rechenergebnissen betreffen,können innerhalb HORCOM7P meist durch das Modul "STATISTIK" ( unter "EPHEMERIDE" ) erledigt werden,indem man "auswertefähige Dateien" aus den reinen Datendateien herstellt.Das geht sehr schnell.

Die Speicherung von Ergebnissen selbst,die im AAF-Format ebenfalls vorgesehen ist,erscheint heute nicht mehr sehr interessant,da solche Ergebnisse jeweils sekundenschnell berechenbar sind.

****************************************************************************

Das AAF-Format wird also einstweilen und sicher auch künftig,innerhalb HORCOM nur als RADIX-Datei benutzt !

****************************************************************************

Das Schreiben von AAF-Datensätzen kann im Prinzip mit einem Text-Editor geschehen.Dies setzt perfekte Kenntnis der Syntax-Regeln voraus ! Da wird es wohl wenige geben,die das fehlerfrei können !

Wesentlich sicherer für die spätere Lesbarkeit ist es daher,die Eingabe mit der AAF-Eingabebox über "NEU-EINGABE" zu bewerkstelligen.

Innerhalb der AAF-Eingabebox können Datensätze,die aus einer AAF-Datei geholt wurden,auch von Hand geändert und zurückgespeichert werden.

Für das Ändern eines Datensatzes gilt das folgende,was für NEUEINGABE aufgelistet ist.

****************************************************************************

Bei NEU-EINGABE mit der AAF-Eingabebox ( Nur HORCOM7P ) füllen Sie die Editier-Felder wie folgt aus :

```text
  1. "Name"
```

```text
  2. "Vorname"
```

```text
  3. "Horoskopart" füllen Sie mit der vorgegebenen Listen-Auswahl.
```

```text
  4. "Datum"
      Bei Daten vor Chr. geben Sie die ASTRONOMISCHE Jahreszahl MIT
      MINUSZEICHEN ein :
      Das HISTORISCHE Jahr 1 vor Chr. entspricht dem Jahr 0 in ASTRONO-
      MISCHER Zählung.
```

```text
      Beispiel:
```

```text
        ASTRONOMISCH : -500
        HISTORISCH   :  501 vor Christus
```

```text
      Jahreszahlen mit negativem Vorzeichen sind also IMMER in ASTRONOMISCHER
      Zählweise zu verstehen ! Die zugehörige HISTORISCHE Jahreszahl wird
      hinter der ASTRONOMISCHEN Jahreszahl in einem "passiven" Feld angezeigt.
```

```text
      Soll der JULIANISCHE Kalender noch NACH dem 4.10.1582 verwendet werden,
      was nicht selten nötig sein wird,wird der Jahreszahl ein "j" angehängt.
```

```text
      Entsprechend bietet das AAF-Format auch noch die Möglichkeit,VOR dem
      15.10.1582 den GREGORIANISCHEN Kalender zu benutzen.Dann wird ein "g"
      angehängt.Dies dürfte praktisch weniger in Betracht kommen.
      In HORCOM ist das NICHT vorgesehen.
```

```text
  5. "Zeit" als jeweilige "bürgerliche Zeit" eingeben,also entweder ZONENZEIT
     ( für jüngere Daten ) oder ORTSZEIT für Daten vor Einführung der Zonenzeit.
```

```text
      Bei ORTSZEIT ist,streng genommen,noch zu unterscheiden zwischen der:
```

```text
      MITTLEREN Ortszeit LMT = Local Mean Time,für Daten etwa nach 1810,die
      auf einer astronomisch definierten MITTLEREN SONNENZEIT beruhen und der
```

```text
      WAHREN Ortszeit    LTT = Local True Time,für Daten davor,die auf dem
      WAHREN SONNENSTAND beruhen und vor dem genannten Datum in jedem Fall
      vorliegt.
```

```text
     Der Unterschied zwischen beiden wird mit der sog. ZEITGLEICHUNG berechnet,
     um die Sie sich NICHT kümmern müssen.Er kann bis zu etwa einer viertel
     Stunde +- betragen ( Maxima im Februar und November ).
     Normalerweise wird bei Daten VOR 1810 die LTT ( nach Abfrage ) in LMT
     umgerechnet,da die heutige astronomische Zeitrechnung dies voraussetzt.
     Antworten Sie bei der entsprechenden Abfrage demnach mit 'OK'.
     Die Abfrage wurde belassen,damit Sie sich der Problematik der Sache
     bewußt bleiben.
```

```text
  6. "Ortsname"
```

```text
  7. "Land" ( = Auto-Kennzeichen für Nation ) geben Sie meist über die
      vorgegebene Liste ein.
```

```text
  8. "Juldatum" lassen Sie NORMALERWEISE LEER ( Eingabe ist allerdings
      NICHT deaktiviert ),es wird normalerweise später automatisch
      gefüllt.
      Das Juldatum ist die astronomisch exakte Zeitangabe für die Ephem-
      eridenrechnung und muß den Anwender normalerweise nicht interessieren,
      es sei denn er braucht es für astronomische Überlegungen.
      Das angegebene Juldatum bezieht sich auf UT = GMT !
      Die Ephemeridenzeit ET wird Programmintern ermittelt.Der Unterschied
      ET - UT beträgt z. B bis A.D. 0 über zwei Stunden ! Darum muß sich
      der Anwender aber NICHT kümmern !
```

```text
      Das Juldatum hat gegenüber anderen Zeitangaben Priorität.
      Wird primär ein JULDATUM eingegeben,so wird automatisch gesetzt:
        ZNAM       = GMT
        ZONE(ZZD)  = 00hW00 oder 00hE00 oder "*"
        SOMMERZEIT = "*"
      Die Datums-Und Zeitangaben beziehen sich dann auf UT = GMT.
      ACHTUNG ! Wenn Sie PRIMÄR ein Juldatum eingeben,brauchen Sie Datum
      und Zeit NICHT auszufüllen ! Die Felder werden errechnet und
      automatisch gefüllt.
```

```text
  9. "Breite" und "Länge" also die geographischen Ortskoordinaten.
```

```text
     "Länge" MUß IMMER AUSGEFÜLLT werden wenn ORTSZEIT vorliegt,
      da sie dann unmittelbar in die Zeit eingeht !!
```

10. "Zone ( ZZD )" füllen Sie normalerweise mit der vorgegebenen Listen-

```text
      auswahl.Das Feld 13. = "ZNAM" wird dann automatisch gefüllt.
      Die SORTIERUNG der Zonennamen ist ALPHABETISCH.
      Die ORTSZEITEN LMT,bzw. LTT finden Sie demnach etwa in der Mitte
      der Liste.
```

11. "Sommerzeit" füllen Sie mit der vorgegebenen Listen-Auswahl.

```text
      Bitte dieses Feld IMMER AUSFÜLLEN,wenn die Zeit stimmen soll und
      ZONENZEIT vorliegt die nicht schon Sommerzeit enthält !!
      Wenn danach bei "ZNAM" ORTSZEIT LMT oder LTT vermerkt wird,wird in
      diesem Feld automatisch ein "*" eingetragen !!
```

12. "COM","VIA","SRC","GZQ" füllen Sie nach Bedarf und den Regeln des AAF.

13. "ZNAM" wird normalerweise automatisch gefüllt ( siehe bei "Zone" ).

14. "CWORD","ATTRB" füllen Sie nach Bedarf und den Regeln des AAF.

Danach betätigen Sie "Speichern" und wählen die Datei in die der Datensatz gespeichert werden soll.Dabei können Sie auch einen NEUEN NAMEN für die Datei eingeben,die dann den aktuellen Datensatz als ersten enthält.

Wollen Sie für EIN Subjekt mehrere Datensätze speichern,müssen Sie z.B. bei "Zuname" ein Merkmal ( z.B eine angehängte Ziffer ) zusätzlich eingeben !!

Als SUCHKRITERIUM,um jeweils den richtigen zugeorneten AAF-Datensatz zu finden dienen: Zuname + Vorname.

ACHTUNG !

Falls Sie einen HORCOM-Datensatz löschen,wird auch ein zugehöriger AAF-Datensatz mit gelöscht.

**************************************************************************

Das ZUSAMMENSETZEN oder AUFTEILEN von AAF-Dateien können Sie mit NOTEPAD oder,bei größeren Dateien,mit WORDPAD erledigen.

Sie müssen dazu zwei Editierfenster geöffnet haben und wissen wie man den Zwischenspeicher ( = CLIPBOARD ) verwendet.

Bitte dabei IMMER NUR das REINE TXT-Format verwenden ! Beim Speichern aber immer die Endung .AAF anstelle von .TXT verwenden !!
