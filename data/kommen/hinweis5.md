# Hinweise

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P), unverändert bis auf die Formatierung.*

Praktische Tips für die Arbeit mit HORCOM unter WINDOWS.

1.Installation von HORCOM .

Die Installation von HORCOM von CD geschieht normalerweise mit dem beigegebenen Installations-Programm INSTALCD.EXE durch Doppelklick auf dieses Programm aus Arbeitsplatz oder Explorer.

Falls Sie das Programm über das Internet heruntergeladen haben führen Sie immer das Programm INSTALH.EXE aus.

Achtung ! Der HORCOM - Ordner sollte nur einmal im Laufwerk C: vorhanden sein.Falls Sie also verschiedene Versionen aufbewahren wollen,diese umbenennen,z.B. HORCOMA.

Es ist zweckmäßig,das betr. HORCOM-Icon in der Bedienungoberfläche sichtbar zu machen :Das geschieht am einfachsten indem man mit ARBEITSPLATZZ > DATEI > VERKNÜPFUNG ERSTELLEN das HORCOM-Icon auf dem Desktop etabliert.

Der Start von HORCOM.. kann dann sehr einfach durch Doppelklick auf das HORCOM-Icon erfolgen.

2.Verhalten wenn HORCOM... während der Arbeit hängen bleibt.

Falls das sichtbare Fenster nicht reagiert,betätigen Sie das Maximierungsfeld ( Quadrat rechts oben ),was ansonsten ohne Konsequenzen bleibt und versuchen es dann nochmals.

Ein grösseres Programm wie HORCOM beinhaltet eine Unmenge von Rechen-Schleifen.Bei bestimmten Randbedingungen kann es vorkommen, daß das Programm in einer solchen Schleife hängenbleibt,oder kein HORCOM-Window mehr sichtbar ist.

Der Benutzer sitzt dann ratlos davor und überlegt ob er den Computer mittels des RESET-Knopfes neu starten soll.Gott sei Dank ist dies aber nur bei größeren Fehlern nötig.

Zuerst drücken Sie immer drei mal die Leertaste ! insbesondere dann wenn kein HORCOM-WINDOW sichtbar sein sollte.

Dann versuchen Sie die ESC - Taste .Immer mehrmals drücken ! ESC steht für ESCAPE = Entkommen.Man entkommt aber nur in den Fällen in denen das Programm diese Möglichkeit vorsieht.

Mißbrauchen Sie aber bitte nicht die ESC - Taste zu sehr für den Normalbetrieb,es sei denn bei mehrseitigen Ausgaben,wo ESC zum Ausstieg aus dem Blätterwald vorgesehen ist.Dann ist dies auch in der Titel-Zeile oder in der Ausgabe selbst angegeben.

Sonst sollte ESC mehr im Notfall versucht werden.

Wenn ESC nicht hilft kommt folgende Escalationsleiter in Anwendung :

a) Warten Sie zunächst 1 Minute.In vielen Fällen ist eine Zeit-

```text
   Begrenzung eingebaut.Wenn auch dann noch nichts geschieht,
   oder Sie ungeduldig sind,gehen Sie nach b) vor !
```

b) Drücken Sie die Tasten-Kombination STRG+ALT+ESC

```text
   Es erscheint eine Auswahlbox,in der Sie "Task beenden"
   drücken.Dann weiter wie unter b).
```

Damit sind Sie wieder im WINDOWS-Desktop.

Sollte dies nicht funktionieren,müssen Sie immer noch nicht den RESET-Knopf betätigen sondern es genügt wenn Sie die Tastenkombination STRG+ALT+ENTF drücken,womit der Computer nochmals startet ohne ausgeschaltet zu werden ( = Warmstart ).

Ein Hinweis auf einen fatalen Fehler ist,wenn die Farben der WINDOWS - Oberfläche sich ändern und der Computer nicht mehr auf Maus oder Tastatur reagiert.

Aber auch in diesem Fall kommen Sie mit der Tastenkombination STRG+ALT+ENTF meist wieder heraus.

Wenn auch dies nicht hilft bleibt immer noch die RESET-Taste als letzte Rettung,der sogenannte Kaltstart.

Falls innerhalb von EDITIER-Feldern der CURSOR einmal nicht mehr zu sehen sein sollte ,klicken Sie mit der linken Maustaste in eines dieser Felder,womit er dann ( hoffentlich ) wieder zu sehen ist.

Nach einem Absturz,also wenn Sie HORCOM nicht ordnungsgemäß verlassen konnten oder versehentlich den Computer bei laufendem WINDOWS ausgeschaltet haben,ist es zweckmäßig,vor einem neuen Start in der DOS-Eingabeaufforderung "scandisk" einzugeben,bei älteren Systemen "chkdsk" um verlorene Datei-Zuordnungen löschen zu können. Das Umwandeln in Dateien können Sie dabei verneinen,falls der Absturz bzw. die Abschaltung aus HORCOM erfolgte.

Bisher kam es bei mir und,soweit ich weiß,auch bei HORCOM-Usern, jedenfalls nicht vor,daß dabei Dateien selbst verloren gingen. Wenn doch,so könnte dies ein Hinweis auf ein Virus sein,das im System steckt.

In WINDOWS XP sind die genannten Effekte weniger zu befürchten.

