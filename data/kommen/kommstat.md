# Erläuterung Statistik

*Robert Rettig, aus der Original-Dokumentation von HORCOM (Ordner KOMMEN7P), unverändert bis auf die Formatierung.*

KOMMMENTAR zu "STATISTIK":

VORBEMERKUNG :

Ich empfehle Ihnen dringend,den folgenden Text einmal aufmerksam durchzulesen damit Sie bei Unklarheiten wissen wo Sie nachlesen müssen.Sie sparen damit kostbare Zeit.

Um die Möglichkeiten des Programms einzuüben,verwenden Sie am besten die Datei "MUSTER.STA" auf der SYSTEM-DISKETTE.

"STATISTIK" ist für den forschenden Astrologen entwickelt worden.Mit Sicherheit ist es nichts für den astrologischen Anfänger oder für unkritische Leute.

Ich neige keineswegs dazu,den Wert statistischer Betrachtung in der Astrologie überzubewerten.Dazu ist der Gegenstand zu komplex.

Aber die jetzt mit diesem Programm-Modul gegebenen Suchmöglichkeiten werden sicher von jedem Astrologen begrüßt werden,bzw. füllen eine lang empfundene Lücke in HORCOM.

Mit im engeren Sinn statistischen Methoden kann man m.E. nur ganz grundlegende Sachverhalte auffinden,wie dies z.B. die GAUQUELINS getan haben.Für individuelle Aussagen sind sie völlig irrelevant.

Wenn Sie versuchen sollten,Ihre astrologischen "Grundüberzeugungen" statistisch zu prüfen,werden Sie in den allermeisten Fällen umsomehr enttäuscht werden,je grösser Ihr Versuchs-Kollektiv ist.

Eigentlich ist aber jeder selbständig arbeitende Astrologe auch "Statistiker",da er nur etwas für bedeutsam halten wird,was aus dem "Zufall" herausfällt.

Das kann auch durchaus an einem einzigen Fall sichtbar werden.Allerdings ist die Quantifizierung schwierig und man sollte den "Zufalls"-Einfluss nicht unterschätzen.

In der Technik wird z.B. der Gehalt einer Nachricht durch ein Maß für deren Unwahrscheinlichkeit ausgedrückt.Mit dem vorliegendem Programm können Sie unwahrscheinliche Konstellationen aufspüren,bzw. entsprechende Hypothesen prüfen.Das ist für mich der eigentliche Sinn eines solchen Programms. Statistik im engeren Sinn wird erst möglich bei Kollektiven etwa ab 400 aufwärts,falls Verteilungen auf die Häuser oder Zeichen geprüft werden.Der "Zufall" muß aber auch dann noch durchaus mit Streuungen bis ca. +- 10% in der relativen Häufigkeit veranschlagt werden.

Falls Sie Verteilungen auf die einzelnen Grade für einzelne Faktoren prüfen wollen,brauchen Sie Kollektive ab 10000 aufwärts,was wohl selten möglich ist.

Die Rechenzeiten würden dann auch recht unangenehm lang werden. Im Einzelfall kann es wirklich schwierig sein,zu entscheiden ob ein Ergebnis aus dem "Zufall" herausfällt,bzw. "bedeutsam" ist oder nicht. Quantitative Verfahren zur Abschätzung der Bedeutsamkeit eines Befundes wurden einstweilen nicht mit eingeführt,da sie in der Regel nur für ganz definierte Fragestellungen formuliert werden können und auch nur wenigen Astrologen geläufig sein dürften.Dazu kommt die prinzipielle Schwierigkeit, "AprioriWahrscheinlichkeiten" aufgrund der astronomischen Gegebenheiten anzugeben,ohne die eine Berechnung von Vertrauenswerten nicht möglich ist. Kenntnisse über astronomische Verhältnisse sind jedenfalls nicht unwichtig. Zum Beispiel ist die Feststellung daß der Aszendent in unseren Breiten in den Zeichen LE,VI,LI,SC,SG häufiger ist als im AQ,PS,AR,TA,GM für alle Kollektive,zu erwarten,da sie auf geometrischen Verhältnissen beruht. Die Verwendung von "STATISTIK" im eigentlichen Sinn des Wortes erfordert also sorgfältige eigene Überlegungen des Anwenders hinsichtlich der Bedeutsamkeit von Ergebnissen.

