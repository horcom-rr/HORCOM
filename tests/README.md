# tests

Verification for the port, per `docs/architecture.md` section 5.

- `golden/` will hold fixture files recorded from the original `HORCOM7P.EXE` (coordinate tables, house tables, parallax on/off pairs) for a grid of test epochs.
- Unit and property tests live next to their fixtures, built on doctest.
- Fixture data must be synthetic or derived from the program itself, never copied from personal data files under `legacy/`.
