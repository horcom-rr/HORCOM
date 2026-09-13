# HORCOM7P Legacy Analysis, Part 1. The Calculation Core

Derived from `reference/HORCOM7P.lst` (50,397 lines). All line numbers refer to that file. German quotes are Robert Rettig's original comments.

---

## 0. Naming traps (read first)

| Looks like | Reality |
|---|---|
| `rechne1`/`rechne` = central planet computation | `rechne1` (L25166) and `rechne` (L25261) are a degree/time unit converter dialog ("WINKEL(ZEIT)-UMRECHNUNG"). No astronomy at all. |
| `ap0`/`ap1` = planet routines | `ap0` (L25868), `ap1` (L25835), `a70` (L25983), `a77` (L25996) are the copy protection / password check (`KENNTS7P.INT`, checksum `acc%`). Not ported. |
| `fixpunkt` = fixed stars | `fixpunkt&`/`fixpunkt$` is a user-defined "Fixpunkt", an arbitrary ecliptic degree placed in the chart as pseudo-planet index 0. Fixed stars live in `stella` (L17974). |
| `ampl_rad()/ampl_mund()/orbh()` are general orbs | They belong exclusively to the Huber / Münchner Rhythmenlehre age-point "Dynamogramm" module (L35340 ff.). General aspect orbs are `or&()`, `orb`, `orbe()`. |

The real entry point of the calculation core is **`a9` → `a90` → `plko` → `plko_einz`** (L16695–16712, L49304).

---

## 1. Planet position calculation

### 1.1 Theory used

Three different models, selected per body:

| Body | Model | Routine |
|---|---|---|
| Sun (=Earth reversed), Mercury…Neptune | VSOP87 periodic series (Meeus) read from binary `PLANETS.DAT`/`PLANETS.NDX` | `plposhi` L48913 |
| Sun/Moon mean elements, nutation | Meeus polynomial series | `somo` L49547 |
| Moon | Meeus Ch. 45 (ELP-2000 truncated), 60 terms longitude/radius + 60 terms latitude, inline `DATA` tables | `moko1` L49744, tables `lablr:` L49839, `labb:` L49903 |
| Pluto | numerically integrated ephemeris `PLUTO.EPH`, fallback to Chapront analytical theory outside the file range | `ephem_auswert` L48309 / `pl_ko` L48609 |
| Chiron, Ceres, Pallas, Juno, Vesta, Quaoar, Halley, Pholus, Damokles, Nessus, Xena | numerically integrated `.EPH` files | `ephem_auswert` L48309 |
| Transpluto, Uranian/Hamburg-School factors (Cupido…Poseidon) | fixed Kepler elements + linear mean anomaly, solved with `kepler` | `plelem` L49045 `CASE 9…16` |
| Mean elements for all planets (used for nodes, perihelia, Pluto, velocity) | Meeus mean orbital elements J2000, T in Julian centuries from J2000 | `plelem` L49045 (`'Elemente nach MEEUS`) |

Key epoch note: `t1 = (jd - 2415020)/36525` (centuries from 1900.0), and `t11 = t1 - 1` = centuries from J2000 (2451545). All Meeus polynomials use `t11`, `t21=t11²`, `t31`, `t41`. VSOP uses `t = t11/10` (Julian millennia from J2000). Set in `juld1` L25642.

### 1.2 `plposhi(pl&)` — VSOP evaluation (L48913)

```
t = 1e-15 + t11/10                       ' julian millennia from J2000
for i& = 1..3 (L, B, R):
   tn = 1
   for j& = 0..5 (power of t):
      for k& = 1..plindex_noterms&(pl&,i&,j&):
         of& = plindex_offset&(pl&,i&,j&) + k&
         arg = nb(b + t*c)                ' b=plterms_b, c=plterms_c
         x  += a*cos(arg)                 ' a=plterms_a
         v  += -a*c*sin(arg)              ' analytic derivative
      ko(i&) += x*tn
      vel(i&) += x*j&*tn/t + v*tn
      tn *= t
```

Writes `hel(p&)` (heliocentric longitude, rad), `heb(p&)`, `r(p&)` (AU), `helt/hebt/hert(p&)` = derivatives /365250 (per day).

Body remap: `d& = 1` for `pl& = 0,4..8`; `d& = 2` for `pl& = 1,2`; `d& = -2` for `pl& = 3`; `p& = pl& + d&`. So VSOP file index 1=Mercury→`p&`3, 2=Venus→4, 3=Earth→`p&`1 (used as Sun), 4=Mars→5 … 8=Neptune→9.

Reads globals `t11`, `lin!` (suppress velocity), `plindex_*`, `plterms_*`. Note that `gen&` ("Genauigkeit", default 2 = "Maximal") and the term-magnitude cutoff are commented out (L48931/48933), the final version always evaluates all terms.

### 1.3 `kepler(f&)` (L48814) — elliptical orbit solver

```
ea = nb(m + e*sin(m)/sqrt(1 - 2e*cos(m) + e²))      'START ODELL..
repeat: ea = nb(ea + (m + e*sin(ea) - ea)/(1 - e*cos(ea)))   'MEEUS
until |Δea| < 1e-7
v(f&)   = nb(2*atan(sqrt((1+e)/(1-e))*tan(ea/2)))    'WAHRE ANOMALIE
r(f&)   = a(f&)*(1 - e*cos(ea))
u(f&)   = nb(p(f&) + v(f&))
hel(f&) = nb(atn(cos(i)*sin(u), cos(p+v)) + o(f&))
heb(f&) = asin(sin(u)*sin(i))
```

Element arrays: `a()` semi-major, `e()` ecc, `i()` incl, `o()` asc. node, `p()` arg. perihelion, `man()` mean anomaly, `mel()` mean longitude, `ean()` ecc. anomaly, `v()` true anomaly, `u()` argument of latitude.

### 1.4 Heliocentric → geocentric: `hel_geo(f&)` (L49321) + `plko12` (L49374)

```
x = R·cos b·cos l − r₁·cos l₁          (r₁,l₁,b₁ = Earth from plposhi(3))
y = R·cos b·sin l − r₁·sin l₁
z = R·sin b       − r₁·sin b₁
dr(g&) = |(x,y,z)|                      ' geocentric distance
el(g&) = nb(atan2(y,x) + dpsi)          ' apparent ecl. longitude (nutation added)
eb(g&) = deps + asin((r(g&)/dr(g&))·sin b)
```

