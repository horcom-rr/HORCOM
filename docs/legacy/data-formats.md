# HORCOM7P Legacy Analysis, Part 3. Persistent File Formats

Derived from `reference/HORCOM7P.lst` and verified against real files under `legacy/HORCOM/`. All line numbers refer to the listing. German quotes are Robert Rettig's original comments.

## 0. Ground rules for reading GFA BASIC I/O

| GFA construct | Meaning for a C++ loader |
|---|---|
| `OPEN "R",#n,f$,L` + `FIELD #n,a AS x$,…` | Fixed-length record file, record size `L`. All fields are raw bytes/ASCII — `FIELD` only slices a byte buffer. `LOF(#n)/L` = record count. |
| `LSET x$ = v` / `RSET x$ = v` | left- / right-aligned, space padded (never NUL padded) |
| `OPEN "O"/"A"/"I"` + `WRITE #`/`INPUT #` | CSV text, CRLF, strings in `"…"`, booleans as `-1`/`0`, floats with `.` decimal point |
| `PRINT #` / `LINE INPUT #` | plain CRLF text lines |
| `STORE #n,arr$()` / `RECALL #n,arr$(),-1,x%` | one array element per CRLF line (quotes may or may not be present — strip if so) |
| `MKL$/CVL` | signed int32, little-endian |
| `CVI` | uint16/int16, little-endian |
| `CVD` | IEEE-754 double, little-endian (verified: `PLANETS.DAT[0] = 1.75347045673`) |
| `kk` (line 9312) | `kk = 1.0E-10` — a no-op epsilon added to coordinates; ignore it |

Path root resolution (lines 205–240):

```
IF EXIST("c:\horcom.pth")
  OPEN "I",#1,"c:\horcom.pth"
  IF LOF(#1) > 3
    INPUT #1,hrc$,aafpth$
```
`C:\HORCOM.PTH` = one CSV line, two quoted strings: install root (`hrc$`) and AAF root. If `aafpth$` does not end in `\`, `"\aafdaten"` is appended (line 216–220). Fallback (lines 236–240): `hrc$ = <drive>:\HORCOM` if `\INTERN\KENNTS7P.INT` exists there.

---

## 1. Chart storage — `<hrc$>\SPEZIAL\*.DAT`

Fixed 128-byte ASCII records, no header, no index. Verified: all 44 `.DAT` files in `SPEZIAL\` are exact multiples of 128.

Canonical `FIELD` (identical at lines 5668, 6021, 21622, 21778/21782, 23226/23230):

```
FIELD #20,2 AS ta$,2 AS mo$,5 AS ja$,2 AS ho$,5 AS mi$,8 AS ggl$,8 AS ggg$,25 AS naa$,20 AS goo$,51 AS bem$
```

| Off | Len | Field | Align | Type / meaning |
|----|----|----|----|----|
| 0 | 2 | `ta$` | RSET | day of month |
| 2 | 2 | `mo$` | RSET | month |
| 4 | 5 | `ja$` | RSET | year, astronomical count (negative = BC, e.g. `-500` = 501 BC) |
| 9 | 2 | `ho$` | RSET | hour, UT/GMT |
| 11 | 5 | `mi$` | RSET | minute, decimal (`STR$(mi,5,2)`), carries seconds as fraction |
| 16 | 8 | `ggl$` | RSET | geogr. longitude, decimal degrees, East positive / West negative |
| 24 | 8 | `ggg$` | RSET | geogr. latitude, decimal degrees, North positive / South negative |
| 32 | 25 | `naa$` | LSET | name (`Zuname` + `" "` + `Vorname`, truncated to 25 — see `make_namen`, line 3817) |
| 57 | 20 | `goo$` | LSET | place name + country abbreviation (`make_namen`) |
| 77 | 51 | `bem$` | LSET | remark; may begin with the 9-char calendar flag `"(JULIAN.)"` or `"(GREGOR.)"` |

Total = 128. Arrays after load (`a201`, line 22258): `taa&() moo&() jaa&() hoo() mii() gel() geg() naa$() goo$() beme$()`.

Time is always UT. Rettig states this in `KOMMEN7P\AAF_KOMM.TXT`:

> „HORCOM speichert z.B. die Zeit immer nur als GMT = UT , Bemerkungen werden nach 51 Zeichen abgeschnitten.Das stammt noch aus der Zeit als Speicher-Platz knapp war.Heute kann man dies vergessen."

In English: "HORCOM for example always stores the time as GMT = UT only, remarks are cut after 51 characters. That dates from the time when storage space was scarce. Today one can forget about it."

### Owning procedures

| Line | Proc | Role |
|---|---|---|
| 21612 | `oeffne20` | opens `datru$` `"R",#20,…,128`, sets `laf& = LOF(#20)/128` |
| 21627 / 21638 | `oeffne` / `oeffne_1` | dispatch chart vs. place |
| 22244 | `a200get` | sequential `GET` loop into arrays |
| 22258 | `a201(i&)` | record → arrays (`VAL()` of the numeric strings, `_ANSI$(_OEM$())` on text) |
| 22272 / 22287 | `a2011` / `a2011f` | arrays → record (`RSET`/`LSET`, blank template) |
| 23270 | `a22dat` | append a record (`PUT #datei&`), mirror into AAF, optionally into `.STA/.STH` |
| 23214 | `a22ueberschrb` | overwrite-by-name: copy to temp excluding matching `naa$`, rename back |
| 21765 | `a2f_tr_dat(t!)` | trim / delete records (via temp file) |
| 21833 | `minim_dat` | de-duplicate (name+date+time identical) |
| 21318 | `a2dat` | `FILESELECT hrc$+"\SPEZIAL\*.DAT"` |
| 21900/21908 | `a2f_tr$` / `a2f_tr2$` | re-pad a field LSET / RSET to length |

### Sorting / index — there is no index file

`sort$()` (declared `$ABIG` at line 262, dimensioned in `dimso`, line 15698) is an in-memory key array. `a211_nnam` (22409) builds keys from `naa$`/`goo$` and calls `QSORT sort$() WITH vg|(),laf&+1,cr%()`; `a211` (22352) builds an int32 key (`ce%()`) for birthday/date sorting. `cr%()` is the resulting permutation. Limit: `laf& < 15975`. No `.NDX` exists for charts — the only `.NDX` in the product is `PLANETS.NDX` (section 8).

