# HORCOM7P Legacy Analysis, Part 4. Standalone Tools, Ephemeris Production and Module Survey

Covers `reference/astronom/` (33 listings + `EPHEMERS/XENA.LST`), `reference/arbeit/` (83 + `GFR/AAF_BOX.LST`), `reference/english/` (9). German quotes are Robert Rettig's original comments.

---

## 1. The ephemeris production chain

All `.EPH` producers are the same program, cloned and re-parameterised per body. The identical algorithmic skeleton appears in `PLUTOGEN`, `PLUTOVSX`, `PLUTOVSY`, `PLUTOVSN`, `PLUTOMAT`, `CHIROGEN`, `CHIRANAL`, `ASTEROI2`, `CHIRPHOL`, `HALLEY`, `QUAOAR`, `EPHEMERS/XENA`, `RUNGKUT1`, `STOERM_V`.

Pipeline (verbatim from `QUAOAR.LST` lines 9–16, repeated near-identically in all clones):

```
' Mit den Anfangswerten für ME bis PL und CE,PA,JN,VS werden 13
' Referenzwerte berechnet und mit diesen im 20-Tage-Abstand die Ephemeride
' von 10.1.2101 bis zum 10.12.1499 gespeichert.
' Wegen der inneren Planeten muß deltat variabel gewählt werden
' Trotzdem sehr schwierig,alle Planeten gleichzeitig genau zu berechnen
' Schrittlänge weit = 0.5 notwendig
' 13 Startwerte für STÖRMER-Verfahren werden mit einem RUNGE-KUTTA-Verfahren
' 6.Ordnung im Abstand von 0.5 Tagen erzeugt.
' Massen der Asteroiden mit berücksichtigt
```

Stage A — osculating elements → state vector: `@elem_oscul` → `@kepler_neu` (`// Nach MEEUS 2, S.214`, bisection Kepler solver to `1E-12`) → `@kartes_koord_ekl2` + `@kartes_vel_ekl` (`//Oskulierende Geschwindigkeit MONT. S.75`). Procedure `anfdat_aus_osc_elem`.

Stage B — 13 starter points via RK6: `PROCEDURE rung_kut(a&,ll&,nk&) // 6. Ordnung GUTHMANN S.256`. A 7-stage, 6th-order explicit Runge-Kutta integrating position and velocity simultaneously. Per-body internal sub-step `deltat(i&)`: ME 0.0005, VE 0.001, TE/MA 0.005, JU/SA/target 0.02, UR/NE/PL 0.05 days. Final weights `11/120, 0, 27/40, 27/40, -4/15, -4/15, 11/120`.

Stage C — Störmer multistep to 12th central difference. Header block, verbatim (`RUNGKUT1`, `CHIROGEN`, `CHIRANAL`, `STOERM_V`):

```
' STÖRMER/STIEFEL  BIS ZUR 12. DIFFERENZ          (  31.7.92 )   28.11.97
' NUMERISCHE INTEGRATION NACH KONSTANTEN 40-TAGESCHRITTEN  MÄRZ 1952
' Nach ASTRONOMICAL PAPERS Vol. XII "Coordinates of the outer Planets 1653-2060"
' Washington 1951
' Staatsbibl.Signatur :  4° Eph.Astr.86f/12
```

The Störmer coefficients (`ast()`), identical in every clone:
```
ast(1)=0  ast(2)=1/12  ast(3)=1/12  ast(4)=19/240  ast(5)=3/40
ast(6)=863/12096  ast(7)=275/4032  ast(8)=33953/518400  ast(9)=8183/129600
ast(10)=3250433/53222400  ast(11)=4671/78848  ast(12)=13695779093/237758976000
```
Core loop is `@gen_dif` → `@integ` → `@shift`:
```
xst(l&,i&,14) = 2 * xst(l&,i&,13) - xst(l&,i&,12) + (ftot(l&,i&,13) + a)
```
i.e. the classical summed-second-difference formula x(n+1) = 2x(n) − x(n−1) + h²[f(n) + Σ aₖ ∇²ᵏf].

Stage D — force model (`PROCEDURE anz_tot(i&,t&) //Gesamt-Anziehung`, and `accel` for the RK phase, `//Gesamt-Beschleunigung Formeln aus "ASTRONOMICAL PAPERS...."`). Heliocentric N-body, no relativity, no oblateness:
```
fso(l,i,t) = -(mass(1)+mass(i)) * xst(l,i,t) / ra(i,t)        //SO-PL   [ra = r³]
fij(l,i,j,t) = mass(j)*(((xst(l,j,t)-xst(l,i,t))/drg(i,j,t)) - xst(l,j,t)/ra(j,t))
ftot(l,i,t) = kw * (fso + Σ fij)
```
with `kw = weit² * k²`, `kg = 0.01720209895`, comment: `kg1 = kg * kg // = G = Gravitationskonstante =0.0002959122083 AE^3 * Mso ^-1 * d^-2`. The `− x_j/r_j³` term is the indirect (barycentre) term — a genuine heliocentric perturbation formulation, not barycentric.

