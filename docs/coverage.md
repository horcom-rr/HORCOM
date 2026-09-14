# Port coverage ledger

Every `PROCEDURE` and `FUNCTION` of the original `HORCOM7P` main program,
one row each, with how the C++ rewrite accounts for it. The list of
names was extracted mechanically from the listing, so a routine cannot
fall through unnoticed, a claim of completeness is a lookup in this
table rather than a judgment call.

Statuses.

- **ported** — the routine is named in the C++ sources or in the
  handbook origin maps, its behaviour carried over with tests.
- **absorbed** — an internal helper of a ported routine, its behaviour
  lives inside the named port without a separate mention.
- **screen era** — screen, dialog, bitmap, mouse or printer plumbing of
  the GFA BASIC environment, superseded by the Qt shell, the display
  list and the shared painter. No astronomical or astrological content.

Counts: 328 ported by name, 165 absorbed, 600 screen era, 1093 total.

| original | status | account |
|---|---|---|
| `DIM` | ported | named in the sources or the handbook origin maps |
| `GetBmpSize` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `MULTINULLOST` | absorbed | internal helper of the MULTI directions |
| `MULTINULLWEST` | absorbed | internal helper of the MULTI directions |
| `PaletteRestore` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `PaletteSave` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `Symbhol` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `VAL` | ported | named in the sources or the handbook origin maps |
| `a1` | ported | named in the sources or the handbook origin maps |
| `a10` | ported | named in the sources or the handbook origin maps |
| `a10nk` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a10re` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a11` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a11_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a11ex` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a11ex1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12` | ported | named in the sources or the handbook origin maps |
| `a12a` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12al` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12asp` | ported | named in the sources or the handbook origin maps |
| `a12beschr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12beschr1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12f` | ported | named in the sources or the handbook origin maps |
| `a12i` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12ia` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a13` | ported | named in the sources or the handbook origin maps |
| `a13aus` | absorbed | internal helper of the combin chart |
| `a13comp_1` | ported | named in the sources or the handbook origin maps |
| `a13comp_2` | ported | named in the sources or the handbook origin maps |
| `a13end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a14` | ported | named in the sources or the handbook origin maps |
| `a14_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a14auscomb` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a14end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a16` | ported | named in the sources or the handbook origin maps |
| `a16_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a16_i` | absorbed | internal helper of the plant longitude search |
| `a16_ta` | ported | named in the sources or the handbook origin maps |
| `a16end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a16zang` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a170` | ported | named in the sources or the handbook origin maps |
| `a17000` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a1701` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a170_1tit` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a171` | ported | named in the sources or the handbook origin maps |
| `a172` | ported | named in the sources or the handbook origin maps |
| `a1720` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a173` | ported | named in the sources or the handbook origin maps |
| `a174` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a174g` | ported | named in the sources or the handbook origin maps |
| `a174init` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a175` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a178` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a1781` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a1791` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a17911` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a1792` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a1795` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a17_3` | ported | named in the sources or the handbook origin maps |
| `a17_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17ach` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17eing` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17eing11` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17eing12` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17end` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a17in` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17sol` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17sonderpkt` | ported | named in the sources or the handbook origin maps |
| `a17tab` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18` | ported | named in the sources or the handbook origin maps |
| `a180` | ported | named in the sources or the handbook origin maps |
| `a1800` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a180_1` | ported | named in the sources or the handbook origin maps |
| `a180a` | ported | named in the sources or the handbook origin maps |
| `a180aus` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a180b` | ported | named in the sources or the handbook origin maps |
| `a180end1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a180iplv` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a180ival` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a180sob1` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a180sob2` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a180trpr` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a180trpr_lin` | ported | named in the sources or the handbook origin maps |
| `a181` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `a181_discr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a181t` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a181t1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a181tx` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a181tx1` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `a181ver` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18_3045` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18_entz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18_lin` | ported | named in the sources or the handbook origin maps |
| `a18_line_col` | ported | named in the sources or the handbook origin maps |
| `a18asw` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a18asw1` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a18eing` | ported | named in the sources or the handbook origin maps |
| `a18eing_plw` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a18end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18ko1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18kopf` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a18list` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a18liv` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18livdir` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18so` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a18st` | ported | named in the sources or the handbook origin maps |
| `a18st0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18tab` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a19` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a20` | ported | named in the sources or the handbook origin maps |
| `a200dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a200get` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a200loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a200loe_1` | absorbed | internal helper of the transit scan with the event place |
| `a200ort` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a201` | ported | named in the sources or the handbook origin maps |
| `a2011` | ported | named in the sources or the handbook origin maps |
| `a2011f` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a20_horg` | absorbed | internal helper of the transit double wheel |
| `a20end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a210` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a211` | absorbed | internal helper of the sign ingresses |
| `a2111` | absorbed | internal helper of the sign ingresses |
| `a2111_i` | absorbed | internal helper of the sign ingresses |
| `a2111_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2113` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2113_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a211_nnam` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a221` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a22dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a22ort` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a22sta_anz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a22ueberschrb` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2f_tr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2f_tr2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2f_tr_dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2f_tr_ort` | absorbed | internal helper of the transit scan with the event place |
| `a2f_weiter` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2fdat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2fort` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2ort` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a3` | ported | named in the sources or the handbook origin maps |
| `a31` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a311` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a311_o` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a316061` | ported | named in the sources or the handbook origin maps |
| `a31_o` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a37dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a4` | ported | named in the sources or the handbook origin maps |
| `a5` | ported | named in the sources or the handbook origin maps |
| `a6` | ported | named in the sources or the handbook origin maps |
| `a60` | ported | named in the sources or the handbook origin maps |
| `a60_l` | ported | named in the sources or the handbook origin maps |
| `a61` | absorbed | internal helper of the house systems and the house table |
| `a611` | absorbed | internal helper of the house systems and the house table |
| `a7` | ported | named in the sources or the handbook origin maps |
| `a70` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a77` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a8` | ported | named in the sources or the handbook origin maps |
| `a81` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a82` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a9` | ported | named in the sources or the handbook origin maps |
| `a90` | ported | named in the sources or the handbook origin maps |
| `a901` | ported | named in the sources or the handbook origin maps |
| `a901_m` | ported | named in the sources or the handbook origin maps |
| `a91` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a92` | ported | named in the sources or the handbook origin maps |
| `a921` | absorbed | internal helper of the kardinal point slots |
| `a93` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a9zw` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_anzahl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_box` | absorbed | internal helper of the AAF reader and writer suite |
| `aaf_datei_oeffnen` | absorbed | internal helper of the AAF reader and writer suite |
| `aaf_edit_nr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_help` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_horcom` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_horcom0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_horcom1` | absorbed | internal helper of the AAF reader and writer suite |
| `aaf_horcom2` | absorbed | internal helper of the AAF reader and writer suite |
| `aaf_horcom_end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_horcom_wandeln` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_ident` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_orteingabe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_satz_add` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_satz_loesch` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_satz_speich` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aafbox_in_horcomdaten` | absorbed | internal helper of the AAF reader and writer suite |
| `aafdatei_anzahl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `abort_pr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `acmcl` | absorbed | internal helper of the degree formatting of the tables |
| `ad` | ported | named in the sources or the handbook origin maps |
| `adjustwin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `adop` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae10` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae11` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae12` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae14` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae3` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae4` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae5` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae6` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae7` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae8` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae9` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aeaaf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aendlist` | ported | named in the sources or the handbook origin maps |
| `aeqe` | ported | named in the sources or the handbook origin maps |
| `aeqh` | ported | named in the sources or the handbook origin maps |
| `aestat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `afo` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ag` | ported | named in the sources or the handbook origin maps |
| `alertbox` | ported | named in the sources or the handbook origin maps |
| `alerte` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `amundpr` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `amundprs` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `ansth` | absorbed | internal helper of the Dynamogramm curves |
| `anzahl_ein` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `anzeigen_asp_zaehl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `anzeigen_halbs_zaehl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ap` | ported | named in the sources or the handbook origin maps |
| `ap0` | ported | named in the sources or the handbook origin maps |
| `ap1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aprim` | ported | named in the sources or the handbook origin maps |
| `aprim1` | ported | named in the sources or the handbook origin maps |
| `aprimout` | ported | named in the sources or the handbook origin maps |
| `aq` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aqn` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ar` | ported | named in the sources or the handbook origin maps |
| `ar_sys` | ported | named in the sources or the handbook origin maps |
| `arab_eig` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arabl` | absorbed | internal helper of the arabic parts |
| `arabl0` | ported | named in the sources or the handbook origin maps |
| `arabl1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arabso` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arabt` | ported | named in the sources or the handbook origin maps |
| `arabte` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arabtex` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arb_dim` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arb_er` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arde_eleb` | absorbed | internal helper of the converter dialogs |
| `areg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `areg1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `areg11` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `art11` | ported | named in the sources or the handbook origin maps |
| `art12` | ported | named in the sources or the handbook origin maps |
| `asp0` | ported | named in the sources or the handbook origin maps |
| `asp1` | ported | named in the sources or the handbook origin maps |
| `asp10` | ported | named in the sources or the handbook origin maps |
| `asp11` | ported | named in the sources or the handbook origin maps |
| `asp_analy_0` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `asp_analy_1` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `asp_analy_1_mund` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `asp_analy_1_rad` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `asp_analy_mund` | ported | named in the sources or the handbook origin maps |
| `asp_analy_rad` | ported | named in the sources or the handbook origin maps |
| `asp_ds` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `asp_halbs_zaehler` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asp_li` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asp_li_0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asp_li_upd` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspar` | ported | named in the sources or the handbook origin maps |
| `aspar2` | absorbed | internal helper of the Aspektarium matrix |
| `aspar2_ini` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspdis` | ported | named in the sources or the handbook origin maps |
| `asphi1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asphist` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps12` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps17` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps18` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps19` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps3` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps4` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps5` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps6` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps8` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asps_dspl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspsenk` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspsenk1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspssyver` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspwag` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspwag1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspz0` | ported | named in the sources or the handbook origin maps |
| `aspz1` | ported | named in the sources or the handbook origin maps |
| `aspz1_0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspz1_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspz1_1_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspz1_2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asy111` | ported | named in the sources or the handbook origin maps |
| `asy112` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `asymb` | ported | named in the sources or the handbook origin maps |
| `asymb1` | ported | named in the sources or the handbook origin maps |
| `asymbin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `atn` | ported | named in the sources or the handbook origin maps |
| `auf_2` | ported | named in the sources or the handbook origin maps |
| `auf_g` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `auf_h` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `auf_pl` | ported | named in the sources or the handbook origin maps |
| `auf_unt` | ported | named in the sources or the handbook origin maps |
| `auf_unt2` | ported | named in the sources or the handbook origin maps |
| `auf_unt3` | absorbed | internal helper of the rise and set screen |
| `aufl_ziff` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ausw_datei` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ausw_obj_e` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ausw_obj_m` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ausw_pl_hs` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ausw_pl_hs1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `auswahl_flag` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `auswert_ortelist` | absorbed | internal helper of the place search over his gazetteer |
| `avd` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `avd_lin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ave` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `avers` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `avg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `avh` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `backg_col` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `balken` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bed_erf` | ported | named in the sources or the handbook origin maps |
| `bed_erf_1` | ported | named in the sources or the handbook origin maps |
| `bed_erf_2` | ported | named in the sources or the handbook origin maps |
| `bed_erf_asp` | ported | named in the sources or the handbook origin maps |
| `bed_erf_asp1` | ported | named in the sources or the handbook origin maps |
| `bed_erf_asp2` | ported | named in the sources or the handbook origin maps |
| `bed_erf_nam` | ported | named in the sources or the handbook origin maps |
| `beep3` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes00` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes1` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `bes10` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes11` | ported | named in the sources or the handbook origin maps |
| `bes110` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes111` | ported | named in the sources or the handbook origin maps |
| `bes1110` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes111_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes11_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes11_11` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes1_big` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `bes2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes2_comp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes_big_asp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes_big_elem` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes_big_halbs` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes_big_haus` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `bes_big_kafige` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes_big_plan` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `bilde_aaffile` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bilde_horcfile` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bilder_loesch` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bildl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bildld` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bildst` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bl_anz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bls` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bmp_color_pl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bmp_color_ze` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bmp_load` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `boxn` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `calc_in` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `camp` | ported | named in the sources or the handbook origin maps |
| `camp1` | ported | named in the sources or the handbook origin maps |
| `ce` | ported | named in the sources or the handbook origin maps |
| `ce_` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ch` | ported | named in the sources or the handbook origin maps |
| `clear_datflg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `clear_ortflg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `closew` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `clr_main` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `clrerw` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `cls_screen` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `cls_std` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `cn` | ported | named in the sources or the handbook origin maps |
| `col_halbsel` | absorbed | internal helper of the midpoint scan and the composite rule |
| `col_norm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `col_zeich` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `color_dial` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `copy_expand_file` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `copy_file` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `cp` | ported | named in the sources or the handbook origin maps |
| `cu` | ported | named in the sources or the handbook origin maps |
| `da` | ported | named in the sources or the handbook origin maps |
| `das_anz` | absorbed | internal helper of the record display boxes |
| `das_anz_odat` | absorbed | internal helper of the record display boxes |
| `dat` | ported | named in the sources or the handbook origin maps |
| `dat_discr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dat_jd` | absorbed | internal helper of the converter dialogs |
| `dat_jd1` | absorbed | internal helper of the converter dialogs |
| `datanz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `date_form` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `datei_pr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `datensatz_maxanz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `daterw` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `datru` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dats_einz_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dats_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dauert` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `day_w` | ported | named in the sources or the handbook origin maps |
| `day_y` | absorbed | internal helper of the calendar and the weekday line |
| `def_but` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `deffi` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `defm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `deftextcol` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `delblackmoon` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `desct` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dial_close` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dial_ini` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dial_shift` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dif_vg` | ported | named in the sources or the handbook origin maps |
| `dim_lin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dima17_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dima17_2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dimd` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dimdir` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dime` | absorbed | internal helper of the coordinate table |
| `dimsc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dimso` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dimsodir` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dir_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `direkt` | ported | named in the sources or the handbook origin maps |
| `dirend` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `dirstop` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `disablewin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `discf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `disckill` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `diso` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dk` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dopp_haus_i` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dr` | ported | named in the sources or the handbook origin maps |
| `dr_ds_halbsum` | absorbed | internal helper of the midpoint scan and the composite rule |
| `dr_fehl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `drad2` | ported | named in the sources or the handbook origin maps |
| `drad2_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dradst` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dragline_y` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dragline_y0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dragline_y1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dragline_yc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `druck_einr_anz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `druck_enbl_alt` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `druck_graph_ein` | ported | named in the sources or the handbook origin maps |
| `druck_horm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `druck_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ds` | ported | named in the sources or the handbook origin maps |
| `dummy` | ported | named in the sources or the handbook origin maps |
| `eckp` | ported | named in the sources or the handbook origin maps |
| `eckp1` | ported | named in the sources or the handbook origin maps |
| `ein_zuo` | absorbed | internal helper of the place search over his gazetteer |
| `eing_box` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `eing_ti` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `eingabe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `eingalp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `eingfl_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `einzel_bogen_anz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `einzel_plan_display` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `einzel_plan_wahl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `einzel_plan_wahl1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `eleb_arde` | absorbed | internal helper of the converter dialogs |
| `elem1` | ported | named in the sources or the handbook origin maps |
| `elem2` | ported | named in the sources or the handbook origin maps |
| `elem_col` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `elemhist1` | absorbed | internal helper of the histogram drawing |
| `enablewin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `enablw` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ende` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `entf_min_max` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ephem_auswert` | ported | named in the sources or the handbook origin maps |
| `er_lin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `era17_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `era17_2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `erasep` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `erdim` | absorbed | internal helper of the coordinate table |
| `erdime` | absorbed | internal helper of the coordinate table |
| `erdir` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ereig_ort` | ported | named in the sources or the handbook origin maps |
| `erg_rad` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `erso` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ersodir` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `erste_hilfe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `et_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `et_reg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `et_sp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `et_ut` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `et_ut_erl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `etm_ut` | absorbed | internal helper of the converter dialogs |
| `etut` | ported | named in the sources or the handbook origin maps |
| `fanz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `fbox` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `fehler` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `fill_box` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `fill_color` | ported | named in the sources or the handbook origin maps |
| `finst` | ported | named in the sources or the handbook origin maps |
| `finst_0` | ported | named in the sources or the handbook origin maps |
| `finst_1` | ported | named in the sources or the handbook origin maps |
| `finst_a` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `finst_e` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `finst_s` | absorbed | internal helper of the eclipse aspects |
| `fixp_def` | absorbed | internal helper of the fixed point of slot zero |
| `fixp_sperr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `fixpunkt_anz` | absorbed | internal helper of the fixed point of slot zero |
| `fixpunkt_def` | ported | named in the sources or the handbook origin maps |
| `fixpunkt_def_a12` | absorbed | internal helper of the fixed point of slot zero |
| `fixpunkt_def_mult` | absorbed | internal helper of the fixed point of slot zero |
| `freebmp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `funkt` | ported | named in the sources or the handbook origin maps |
| `geb_herr` | absorbed | internal helper of the birth ruler |
| `genau` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `geohelio` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `gesp_neu` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `get_aufl_bs` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `get_aufl_pr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `getbm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `gettext` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `gettextaaf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `gettexti` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `gl` | ported | named in the sources or the handbook origin maps |
| `glanz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `gm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `greg_doll` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `greg_doll_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `grli` | absorbed | internal helper of the Grad-Liste |
| `grli1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `grli_f` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `grlinit` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `grmise` | absorbed | internal helper of the degree formatting of the tables |
| `grossj` | absorbed | internal helper of the Grosses Jahr age point |
| `grossj1` | absorbed | internal helper of the Grosses Jahr age point |
| `grundaspekt` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `grze` | absorbed | internal helper of the degree formatting of the tables |
| `grze_0` | absorbed | internal helper of the degree formatting of the tables |
| `grzemise` | absorbed | internal helper of the degree formatting of the tables |
| `ha` | ported | named in the sources or the handbook origin maps |
| `habes` | absorbed | internal helper of the Dynamogramm curves |
| `haeuser_pruef` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `halbs1` | ported | named in the sources or the handbook origin maps |
| `halbs11` | ported | named in the sources or the handbook origin maps |
| `halbs111` | ported | named in the sources or the handbook origin maps |
| `halbs_dial` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `halbs_ruecksetz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `halbs_zaehl_gr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `halbs_zaehler` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `halbsa` | absorbed | internal helper of the midpoint scan and the composite rule |
| `halbsm` | absorbed | internal helper of the midpoint scan and the composite rule |
| `halbsmin` | ported | named in the sources or the handbook origin maps |
| `halbszaus` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `handlemessaaf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `handlemessage` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `handlemessagei` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hardc_kompl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hardcopy` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `harm` | absorbed | internal helper of the harmonic charts |
| `harm21` | ported | named in the sources or the handbook origin maps |
| `haus_ber` | ported | named in the sources or the handbook origin maps |
| `haus_def` | ported | named in the sources or the handbook origin maps |
| `haus_sel` | ported | named in the sources or the handbook origin maps |
| `hausa` | absorbed | internal helper of the house systems and the house table |
| `haust` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `haustab` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hausw` | ported | named in the sources or the handbook origin maps |
| `hd_hs` | ported | named in the sources or the handbook origin maps |
| `hel_geo` | ported | named in the sources or the handbook origin maps |
| `hi2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `himmel` | ported | named in the sources or the handbook origin maps |
| `hl` | ported | named in the sources or the handbook origin maps |
| `homise` | absorbed | internal helper of the degree formatting of the tables |
| `hor_add` | absorbed | internal helper of the AAF reader and writer suite |
| `hor_art` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hor_farb` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `horbeg` | ported | named in the sources or the handbook origin maps |
| `horcom_aaf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `horcom_aaf1` | absorbed | internal helper of the AAF reader and writer suite |
| `horcom_aaf2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `horcom_aaf3` | absorbed | internal helper of the AAF reader and writer suite |
| `horcom_titel` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `horg` | ported | named in the sources or the handbook origin maps |
| `horg1` | ported | named in the sources or the handbook origin maps |
| `horg10` | ported | named in the sources or the handbook origin maps |
| `horg11` | ported | named in the sources or the handbook origin maps |
| `horg110` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `horg110_bes` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `horg11mult` | absorbed | internal helper of the MULTI directions |
| `horg11mult_1` | absorbed | internal helper of the MULTI directions |
| `horgt` | ported | named in the sources or the handbook origin maps |
| `horm2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hs_dop` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hsa0` | absorbed | internal helper of the coordinate table |
| `hub_anz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hub_auswert_mund` | absorbed | internal helper of the Dynamogramm curves |
| `hub_auswert_rad` | absorbed | internal helper of the Dynamogramm curves |
| `hub_tit` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hubausg` | absorbed | internal helper of the Dynamogramm curves |
| `hubephn` | ported | named in the sources or the handbook origin maps |
| `hubephp` | ported | named in the sources or the handbook origin maps |
| `huber` | ported | named in the sources or the handbook origin maps |
| `indim` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `iner` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box11` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box20` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box21` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box22` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box2_ds` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box3` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box4` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ingr_ort` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ingre` | absorbed | internal helper of the sign ingresses |
| `ingre1` | ported | named in the sources or the handbook origin maps |
| `ingrend` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `initpterm` | ported | named in the sources or the handbook origin maps |
| `input_grmise_ekl_aeq` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `input_grmise_zod` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inputbox` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ipol` | ported | named in the sources or the handbook origin maps |
| `ipol3` | ported | named in the sources or the handbook origin maps |
| `ivalin` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `jdplanetex_discr` | absorbed | internal helper of the statistics store and its writer |
| `jn` | ported | named in the sources or the handbook origin maps |
| `jok` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `jseckp` | ported | named in the sources or the handbook origin maps |
| `ju` | ported | named in the sources or the handbook origin maps |
| `jul_doll` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `jul_doll_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `juld` | ported | named in the sources or the handbook origin maps |
| `juld1` | ported | named in the sources or the handbook origin maps |
| `k5` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `k6` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `k7` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `kard_fix_gem` | ported | named in the sources or the handbook origin maps |
| `kard_fix_gem1` | absorbed | internal helper of the histogram drawing |
| `kard_fix_gemh` | ported | named in the sources or the handbook origin maps |
| `kepler` | ported | named in the sources or the handbook origin maps |
| `klplanz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `klsyt` | ported | named in the sources or the handbook origin maps |
| `kltext` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ko_ta` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ko_tab0` | absorbed | internal helper of the coordinate table |
| `ko_tab00` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ko_tr1` | ported | named in the sources or the handbook origin maps |
| `ko_tr2` | ported | named in the sources or the handbook origin maps |
| `koch` | ported | named in the sources or the handbook origin maps |
| `koch1` | ported | named in the sources or the handbook origin maps |
| `komm_les` | ported | named in the sources or the handbook origin maps |
| `komm_les1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `komma_pkt` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `kon_dhol` | ported | named in the sources or the handbook origin maps |
| `kon_dsp` | ported | named in the sources or the handbook origin maps |
| `konre` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `konsp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `korh` | ported | named in the sources or the handbook origin maps |
| `korr` | absorbed | internal helper of the Korrektur of the birth time |
| `korr0` | absorbed | internal helper of the Korrektur of the birth time |
| `korr1` | absorbed | internal helper of the Korrektur of the birth time |
| `korr10` | absorbed | internal helper of the Korrektur of the birth time |
| `korr2` | absorbed | internal helper of the Korrektur of the birth time |
| `korr21` | absorbed | internal helper of the Korrektur of the birth time |
| `kotab_sta` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `kotab_sta_l` | absorbed | internal helper of the statistics store and its writer |
| `kr` | ported | named in the sources or the handbook origin maps |
| `kreis` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `labr2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `lad_exe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `laender` | ported | named in the sources or the handbook origin maps |
| `land_List` | ported | named in the sources or the handbook origin maps |
| `le` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `leer` | ported | named in the sources or the handbook origin maps |
| `lese_text` | ported | named in the sources or the handbook origin maps |
| `lese_text_pr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `li` | ported | named in the sources or the handbook origin maps |
| `lin_inv` | ported | named in the sources or the handbook origin maps |
| `lin_min_ival` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `line` | ported | named in the sources or the handbook origin maps |
| `list_ausg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `list_ausg_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `list_ausg_ueb` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `list_zeil_loe` | absorbed | internal helper of record deletion in the collections browser |
| `list_zeil_loe1` | absorbed | internal helper of record deletion in the collections browser |
| `listscal` | absorbed | internal helper of the Grad-Liste |
| `lpkt` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `lt_ut` | absorbed | internal helper of the converter dialogs |
| `ma` | ported | named in the sources or the handbook origin maps |
| `maf_abk` | absorbed | internal helper of the AAF reader and writer suite |
| `maf_abkl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `maf_dspl_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `maf_dspl_par` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mainkont` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mainkont_dat_zeit` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_aaf` | ported | named in the sources or the handbook origin maps |
| `make_aaf_eing_horc` | ported | named in the sources or the handbook origin maps |
| `make_dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_dol` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_gg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_gl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_juld` | absorbed | internal helper of the AAF reader and writer suite |
| `make_namen` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_ort_horc_aaf` | absorbed | internal helper of the AAF reader and writer suite |
| `make_resdat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mark_jahr` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `mark_zeil` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `maxbreit` | ported | named in the sources or the handbook origin maps |
| `mc_armcb` | ported | named in the sources or the handbook origin maps |
| `mc_armcb1` | ported | named in the sources or the handbook origin maps |
| `md` | ported | named in the sources or the handbook origin maps |
| `md11` | ported | named in the sources or the handbook origin maps |
| `me` | ported | named in the sources or the handbook origin maps |
| `mehrf_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_1_mem` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_a` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_a_end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_a_ex` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_a_pr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_a_sc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_aus` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_aus_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `men2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `men3` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `merk` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `merk_o` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `merkeing` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `merkr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `merkr_o` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mess_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `message` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `minim_dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mo` | ported | named in the sources or the handbook origin maps |
| `mo_na` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mo_ta` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `moko` | ported | named in the sources or the handbook origin maps |
| `moko1` | ported | named in the sources or the handbook origin maps |
| `mondph` | absorbed | internal helper of the coordinate table |
| `mst` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mtrim` | ported | named in the sources or the handbook origin maps |
| `mtst` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mult_mult` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mult_rad` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mult_titel` | absorbed | internal helper of the MULTI directions |
| `multi0ost1` | ported | named in the sources or the handbook origin maps |
| `multi0west1` | ported | named in the sources or the handbook origin maps |
| `multi1` | absorbed | internal helper of the MULTI directions |
| `multi11` | ported | named in the sources or the handbook origin maps |
| `multi2` | absorbed | internal helper of the MULTI directions |
| `multi21` | ported | named in the sources or the handbook origin maps |
| `multi3` | absorbed | internal helper of the MULTI directions |
| `multi31` | ported | named in the sources or the handbook origin maps |
| `multi_arc` | absorbed | internal helper of the MULTI directions |
| `multiarc1` | ported | named in the sources or the handbook origin maps |
| `multiple` | ported | named in the sources or the handbook origin maps |
| `mund` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mund1` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `mundan` | ported | named in the sources or the handbook origin maps |
| `mundh1` | ported | named in the sources or the handbook origin maps |
| `mundhorh` | ported | named in the sources or the handbook origin maps |
| `mundhorp` | ported | named in the sources or the handbook origin maps |
| `mureh` | absorbed | internal helper of the MULTI directions |
| `mustere` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `musterend` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `nam` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ne` | ported | named in the sources or the handbook origin maps |
| `neu_voll` | ported | named in the sources or the handbook origin maps |
| `nochjul` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `notiz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `notiz_in` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `notvoidofcourse` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ns` | ported | named in the sources or the handbook origin maps |
| `null_fill` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `num_zuspl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `num_zuspl_hamb` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `numtag_mo` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `numw` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `numw1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `obj_nam` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `obj_w_ara` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `obj_wahl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `od_un` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `oeffne` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `oeffne20` | ported | named in the sources or the handbook origin maps |
| `oeffne3` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `oeffne_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `open` | ported | named in the sources or the handbook origin maps |
| `orb_gen_mund` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `orb_gen_rad` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `orbis_asp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `orbis_discr2` | ported | named in the sources or the handbook origin maps |
| `orbis_discr3` | ported | named in the sources or the handbook origin maps |
| `orbis_pla` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `org` | ported | named in the sources or the handbook origin maps |
| `ort` | ported | named in the sources or the handbook origin maps |
| `ort_ko` | absorbed | internal helper of the place search over his gazetteer |
| `ort_koord_diff` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ort_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ort_l1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ort_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ort_name_discr` | ported | named in the sources or the handbook origin maps |
| `ort_name_display` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ort_parall` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ort_wahl` | ported | named in the sources or the handbook origin maps |
| `ortg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ortgalp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ortp` | absorbed | internal helper of the place search over his gazetteer |
| `ortwandern` | absorbed | internal helper of the wandering walks |
| `ortwi` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `p_line` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pa` | ported | named in the sources or the handbook origin maps |
| `par` | ported | named in the sources or the handbook origin maps |
| `par_ap_ktr` | ported | named in the sources or the handbook origin maps |
| `param_sp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pboxn` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ph` | ported | named in the sources or the handbook origin maps |
| `pl` | ported | named in the sources or the handbook origin maps |
| `pl_el_ve` | absorbed | internal helper of the Pluto ephemeris and its velocities |
| `pl_h` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pl_h1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pl_ko` | ported | named in the sources or the handbook origin maps |
| `pl_praez` | ported | named in the sources or the handbook origin maps |
| `pl_vel` | ported | named in the sources or the handbook origin maps |
| `pl_vel1` | absorbed | internal helper of the Pluto ephemeris and its velocities |
| `pl_velpl` | absorbed | internal helper of the Pluto ephemeris and its velocities |
| `pl_velpl1` | absorbed | internal helper of the Pluto ephemeris and its velocities |
| `pl_ze` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plac` | ported | named in the sources or the handbook origin maps |
| `plac1` | ported | named in the sources or the handbook origin maps |
| `plac2` | ported | named in the sources or the handbook origin maps |
| `plan_alph` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plan_ds` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plan_wahl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `planp_dspl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plant` | ported | named in the sources or the handbook origin maps |
| `plant1` | ported | named in the sources or the handbook origin maps |
| `plant2` | absorbed | internal helper of the plant longitude search |
| `plant4` | absorbed | internal helper of the plant longitude search |
| `plant_anz` | absorbed | internal helper of the plant longitude search |
| `plant_e` | absorbed | internal helper of the plant longitude search |
| `plantex` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plantrans` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `planziff1` | absorbed | internal helper of the degree formatting of the tables |
| `planziff2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plein0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plein1` | ported | named in the sources or the handbook origin maps |
| `plein11` | ported | named in the sources or the handbook origin maps |
| `plein2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plele1` | ported | named in the sources or the handbook origin maps |
| `plelem` | ported | named in the sources or the handbook origin maps |
| `plelempl` | ported | named in the sources or the handbook origin maps |
| `plentz` | ported | named in the sources or the handbook origin maps |
| `plentz1` | ported | named in the sources or the handbook origin maps |
| `plentz10` | ported | named in the sources or the handbook origin maps |
| `plentz11` | ported | named in the sources or the handbook origin maps |
| `plentz2` | ported | named in the sources or the handbook origin maps |
| `plentz21` | ported | named in the sources or the handbook origin maps |
| `plentzf` | absorbed | internal helper of the degree formatting of the tables |
| `plg0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plgen11` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plgen12` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plgenkl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pline` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plinkl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plinv` | ported | named in the sources or the handbook origin maps |
| `plko` | ported | named in the sources or the handbook origin maps |
| `plko10` | ported | named in the sources or the handbook origin maps |
| `plko100` | ported | named in the sources or the handbook origin maps |
| `plko1001` | ported | named in the sources or the handbook origin maps |
| `plko12` | ported | named in the sources or the handbook origin maps |
| `plko_einz` | ported | named in the sources or the handbook origin maps |
| `plkoap` | ported | named in the sources or the handbook origin maps |
| `plkotr` | ported | named in the sources or the handbook origin maps |
| `pllreg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pllsp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plmk` | ported | named in the sources or the handbook origin maps |
| `plmk1` | absorbed | internal helper of the degree formatting of the tables |
| `plnm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plnms` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plposhi` | ported | named in the sources or the handbook origin maps |
| `plre` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plsort` | ported | named in the sources or the handbook origin maps |
| `plsyls1` | ported | named in the sources or the handbook origin maps |
| `plsyver` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pluto_ex` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `po` | ported | named in the sources or the handbook origin maps |
| `pos_cursor` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pr_enabel` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `praez` | ported | named in the sources or the handbook origin maps |
| `praez_kartes_aeqn` | ported | named in the sources or the handbook origin maps |
| `praez_kartes_ekln` | ported | named in the sources or the handbook origin maps |
| `prima` | ported | named in the sources or the handbook origin maps |
| `primhorg` | ported | named in the sources or the handbook origin maps |
| `print_font` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `prog_mode` | ported | named in the sources or the handbook origin maps |
| `proho` | ported | named in the sources or the handbook origin maps |
| `proho_haus` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `proho_planet` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pruef_format` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pruef_sperre` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ps` | ported | named in the sources or the handbook origin maps |
| `punkte_pla` | ported | named in the sources or the handbook origin maps |
| `putbm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `qu` | ported | named in the sources or the handbook origin maps |
| `quit` | ported | named in the sources or the handbook origin maps |
| `rahmen` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ran` | ported | named in the sources or the handbook origin maps |
| `raus` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `rausf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `re` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `readpterm` | ported | named in the sources or the handbook origin maps |
| `rechne` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `rechne1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `reg_resdat` | ported | named in the sources or the handbook origin maps |
| `regg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `regio` | ported | named in the sources or the handbook origin maps |
| `regio0` | ported | named in the sources or the handbook origin maps |
| `regio1` | ported | named in the sources or the handbook origin maps |
| `rer` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `richtung` | ported | named in the sources or the handbook origin maps |
| `rinstr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `rot_grn_line` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `sa` | ported | named in the sources or the handbook origin maps |
| `saub` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `sc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `scget` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `scput` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `scree` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `screen` | ported | named in the sources or the handbook origin maps |
| `scrg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `seite` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `setbackgmode` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `setf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `setmouse` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `setw` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `sg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `shift_item` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `shift_item_aaf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `sidt` | ported | named in the sources or the handbook origin maps |
| `skalh` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `skalh1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `skalh_gitter` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `skalv` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `sm` | ported | named in the sources or the handbook origin maps |
| `so` | ported | named in the sources or the handbook origin maps |
| `soko` | ported | named in the sources or the handbook origin maps |
| `sol_lun_tabelle` | absorbed | internal helper of the solar and lunar list outputs |
| `solnummer` | ported | named in the sources or the handbook origin maps |
| `solnummer1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `solnumplant` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `soltrim` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `sommerzeit` | ported | named in the sources or the handbook origin maps |
| `somo` | ported | named in the sources or the handbook origin maps |
| `sort` | ported | named in the sources or the handbook origin maps |
| `sort_pl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `sortl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `sp_line` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `spieg1` | ported | named in the sources or the handbook origin maps |
| `spieg2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `start_hardc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat1` | ported | named in the sources or the handbook origin maps |
| `stat1_0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat1_1` | absorbed | internal helper of the statistics store and its writer |
| `stat1_2` | absorbed | internal helper of the statistics store and its writer |
| `stat1_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat2` | ported | named in the sources or the handbook origin maps |
| `stat2_teil` | ported | named in the sources or the handbook origin maps |
| `stat2_zuord` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat2parl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat3` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat_ausg_ar` | ported | named in the sources or the handbook origin maps |
| `stat_ausw` | ported | named in the sources or the handbook origin maps |
| `stat_ausw_1` | ported | named in the sources or the handbook origin maps |
| `stat_auswh` | ported | named in the sources or the handbook origin maps |
| `stelaus` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stelk` | ported | named in the sources or the handbook origin maps |
| `stella` | ported | named in the sources or the handbook origin maps |
| `stelt` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stend` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stend1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stern` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stop` | ported | named in the sources or the handbook origin maps |
| `stope` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `suc_nam` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `such_wo` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `suchas` | ported | named in the sources or the handbook origin maps |
| `sum_z_h` | ported | named in the sources or the handbook origin maps |
| `symb_anz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `symb_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `syt` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ta` | ported | named in the sources or the handbook origin maps |
| `ta_na` | ported | named in the sources or the handbook origin maps |
| `ta_pro` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `tag_elim` | ported | named in the sources or the handbook origin maps |
| `taho` | ported | named in the sources or the handbook origin maps |
| `taho_proho_ini` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `text` | ported | named in the sources or the handbook origin maps |
| `text_light` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textc` | ported | named in the sources or the handbook origin maps |
| `textg` | ported | named in the sources or the handbook origin maps |
| `textr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textrc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textrl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `texts` | ported | named in the sources or the handbook origin maps |
| `textsy` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textzent` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textzentc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textzentr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textzentrl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `tinv` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `tinvg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `titlew` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `tm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `topo` | ported | named in the sources or the handbook origin maps |
| `topo1` | ported | named in the sources or the handbook origin maps |
| `tp` | ported | named in the sources or the handbook origin maps |
| `trim_wind` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `uhr` | ported | named in the sources or the handbook origin maps |
| `uhr_kon_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `uhr_par` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `uindo` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ur` | ported | named in the sources or the handbook origin maps |
| `urn` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `usuch` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ut_et` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ut_etd` | absorbed | internal helper of the converter dialogs |
| `ut_lt` | absorbed | internal helper of the converter dialogs |
| `utet` | ported | named in the sources or the handbook origin maps |
| `utet1` | ported | named in the sources or the handbook origin maps |
| `vchr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ve` | ported | named in the sources or the handbook origin maps |
| `vehlow` | ported | named in the sources or the handbook origin maps |
| `vel_om_pd` | ported | named in the sources or the handbook origin maps |
| `ver` | ported | named in the sources or the handbook origin maps |
| `ver1r` | ported | named in the sources or the handbook origin maps |
| `vergl1` | ported | named in the sources or the handbook origin maps |
| `vergl1r` | ported | named in the sources or the handbook origin maps |
| `vergl2` | ported | named in the sources or the handbook origin maps |
| `vergl2r` | ported | named in the sources or the handbook origin maps |
| `verv` | ported | named in the sources or the handbook origin maps |
| `vi` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `vs` | ported | named in the sources or the handbook origin maps |
| `vu` | ported | named in the sources or the handbook origin maps |
| `wahrso` | ported | named in the sources or the handbook origin maps |
| `wart` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `wart_erl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `wart_gem` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `wart_gem_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `wart_gem_1_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `warts` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `weit_dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `wert` | ported | named in the sources or the handbook origin maps |
| `win_reg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `winkel_eckp` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `wrkp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `xe` | ported | named in the sources or the handbook origin maps |
| `xk` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `xl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `xy` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `yk` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `yl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ze` | ported | named in the sources or the handbook origin maps |
| `ze_pl` | ported | named in the sources or the handbook origin maps |
| `ze_pl_wa` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeich_col` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeichp_dspl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeige_horoskop` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeilklick` | absorbed | internal helper of record deletion in the collections browser |
| `zein` | ported | named in the sources or the handbook origin maps |
| `zein1` | ported | named in the sources or the handbook origin maps |
| `zeit_angabe` | absorbed | internal helper of the plant longitude search |
| `zeit_discr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeit_form` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeit_gz` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `zeit_korr` | ported | named in the sources or the handbook origin maps |
| `zeit_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeiteing` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeitgleichung` | ported | named in the sources or the handbook origin maps |
| `zeitw` | absorbed | internal helper of the wandering walks |
| `zeitwi` | ported | named in the sources or the handbook origin maps |
| `zeitwim` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeitzon` | ported | named in the sources or the handbook origin maps |
| `zeitzon_nam_aaf` | ported | named in the sources or the handbook origin maps |
| `zeitzon_nam_horc` | ported | named in the sources or the handbook origin maps |
| `zesyls` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zod_zeich_alph` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zuo` | absorbed | internal helper of the place search over his gazetteer |
| `zuord1` | absorbed | internal helper of the place search over his gazetteer |
| `zuort` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zusp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zusp_mult` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zwihat` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `zwihau` | absorbed | internal helper of the symbolic and mundane direction evaluation |