### Scratch / safety files

| Path | Purpose | Proc |
|---|---|---|
| `SPEZIAL\QAYWSXED.DAT` | 128-B temp for trim/minimize/overwrite, then `NAME … AS datru$` | 21771, 21839, 23220 |
| `SPEZIAL\RRESERVE.DAT` | full byte copy of the file before a destructive op | `make_resdat` 23182 / `reg_resdat` 23197 |
| `INTERN\QAYWSX.AAF` | temp for AAF record deletion | 4952 |

> Bug worth carrying forward as a note: `reg_resdat` restores the place backup from `SPEZ_ORT\RRESERVE.DAT` (line 23207) while `make_resdat` wrote `SPEZ_ORT\RRESERVE.INT`. Place-file recovery is therefore broken in the original.

---

## 2. Places database

### 2.1 `<hrc$>\SPEZ_ORT\*.INT` and `SPEZ_ORT\BIGFILES\*.INT` — 36-byte records

```
FIELD #3,8 AS ggl$,8 AS ggg$,20 AS goo$
```
(lines 21607, 22016/22020, 23890/23903, 24951/24964, 9654/9669)

| Off | Len | Field | Align | Meaning |
|----|----|----|----|----|
| 0 | 8 | `ggl$` | RSET | longitude, decimal degrees, `E+ / W−` (3–4 decimals) |
| 8 | 8 | `ggg$` | RSET | latitude, decimal degrees, `N+ / S−` |
| 16 | 20 | `goo$` | LSET | place name, often `Name / CC` |

Verified by hexdump (`GERMANY.INT` rec 3 = `"  6.075555","50.77611","AACHEN"`; `bigfiles\AA_A_Z.INT` rec 1 = `"  -70.067"," 12.6167"`). All 383 bigfiles + 42 top-level `.INT` are exact multiples of 36. `RRESERVE.INT` is byte-identical to `GERMANY.INT` (it is a stale backup).

Naming convention for bigfiles (`a2ort`, lines 21514–21530): `CC_A_K.INT` where `CC` = 2-letter NIMA country code and `A_K` = alphabetic letter group. Detected by `MID$(dt$,3,1)="_" && MID$(dt$,5,1)="_"`. Comment at line 21377:

> `'Liste der geographischen country-codes nach NIMA`

`SPEZ_ORT\ortelist.txt` is a quoted-per-line inventory of the bigfile names, not referenced anywhere in HORCOM7P.lst (leftover). `SPEZ_ORT\AA_CODES.txt`, `AA_TIMEZ.txt`, `AA_ZONENZEITEN.htm` are human reference documents.

### 2.2 Zone-picker variant — `EUROPA.INT` / `WELT.INT` (also `EUROPE.INT`, `WORLD.INT`)

Same 36-byte layout, but the last 5 characters of `goo$` carry the time-zone difference, read as `@VAL(RIGHT$(lt$(select&),5))` in `zeitzon` (lines 25050, 25058). Verified: `"Agram / YU" … "  -1"`, `"Acapulco / Mex" … "   +6"`.

Sign convention: the stored value is the correction to add to local zone time to get UT, i.e. the negated UTC offset. The dialog label (line 24992) documents it:

> `CONTROL "ZEIT-ZONEN-DIFF. (h): Z.B. -1 h mit MEZ",107,…`

So MEZ (UTC+1) is stored as `-1`; Acapulco (UTC−6) as `+6`.

### 2.3 `SPEZ_ORT\WCAPITAL.INT` — nearest-capital lookup

Same 36-byte layout. `auswert_ortelist` (9647) makes two full linear passes computing planar distance with `ra = 6371.229315` km to find the minimum; used to offer "≈ N km east/north of <capital>".

### 2.4 `<hrc$>\ORT.EXT` — single "preferred place" record

One 36-byte record, same `FIELD` (`8 AS l$,8 AS b$,20 AS o$`), but the coordinates use a different encoding: written as `STR$(CINT(gl*1000000),8,0)` (`ortp`, 23898) → integer micro-degrees. Verified: 36 bytes, `"11324444","48174167","EICHENAU"`.

The reader accepts both encodings (lines 4802–4811, 24221–24230, 32140–32148):
```
IF INSTR(glm$,".") = 0
  gl = kk + VAL(glm$) / 1000000
ELSE
  gl = kk + VAL(glm$)
```

> Not a bug: `ortp` is guarded by `IF NOT EXIST(hrc$ + "\ORT.EXT") = TRUE`. GFA evaluates `EXIST(...)=TRUE` first, so the write only happens when the file is missing. That is the design. Every caller that replaces the preferred place first asks `BISHERIGEN VORZUGSORT LÖSCHEN|Und AKTUELLEN ORT dafür eintragen ?` and `KILL`s the old file on JA (`eingabe` CASE 111), or deletes it through `VORZUGSORT LÖSCHEN`. The real flaw is the field width. `STR$(CINT(gl*1000000),8,0)` needs nine bytes west of −9.999999°, so a place in the Americas cannot be stored. The port writes a decimal into the eight bytes then, which the reader's decimal branch takes (`write_preferred_place`).

### 2.5 Search mechanism

There is none on disk. `a200ort` → `a2fort` → `ausw_datei` (22457) loads the whole file into `gel()/geg()/goo$()`, sorts with `QSORT`, and presents a Win32 listbox with incremental typing + a `SUCHEN` substring scan (`INSTR(UPPER$(...),su$)`). Lookup is O(n) linear over the array; the "database" is really 400 flat files chosen by the user in a `FILESELECT`.

### 2.6 Place-file scratch

`SPEZ_ORT\YWSXED.INT` (36 B temp for `a2f_tr_ort`, line 22010) and `SPEZ_ORT\RRESERVE.INT` (backup, `make_resdat`).

---

## 3. Time-zone history

### 3.1 `<hrc$>\ZEITBEST\*.TXT` — prose, not parsed

29 per-country files (`ALBANIEN.TXT`, `GERMANY.TXT`, …). They are opened only by the generic text viewer:

```
FILESELECT hrc$ + "\ZEITBEST\*.TXT","",nam$
IF nam$ > ""
  @lese_text(nam$,7,0)
```
(`zeitzon`, lines 25078–25082; handle 7 is special-cased in `lese_text` line 10162: `EXIT IF LEFT$(a$) = "-" && dat& <> 7 // nicht bei Zeitbest` — i.e. leading-dash lines terminate every other help file but not these.)

Content is free-form German, e.g. from `ALBANIEN.TXT`:

```
      TIRANA  Br.:41.20 N   Lg.:19.49 °  =  1h 19m 16s
      bis 01.01.1914 bestand Ortszeit
      ab  01.01.1914 war zuerst Tirana-Zeit  GMT = Ereig.Zeit -1h 19m
      SOMMERZEITEN
      01.04.1940  2h  durchgehend
                      bis  04.10.1943  3h  GMT = Ereig.Zeit -2h
      06.05.1972  2h  bis  30.09.1972  3h  GMT = Ereig.Zeit -2h
```

There is no automatic DST/zone-history engine in HORCOM7P. The operator reads the table and types the offset into the `ZEIT-ZONEN-DIFF. (h)` edit field, or ticks `EINFACHE SOMMERZEIT = DSZ` / `DOPPELTE SOMMERZEIT = DDSZ` (`zeitzon`, 24932–25139) which sets the sentinel values `-2` / `-3` in `et$(1)` and `somz(ze) = -1 / -2`.

The recurring DST lines are regular enough to be machine-parsed for a modern port (`dd.mm.yyyy Hh bis dd.mm.yyyy Hh GMT = Ereig.Zeit -Nh`) — but mark this as a new capability, not a port of existing code.

### 3.2 `INTERN\zonnamen.int` — zone-name catalogue (177 entries)

`RECALL`-loaded (`zeitzon_nam_horc` 5195, `zeitzon_nam_aaf` 5265, `DIM zone$(180)`, loop `FOR j& = 0 TO 176`). Plain CRLF text, fixed columns, header in row 0:

| Cols (1-based) | Content |
|---|---|
| 1 … ~42 | `Name der Zeit-Zone` (space padded) |
| ~43 … | `Abkürzung` (GMT, UT, LMT, LTT, …) |
| last 10 chars of the line | UT difference, format `"-11 h 23 m"` / `" 00 h 00 m"` |

Parsed at lines 5240–5249:
```
zd$ = RIGHT$(z$,10)
h   = VAL(LEFT$(zd$,3))
min = VAL(MID$(zd$,7,2))
zzd = ABS(h) + min/60 ; negated if LEFT$(zd$)="-"
```
Dialog title credits the source: `"Zeit-Zonen (P.D. Via B.MAHL )"`. Entries 4 and 5 (LMT / LTT) are excluded from single-click selection (line 5228).

### 3.3 `INTERN\laender.int` — German country abbreviations (58 entries)

`RECALL` into `land$(60)`, `laender(men0&)` line 5069. Plain text, 2 columns: `LEFT$(...,3)` = abbreviation (auto-Kennzeichen), rest = German country name. Header row 0 = `Länderkürzel`. Selection writes `ed$(10) = LEFT$(land$(select&-1),3)`.

### 3.4 `INTERN\landnima.int` — NIMA country codes (263–264 entries)

`RECALL` into `land$(265)`, `land_List` line 21376. Line format `"XX = COUNTRY NAME"`; matched by `LEFT$(land$(i&),2)` in `ort_name_discr$` (21442). The original `DATA` statements that generated this file are preserved as comments at lines 21385–21432 — a complete verbatim code table, useful as a regeneration source.

### 3.5 DST codes (hard-coded, `sommerzeit`, line 5124)

```
soz$(1) = "0 = Standardzeit              "    korr = 0
soz$(2) = "1 = Einfache Sommerzeit       "    korr = 1
soz$(3) = "2 = Doppelte Sommerzeit       "    korr = 2
soz$(4) = "w = Kriegszeiten  = '1'       "    korr = 1
soz$(5) = "h = Halbe    Sommerzeit       "    korr = 0.5
soz$(6) = "m = Bestimmter Zeitmeridian   "    korr = 0
soz$(7) = "L = Ortszeit                  "    korr = 0
soz$(8) = "* = Ortszeit oder ohne Belang "    korr = 0
```
The single character goes to `ed$(21)` → AAF field 12. The same codes are decoded on AAF import at lines 5885–5900.

---

## 4. Interpretation texts — `<hrc$>\KOMMEN7P\*.TXT`

### Key finding: there is no keyed interpretation engine in HORCOM7P.

- The folder contains `.TXT`, not `.KOM` (11 files: `KOMM1..9.TXT`, `KOMMSTAT.TXT`, `AAF_KOMM.TXT`).
- The only mention of `.KOM` in 50,397 lines is a dead probe at line 291: `kom! = EXIST(hrc$ + "\KOMMEN7P\*.KOM")` — `kom!` is never read again.
- There is no `Deutung`/`interpretation` routine anywhere.
- No planet-in-sign / planet-in-house / aspect text lookup exists. Planet-in-sign/house results are rendered as symbols and tables (`ko_ta`, `bes11`, `plein1`), never as prose.

### What the files actually are

Flat, human-authored German essays displayed in a scrolling listbox.

`komm_les(f&)` (line 10060) is a pure switch from a menu index to a filename:

| `f&` | File |
|---|---|
| 1–9 | `<hrc$>\KOMMEN7P\KOMM<f>.TXT` |
| 10 | `<hrc$>\AENDLIST.TXT` (change log since 1996) |
| 11 | `<hrc$>\HINWEIS5.TXT` |
| 12 | `<hrc$>\KURZANL5.TXT` |
| 13 | `<hrc$>\KOMMEN7P\KOMMSTAT.TXT` |
| 14 | `<hrc$>\KOMMEN7P\AAF_KOMM.TXT` |

Menu entry points `ae1`…`ae14` at lines 9989–10058. `KOMM1` is titled `erle$ = "Einführender Kommentar"`.

### File structure rules (from `lese_text`, line 10097)

```
WHILE NOT EOF(#dat&)
  LINE INPUT #dat&,a$
  EXIT IF LEFT$(a$) = "-" && dat& <> 7     // nicht bei Zeitbest
  IF LEN(a$) < 256 && INSTR(a$,"~") = 0
    ltx$(i&) = a$
```