Velocity `tb(g&)` and acceleration `ttb(g&)` are obtained by numerical differencing of the geocentric longitude at `l ± helt/10`, a symmetric two-sided difference.

Sun: `soko` (L49195): `el(1) = nb(hel(1) + PI + dpsi)`, `eb(1) = -heb(1) + deps`, `dr(1) = r(1)`.

### 1.5 helio/geo switch (F6)

Global boolean `hrg!` (0 = geocentric, −1 = heliocentric). `geohelio` L869 toggles it; the key handler is at L741 (`VK_F6` or Alt+H, `MENU(12)=72/104`) and duplicated at L6490 and L22561; `a93` (L16880) is the menu version.

When `hrg!` is set: the Moon slot is used for the Earth (`pl$(2) = "TE"`, `hel(2)=hel(1)`), houses/AC/MC/nodes/Lilith/Part of Fortune are switched off (`aa& = 2`, `bb& = 10`, see `asp0` L20462), `dpsi=deps=0` (`somo` L49602), and `a92` stores `hel()/heb()/helt()` instead of `el()/eb()/tb()` into `plz/ebz/tbz`.

### 1.6 Per-body dispatcher `plko_einz(f&)` (L49241) and `plko` (L49304)

```
plko: @asp0 ; FOR f& = aa& TO MAX(12,np&) : IF f&=11 && klpl! THEN f&=19 : @plko_einz(f&)
```

`plko_einz`:
- 0 → `lpkt` (Fixpunkt)
- 1 → `soko` (geo) / `plposhi(3)` (helio)
- 2 → Earth copy in helio mode; Moon is done separately by `moko`
- 3,4 → `pl_el_ve` + `plposhi(f&-2)` + `hel_geo` + `par_ap_ktr`
- 5…9 → `pl_el_ve` + `plposhi(f&-1)` + `hel_geo` + `par_ap_ktr`
- 10 (Pluto) → `pl_ko` if outside range, else `ephem_auswert(10)`, then `plko10`, `hel_geo`, `par_ap_ktr`
- `nk&(1…22)` → `pl_el_ve` + `hel_geo` + `par_ap_ktr` if `jdplanetex!(f&)=0`

### 1.7 Post-processing chain `par_ap_ktr(f&)` (L49393)

1. `plkoap(f&)` (L49451), apparent position. `e = 0.0057755 * tb(f&) * dr(f&)` (light-time 0.0057755 d/AU × daily motion). `appa&=1`: `el -= e`. `appa&=2`: additionally annual aberration `−0.00009936509·cos(el−l☉)/cos(eb)` and `db = b·sin(el−l☉)·sin(eb)`. `appa&=3`: none (true position).
2. `plkotr(f&)` (L49443) → equatorial `ar(f&)`, `de(f&)` via `ko_tr1`.
3. `par(f&)` (L49398), topocentric parallax, active only if `par = 1` (default `par = 2` = off). `//aus Montenbruck S.25  Ohne Ber. d.Meereshöhe`, Earth radius `ro = 0.000042634515` AU, uses `gg` (geographic latitude) and `armc`. **Protected logic per CLAUDE.md.**

### 1.8 Pluto's Chapront theory `pl_ko` (L48609)

`//NACH CHAPRONT außerhalb des num. berechneten Bereichs`. Perturbation series in the arguments j,s,u,n (mean anomalies of Jupiter, Saturn, Uranus, Neptune from Meeus) and p = Pluto's mean longitude `nb(2.53333·t11 + 4.166868)`. Produces corrections `sa, sl, sq, sp, sk, sm` to the mean elements `ap0=39.544674, kp0=−0.178744, qp0=−0.051699, ep0=4.166868, hp0=−0.173426, pp0=0.139782, nup=2.53333 °/century`. `plelempl` (L48717) converts the (h,k,p,q) equinoctial set back to classical elements; `pl_praez` (L48748) runs `kepler(10)` and precesses from J2000 to date.

---

## 2. Ephemeris files (`.EPH`)

### 2.1 `initpterm` / `readpterm` are NOT the `.EPH` reader

- `initpterm` (L48841) allocates the VSOP arrays: `nopterms& = 2638` `'NOTWENDIGER WERT UM ALLE TERME AUSZULESEN`, `plindex_noterms&(9,3,5)`, `plindex_offset&(9,3,5)`, `plterms_a/b/c(2638)`, `cterm& = 1`.
- `readpterm(pl&)` (L48855, `'MEEUS`) reads `\INTERN\PLANETS.NDX` and `\INTERN\PLANETS.DAT`. Called once at startup for `pll& = 1…8` (L412–415).

`PLANETS.NDX`, record length 4, fields `2 AS a$, 2 AS b$` → two 16-bit ints (`CVI`) = (offset, number of terms). 18 records per planet, seek base `l& = pl& * 18`, iterated `i& = 1..3` (L,B,R) × `j& = 0..5` (power of t).

`PLANETS.DAT`, record length 24, fields `8 AS c$, 8 AS d$, 8 AS e$` → three 64-bit doubles (`CVD`) = A, B, C of `A·cos(B + C·t)`.

Offsets are rebased into the in-memory array: `change& = cterm& − datoffset& − 1`, then `plindex_offset&(pl&,i&,j&) += change&`.

### 2.2 `.EPH` binary layout — `ephem_auswert(f&)` (L48309)

Record: 16 bytes, fixed-length random access.

```
FIELD #10, 4 AS jdt$, 4 AS x1$, 4 AS x2$, 4 AS x3$
```

Four 32-bit signed integers (`CVL`, little-endian):
- `jdt$` = `FIX(jd)` at the record's epoch
- `x1$,x2$,x3$` = rectangular coordinates × `fplanet`, i.e. `coord_AU = CVL(x)/fplanet`

`fplanet` is a per-body scale keeping values inside int32 (`2.1748E9 ≈ 2^31`), i.e. `2^31 / max_AU`:

| file | `fplanet` | body idx | frame |
|---|---|---|---|
| `pluto.eph` | 2.1748E9/52 | 10 | equatorial, equinox J2000 |
| `chiron.eph` | 2.1748E9/20 | `nk&(2)` | ecliptic, equinox B1950 (JD 2433282.423) |
| `ceres/pallas/juno/vesta.eph` | 2.1748E9/5 | `nk&(5..8)` | ecliptic J2000 |
| `quaoar.eph` | 2.1748E9/60 | `nk&(17)` | ecliptic J2000 |
| `halley.eph` | 2.1748E9/40 | `nk&(18)` | ambiguous, see 2.5 |
| `pholus/damokles/nessus.eph` | 2.1748E9/40 | `nk&(19..21)` | ecliptic J2000 |
| `xena.eph` | 2.1748E9/100 | `nk&(22)` | ecliptic J2000 |

