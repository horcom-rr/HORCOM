# tests

Verification for the port, per `docs/architecture.md` section 5.

- `golden/` holds the coordinate tables recorded from the original `HORCOM7P.EXE` for 50 synthetic charts beside the rewrite's output for the same charts, see `golden/README.md`. `tools/compare_golden.py` compares the two, and the ctest `cli_golden` runs the command line tool on every chart and fails when its output drifts from the recorded new side.
- Unit and property tests live next to their fixtures, built on doctest. The GUI tests in `gui/` drive the MainWindow and its dialogs on the offscreen platform.
- Fixture data must be synthetic or derived from the program itself, never copied from personal data files under `legacy/`.