Masses (reciprocal solar masses, `QUAOAR`/`XENA`):
```
mass(1) = 1 + 5.97682E-06        mass(6) = 1/1047.350   //JU
mass(2) = 1/6023600   // ME      mass(7) = 1/3498       //SA  NEU
mass(3) = 1/408523.5  // VE      mass(8) = 1/22960      //UR  NEU
mass(4) = 1/328900.55 //TE NEU   mass(9) = 1/19314      //NE
mass(5) = 1/3098710   //MA       mass(10)= 1/130000000  //PL  NEU
mass(11) = mass(10) / 5.36  // Masse von QUAOAR aus Durchmesserverhältnis bei gleicher Dichte
```
`mass(1)` folds the inner-planet mass into the Sun when ME…MA are dropped from the integration (`nk& = 11`, loops `FOR i& = 6 TO nk&`). In `ASTEROI2` the belt masses are real (`mass(11)=5.9E-10` Ceres … `mass(14)=1.2E-10` Vesta); in `CHIRPHOL` they are explicitly zeroed (`mass(11) = 0 //5.9E-10`, comment `' Massen der Asteroiden NICHT berücksichtigt`).

Stage E — bidirectional integration and writing: integrate forward to `jdende`, then `@anf_wert_r //Rückwärts` reverses the 13-point stencil (`xst(l,i,t) = xs(l,i,14-t)`), flips `weit2`, and the backward pass writes the file. Records are therefore in descending JD order.

---

## 2. The `.EPH` binary record layout — exact

There is no `BSAVE`/`BPUT` anywhere. Writing is GFA random access with `FIELD`/`RSET`/`PUT`. Canonical writer (`QUAOAR.LST:289–299`; byte-identical in `ASTEROI2`, `CHIROGEN`, `CHIRPHOL`, `HALLEY`, `PLUTOGEN`, `PLUTOVSX`, `XENA`):

```gfa
IF ABS(FRAC(jd)) > 0.3 AND FRAC(FIX(jd) / 40) = 0 AND sfg!   // 40-Tage-Abstand )
  INC zae%
  OPEN "R",#11,quaoar$,16
  FIELD #11,4 AS jdt$,4 AS x1$,4 AS x2$,4 AS x3$
  RSET jdt$ = MKL$(FIX(jd))
  RSET x1$  = MKL$(CINT(fquaoar * xst(1,11,13)))
  RSET x2$  = MKL$(CINT(fquaoar * xst(2,11,13)))
  RSET x3$  = MKL$(CINT(fquaoar * xst(3,11,13)))
  PUT #11,zae%
  CLOSE #11
ENDIF
```

Record structure — 16 bytes, fixed, no header, no trailer, 1-based record index:

| Offset | Size | Type | GFA | Content | Decode |
|---|---|---|---|---|---|
| 0 | 4 | int32 LE, signed | `MKL$(FIX(jd))` | integer part of the Julian Day (TT/ET), always ≡ 0 (mod step) | `JD = int32 + 0.5` |
| 4 | 4 | int32 LE, signed | `MKL$(CINT(f*x1))` | scaled heliocentric rect. X | `X[AU] = int32 / f` |
| 8 | 4 | int32 LE, signed | `MKL$(CINT(f*x2))` | scaled heliocentric rect. Y | `Y[AU] = int32 / f` |
| 12 | 4 | int32 LE, signed | `MKL$(CINT(f*x3))` | scaled heliocentric rect. Z | `Z[AU] = int32 / f` |

- `MKL$`/`CVL` = 32-bit long, little-endian two's complement.
- `CINT()` here is GFA's round-to-nearest (not C truncation) — port as `lround()`.
- Scale factor `f = 2.1748E9 / Rmax`, where `Rmax` is the max heliocentric distance in AU the body may reach. `2.1748E9` is a slightly over-generous stand-in for 2³¹ = 2.147483648E9 — with a body actually reaching `Rmax` this overflows int32 by 1.3 %. Guard this in the port.
- Records run from the newest date downward. `djd = record[1].jd − record[2].jd` is therefore positive and equals the step in days.
- File length = 16 × N; `SEEK #f&,EOF(#f&) - 16 : GET #f&` reads the last record.
- Nothing is precessed at write time. The file holds coordinates in the generator's reference frame (`jd0`); the reader precesses to date.

Reader / interpolator (`EPHEMER5.LST:1107`, `arbeit/EPHAUSW.LST`, `HORCOM7P.lst:48309`), identical logic:
```gfa
jdsuch = jd - 0.5
ind%   = FIX((jda - jdsuch) / ABS(djd))     // record index counted from the top (jda)
FOR j& = 5 DOWNTO 1
  GET #10,ind% + 5 - j&
  ys(1..3,j&) = CVL(x1$..x3$) / fplanet
  ... @praez_kartes_aeqn / @praez_kartes_ekln(f&,jdn,jdaeq) ...
NEXT j&
jdip = (jdsuch - jd3) / djd
yps = @ipol(ys(..,1..5), jdip)              // 5-point Newton-Stirling
v   = @ipol3(vs(..,2..4), jdip - djd/2)     // 3-point, velocities from 1st differences
```
`FUNCTION ipol(y1..y5,n) // Newtonsche Interpolation mit 5 Stützpunkten`:
```
yp = y3 + (b+c)*n/2 + f*n²/2 + (h+j)*n*(n²-1)/12 + k*n²*(n²-1)/24
```
Velocity is obtained from first differences referred to interval midpoints; longitude/latitude/radius rates then follow analytically (`helt`, `hebt`, `hert` in `EPHAUSW.LST`).

