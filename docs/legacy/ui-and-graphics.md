# HORCOM7P Legacy Analysis, Part 2. UI, Menu System and Chart Graphics

Derived from `reference/HORCOM7P.lst` (50,397 lines). All line numbers refer to that file. German quotes are Robert Rettig's original comments and dialog texts.

---

## 0. Orientation: global setup that everything else depends on

| Line | What |
|---|---|
| 1–195 | Change log (German, dated). This is the single best source for intent; quoted throughout below. |
| 204–246 | Path bootstrap: `c:\horcom.pth` → `hrc$` (program folder) and `aafpth$` (`\AAFDATEN`). |
| 283–287 | `hff$ = hrc$ + "\MODERN.FON"` → `AddFontResource(hff$)`. The whole UI text engine depends on this font being installed. Removed again in `a1` (line 1081). |
| 293–357 | Global defaults. Key graphics ones: `dop = 1`, `amh& = 430   //Horoskop-Mitte`, `bmh& = 224`, `km = 0.95`, `hrg! = 0` (helio flag), `haw& = 1` (house system), `nasp& = 12`, `klsy! = 0` (small symbols), `hard& = 2`, `elem& = 1`, `moda& = 1`. |
| 3311–3338 | `get_aufl_bs` — screen metrics. `gdx& / gdy&` = pixel size, `gdx = gdx&/640`, `gdy = gdy&/480`. The entire program draws in a virtual 640×480 space and scales at draw time. Comment: `IF v < 3 / 4     // Maßstabs-Anpassung bei NICHT VGA`. `gdxt/gdyt = 0.95*gdx/gdy` above 1040 px "für text( bes. höhere Auflösung". `gdyh& = gdy& * 459 / 480` — the usable client height. |
| 9305–9319 | Angle constants: `po = PI/2`, `pu = PI/180`, `up = 180/PI`, `pv2 = 2*PI`, `kk = 1E-10`, `DEFFN nb(x) = x - pv2*INT(x/pv2)` (normalise to [0,2π)). |
| 28097–28124 | `yk()/xk()` — virtual→device (`*gdy` / `*gdx`, only if `gdx& > 640`); `yl()/xl()` — a second mapping normalised to 1024×768 (used by dialogs/lists). |

---

## 1. FEATURE INVENTORY VIA MENU TREE

### 1.1 How the menu is built

`PROCEDURE men2` — line 3377. It:
1. `DIM mend1$(105)`, reads 103 `DATA` items (`RESTORE men2`, label at line 3436) into `mend1$(0..102)`, loop `FOR i& = 0 TO 102 … EXIT IF menu$ = "***"`.
2. Patches indices 15–19 (Radix slots), 21–25 (Solar slots) and 27–29 (Doppel slots) at runtime by appending `": " + sol$(od,ze) + "  " + na$(od,ze)` so the menu shows which chart is loaded in which slot.
3. Patches index 84 (`UHR`): `IF zeuhr > 0 : menu$ = " *  => DATENSATZ 'UHR' G/H" ELSE " * UHR G/H"`.
4. Appends two empty strings (`mend1$(103)`, `mend1$(104)`) as array terminator.

The menu is installed in the main program at line 493: `MENU mend1$()`, handle cached in `hdmen& = MENU(15)` (line 494), destroyed in `ende` (line 862, `~DestroyMenu(hdmen&)`).

There is no second menu builder. No popup/context menu array exists. The "context menu" role is played by the right mouse button → `einzel_plan_wahl` (line 6615), which opens an `alertbox` (not a real menu).

Lines 495–678 are pure enable/gray/check logic — effectively the program's state machine expressed as menu state:
* `FOR iii& = 2 TO 8 : MENU iii&,MF_GRAYED` — the F-key legend rows are inert labels.
* `IF ze = 0` → gray 14–38 and 41–101 (no data record ⇒ almost nothing available).
* `srr$ = RIGHT$(sol$(od,ze),2)` (chart-type suffix: `IC`, `IS`, `IT`, `IX`, `AR`, `OR`, `IN`) drives fine-grained graying — e.g. Septar/Solar disable 37–42.
* `MENU aaa&,MF_CHECKED` marks the currently active data slot (15–19 / 21–25 / 27–29, lines 602–637) and marks which outputs have a cached BMP (`a!(od,ze,n)`, lines 638–662 → items 53, 54, 55, 57, 58, 59, 69, 74).
* Items 93 and 95 (`--------`) are permanently grayed (lines 597–598) — retired features (`notiz`/MSPAINT and `lad_exe`); cf. change log line 160 `// F7 WRITE usw. stillgelegt`.

Naming conventions in the labels (inferred, backed by code):
* `&` = Windows accelerator marker (`&ÜBER HORCOM`, `EI&N-AUSG.`, `H&OROSKOPE`, `D&IVERSES`).
* `G/H` suffix = item works geocentric and heliocentric (F6 toggles).
* `*` prefix = item does not need a loaded data record — exactly the set force-enabled at lines 567–578 / 593–600 regardless of `ze`.
* `^` suffix = item produces a derived chart that is written back into a data slot.
* `|` prefix — uncertain. In GFA BASIC menu arrays this is a formatting control (separator or `MF_MENUBARBREAK`); not confirmed from this listing. It is applied to the items that cache a screen BMP (53, 54, 55, 57, 58, 59, 69, 74) and is added at runtime only to index 28 (`men2`, line 3427). Flagged rather than guessed.

### 1.2 Keyboard shortcuts (event loop)

Handled in three places: the main loop (lines 736–853), `wart_gem` (6913–7011) for output screens, and `men3`'s inner loop (6433–6495).

| Key | Alt alias (`WM_SYSKEYDOWN` vkey) | Handler | Line |
|---|---|---|---|
| F1 | ALT+E (69) | `@erste_hilfe` (main) / `@wart_erl` → `ae1…ae9` (output) | 737, 749, 6954 |
| F2 | ALT+A (65) | `@zeige_horoskop` — temporarily overlays the radix chart in window #13 | 6542 |
| F3 | ALT+C (67) | `@calc_in` — launches Windows CALC | 6976 |
| F5 | ALT+R (82) | `@rechne1` — decimal ↔ °/′/″ converter | 6962 |
| F6 | ALT+H (72/104) | `@geohelio` — helio/geo toggle; `GOTO mainst1`. In `men3` only for `muuu& = 53` (chart graphic) → `GOTO men3_0` (full redraw) | 741, 6490 |
| F7 | ALT+F (70) | `@notiz_in` — opens MSPAINT to save output as GIF | 6967 |
| F8 | ALT+D (68/100), also 90 ('Z') | `@druck_enbl_alt` — printer option on/off | 745, 6981, 45111 |
| F9 | ALT+M (77) | `@mehrf_aus` / `@mehrf_1` — 2-up "Doppel-Ausdruck" buffer | 7451, 7463 |
| ESC (27) | — | exit output screen / quit confirmation dialog in main | 831, 6936 |
| RMB (`MOUSEK = 2`) | — | `einzel_plan_wahl` — per-planet highlight/filter | 6617 |
| PgUp/PgDn (33/34), `+`/`-` (107/187, 109/189), `R`/`V` | — | paging / step in `wart` | 7153–7167 |

### 1.3 THE COMPLETE MENU TREE

Indices are the `mend1$()` subscript = the value of `MENU(0)` / `muuu&`. Handler = target of the `ON … GOSUB` chain in `men3` (lines 6365–6376). Labels are verbatim (leading spaces trimmed for readability).

