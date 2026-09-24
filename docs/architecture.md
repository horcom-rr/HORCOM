# horcom C++ Architecture

The rewrite plan, derived from the four legacy analyses in `docs/legacy/`. Read those first for any subsystem detail; this document decides how the new program is built.

## 1. What HORCOM is, in one paragraph

An event-driven chart calculator. Input (date, time, place) is normalised to UT through zone/DST/LMT logic, converted to Julian date and then ET via a ΔT model. Planet positions come from three engines: truncated VSOP87 series (Sun through Neptune, from a binary term table), a Meeus Moon theory, and self-integrated binary ephemerides for Pluto, Chiron, the centaurs, the big asteroids, Quaoar, Eris and Halley. Positions pass through light-time/aberration, equatorial conversion and (optionally) Robert Rettig's topocentric parallax correction. Houses (7 systems), aspects (harmonic scan with per-body orbs), midpoints, mundane positions and a dozen special factors complete the chart, which is rendered on a 640×480 virtual canvas and dispatched through one universal output-screen driver with ~60 menu features.

## 2. Module layout

```
src/
  core/     Constants and angle plumbing. funkt constants (pu, up, puu, tja, kk),
            normalisation nb/ng/nh/wz, the atn quadrant fix, the vergl1/vergl2
            branch reconcilers. Ported verbatim; everything depends on their
            exact branch behaviour.
  time/     Julian date <-> calendar with the Julian/Gregorian override (juld/dat),
            time arguments t1..t41 (juld1), delta T (utet), sidereal time (sidt),
            equation of time (zeitgleichung), zone/DST/LMT input normalisation.
  ephem/    Position engines. VSOP term-table loader and evaluator
            (readpterm/plposhi), Moon (moko1/somo), Kepler solver, mean elements
            (plelem), Chapront Pluto fallback (pl_ko), the .EPH reader
            (ephem_auswert + ipol/ipol3), nutation and obliquity (somo),
            precession (praez, praez_kartes_aeqn/ekln).
  chart/    The chart pipeline. Dispatcher (plko/plko_einz), helio->geo (hel_geo),
            apparent position (plkoap), PARALLAX (par) [protected, see 4],
            coordinate transforms (ko_tr1/ko_tr2), AC/MC/Vertex (eckp1), all house
            systems, aspects (asp0/asp1/asp11 + orbs), midpoints (halbs*), mundane
            positions (md/mundan), special factors (true node and Lilith from moko,
            Part of Fortune, Arabic parts, fixed stars, Grosses Jahr), transit and
            return root finding (plant).
  data/     Byte-exact loaders for the legacy formats (chart .DAT, places .INT,
            KONSTA7P settings, PLANETS.NDX/DAT, .EPH, AAF, statistics), per
            docs/legacy/data-formats.md, plus the port's own storage later
            (SQLite; AAF stays the interchange format).
  geo/      Places gazetteer and the historical timezone data. The 177-entry
            ZEITZONN table (with pre-1900 local mean times) becomes seed data;
            IANA tzdata covers the modern era.
  render/   Chart geometry as data: ring radii, polar mapping, de-clumping,
            colours and draw order per docs/legacy/ui-and-graphics.md. Produces a
            backend-neutral display list so screen, PNG/SVG export and print are
            one code path.
  app/      The GUI shell, Qt 6 Widgets. The display list renderer feeds
            QPainter directly and the licence sits cleanly with the project.
            The rest of the tree never includes UI headers, the CLI front
            end came first for testing.
tests/      Golden-value tests against the original program plus unit tests.
```

Dependency direction is strictly downward: `app -> render -> chart -> ephem -> time -> core`, with `data` and `geo` as leaves used by `chart`/`app`. The calculation library must build and run headless.

## 3. Design decisions

1. **Radians and doubles everywhere.** The original computes in radians with GFA 8-byte floats. Rettig's own porting advice (EPHEMER5 header): time variables need at least 12 significant digits — `double` satisfies this. His exact constants are kept, including `tja = 365.24219878/9` and the `kk = 1e-10` epsilon where behaviour depends on it.
2. **Same numbers first, better structure second.** Every ported routine first reproduces the original's output (golden tests, see 5), then may be refactored under those tests. Algorithm upgrades (IAU 2006 precession, modern ΔT, Swiss-Ephemeris-grade positions) are explicitly out of scope for the port and become opt-in engine variants later.
3. **His data structures are kept where they carry the design.** The body index scheme 0..40 (0 Fixpunkt, 1 Sun … 10 Pluto, 11/12 nodes, 13/14 AC/MC, 19+ extra bodies via the `nk&` slot table) stays as a typed enum with identical numbering so every formula and file format reads literally. The chart-slot model (`od` 0..3 × `ze` 0..5) becomes a `ChartSet` type with the same shape. The int32-scaled 16-byte `.EPH` record stays the on-disk ephemeris format, read by a byte-exact loader — those files are his data legacy and remain valid.
4. **Verbatim-semantics kernel.** `nb/ng/nh`, `atn`, `vergl1/vergl2`, `ipol/ipol3` are ported instruction-for-instruction, tests pinning their edge cases, because aspects and interpolation depend on their exact branch behaviour.
5. **Comments travel with the code.** Per CLAUDE.md, Rettig's comments are carried into the ported routines in his German wording where they still apply, marked as his.
6. **Dropped subsystems** (decided, documented): the KENNTS copy protection and demo gates (contain personal data, no value), NOTIZ/AEND7 vestiges, the resolution-locked tiled BMP caches (regenerate instead), MSPAINT/clipboard export (native PNG/SVG export instead), the temp-file KILL/NAME edit pattern (atomic writes instead).
7. **Known original bugs** are reproduced by the first port, then fixed knowingly, each with a test documenting the deviation:
   - Halley precessed from B1950 through a duplicated `CASE` (should be J2000, ~0.7° error) — `calculation-core.md` 2.5, `tools-and-modules.md` 6.
   - Velocity interpolation offset `jdip − djd/2` mixes a dimensionless argument with days — affects displayed speed only.
   - `ORT.EXT` writer guard inverted (write-once file).
   - `.STA`/`.STH` append desync (`RECORD #26,k&` with stale `k&`).
   - `reg_resdat` restores the wrong place-file backup name.
   - `textg` overlapping width ranges (19/20).