Per-file production table (generator column = file in `astronom/`):

| `.EPH` | Generator | Step | `f = 2.1748E9 /` | Epoch of start elements | Reference frame written | Coverage |
|---|---|---|---|---|---|---|
| `pluto.eph` | `PLUTOGEN.LST` (`PLUTOVSX` = experimental twin) | 40 d | 52 | JD 2440400.5 = 28.06.1969, state vector from Suppl. 1992 S.304 | equatorial J2000 | 1502079.5 … 2525120.5 = 18.05.602 BC … 12.06.2201 |
| `chiron.eph` | `CHIROGEN.LST` | 20 d | 20 | JD 2443400.5 = 14.9.1977, `jd0 = 2433282.423 // Jan. 0.923,1950` | ecliptic B1950 | 1502279.5 … 2524460.5 = 7.01.600 BC … 21.08.2199 |
| `ceres2/pallas2/juno2/vesta2.eph` | `ASTEROI2.LST` | 10 d | 5 | JD 2450600.5 = 1.6.1997, AHNERT 97 | ecliptic J2000 | 2268939.5 … 2488390.5 = 08.01.1500 … 18.11.2100 |
| `chiron2 / pholus / damocles / nessus.eph` | `CHIRPHOL.LST` | 10 d | 30 in listing (main program uses 40) | JD 2451400.5 = 10.08.1999 | ecliptic J2000 | 1500 … 2100 |
| `halley.eph` | `HALLEY.LST` | 20 d | 50 in listing (main program uses 40) | JD 2446471.5 = 10.02.1986, Montenbruck S.165, B1950 → J2000 via `@praez_elem` | ecliptic J2000 | 2305446.5 … 2469806.5 = 31.12.1599 … 31.12.2049 |
| `quaoar.eph` | `QUAOAR.LST` | 40 d | 60 | JD 2452800.5 = 10.06.2003, MPC | ecliptic J2000 | as Pluto |
| `xena.eph` | `EPHEMERS/XENA.LST` | 40 d | 100 | JD 2453600.5 = 18.08.2005, ALMANAC 2005 | ecliptic J2000 | as Pluto |

Reference-value sidecar files `*.EPR` are ASCII (`OPEN "O"` + `WRITE #`): `weit`, then for t = 1…13 `jdref(t)` followed by `xref(1..3, i, t)` for the integrated bodies. Names: `quao_ref.epr`, `xena_ref.epr`, `ahn_ref.epr`, `chir_ref.epr`, `alle_ref.epr`, `plu_ref.epr`, `guth_ref.epr`, `hallnref.epr`. (`refwert_ast_save` / `refwert_ast_read`.)

> Three deployment discrepancies to carry into the port. (a) `ASTEROI2` writes `…2.eph`, the product ships `ceres.eph` etc. — explained by `' Die so errechneten Ephemeriden wurden am 20.10.98 als offizielle in HORCOM5P übernommen ( unten mit Index 2 bezeichnet ).` (b) `CHIRPHOL` writes `damocles.eph`, the main program opens `damokles.eph`. (c) Scale factors for `halley` and the centaurs differ between the generator listings (50 / 30) and the shipped reader (40 / 40) — the shipped binaries must be decoded with the reader's value, 40. Determine `f` empirically from the binaries, never from these listings.

---

## 3. Per-listing summaries — `astronom/`

### Ephemeris producers