```
[0]  &ÜBER HORCOM
 ├─[1]   EINFÜHRUNG = ERLÄUTERUNG 1   || FUNKTIONS-Tasten : F1 ( oder ALT + E )
 │       = ZUSTÄNDIGE ERLÄUTERUNG                                       -> ae1
 ├─[2]   F2 ( oder ALT + A ) = HOROSKOP ANSEHEN ( In sonstigen Ausgaben )   [grayed label]
 ├─[3]   F3 ( oder ALT + C ) =  WINDOWS CALC(ULATOR) STARTEN                [grayed label]
 ├─[4]   F5 ( oder ALT + R ) = WINKEL/ZEIT DEZIMAL in G/H MIN SEK          [grayed label]
 ├─[5]   F6 ( oder ALT + H ) = HELIO- bzw. GEOZENTRISCH UMSCHALTEN         [grayed label]
 ├─[6]   F7 ( oder ALT + F ) = MSPAINT ÖFFNEN zum SPEICHERN von Ausgaben
 │       im GIF-FORMAT                                                     [grayed label]
 ├─[7]   F8 ( oder ALT + D ) = DRUCKER-OPTION EIN-AUS                       [grayed label]
 ├─[8]   F9 ( oder ALT + M ) = DOPPEL-AUSDRUCK AKTIVIEREN                   [grayed label]
 └─[9]   ""   (dropdown terminator)

[10] EI&N-AUSG.
 ├─[11]  DATEN-DATEI EIN-AUSGABE                                        -> a2dat   (21318)
 ├─[12]  NEU-EINGABE von DATENSÄTZEN                                    -> a3      (23527)
 ├─[13]  VORGABEN EIN-AUSGABE ÄNDERN                                    -> avg     (27036)
 ├─[14]  RADIX-DATEN:                                   [heading, grayed]
 │   ├─[15] SATZ1 …                                                     -> a4      (25530)
 │   ├─[16] SATZ2                                                       -> a4       (od=1)
 │   ├─[17] SATZ3                                                       -> a4
 │   ├─[18] SATZ4                                                       -> a4
 │   └─[19] SATZ5                                                       -> a4
 ├─[20]  SOLAR...-DATEN:                                [heading, grayed]
 │   ├─[21] SATZ1 …                                                     -> a5      (25549)
 │   ├─[22] SATZ2                                                       -> a5       (od=2)
 │   ├─[23] SATZ3                                                       -> a5
 │   ├─[24] SATZ4                                                       -> a5
 │   └─[25] SATZ5                                                       -> a5
 ├─[26]  DOPPEL-DATEN:                                  [heading, grayed]
 │   ├─[27] COMPOSIT                                                    -> a13     (30007)  (od=0)
 │   ├─[28] COMBIN         ("|" prefix added in men2)                   -> adop    (25563)
 │   └─[29] DOPPEL-KREIS                                                -> a12     (29299)
 ├─[30]  AUFRÄUMEN / RÜCKSETZEN                                         -> areg    (1124)
 ├─[31]  DRUCKER-OPTION EIN / AUS                                       -> pr_enabel (42607)
 ├─[32]  LETZTES BILD ZEIGEN / bzw.SPEICHERN                            -> screen  (26033)
 ├─[33]  ERLÄUTERUNG 2                                                  -> ae2     (9994)
 └─[34]  ""

[35] &EPHEMERIDE
 ├─[36]  VORGABEN EPHEMERIDE ÄNDERN                                     -> ave     (26394)
 ├─[37]  PLANETEN-KOORDINATEN                                           -> a91     (16728)
 ├─[38]  ZUSATZ-PLANETEN-KOORDINATEN                                    -> a10     (16898)
 ├─[39]  HELIOZENTRISCHE VERSION EIN/AUS                                -> a93     (16880)
 ├─[40]  * STATISTIK G/H                                                -> stat    (11830)
 ├─[41]  ERLÄUTERUNG STATISTIK                                          -> aestat  (10004)
 ├─[42]  GRAD-LISTE G/H                                                 -> grli    (30666)
 ├─[43]  FIX-STERN-POSITIONEN                                           -> stella  (17974)
 ├─[44]  ARABISCHE TEILE ( SENS.PUNKTE )                                -> arabt   (18286)
 ├─[45]  * INGRESSE SONNE-MOND-MC-AC                                    -> ingre   (10412)
 ├─[46]  * ET aus UT                                                    -> et_ut   (42137)
 ├─[47]  * UT aus ET                                                    -> ut_et   (42164)
 ├─[48]  * DATUM aus JD                                                 -> dat_jd  (42447)
 ├─[49]  ERLÄUTERUNG 3                                                  -> ae3     (10009)
 └─[50]  ""

[51] H&OROSKOPE
 ├─[52]  VORGABEN HOROSKOP ÄNDERN                                       -> avh     (27152)
 ├─[53]  | HOROSKOP - GRAPHIK                                           -> a11     (45227)   ***
 ├─[54]  | ASPEKTARIUM G/H                                              -> aspar   (19275)
 ├─[55]  | HALBSUMMEN-GRAPHIK G/H                                       -> aspar2  (19689)
 ├─[56]  MULTIPLE DIREKTIONEN / HARMONICS G/H                           -> multiple(43338)
 ├─[57]  | COMPOSIT ^                                                   -> a13     (30007)
 ├─[58]  | COMBIN   ^                                                   -> a14     (30448)
 ├─[59]  | DOPPEL-KREIS / 90-GRAD-KREIS ^                               -> a12     (29299)
 ├─[60]  ERLÄUTERUNG 4                                                  -> ae4     (10014)
 └─[61]  ""

[62] &AUSWERTUNG
 ├─[63]  VORGABEN DIREKTIONEN ÄNDERN                                    -> avd     (28921)
 ├─[64]  SOLAR-SEPTAR-LUNAR-PLANETARE-PERSONARE ^                       -> a16     (31067)
 ├─[65]  TAGES-HOR. / PROGRESS.- HOR.^                                  -> ta_pro  (18916)
 ├─[66]  ERLÄUTERUNG 5                                                  -> ae5     (10019)
 ├─[67]  MÜNCHNER RHYTHMENLEHRE                                         -> a17     (33266)
 ├─[68]  ERLÄUTERUNG 6                                                  -> ae6     (10024)
 ├─[69]  | SEKUNDÄR-DIREKTION / DYNAMOGRAMM                             -> a18     (35100)
 ├─[70]  SONNE (MOND)-BOGEN-DIREKTION                                   -> a19     (40789)
 ├─[71]  PRIMÄR-DIREKTION ( E.C.KÜHR )                                  -> aprim   (41790)
 ├─[72]  SYMB. DIREKTION: ÄQUATORIAL                                    -> ar_sys  (41292)
 ├─[73]  SYMB. DIREKTION: EKLIPT. G/H                                   -> asymb   (41362)
 ├─[74]  | TRANSITE                                                     -> a20     (37865)
 ├─[75]  * MUNDAN-ASPEKTE                                               -> mund    (40821)
 ├─[76]  ERLÄUTERUNG 7                                                  -> ae7     (10029)
 └─[77]  ""

[78] D&IVERSES
 ├─[79]  * HÄUSER-SYSTEM                                                -> hausw   (25580)
 ├─[80]  HÄUSER-TABELLE                                                 -> hausa   (17623)
 ├─[81]  ERLÄUTERUNG 8                                                  -> ae8     (10034)
 ├─[82]  KORREKTUR                                                      -> korr    (15846)
 ├─[83]  ZEIT-WANDERN G/H                                               -> zeitw   (38066)
 ├─[84]  UHR G/H   (rewritten by men2 to " * UHR G/H" or
 │                  " *  => DATENSATZ 'UHR' G/H")                       -> uhr     (8172)
 ├─[85]  * AR-DE aus EL-EB                                              -> arde_eleb (16592)
 ├─[86]  * EL-EB aus AR-DE                                              -> eleb_arde (16551)
 ├─[87]  * LT aus UT                                                    -> lt_ut   (16633)
 ├─[88]  * UT aus LT                                                    -> ut_lt   (16664)
 ├─[89]  * AUFGANG.........                                             -> auf_unt (11425)
 ├─[90]  FINSTERNISSE....                                               -> finst   (10741)
 ├─[91]  * DATEIEN VERKETTEN                                            -> daterw  (21222)
 ├─[92]  * AAF-DATEI < > HORCOM-DATEI                                   -> aaf_horcom (5566)
 ├─[93]  --------                                       [permanently grayed] (-> notiz, retired MSPAINT)
 ├─[94]  * WINKEL-UMRECHNUNG                                            -> rechne  (25261)
 ├─[95]  --------                                       [permanently grayed] (-> lad_exe, retired)
 ├─[96]  * HINTERGRUND-FARBEN                                           -> color_dial (7264)
 ├─[97]  ORT-WANDERN                                                    -> ortwandern (9588)
 ├─[98]  GROßES ( = PLATONISCHES ) JAHR                                 -> grossj  (9453)
 ├─[99]  DESKTOP ( QUIT HORCOM )                                        -> desct   (42604)
 ├─[100] ERGEBNIS als RADIX                                             -> erg_rad (43239)
 ├─[101] ERLÄUTERUNG 9                                                  -> ae9     (10039)
 ├─[102] ÄNDERUNGEN / HINWEISE / KURZANL.                               -> aendlist(6739)
 └─[103] ""   (set programmatically, men2 line 3433)
```