Generation confirmed in `reference/astronom/QUAOAR.LST` L285–300:

```
RSET jdt$ = MKL$(FIX(jd))
RSET x1$  = MKL$(CINT(fquaoar * xst(1,11,13)))   ...
PUT #11, zae%
```

written every 40 days (`FRAC(FIX(jd)/40)=0`; a 10-day variant is commented out) from a Runge-Kutta/Størmer n-body integration (`RUNGKUT1.LST`, `STOERM_V.LST`).

### 2.3 Indexing

Records are stored descending in time (the generator integrates backwards):

```
GET #10,1 : jda = CVL(jdt$)            ' newest record
GET #10,2 : jd_ = CVL(jdt$)
djd = jda - jd_                        ' Intervall-Länge  (positive, e.g. 40)
jdplaneta = jda - 3*djd + 0.5          ' usable upper bound
SEEK #10,EOF(#10)-16 : GET #10 : jdplanete = CVL(jdt$) + 3*djd - 0.5
jdsuch = jd - 0.5                      ' !!! FIX(jd) NOT used
ind%   = FIX((jda - jdsuch)/ABS(djd))  ' Datensatzindex von oben ( jda ) gerechnet
```

Then five records `ind%+1 … ind%+5` are read into `ys(1..3, 5..1)` (loop `j& = 5 DOWNTO 1`, `GET #10, ind%+5-j&`), so `ys(*,3)` is the centre epoch `jd3` with `jd1..jd5 = jd3 + (k-3)·djd`.

Out-of-range handling: if `jd < jdplanete OR jd > jdplaneta` → `jdplanetex!(f&) = -1` and all outputs zeroed (`el, eb, hel, heb, ar, de, plz` = 0). Every consumer tests `jdplanetex!()`.

Documented coverage (comments L48382–48391):

```
// PLUTO und QUAOAR und XENA :
'1502079.5 bis 2525120.5   18.05.602 v.Chr. bis 12.06.2201
// CHIRON:
'1502279.5 bis 2524460.5    7.01.600 v.Chr. bis 21.08.2199
// ASTEROIDEN CE...VS :
'2268939.5 bis 2488390.5   08.01.1500 A.D.  bis 18.11.2100 A.D.
// Fü Komet HALLEY :
'2305446.5 bis 2469806.5     31.12.1599       bis 31.12.2049
```

### 2.4 Precession of the stored vectors, then interpolation

Each of the 5 sampled vectors is individually precessed from its catalogue equinox to the epoch of date before interpolation:
- `praez_kartes_aeqn` (L48542), equatorial rotation matrix, `//nach MONT. 2. S 18  Elemente nach MEEUS`, angles ζ, z, θ (IAU 1976 polynomials with `t0 = (jdaeq−2451545)/36525`, `t = (jdn−jdaeq)/36525`).
- `praez_kartes_ekln` (L48575), ecliptic rotation matrix with π (`pik`), Π (`pig`), p_A (`pp`), `lag = pig + pp`.

Interpolation `ipol` (L48492), 5-point central difference (Newton-Stirling, 4th order):

```
n = (jdsuch - jd3)/djd            ' -1 … +1
a=y2-y1, b=y3-y2, c=y4-y3, d=y5-y4
e=b-a, f=c-b, g=d-c, h=f-e, j=g-f, k=j-h
yp = y3 + (b+c)*n/2 + f*n²/2 + (h+j)*n*(n²-1)/12 + k*n²*(n²-1)/24
```

`ipol3` (L48526) is the 3-point quadratic used for velocity. Both contain a branch-cut guard: if `y_i − y_{i+1} > 3` (rad) then `+2π` is added to the following values, needed because the same routines are reused for angle series.

Velocities: `vs(*,j&) = (ys(*,j&+1) − ys(*,j&))/djd` (mid-interval), interpolated at `yip = jdip − djd/2` via `ipol3`. Note that `jdip` is dimensionless and `djd/2` is in days; this offset looks dimensionally inconsistent and should be reviewed when porting (it only affects speed display, not position).

Output: for Pluto `ar/de/r` then `ko_tr2` → `hel/heb`; for the others `hel = atan2(x2,x1)`, `heb = asin(x3/rd)`, `r = rd`, plus analytic derivatives `helt`, `hert`, `hebt`.

### 2.5 Genuinely unclear, verify before porting

- `n18&` (Halley) appears in both `CASE n2&,n18&` (B1950) and `CASE n5&,…,n18&,…` (J2000) at L48426/48429. GFA's `SELECT` takes the first matching CASE, so Halley is precessed from B1950, but this is almost certainly an editing accident. Verify against `astronom/HALLEY.LST` before porting.
- `jdplaneta` (upper) and `jdplanete` (lower) are named as if swapped relative to the range test `IF jd < jdplanete OR jd > jdplaneta`; the code is self-consistent (records descend) but the naming is misleading.
- The commented-out `k5/k6/k7.eph` (1997 CU26, 1995 GO, 1995 DW2) are dead code.

---

## 3. Time handling

### 3.1 Julian date — `juld` (L25614) / `dat` (L25655)

Standard Meeus algorithm with an explicit Julian/Gregorian override:

```
IF mo > 2 THEN y=ja,m=mo ELSE y=ja-1,m=mo+12
IF ja+mo/100+ta/10000 < 1582.1015            THEN b = 0
ELSE IF INSTR(jul$(1,ze),"(JULIAN.") > 0     THEN b = 0
ELSE a = FIX(y/100) : b = 2 - a + FIX(a/4)
IF date < 1582.1015 AND jul$ contains "(GREGOR." THEN b = 2 - a + FIX(a/4)   ' proleptic Gregorian
jd = FIX(365.25*y [- 0.75 if y<0]) + FIX(30.6001*(m+1)) + ta + ho/24 + mi/1440 + 1720994.5 + b
```

`dat` is the exact inverse (`z<2299161` ⇒ Julian branch, same `jul$` overrides). The calendar choice per chart lives in the string array `jul$(3,6)` containing `"(JULIAN.)"` / `"(GREGOR.)"`.

`juld1` (L25642) recomputes the time arguments and then calls `somo`:

```
t1 = (jd-2415020)/36525 ; t2=t1² ; t3 ; t4
t11 = t1-1 ; t21 ; t31 ; t41
tja  = 365.24219879 - 6.14E-06*t1
ekls = 0.40931974745 - 0.0002271109689*t1 - 2.86234E-08*t2 + 8.779E-09*t3   ' mean obliquity, rad
@somo
```

### 3.2 Sidereal time — `sidt` (L37888)

```
' computed for 0h UT of the day
h0 = nh(6.6460656 + 2400.051262*t1 + 0.00002581*t2 - 1.72222E-09*t³)   ' GMST at 0h, hours
IF stzw& = 1 THEN h0 = nh(h0 + dpsi*3.8197186*COS(ekls))                ' → apparent sidereal time
hs = nh(h0 + (ho + mi/60)*1.002737908)                                  ' GAST/GMST at the moment
```

`stzw&` (default 1, `// wahre Sternzeit`) selects apparent vs. mean sidereal time. Globals written `h0`, `hs`; persisted as `h0(od,ze)`, `hs(od,ze)`. `eckp` (L25712): `armc = 15 * nh(hs + gl/15)` (degrees, local sidereal time; `gl` positive East), `armcb = pu*armc`.

### 3.3 Equation of time — `zeitgleichung` (L49608)

```
y = tan²(ekls/2)
z = y·sin(2L) − 2e·sin(M) + 4ey·sin(M)·cos(2L) − ½y²·sin(4L) − 1.25e²·sin(2M)
```

with `M = man(1)`, `L = mel(1)`, `e = e(1)`. Returns radians; callers convert with `* up/15` (→ hours).

Applied in two places, both keyed on year 1810 (change log `// Anwendung der Zeitgleichung für <1810 automatisch  17.01.05`, L140):
- `zuo(nein!)` L23808, the HORCOM input path: `IF ortsz!(ze)` (local-time flag) and `ja < 1810` → automatic `jd = jd - zeitgl*up/15/24` with the message `"WAHRE Ortszeit wird in MITTLERE Ortszeit umgerechnet !"`. For `1810 ≤ ja < 1890` the user is asked (`"Meist war zu diesem Datum bereits MITTLERE Ortszeit üblich !"`, LTT vs LMT). Afterwards `jd = jd - gl/360` (LMT→UT from longitude).
- `zeitzon_nam_aaf` L5265, the AAF input path with zone abbreviations `LMT`/`LTT` in the zone table (`\intern\zonnamen.int`, "Zeit-Zonen (P.D. Via B.MAHL)"): `zzd = -gl*24/360`, plus `- zeitgl*up/15` for LTT.

State kept per chart slot: `zeitgl(6)` (the applied value) and `zeitgl_korr!(6)` (already-asked flag). Reconstruction back to LMT happens at L45578: `jd = jd + zeitgl(ze)*up/15/24 // LMT rekonstruieren`.

### 3.4 ΔT / UT→ET — `utet` (L17944), `utet1` (L17966), `etut` (L17970)

```
j = 1900 + (jd - 2415020)/365.25            ' approximate year
1620 ≤ j ≤ 2008 : linear interpolation in table (y&(1..22), z(1..22)), result /60 → minutes
j > 2008        : delt = 1.2053*t1 + 0.4992*t1² - 0.7506263        '2008 a.d. ........
948 < j < 1620  : delt = 0.425 + 0.85*t1 + 0.425*t1²               '948 a.d. ..... 1600 a.d.
j ≤ 948         : delt = 28.74 + 6.81*t1 + 0.7383*t1²              '390 v.chr. .... 948 a.d.
```

`delt` is in minutes. `utet1`: `jd = jd + delt*0.0006944444444` (=/1440) then `@dat`; `etut` subtracts. The table (`DIM` L15759, values L15777–15820) is (year, ΔT seconds): 1620/124, 1640/62, 1660/37, 1680/16, 1700/9, 1720/11, 1740/12, 1760/15, 1780/17, 1800/13.7, 1820/12, 1840/5.7, 1860/7.88, 1880/−5.4, 1900/−2.72, 1920/21.16, 1940/24.33, 1960/33.15, 1991/57.2, 1997/63, 2000/65, 2008/67. Change log `// UTET bis 2008 extrapoliert  28.06.04` (L157).

All planet computation runs in ET: `a90` L16700 does `@utet : @utet1 : @juld1 : @plko : @moko : @vel_om_pd : @a901 : @etut`. Houses (`jseckp`) are computed in UT before that.

### 3.5 Time zone / DST at input time

- `zeitzon(6)` = zone offset in hours (west positive; `jd = jd + zeitzon(ze)/24`, L23856).
- `ortsz!(6)` = "local (mean/true) time used" flag → `jd = jd - gl/360`.
- `somz(6)` = summer time: `-1` = 1 h DST, `-2` = 2 h ("Doppelsommerzeit"); set at L25017/25027/25095/25112, applied when reconstructing local times at L45498–45504.
- `hor_add(dho,dmi,VAR ta,mo,ja,ho,mi)` (L3835) does calendar-safe hour addition; `make_juld` (L4986) parses the AAF zone string (`"02hE00:00"`) and calls it with `-h`.

---

## 4. House systems

Selector `haw&` (default 1), name string `haus$` (default `"Placidus"`). Menu `hausw` (L25580):

| `haw&` | Label (`hausw` L25582) | Routine | `haus$` |
|---|---|---|---|
| 1 | PLACIDUS | `a6` L25772 → `plac` L48064 | "Placidus" |
| 2 | TOPOZENTRISCH | `a7` L25832 → `topo` L48218 | "Topozentr." |
| 3 | KOCH-GOH | `a8` L26005 → `koch` L47998 | "Koch-GOH" |
| 4 | REGIOMONTANUS | `a81` → `regio` L48129 | "Regiomont." |
| 5 | CAMPANUS | `a82` → `camp` L48182 | "Campanus" |
| 6 | ÄQUAL EKLIPTIKAL ab AC | `aeqe` L26014 | "Äqual-Ekl." |
| 7 | ÄQUAL EKLIPTIKAL n. VEHLOW | `vehlow` L26024 | "Äqual-Vehl" |
| 8 | KEINE Häuser, NUR AC und MC | `leer` L25775 | "Keine" |
| 9 | WEDER HÄUSER noch AC oder MC | `leer` | "Keine" |
| 10 | …noch MONDKNOTEN | `leer` | "Keine" |