- `PLUTOGEN.LST` (1931 ln, 02.04.98) — the generator for `pluto.eph`. 10-body (Sun+ME…PL) Störmer/RK6 integration; start state vector copied literally in `PROCEDURE anfdat_aus_kart_neu // aus Supplement 1992 S.304 übernommen` (equatorial rectangular positions + velocities for JD 2440400.5). Header: `' Versuch für ALLE PLANETEN mit Anfangsdaten aus Supplement 92 S.304 / Als Grundlage für die PLUTO-Ephemeride / Wegen der inneren Planeten muß deltat variabel gewählt werden`. Writes a 40-day record every `FRAC(FIX(jd)/40)=0`.
- `PLUTOVSX.LST` (2047 ln, 11.03.98) — near-clone of `PLUTOGEN`; the "Versuch" (trial) branch with extra difference diagnostics (`maxdif/mindif/maxsum/minsum`). Same output path/scale. Experiment, not the shipping generator.
- `PLUTOVSY.LST` (1624 ln, 20.02.98) — `' Versuch für ALLE PLANETEN ME....PL … Unterschiedliche Schrittweiten von ME bis PL … Sehr brauchbar und genau !` Step-size study; writes only `.epr`, no `.eph`.
- `PLUTOVSN.LST` (1703 ln, 25.02.98) — Pluto from MATRIX IX 1982 start data, B1950. `' Gute Ergebnisse wenn die inneren Planeten zur Sonne gerechnet werden / Versuche,die inneren Planeten durch mittlere Bahnelemente zu repräsentieren führen zu Ungenauigkeiten` — the decisive experiment that justified folding ME…MA into `mass(1)`. Its `PLUTO.EPH` is written with `OPEN "O"` (ASCII, an abandoned earlier format).
- `PLUTOMAT.LST` (1396 ln, 13.02.98) — Pluto only, start data from MATRIX IX in equatorial cartesian, B1950. `' Sehr brauchbar und genau !` Precursor to `PLUTOVSN`.
- `CHIROGEN.LST` (1810 ln, 04.03.98) — generator for `chiron.eph`. 11 bodies, Chiron as #11 with `mass(11) = 0` (massless test particle). `' Extreme Radien von CHIRON etwa zwischen 8.45 und 18.864 … Interpolationsfehler bezogen auf kleinsten Radius ( = 8 ) bleibt < 7*E-8`. Frame B1950 ecliptic, 20-day records.
- `CHIRANAL.LST` (1367 ln, 12.02.98) — analysis precursor: `' Versuch: Anfangswerte JU bis NE aus analytischen ( MEEUS ) Formeln berechnen, dann mit numerischer Integration weiterrechnen / Macht guten Eindruck,allerdings läuft JU aus dem Ruder ! … Bahnelemente aus MONTENBRUCK machen guten Eindruck.Für ersten Ausbau von CH geeignet / Für Präzisionsephemeride für Pluto nicht ganz ausreichend`. No `.eph` output.
- `ASTEROI2.LST` (1834 ln, 19.10.98) — generator for Ceres/Pallas/Juno/Vesta. 14 bodies including the four belt objects with masses. 10-day records, `f = …/5`, ecliptic J2000. Elements from AHNERT 97, epoch JD 2450600.5.
- `CHIRPHOL.LST` (1818 ln, 30.07.01) — generator for the four centaurs. `' 30.07.01 Versuch,die Asteroiden CHIRON,PHOLUS,DAMOCLES und NESSUS zu berechnen / Mit Bahnelementen aus Internet Epoche 10.08.1999 / Elemente für ME bis PL aus Almanach 1999 … Massen der Asteroiden NICHT berücksichtigt`. Elements (J2000, epoch JD 2451400.5): Chiron i 6.94144 Ω 209.39582 ω 339.21394 a 13.6102621 e 0.3793816 M 25.11847; Pholus 24.69869 / 119.34869 / 354.47027 / 20.2254654 / 0.5721034 / 31.22325; Damocles 61.84100 / 314.17588 / 191.16703 / 11.8204486 / 0.8662824 / 77.05364; Nessus 15.65961 / 31.41303 / 170.31753 / 24.4562832 / 0.5171464 / 22.42329.
- `HALLEY.LST` (2071 ln, 28.08.03) — generator for `halley.eph`. Two-stage: (1) `nk&=10`, propagate ME…PL from GUTHMANN elements at JD 2442000.5 to the comet's osculation epoch, save `guth_ref.epr`; (2) `nk&=11`, inject Halley from `PROCEDURE elem_halley // aus MONTENBRUCK S. 165` (`e=0.967277`, `q=0.587157` ⇒ `a = q/(1−e)`, B1950 elements precessed to J2000 by `@praez_elem(11,jdn,jd0) //n. MEEUS 1,S.75`), save `hallnref.epr`, integrate ±, write 20-day records. Also carries `PROCEDURE halley_grob` which linearly interpolates historic apparition elements (1682, 1759, …) so the comet can be shown outside the tabulated window.
- `QUAOAR.LST` (1749 ln, 14.08.03) — generator for `quaoar.eph`; the cleanest, most readable member of the family and the best porting template. MPC elements quoted verbatim in the source (`'(50000) Quaoar / Epoch 2003 June 10.0 TT = JDT 2452800.5 … From 32 observations at 9 oppositions, 1982-2003, mean residual 0".54.`). 40-day records, `f = …/60`.
- `EPHEMERS/XENA.LST` (1742 ln, 14.02.06) — generator for `xena.eph` (2003 UB313 / Eris). `mass(11) = 0.000000039 // Masse von XENA aus Durchmesserverhältnis bei gleicher Dichte`. Elements `i 44.1774440902399, Ω 35.8747379052262, ω 151.333690213266, a 67.6619516157667, e 0.442156219250816, M 197.506001085271 //Äquin. 2000.0`, `'From 134 observations 1954-2005`. 40-day records, `f = …/100`. Unlike its siblings it keeps the file open across the whole write loop (faster).

### Numerical-method testbeds

- `RUNGKUT1.LST` (1590 ln, 05.02.98) — isolated RK6 driver, `ALERT 2,"WELCHES OBJEKT ?",1,"CHIRON|PLUTO"`. `' Nur für Pluto mit Elementen von GUTHMANN / Erster Versuch mit CHIRON … Die Integrationsgenauigkeit ist gut`. Use this to validate an RK6 port in isolation.
- `STOERM_V.LST` (1133 ln, 30.01.98) — the pure Störmer driver: `weit = 40 //Tage` hard-coded, start values from a table (`@stoeinit`) rather than RK6, start JD 2430080.5 (27.03.1941) forward or JD 2429600.5 backward, `' Differenz zu unten = 480 = 12*40 Tage`. The historically oldest file (`31.7.92`) and the one closest to the 1951 Astronomical Papers XII method.

### Orbital element sets (pure data procedures)