---

## 2. EVENT LOOP AND SCREEN FLOW

### 2.1 Startup (lines 198–429)

Path setup → `@indim` / `@get_aufl_bs` / `@get_aufl_pr` → `AddFontResource(MODERN.FON)` → `@kon_dhol` (load `\INTERN\KONSTA7P.INT` parameters) → splash window `OPENW #1,0,0,gdx&,410*gdy` with `@himmel` (starfield) + `@horcom_titel` → `@initpterm` / 8× `@readpterm` (ephemeris terms) → `@Symbhol` (loads all symbol bitmaps) → `@closew(1)` → falls into `mainst:`.

### 2.2 `mainst:` — the main menu screen (lines 430–855)

Three labels form nested re-entry points:

* `mainst:` (430) — full rebuild. `MENU KILL`, `moda& = 1`, `MODE 0`, `WHILE diha& > 16 : @dial_close : WEND` (unwind any stacked dialogs), `km = 0.95`, `@plnm`, sets window geometry defaults (`xmen&/ymen&/bmen& = gdx&-12/hmen& = 361*gdy`), then:
  ```
  TITLEW #1,"Astrologie - "+ddd$+"Programm | HORCOM7P   HAUPT - MENÜ | Ausgabe : "+ausgabe$
  OPENW #1,xmen&,ymen&,bmen&,hmen&,0
  SETWINDOWSTYLE 1, WS_CAPTION|WS_SYSMENU|WS_VISIBLE|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_THICKFRAME
  ADJUSTW #1,bmen&,hmen&   :  @men2  :  MENU mend1$()  :  hdmen& = MENU(15)
  ```
  followed by the ~180 lines of enable/gray/check logic described in 1.1.
* `mainst1:` (679) — light refresh: `ADJUSTW`, close windows 2–7, `TOPW #1`, `ON MENU`, `@mainkont` (paints the main info panel), optional `@acmcl` (clock overlay).
* `mainst2:` (692) — enter the message pump.

The pump (703–855) is a `DO … LOOP` with:
1. `datumakt$ = DATE$ : uhrzeit$ = TIME$` (change log line 164: `// Uhrzeit in Hauptschleife abgefragt`).
2. Clock ticking: `IF uhr& = 1 && TIMER - tu% > 1000 : @acmcl` (1 s) and, if a live clock data record exists (`zeuhr > 0`), a 5 s / 15 s block that swaps `ze = zeuhr`, recomputes `jd = jduhr + (TIMER-timh%)/86400000`, calls `@a311/@dat/@jseckp/@a316061`, and every 30 s refreshes the slot displays (`@dats_einz_l`).
3. `ON MENU` (GFA event dispatch, not `GETEVENT`/`PEEKEVENT` — explicit comment at line 733: `ON MENU  //Nicht GETEVENT od  PEEKEVENT`).
4. `IF MENU(1) OR MENU(4) OR MENU(12)` → F-key tests (see 1.2), then `SWITCH MENU(1)`:
   * `1` → `@mehrf_aus` (F9 double-print)
   * `4` → quit confirmation (`MESSAGE … MB_YESNO | MB_ICONHAND` → `@ende : CLEAR : END`)
   * `5` → minimise; `6` → reset window geometry → `mainst`
   * `17` → window moved (`xmen&/ymen&` from `MENU(7)/MENU(8)`) → `mainst1`
   * `18` → window resized (`bmen&/hmen&`) → `mainst`
   * `21` → maximise/restore toggle → `mainst`
   * `20` → a menu item was picked: `MENU KILL : @closew(1) : @men3 : @closew(11) : @closew(12)`; if the handler set `muuu& = 99` (DESKTOP) ask to quit.
   * `30` → `mainst1`; default → `@mtst` (drain mouse/keys).

`@mainkont` (7724) paints the main-menu information panel in window #1: starfield banner `@himmel(632,67,150)` + `@horcom_titel(10)`, then a `PBOX 0,67,632,360` in `RGB($C0,$DC,$C0)` with the loaded record's Name / Ort / Datum / UT / BEM, house system, parameters (`bres&`/`brep&` "SIGN./PROM.: Mit/Ohne Breite", Kardinalpunkte, symbol size …). `mainkont_dat_zeit` (8112) is the routine the clock re-calls to repaint just date/time.

### 2.3 `men3()` — the universal output-screen driver (line 6240)

Every menu item goes through this one procedure. Structure:

1. Preamble (6245–6304): `@clr_main` (9381 — a 70-line global reset: clears ~300 scalars, ERASEs ~40 arrays, closes all files, `@druck_loe`), re-DIMs the counter arrays, `muuu& = MENU(0)`, `@cls_std`, `@kon_dhol`.
2. Output window: unless `muuu& = 12 or 13`,
   ```
   win& = 2 : OPENW #win&,0,0,gdx&,gdy&,0
   SETWINDOWSTYLE win&, WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MAXIMIZEBOX|WS_MINIMIZEBOX
   gdxh& = gdx& - 2*(WIND_GET(4)-WIND_GET(0))   : ADJUSTW #win&,gdxh&,gdyh&
   TITLEW #win&,mend1$(muuu&)                   : TOPW #win&
   ```
   `GRAPHMODE R2_COPYPEN,OPAQUE`, `@PaletteSave(win&,PalEntr$)`.
3. Guards (6318–6360): if a Composit/Combin/Doppel entry is half-built, abort with `@fanz("EINGABE für COMPOSIT BETÄTIGEN !")` etc. For Solar slots with `zeins(ze) > 0` → `fanz$ = @syt$(78,85,82,32,68,69,77,79)` ("NUR DEMO", obfuscated) and abort.
4. Dispatch (6365–6376) — the seven `ON … GOSUB` lines listed in the tree above. `@rahmen` afterwards.
5. Screenshot caching (6379–6385): for graphic outputs (32,37–39,42–44,53–59,64–75,79,80,84,89,90) do `@scget(win&)`; `s& = scrn&(win&)`.
6. Wait loop `men31:` (6418–6495): `@titlew` sets the caption hint, then `DO … UNTIL (e! OR asc& = 27 OR ex| = 2) AND MOUSEK = 0`, containing `GETEVENT`, `WM_PAINT` → `STRETCH 0,0,s&,_X,_Y,SRCCOPY` (repaint from the cached bitmap), `@dragline_y(60,420)` (draggable cursor line on linear graphics), `@einzel_plan_wahl` (RMB), `@zeige_horoskop` (F2), `@wart_gem(...)` (the shared key handler), `@mehrf_1` (F9), and the `muuu& = 53` F6 helio/geo re-entry.
7. Exit `men3e:` (6496–6533): `@hardc_kompl` (offers hardcopy), restore palette, `@closew(win&)`, `@closew(11)` (Halbsummen counter popup), `@closew(12)` (Aspekt counter popup), `CLOSEDIALOG #20`.

Window numbering convention — #1 main menu, #2 standard output, #3–#7 auxiliary (closed at `mainst1`), #7 the 2-up print preview (`mehrf_a`, 7534), #10 symbol dump (`symb_anz`), #11 half-sum counter, #12 aspect counter, #13 the F2 "show chart" overlay (`zeige_horoskop`, 6577), #20 big-file dialog.

Dialog stack — `diha&` starts at 12 (line 395) and is a stack pointer: `dial_ini` (50286) does `INC diha&`, `dial_close` (50315) does `DEC diha&`. All per-dialog state is array-indexed by `diha&` (`oldw&()`, `oldfont&()`, `fontdial&()`, `defaui&()`, `listbi1&()`, `editi1&()`, `mousestop!()`). `mainst:` unwinds with `WHILE diha& > 16 : @dial_close : WEND`. `dial_ini` also builds a MODERN font at 8×12 (or 10×14 for `gdx ≥ 1.3`) scaled by `gdx/gdy`.