Dispatch: `a60` (L25762) → `ON haw& GOSUB a6,a7,a8,a81,a82,aeqe,vehlow,leer,leer,leer`, guarded by `maxbreit` (L25745): for Placidus/Koch, `|lat| > 90 − ε` is refused with `"Geog. Breite zu groß ! Nur bis +- …"`.

### AC / MC / Vertex — `eckp1(armcb,gg)` (L25719)

```
AC  f(1)  = atan2( cos(RAMC), -(sin ε·tan φ + cos ε·sin RAMC) )   ; pl(13)=AC
DC  f(7)  = f(1) + π
MC  f(10) = atan2( sin RAMC, cos RAMC · cos ε )                   ; pl(14)=MC
IC  f(4)  = f(10) + π
Vertex vert(od,ze) = atan2( cos(π+RAMC), -(sin ε·tan(π/2 − φ) + cos ε·sin(π+RAMC)) )
```

### Cusp formulas

- Placidus `plac1` (L48088), fixed-point iteration on the semi-arc: `x = nb(RAMC + acos(−sin x·tan ε·tan φ)/n1)`, `y = nb(RAMC + π − acos(sin y·tan ε·tan φ)/n1)` with (w1,n1) = (π/6,3), (π/3,1.5), (2π/3,1.5), (5π/6,3); converged to 1e-6, then `plac2` converts RA→λ: `f = atan2(sin ar, cos ε·cos ar)`.
- Koch `koch1` (L48015): `a = asin(sin RAMC·tan φ·tan ε)`, `b = 2h/π − 1`, `c = nb(RAMC + h + a·b)`, `k = atan2(sin c, cos c·cos ε − tan φ·sin ε)`.
- Regiomontanus `regio1` (L48146): `R = atan2(sin h·sin φ, cos φ·cos(RAMC+h))`, `k = R + ε`, cusp `= atan2(cos R·sin(RAMC+h), cos(RAMC+h)·cos k)`.
- Campanus `camp1` (L48203): `d = atan2(sin h·cos φ, cos h)`, cusp `= atan2(sin(RAMC+d), cos(RAMC+d)·cos ε − sin d·tan φ·sin ε)`.
- Topocentric `topo1` (L48245): `g = atan(k1·tan φ/3)` used as a pole latitude, then the AC formula with `a = RAMC − π/k2`, (k1,k2) = (1,3),(2,6),(2,−6),(1,−3).
- Equal `aeqe`: `f(n) = nb(AC + (n−1)·30°)`; Vehlow same minus 15° (`− PI/12`).
- `regio0` (L48162) normalises cusp ordering (adds π where the sequence would decrease) and mirrors 7–12 from 1–6.

Storage: `a61` (L25796) copies `f(1..12)` → `fz(od,ze,1..12)` plus `fz(od,ze,13)=AC`, `fz(od,ze,14)=MC`; `a611` restores. `a60_l` (L25779) zeroes cusps for `haw& ≥ 8`. House table output: `hausa` (L17623) prints cusps in longitude, RA, declination, plus for Placidus "AO" (oblique ascension) and "Polhöhe".

---

## 5. Coordinate transforms, obliquity, precession

| Routine | Line | Formula |
|---|---|---|
| `ko_tr1(br,la,ekls,VAR ar,de)` | 49427 | ecliptic → equatorial: `de = asin(sin b·cos ε + cos b·sin ε·sin l)`, `ar = atan2(sin l·cos ε·cos b − sin b·sin ε, cos l·cos b)` |
| `ko_tr2(ar,de,ekls,VAR la,br)` | 49435 | equatorial → ecliptic (inverse signs) |
| `atn(z,VAR fii)` | 42489 | quadrant fix: given `fii = atan(z/n)`, add π if `fii<0`, add π if `z<0`, the program's atan2 |
| `praez(jda,ar0,de0,VAR ar,de)` | 49473 | Newcomb precession referred to epoch 1900.0 (`jdbez = 2415020.313`): `ζ = puu·((2304.25+1.396·T0)·T + 0.302·T² + 0.018·T³)`, `z = ζ + puu·(0.791T² + 0.001T³)`, `θ = puu·((2004.682−0.853·T0)·T − 0.426T² − 0.042T³)`, with `T0 = (jda−jdbez)/(100·tja)`, `T = (jd−jda)/(100·tja)` |
| `praez_kartes_aeqn` / `praez_kartes_ekln` | 48542 / 48575 | IAU-1976 matrices (`//nach MONT. 2. S 18  Elemente nach MEEUS`) used only for `.EPH` vectors |
| Obliquity `ekls` | 25652, 49549 | `0.40931974745 − 0.0002271109689·t1 − 2.86234E-08·t2 + 8.779E-09·t3` (rad, mean, t1 from 1900); `somo` then does `ADD ekls, deps` so `ekls` normally holds the TRUE obliquity |
| Nutation `dpsi`, `deps` | 49586–49595 | Meeus abridged nutation, 10 terms in Δψ, 7 in Δε, scale `pp = puu/10000` (0.0001″ → rad). Set to 0 in heliocentric mode |

Latitude: geographic latitude `gg` (degrees, N positive, `gg$` "N"/"S"), longitude `gl` (degrees, E positive, `gl$` "E"/"W"); parsed in `zuort` (L23862) and `labr2` (L50168). Stored `gl(3,5)`, `gg(3,5)`. No flattening/geodetic correction anywhere (`par` explicitly notes `Ohne Ber. d.Meereshöhe`).

---

## 6. Aspects

### 6.1 Pipeline

`asp0` (L20462) sets the loop bounds per mode:

| | geocentric | heliocentric |
|---|---|---|
| `aa&` (first body) | 1 (or 0 with Fixpunkt) | 2 |
| `bb&` | 14 | 10 |
| `bb0&/bb1&/bb2&/bb3&` | 11 / 18 / 12 / 10 | 10 |
| `cc&/dd&/ee&` | 13 / 15 / 12 | 11 / 11 / 11 |
| `np&` | 12, or `18 + @zusp` when `klpl!` | same |

`asp10` (L20506) fills the flat working array `as(0..40)` from `plz(od,ze,·)` (or `mup()` in mundane mode `horm&=2`), with `as(13)=AC`, `as(14)=MC`.

### 6.2 `asp1` (L20531) — harmonic aspect scan

For each divisor `n& = 1 … nas&` (`nas& = nasp&`, default 12, max 16):