## BEDIENUNG

Die Bedienung von "STATISTIK" dürfte für Benutzer,die HORCOM bereits kennen, keine Probleme bieten,da gerade hier die Dialog-geführte Bedienung konsequent praktiziert ist.

Beim Anwählen des Menü-Punktes "STATISTIK" im Haupt-Menü sehen Sie ein erstes Unter-Menü mit Namen "WAS WOLLEN SIE TUN ?",dessen erster Balken "VORGABEN ÄNDERN" hat folgende Funktion:

Sie können hier vormerken,daß Ihre Statistik-Datei jeweils UPGEDATET wird, wenn Sie einen DATENSATZ in einer DATEN-Datei abspeichern,falls bereits eine STATISTIK-Datei gleichen Namens im Ordner \STATIST existiert. Da die Daten mit den richtigen Parametern übernommen werden müssen,ist der Vorgang automatisch,ohne Eingriffsmöglichkeit,und dauert etwas,da vorsichtshalber neu berechnet wird.

ACHTUNG !! Wenn in einer STATISTIK-Datei ein Datensatz mit einem Namen gespeichert werden soll,der bereits existiert,so wird dieser abgewiesen.Das soll gewährleisten,daß nicht 2 oder mehr identische Sätze in der STATISTIK-Datei vorhanden sind.Wenn Sie diesen Satz doch speichern wollen,müssen Sie den Namen ändern.Ein Buchstabe oder Ziffer genügt.

Datensätze,die in der DATEN-Datei zu Korrekturzwecken ÜBERSCHRIEBEN werden, werden allerdings ebenfalls in der STATISTIK-Datei neu eingeschrieben,wenn das UPDATEN eingeschaltet ist.Das funktioniert nur wenn die Namen in beiden Dateien übereinstimmen.Bitte auch nicht versuchen,neue Datensätze unter gleichem Namen "einzuschmuggeln".Das geht schief !!

Ausserdem können Sie nachfolgend wählen,ob Sie in der Ausgabeliste mehr Information in Kleinschrift oder weniger in grösserer Schrift haben wollen., Der zweite Menü-Balken "AUSWERTEFÄHIGE DATEI ERSTELLEN" hat folgende Funktion:

Dieses Programm macht aus einer auszuwählenden DATEN-Datei,die im Ordner \SPEZIAL\... .DAT vorausgesetzt wird,eine auswertefähige Datei,indem für jeden Datensatz die Planeten- und Häuserpositionen berechnet und abgespeichert werden.Bei grösseren Dateien kann dieser Vorgang etwas lange dauern. Jeder Datensatz beansprucht 210 BYTE auf der Festplatte.

Von den Häuserspitzen werden AC,MC sowie Spitze 2,3,5,6 gespeichert. Die AUSWERTEFÄHIGE DATEI = A.D. wird im Ordner \STATIST\ mit der Extension .STA abgelegt und hat im übrigen den gleichen Namen wie die zugrunde liegende DATEN-Datei.

ACHTUNG ! Der Ordner \STATIST wird normalerweise vollständig vom Programm verwaltet.Also bitte nichts selbst hineinladen oder entfernen.Das könnte zum Absturz führen.

Folgendes ist allerdings zu beachten :

Falls Sie eine DATEN-Datei ( Extension .DAT ,Ordner SPEZIAL ) löschen,zu der auch eine STATISTIK-Datei ( Extension .STA ,Ordner STATIST ) existiert, ist es empfehlenswert,auch die betreffenden STATISTIK-Dateie gleichen Namens über den Menüpunkt "STATISTIK-DATEI LÖSCHEN" ( also nicht über das DESKTOP ) zu löschen.

Wird z.B. die DATEN-Datei \HORCOM\SPEZIAL\MUSTER.DAT über das Desktop gelöscht,vergessen Sie nicht,über den erwähnten Menüpunkt \MUSTER.STA eben falls zu löschen.Sonst kann es,wenn eine DATEN-Datei unter gleichem Namen wieder angelegt werden sollte,zu unsinnigen Ausgaben bei "STATISTIK" kommen.