### 2.4 Main states / screens

| State | Entry | Notes |
|---|---|---|
| Main menu | `mainst:` / `mainkont` (7724) | window #1, menu bar + info panel + starfield |
| Data entry | `eingabe(nein!,sp!,nna&,nne&)` (24032, ~770 lines) + `eing_box` (24800) + `zeitzon()` (24932) + `a2ort` (21495) | the biggest single dialog; name/date/time/place/timezone |
| File browser | `ausw_datei(x&,y&,b&,h&,tit$,VAR na&,ne&)` (22457) | a `DIALOG #diha&` with a listbox id 100 plus buttons `WAHL - ENDE`(101), `&EXIT`(102), `&DRUCKEN`(103), `Wortanfang suchen`(104), `Allgemein suchen`(105), `NUR HOROSKOP ZEIGEN`(107). Header line id 106. Wrapped in `TRY`/`CATCH` (change log line 25: `// In ausw_datei TRY,CATCH eingeführt`). |
| Chart display | `a11` (45227) → `a11_1` (45423) → `horg` (46005) | see section 3 |
| List / table output | `list_ausg` (14299), `ko_ta` (17038), `a18tab` (40703) | scrollable via `warts()` (14767) / `zeilklick()` (14889) |
| Statistics | `stat` (11830), `stat1` (11982 "AUSWERTEFÄHIGE DATEI ERSTELLEN"), `stat2` (12267 "DATEI ZUR AUSWERTUNG LADEN"), `stat3` (12644 "LÖSCHEN"), `stat_ausw` (13501) | own folder `\STATIST7` |
| Clock | `uhr` (8172) + `acmcl` (8128) | see 3.9 |
| Print preview (2-up) | `mehrf_a` (7527) in window #7 | see section 4 |

---

## 3. CHART RENDERING PIPELINE

### 3.1 Coordinate system — everything a C++ renderer needs

Virtual canvas: 640 × 480 logical units; the usable client area is 640 × 459 (`gdyh& = gdy&*459/480`; the bottom ~21 units carry the `drad2` credit line).

Device mapping (`xk()`/`yk()`, 28097):
```
device_x = round(vx * gdx)      where gdx = screen_width  / 640
device_y = round(vy * gdy)      where gdy = screen_height / 480
```
(identity when `gdx& = 640`). All primitives route through `@line`, `@boxn`, `@pboxn`, `@kreis`, `@text`, `@putbm`, each of which applies `xk/yk` when `moda& = 1` and passes raw coordinates when `moda& = 2` or `3` (printer).

`moda&` is the output-target mode and it is checked everywhere:
* `1` = screen (bitmap symbols, `xk/yk` scaling, MODERN font)
* `2` = printer DIN A5 landscape (vector symbols via `so()/mo()/me()/…`, `DEVICE` font, raw coords)
* `3` = printer DIN A4 portrait, 640 × 980 page

Chart polar mapping (`horg10`, 46344; `plein1`, 47325; `plmk1`, 47468; `aspz0`, 20944 — all identical):
```
w  = nb(lambda + PI - fza)          // lambda = ecliptic longitude in radians
x  = amh& + km * r * cos(-w)
y  = bmh& + km * r * sin(-w)
```
`fza` (the rotation origin) is set by `horbeg` (45469) from `begz&`:
`1`→AC (`fz(od,ze,1)`), `2`→MC (`fz(od,ze,10)`), `3`→0° Aries, `4`→π, `5`→user degree `pu*VAL(begz$)`; heliocentric forces `fza = 0`.
With the default (AC at origin) the AC lands at `w = π` ⇒ screen-left, i.e. counter-clockwise zodiac, AC on the left horizon — the standard layout.

For the zodiac ring and sign dividers a half-sign offset is used (`zein1`, 47741; `horg1`, 46066):
```
w = nb(j*PI/6 + 11*PI/12 - fza)     // glyph centred in the sign
x = amh& + round(km*rz) * cos(-w + pv2)     // pv2 = 2*PI, i.e. a no-op
```

Centre and scale:

| Context | `amh&` | `bmh&` | `km` | Line |
|---|---|---|---|---|
| default / `a11` screen | 430 | 224 | 0.95 | 294–296, 449 |
| `uhr` | 430 | 224 | 0.95 | 8222 |
| `a20_horg` (transits) | 430 | 224 | 0.85 | 37937 |
| `a12` (Doppelkreis) | 430 | 224 | 0.8 | 29334 |
| `multiple` / `harm` | 430 | 224 | 0.82 | 43357, 44986 |
| `prima` | 430 | 224 | 0.89 | 16135 |
| `a11` DIN A4 print | 340 | 340 | `km*1.48` | 45341–45343 |

### 3.2 Ring radii (virtual units, before `km`)

`horg` (46005) is the master:
```
CASE moda& = 1:
  @horg1(90,152,182)      // twice, to darken the AA'd strokes
  @horg1(90,152,182)
  rz& = 165 : fill! = -1 : @zein(rz&,fill!)    // fill sign sectors + glyphs
  @horg11                                      // houses
  fill! = 0 : @zein(rz&,fill!)                 // glyphs again on top
  @plein1(128)                                 // planets
@bes1  : @bes10                                // text block / tables
```
`horg1(r1,r2,r3)` (46032) draws concentric circles at `km*r2` (152), `km*r3` (182) and `km*r1` (90, skipped when `nasp& = 1 AND hrg!`); printers/helio additionally get `km*1` (centre dot) and — heliocentric — `km*7` for the Sun. It then draws the 12 `//Zeichentrenn-Linien` (sign dividers) from radius `r2-1 = 151` to `r3+1 = 183`.

| Element | Radius | Routine |
|---|---|---|
| aspect-line endpoints | 90 | `aspz0` 20944 |
| inner aspect circle | 90 | `horg1` |
| planet marker tick (inner) | 90 ± (−3 … +5) | `plmk1` 47468 |
| planet glyph ring | 128 + `dc&(z&)` (de-clump offset) | `plein1(128)` |
| house-cusp inner end (normal) | `r3 = 90` | `horg11` 46131 |
| planet marker tick (outer) | 150 ± (−3 … +5) | `plein11` 47445 |
| zodiac glyph ring | 165 (A4: 167) | `zein` 47890 / `a11` 45358 |
| sign-band inner circle | 152 | `horg1` |
| house cusps end / `r4` | 152 | `horg11` |
| sign-band outer circle | 182 | `horg1` |
| axis (AC/MC) line end `r7` | 188 | `horg11` 46133 |
| axis label position `r8` | 200 | `horg11` 46134 |
| `r9` (double-chart aux) | 190 | `horg11` 46135 |
| double chart, inner+outer | `r3=182, r7=180, r4=233, r8=252, r9=152`, extra circle at 230 | `horg11` 46117–46122 |
| double chart, inner only | `r7=180, r4=90, r3=152, r8=200` | 46124–46129 |
| double chart, `auu! = 0` | `r8 = 76` | 46138 |
| outer planet ring (double) | 204 | `a12` 29509; `plmk` r=180 |
| transit/`drgrph!` marker | 182 | `plein11` 47438 |

### 3.3 Zodiac ring — `zein` (47890) / `zein1` (47741) / `fill_color` (47783)

Two passes over `j& = 1..12`:
* Pass 1 (fill): compute a seed point at `rz&` in the middle of the sign, `@fill_color(j&,…)` picks the element colour and the GFA `DEFFILL` hatch index, then `FILL xk(x),yk(y),0` — a flood fill bounded by the two ring circles and the divider lines. With `farbp!` set it instead creates a `CreateSolidBrush(vg%)` and selects it around the FILL.
* Pass 2 (glyphs): `@zeichp_dspl(j&, x - vd&, y - vd&)` where `vd& = @plsyver` (5 for small symbols, 7 otherwise) centres the bitmap.

Element colours (`fill_color` 47783 for the ring, `zeich_col`/`elem_col` 2753/2737 for glyphs). Default (`eigfarb! = 0`), normal 12-sign mode:

| Signs | Element | RGB | `DEFFILL` index |
|---|---|---|---|
| 1, 5, 9 (♈♌♐) | Feuer | `RGB(255,0,0)` | 5 |
| 2, 6, 10 (♉♍♑) | Erde | `RGB(128,128,0)` | 1 |
| 3, 7, 11 (♊♎♒) | Luft | `RGB(0,128,128)` | 7 |
| 4, 8, 12 (♋♏♓) | Wasser | `RGB(0,255,255)` | 3 |

In 90°-mode (`dop = 4`) only three groups exist: {1,4,7,10}→red/5, {2,5,8,11}→olive/1, {3,6,9,12}→teal/7. Background `hg%` is always `RGB(255,255,255)`. If the user chose own colours (`eigfarb!`), `cols%(1..4)` from `hor_farb` (28140 — "FARBEN für HOROSKOP - RING FESTLEGEN ! ( ANKLICKEN )") override.

`nursymb&` controls whether glyphs are drawn plain (`0`, `2` → `putbm …,3` = `SRCCOPY`) or tinted (`1` → `bmp_color_ze`).

### 3.4 Houses — `horg11` (46094) `//  Häuser`

Skipped entirely when `hrg!` (heliocentric) or `haw& >= 9`. For `w& = 1..12`:
* `w = nb(fz(od,ze,w&) + PI - fza)`; `horg10` derives the 9 endpoint pairs.
* Cusps 1, 10 (and 4, 7 when `dop < 4`) use `DEFLINE 0,2` (2 px) and run `x3,y3 → x7,y7`; the other eight use `DEFLINE 0,1` and run `x3,y3 → x4,y4`.
* Axis labels via `@textc(x8-5, y8+5, tg&, h$/g$/i$/k$)` where `tg& = 11` on screen, `13` on A4, and `aeqh()` (45968) returns the four axis captions. `habes` (46269) then prints the degree-within-sign as a 2-digit number 8 units below (`g = up*f - 30*INT(up*f/30)`).
* For `haw& = 6` (Äquales) / `haw& = 7` (Vehlow) a second AC/MC set is drawn in `RGB(0,0,255)` (`horg110`, `horg110_bes`) and house numbers are printed blue (`dopp_haus_i`, 46074).
* Composit charts with `comp_mstz!`/`comp_hand!` get an extra AC line at `plz(0,1,13)` (46232–46244).

### 3.5 Planets — `plein1(r0&)` (47252) and the symbol system

`Symbhol` (1189) loads six size variants of every symbol from `hrc$ + "\symbbmp\*.bmp"` via `bmp_load()` (1882):

```
// .._k   10*10 Pixel
// .._l   13*13
// ohne   16*16
// .._m   20*20
// .._g   24*24
// .._r   32*32
```

Base names `ps$(0..40)`: `te so mo me ve ma ju sa ur ne pl dr ds ac mc ar cn li cp ag ch tp gl ce pa jn vs cu ha ze kr ap ad vu po qu hl ph da ns xe` (0 = Fixpunkt "te", 11/12 = Drachenkopf/-schwanz, 19+ = Chiron, Transpluto, Glückspunkt, Ceres, Pallas, Juno, Vesta, Cupido…, Quaoar, Halley, Pholus, Damokles, Nessus, Xena). Zodiac signs `zi$(1..12)`, aspect glyphs `aspz$(1..19)` (only 1,2,3,4,5,6,8,12,17,18,19 exist). Handles land in `pls1&()/plsk&()/plsl&()/plsm&()/plsg&()/plsr&()`, `zes*&()`, `asps*&()`. `symb_anz` (1810) is a hidden debug dump of the whole set. `symb_loe` (1903) frees them at exit.

`plsyls1` (2052) selects the active variant into `pls&()`/`zes&()`/`asps&()` from screen width and `klsy!`/`lin!`:

| `gdx&` | `klsy! = 0` | `klsy! = -1` (small) |
|---|---|---|
| 600–790 | 16×16 (`pls1&`) | 10×10 (`plsk&`) |
| 796–1010 | 20×20 (`plsm&`) | 13×13 (`plsl&`), 10×10 if `lin!` |
| 1020–1260 | 24×24 (`plsg&`) | 16×16 (`pls1&`), 10×10 if `lin!` |

In heliocentric mode index 2 (Moon) is swapped for index 22: `pls&(2) = plsg&(22) // Glückspunkt=Erde`.

`plein1` then, for `z& = an&..bb4&`:
* `w2(z&) = nb(wl(z&) + PI - fza)`; position as in 3.1 with `r = r0& + dc&(z&)`.
* `dc&()` comes from `plentz` (47592) / `plentz1` / `plentz2` — the de-clumping pass that pushes overlapping planets onto staggered radii; `plsort`/`sort_pl` order them first.
* Dispatch: rulers of the chart (`geb_herr` → `k1&,k3&`), the lunar nodes when `moknw!`, and the apogee when `apogw!` go through `plinkl` (47400) which draws them inverted (`putbm …,12` = `NOTSRCCOPY`) on screen and green/red on the printer; everything else through `planp_dspl` (2504).
* `plein11` (47420) draws the radial tick marks; `planziff1` (47474) prints the degree number under the glyph (offset `dz&` = 22/24 normally, 15/18 for small symbols on 800 px).
* Line 47314: `IF r0& > 216 && an& = 0 : an& = 1 // Fixpunkt nicht laufend`.

`planp_dspl` (2504) is where colouring happens. It recomputes the planet's radius from the pixel position:
```
rm& = SQR((xm²+ym² - 2·xm·amh& - 2·ym·bmh& + amh&² + bmh&²)/(km·km))
```
and then:
```
IF (plw&(pl&) && plan_col!)
   OR (hard& = 1 && ((zeitwi! && drgrph!) OR dppel!) && rm& > 170 && rm& < 250)
   OR rot!                      -> vg% = RGB(255,0,0)  : @bmp_color_pl(...)
ELSE IF (hard& = 3 && (...) && rm& > 170 && rm& < 250) OR blau!
                                -> vg% = RGB(0,0,255)  : @bmp_color_pl(...)
ELSE                            -> @putbm(xm&,ym&,pli&,ppa&)
```
So the "red outer planets in a double chart" rule is purely radius-based: 170 < r < 250. `hard&` is the user choice from `avh` (28038): "Bei DOPPELKREIS für ÄUßERE SYMBOLE / Und bei RÜCKLÄUFIGKEITS-ANZEIGE / Die FARBE FESTLEGEN !" → `1 = ROT`, `2 = SCHWARZ`, `3 = BLAU`. Change-log confirmation: `// Bei Doppel-Horoskop Planeten außen wahlweise ROT färben   28.09.04` (line 153) and `// Rotfärbung bei DOPPEL-HOR. verbessert   22.11.04` (line 143).

`bmp_color_pl` (2462) `// Symbole färben` — the tinting trick, reproducible in C++ as a masked blit:
```
@putbm(xb&,yb&,hBitmap&,0)        // Feld freimachen      (WHITENESS)
RGBCOLOR vg%,hg%                  // rot färben
FOR i& = 1 TO height&-1 : LINE ... // fill the glyph box with the tint colour
GET ... sc&                        // grab the coloured patch
@putbm(xb&,yb&,hBitmap&,3)         // Bitmap normal zeichnen (SRCCOPY, black-on-white)
PUT ..., sc&, SRCPAINT             // OR the tint through the white areas
                                   // (SRCINVERT instead, for rulers / nodes / apogee)
```
`putbm` (8866) maps `ppa&`: `0 → WHITENESS`, `3 → SRCCOPY`, `10/12 → NOTSRCCOPY`.

Planet index 0 (Fixpunkt) is never a bitmap — it is the letter `"F"` in `RGB(255,0,0)`, size 16 (or 10 when `klsy!`).

### 3.6 Aspect lines — `aspz1` (21052)

Called from `asp1` (20531). Endpoints always on the r = 90 circle (`aspz0`). The whole block is skipped if `asp1!` (table-only mode), `nasp& <= 1` or `horm& <> 1`.