```
pn = 2π/n&                                   ' Grundwinkel
dd = orb * (orbe! ? orbe(n&) : pn/30)        ' base orb
for each pair (t&,w&) with t&<w& :
   o1 = @org(t&,1) ; o2 = @org(w&,1)
   dds = @orbis_discr2(o1,o2,dd,dds)         ' per-planet weighting
   n&=1  : hit if |wa1-wa2| < dds  (conjunction, drawn as a red dot)
   n&>=2 : for m& = 1..n&-1 → @asp11(...)
```

`asp11` (L20896):

```
w1 = nb(m&*pn - dds) ; w2 = nb(m&*pn + dds) ; w3 = |as(t&) - as(w&)|
@vergl2(w1,w2,w3)
IF w1 < w3 < w2 :
   asp(t&,w&) = m&*pn ; INC zh&(n&) ; INC az&(t&) ; INC az&(w&)
   INC aspz%(n&) ; INC asp%(n&)
   IF n& = 3 : record (t&,w&) in tgrt&()/wgrt&()    ' Schiemenz
   @aspz1(...)  ' draw
```

When `orbe!` (equal-probability mode) is active, only the classical multiples `m&` of each divisor are allowed (the big `SELECT n& / SELECT m&` block L20629–20682): n=3→{1,2}, n=4→{1,3}, n=6→{1,5}, n=8→{1,3,5,7}, n=9→{1,2,4,5,7,8}, n=10→{1,3,7,9}, n=12→{1,5,7,11}, etc.

### 6.3 Orbs

- `orb` global multiplier ("ORBIS-FAKTOR", 1.0 = 100 %).
- `or&(0..40)` per-body percentage weights (100 = neutral), edited in `orbis_pla` (L28409); `orbpl!` flags any deviation.
- `org(or&,nh&)` (L29699): `o = or&(or&)/(nh&*100)`.
- `orbis_discr2/3` (L11010/11018) combine two/three body weights into `dds`.
- Default orb when `orbe!=0`: `dd = orb * pn/30` (1/30 of the base angle → 12° conjunction, 6° opposition, 4° trine …).
- Gleichwahrscheinliche Aspekte (`orbe!`, `orbe(0..14)`, `orb$()`): user/preset table of base orbs in degrees stored in `KONSTA7P.INT`, converted at load (`orbe(i&) = pu*ABS(VAL(orb$(i&)))`, L9052). Preset `or$(i&) = 12/i` degrees for divisors 1..12, 2° for mirror points, 1° for midpoints (`orbis_asp` L28736). Rettig's own explanation in the dialog text: `"…werden Orbes gewählt, die" + h$ + "etwa GLEICHE Wahrscheinlichkeit für alle Aspekte mit Teiler 1 bis 12 aufweisen ! ( 1930 bis 2050 )"`.

### 6.4 Schiemenz counters (`asp1`, L20699–20760)

- Triga counter (`// Schiemenz Triga-Zähler:`): over all recorded conjunctions, any two sharing a body count as one "Triga" → `aspz%(14)`, `asp%(14)`; participating entries are zeroed to prevent double counting.
- Grand-trine counter (`// Schiemenz Großtrigon-Zähler:`): triples of recorded trines whose endpoints close a triangle → `aspz%(15)`, `asp%(15)`, with a degeneracy guard `NOT(nb(pl(ti)-pl(wi)) < 1 && …)`.

Change log `// Schiemenzschen Aspektezähler eingebaut 04.01.03` (L172); `// asp1 mit haw&=10 verbessert ( Schiemenz ) 13.11.08` (L13). With `haw& = 10` all of DR/DS/AC/MC are excluded from aspecting (L20591, L20595, L20898).

### 6.5 Midpoints (Halbsummen)

`halbs1` (L20139) runs three passes: `nh& = 1` (direct), `nh& = 2` (square), `nh& = 4` (semi-square). `halbs11` (L20191) / `halbs111` (L20221) test for each triple (t&,u&,w&):

```
ph  = nb((plz(u&) + plz(w&))/2)                ' midpoint
c1  = (nh&=1) ? 0 : π/nh&
hit if  nb(plz(t&) - dds + l·c1) < ph < nb(plz(t&) + dds + l·c1)
dd = orb * (orbe! ? orbe(14) : pu)             ' 1° base orb
```

Counters `halbsz1%…halbsz4%`, duplicate suppression via `drk!(41,41,41)`. Mirror points (`Spiegelungen an Kardinal-Achsen`) in `spieg1` (L42498) with base orb `orbe(13)` (2°).

### 6.6 Mundane aspects (`horm& = 2`)

- `mundh1` (L41992): `g = φ`, `arm = RAMC`, `aric = RAMC+π`, `aoac = RAMC + π/2`.
- `md11` (L42122) decides left/right half of the chart (`o!`/`w!`).
- `md` (L42075): ascensional difference `ad = asin(tan φ · tan δ)`, `sad = π/2 + ad` (diurnal semi-arc), `san = π/2 − ad`. Per quadrant `md` = meridian distance, `ade = md·ad/sad` (or `/san`), `phs = atan(sin(ade)/tan δ)` (the pole), `aoe = ar − ade` (oblique ascension) / `doe = ar + ade`.
- `mundan(la,br,VAR mup,phmu)` (L41998): ecliptic → equatorial → oblique ascension → `mup` = mundane position measured from the MC/IC.
- `mundhorp` (L42018) converts all `plz(od,ze,·)` to mundane; `mundhorh` (L42049) replaces the cusps by exact 30° divisions and saves the originals in `muh(13)`; `mureh` (L42066) restores.
- `mund` (L40821) is the UI driver; `mund1` (L40967) the table/graph generator. Change log `// Fehler bei Tabelle mundan verbessert 29.03.10` (L4) is the very last functional change in the listing.

---

## 7. Special factors