Bitte achten Sie darauf,daß Sie die Parameter zur Planetenberechnung gemäß Ihren Gewohnheiten eingestellt haben,sonst müssen Sie das Ganze nochmals wiederholen.Diese werden im Ordner \STATIST ebenfalls gespeichert und bei der Auswertung wieder restauriert.Wundern Sie sich also nicht,wenn nach einer Statistik-Auswertung die Ephemeriden-Parameter evtl. geändert sind. Die Rechenergebnisse sind als 4-Byte-Integer-Zahlen gespeichert.Das genügt, um die Werte weit besser als 1 Bogensekunde Genauigkeit zurückzuholen.Ebenso werden die NAMEN gespeichert,nicht jedoch die BEMERKUNGEN der DATEN-Dateien.

Der dritte Menü-Balken "AUSWERTUNG" erlaubt Ihnen die Anwendung vielseitiger Such-Vorgänge in Ihrer A.D.

Die Bedienung des Programms ist vollständig und Schritt für Schritt DIALOGgesteuert.

Ganze Zahlen können Sie über spezielle Boxen mit der Maus eingeben.Wollen Sie Dezimalzahlen eingeben,können Sie nach Drücken einer beliebigen Taste den Dezimalwert über die Tastatur mit Dezimal-PUNKT eingeben und mit "RETURN" weitergehen.Manchmal ist dieser Modus der bequemere. Sie erblicken als erstes eine Auswahlbox zur Wahl eines zu suchenden OBJEKTES.

Haben Sie dieses gewählt,erscheint eine zweite Auswahlbox mit verschiedenen Such-"KRITERIEN" und danach eine dritte Box mit der Sie zusätzliche ODER bzw. UND- Bedingungen eingeben oder direkt die AUSGABE-LISTE wählen können. Es wurde möglichste Vielseitigkeit angestrebt,sodaß auch die Bedienung schon etwas Übung verlangt.Einige Eingabemöglichkeiten sind doppelt vorhanden,um von verschiedenen Objekten her erreichbar zu sein. Z.B. können die Häuser (-Spitzen ) H2,H3,H5,H6 direkt gewählt werden,da sie direkt gespeichert sind,während die übrigen Häuser nur bei "HAUS NR." wählbar sind.Auch "HERR v. HAUS.." ist in zweierlei Weise zugänglich,um auch von anderen Objekten ( z.B. Halbsummen,Aspekten ) aus wählbar zu sein. Wollen Sie mit einem der Datensätze der Ausgabeliste weiterarbeiten,so klicken Sie die betr. Zeile einfach an.Es wird dann zunächst abgefragt ob Sie den Datensatz für weitere Untersuchungen übernehmen wollen.Was Sie hier auch antworten,in jedem Fall wird die HOROSKOP-GRAPHIK dargestellt. Wenn Sie mit "JA" antworten,wird er wie ein normaler RADIX-Datensatz weiterbehandelt.Allerdings fehlen dann eventuelle Bemerkungen. Dies kann auch zum SCHNELLEN DURCHMUSTERN VIELER HOROSKOPE benutzt werden indem die betreffende Zeile angeklickt wird !

Dabei kann auch ein ASPEKTE- und HALBSUMMEN-Zähler eingeschaltet werden,wie er auch bei ZEITWANDERN anwendbar ist.

```text
Wird in einer vollständigen Liste angeklickt kann der Zähler wahlweise auch   automatisch die gesamte Datei durchzählen.
```

Am übersichtlichsten sind einfache Fragestellungen oder Suchvorgänge,wie z.B. Welche Datensätze haben die Sonne im Zeichen Zwillinge oder: Bei welchen steht der Herr von Haus 1 im Haus 10 oder: Bei welchen Datensätzen liegt die Halbsumme SO-JU auf dem MC oder irgend einem anderen Faktor. Die normale Art der AUSGABE ist dann eine Liste der Datensätze,die das SuchKriterium erfüllen,wobei nach Namen sortiert wird.