| File | Contents |
|---|---|
| `ELEM96.LST` | `'Daten aus ASTRONOMICAL ALMANACH 1996 / 'Osculierende Bahnelemente für Ekliptik und Equinox J2000.0` — ME…PL for JD 2450120.5 = 7.2.1996 0h, with mean motion `nd()` and mean longitude `mel()`. |
| `ELEM2000.LST` | `PROCEDURE koord3to5mitt(_ruku)` — mean (secular) elements VE/TE/MA for equinox J2000, polynomials in `t11 = t1 − 1`; converts to equatorial cartesian + osculating velocity. Used to represent the inner planets cheaply. |
| `ELEM1950.LST` | `PROCEDURE koord3to9 // VE bis NE Äquin. 1950.0`, `' Mittlere Elemente … aus MEEUS 1`, polynomials in `t1`. |
| `ELEMMITT.LST` | Same as `ELEM1950` but with `GOTO JUPITER` skipping VE…MA (outer planets only). |
| `ELEMKORR.LST` (531 ln) | `ELEM1950` plus the Meeus-1 periodic perturbation corrections: `@planp(p,q,s,g) //***` and `@planp1(ze1,et,te) //*** HILFSROUTINE FÜR UR und NE (ABKÜRZUNGEN)`; `sl`/`sr` long-period terms with arguments `mq1…mq7`, `man = 3*mq6 - 8*mq5 + 4*mq1 //ARG.LANGP.ST`. The highest-fidelity analytic element set in the folder. |
| `ELEMGUTH.LST` | `// Elemente aus GUTHMANN für Epoche jd=2442000.5 14.11.1973 0h` — ME…PL, used as `HALLEY.LST`'s stage-1 seed. |
| `ELEMCHIR.LST` | JU…NE from Almanac 1996 at JD 2443400.5, plus `CHIRON: // aus H.SCHOLL History and Evolution of Chirons Orbit //ICARUS 40,345-349 ( 1979 )` — `i 6.9229 Ω 208.7141 ω 339.1051 a 13.695105 e 0.378623 M 229.0722669`. |
| `ELEM_AST.LST` | `PROCEDURE elem_asteroid // aus AHNERT 98 S.290` — Ceres/Pallas/Juno/Vesta at JD 2450965.5 = 1.6.1998. |
| `ELEMAHN.LST` | Same four, wrapped in a self-test. Comment: `' Wahrscheinlich fehlerhaft !! Alle einige Grade daneben ( zu groß )` — do not reuse these numbers. |
| `AST_ELEM.LST` | `'Daten aus ASTRONOMICAL ALMANACH 1996 … ( Epoche 1996 Nov.13.0 )` — Ceres/Pallas/Juno/Vesta as plain labelled scalars (documentation stub). |
| `ELHALLEY.LST` | 9 lines: Halley from Montenbruck S.165, epoch JD 2446471.5. |
| `ELEMTEST.LST` | Synthetic circular coplanar orbits a = 5/10/20/30/40 AU, e = i = 0 — integrator unit test. |

### Frame / epoch utilities

- `PRAEZ.LST` (201 ln) — the precession library, extracted. Contains `praez_kartes_ekl` / `..._ruku` (`//nach MONT. 2. S 18`, rotation matrix from `pik`, `pig`, `pp`, `lag = pig+pp`), `praez_ekl_meeus` (`//nach MEEUS 2. S141 Neues Sywstem FK5`, with `hel(f&) = FN nb(la0 + pp) // pp = Präzession in Länge` and `heb(f&) = br0 + eta // eta = Degression der Ekliptikschiefe`), `praez_ekl` (`//n. MONTENBRUCK S.33`), `praez_newcomb_geoz` (old Newcomb ζ/z/θ referred to 1900.0), and `plko_ekl_aeq`/`plko_aeq_ekl`. The single most reusable file in the folder.
- `ELEM_UMR.LST` / `EPH_UMR.LST` (05./06.08.03) — `'Umrechnung von Bahnelementen auf anderes Äquinoktium Montenbruck Seite 33,34`. `PROCEDURE elem_aequin_umrechn(jd0,jdn,inkl0,omkl0,omgr0,VAR inkl,omkl,omgr)`: rotates i, ω, Ω between equinoxes using Π, π, p_a. `EPH_UMR` is the self-contained runnable tool (`'Funktioniert selbständig`); `ELEM_UMR` is the merge-in snippet (`' Zu HORCOM5P.GFW hinzumergen !`).

### Analytic ephemeris deliverables (consumers, not producers)

- `EPHEMERA.LST` (3030 ln) — the 1992 "EPHEMERO"/"EPHEMERN" licensed source delivery to three named licensees (contract header dated `R.RETTIG / EICHENAU 07.03.92`). Purely analytic: `so_ko //***SONNEN-ROUTINE ( NEWCOMB )`, `me_ko/ve_ko/ma_ko //NEWCOMB`, `ju_ko //THEORIE VON A.GAILLOT`, `sa_ko`, `ur_ko`, `ne_ko`, `pl_ko //*** PLUTO`, `ch_ko //*** CHIRON ( NUR 1890 bis 2030 A.D. !!! )` (table + `ipol`), `klpl_ko //***ÜBRIGE KLEINPLANETEN UND "HAMBURGER PLANETEN"`, `mo_ko1 //***MOND-LÄNGE,BREITE UND ABSTAND (N.MEEUS)`. No `.EPH` involvement. Historically the ancestor of the whole planet engine.
- `EPHEMER5.LST` (2633 ln, 05.05.2000) and `EPHEMENN.LST` (2586 ln, 14.04.2000) — the 2000 deliveries. 98 % identical to each other. These are the reference implementation of the `.EPH` reader (`ephem_auswert`) alongside the Meeus-II series engine (`initpterm`/`readpterm`/`plposhi`, `pl_ko` Chapront fallback). The porting-advice preamble is worth keeping verbatim:

> `// ES WIRD ABER KEINESWEGS EMPFOHLEN,ROUTINEN DIE IM RECHENGANG VERWENDET WERDEN,SELBST UMZUFORMEN.DER TEUFEL STECKT FAST IMMER IM DETAIL !`
> `// ALSO: IM RECHENGANG ALLES MÖGLICHST SO LASSEN WIE ES IST,BESONDERS AUCH DIE REIHENFOLGE DER PROCEDUREN.`
> `// Bei Anpassung auf eine andere Programmiersprache,die 2 Floating-Formate hat, dürfte für Winkel und Winkelfunktionen die niedrigere Genauigkeit reichen, falls sie besser als 9 Dezimalstellen ist … Nur für die wichtigen Variablen jd,t1,t2,t3..t11,t21,t31 und sonstige mit t beginnende Variable muß mindestens auf 12 Stellen gerechnet werden.`

- `AANZEIGE.LST` (4 lines) — debug window. Trivial.

No `ELLIPSE.LST` source, no `EPHEMERN.LST`, no `CHIRON2.LST` exist as separate algorithms — `EPHEMERN` is the 1992 product name embedded in `EPHEMERA.LST`; `CHIRON2` is an output file of `CHIRPHOL.LST`.

---

## 4. `arbeit/` module survey

Classification: [M] = module of the main program, [T] = standalone tool, [X] = experiment/scratch. ★ = real reuse value for the C++ port.