Im übrigen ist es immer ratsam,eigene Dateien täglich zu sichern,z.B. in einem eigenen Ordner \BACKUP\.... und auch den Inhalt dieses Ordners z.B. wöchentlich auf Diskette zu sichern.

Ein VIREN-Schutzprogramm ist heutzutage auch sehr zu empfehlen und zwar sollten es gerade Laien schon dann installieren wenn noch kein Virus im Computer ist.Wenn es erst mal drin ist wird seine Entfernung schon schwieriger.Ich rede aus eigener trüber Erfahrung obwohl ich bisher kaum nennenswerten Austausch von Disketten mit anderen Usern habe. Besonders dringend ist dies wenn Sie On Line sind.

3. Besonderheiten bei Betrieb von HORCOM innerhalb WINDOWS.

HORCOM.. wurde noch mit einer Programmiersprache ( GFA-BASIC ) für WINDOWS 3.1 entwickelt.

Die Erfahrung lehrte inzwischen daß die späteren WINDOWS-Versionen nicht 100% kompatibel damit umgehen.Die besten Erfahrungen habe ich bisher mit den späteren WINDOWS 95 - Versionen gemacht.

Das bestgeeignete BILDSCHIRM-Format ist VGA 1024*768 mit 256 Farben oder mehr.

Mit HIGH-COLOR oder gar TRUE COLOR sollten Sie nur arbeiten wenn Sie einen wirklich gut ausgestatteten Computer besitzen da dies sehr viel Speicher benötigt.Wenn der Speicher bei irgendeinem Programm nicht reicht,stürzt das Programm mit "RUNTIME ERROR -12" ab.

Kurz gesagt,ist für HORCOM eine hochgezüchtete Graphik-Ausrüstung eher nachteilig.Es ist so als wollte man mit einem Formel 1 - Rennwagen zum Einkaufen fahren.

Dagegen ist ein schneller Prozessor,wegen der aufwendigen Planeten-Berechnung immer ein Vorteil.

Das Prinzip von HORCOM ist,daß immer der volle Bildschirm für die jeweilige Ausgabe zur Verfügung steht.

"Fensterzauber" ist also nicht vorgesehen.

Beim Arbeiten mit HORCOM in WINDOWS.... ist es zweckmäßig unter "EINSTELLUNGEN > TASKLEISTE" die Einstellung "Immer im Vordergrund" zu deaktivieren. Sonst wird der untere Teil der Ausgabe eventuell von der Taskleiste verdeckt.

Hinsichtlich der Graphik gilt das oben gesagte ebenfalls.

Sehr wichtig ist noch folgendes,wenn SUPER - VGA verwendet wird. In diesem Fall sind unter "ANZEIGE/EINSTELLUNGEN.." wahlweise "Kleine Schriftarten" oder "Große Schriftarten" wählbar.Merke ! HORCOM.. ist mit "Große Schriftarten" besser bedienbar.Für die hohe Auflösung 1024*768 können Sie sogar selbst eine Schriftgröße von 150% einstellen.Dann sind Menü-Einträge usw. schön groß zu sehen.

Hier können und sollen Sie auch die Schriftgröße in den Titelleisten oder die Größe der Symbole ( Ikons ) und deren Beschriftung fast beliebig einstellen.

All dies ist unter SYSTEMSTEUERUNG > ANZEGE > DARSTELLUNG zu finden.

Die Besonderheiten bei der Installation von HORCOM sind oben bereits genannt.Ab WINDOWS 95 kann hierzu auch aus der SYSTEM-STEUERUNG das Programm SOFTWARE verwendet werden.Die Verknüpfung mit dem Desktop sollte aber trotzdem,wie beschrieben,durchgeführt werden.

Falls Sie neben HORCOM gleichzeitig oder abwechselnd andere Programme betreiben und feststellen daß danach Farbänderungen in HORCOM z.B bei den Symbolen stattfinden,so liegt dies vermutlich nicht an HORCOM, sondern daran,daß das andere Programm die Systempalette verändert. Sie können dies eventuell rückgängig machen,indem Sie bei SYSTEM-STEUERUNG > ANZEIGE > DARSTELLUNG den Knopf "OK" betätigen.Damit wird normalerweise Ihre gewählte Farbpalette wieder restauriert.Wenn Sie danach HORCOM starten,sollte wieder alles beim alten sein.

Wollen Sie Ausgaben von HORCOM als Bilder über E-mail versenden ist es zweckmäßig die Ausgabe zuerst in das GIF-Format umzusetzen.Dies ist wie folgt zu bewerkstelligen:

Aus einer HORCOM-Ausgabe heraus drücken Sie F7 und gelangen in MSPAINT wenn dieses im Ordner ..\system32 steht.

```text
  Die Ausgabe dann mit EINFÜGEN in MSPAINT übernehmen.
  Dann im GIF-Format abspeichern und als Anhang versenden.
  Der Speicher- bzw. Zeitbedarf wird dadurch bis etwa den Faktor 100
  herabgesetzt.
```

```text
  ACHTUNG !
  Wenn Sie Ihren Computer ausschalten wollen ( auch in den Ruhezustand ) ist es
  zweckmäßig vorher HORCOM zu beenden,da sonst bei neuerlichem Start von HORCOM
  nicht mehr im Initial.Status vorliegt und bestimmte Funktionen fehlerhaft
  nicht mehr richtig laufen.
```

R.Rettig den 20.09.2008
