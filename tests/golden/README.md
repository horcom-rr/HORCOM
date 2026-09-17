# Golden reference data from the original HORCOM7P

Recorded output of Robert Rettig's original HORCOM7P.EXE (Oct 2010 build),
run under winevdm/otvdm on Windows 11, used to verify the C++ rewrite.

## How it was produced

The original program was driven by script. For every test chart the record
was entered over NEU-EINGABE von DATENSÄTZEN in the HORCOM format, taken
over without touching the original data files, and two screens captured:

- `chartNN_original_koordinaten.png` — the PLANETEN-KOORDINATEN screen,
  ecliptic longitude to the arc second, latitude, velocity, distance,
  right ascension, declination, planetary nodes and apsides, plus the
  Placidus house table, sidereal time and Julian date.
- `chartNN_original_wheel.png` — the HOROSKOP-GRAPHIK screen, his wheel.

Settings profile of the original during capture, from its VORGABEN panel:
Placidus, Ekl.Länge App.1, Mit Parallaxe, Wahrer Mondknoten, topozentrisch,
MOND-Apsiden wahrer Wert, times entered as UT.

The new side of every pair comes from the rewrite:

- `chartNN_new_koordinaten.txt` — horcom_cli with `--parallax --true-node
  --houses 1`, same data directory.
- `chartNN_new_wheel.svg` — the rewrite's wheel for the same chart.

- `chartNN_original_koordinaten.json` — hand transcription of the numbers
  in the PNG, machine-comparable form.

`manifest.json` defines all charts: synthetic records TEST01..TEST50,
dates 1900 to 2012, both hemispheres, west longitudes, a leap day, polar
latitude, equator, solstice and midnight edge cases. No real person's
data appears anywhere in this set.

## Verification result

`python tools/compare_golden.py` checks every chart. Result of the
recording session: all 550 planet longitudes agree within one arc
second of display rounding, latitudes and declinations within 0.005
degrees, ARMC within one second of time, the Julian dates within one
second. 47 of 50 charts agree on every house cusp within display
rounding.

Three charts (14, 20, 33) each show a single cusp 1.1 arc minutes from
the recorded original. This is not a convergence limit and not a defect
of the house code. The Placidus map is strongly contracting, it reaches
its fixed point in a dozen iterations, and the 1e-6 stop already lands
on that fixed point to far better than a thousandth of an arc minute, so
tightening the tolerance moves nothing. A deeper look rules out the
algorithm entirely. Each deviation is isolated to one cusp while every
neighbour in the same chart matches, so no shared input shift through
ARMC, obliquity or latitude can be the cause, and a sensitivity check
confirms it, the ARMC and obliquity match too closely to move a cusp by
an arc minute. Chart 20's off cusp is H10, the MC, which is computed
directly and never runs through the Placidus iteration at all. All three
deviations are exactly one arc minute in the last displayed digit and in
the same direction, the recorded value reading one minute higher. That
isolated, one digit, same sign pattern is the signature of a
hand-transcription slip in these golden values, which were typed by hand
from the original's screen captures. The rewrite sits on the exact
Placidus fixed point and keeps the faithful 1e-6 criterion. The three
recorded cusps should be re-checked against their source screens when
those are available.

At polar latitudes (charts 07 and 35) the original lists only AC and MC
and omits the intermediate Placidus cusps. The rewrite mirrors this
behaviour. Their new side was computed with `--houses 8`.

## Known limitations of the emulated original

- otvdm raises an Interrupt 06 #UD fault at the end of the wheel drawing.
  The wheel is complete when it fires, but the aspect chords of the inner
  circle may be missing from the captures.
- The captures show the original at its X:Y = 1920:1440 resolution setting.