| File | Class | One-line |
|---|---|---|
| `AANZEIG.LST`, `AANZEIG1.LST` | X | 4-line debug pop-up windows. |
| `ALERTE.LST` | M | Custom modal alert built from a GFA `DIALOG`; the app's message-box primitive. |
| `AP0.LST` | M | Copy-protection gate (HORCOM5P variant). Not ported. |
| `APPEND.LST` | M ★ | `append_aaf_satz` — appends/replaces a record in an AAF file. |
| `ARABT.LST` | M ★ | Arabic Points. ~60 parts computed as `AC (or DC/MC) + planet_a − planet_b` via `@art11`/`@art12`. Named list incl. `Glück`, `Liebe und Ehe`, `Mutter`, `Vater`, `Krankheit`, `Astrologie` (= AC + H9 − H12), attribution `ma$ = "Von B.MAHL"`. Directly portable as a data table. |
| `ASP0.LST` | M | Aspect-engine body-count limits for geo vs helio. |
| `ASPDIS.LST` | M ★ | `FUNCTION aspdis(w)` — harmonic aspect classifier (360/180/120/90/72/60/51/45/40/36/32/30/27/25/24/22 plus 144, 150, 135). Compact and fully portable. |
| `ASPHIST.LST` | M | Aspect-frequency bar histogram. |
| `AUFL_ZIF.LST` | M | Resolution → tile-count lookup. |
| `AUF_UNT.LST` | M ★ | Rise / meridian transit / set, Meeus method, iterated to 1e-5 day. Standard altitudes: Sun −0.8333°, Moon 0.125° (iterated for parallax), planets −0.566667°. |
| `AVD.LST`, `AVH.LST` | M | Preference/option dialogs. |
| `BMPVIEW.LST` | X | `(c) 1995 Sjouke Hamstra` bitmap-viewer sample. Third-party. |
| `BUBANZP.LST`, `HUBANZP.LST`, `HUB_ANZ.LST` | X | Debug printers for the Huber age-point stepper. |
| `CALL_HLP.LST` | X | 8-line WinHelp test. |
| `CLIPCOPY/CREATFON/DATFIX/DRIVETYP/INKEY/KEYGET/MOUSEX/MUSTER/RFONT/RINSTR/STR/TAB.LST` | X | One-screen snippets. `RINSTR.LST` defines `FUNCTION rinstr` (reverse INSTR) used throughout. |
| `COUNTCOD.LST` | T ★ | 280 NIMA two-letter country codes as DATA lines. Clean data asset. |
| `DATEINA.LST` | M | "Mark a dataset" dialog. |
| `DIREKT.LST` | M | Chart-entry dispatcher. |
| `DISPLAYL.LST` | M | Country-code list display. |
| `EDIT.LST` | M | Demo trial counter (decrements `INTERN\AEND.INT`). |
| `EINGABE.LST` | M | The 649-line main birth-data entry dialog. |
| `ENDSUB.LST` | M | Global error handler. |
| `EPHAUSW.LST` | M ★★ | `ephem_auswert(f&) // für Einbau in HORCOM` — the standalone `.EPH` reader + interpolator + precession + rates. The definitive spec for the binary format. |
| `ERSTHILF.LST` | M | Quick-start help text (with an assignment bug making most lines dead). |
| `FIRST_NK.LST` | M | First configured extra-body slot. |
| `HERMWAG.LST` | M ★ | Hermetic rule / conception chart. Searches backwards ~266 days (`djd = 266 // PSCHYREMBEL`) for the Moon on the natal AC/DC depending on lunar phase and Sun above/below horizon. Self-contained algorithm. |
| `HORC_AAF.LST` | M | Bulk-converts `SPEZIAL\*.DAT` to `.AAF`. |
| `KENTAUR.LST` | X | Symbol test revealing three never-shipped bodies: k5 = 1997 CU26 (Chariklo), k6 = 1995 GO (Asbolus), k7 = 1995 DW2. |
| `KON_DHOL.LST` | M ★ | `KONSTA7P.INT` reader — definitive settings schema. |
| `KOTABSTA.LST` | M | Loads a chart out of the statistics database. |
| `KO_TA.LST` | M | Coordinate table renderer. |
| `LAD_EXE.LST` | M | External EXE launcher. |
| `LANDNAM.LST` | T | 57 German country abbreviations (commented out; source of `laender.int`). |
| `LEGEND/LOC/MAXBREIT/SCGETALT/SETF/ST_ANZ/PLANKLIK/MAFDSPL1/PLANT_AN/PLEIN2/POPUP/SYMB_ANZ/TEXT_LES/PLANT2/PLANTR/SCROLL.LST` | M/X | Small UI/scaffolding procedures. `PLANT2.LST` reports transit-search interpolation error in seconds. |
| `LISTAUS7.LST` | M | 431-line list/table output engine. |
| `MAKEGRP.LST` | X | Windows 3.x Program Manager DDE group creator. Obsolete. |
| `MEN3.LST` | M | Main menu screen module. |
| `MOKO.LST` | M ★★ | True lunar node and apogee (Black Moon) from the osculating lunar orbit (r × v, Montenbruck S.78), with mean-element fallbacks. Self-contained and highly portable. |
| `MUND.LST` | M | Mundane module driver. |
| `ORTWAND.LST` | M ★ | "Place wandering" — animated relocation astrology. Trivial geometry; the value is the interaction model. |
| `PLANP_DS.LST` | M | Planet-glyph blitter with the `jdplanetex!` out-of-range guard. |
| `QUAOAR.LST` | X | Older (02.08.2003) sibling of `astronom/QUAOAR.LST`. Superseded — use the astronom copy. |
| `READPT.LST` | M ★ | Standalone `readpterm` — the `PLANETS.NDX`/`PLANETS.DAT` loader spec. |
| `RELEAS.LST` | M | Clears read-only attributes after CD install. |
| `SCROLLBA.LST` | X | Scrollbar sample. |
| `SYMBGEN.LST` | T ★ | Symbol generator: `'Versuch,Vektorsymbole zu generieren. 18.2.97`. Defines a small GDI vector API (`defline_api`, `ellipse_api`, `circle_api`, `arc_api`, …). The planet/zodiac/aspect glyphs as vector primitives — the right source for a resolution-independent C++ port instead of the shipped bitmaps. |
| `UTET.LST` | M ★★ | The complete ΔT model. Ten lines; everything downstream depends on it. |
| `VERSUCH.LST` | X | 1.28 MB complete snapshot of the main program dated 21.10.2008. Do not port from it, but its header changelog is the best release history. |
| `WELT.LST` | T ★ | World city gazetteer, ~318 entries. Pure data. |
| `WRKP.LST` | M | Demo date-range gate. Not ported. |
| `INSTALCD.LST` | T ★ | CD installer incl. `copy_expand_file` (LZ expand) and the DPI probe. Relevant only as a spec of the directory layout. |
| `GFR/AAF_BOX.LST` | M | GFA dialog resource for the AAF record editor — documents the full AAF field set. |
| `ZEITZONN.LST` | T ★★ | Timezone master table, 177 entries, format `"<Full name>   <ABBR>   ±HH h MM m"`, ordered east→west. Includes ~60 historic local mean times with validity ranges (`Pariser Time ( 1891 - 1911 ) -00 h 09 m`, `Amsterdamer Time ( 1892 - 1940 ) AMT -00 h 20 m`, `Prager Time ( 1850 - 1891 ) -00 h 58 m`, `Reykjavik Time ( 1837 - 1908 ) +01 h 28 m`, …). A curated historical dataset not obtainable from IANA tzdata — the single highest-value non-astronomical asset in the whole reference tree. |

---

## 5. `english/` survey

The English line is a string-table fork only — no algorithmic divergence anywhere.

| File | Module | Differs beyond language? |
|---|---|---|
| `AANZEIGE.LST` | Debug pop-up | No. |
| `EDIT.LST` | `bes2` — chart-header captions | Slightly: different procedure than `arbeit/EDIT.LST`; English literals `"90°-CIRCLE"`, `"GEOZENTRIC"` (sic). |
| `HIMMEL.LST` | One-shot tool rendering the sky-strip banner bitmap (592×67) | Standalone asset-build tool, no German counterpart preserved. |
| `LAND.LST` | 57 country abbreviations, English names | 1:1 translation of `LANDNAM.LST`. |
| `WRKP.LST` | Demo year-range gate | Materially: only three allowed year ranges vs four in German; latent stray `0` byte in the nag string. |
| `ZONEALT.LST` | Legacy short zone table, 64 entries with standard meridians | Older, different table; sign convention opposite to `ZONEN.LST`. No German equivalent survives. |
| `ZONEN.LST` | Current zone table | Structural twin of `ZEITZONN.LST`. |
| `ZEITZON.LST` / `AA_TIMEZ.LST` | `'INTERNATIONAL TIME - ZONES / 'Compiled by Bruno Mahl` — human-readable source document | Sign convention here is `TIME - GMT` (offset to add to UT), the negation of the `zone$()` values in `ZONEN`/`ZEITZONN`. Critical for the port. |

