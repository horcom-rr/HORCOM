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

Counts: 701 ported by name, 63 absorbed, 329 screen era, 1093 total.

| original | status | account |
|---|---|---|
| `a1` | ported | named in the sources or the handbook origin maps |
| `a10` | ported | named in the sources or the handbook origin maps |
| `a10nk` | ported | named in the sources or the handbook origin maps |
| `a10re` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a11` | ported | named in the sources or the handbook origin maps |
| `a11_1` | ported | named in the sources or the handbook origin maps |
| `a11ex` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a11ex1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12` | ported | named in the sources or the handbook origin maps |
| `a12a` | ported | named in the sources or the handbook origin maps |
| `a12al` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12asp` | ported | named in the sources or the handbook origin maps |
| `a12beschr` | ported | named in the sources or the handbook origin maps |
| `a12beschr1` | ported | named in the sources or the handbook origin maps |
| `a12end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a12f` | ported | named in the sources or the handbook origin maps |
| `a12i` | ported | named in the sources or the handbook origin maps |
| `a12ia` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a13` | ported | named in the sources or the handbook origin maps |
| `a13aus` | ported | named in the sources or the handbook origin maps |
| `a13comp_1` | ported | named in the sources or the handbook origin maps |
| `a13comp_2` | ported | named in the sources or the handbook origin maps |
| `a13end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a14` | ported | named in the sources or the handbook origin maps |
| `a14_1` | ported | named in the sources or the handbook origin maps |
| `a14auscomb` | ported | named in the sources or the handbook origin maps |
| `a14end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a16` | ported | named in the sources or the handbook origin maps |
| `a16_1` | ported | named in the sources or the handbook origin maps |
| `a16_i` | absorbed | internal helper of the plant longitude search |
| `a16_ta` | ported | named in the sources or the handbook origin maps |
| `a16end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a16zang` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17` | ported | named in the sources or the handbook origin maps |
| `a170` | ported | named in the sources or the handbook origin maps |
| `a17000` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a1701` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a170_1tit` | ported | named in the sources or the handbook origin maps |
| `a171` | ported | named in the sources or the handbook origin maps |
| `a172` | ported | named in the sources or the handbook origin maps |
| `a1720` | ported | named in the sources or the handbook origin maps |
| `a173` | ported | named in the sources or the handbook origin maps |
| `a174` | ported | named in the sources or the handbook origin maps |
| `a174g` | ported | named in the sources or the handbook origin maps |
| `a174init` | ported | named in the sources or the handbook origin maps |
| `a175` | ported | named in the sources or the handbook origin maps |
| `a178` | ported | named in the sources or the handbook origin maps |
| `a1781` | ported | named in the sources or the handbook origin maps |
| `a1791` | ported | named in the sources or the handbook origin maps |
| `a17911` | ported | named in the sources or the handbook origin maps |
| `a1792` | ported | named in the sources or the handbook origin maps |
| `a1795` | ported | named in the sources or the handbook origin maps |
| `a17_3` | ported | named in the sources or the handbook origin maps |
| `a17_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17ach` | ported | named in the sources or the handbook origin maps |
| `a17dat` | ported | named in the sources or the handbook origin maps |
| `a17eing` | ported | named in the sources or the handbook origin maps |
| `a17eing11` | ported | named in the sources or the handbook origin maps |
| `a17eing12` | ported | named in the sources or the handbook origin maps |
| `a17end` | absorbed | internal helper of the rhythm walk, degree date list and Sonderpunkt |
| `a17in` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a17sol` | ported | named in the sources or the handbook origin maps |
| `a17sonderpkt` | ported | named in the sources or the handbook origin maps |
| `a17tab` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18` | ported | named in the sources or the handbook origin maps |
| `a180` | ported | named in the sources or the handbook origin maps |
| `a1800` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a180_1` | ported | named in the sources or the handbook origin maps |
| `a180a` | ported | named in the sources or the handbook origin maps |
| `a180aus` | ported | named in the sources or the handbook origin maps |
| `a180b` | ported | named in the sources or the handbook origin maps |
| `a180end1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a180iplv` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a180ival` | ported | named in the sources or the handbook origin maps |
| `a180sob1` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a180sob2` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a180trpr` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `a180trpr_lin` | ported | named in the sources or the handbook origin maps |
| `a181` | ported | named in the sources or the handbook origin maps |
| `a181_discr` | ported | named in the sources or the handbook origin maps |
| `a181t` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a181t1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a181tx` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a181tx1` | ported | named in the sources or the handbook origin maps |
| `a181ver` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18_3045` | ported | named in the sources or the handbook origin maps |
| `a18_entz` | ported | named in the sources or the handbook origin maps |
| `a18_lin` | ported | named in the sources or the handbook origin maps |
| `a18_line_col` | ported | named in the sources or the handbook origin maps |
| `a18asw` | ported | named in the sources or the handbook origin maps |
| `a18asw1` | ported | named in the sources or the handbook origin maps |
| `a18eing` | ported | named in the sources or the handbook origin maps |
| `a18eing_plw` | ported | named in the sources or the handbook origin maps |
| `a18end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18ko1` | ported | named in the sources or the handbook origin maps |
| `a18kopf` | ported | named in the sources or the handbook origin maps |
| `a18list` | ported | named in the sources or the handbook origin maps |
| `a18liv` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18livdir` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a18so` | ported | named in the sources or the handbook origin maps |
| `a18st` | ported | named in the sources or the handbook origin maps |
| `a18st0` | ported | named in the sources or the handbook origin maps |
| `a18tab` | ported | named in the sources or the handbook origin maps |
| `a19` | ported | named in the sources or the handbook origin maps |
| `a20` | ported | named in the sources or the handbook origin maps |
| `a200dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a200get` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a200loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a200loe_1` | absorbed | internal helper of the transit scan with the event place |
| `a200ort` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a201` | ported | named in the sources or the handbook origin maps |
| `a2011` | ported | named in the sources or the handbook origin maps |
| `a2011f` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a20_horg` | ported | named in the sources or the handbook origin maps |
| `a20end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a210` | ported | named in the sources or the handbook origin maps |
| `a211` | ported | named in the sources or the handbook origin maps |
| `a2111` | ported | named in the sources or the handbook origin maps |
| `a2111_i` | absorbed | internal helper of the sign ingresses |
| `a2111_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2113` | ported | named in the sources or the handbook origin maps |
| `a2113_1` | ported | named in the sources or the handbook origin maps |
| `a211_nnam` | ported | named in the sources or the handbook origin maps |
| `a221` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a22dat` | ported | named in the sources or the handbook origin maps |
| `a22ort` | ported | named in the sources or the handbook origin maps |
| `a22sta_anz` | ported | named in the sources or the handbook origin maps |
| `a22ueberschrb` | ported | named in the sources or the handbook origin maps |
| `a2_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2dat` | ported | named in the sources or the handbook origin maps |
| `a2end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2f_tr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2f_tr2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2f_tr_dat` | ported | named in the sources or the handbook origin maps |
| `a2f_tr_ort` | ported | named in the sources or the handbook origin maps |
| `a2f_weiter` | ported | named in the sources or the handbook origin maps |
| `a2fdat` | ported | named in the sources or the handbook origin maps |
| `a2fort` | ported | named in the sources or the handbook origin maps |
| `a2l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a2ort` | ported | named in the sources or the handbook origin maps |
| `a3` | ported | named in the sources or the handbook origin maps |
| `a31` | ported | named in the sources or the handbook origin maps |
| `a311` | ported | named in the sources or the handbook origin maps |
| `a311_o` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `a316061` | ported | named in the sources or the handbook origin maps |
| `a31_o` | ported | named in the sources or the handbook origin maps |
| `a37dat` | ported | named in the sources or the handbook origin maps |
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
| `a91` | ported | named in the sources or the handbook origin maps |
| `a92` | ported | named in the sources or the handbook origin maps |
| `a921` | ported | named in the sources or the handbook origin maps |
| `a93` | ported | named in the sources or the handbook origin maps |
| `a9zw` | ported | named in the sources or the handbook origin maps |
| `aaf_anzahl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_box` | ported | named in the sources or the handbook origin maps |
| `aaf_datei_oeffnen` | absorbed | internal helper of the AAF reader and writer suite |
| `aaf_edit_nr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_help` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_horcom` | ported | named in the sources or the handbook origin maps |
| `aaf_horcom0` | ported | named in the sources or the handbook origin maps |
| `aaf_horcom1` | absorbed | internal helper of the AAF reader and writer suite |
| `aaf_horcom2` | ported | named in the sources or the handbook origin maps |
| `aaf_horcom_end` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_horcom_wandeln` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aaf_ident` | ported | named in the sources or the handbook origin maps |
| `aaf_orteingabe` | ported | named in the sources or the handbook origin maps |
| `aaf_satz_add` | ported | named in the sources or the handbook origin maps |
| `aaf_satz_loesch` | ported | named in the sources or the handbook origin maps |
| `aaf_satz_speich` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aafbox_in_horcomdaten` | absorbed | internal helper of the AAF reader and writer suite |
| `aafdatei_anzahl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `abort_pr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `acmcl` | ported | named in the sources or the handbook origin maps |
| `ad` | ported | named in the sources or the handbook origin maps |
| `adjustwin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `adop` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae10` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae11` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae12` | ported | named in the sources or the handbook origin maps |
| `ae14` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ae2` | ported | named in the sources or the handbook origin maps |
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
| `afo` | ported | named in the sources or the handbook origin maps |
| `ag` | ported | named in the sources or the handbook origin maps |
| `alertbox` | ported | named in the sources or the handbook origin maps |
| `alerte` | ported | named in the sources or the handbook origin maps |
| `amundpr` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `amundprs` | absorbed | internal helper of the symbolic and mundane direction evaluation |
| `ansth` | absorbed | internal helper of the Dynamogramm curves |
| `anzahl_ein` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `anzeigen_asp_zaehl` | ported | named in the sources or the handbook origin maps |
| `anzeigen_halbs_zaehl` | ported | named in the sources or the handbook origin maps |
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
| `arab_eig` | ported | named in the sources or the handbook origin maps |
| `arabl` | absorbed | internal helper of the arabic parts |
| `arabl0` | ported | named in the sources or the handbook origin maps |
| `arabl1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arabso` | ported | named in the sources or the handbook origin maps |
| `arabt` | ported | named in the sources or the handbook origin maps |
| `arabte` | ported | named in the sources or the handbook origin maps |
| `arabtex` | ported | named in the sources or the handbook origin maps |
| `arb_dim` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arb_er` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `arde_eleb` | ported | named in the sources or the handbook origin maps |
| `areg` | ported | named in the sources or the handbook origin maps |
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
| `asp_analy_1_mund` | ported | named in the sources or the handbook origin maps |
| `asp_analy_1_rad` | ported | named in the sources or the handbook origin maps |
| `asp_analy_mund` | ported | named in the sources or the handbook origin maps |
| `asp_analy_rad` | ported | named in the sources or the handbook origin maps |
| `asp_ds` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `asp_halbs_zaehler` | ported | named in the sources or the handbook origin maps |
| `asp_li` | ported | named in the sources or the handbook origin maps |
| `asp_li_0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `asp_li_upd` | ported | named in the sources or the handbook origin maps |
| `aspar` | ported | named in the sources or the handbook origin maps |
| `aspar2` | ported | named in the sources or the handbook origin maps |
| `aspar2_ini` | ported | named in the sources or the handbook origin maps |
| `aspdis` | ported | named in the sources or the handbook origin maps |
| `asphi1` | ported | named in the sources or the handbook origin maps |
| `asphist` | ported | named in the sources or the handbook origin maps |
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
| `aspsenk` | ported | named in the sources or the handbook origin maps |
| `aspsenk1` | ported | named in the sources or the handbook origin maps |
| `aspssyver` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspwag` | ported | named in the sources or the handbook origin maps |
| `aspwag1` | ported | named in the sources or the handbook origin maps |
| `aspz0` | ported | named in the sources or the handbook origin maps |
| `aspz1` | ported | named in the sources or the handbook origin maps |
| `aspz1_0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspz1_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspz1_1_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `aspz1_2` | ported | named in the sources or the handbook origin maps |
| `asy111` | ported | named in the sources or the handbook origin maps |
| `asy112` | ported | named in the sources or the handbook origin maps |
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
| `auf_unt3` | ported | named in the sources or the handbook origin maps |
| `aufl_ziff` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ausw_datei` | ported | named in the sources or the handbook origin maps |
| `ausw_obj_e` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ausw_obj_m` | ported | named in the sources or the handbook origin maps |
| `ausw_pl_hs` | ported | named in the sources or the handbook origin maps |
| `ausw_pl_hs1` | ported | named in the sources or the handbook origin maps |
| `auswahl_flag` | ported | named in the sources or the handbook origin maps |
| `auswert_ortelist` | ported | named in the sources or the handbook origin maps |
| `avd` | ported | named in the sources or the handbook origin maps |
| `avd_lin` | ported | named in the sources or the handbook origin maps |
| `ave` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `avers` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `avg` | ported | named in the sources or the handbook origin maps |
| `avh` | ported | named in the sources or the handbook origin maps |
| `backg_col` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `balken` | ported | named in the sources or the handbook origin maps |
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
| `bes10` | ported | named in the sources or the handbook origin maps |
| `bes11` | ported | named in the sources or the handbook origin maps |
| `bes110` | ported | named in the sources or the handbook origin maps |
| `bes111` | ported | named in the sources or the handbook origin maps |
| `bes1110` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes111_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes11_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes11_11` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bes1_big` | ported | named in the sources or the handbook origin maps |
| `bes2` | ported | named in the sources or the handbook origin maps |
| `bes2_comp` | ported | named in the sources or the handbook origin maps |
| `bes_big_asp` | ported | named in the sources or the handbook origin maps |
| `bes_big_elem` | ported | named in the sources or the handbook origin maps |
| `bes_big_halbs` | ported | named in the sources or the handbook origin maps |
| `bes_big_haus` | ported | named in the sources or the handbook origin maps |
| `bes_big_kafige` | ported | named in the sources or the handbook origin maps |
| `bes_big_plan` | ported | named in the sources or the handbook origin maps |
| `bilde_aaffile` | ported | named in the sources or the handbook origin maps |
| `bilde_horcfile` | ported | named in the sources or the handbook origin maps |
| `bilder_loesch` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bildl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bildld` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bildst` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bl_anz` | ported | named in the sources or the handbook origin maps |
| `bls` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `bmp_color_pl` | ported | named in the sources or the handbook origin maps |
| `bmp_color_ze` | ported | named in the sources or the handbook origin maps |
| `bmp_load` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `boxn` | ported | named in the sources or the handbook origin maps |
| `calc_in` | ported | named in the sources or the handbook origin maps |
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
| `col_halbsel` | ported | named in the sources or the handbook origin maps |
| `col_norm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `col_zeich` | ported | named in the sources or the handbook origin maps |
| `color_dial` | ported | named in the sources or the handbook origin maps |
| `copy_expand_file` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `copy_file` | ported | named in the sources or the handbook origin maps |
| `cp` | ported | named in the sources or the handbook origin maps |
| `cu` | ported | named in the sources or the handbook origin maps |
| `da` | ported | named in the sources or the handbook origin maps |
| `das_anz` | ported | named in the sources or the handbook origin maps |
| `das_anz_odat` | absorbed | internal helper of the record display boxes |
| `dat` | ported | named in the sources or the handbook origin maps |
| `dat_discr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dat_jd` | ported | named in the sources or the handbook origin maps |
| `dat_jd1` | ported | named in the sources or the handbook origin maps |
| `datanz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `date_form` | ported | named in the sources or the handbook origin maps |
| `datei_pr` | ported | named in the sources or the handbook origin maps |
| `datensatz_maxanz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `daterw` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `datru` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dats_einz_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dats_l` | ported | named in the sources or the handbook origin maps |
| `dauert` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `day_w` | ported | named in the sources or the handbook origin maps |
| `day_y` | ported | named in the sources or the handbook origin maps |
| `def_but` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `deffi` | ported | named in the sources or the handbook origin maps |
| `defm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `deftextcol` | ported | named in the sources or the handbook origin maps |
| `delblackmoon` | ported | named in the sources or the handbook origin maps |
| `desct` | ported | named in the sources or the handbook origin maps |
| `dial_close` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dial_ini` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dial_shift` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dif_vg` | ported | named in the sources or the handbook origin maps |
| `DIM` | ported | named in the sources or the handbook origin maps |
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
| `dirend` | ported | named in the sources or the handbook origin maps |
| `dirstop` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `disablewin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `discf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `disckill` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `diso` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dk` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dopp_haus_i` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dr` | ported | named in the sources or the handbook origin maps |
| `dr_ds_halbsum` | absorbed | internal helper of the midpoint scan and the composite rule |
| `dr_fehl` | ported | named in the sources or the handbook origin maps |
| `drad2` | ported | named in the sources or the handbook origin maps |
| `drad2_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dradst` | ported | named in the sources or the handbook origin maps |
| `dragline_y` | ported | named in the sources or the handbook origin maps |
| `dragline_y0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dragline_y1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `dragline_yc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `druck_einr_anz` | ported | named in the sources or the handbook origin maps |
| `druck_enbl_alt` | ported | named in the sources or the handbook origin maps |
| `druck_graph_ein` | ported | named in the sources or the handbook origin maps |
| `druck_horm` | ported | named in the sources or the handbook origin maps |
| `druck_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ds` | ported | named in the sources or the handbook origin maps |
| `dummy` | ported | named in the sources or the handbook origin maps |
| `eckp` | ported | named in the sources or the handbook origin maps |
| `eckp1` | ported | named in the sources or the handbook origin maps |
| `ein_zuo` | absorbed | internal helper of the place search over his gazetteer |
| `eing_box` | ported | named in the sources or the handbook origin maps |
| `eing_ti` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `eingabe` | ported | named in the sources or the handbook origin maps |
| `eingalp` | ported | named in the sources or the handbook origin maps |
| `eingfl_l` | ported | named in the sources or the handbook origin maps |
| `einzel_bogen_anz` | ported | named in the sources or the handbook origin maps |
| `einzel_plan_display` | ported | named in the sources or the handbook origin maps |
| `einzel_plan_wahl` | ported | named in the sources or the handbook origin maps |
| `einzel_plan_wahl1` | ported | named in the sources or the handbook origin maps |
| `eleb_arde` | ported | named in the sources or the handbook origin maps |
| `elem1` | ported | named in the sources or the handbook origin maps |
| `elem2` | ported | named in the sources or the handbook origin maps |
| `elem_col` | ported | named in the sources or the handbook origin maps |
| `elemhist1` | ported | named in the sources or the handbook origin maps |
| `enablewin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `enablw` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ende` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `entf_min_max` | ported | named in the sources or the handbook origin maps |
| `ephem_auswert` | ported | named in the sources or the handbook origin maps |
| `er_lin` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `era17_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `era17_2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `erasep` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `erdim` | absorbed | internal helper of the coordinate table |
| `erdime` | absorbed | internal helper of the coordinate table |
| `erdir` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ereig_ort` | ported | named in the sources or the handbook origin maps |
| `erg_rad` | ported | named in the sources or the handbook origin maps |
| `erso` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ersodir` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `erste_hilfe` | ported | named in the sources or the handbook origin maps |
| `et_l` | ported | named in the sources or the handbook origin maps |
| `et_reg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `et_sp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `et_ut` | ported | named in the sources or the handbook origin maps |
| `et_ut_erl` | ported | named in the sources or the handbook origin maps |
| `etm_ut` | ported | named in the sources or the handbook origin maps |
| `etut` | ported | named in the sources or the handbook origin maps |
| `fanz` | ported | named in the sources or the handbook origin maps |
| `fbox` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `fehler` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `fill_box` | ported | named in the sources or the handbook origin maps |
| `fill_color` | ported | named in the sources or the handbook origin maps |
| `finst` | ported | named in the sources or the handbook origin maps |
| `finst_0` | ported | named in the sources or the handbook origin maps |
| `finst_1` | ported | named in the sources or the handbook origin maps |
| `finst_a` | ported | named in the sources or the handbook origin maps |
| `finst_e` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `finst_s` | ported | named in the sources or the handbook origin maps |
| `fixp_def` | absorbed | internal helper of the fixed point of slot zero |
| `fixp_sperr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `fixpunkt_anz` | ported | named in the sources or the handbook origin maps |
| `fixpunkt_def` | ported | named in the sources or the handbook origin maps |
| `fixpunkt_def_a12` | absorbed | internal helper of the fixed point of slot zero |
| `fixpunkt_def_mult` | absorbed | internal helper of the fixed point of slot zero |
| `freebmp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `funkt` | ported | named in the sources or the handbook origin maps |
| `geb_herr` | ported | named in the sources or the handbook origin maps |
| `genau` | ported | named in the sources or the handbook origin maps |
| `geohelio` | ported | named in the sources or the handbook origin maps |
| `gesp_neu` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `get_aufl_bs` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `get_aufl_pr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `getbm` | ported | named in the sources or the handbook origin maps |
| `GetBmpSize` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `gettext` | ported | named in the sources or the handbook origin maps |
| `gettextaaf` | ported | named in the sources or the handbook origin maps |
| `gettexti` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `gl` | ported | named in the sources or the handbook origin maps |
| `glanz` | ported | named in the sources or the handbook origin maps |
| `gm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `greg_doll` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `greg_doll_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `grli` | ported | named in the sources or the handbook origin maps |
| `grli1` | ported | named in the sources or the handbook origin maps |
| `grli_f` | ported | named in the sources or the handbook origin maps |
| `grlinit` | ported | named in the sources or the handbook origin maps |
| `grmise` | ported | named in the sources or the handbook origin maps |
| `grossj` | ported | named in the sources or the handbook origin maps |
| `grossj1` | ported | named in the sources or the handbook origin maps |
| `grundaspekt` | ported | named in the sources or the handbook origin maps |
| `grze` | ported | named in the sources or the handbook origin maps |
| `grze_0` | ported | named in the sources or the handbook origin maps |
| `grzemise` | ported | named in the sources or the handbook origin maps |
| `ha` | ported | named in the sources or the handbook origin maps |
| `habes` | ported | named in the sources or the handbook origin maps |
| `haeuser_pruef` | ported | named in the sources or the handbook origin maps |
| `halbs1` | ported | named in the sources or the handbook origin maps |
| `halbs11` | ported | named in the sources or the handbook origin maps |
| `halbs111` | ported | named in the sources or the handbook origin maps |
| `halbs_dial` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `halbs_ruecksetz` | ported | named in the sources or the handbook origin maps |
| `halbs_zaehl_gr` | ported | named in the sources or the handbook origin maps |
| `halbs_zaehler` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `halbsa` | ported | named in the sources or the handbook origin maps |
| `halbsm` | ported | named in the sources or the handbook origin maps |
| `halbsmin` | ported | named in the sources or the handbook origin maps |
| `halbszaus` | ported | named in the sources or the handbook origin maps |
| `handlemessaaf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `handlemessage` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `handlemessagei` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hardc_kompl` | ported | named in the sources or the handbook origin maps |
| `hardcopy` | ported | named in the sources or the handbook origin maps |
| `harm` | ported | named in the sources or the handbook origin maps |
| `harm21` | ported | named in the sources or the handbook origin maps |
| `haus_ber` | ported | named in the sources or the handbook origin maps |
| `haus_def` | ported | named in the sources or the handbook origin maps |
| `haus_sel` | ported | named in the sources or the handbook origin maps |
| `hausa` | ported | named in the sources or the handbook origin maps |
| `haust` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `haustab` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hausw` | ported | named in the sources or the handbook origin maps |
| `hd_hs` | ported | named in the sources or the handbook origin maps |
| `hel_geo` | ported | named in the sources or the handbook origin maps |
| `hi2` | ported | named in the sources or the handbook origin maps |
| `himmel` | ported | named in the sources or the handbook origin maps |
| `hl` | ported | named in the sources or the handbook origin maps |
| `homise` | ported | named in the sources or the handbook origin maps |
| `hor_add` | absorbed | internal helper of the AAF reader and writer suite |
| `hor_art` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hor_farb` | ported | named in the sources or the handbook origin maps |
| `horbeg` | ported | named in the sources or the handbook origin maps |
| `horcom_aaf` | ported | named in the sources or the handbook origin maps |
| `horcom_aaf1` | absorbed | internal helper of the AAF reader and writer suite |
| `horcom_aaf2` | ported | named in the sources or the handbook origin maps |
| `horcom_aaf3` | ported | named in the sources or the handbook origin maps |
| `horcom_titel` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `horg` | ported | named in the sources or the handbook origin maps |
| `horg1` | ported | named in the sources or the handbook origin maps |
| `horg10` | ported | named in the sources or the handbook origin maps |
| `horg11` | ported | named in the sources or the handbook origin maps |
| `horg110` | ported | named in the sources or the handbook origin maps |
| `horg110_bes` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `horg11mult` | ported | named in the sources or the handbook origin maps |
| `horg11mult_1` | ported | named in the sources or the handbook origin maps |
| `horgt` | ported | named in the sources or the handbook origin maps |
| `horm2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hs_dop` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hsa0` | ported | named in the sources or the handbook origin maps |
| `hub_anz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `hub_auswert_mund` | absorbed | internal helper of the Dynamogramm curves |
| `hub_auswert_rad` | ported | named in the sources or the handbook origin maps |
| `hub_tit` | ported | named in the sources or the handbook origin maps |
| `hubausg` | ported | named in the sources or the handbook origin maps |
| `hubephn` | ported | named in the sources or the handbook origin maps |
| `hubephp` | ported | named in the sources or the handbook origin maps |
| `huber` | ported | named in the sources or the handbook origin maps |
| `indim` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `iner` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `inf_box1` | ported | named in the sources or the handbook origin maps |
| `inf_box11` | ported | named in the sources or the handbook origin maps |
| `inf_box2` | ported | named in the sources or the handbook origin maps |
| `inf_box20` | ported | named in the sources or the handbook origin maps |
| `inf_box21` | ported | named in the sources or the handbook origin maps |
| `inf_box22` | ported | named in the sources or the handbook origin maps |
| `inf_box2_ds` | ported | named in the sources or the handbook origin maps |
| `inf_box3` | ported | named in the sources or the handbook origin maps |
| `inf_box4` | ported | named in the sources or the handbook origin maps |
| `ingr_ort` | ported | named in the sources or the handbook origin maps |
| `ingre` | ported | named in the sources or the handbook origin maps |
| `ingre1` | ported | named in the sources or the handbook origin maps |
| `ingrend` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `initpterm` | ported | named in the sources or the handbook origin maps |
| `input_grmise_ekl_aeq` | ported | named in the sources or the handbook origin maps |
| `input_grmise_zod` | ported | named in the sources or the handbook origin maps |
| `inputbox` | ported | named in the sources or the handbook origin maps |
| `ipol` | ported | named in the sources or the handbook origin maps |
| `ipol3` | ported | named in the sources or the handbook origin maps |
| `ivalin` | absorbed | internal helper of the direction evaluation drivers and the Linear-Graphik |
| `jdplanetex_discr` | absorbed | internal helper of the statistics store and its writer |
| `jn` | ported | named in the sources or the handbook origin maps |
| `jok` | ported | named in the sources or the handbook origin maps |
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
| `kard_fix_gem1` | ported | named in the sources or the handbook origin maps |
| `kard_fix_gemh` | ported | named in the sources or the handbook origin maps |
| `kepler` | ported | named in the sources or the handbook origin maps |
| `klplanz` | ported | named in the sources or the handbook origin maps |
| `klsyt` | ported | named in the sources or the handbook origin maps |
| `kltext` | ported | named in the sources or the handbook origin maps |
| `ko_ta` | ported | named in the sources or the handbook origin maps |
| `ko_tab0` | ported | named in the sources or the handbook origin maps |
| `ko_tab00` | ported | named in the sources or the handbook origin maps |
| `ko_tr1` | ported | named in the sources or the handbook origin maps |
| `ko_tr2` | ported | named in the sources or the handbook origin maps |
| `koch` | ported | named in the sources or the handbook origin maps |
| `koch1` | ported | named in the sources or the handbook origin maps |
| `komm_les` | ported | named in the sources or the handbook origin maps |
| `komm_les1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `komma_pkt` | ported | named in the sources or the handbook origin maps |
| `kon_dhol` | ported | named in the sources or the handbook origin maps |
| `kon_dsp` | ported | named in the sources or the handbook origin maps |
| `konre` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `konsp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `korh` | ported | named in the sources or the handbook origin maps |
| `korr` | ported | named in the sources or the handbook origin maps |
| `korr0` | ported | named in the sources or the handbook origin maps |
| `korr1` | ported | named in the sources or the handbook origin maps |
| `korr10` | ported | named in the sources or the handbook origin maps |
| `korr2` | ported | named in the sources or the handbook origin maps |
| `korr21` | ported | named in the sources or the handbook origin maps |
| `kotab_sta` | ported | named in the sources or the handbook origin maps |
| `kotab_sta_l` | absorbed | internal helper of the statistics store and its writer |
| `kr` | ported | named in the sources or the handbook origin maps |
| `kreis` | ported | named in the sources or the handbook origin maps |
| `labr2` | ported | named in the sources or the handbook origin maps |
| `lad_exe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `laender` | ported | named in the sources or the handbook origin maps |
| `land_List` | ported | named in the sources or the handbook origin maps |
| `le` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `leer` | absorbed | internal helper of the house systems, the Keine system with its empty cusps |
| `lese_text` | ported | named in the sources or the handbook origin maps |
| `lese_text_pr` | ported | named in the sources or the handbook origin maps |
| `li` | ported | named in the sources or the handbook origin maps |
| `lin_inv` | ported | named in the sources or the handbook origin maps |
| `lin_min_ival` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `line` | ported | named in the sources or the handbook origin maps |
| `list_ausg` | ported | named in the sources or the handbook origin maps |
| `list_ausg_1` | ported | named in the sources or the handbook origin maps |
| `list_ausg_ueb` | ported | named in the sources or the handbook origin maps |
| `list_zeil_loe` | ported | named in the sources or the handbook origin maps |
| `list_zeil_loe1` | absorbed | internal helper of record deletion in the collections browser |
| `listscal` | ported | named in the sources or the handbook origin maps |
| `lpkt` | ported | named in the sources or the handbook origin maps |
| `lt_ut` | ported | named in the sources or the handbook origin maps |
| `ma` | ported | named in the sources or the handbook origin maps |
| `maf_abk` | absorbed | internal helper of the AAF reader and writer suite |
| `maf_abkl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `maf_dspl_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `maf_dspl_par` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mainkont` | ported | named in the sources or the handbook origin maps |
| `mainkont_dat_zeit` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_aaf` | ported | named in the sources or the handbook origin maps |
| `make_aaf_eing_horc` | ported | named in the sources or the handbook origin maps |
| `make_dat` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_dol` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_gg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_gl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_juld` | absorbed | internal helper of the AAF reader and writer suite |
| `make_namen` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `make_ort_horc_aaf` | ported | named in the sources or the handbook origin maps |
| `make_resdat` | ported | named in the sources or the handbook origin maps |
| `mark_jahr` | ported | named in the sources or the handbook origin maps |
| `mark_zeil` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `maxbreit` | ported | named in the sources or the handbook origin maps |
| `mc_armcb` | ported | named in the sources or the handbook origin maps |
| `mc_armcb1` | ported | named in the sources or the handbook origin maps |
| `md` | ported | named in the sources or the handbook origin maps |
| `md11` | ported | named in the sources or the handbook origin maps |
| `me` | ported | named in the sources or the handbook origin maps |
| `mehrf_1` | ported | named in the sources or the handbook origin maps |
| `mehrf_1_mem` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_a` | ported | named in the sources or the handbook origin maps |
| `mehrf_a_end` | ported | named in the sources or the handbook origin maps |
| `mehrf_a_ex` | ported | named in the sources or the handbook origin maps |
| `mehrf_a_pr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_a_sc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mehrf_aus` | ported | named in the sources or the handbook origin maps |
| `mehrf_aus_1` | ported | named in the sources or the handbook origin maps |
| `men2` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `men3` | ported | named in the sources or the handbook origin maps |
| `merk` | ported | named in the sources or the handbook origin maps |
| `merk_o` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `merkeing` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `merkr` | ported | named in the sources or the handbook origin maps |
| `merkr_o` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mess_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `message` | ported | named in the sources or the handbook origin maps |
| `minim_dat` | ported | named in the sources or the handbook origin maps |
| `mo` | ported | named in the sources or the handbook origin maps |
| `mo_na` | ported | named in the sources or the handbook origin maps |
| `mo_ta` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `moko` | ported | named in the sources or the handbook origin maps |
| `moko1` | ported | named in the sources or the handbook origin maps |
| `mondph` | ported | named in the sources or the handbook origin maps |
| `mst` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mtrim` | ported | named in the sources or the handbook origin maps |
| `mtst` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `mult_mult` | ported | named in the sources or the handbook origin maps |
| `mult_rad` | ported | named in the sources or the handbook origin maps |
| `mult_titel` | absorbed | internal helper of the MULTI directions |
| `multi0ost1` | ported | named in the sources or the handbook origin maps |
| `multi0west1` | ported | named in the sources or the handbook origin maps |
| `multi1` | ported | named in the sources or the handbook origin maps |
| `multi11` | ported | named in the sources or the handbook origin maps |
| `multi2` | absorbed | internal helper of the MULTI directions |
| `multi21` | ported | named in the sources or the handbook origin maps |
| `multi3` | ported | named in the sources or the handbook origin maps |
| `multi31` | ported | named in the sources or the handbook origin maps |
| `multi_arc` | ported | named in the sources or the handbook origin maps |
| `multiarc1` | ported | named in the sources or the handbook origin maps |
| `MULTINULLOST` | absorbed | internal helper of the MULTI directions |
| `MULTINULLWEST` | ported | named in the sources or the handbook origin maps |
| `multiple` | ported | named in the sources or the handbook origin maps |
| `mund` | ported | named in the sources or the handbook origin maps |
| `mund1` | ported | named in the sources or the handbook origin maps |
| `mundan` | ported | named in the sources or the handbook origin maps |
| `mundh1` | ported | named in the sources or the handbook origin maps |
| `mundhorh` | ported | named in the sources or the handbook origin maps |
| `mundhorp` | ported | named in the sources or the handbook origin maps |
| `mureh` | absorbed | internal helper of the MULTI directions |
| `mustere` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `musterend` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `nam` | ported | named in the sources or the handbook origin maps |
| `ne` | ported | named in the sources or the handbook origin maps |
| `neu_voll` | ported | named in the sources or the handbook origin maps |
| `nochjul` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `notiz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `notiz_in` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `notvoidofcourse` | ported | named in the sources or the handbook origin maps |
| `ns` | ported | named in the sources or the handbook origin maps |
| `null_fill` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `num_zuspl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `num_zuspl_hamb` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `numtag_mo` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `numw` | ported | named in the sources or the handbook origin maps |
| `numw1` | ported | named in the sources or the handbook origin maps |
| `obj_nam` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `obj_w_ara` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `obj_wahl` | ported | named in the sources or the handbook origin maps |
| `od_un` | ported | named in the sources or the handbook origin maps |
| `oeffne` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `oeffne20` | ported | named in the sources or the handbook origin maps |
| `oeffne3` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `oeffne_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `open` | ported | named in the sources or the handbook origin maps |
| `orb_gen_mund` | ported | named in the sources or the handbook origin maps |
| `orb_gen_rad` | ported | named in the sources or the handbook origin maps |
| `orbis_asp` | ported | named in the sources or the handbook origin maps |
| `orbis_discr2` | ported | named in the sources or the handbook origin maps |
| `orbis_discr3` | ported | named in the sources or the handbook origin maps |
| `orbis_pla` | ported | named in the sources or the handbook origin maps |
| `org` | ported | named in the sources or the handbook origin maps |
| `ort` | ported | named in the sources or the handbook origin maps |
| `ort_ko` | absorbed | internal helper of the place search over his gazetteer |
| `ort_koord_diff` | ported | named in the sources or the handbook origin maps |
| `ort_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ort_l1` | ported | named in the sources or the handbook origin maps |
| `ort_loe` | ported | named in the sources or the handbook origin maps |
| `ort_name_discr` | ported | named in the sources or the handbook origin maps |
| `ort_name_display` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ort_parall` | ported | named in the sources or the handbook origin maps |
| `ort_wahl` | ported | named in the sources or the handbook origin maps |
| `ortg` | ported | named in the sources or the handbook origin maps |
| `ortgalp` | ported | named in the sources or the handbook origin maps |
| `ortp` | ported | named in the sources or the handbook origin maps |
| `ortwandern` | ported | named in the sources or the handbook origin maps |
| `ortwi` | ported | named in the sources or the handbook origin maps |
| `p_line` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pa` | ported | named in the sources or the handbook origin maps |
| `PaletteRestore` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `PaletteSave` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `par` | ported | named in the sources or the handbook origin maps |
| `par_ap_ktr` | ported | named in the sources or the handbook origin maps |
| `param_sp` | ported | named in the sources or the handbook origin maps |
| `pboxn` | ported | named in the sources or the handbook origin maps |
| `ph` | ported | named in the sources or the handbook origin maps |
| `pl` | ported | named in the sources or the handbook origin maps |
| `pl_el_ve` | absorbed | internal helper of the Pluto ephemeris and its velocities |
| `pl_h` | ported | named in the sources or the handbook origin maps |
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
| `plan_ds` | ported | named in the sources or the handbook origin maps |
| `plan_wahl` | ported | named in the sources or the handbook origin maps |
| `planp_dspl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plant` | ported | named in the sources or the handbook origin maps |
| `plant1` | ported | named in the sources or the handbook origin maps |
| `plant2` | absorbed | internal helper of the plant longitude search |
| `plant4` | absorbed | internal helper of the plant longitude search |
| `plant_anz` | absorbed | internal helper of the plant longitude search |
| `plant_e` | absorbed | internal helper of the plant longitude search |
| `plantex` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plantrans` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `planziff1` | ported | named in the sources or the handbook origin maps |
| `planziff2` | ported | named in the sources or the handbook origin maps |
| `plein0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plein1` | ported | named in the sources or the handbook origin maps |
| `plein11` | ported | named in the sources or the handbook origin maps |
| `plein2` | ported | named in the sources or the handbook origin maps |
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
| `plnm` | ported | named in the sources or the handbook origin maps |
| `plnms` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `plposhi` | ported | named in the sources or the handbook origin maps |
| `plre` | ported | named in the sources or the handbook origin maps |
| `plsort` | ported | named in the sources or the handbook origin maps |
| `plsyls1` | ported | named in the sources or the handbook origin maps |
| `plsyver` | ported | named in the sources or the handbook origin maps |
| `pluto_ex` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `po` | ported | named in the sources or the handbook origin maps |
| `pos_cursor` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pr_enabel` | ported | named in the sources or the handbook origin maps |
| `praez` | ported | named in the sources or the handbook origin maps |
| `praez_kartes_aeqn` | ported | named in the sources or the handbook origin maps |
| `praez_kartes_ekln` | ported | named in the sources or the handbook origin maps |
| `prima` | ported | named in the sources or the handbook origin maps |
| `primhorg` | ported | named in the sources or the handbook origin maps |
| `print_font` | ported | named in the sources or the handbook origin maps |
| `prog_mode` | ported | named in the sources or the handbook origin maps |
| `proho` | ported | named in the sources or the handbook origin maps |
| `proho_haus` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `proho_planet` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pruef_format` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `pruef_sperre` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ps` | ported | named in the sources or the handbook origin maps |
| `punkte_pla` | ported | named in the sources or the handbook origin maps |
| `putbm` | ported | named in the sources or the handbook origin maps |
| `qu` | ported | named in the sources or the handbook origin maps |
| `quit` | ported | named in the sources or the handbook origin maps |
| `rahmen` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ran` | ported | named in the sources or the handbook origin maps |
| `raus` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `rausf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `re` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `readpterm` | ported | named in the sources or the handbook origin maps |
| `rechne` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `rechne1` | ported | named in the sources or the handbook origin maps |
| `reg_resdat` | absorbed | the RRESERVE.DAT rescue, superseded by the atomic writes of the data layer |
| `regg` | ported | named in the sources or the handbook origin maps |
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
| `seite` | ported | named in the sources or the handbook origin maps |
| `setbackgmode` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `setf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `setmouse` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `setw` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `sg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `shift_item` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `shift_item_aaf` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `sidt` | ported | named in the sources or the handbook origin maps |
| `skalh` | ported | named in the sources or the handbook origin maps |
| `skalh1` | ported | named in the sources or the handbook origin maps |
| `skalh_gitter` | ported | named in the sources or the handbook origin maps |
| `skalv` | ported | named in the sources or the handbook origin maps |
| `sm` | ported | named in the sources or the handbook origin maps |
| `so` | ported | named in the sources or the handbook origin maps |
| `soko` | ported | named in the sources or the handbook origin maps |
| `sol_lun_tabelle` | ported | named in the sources or the handbook origin maps |
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
| `start_hardc` | ported | named in the sources or the handbook origin maps |
| `stat` | ported | named in the sources or the handbook origin maps |
| `stat1` | ported | named in the sources or the handbook origin maps |
| `stat1_0` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat1_1` | absorbed | internal helper of the statistics store and its writer |
| `stat1_2` | absorbed | internal helper of the statistics store and its writer |
| `stat1_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat2` | ported | named in the sources or the handbook origin maps |
| `stat2_teil` | ported | named in the sources or the handbook origin maps |
| `stat2_zuord` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stat2parl` | ported | named in the sources or the handbook origin maps |
| `stat3` | ported | named in the sources or the handbook origin maps |
| `stat_ausg_ar` | ported | named in the sources or the handbook origin maps |
| `stat_ausw` | ported | named in the sources or the handbook origin maps |
| `stat_ausw_1` | ported | named in the sources or the handbook origin maps |
| `stat_auswh` | ported | named in the sources or the handbook origin maps |
| `stelaus` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stelk` | ported | named in the sources or the handbook origin maps |
| `stella` | ported | named in the sources or the handbook origin maps |
| `stelt` | ported | named in the sources or the handbook origin maps |
| `stend` | ported | named in the sources or the handbook origin maps |
| `stend1` | ported | named in the sources or the handbook origin maps |
| `stern` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `stop` | ported | named in the sources or the handbook origin maps |
| `stope` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `suc_nam` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `such_wo` | ported | named in the sources or the handbook origin maps |
| `suchas` | ported | named in the sources or the handbook origin maps |
| `sum_z_h` | ported | named in the sources or the handbook origin maps |
| `symb_anz` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `symb_loe` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `Symbhol` | ported | named in the sources or the handbook origin maps |
| `syt` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ta` | ported | named in the sources or the handbook origin maps |
| `ta_na` | ported | named in the sources or the handbook origin maps |
| `ta_pro` | ported | named in the sources or the handbook origin maps |
| `tag_elim` | ported | named in the sources or the handbook origin maps |
| `taho` | ported | named in the sources or the handbook origin maps |
| `taho_proho_ini` | ported | named in the sources or the handbook origin maps |
| `text` | ported | named in the sources or the handbook origin maps |
| `text_light` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textc` | ported | named in the sources or the handbook origin maps |
| `textg` | ported | named in the sources or the handbook origin maps |
| `textr` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textrc` | ported | named in the sources or the handbook origin maps |
| `textrl` | ported | named in the sources or the handbook origin maps |
| `texts` | ported | named in the sources or the handbook origin maps |
| `textsy` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textzent` | ported | named in the sources or the handbook origin maps |
| `textzentc` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `textzentr` | ported | named in the sources or the handbook origin maps |
| `textzentrl` | ported | named in the sources or the handbook origin maps |
| `tinv` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `tinvg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `titlew` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `tm` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `topo` | ported | named in the sources or the handbook origin maps |
| `topo1` | ported | named in the sources or the handbook origin maps |
| `tp` | ported | named in the sources or the handbook origin maps |
| `trim_wind` | ported | named in the sources or the handbook origin maps |
| `uhr` | ported | named in the sources or the handbook origin maps |
| `uhr_kon_l` | ported | named in the sources or the handbook origin maps |
| `uhr_par` | ported | named in the sources or the handbook origin maps |
| `uindo` | ported | named in the sources or the handbook origin maps |
| `ur` | ported | named in the sources or the handbook origin maps |
| `urn` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `usuch` | ported | named in the sources or the handbook origin maps |
| `ut_et` | ported | named in the sources or the handbook origin maps |
| `ut_etd` | ported | named in the sources or the handbook origin maps |
| `ut_lt` | ported | named in the sources or the handbook origin maps |
| `utet` | ported | named in the sources or the handbook origin maps |
| `utet1` | ported | named in the sources or the handbook origin maps |
| `VAL` | ported | named in the sources or the handbook origin maps |
| `vchr` | ported | named in the sources or the handbook origin maps |
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
| `wart` | ported | named in the sources or the handbook origin maps |
| `wart_erl` | ported | named in the sources or the handbook origin maps |
| `wart_gem` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `wart_gem_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `wart_gem_1_1` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `warts` | ported | named in the sources or the handbook origin maps |
| `weit_dat` | ported | named in the sources or the handbook origin maps |
| `wert` | ported | named in the sources or the handbook origin maps |
| `win_reg` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `winkel_eckp` | ported | named in the sources or the handbook origin maps |
| `wrkp` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `xe` | ported | named in the sources or the handbook origin maps |
| `xk` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `xl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `xy` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `yk` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `yl` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `ze` | ported | named in the sources or the handbook origin maps |
| `ze_pl` | ported | named in the sources or the handbook origin maps |
| `ze_pl_wa` | ported | named in the sources or the handbook origin maps |
| `zeich_col` | ported | named in the sources or the handbook origin maps |
| `zeichp_dspl` | ported | named in the sources or the handbook origin maps |
| `zeige_horoskop` | ported | named in the sources or the handbook origin maps |
| `zeilklick` | ported | named in the sources or the handbook origin maps |
| `zein` | ported | named in the sources or the handbook origin maps |
| `zein1` | ported | named in the sources or the handbook origin maps |
| `zeit_angabe` | absorbed | internal helper of the plant longitude search |
| `zeit_discr` | ported | named in the sources or the handbook origin maps |
| `zeit_form` | ported | named in the sources or the handbook origin maps |
| `zeit_gz` | absorbed | internal helper of the chart data block and the bes_big sheet |
| `zeit_korr` | ported | named in the sources or the handbook origin maps |
| `zeit_l` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zeiteing` | ported | named in the sources or the handbook origin maps |
| `zeitgleichung` | ported | named in the sources or the handbook origin maps |
| `zeitw` | ported | named in the sources or the handbook origin maps |
| `zeitwi` | ported | named in the sources or the handbook origin maps |
| `zeitwim` | ported | named in the sources or the handbook origin maps |
| `zeitzon` | ported | named in the sources or the handbook origin maps |
| `zeitzon_nam_aaf` | ported | named in the sources or the handbook origin maps |
| `zeitzon_nam_horc` | ported | named in the sources or the handbook origin maps |
| `zesyls` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zod_zeich_alph` | ported | named in the sources or the handbook origin maps |
| `zuo` | ported | named in the sources or the handbook origin maps |
| `zuord1` | absorbed | internal helper of the place search over his gazetteer |
| `zuort` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zusp` | ported | named in the sources or the handbook origin maps |
| `zusp_mult` | screen era | GEM and GDI era screen, dialog, bitmap or printer plumbing, superseded by the Qt shell and the display list |
| `zwihat` | ported | named in the sources or the handbook origin maps |
| `zwihau` | ported | named in the sources or the handbook origin maps |