| `n&` (aspect) | Colour | `DEFLINE` style `ls&` | `ml&` legend index |
|---|---|---|---|
| 2 Opposition | `RGB(255,0,0)` | 2 | 2 |
| 3 Trigon | `RGB(0,200,0)` | 3 (2 if custom) | 3 |
| 4 Quadrat | `RGB(255,0,0)` | 4 (0) | 4 |
| 5 Quintil | `RGB(0,0,200)` | 13 / 5 | 13 / 5 |
| 6 Sextil | `RGB(0,200,0)` | 6 (2) | 6 |
| 7 Septil | `RGB(0,0,200)` | 14 / 7 (3) | 14 / 7 |
| 8 Halbquadrat | `RGB(255,0,0)` | 15 / 8 | 15 / 8 |
| 9 Novil | `RGB(255,0,0)` | 16 / 9 | 16 / 9 |
| 10 Dezil | `RGB(0,0,200)` | 17 / 10 | 17 / 10 |
| 11 | `RGB(0,0,0)` | 18 / 11 | 18 / 11 |
| 12 Quinkunx | `RGB(0,200,0)` | 19 / 12 | 19 / 12 |

`aspz1_2` gates each line on the per-aspect visibility flags `aspli%()` / `aspli|()`; `aspz1_0` substitutes the user's own line style `aspst|(ml&)` when `selbst_cl_st!` ("Eigenvorgabe benutzen", `avh` case 20). Line primitives: `aspz1_1_1` uses native `DEFLINE ls&,1` on screen; on the printer it falls back to `p_line` (20950, `//gestrichelt`, 4-px dash by even/odd segment index) or `sp_line` (20976, `//strichpunk`, period 10 with a dot at 0.4–0.6 phase). `pline` (20924, `//punktiert`) plots every 4th pixel.

Mondknotenlinie — `asp1`, line 20776:
```
IF horm& = 1 && plw&(11) && plw&(12) && hrg! = 0 && haw& <> 10// Mondknotenlinie
  wa1 = FN nb(el(11) - fza) : wa2 = FN nb(el(12) - fza)
  @aspz0(wa1,wa2,x1&,y1&,x2&,y2&)
  RGBCOLOR RGB(0,0,255),RGB(255,255,255)
  CASE moda& = 1: DEFLINE 1,1 : @line(...) : DEFLINE 0,1
  DEFAULT:        @p_line(...)
```
Change log line 135: `// Mondknotenlinie zeichnen,blau gestrichelt   09.02.05`. The Apogee (Lilith) axis directly above it (20767–20775) uses the same r = 90 chord with `DEFLINE 1,1` in the normal colour.

### 3.7 Double chart — `a12` (29299)

`km = 0.8`, `dppel! = -1`. Two modal passes controlled by `inn!` / `auu!`:
1. `@a12i` picks the inner chart, `@a12a` the outer.
2. With both set, header text is laid out: `"INNEN-Kreis"` at (4,14), inner name/place/date at x = 224/510/376 y = 14–26; `"AUSSEN-Kreis"` at (114,14), outer block at y = 444–458.
3. Inner chart: `auu! = 0   //nur zur Steuerung der Graphik` (29476) → `horg1(90,152,182)` ×2, `zein(165)`, `horg11`, `plein1(128)`, `@asp1`.
4. Outer chart: `auu! = -1`, `@horg11` again (now with the `r3=182 / r4=233 / r8=252 / r9=152` radius set and the extra circle at 230), `plein1(204)`.
5. `a12asp` (29704) builds the paired aspect table; it is here that `rot!`/`blau!` are set per `hard&` (29905–29917 and 29956–29969) so the outer chart's symbols in the table match the ring colouring.
6. `dop = 4` switches the whole thing to a 90° circle (`ze& = 2` in the `"MODUS ?"` alert, 29344–29353: `" NORMAL-KREIS "` / `" 90°-KREIS"`).
7. The result is registered as slot `od = 0, ze = 3` with `sol$ = "DOPPELKREIS"` and cached to `hrc$ + "\INTERN\DOPPEL03.BMP"` via `@scrg(4,im$)`, plus `GET 0,0,_X,_Y,scrdopp&` (29571) for the "letztes Bild" feature.

Change log line 144: `// Bei Doppelhoroskop INNEN Aspektlinien gezeichnet   11.11.04`.

### 3.8 Linear graphic — `a18_lin` (37717)

Rectangular time/angle plot, not a wheel. Frame `boxn(18,60,622,420)` inside `boxn(1,1,639,458)`, horizontal scale `skalh(600,1,420,80,568,mo1)` (37367), vertical `skalv(360,1,18,420,60,-1)` (37302). Y mapping:
```
yy&(i&) = @lin_inv(420 - up * FN nb((360/w4d) * plz(od,ze,i&)))
lin_inv(y) = 480 - y   when lin_inv! is set (37665)
```
Sign boundaries are drawn with `DEFLINE PS_DASHDOT,1` in `RGB(0,0,255)` via `sp_line(54,y,622,y)`.

Variable line thickness — `a18_line_col(i&)` (37677), matching change-log line 23 `// Liniendicke bei Lineargraphik variabel   10.06.08`:
```
moda& = 1 : bildsdick& = 1 -> DEFLINE 0,1 ; else DEFLINE 0,2
moda& = 2/3 : duennlin! -> DEFLINE 0,0.5 ; bildsdick& 2 -> 0,2 ; 3 -> 0,3 ; then DEFLINE 0,3
```
Track colours: planets 1–3 `RGB(0,160,0)`, 5 & 7–10 `RGB(255,0,0)`, 4, 6, 11, 12 `RGB(0,0,255)`, rest black. `a18_entz` (37838) is the label de-clumper (shifts x by 9 or 14 when two curves come within 10 y-units). Symbols are forced small (`klsy! = TRUE : klsyt! = TRUE`) for the whole routine. With `w4d = 360 && mund!` the 12 sign glyphs are stacked down both margins at `x = 45` / `x = 605`, `y = 434 - 30*u&`.

### 3.9 The clock chart — `uhr` (8172) and `acmcl` (8128)

* `uhr` first shows an `alertbox`: "Wenn UHRZEIT GROB FALSCH ist / HORCOM NEU STARTEN !" with `"SYSTEM-UHRZEIT hh:mm  RICHTIG ? = OK "`.
* It then creates a synthetic data record (`od = 0, ze = 0, sol$ = "RADIX", na$ = "UHR"`), runs the normal `eingabe`/`zeitzon` flow for the location, and stores `jduhr = jd`, `timh% = TIMER`.
* Optionally the clock is promoted to a real slot: `" LAUFENDE UHR als DATENSATZ ÜBERNEHMEN ?" / "Wird NICHT empfohlen !"`, with `bem$ = "WIRD ALLE 15 SEK AKTUALISIERT !"` and `zeuhr = ze`.
* Redraw loop `uhr1:` (8310–8335):
  ```
  @dat : @date_form : @jseckp : @a316061 : @kon_dhol : @plsyls1
  @a11_1                                   // full chart rebuild
  @textc(amh&-17, bmh&+6, 13, " UHR ")     // label at the wheel centre
  t2% = TIMER
  DO : ON MENU : m& = MENU(12)
       IF anz! = 0
         @fanz("Solange UHR SICHTBAR wird HOROSKOP ALLE 15 SEK NACHGEZEICHNET !")
       ENDIF
  UNTIL TIMER - t2% > 15000 OR m& = 27 OR zeuhr > 0
  jd = jduhr + (TIMER - timh%)/86400000 : GOTO uhr1
  ```
  → full chart redraw every 15 s, exit on ESC. Change log line 36: `// Bei Uhr alle 15 sek neu   03.01.05`; line 44: `// UHR dauernd beobachtbar machen   04.09.05`.
* `acmcl` (8128) is the cheap update used from the main menu (1 s cadence, `TIMER - timh% > 900` guard): recomputes `jd`, then repaints four inverse-video bars at y = 357 via `textrl`: `"UHR: hh h mm m ss s UT"` at x = 4, `"AC: …"` at 164, `"MC: …"` at 324, `"STZ: …"` at 484. The main loop drives the 5 s / 15 s / 30 s cadences for a promoted clock record (lines 706–730).

### 3.10 Text, fonts and colours

