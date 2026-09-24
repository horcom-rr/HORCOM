#!/usr/bin/env sh
# SPDX-License-Identifier: GPL-3.0-or-later
# horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
# Copyright (c) 2026 Dominik Schwimmbeck
#
# The Linux and macOS twin of build.bat. Configures Ninja, builds and runs
# the tests. Pass a Qt prefix as QT_PREFIX when Qt 6 is not on the system.
set -eu
root=$(cd "$(dirname "$0")" && pwd)
cmake -S "$root" -B "$root/build" -G Ninja -DCMAKE_BUILD_TYPE=Release ${QT_PREFIX:+-DCMAKE_PREFIX_PATH="$QT_PREFIX"}
cmake --build "$root/build"
ctest --test-dir "$root/build" --output-on-failure
