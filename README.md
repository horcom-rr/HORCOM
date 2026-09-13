# horcom

A modern C++ rewrite of **HORCOM**, the astrology program that **Robert Rettig** wrote and refined over more than three decades, from the 1970s until 2010.

## About the original

HORCOM was Robert Rettig's life project. Written in GFA BASIC, it grew into a complete professional astrology suite that he distributed on CD and through his website. It computed and drew birth charts, transits, solar and lunar returns, composite and double charts, and a live clock chart. He did not stop at the standard planets. When no ephemerides existed for bodies he cared about, he computed his own, integrating the orbits of Chiron, Pholus, Nessus, Damokles, Quaoar, Eris and Halley's comet numerically and shipping the results as ephemeris files spanning centuries. The program carried interpretation texts, a statistics module, historical time zone tables for dozens of countries and a places database, all built and maintained by him.

He wished for HORCOM to live on in C++. This project is that rewrite, done carefully rather than quickly, staying faithful to his algorithms and keeping his own comments alive in the ported code.

## Repository layout

| Path | Purpose |
|---|---|
| `reference/` | Verified UTF-8 copies of his original GFA BASIC listings, the factual base of the port (local only, not in git) |
| `legacy/` | The complete original archive, programs, data and documents (local only, not in git, contains private data) |
| `docs/` | Architecture notes, the map of the original program, the rewrite plan and the handbook (`docs/handbook/index.html`) |
| `src/` | The new C++ implementation |
| `tests/` | Tests, including comparisons against the original program's results |

## Principles

- The original program defines correct behavior. Where the rewrite deviates, the deviation is documented.
- Robert Rettig's own comments are carried over into the C++ code wherever they still apply, in his words.
- Personal data from the original archive (chart collections, customers, keys) never enters this repository.

## Status

Analysis complete, implementation not yet started. The original program (50,397 lines, 1,095 procedures) has been mapped end to end:

- `docs/legacy/calculation-core.md` — the astronomical engine (VSOP series, Moon theory, his integrated ephemerides, parallax, houses, aspects) with the porting order
- `docs/legacy/ui-and-graphics.md` — the full menu tree (the feature inventory), event loop and the exact chart geometry
- `docs/legacy/data-formats.md` — byte-level specifications of every file format
- `docs/legacy/tools-and-modules.md` — the ephemeris production chain (Runge-Kutta + Störmer integration) and the survey of all standalone tools
- `docs/architecture.md` — the target C++ design, verification strategy and phase plan

Implementation is under way. Ported and green so far: phase 1, the calculation kernel with calendar, delta T and sidereal time. Phase 2, the position engines, VSOP series, Moon, Kepler orbits, his integrated ephemeris files, nutation, precession and the Chapront Pluto fallback. Phase 3, the chart pipeline, all seven house systems, the correction chain with his protected topocentric parallax, lunar nodes and Black Moon, and the Part of Fortune. Everything is verified against Meeus's worked examples, the binary data files and independent cross computations. The growing handbook lives at `docs/handbook/index.html`, his ephemerides and term tables ship in `data/`. Next are the aspect engine and the first golden fixtures recorded from the original program.

## License

GPL-3.0-or-later, see `LICENSE`. The rewrite stays open, every derivative stays open, and Robert Rettig's name stays attached to his work. Maintained by Dominik Schwimmbeck.

---

*In memory of Robert Rettig, who built all of this first.*