* `text(xte&,yte&,te_gr&,tex$)` (8597) — the workhorse. Screen: `FONT "MODERN" : FONT FAMILY FF_MODERN,QUALITY 0 : FONT WIDTH te_w&*gdxt, HEIGHT te_gr&*gdyt`. Printer: `FONT "DEVICE" / DEVICE_DEFAULT_FONT`, unscaled. Always `ITALIC 0, WEIGHT FW_BOLD, CHARSET ANSI_CHARSET, PITCH FIXED_PITCH, OUTPRECISION OUT_STRING_PRECIS`, `SetTextAlign(TA_BOTTOM)`, `SetBkMode(TRANSPARENT)`. The string is drawn twice (spaces first, then the text) to clear the cell.
* `textg(te_gr&)` (8582) maps nominal height → cell width: `7..14 → 7`, `15,16 → 8`, `17..20 → 10`, `19..30 → 16`. (Note the overlapping 19/20 ranges — a latent bug in the original; the first matching `CASE` wins, so 19–20 get width 10.) Change log line 63: `// Zeichenbreite textg() bis 14 auf 7 reduziert`.
* Variants: `textc` (8701, opaque/contrast), `text_light` (8636, thin — used with `duennlin!`), `textsy` (8677, symbols only for printer), `texts` (8744, rotated/vertical, used by the `drad2` credit), `textrl`/`textzentrl` (8506/8545, boxed inverse-video status fields), `tinv`/`tinvg` (8768/8782).
* `deftextcol(te_col&)` (8471): `0` = inverse (white on black), `1` = normal (black on white), `2` = `RGB(0,0,128)` on `RGB(255,255,0)`, `3` = `RGB(255,0,0)` on `RGB(0,255,255)`. `col_norm` (8468) = black on white.
* Background: `cls_std` (3339) branches on `coln%` (`GETDEVCAPS(24)`): 2 colours → `col_norm`+`cls_screen`; 16 → `CLS(col_backg%)` (default 7); else `CLS(GetNearestPaletteIndex(_PAL(WIN()), col_backg%))` with `col_backg%` defaulting to `RGB(192,192,192)` and user-settable via `backg_col` (7289) / menu 96.
* Palette is saved/restored around every output screen (`PaletteSave`/`PaletteRestore`, 880/888).
* `drad2` (42640) stamps the credit line — `"HORCOM7P R.Rettig User:… <date> <time>"` — at y = 454 (screen, rotated via `texts`), y = 471 (A5) or y = 974/994 (A4), plus a frame box.

---

## 4. PRINTING AND EXPORT

### 4.1 The three output modes

`druck_graph_ein` (45143) is the mode chooser, an `alertbox`:
```
a$ = "AUSGABE auf BILDSCHIRM oder als DRUCKER-GRAPHIK ?"
b$ = "BILDSCHIRM ( evtl. HARDCOPY )?"          -> moda& = 1
h$ = "DRUCKER-GRAPHIK  DIN A5 ?"               -> moda& = 2
g$ = "DRUCKER-GRAPHIK  DIN A4 ?"               -> moda& = 3
e$ = "( DRUCKER-OPTION UMSCHALTEN mit F8 aus MENÜ oder AUSGABEN ! )"
```
Only offered for `muuu& = 53,64,69,70,74,75` in the 3-way form; everything else gets screen/A5 only. Gated by `prenbl&`.

`druck_einr_anz` (45175) is the "Drucker-Option" dialog proper: shows `" DRUCKER BEREIT ? "`, composes the orientation hint (`" QUERFORMAT ( bzw. 'LANDSCAPE' ) "` / `" HOCHFORMAT ( bzw. 'PORTRÄT' ) "`, `" oder BENUTZERDEF. HALBSEITE "`), the format (DIN A4/DIN A5), then `DLG PRINT WIN(),0,dr&` → the Windows printer dialog; returns the printer DC. The copy count comes back in `_DX`, success in `_AX`.

`druck_loe` (45128) tears down: `SETDC currdc&`, `FREEDC druck&`, `MAPMODE MM_TEXT`, `VIEWPORT 0,0,1,1,1,1`, `moda& = 1`.

`druck_enbl_alt` (45109) is the F8 toggle of `prenbl&` (`"DRUCKER-OPTION EIN !"` / `"… AUS !"` + `@param_sp` to persist). `pr_enabel` (42607) is the menu-31 equivalent.

### 4.2 Vector printing of the chart — `a11` (45227)

* A5 (`moda& = 2`), lines 45267–45307:
  ```
  xdr& = 640 : ydr& = 459
  SETDC druck& : STARTDOC "DRUCK_A5"
  GRAPHMODE R2_COPYPEN,TRANSPARENT : MAPMODE MM_ANISOTROPIC
  VIEWPORT gdxp&/15, gdyp&/90, xdr&, ydr&, 0.8*xdr&*dpixp&/72, 0.8*ydr&*dpiyp&/72
  @horbeg : @a11_1 : @rahmen : @drad2
  NEW FRAME / ENDDOC  (loop for nr& copies)
  ```
* A4 (`moda& = 3`), lines 45308–45388: page 640 × 980, `amh& = bmh& = 340`, `MUL km,1.48`, `VIEWPORT gdxp&/14, gdyp&/80, 640, 980, 0.75*…`. The draw order is explicitly annotated: `@bes_big_plan //Reihenfolge wichtig !` then `bes_big_asp`, `bes_big_haus`, `bes_big_elem`, `bes_big_kafige`, `bes_big_halbs`, `bes1_big(...)`, `horg11`, `plein1(128)`, `zein(167)`, `horg1(90,152,182)` twice, page frame `boxn(1,1,639,979|999)`, `drad2`.
* `a11ex` / `a11ex1` (45395 / 45407) restore `km`, `amh&`, `bmh&`, `klsy!`, `dbr&`, `fixpunkt&` afterwards.

`a12` uses the same pattern with `STARTDOC "DRUCK_GRAPH_DOPP"` and `VIEWPORT gdxp&/20, gdyp&/90, 640, 459, 0.8*…` (29415–29421).

### 4.3 Bitmap hardcopy — `start_hardc` (6757) / `hardcopy` (6830)

* `start_hardc` builds a tiny modal `DIALOG #diha&` titled `" HARDCOPY ?"` at the bottom-right corner (`x& = xk(632-124)`, `y& = yk(458-52)`) with buttons `"JA"` (101) and `"NEIN"` (102), with the cursor clipped into it. Suppressed for `muuu& = 46..49,52,60,63,66,68,76,79,81,85..88,91..101` unless a double/composit/combin chart is on screen.
* `hardcopy` grabs the window (`GET 0,0,_X,_Y,sc&`), makes a MEMDC over the cached `scrn&(win&)`, calls `druck_einr_anz`, then:
  ```
  STARTDOC "DR_EINF"
  ' DIN A4  210*297 mm
  halbs& = 1 :  fx = 0.8*640*dpixp&/72 : fy = 0.8*459*dpiyp&/72
               StretchBlt(druck&, gdxp&/20, gdyp&/90, fx, fy, hm&,0,0,bsc&,hsc&,SRCCOPY)
  halbs& = 0 :  fx = 0.8*640*dpiyp&/72 : fy = 0.8*459*dpixp&/72     // axes swapped = landscape
               StretchBlt(druck&, gdyp&/90, gdxp&/90, 1.4*fx, 1.4*fy, hm&,0,0,bsc&,hsc&,SRCCOPY)
  ```
  then repeats for `nr&` copies, finishes with `_WIN$(...) = ... + "  |  WEITER mit LEERTASTE"` and `KEYGET`.
* `hardc_kompl` (6897) is the standard call site, invoked from `men3e:` (6510) and from `wart` (7170).

### 4.4 Two-up printing ("DOPPEL-AUSDRUCK", F9)

`mehrf_1` (7461) explains it verbatim:
> `"Mittels der FUNKTIONS-TASTE  F9  ( oder 'ALT + M' ) Können Sie"`
> `"2 Auswertungen auf einer DINA4-Seite ausdruckenm indem Sie diese mit F9 SPEICHERN,"`
> `"dann aus dem Haupt-Menü wieder F9 drücken und ausdrucken."`