8. **Personal data** never enters the repo (CLAUDE.md). Loaders are tested with synthetic fixture files, not with files from `legacy/`.

## 4. Protected logic (CLAUDE.md "Preserving Robert Rettig's engineering")

The cluster `par` (topocentric parallax, Montenbruck p.25 formulation in equatorial coordinates), `ko_tr1`/`ko_tr2`, `praez`, `plkoap` (L49393–49493) and the osculating-orbit true node/Lilith in `moko` are ported faithfully first and verified against the original program's printed coordinates before any change is even proposed. The parallax path notably includes the `par = 1` variant of the transit search (`plant1` recomputes sidereal time per step so the topocentric correction tracks the event location). When old and new disagree, the original is presumed right.

## 5. Verification strategy

1. **Golden values from the original.** `HORCOM7P.EXE` (16-bit) runs under winevdm/otvdm or a 32-bit VM; the GFA IDE (unpacked from `legacy/GFAWIN`) can also run the source directly. For a grid of test epochs (spanning Julian/Gregorian, BC years, the .EPH coverage bounds, DST cases) we record the program's planet coordinate tables (menu 37/38), house tables (menu 80), and parallax on/off pairs, and store them as fixture files.
2. **Data-derived checks.** The `.EPH` binaries themselves are ground truth for the reader; `PLANETS.DAT` record 0 (Earth L0 = 1.75347045673) and friends pin the VSOP loader.
3. **Property tests** for the kernel (normalisation, atan quadrants, calendar round-trips, interpolation continuity at record boundaries).
4. Each module lands only with its tests; nothing merges on visual inspection alone.

## 6. Phases

| Phase | Deliverable |
|---|---|
| 0 | Toolchain (compiler + CMake + test framework), CI-less local test run |
| 1 | `core` + `time` (constants, angles, JD, ΔT, sidereal time, equation of time) with golden tests |
| 2 | `ephem` (VSOP loader/eval, Moon, Kepler, .EPH reader, nutation, precession) — planet longitudes match the original |
| 3 | `chart` calculation (geo conversion, apparent, parallax, houses, aspects, midpoints, special factors) — full coordinate + house tables match |
| 4 | `data` + `geo` (legacy loaders, AAF, settings; timezone seed data) — existing user files open |
| 5 | `render` display list + PNG/SVG export — the wheel reproduces the legacy layout (radii table in ui-and-graphics.md) |
| 6 | `app` GUI shell (framework decision due here), transits/directions/returns screens |
| 7 | Extras: statistics module, Münchner Rhythmenlehre/Huber, primary directions, eclipse finder, clock chart |

## 7. Toolchain

Target: C++20, CMake ≥ 3.25, warnings-as-errors, `doctest` (vendored single header) for tests. MSVC 2022 is the primary target on Windows. Linux builds with GCC ≥ 12 and Clang ≥ 18 under `-Wall -Wextra -Wpedantic -Werror` and `-ffp-contract=off`, so no compiler fuses Robert's multiply-add steps and the numbers stay byte identical to MSVC (verified over 600 randomised charts). CI runs GCC, Clang and an ASan/UBSan build on Ubuntu 24.04 and packs the release AppImage on Ubuntu 22.04 with Qt 6.8.3 (`packaging/linux/appimage.sh`). On a read-only install (AppImage, `/usr`) the GUI works in `~/.local/share/horcom/data`, seeded from the shipped data, while planet tables and ephemerides stay read from the install.

## 8. References Robert Rettig worked from

Jean Meeus, "Astronomische Algorithmen" (VSOP87 abridgement, Moon, nutation, mean elements — his own citation is preserved in `data-formats.md` 8.2). Oliver Montenbruck, "Grundlagen der Ephemeridenrechnung" (parallax p.25, node/orbit geometry p.75/78, precession p.18/33, Halley p.165). "Coordinates of the Outer Planets 1653–2060", Astronomical Papers Vol. XII, Washington 1951 (the Störmer method, which he cites down to the Staatsbibliothek shelf mark). Guthmann (RK6, p.256). Chapront (Pluto series). P.D. via B. Mahl (timezone tables), B. Mahl (Arabic parts). The Aramis GFA-to-C++ converter in `legacy/HORCOM/GFA/` is not used for the port (it generates BASIC-emulation wrapper code) but may serve as a semantics reference for GFA constructs.