| Factor | Index | Routine / line | Algorithm |
|---|---|---|---|
| Mean lunar node | 11 (DR), 12 (DS) | `somo` L49564/49596 | `o(2) = nb(pu·(125.044555 − 1934.1361849·t11 + 0.0020762·t21 + t31/467410 − t41/60616000))`; `el(11) = nb(o(2) + dpsi)`, `el(12) = el(11)+π`, fixed speed `tb(11) = −0.00092422029` rad/d |
| True lunar node (`moknw!`) | 11/12 | `moko` L49617 | From the Moon's state vector: angular momentum `c = r × v`, `el11 = atan2(c1, −c2)` `//wahrer Knoten`; inclination `i4 = asin(√(c1²+c2²)/c)` `//MONT. S.78 oben`; `u4` = argument of latitude `//S.78 mitte`; node latitude `eb = asin(sin u4·sin i4)` |
| Apogäum / Lilith (`apogw!`) | `nk&(1)` = "AG" | `moko` L49655 / L49672 | True (osculating): `a4 = 1/((2/r4) − v²/(G·M))`, `p = c²/(G·M)`, `e4 = √(1 − p/a4)`, eccentric anomaly, true anomaly, then `el24 = nb(π + u4 − wa + el11)  //Apogäum`. Constants `g = 0.0002959122083 //Grav.Konst`, `m = 3.0404332E-06 //Masse Erde +Mond`. Mean (`apogw! = 0`): `el = nb(dpsi + pu·(180 + 83.3532430 + 4069.0137111·t11 − 0.0103238·t21 − t31/80053 − t41/18999000))` |
| Node/apogee speed | | `vel_om_pd` L49688 (`// Geschw. moment. Mondknoten und Apogäum`) | central difference over ±1 h (`djd = 0.0416666666666 // 1h`), recomputing `moko` three times |
| Chiron / Centaurs (Pholus, Nessus, Damokles) | `nk&(2)`, `nk&(19..21)` | `ephem_auswert` | `.EPH` files, section 2 |
| Part of Fortune / Glückspunkt | `nk&(4)` = "GL" | `a901` L16713 | day birth `el = nb(AC + el(Moon) − el(Sun))`; night `nb(AC + el(Sun) − el(Moon))`. Day/night from `ta_na` (L18903), Sun between AC and DC |
| Transpluto | `nk&(3)` = "TP" | `plelem` L49136 | fixed elements `a = 77.755, e = 0.3, i = 0, o = 0, p = 0.00076575972 + 0.024365·t1, man = nb(1.1659863 + 0.91638372749·t1)` + `kepler` |
| Uranian/Hamburg factors CU,HA,ZE,KR,AP,AD,VU,PO | `nk&(9..16)` | `plelem` L49146–49177 | circular orbits, only `a()` and a linear `man()`; `e=i=p=o=0` |
| Arabic points | table | `arabt` L18286, `arabl0/arabl/arabl1` L18796–18855 | Generic three-term formula `A + B − C`. Terms may be a planet (`c4&=0`), a house cusp (`c4&=12`), the ruler of a house (`c4&=13`, via `ze_pl`), or a fixed zodiac degree (`c4&=14`). User definitions persisted in `\INTERN\ARABTEI1.INT` (39-byte records: `4 AS jj$, 21 AS an$, 14 AS bem$`) and `ARABTEI2.INT` (16-byte: `4 AS jj$, 4 AS zz$, 4 AS c3$, 4 AS c4$`). Formula variant `af&` = 1 traditional / 2 always-day / 3 always-night |
| Fixed stars | | `stella` L17974 | 61 stars in two `DATA` blocks (name+rulers+astro name, RA°, dRA°/century, Dec°, dDec°/century, distance ly). Position `ar = ar + (t1−1)·dar`, `de = de + (t1−1)·dde` (linear, epoch J2000); for clusters and Galactic Centre/Apex instead full `praez` from B1950 (2433282.423) or 1988 (2447344.2481). Then annual aberration with `bb = 0.000099338` and `ko_tr2` to ecliptic. `stelk` (L18124 region) matches aspects against `pl()` with orb `2·orb` degrees |
| Großes Jahr | | `grossj` L9453, `grossj1` L9546 | `jdgross = 2370832 //CHAUVIN f.AQU.`, `zal_grossj& = 330` (Aquarius). Reference ecliptic longitude `la = pu·zal_grossj&` at `jdgross`, converted to equatorial, precessed with `praez` to the chart date, converted back; `di = (la1 − la)·up` is the age-point drift. UI comment `"Pro Zeichen ca. 2148 Jahre ~ 50.269"" pro Jahr"`. Options 30°/360°/330°/300° = Aries/Pisces/Aquarius/Capricorn |
| Personare | | L31099, L31985 | Menu-level chart type only (`ze$(6) = "PERSONARE"`); no separate astronomy. Change log `// Versuch mit Personaren 08.02.04` |
| Fixpunkt | 0 | `lpkt` L1983, `fixpunkt_def` L2015 | Pure user-entered degree (`fixpunkt$` / `fixpunkt_rh$`), converted to mundane when `horm&=2` |

---

## 8. Data flow: input → displayed chart

### 8.1 Chart slots

Everything is indexed by `od` (1 = radix / 2 = partner / 3 = outer ring, dim 0..3) and `ze` (chart number 1..5). `DIM` (L15759) declares the full store.

### 8.2 The chain

```
a3 (L23527)  input dialog (HORCOM or AAF format)
  └ eingabe() → zuo() L23808        parse date/time/place; DST, zone, LMT/LTT, Zeitgleichung
       @juld                        → jd (UT)
  └ @jseckp L25609:  @juld → @sidt → @eckp          (jd, t1..t4, h0, hs, armc, f(1),f(10),vert)
  └ @a316061 L25740: @a31 → @a60 → @a61             (persist scalars; compute + persist cusps)
  └ @ein_zuo / @direkt → display

a9 (L16695)  ← every display module calls this
  ├ @a311  L23649   restore slot scalars into working globals
  ├ @a90   L16700   @utet → @utet1 (UT→ET) → @juld1 → @plko → @moko → @vel_om_pd → @a901 → @etut
  └ @a92   L16781   copy results into the persistent per-slot arrays
```

### 8.3 Globals that carry results

Working (current epoch, radians unless noted)

| Array | Meaning |
|---|---|
| `hel()`, `heb()`, `r()` | heliocentric longitude, latitude, radius (AU) |
| `helt()`, `hebt()`, `hert()` | heliocentric rates per day |
| `el()`, `eb()` | geocentric apparent ecliptic longitude / latitude |
| `ar()`, `de()` | right ascension / declination |
| `dr()` | geocentric distance (AU) |
| `tb()`, `ttb()` | geocentric daily motion and its change (retrograde ⇔ `tb < 0`) |
| `pl(0..40)` | flat display array (planets + AC/MC + cardinal points) |
| `f(1..14)` | house cusps 1–12, `f(13)`=AC copy, `f(14)` unused/MC |
| `a(),e(),i(),o(),p(),man(),mel(),u(),v(),ean()` | orbital elements |
| `gom(),goms(),pdg(),pdga()` | ascending/descending node and perihelion/aphelion longitudes per body (`plko10`/`plko100` L49494/49508) |
| `dpsi, deps, ekls, delt` | nutation in longitude/obliquity, true obliquity, ΔT (minutes) |
| `armc, armcb, hs, h0` | RAMC (deg / rad), local sidereal time (h), GMST at 0h (h) |
| `as(0..40)`, `asp(41,41)`, `zh&(16)`, `az&()`, `aspz%()`, `asp%()` | aspect working set and counters |