Links oben im Bildschirm ist das gewählte Kriterium in einem kleinen Rechteck nochmals aufgeführt,in einer qwer beschriebenen Box die Anzahl der gefundenen Datensätze,die das Suchkriterium erfüllen.

Falls Sie nach NAMEN suchen,benutzen Sie bitte das Suchkriterium NAME ( BUCHSTABENFOLGE ).Dann sucht das Programm alle NAMEN nach der betreffenden Buchstabenfolge durch.Falls Sie z.B. alle Namen suchen,die JOSEPH,JOSEF oder JOSEFA enthalten,geben Sie einfach "JOSE" oder "jose" oder "jos" ein. ACHTUNG ! Auch LEERSTELLEN gelten als Buchstabe !

Wollen Sie die Liste ALLER NAMEN einer Datei sehen,geben Sie bei "NAME.." eine Leerstelle ein ( einmal Leertaste ).

Falls Sie einen GANZ BESTIMMTEN DATENSATZ suchen,suchen Sie bei "NAME.." und geben einen Teil des Namens ein,dessen Sie sicher sind,da es auf jeden Buchstaben ankommt.

Bei "ASPEKT" als Such-Objekt wird unterschieden zwischen EINZEL-Aspekt und Durchsuchen ALLER Aspekte bis zu einem bestimmten Teiler.Z.B. werden bei einem vorgewählten maximalen Teiler 4 nach Konjunktion,Opposition,Trigon und Quadrat gesucht ( Es ist zu beachten daß hier bei großem maximalem Teiler lange Rechenzeiten resultieren ).

Im diesem Fall wird der Orbis,wie überall sonst in HORCOM,proportional dem Grund-Aspekt berechnet,wobei die ORBIS-FAKTOREN mit "VORGABEN HOROSKOP ÄNDERN" eingestellt werden.Siehe auch dazu die Ausführungen in ERLÄUTE-RUNG 4.

Bei Eingabe von EINZEL-Aspekten wird der eingegebene Orbis nur eventl. für SO,MO,AC mit dem Faktor 1.5 versehen,falls dies vorgewählt ist. Die Konjunktion wird übrigens auch als "Aspekt 360 Grad" bezeichnet ! Innerhalb des SUCH-OBJEKTS "ASPEKT" können Sie auch Aspekte zwischen Einzel-Planeten/Häusern und HALBSUMMEN suchen.Dies dürfte für die Anhänger der Schule R.EBERTIN bzw. für die "HAMBURGER" wichtig sein ( und interessiert auch mich selbst ).Die übrigen Anwender dürfen sich über eine zusätzliche Abfrage ärgern.

Sie können "STATISTIK" auch zum Auflisten der PLANETEN oder sonstiger Objekte der ganzen Statistik-Datei benützen.Dazu wählen Sie in der Auswahlbox für das Suchkriterium "OHNE EINSCHRÄNKUNG ( 0....360 GRAD )".Sie erhalten dann alle Datensätze,geordnet nach der ekliptikalen Länge des vorgewählten Objektes.

Das gleiche erreichen Sie,allerdings mit alphabetischer Sortierung,indem Sie bei "NAME" eine Blank eingeben und gleich auf "AUSGABE" gehen. Bei Einfach-Auswertungen ( Ohne ODER- bzw. UND-Bedingung ) wird i.a. links im Bild noch eine Verteilung auf ZEICHEN bzw. HÄUSER angegeben,je nachdem, welches Kriterium gefragt ist.

Beim Suchobjekt "PLANETEN/HÄUSER..." gibt es neben den Einzel-Faktoren noch die Möglichkeit "ALLE PLANETEN".Dies soll dazu dienen,die Besetzung der Grade zu studieren.Man darf hier nur einen Orbis von maximal ca. 3 Grad eingeben,wenn nicht,droht Fehlermeldung !!.Grössere Orbes haben auch keinen Sinn,da z.B. schon bei 10 Grad fast alle Datensätze die Bedingung erfüllen würden.In der Praxis sollte man nicht über einen Orbis von 2 Grad hinausgehen.

