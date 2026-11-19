# Contributing to HORCOM

Thank you for wanting to help! HORCOM is the rewrite of the astrology program Robert Rettig developed from 1989 to 2010. It aims to keep his program alive as he built it, so a few rules here are stricter than in other projects. This page explains them.

HORCOM has a single maintainer, so an answer can take a few days. Issues and pull requests are welcome in English or German, and questions about using the program fit well in [Discussions](https://github.com/horcom-rr/HORCOM/discussions). Everyone taking part follows the [code of conduct](CODE_OF_CONDUCT.md).

## Ways to help

- **Report a problem.** Open an issue with what you did, what you expected and what HORCOM showed. A screenshot helps. If you know the original HORCOM, a screenshot of the original beside the new program helps most.
- **Test on your system.** Reports from Linux distributions and Windows versions not tried yet are welcome, also when everything works.
- **Improve the English.** The English wording of the program lives in `apps/gui/i18n/horcom_en.ts`, the handbook in `docs/handbook/`.
- **Fix a bug or a build problem**, for example a warning from a new compiler.

For anything bigger than a small fix, please open an issue first, so the approach is agreed before you spend time on it.

## Please use made-up birth data

Charts hold personal data. Please don't post real people's birth data in issues or pull requests, made-up data that shows the problem is enough. Test data added to the repository must be fictional too.

## The original defines what is right

- When the rewrite and the original disagree, the original is assumed right until shown otherwise.
- Menus, captions, dialogs and screens follow the original. Modern additions are welcome where the platform needs them, but they don't replace his structure.
- A calculation the original got wrong is fixed, never copied. Such a fix needs an independent reference (Meeus, orbital elements, finite differences or Robert Rettig's own stated intent) and a test that pins the corrected value. It is listed under Known original bugs in [`docs/architecture.md`](docs/architecture.md).
- Robert Rettig's own comments in the code are marked `//RR` and keep his German wording.
- The original sources are not in the repository for now. Changes that need a look into them are done by the maintainer, so an issue describing the problem is the best start. [`docs/legacy/`](docs/legacy) maps the original program, and `docs/architecture.md` explains the design of the rewrite.

## Downloads and packaging

- The official downloads are the Windows zip and the Linux AppImage that the project's CI builds and publishes for each release. The official install steps only use those.
- Install scripts that download from other places aren't accepted, and native .deb and .rpm packages aren't planned for now.
- Packages kept in a distribution's own place, like the AUR, are welcome as community packages. Open an issue and they can be listed in the installation guide.

## Code

- C++20 with CMake, and Qt 6 for the desktop program. `build.bat` on Windows and `build.sh` on Linux configure, build and run every test, the [README](README.md#building-from-source) has the details.
- Warnings are errors on MSVC, GCC and Clang, so a change has to build without warnings on all three. CI checks every pull request, for new contributors once the maintainer approves the run.
- Match the surrounding code: two space indent, `///` Doxygen comments with `@param` and `@return` on public functions, and short comments that say why, not what.
- Recurring values such as epochs, unit conversions and physical constants are named `constexpr` constants, shared ones live in `src/horcom/core/constants.hpp`.
- A change in behaviour comes with a test in `tests/` (doctest). If a change can move a calculated number, also run `tools/compare_golden.py` against the recorded output of the original program, see `tests/golden/README.md`.
- German is the source language of the program. New texts in the interface get their English translation in `apps/gui/i18n/horcom_en.ts`, the `update_translations` target adds the new entries there.
- If a change alters what users see, the matching handbook page in `docs/handbook/` is updated in the same pull request. All documentation is in English.

## Pull requests

- One topic per pull request keeps it easy to review.
- The maintainer squash-merges and writes the commit message in the project's format (`type/place: message`, for example `fix/render: ...`), so you don't need to worry about it.

## License

HORCOM is licensed under the GPL-3.0-or-later. By contributing you agree that your contribution is published under the same license. New source files start with this header, with your own name in the copyright line:

```
// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Your Name
```

Thank you for helping to keep Robert Rettig's work alive.