---

## 6. Definitive per-body position source

Resolved from `HORCOM7P.lst:48309–48440` cross-checked against `EPHEMER5.LST` and `EPHAUSW.LST`. Slot numbers `nk&(1..22)` are user-configurable via `KONSTA7P.INT`; the table gives the canonical assignment.

| Body | Source | Stored frame in `.EPH` | Precession applied on read |
|---|---|---|---|
| Sun (1) | analytic, `plposhi(3)`/`soko` — Meeus II series from `PLANETS.DAT` | — | series is of date |
| Moon (2) | analytic, `mo_ko1 // MOND-KOORDINATEN nach MEEUS II` | — | — |
| True node (11/12), Apogee/Black Moon | derived, `moko` from the osculating lunar orbit (r × v) | — | — |
| Mercury…Neptune (3–9) | analytic, `plposhi` — Meeus II truncated VSOP terms | — | series of date |
| Pluto (10) | `pluto.eph`, 40 d, `f = 2.1748E9/52`; outside range → `pl_ko` (Chapront) | equatorial J2000 | `praez_kartes_aeqn(f&, jd, 2451545.0)` then `ko_tr2` |
| Chiron (`nk&(2)`) | `chiron.eph`, 20 d, `f = …/20` | ecliptic B1950 | `praez_kartes_ekln(f&, jd, 2433282.423)` |
| Ceres / Pallas / Juno / Vesta (`nk&(5..8)`) | `.eph`, 10 d, `f = …/5` | ecliptic J2000 | `…ekln(…, 2451545.0)` |
| Quaoar (`nk&(17)`) | `quaoar.eph`, 40 d, `f = …/60` | ecliptic J2000 | `…ekln(…, 2451545.0)` |
| Halley (`nk&(18)`) | `halley.eph`, 10 d in the shipped file, `f = …/40` | ecliptic B1950 in fact (`HALLEY.LST` says J2000) | `…ekln(…, 2433282.423)`, B1950 through the first CASE. The shipped file meets his B1950 start elements only read this way (0.04° against 0.71°), so the reader is right. |
| Pholus / Damocles / Nessus (`nk&(19..21)`) | `.eph`, 10 d, `f = …/40` | ecliptic J2000 | `…ekln(…, 2451545.0)` |
| Xena/Eris (`nk&(22)`) | `xena.eph`, 40 d, `f = …/100` | ecliptic J2000 | `…ekln(…, 2451545.0)` |
| Transpluto, Uranian bodies CU…PO (`nk&(9..16)`) | analytic, `plelem` — mean-element circular/Keplerian formulae | — | — |
| AC / MC / house cusps | spherical trig from ARMC + φ + ε | — | — |
| Chariklo / Asbolus / 1995 DW2 | planned, never shipped — `.eph` cases commented out; glyphs exist | — | — |

Every `.EPH` body is guarded by `jdplanetex!(f&)`; on failure all coordinates are zeroed and the glyph is suppressed.

---

## 7. Top 10 listings a C++ reimplementer should study closely

1. `arbeit/EPHAUSW.LST` — the complete, self-contained `.EPH` reader. Port this first; it unlocks 12 bodies.
2. `astronom/QUAOAR.LST` — the cleanest full producer, the canonical porting template for regenerating ephemerides.
3. `astronom/PRAEZ.LST` — all four precession routines plus the ecliptic↔equatorial pair, isolated and dependency-light.
4. `astronom/EPHEMER5.LST` — the reference analytic engine, plus the author's explicit porting warnings.
5. `arbeit/ZEITZONN.LST` (with `english/ZEITZON.LST` / `ZONEALT.LST` as cross-checks) — the 177-entry historical timezone table. Watch the sign-convention flip between representations.
6. `arbeit/MOKO.LST` — osculating-orbit true node and lunar apogee. Short, exact, non-obvious, and widely wrong in other implementations.
7. `arbeit/UTET.LST` — the complete ΔT model.
8. `arbeit/AUF_UNT.LST` — rise/transit/set with exact constants.
9. `arbeit/KON_DHOL.LST` — the full `KONSTA7P.INT` settings record in read order.
10. `arbeit/ARABT.LST` + `arbeit/ASPDIS.LST` — the two purest astrological-logic files, both convertible to static C++ tables.

Runners-up: `arbeit/SYMBGEN.LST` (vector glyph primitives), `arbeit/HERMWAG.LST` (Hermetic/Epoch chart), `arbeit/WELT.LST` + `COUNTCOD.LST` (gazetteer + NIMA codes), `arbeit/GFR/AAF_BOX.LST` (AAF field schema), `astronom/ELEMKORR.LST` (Meeus-1 perturbation series).