Diese Möglichkeit ist vor allem für Einzel-Auswertungen gedacht und enthält nur in diesem Fall die volle Information.Der Planet,welcher die Bedingung erfüllt steht als Symbol vor der Länge dieses Planeten.

Es ist klar,daß diese Auswertung ein Vielfaches der Rechenzeit gegenüber einem Einzel-Faktor benötigt !

Als Zwischenstufe gibt es auch die Möglichkeit SO,MO,AC zusammen abzufragen.Hier kann auch neben einer Grad-Eingabe das Suchkriterium "IM ZEICHEN" noch verwendet werden.

Im Grunde handelt es sich hier um eine ODER-Verknüpfung ( siehe unten ). Auf Sortierung wurde bei diesen Objekten verzichtet,da sie entweder Speicher- oder Rechenzeitaufwendig wäre.

Bei diesen letzteren Objekten kann es auch vorkommen,daß folgende Meldung kommt, "ZU VIELE,ODER ZU UNSCHARFE BEDINGUNGEN" wonach Sie sich wieder im Menü befinden.Dies hat seinen Grund darin,daß die maximale Anzahl der gefundenen Datensätze auf das 1.5-fache der Gesamtzahl der Datensätze begrenzt wurde.

Für sinnvolle Ergebnisse darf natürlich die Anzahl der gefundenen Datensätze nur ein Bruchteil der Gesamtzahl sein.Wenn diese Meldung kommt,heißt das also,daß Ihre Frage bei weitem zu unscharf formuliert ist. Schwieriger gestaltet sich die Bedienung ( und gestaltete sich auch die Programmerstellung ) wenn mehrere Kriterien zugleich als "ODER-Verknüpfung" oder "UND-Verknüpfung" untersucht werden sollen,wie z.B:

Welche Datensätze haben die Sonne im Trigon mit einem Planeten ODER im

9. Haus ODER im Aspekt mit Jupiter oder im Schützen.Das wäre eine Frage-

stellung,wie sie sich in der astrologischen Betrachtungsweise von LUTZ RATHKE hinsichtlich einer JUPITER-Dominanz stellen würde.

In der "MÜNCHNER RHYTHMENLEHRE" von W.DÖBEREINER dagegen könnte z.B. folgende Frage geprüft werden:

Bei welchen Datensätzen treten "harte" Aspekte oder Spiegelpunkte zwischen SO und UR ODER eine Besetzung der Grade 22 ZWILLING ODER 22 SCHÜTZE auf.

Im Sinne der KOSMOBIOLOGIE von R.EBERTIN könnte vielleicht interessant sein zu prüfen,ob für "Erfolg" die HALBSUMME SO-MC-JU ODER SO-AC-JU bedeutsam ist.

Bitte beachten Sie,daß "UND"-Verknüpfungen sehr schnell dazu führen,daß Sie nichts mehr finden,es sei denn,Sie hätten sehr grosse Dateien. Bei einem Kollektiv von einigen hundert Datensätzen wird z.B. die Suche nach:

Sonne im Steinbock "UND" Mond im Steinbock "UND" Aszendent im Steinbock schon kaum noch einen entsprechenden Datensatz liefern.

"ODER"-Verknüpfungen hingegen erweitern die Anzahl der gefundenen Datensätze.

Anders ausgedrückt,sucht man mit "UND"-Bedingungen nach seltenen ( = interessanten ) Konstellationen,während die "ODER"-Verknüpfung mehr und mehr banale Ergebnisse liefert,je mehr Möglichkeiten zugelassen werden. ACHTUNG ! Bitte das UND nicht mit einem + verwechseln.Das UND ist ein rein logischer Begriff !

Die Bedienung einer einfachen "ODER"-Abfrage verlangt bereits,daß bis 12 mal geklickt werden muß,da auch in manchen Fällen ein Orbis eingegeben werden muß,was eine zusätzliche Abfrage bedingt.

Allerdings bleibt während der "UND"- bzw. "ODER"- Abfragen die Datei im RAM- Speicher.Erst wenn zu einer anderen DATEI gewechselt werden soll,muß wieder über das Menü eingestiegen werden.