`wmehr&` is the buffer slot counter (starts at 3, windows #3 and #4 hold the two captured bitmaps via `mehrf_1_mem`, 7512). `mehrf_aus_1` (7386) offers `"WEITER = DRUCKEN"` / abort / `"DOPPELBILD - SPEICHER LÖSCHEN"`. `mehrf_a` (7527) opens window #7 titled `"DOPPELBILD AUSDRUCKEN"` and tiles the two saved bitmaps at `bt& = gdxh&/2, hh& = gdyh&/2` (`mehrf_a_sc` 7654 for screen preview, `mehrf_a_pr` 7662 for the printer DC). The 4-up variant exists but is commented out — change log line 99: `// 4-fach-Ausdruck totgelegt   22.09.05`.

### 4.5 Image storage / export — `screen` (26033) and the tile format

Menu 32, "LETZTES BILD ZEIGEN / bzw.SPEICHERN".

```
IF doppelh! OR composit! OR combin!
  PUT 0,0,scrdopp&,SRCCOPY : @scget(win&) : FREEBMP scrdopp&
  CLR doppelh!,composit!,combin!,scrdopp&
ELSE
  IF scrn&(win&) > 0 : @scput(win&) ELSE @fanz("Noch kein Ausgabe - Bildschirm vorhanden !")
```
This `scrdopp&` special case is the fix noted in change log line 26: `// Fehler bei Bild-Speicherung von Doppelhoroskopen beseitigt   24.04.08`. `scrdopp&` is filled at the end of `a12` (29570–29571), `a13` and `a14`, and freed again in `men3` case 32,37–39,… (6383).

Then an `alertbox` offers `" SPEICHERN "` / `"LADEN"` / `"Bild LÖSCHEN"` / abort, all through `FILESELECT hrc$ + "\BILDER\*.BMP"`, with the hint `"Gelegentliches LÖSCHEN im Ordner '\BILDER' NICHT VERGESSEN !"`.

The on-disk format is not a real BMP despite the extension. `bildst` (26292) / `bildl` (26257) split the screen into an `aufz& × aufz&` tile grid (`aufl_ziff` 26216: 16 tiles for 640, 20 for 800, 32 for 1024–1380, 40 for ≥1400) and `BPUT`/`BGET` each tile's raw GFA `GET` string sequentially. `dimsc` (26234) `//BILDPUFFER ERZEUGEN` pre-measures the tile byte sizes into `sc%(i&,k&)`. A sidecar file `\BILDER\FORMAT.BLD` stores `coln%,coleb&,gdx&,gdy&`; `pruef_format` (26157) refuses to load images made at a different resolution (`"BILDER wurden mit ANDERER AUFLÖSUNG erzeugt !|ALTE BILDER LÖSCHEN ?"`).

Real image export is delegated to MS Paint: `notiz_in` (7210), bound to F7 / ALT+F, launches `MSPAINT.EXE` (from `wind$` or `wind$\system32`) with `"AUSGABE-BILD in MSPAINT übernehmen ?"` — the bitmap arrives via the clipboard, which `scget` (26186) always fills: `CLRCLIP : GET 0,0,gdxh&,gdyh&,screen& : CLIPFORMAT CF_BITMAP : CLIPCOPY screen&,0`. The menu-0 legend calls this `"F7 … = MSPAINT ÖFFNEN zum SPEICHERN von Ausgaben im GIF-FORMAT"`.

Per-output BMP caching: `scrg(is&,im$)` (26322) writes the tile file only if the computation took > 3 s (`t% = TIMER - tmc1% : IF t% > 3000 OR comb! OR comp! OR dppel!`) and disk space is OK (`discf`), into `hrc$ + "\INTERN\<TAG><od><ze>.BMP"` (e.g. `TRANST13.BMP`, `DOPPEL03.BMP`), flagging `a!(od,ze,is&) = -1`. `gesp_neu(im$)` (19594) asks "Gespeichertes (1) oder Neu (2)?" on the next invocation.

### 4.6 `bres&` / `brep&` — correction to the brief

These are not printer variables. Defaults at lines 320–321; set in `avd` (Vorgaben Direktionen) at 29120–29170 via the alertbox `"VORGABEN für PRIMÄR-DIREKTION:" / "SIGNIFIKAT. MIT/OHNE Breite ?"` and `"PROMISSOREN mit Breite ?"`; displayed on the main panel at 8000–8012 as `"SIGN.: Mit/Ohne Breite"` / `"PROM.: Mit/Ohne Breite"`; consumed in `aprim1` (41908/41911) as `br = ASIN(SIN(brep& * ebz(od,ze,u&)) * SIN(po - f))`. They select whether ecliptic latitude is included for significators / promissors in the Primärdirektion (E.C. Kühr).

---

## 5. TOP 10 UI/GRAPHICS ROUTINES FOR A FAITHFUL C++ PORT

| # | Routine (line) | Why it is critical |
|---|---|---|
| 1 | `plein1` (47252) + `planp_dspl` (2504) + `plein11` (47420) + `planziff1` (47474) | The planet layer: polar placement, de-clumping offsets `dc&()`, tick marks, degree numbers, and all symbol colouring rules (including the `170 < rm& < 250` outer-ring red/blue rule for double charts). Most behaviour and most edge cases live here. |
| 2 | `horg11` (46094) + `horg10` (46344) / `horg110` (46364) / `habes` (46269) | House cusps and axes. Owns the entire radius table (normal / double-inner / double-outer / 90°), line weights, axis captions and the degree-in-sign labels. Most special-casing of any chart routine. |
| 3 | `zein` (47890) + `zein1` (47741) + `fill_color` (47783) | Zodiac ring: half-sign angular offset, element colours + `DEFFILL` hatches, and the flood-fill approach that a C++ renderer must replace with explicit annular-sector paths. |
| 4 | `men3` (6240) | The single universal output-screen driver: window creation, `clr_main` reset, the `ON … GOSUB` dispatch table, the repaint/wait loop, hardcopy hook, teardown. Port this and every one of the 60+ features gets a consistent host. |
| 5 | `a11` (45227) + `a11_1` (45423) + `horg` (46005) + `horg1` (46032) | The chart entry point and the three-way screen/A5/A4 branch, including the A4 re-centring (`amh&=bmh&=340`, `km*1.48`) and the documented `//Reihenfolge wichtig !` draw order. |
| 6 | `asp1` (20531) + `aspz1` (21052) + `aspz0`/`aspz1_1`/`aspz1_2`/`aspz1_0` (20944–21051) | Aspect lines: r = 90 chord geometry, the 12-row colour/style/legend-index table, per-aspect visibility gating, custom-style override, and the blue dashed Mondknotenlinie at 20776. |
| 7 | `Symbhol` (1189) + `plsyls1` (2052) + `bmp_color_pl` (2462) / `bmp_color_ze` (2489) + `putbm` (8866) | The whole glyph asset pipeline: six size variants × 41 planets + 12 signs + 11 aspects from `\symbbmp`, the resolution/`klsy!`/`lin!` selection matrix, and the SRCPAINT/SRCINVERT tinting technique. |
| 8 | `text` (8597) + `textg` (8582) + `textc`/`text_light`/`texts`/`textrl` + `deftextcol` (8471) | MODERN.FON metrics, the `te_gr& → te_w&` width table, `gdxt/gdyt` high-DPI compensation, and the four text colour schemes. Get this wrong and every label in every layout shifts. |
| 9 | `get_aufl_bs` (3311) + `xk`/`yk`/`xl`/`yl` (28097–28124) + `line`/`boxn`/`pboxn`/`kreis` (42702–42771, 45994) | The 640×480 virtual coordinate system and its device mapping — the foundation every other coordinate in this report is expressed in, plus the `moda&`-dependent primitive dispatch. |
| 10 | `hardcopy` (6830) + `start_hardc` (6757) + `druck_einr_anz` (45175) / `druck_graph_ein` (45143) / `druck_loe` (45128) + `screen` (26033) / `bildst` (26292) | The complete output path: hardcopy dialog, printer DC lifecycle, `MM_ANISOTROPIC`+`VIEWPORT` page setup, A4/A5 orientation maths, and the (non-standard, tiled) image persistence format. |

Runners-up worth reading early: `mainkont` (7724) for the main-menu panel layout, `uhr` (8172)/`acmcl` (8128) for the timed redraw cadences, `a12` (29299) for double-chart composition, and `a18_lin` (37717) for the rectangular-graph family.