1. Plain CRLF lines, CP1252 (verified: `0xFC` = ü, `0xF6` = ö, `0xDF` = ß).
2. A line whose first character is `-` terminates the file (`AAF_KOMM.TXT` ends with a bare `-`). Exception: handle 7 = the ZEITBEST viewer.
3. Lines containing `~` are silently dropped (the tilde is HORCOM's universal "ignore" marker — same rule appears in the AAF parser).
4. Lines ≥ 256 chars are dropped.
5. For `f& = 1` only, 20 synthetic header lines (title, author address, licence/DEMO banner) are prepended before the file content; `bias& = 20` (licensed) or `22` (demo).
6. Max 3000 lines (`DIM ltx$(3000)`).

Display: modal `DIALOG #diha&` with a listbox, plus `SUCHEN` (case-insensitive `INSTR` scan) and `DRUCKEN` (`lese_text_pr`, 10303). Each line is pushed as `_ANSI$(_OEM$(ltx$(j&)))`.

`erste_hilfe` (line 1009) is the same idea for `INTERN\ersthilf.int`, but loaded via `RECALL #50,t$(),-1,x%` into `t$(58)` and shown as 57 fixed rows.

---

## 5. Config and licensing

### 5.1 `INTERN\KONSTA7P.INT` — all settings (verified, 940 bytes)

GFA `WRITE`/`INPUT` CSV. Writer `kon_dsp` (8971), reader `kon_dhol` (9022). `konsp` (9122) / `konre` (9189) are pure in-memory save/restore of the same variables (no I/O).

Exact stream order — a C++ loader must reproduce it byte-for-byte or the file is unreadable:

| Line | Fields |
|---|---|
| 1 | `haw&, haus$, appa&, appa$, gen&, gena$, apogw!, moknw!, orb, klsy!` |
| 2 | `klsyt!, sext!, pziff, voll!, nasp&, bdsp!` |
| 3 | `ryt!, par, fza, begz&, begz$, hard&, lfm&, bres&, brep&, zwhd!, kard!, horm&` |
| 4 | `orbe!, stats!, slist!, plusl!, col_dial%, col_backg%, farbs!, linie!` |
| 5 | `plinv&, tabstop&, prenbl&, halbs&, zeichen!, comp_mstz!` (+`comp_hand!` on read) |
| 6 | `comp_hand!, gitter!, selbst_cl_st!, eigfarb!, farbp!, weiss!` |
| 7 | `elem&, gebherr_dop!, haus1_dop!, jdgross, zal_grossj&, entf&, stzw&` |
| 8 | `erase&, fixpunkt&, fixpunkt$, fixpunkt_rh$, lpktg!, anzweg&, halbs_dir&, nursymb&, lin_inv!` |
| then | 41 lines `or&(0..40)` — per-object orb percentage |
| then | if `orbe!` 15 lines `orb$(0..14)` — per-aspect orb in degrees, quoted strings like `" 5.40"` |
| then | 22 lines `nk&(1..22)` — extra-body slot table (0 = off) |
| then | 19 lines `aspli|(1..19)` (byte), 19 `aspli%(1..19)` (RGB), 19 `aspst|(1..19)` (byte) |
| then | 4 lines `cols%(1..4)` (RGB) |
| then | 15 lines `pn&(1..15)` |

Note the write/read asymmetry on lines 5–6 (`comp_hand!` is written at the start of line 6 but read at the end of line 5). GFA `INPUT` ignores line boundaries, so it still works — but a strict line-oriented C++ parser will break. Parse the whole file as one comma-separated token stream, not line by line.

Quoted strings may contain commas — the real file contains `" Ephem ::App.1,MitParall."`. Booleans are `-1`/`0`. Read failure is caught:
```
CATCH
@fanz("FORMAT-ÄNDERUNG ! ALLE VORGABEN in HORCOM neu FESTLEGEN !")
```
Derived flags on load: `orbpl!` (any `or&(i)≠100`), `apog!` (`nk&(1)>0`), `np& = 18 + count(nk&>0)`, `klpl!`.

### 5.2 `INTERN\KENNTS7P.INT` — licence record (do not port)

Read once in `ap0` (line 25868). The path is assembled through the obfuscator `syt$()` (25978, builds a string from 8 char codes) so the literal never appears in the binary. Then:

```
OPEN "I",#80,k$
INPUT #80,accd%,rob$,pro$,aut$,adr10$,adr11$,adr12$,xkn%,asg$,kdn$
```

10 CSV fields: one int32 stored checksum, six licensee/author strings, one int32 (`xkn%`), one string, one copy-number string.

Mechanism (no key material quoted):

1. `a77(a$)` (25996) = plain sum of `ASC(UPPER$(char))` over the string — an 8-bit-sum hash, no salt.
2. `a70` (25983) computes `acc% = 17*a% + 7*b% + 3*c% + 30*d% + 12*e% + 43*f% + 11*g% + 3*h% + 9*i%` over the `a77` hashes of the licence fields, with `g% = xkn%` used raw. The comment at line 25990 notes `//!KENNWORT VERDECKT`.
3. The user types characters one at a time; after each keystroke `ae% = a77(typed_so_far)` and the loop exits when `ae% = xkn% AND accd% = acc%`. The password itself is never stored — only the sum of its character codes. Any anagram unlocks it.
4. `ESC` → `wrke! = TRUE` = DEMO mode. `wrkp` (25846) then restricts chart years to five hard-coded windows (obfuscated as `CHR$(100-nn)` arithmetic); out-of-range shows a `syt$`-built refusal string.
5. `ap1` (25835) re-checks and deliberately executes `a=1 : b=0 : c=a/b` (division by zero) on mismatch. The same trap fires if `KENNTS7P.INT` is missing (25890).
6. `kdn$` (copy number) is shown in the KOMM1 header; `adr10$/adr11$/adr12$` are the licensee's address lines.

`INTERN\KENNTS5P.INT` is the HORCOM5P equivalent, not touched by 7P.

Recommendation: delete the whole subsystem in the port. It offers no real protection and the file contains personal data of the licensee.

### 5.3 `INTERN\NOTIZ.INT` — vestigial

Only `nti! = EXIST(hrc$ + "\INTERN\NOTIZ.INT")` (line 290). `nti!` is never read. `PROCEDURE notiz` (25291) is a two-line stub; the menu entry now launches MSPAINT via `notiz_in` (7210). No format; drop it.

### 5.4 `INTERN\ersthilf.int` — quick-help text

`RECALL #50,t$(),-1,x%` into `t$(58)`, displayed as 57 listbox rows (`erste_hilfe`, 1009–1080). Plain CRLF text, CP1252, fixed 58 elements (element 0 is empty). Note the root also carries `ERSTHILF.TXT` / `ERSTHILFN.txt` which are not referenced by HORCOM7P — editing sources only.

### 5.5 `INTERN\AEND7.INT` — 6 bytes, `"4833\r\n"`

Not referenced anywhere in HORCOM7P.lst. Almost certainly a line counter for `AENDLIST.TXT` written by an external maintenance tool. Uncertain — ignore.

### 5.6 `INTERN\GRADE.INT` — user-defined "Gradpositionen"

`a17_3` (33170) reads, `a17eing` (33667) appends:
```
OPEN "I",#30,gri$ : INPUT #30,gr$,p1$,p2$      // read
OPEN "A",#30,gri$ : WRITE #30,gr$,p1$,p2$      // append
```
CSV, 3 quoted strings per line: degree (half-degree resolution — `gr& = 2*VAL(gr$)`, index range 0…719), planet 1, planet 2. Deleted wholesale with `KILL gri$`. File absent in the sample install.

### 5.7 `INTERN\ARABTEI1.INT` / `ARABTEI2.INT` — user-defined Arabic Parts

Two parallel random files (writer `arabt` 18363, reader `arab_eig` 18856). Paths at 18326/18327. Max 35 points (`jok& < 36`).

`ARABTEI1.INT`, 39-byte records — `FIELD #1,4 AS jj$,21 AS an$,14 AS bem$`:

| Off | Len | Field | Meaning |
|---|---|---|---|
| 0 | 4 | `jj$` | point index (ASCII int, `LSET STR$(jj&,4)`) |
| 4 | 21 | `an$` | point name |
| 25 | 14 | `bem$` | remark |

`ARABTEI2.INT`, 16-byte records — `FIELD #2,4 AS jj$,4 AS zz$,4 AS c3$,4 AS c4$` — three rows per point (the formula `A + B − C`):

| Off | Len | Field | Meaning |
|---|---|---|---|
| 0 | 4 | `jj$` | point index (FK to file 1) |
| 4 | 4 | `zz$` | term number 1..3 |
| 8 | 4 | `c3$` | object number, or house no. / ruler-of-house no. / zodiac degree ×`up` |
| 12 | 4 | `c4$` | kind: `0` = planet/point, `12` = house cusp, `13` = ruler of house, `14` = fixed zodiac degree |

All fields are ASCII decimal, written via `STR$(x,4)`. Neither file is present in the sample install.

### 5.8 `BILDER\FORMAT.BLD` — screen-format stamp

```
WRITE #1,coln%,coleb&,gdx&,gdy&      // screen 26092
INPUT #1,cn%,cb&,gx&,gy&             // pruef_format 26160
```
4-value CSV: colour count, colour bits, screen width, height. Used to invalidate saved images after a resolution change.

### 5.9 `BILDER\*.BMP` and `INTERN\<view><od><ze>.BMP` — not real BMP files

`bildst` (26292) / `bildl` (26257) write raw GFA `GET`/`PUT` sprite blobs, tiled `aufz& × aufz&`, each tile `sc%(i,k)` bytes, concatenated with `BPUT`/`BGET`. There is no BMP header and the byte layout depends on the display mode and resolution current at save time. Names: `INTERN\ASPEKT<od><ze>.BMP`, `HALBSM…`, `DOPPEL…`, `COMPOS…`, `COMBIN…`, `SECDIR…`, `TRANST…` (lines 19296, 19706, 29318, 30020, 30461, 35106, 36795). Do not write a loader — regenerate from the chart instead.

---

## 6. AAF import/export — `<aafpth$>\*.AAF`

Line-oriented CRLF text, CP1252, one logical record = several `#TAG:` lines with comma-separated fields. `~` anywhere in a line ⇒ the line is skipped entirely. `*` means "no value" (`ls& = INSTR(a$,"*")` truncates at the first `*`, line 5680). HTML tags are stripped by `tag_elim$` (4011) so accidentally-saved `.htm` downloads still parse.

Real sample (`150AST.AAF`):
```
#A93:Angeli Gottfried,*,*,05.08.1953,21:55:00,Haagen              ,*
#B93:2434595.3715,47N38:00,007E40:00,01he00:00,*
#COM:persönliche Angabe
```

### Field map (writer `make_aaf`, 3459; parser `aaf_horcom2`, 5648)

`#A93:` — 7 comma-separated fields:

| # | `ed$` | Content | Parser case |
|---|---|---|---|
| 1 | `ed$(0)` | Zuname (surname) | 1 |
| 2 | `ed$(1)` | Vorname | 2 |
| 3 | `ed$(2)` | `m` / `w` (sex) | 3 |
| 4 | `ed$(3).ed$(4).ed$(5)` | `dd.mm.yyyy`, optional `j` (force Julian) or `g` (force Gregorian) suffix | 4 |
| 5 | `ed$(6)h ed$(7):ed$(8)` | `HHhMM:SS` (also accepts `HH:MM:SS`) — civil time | 5 |
| 6 | `ed$(9)` | place name | 6 |
| 7 | `ed$(10)` | country abbreviation | 7 |

`#B93:` — 5 comma-separated fields:

| # | `ed$` | Content | Parser case |
|---|---|---|---|
| 1 | `ed$(11)` | Julian Date, `STR$(jd,13,5)` — takes priority over date/time if > 0 (`juld! = -1`) | 8 |
| 2 | `ed$(13)+ed$(12)+ed$(14):ed$(15)` | latitude `DDNMM:SS` / `DDSMM:SS` | 9 |
| 3 | `ed$(17)+ed$(16)+ed$(18):ed$(19)` | longitude `DDDEMM:SS` / `DDDWMM:SS` | 10 |
| 4 | `ed$(20)` | zone ZZD, `NNhE MM` / `NNhW MM` — `E` ⇒ subtract, `W` ⇒ add | 11 |
| 5 | `ed$(21)` | DST code `0 1 2 w h m L *` (see 3.5) | 12 |

Further single-field tags: `#COM:` `ed$(22)`, `#VIA:` `ed$(23)`, `#SRC:` `ed$(24)`, `#GZQ:` `ed$(25)`, `#ZNAM:` `ed$(26)`, `#CWORD:` `ed$(27)`, `#ATTRB:` `ed$(28)`. Continuation: any non-`#` line after a `#COM:` is appended with a space (line 3936).

Rettig's own warning about a real hazard in this format (`AAF_KOMM.TXT`) — worth keeping verbatim in the port's docs:

> „ACHTUNG! Mit den Umlauten gibt es immer wieder Fehl-Übersetzungen,die kaum vermeidbar zu sein scheinen,da die Dateien mit unterschiedlichen Code-Tabellen erstellt sein können.
>  Im AAF-Format sind Kommata wichtige Trennzeichen ! Wenn an der falschen Stelle ein Komma "entsteht",z.B. weil statt eines Umlauts ein Komma "übersetzt" wird,kann der betr. Datensatz nicht mehr korrekt interpretiert werden.
>  Sie erkennen das daran,daß beim Datum der 1.1.-4712 12h erscheint."

In English: "ATTENTION! Umlauts keep producing wrong translations that seem hardly avoidable, since the files may have been made with different code tables. In the AAF format commas are important separators! When a comma "appears" in the wrong place, for example because an umlaut is "translated" into a comma, the record concerned can no longer be read correctly. You recognise this by the date showing 1.1.-4712 12h."

and the JD priority rule:

> „Das Juldatum hat gegenüber anderen Zeitangaben PRIORITÄT. Wird primär ein JULDATUM eingegeben,so wird automatisch gesetzt: ZNAM = GMT / ZONE(ZZD) = 00hW00 oder 00hE00 oder "*" / SOMMERZEIT = "*""

In English: "The Julian date takes PRIORITY over all other time data. When a JULIAN DATE is entered first, the program sets ZNAM = GMT / ZONE(ZZD) = 00hW00 or 00hE00 or "*" / SUMMER TIME = "*" automatically."

### Pairing rule and helpers

> „Ist z.B eine neue AAF-Datei namens ..\AAFDATEN\NEUDAT.AAF gefunden worden, so wird eine HORCOM-Datei namens...\HORCOM\SPEZIAL\NEUDAT.DAT gebildet. Die parallele HORCOM-Datei GLEICHEN NAMENS dient als Pilot."

In English: "If for example a new AAF file named ..\AAFDATEN\NEUDAT.AAF was found, a HORCOM file named ...\HORCOM\SPEZIAL\NEUDAT.DAT is built. The parallel HORCOM file of the SAME NAME serves as the pilot."

`bilde_horcfile$` (24915) / `bilde_aaffile$` (24922) implement exactly that basename swap. Record identity key: surname + given name (`aaf_ident`, 22862; `datensatz_maxanz`, 22223).

| Line | Proc | Role |
|---|---|---|
| 5648 | `aaf_horcom2` | AAF → 128-byte `.DAT` (this is the import path) |
| 6012/6041 | `horcom_aaf2/3` | `.DAT` → AAF (`STORE #7,st$(),5` writes 5 packed lines) |
| 4732 | `aaf_anzahl` | count `#A93:` lines |
| 3867 | `aafdatei_anzahl` | same + 10 slack |
| 4746 / 4909 | `aaf_satz_add` / `aaf_satz_speich` | append 10 `sti$()` lines, skipping empty/`*` ones |
| 4948 | `aaf_satz_loesch` | rewrite the file omitting record `nrds&`, via `INTERN\QAYWSX.AAF` |
| 953 | `aaf_horcom_wandeln` | `DIR aafpth$+"\*.AAF" TO aafpth$+"\DIRE.TXT"`, then bulk-convert. `DIRE.TXT` is a throwaway directory listing, one filename per line. |
| 3889 | `aaf_datei_oeffnen` | scan + index into `sti$(nrds&,1..3)` |

`AAFHELP\AAF.HLP` is Martin Garms' original Windows help file, launched externally.

---

## 7. Statistics — `<hrc$>\STATIST7\`

Three files per dataset, base name taken from the source `SPEZIAL\<NAME>.DAT`.

### 7.1 `<NAME>.STA` — 210-byte records (one per chart)

Writer `stat1` (11982) + `stat1_0/1/2` (12161/12185/12229); reader `stat2` (12267). Verified: all 5 `.STA` files are exact multiples of 210, and record counts match their `.STH` twins exactly.

| Off | Len | Field | Type |
|----|----|----|----|
| 0 | 25 | `na$` | name, LSET, uppercased, trimmed |
| 25 | 20 | `go$` | place, LSET, uppercased |
| 45 | 16 | `da$` | packed date/time, exactly `STR$(ta,2,0)+STR$(mo,2,0)+STR$(ja,5,0)+STR$(ho,2,0)+STR$(mi,5,2)` |
| 61 | 13 | `gf$` | packed coords: `STR$(gl,7,2)` + `STR$(gg,6,2)` |
| 74 | 4×34 | 34 longs | `MKL$` int32 LE |

`da$` sub-layout (1-based `MID$` offsets within `da$`, from `stat2` lines 12341–12345): day (1,2), month (3,2), year signed (5,5), hour (10,2), minute 2 decimals (12,5). `gf$`: bytes 1–7 = longitude (2 decimals), 8–13 = latitude (2 decimals).

The 34 int32 values are angle in radians × 10 000 000 (`asn% = 10000000`, line 9304; encode `MKL$(el(n) * a%)`, decode `CVL(x$) / asn%`). Order (from `stat1_1`, with Rettig's inline comments):

| Slot | Source | `as%()` idx | Comment in listing |
|---|---|---|---|
| 1–12 | `el(1)…el(12)` | 8–19 | `//SO`, `//MO bzw.TE`, `//ME`, `//VE`, `//MA`, `//JU`, `//SA`, `//UR`, `//NE`, `//PL`, `//DR`, `//DS` |
| 13 | `f(1)` | 20 | `//13 AC` |
| 14 | `f(10)` | 21 | `//14 MC` |
| 15 | `f(2)` | 22 | house 2 |
| 16 | `f(3)` | 23 | house 3 |
| 17 | `f(5)` | 24 | house 5 |
| 18 | `f(6)` | 25 | house 6 |
| 19–34 | `el(nk&(1))` … `el(nk&(16))` | `nk&(i)+7` | `// AG ZUSATZ-PL.`, `// CH`, `// TP`, `// GL`, `// CE`, `// PA`, `// JN`, `// VS`, `// CU`, …, `// PO` |

`stat2_zuord` (12257) only stores extras when `nk&(zk&) > 18`. Heliocentric files are detectable: `IF as%(i&,20)=0 && as%(i&,21)=0 && as%(i&,22)=0 THEN hrge! = -1` (AC/MC/H2 all zero). Hard limit: `laf& > 15975` ⇒ `"ZU GROßE Anzahl Datensätze !"`.

### 7.2 `<NAME8>1.STH` — 24-byte records (one per `.STA` record)

`FIELD #26,4 AS s2$,4 AS t2$,4 AS u2$,4 AS v2$,4 AS w2$,4 AS y2$` — six more int32 (radians × 1e7), being `el(nk&(17))` … `el(nk&(22))` (commented `// QU`, `// HL`, `// PH`, `// DA`, `// NS`, and a sixth). This is the HORCOM7 extension bolted onto the v5 `.STA`. Naming rule (`stat2_teil`, 12245): `daa$` carries the leading backslash, so `LEFT$(daa$,8)` keeps seven letters of the base name, then `"1"` is appended, then `.STH`. E.g. `BERUEHMT.STA` → `BERUEHM1.STH`, a name of seven letters or fewer keeps all of them.

### 7.3 `<NAME>.PAR` — the calculation parameters (CSV)

Writer 12037, reader `stat2parl` (12588). Verified against `150AST.PAR` (131 bytes):

```
1,"Placidus",1,"App.1",2," Ephem ::App.1,MitParall.",-1,-1,1
19
20
0            (× 19)
21
```

| Position | Variable | Meaning |
|---|---|---|
| 1 | `haw&` | house-system index |
| 2 | `haus$` | house-system name |
| 3 | `appa&` | apparent-position mode index |
| 4 | `appa$` | its label |
| 5 | `gen&` | precision mode index |
| 6 | `gena$` | its label (contains a comma — must be read as a quoted string) |
| 7 | `apogw!` | apogee flag (`-1`/`0`) |
| 8 | `moknw!` | node flag |
| 9 | `par` | parallax flag |
| 10…31 | `nk&(1..22)` | extra-body slot table, one per line |

A `.STA` cannot be interpreted without its `.PAR` — the meaning of the 16+6 extra-body slots is entirely defined by `nk&()`.

### 7.4 `BERUEHMT*` and the v5 `STATIST\` folder

`STATIST7\BERUEHMT.{STA,STH,PAR}` is just an ordinary dataset built from `SPEZIAL\BERUEHMT.DAT` ("famous people"). The legacy `STATIST\` folder holds a v5-era pair; HORCOM7P never references `\STATIST\` (only `\STATIST7`). Treat v5 statistics files as out of scope.

### 7.5 Incremental update

`a22dat` (23270) appends a newly saved chart to an existing `.STA`/`.STH` pair, searching linearly for a record with matching trimmed `na$` and overwriting it via `RECORD #25,k& : PUT #25`, otherwise appending at `RECORD #25,i&+1`.

> Bug to preserve or fix knowingly: in the append branch (lines 23459–23464) the `.STA` write uses `RECORD #25,i&+1` but the `.STH` write uses `RECORD #26,k&` — and `k&` is still 0 when no match was found. The two files can therefore go out of sync on append.

---

## 8. Ephemerides (brief — see Part 1 and Part 4)

### 8.1 `<hrc$>\*.EPH` — 16-byte records

`ephem_auswert` (48309), open at 48370:
```
OPEN "r",#10,p$,16
FIELD #10,4 AS jdt$,4 AS x1$,4 AS x2$,4 AS x3$
```
Four int32 LE: JD day number, then X, Y, Z × `fplanet`. Step `djd` derived from records 1 and 2; range end from the last record. 5-point interpolation (`ipol`, 48492) around `ind%`.

### 8.2 `INTERN\PLANETS.NDX` + `INTERN\PLANETS.DAT` — Meeus/VSOP87 series

`readpterm(pl&)` (48855), `plposhi(pl&)` (48913). Comment: `'MEEUS`.

`PLANETS.NDX`, 4-byte records, `FIELD 2 AS a$,2 AS b$` — 648 B / 4 = 162 records = 9 bodies × 3 coordinates (L, B, R) × 6 powers (T⁰…T⁵). Fields: `CVI` uint16 offset (0-based record index into PLANETS.DAT), `CVI` uint16 term count. Record index = `pl& * 18 + (i-1) * 6 + j`, with `i = 1..3` (L,B,R) and `j = 0..5`.

`PLANETS.DAT`, 24-byte records, `FIELD 8 AS c$,8 AS d$,8 AS e$` — 63 288 B / 24 = 2 637 records: three `CVD` doubles A, B, C of `A·cos(B + C·t)`. Verified: record 0 = `A=1.75347045673, B=0, C=0` — Earth's L0 leading term from Meeus. NDX[0] = `(0, 64)` and the offsets accumulate 0→64→98→118→125→127, confirming 0-based record indices.

Rettig documents the provenance in `KOMMEN7P\KOMM3.TXT`:

> „Für die grossen Planeten ME,VE,ERDE (SONNE) MA,JU,SA,UR,NE, wird in HORCOM eine abgekürzte Version der neuesten analytischen Theorie VSOP87 des BUREAU DES LONGITUDES,PARIS ( P.BRETAGNON ) verwendet.Diese wurde nun durch JEAN MEEUS ("ASTRONOMISCHE ALGORITHMEN", WILLMANN BELL INC,JOHANN AMBROSIUS BARTH, ISBN 3-335-00318-7) einem größeren Kreis zugänglich gemacht."

In English: "For the major planets ME, VE, EARTH (SUN), MA, JU, SA, UR, NE HORCOM uses an abridged version of the latest analytical theory VSOP87 of the BUREAU DES LONGITUDES, PARIS (P. BRETAGNON). JEAN MEEUS has now made it accessible to a wider circle (ASTRONOMICAL ALGORITHMS, WILLMANN BELL INC, JOHANN AMBROSIUS BARTH, ISBN 3-335-00318-7)."

---

## 9. Character encoding

| Family | Encoding | Evidence |
|---|---|---|
| `INTERN\*.int` (laender, zonnamen, ersthilf) | CP1252 | `0xE4`=ä, `0xFC`=ü, `0xDC`=Ü |
| `KOMMEN7P\*.TXT`, `AENDLIST.TXT` | CP1252 | `0xFC`=ü, `0xF6`=ö, `0xDF`=ß |
| `*.AAF` | CP1252 | `0xF6`=ö |
| `ZEITBEST\*.TXT` | mixed CP437 / CP1252 — same file contains `0x81` (CP437 ü) and `0xFC` (CP1252 ü) | `ALBANIEN.TXT` |
| `.DAT` / `.INT` / `.STA` string fields | mostly ASCII; assume CP1252 with fallback | round-tripped through `_ANSI$(_OEM$())` in `a201`/`a2f_tr_ort` |

Treat encoding as best-effort and normalise to UTF-8 on import.

---

## 10. Recommended implementation order for the C++ port

### Tier 1 — implement as real loaders (needed to open any existing user data)

| # | Format | Why first | Effort |
|---|---|---|---|
| 1 | `SPEZIAL\*.DAT` (128 B chart record) | The entire user corpus. Trivial fixed-record parse; every other subsystem keys off it. | XS |
| 2 | `SPEZ_ORT\*.INT` + `BIGFILES\*.INT` (36 B place record) | 425 files, ~200k places, no alternative source. Same trivial parse. | XS |
| 3 | `INTERN\KONSTA7P.INT` | Without it the app cannot restore the user's house system, orbs, body selection. Needs a proper GFA-CSV tokeniser (quoted strings containing commas, `-1` booleans, whole-file token stream, not line-based). | S |
| 4 | `INTERN\PLANETS.NDX` + `PLANETS.DAT` | Required for any planetary position at all. Fully decoded and verified above. | S |
| 5 | `*.EPH` (16 B) | Asteroids/Chiron/Pluto. Needs the per-body `fplanet` table from lines 48310–48360. | S |
| 6 | AAF reader/writer | The only interoperable format; also the richest (comments, sources, data quality). Reuse it as the port's own archive format. | M |

### Tier 2 — implement, but only if the feature ships

| # | Format | Note |
|---|---|---|
| 7 | `STATIST7\*.STA` + `*.STH` + `*.PAR` | Straightforward once `.PAR`'s `nk&()` is read first. Only needed if the statistics module is ported. Regenerable from the `.DAT` files, so a convenience loader, not a necessity. |
| 8 | `INTERN\zonnamen.int`, `laender.int`, `landnima.int` | Tiny fixed-column text. Best treated as seed data: parse once, emit JSON/CSV, then delete the loader. `landnima.int` can be regenerated from the `DATA` comments at lines 21385–21432. |
| 9 | `ORT.EXT` | One record, dual coordinate encoding. Ported as a file (`read_preferred_place`, `write_preferred_place`), the VORZUGSORT menus of both entry boxes read and write it. |
| 10 | `INTERN\ARABTEI1/2.INT`, `INTERN\GRADE.INT` | Absent from the sample install; implement only on demand. Formats fully specified above. |

### Tier 3 — migrate, do not port

| Format | Recommendation |
|---|---|
| `ZEITBEST\*.TXT` | Never machine-read by HORCOM. Replace with IANA tzdata (`std::chrono::tzdb`), which supersedes these tables for the modern era. Keep the 29 files as read-only historical documents; Rettig's per-country prose is genuinely valuable as citations. Note the 177-entry historic table in `reference/arbeit/ZEITZONN.LST` (pre-1900 local mean times) is NOT covered by IANA and must be preserved as data. |
| `KOMMEN7P\*.TXT` | Not an interpretation system — nine German essays plus a change log. Convert once to Markdown (strip `~` lines, honour the leading-`-` terminator, decode CP1252) and ship as static help. |
| `INTERN\KENNTS7P.INT` + `ap0`/`a70`/`a77`/`ap1`/`wrkp` | Drop entirely. The scheme is an additive character-code sum, the "protection" is a deliberate divide-by-zero, and the file holds the licensee's name and address. Deleting it also removes the DEMO year-range restrictions in `wrkp`. |
| `INTERN\NOTIZ.INT`, `INTERN\AEND7.INT` | Vestigial. Delete. |
| `BILDER\*.BMP`, `INTERN\*.BMP`, `BILDER\FORMAT.BLD` | Resolution-locked raw sprite blobs with no header. Regenerate charts on demand; export real PNG. |
| `SPEZ_ORT\ortelist.txt`, `AA_CODES.txt`, `AA_TIMEZ.txt`, `AA_ZONENZEITEN.htm`, `ERSTHILF.TXT`, `ERSTHILFN.txt`, `HINWEIS5.TXT`, `KURZANL5.TXT` | Unreferenced or documentation-only. Archive as-is. |
| Scratch files (`QAYWSXED.DAT`, `YWSXED.INT`, `QAYWSX.AAF`, `DIRE.TXT`, `RRESERVE.*`) | Artefacts of the "copy to temp, KILL, NAME" edit pattern. Replace with in-memory edits + atomic rename. |

### Suggested target format for migrated data

One SQLite database (`charts`, `places`, `chart_sets`) plus AAF as the interchange/export format. That removes the FILESELECT-per-file model, the 15,975-record QSORT ceiling, the linear place search, the `.STA`/`.STH` sync bug, and the temp-file-rename data-loss window in one move — while AAF preserves the fields (comments, sources, data quality, `#CWORD`, `#ATTRB`) that the 128-byte `.DAT` record has always been truncating to 51 characters.