Haben Sie einige Bedingungen bereits eingegeben und ausgelistet und wollen noch eine weitere hinzufügen,können Sie dies durch Eingabe von "W" oder "w" bewirken,ohne nochmals über das Menü einsteigen zu müssen ! Dies ist fast immer empfehlenswert um sich einer Fragestellung SCHRITTWEISE zu nähern. Nach Beendigung einer AUSGABE wird jeweils abgefragt,ob Sie weitere NEUE OBJEKTE mit der geladenen Datei auswerten wollen.Das geht dann direkt,ohne neues Laden.Bitte dies nicht mit dem obigen "W" verwechseln,das nur für neue Suchbedingungen mit einunddemselben Objekt gedacht ist. Bei zu vielen Bedingungen kann es vorkommen,daß der vorgesehene Speicherplatz nicht mehr ausreicht.Das wird allerdings nur bei ohnehin nicht mehr sinnvollen Fragen passieren ( hoffe ich ).

Treffen mehrere Bedingungen bei ODER-Verknüpfung auf einunddenselben Datensatz zu,so erscheint dieser auch mehrfach in der Ergebnis-Liste. Der Ausgabe-Bildschirm enthält bei zusammengesetzten Suchkriterien eine zusätzliche Informationsbox,links im Bild in der nochmals die gewählten Kriterien vermerkt sind,allerdings ohne Orbes ( Außer bei "ASPEKT" als Objekt ),wegen besserer Übersicht.Meist wird man auch bei diesen Orbes 0 wählen.

Es können maximal 12 Bedingungen ( evtl. in Kleinschrift ) vermerkt sein. Die UND-Bedingungen sind auf maximal 8 begrenzt.

Allerdings wird man bei 12 ODER-Bedingungen bereits fast alle Datensätze vorfinden,d.h. ein nichtssagendes Ergebnis erhalten.In der Praxis sollten Sie mit wesentlich weniger Bedingungen arbeiten.Bei 8 UND-Bedingungen werden Sie in aller Regel nichts finden,es sei denn Sie suchen etwas bereits Bekanntes,was ja nicht Sinn dieses Programms ist.

ACHTUNG ! Durch Klick auf die RECHTE Maustaste erscheint die Informations-Box zusätzlich waagrecht in der rechten Hälfte des Bildschirms und verschwindet wieder bei nochmaligem Klick.Bevor Sie weitergehen,müssen Sie diese eingeblendete Box wieder verschwinden lassen !

Die Bedingungen sind in diesen Boxen fortlaufend numeriert. Diese Nummern sind nach dem Namen ebenfalls eingetragen,damit Sie ersehen können,welche Bedingung von welchem Datensatz erfüllt wird.Bei "UND"- Bedingungen braucht diese Beschriftung mehr Platz und kann den Namen etwas kürzen.

Es können auch ODER- sowie UND-Bedingungen zusammen verwendet werden. Eine UND-Bedingung bezieht sich also immer auf ALLE Datensätze,die den vorher eingegebenen Bedingungen genügen.Es ist daher sinnvoll,eine UND-Bedingung als letzte einzugeben.Sonst würde die Sache auch sehr bald nicht mehr zu durchschauen sein.Dies ist auch insofern sinnvoll,als eine UND-Bedingung die Anzahl der gefundenen Datensätze einschränkt.

ALSO BITTE : UND-BEDINGUNGEN IMMER ALS LETZTE EINGEBEN !!

Die Datensätze,welche eine ODER-Bedingung erfüllen,haben am Ende EINE Ziffer,nämlich die Nr. der ODER-Bedingung.

BITTE BEACHTEN ! : Datensätze,die UND- Bedingungen erfüllen,sind daran zu erkennen,daß sie nach dem Namen mindestens 2 Ziffern und ein "u" enthalten. Z.B. heißt "2u3u4" ,daß die Bedingungen 1 ODER (2 UND 3 UND 4) erfüllt sind.

Der bzw. die Datensätze,die eine UND- Bedingungen erfüllen,sind durch INVERSION angemerkt !

Auch die Datensätze,welche nur Einzelbedingungen erfüllen,erscheinen ebenfalls noch in der Ausgabeliste,falls Sie in der Auswahlbox nicht "UND EXKLUSIV" gewählt haben ( siehe unten ).