Persistent per slot (`od`,`ze`), written by `a31`/`a61`/`a92`, read by `a311`/`a611`/`zuord1`: `ta&,mo&,ja&,ho,mi,jd,t1,t2,t3,armc,h0,hs,gl,gg,gl$,gg$,na$,go$,bem$,sol$,jul$,ndj&` and the arrays `plz(3,5,41)` longitudes, `ebz` latitudes, `arz` RA, `dez` decl, `tbz` speed, `ttbz` accel, `fz(3,5,15)` cusps (+13=AC, 14=MC), `vert(3,5)` vertex, `gomz/gomsz(3,5,2)` nodes.

### 8.4 Global switches to carry over

`hrg!` helio/geo · `haw&`/`haus$` house system · `appa&`/`appa$` apparent mode (1/2/3) · `par` parallax (1 = topocentric, 2 = off) · `apogw!` true vs mean Lilith · `moknw!` true vs mean node · `stzw&` true vs mean sidereal time · `klpl!` extra bodies on · `nk&(1..22)` slot numbers of the extra bodies · `np&` number of bodies · `orb`, `or&()`, `orbe!`, `orbe()`, `nasp&` aspect config · `horm&` 1 = ecliptic / 2 = mundane · `fza` chart rotation offset · `jdplanetex!(41)` per-body outside-ephemeris-range flag.

Persistence: `kon_dsp`/`kon_dhol` (L8986/L9022) read/write `\INTERN\KONSTA7P.INT`, the exact field order at L9031–9088 is the settings-file schema.

### 8.5 Constants (`funkt` L9256)

```
po = π/2 ; pu = π/180 ; up = 180/π ; puu = pu/3600 ; pup = pu/36525 ; pv2 = 2π
tja = 365.24219878 ; kk = 1.0E-10          ' kk = epsilon guard against /0 and atan branch
DEFFN ng(x) = x - 360*INT(x/360)           ' normalise degrees
DEFFN nb(x) = x - pv2*INT(x/pv2)           ' normalise radians
DEFFN nh(x) = x - 24*INT(x/24)             ' normalise hours
DEFFN wz(x) = x - 30*FIX(x/30)             ' degree within sign
```

`vergl1/vergl1r/vergl2/vergl2r` (L32169–32194) are the ±2π branch reconcilers used before every angular comparison. Port them verbatim, the aspect logic depends on their exact asymmetry.

### 8.6 Root finding (transits, ingresses, solars)

`plant(jdz,pl&,pz,VAR ns1&)` (L32369) searches the JD at which body `pl&` reaches longitude `pz`. Per-body initial step `dt1` and convergence tolerance `tbg` (e.g. Mercury 1°, Venus ½°, Mars 25′, Jupiter 8′, Saturn/Chiron 5′, Uranus 2.5′, Neptune/Pluto/Quaoar/Xena 1.6′). `plant1(pl&)` (L32195) is the evaluation kernel: `dat → juld1 → [sidt/eckp if par=1] → utet → utet1 → juld1 → soko/moko/plko_einz → etut → a316061`, returning `nb(el(pl&))`. `ingre1` (L10637) drives sign ingresses in 30° steps (`pz = (t&-1)·π/6`); `korh` (L16106) wraps `plant` for solar/lunar returns.

---

## 9. The 15 routines to port first

A working calculation core (Sun–Pluto + Moon + nodes + houses + aspects) needs, in dependency order:

1. `funkt` constants + `nb/ng/nh` + `atn` + `vergl1/vergl2` — L9256, L42489, L32169. Everything else depends on the exact branch handling.
2. `juld` / `dat` — L25614 / L25655. JD ↔ calendar with Julian/Gregorian override.
3. `juld1` — L25642. `t1..t4`, `t11..t41`, `tja`, mean obliquity; calls `somo`.
4. `utet` / `utet1` / `etut` — L17944–17973. ΔT table + extrapolation; the planets are computed in ET.
5. `somo` — L49547. Sun/Moon mean elements, nutation Δψ/Δε, true obliquity, mean node, `gom/goms`.
6. `initpterm` / `readpterm` — L48841 / L48855. VSOP index+data loader (`PLANETS.NDX`, `PLANETS.DAT`).
7. `plposhi` — L48913. VSOP series evaluation with analytic velocities.
8. `plelem` — L49045. Meeus mean elements for all bodies + extra-body element table.
9. `kepler` — L48814. Kepler solver (Odell start, Newton iteration).
10. `ephem_auswert` + `ipol` / `ipol3` + `praez_kartes_aeqn` / `praez_kartes_ekln` — L48309, 48492, 48526, 48542, 48575. The whole `.EPH` path.
11. `hel_geo` + `plko12` + `verv` — L49321, 49374, 49366. Heliocentric → geocentric with numeric velocity.
12. `soko` / `moko` / `moko1` — L49195, 49617, 49744. Sun, Moon (Meeus tables), true node and Lilith.
13. `plko` / `plko_einz` / `pl_el_ve` / `par_ap_ktr` / `plkoap` / `plkotr` / `par` / `ko_tr1` / `ko_tr2` — L49304, 49241, 49213, 49393, 49451, 49443, 49398, 49427, 49435. The driver plus all output conversions.
14. `sidt` + `eckp` + `eckp1` — L37888, 25712, 25719. Sidereal time, RAMC, AC/MC/Vertex.
15. `a60` dispatcher + `plac/plac1/plac2`, `koch/koch1`, `regio/regio1/regio0`, `camp/camp1`, `topo/topo1`, `aeqe`, `vehlow` — L25762, 47998–48256, 26014, 26024. All house systems.

Immediately after those, for a usable chart: `asp0` / `asp10` / `asp1` / `asp11` / `org` / `orbis_discr2` (L20462, 20506, 20531, 20896, 29699, 11010) and the slot-persistence pair `a31`/`a311` + `a61`/`a611` + `a92` (L23624, 23649, 25796, 25814, 16781).
