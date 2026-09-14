# Erläuterung AAF-Ein-Ausgabe

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P). Wortlaut unverändert, weggelassen sind technische Abschnitte zur Installation, Tastatur- und Druckersteuerung der historischen Programmfassung.*

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