Es können jetzt 2 Arten der Anzeige bei der UND-Verknüpfung gewählt werden:

UND INKLUSIV :

Hier werden die UND-Verknüpfungen jeweils mit allen vorhergehenden ODERbzw. UND-Bedingungen dargestellt.Es werden der,bzw. die "Treffer" der letzten UND- Bedingung durch Inversion angezeigt.

Wenn Sie z.B. hier die Anzeige "3u4" sehen,so bedeutet dies,daß erfüllt ist:

((1 oder 2 oder 3) und 4).

Bei mehreren ODER- ,sowie UND-Bedingungen muß man schon etwas studieren.Die Sache wird dann sehr schnell unübersichtlich.

UND EXKLUSIV :

Hier sehen Sie in der Ausgabeliste nur noch die Datensätze,die die letzte UND- Bedingung sowie etwa vorhergehende ODER- ,bzw. UND-Bedingungen erfüllen.

Diese Anzeigeart eignet sich vor allem für das Aufsuchen ganz bestimmter Konstellationen.Bei dieser Art der Anzeige wird es allerdings öfter vorkommen,daß die Meldung kommt : "KEIN DATENSATZ ERFÜLLT ALLE BEDINGUNGEN" , womit sie gar keine Ausgabe mehr sehen,sondern in die Objekt-Auswahl zurückkommen.

ACHTUNG ! Bei sinnlosen Eingaben wie z.B. SO in CP UND SO in CN kann man keine sinnvollen Ausgaben erwarten ! Es ist praktisch unmöglich,alle sinnlosen Eingaben durch eingebaute Sperren auszuschliessen.Im günstigsten Fall werden Sie bei sinnlosen Eingaben ins Menü zurückgeworfen.Unsinnige Anzeigen oder Abstürze sind aber ebenfalls möglich !!

Bei Versuchen mit komplexen Abfrage-Bedingungen muß gut überlegt werden,ob Bedingungen nicht untereinander zusammenhängen.So hängen AC und MC mit den übrigen Häusern gesetzlich zusammen,oder ME,VE können sich nicht weit von der Sonne entfernen usw,UR,NE,PL nur in bestimmten Zeichen zu erwarten sein usw.

Leider dürfte es kaum möglich sein,für jede Fragestellung eine "Apriori-Wahrscheinlichkeit anzugeben,so schön und wichtig dies auch wäre.Wenn man es denn könnte,würden auch gewaltige Rechenzeiten resultieren. Bei komplexen Fragestellungen die man an "SPEZIAL"-Kollektive stellt,ist es daher sehr wichtig,daß man an ein Kollektiv mit "DURCHSCHNITTS" - Fällen die gleiche Frage stellt,bevor man im Entdeckertaumel neue "Gesetze" ableitet.

Bei der Beschäftigung mit ODER- Bedingungen wird man finden,daß man schon bei recht elementaren Fragen zu verwaschenen Aussagen kommt. Suchen Sie z.B. nach einer "Waage-Betonung" etwa in folgender Form : SO in LI ODER MO in LI ODER AC in LI ODER SO in Haus 7 so werden Sie vielleicht feststellen,daß fast die Hälfte aller Datensätze "waagebetont" sind, was sicher nicht mehr sehr bedeutsam sein kann.

Derartige Experimente mögen dazu dienen,allzu unkritische Betrachtungsweisen nach und nach aufzugeben und die wirklich interessanten Fakten aufzufinden bzw. zu bestätigen.

Gerade die Beschäftigung mit solchen Fragen kann,wie ich glaube,dazu verhelfen,sich einen kritischen Maßstab anzueignen,was unter Astrologen nicht selbstverständlich ist.

Ich wünsche mir,daß dieses Programm in dieser Richtung erzieherisch wirken und zur Seriosität der Astrologie beitragen möge.

Ich betrachte die Bemühungen die Astrologie zu einer "Wissenschaft" zu machen nicht etwa mit Zustimmung.Aber auch innerhalb der Esoterik müssen die Gesetze der Logik und Mathematik gelten,die beide zutiefst "esoterisch" sind.
