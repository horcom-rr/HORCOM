#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
# Copyright (c) 2026 Dominik Schwimmbeck
#
# Builds, tests and packs horcom as one AppImage. Build on the oldest
# distribution the image should run on, its glibc sets the floor.
#
#   QT_ROOT_DIR     Qt 6 prefix, e.g. /opt/Qt/6.8.3/gcc_64 (required)
#   HORCOM_VERSION  release number stamped into the GUI, default 0.0
#   OUTPUT          file name of the image, default horcom-linux-x86_64.AppImage
set -euo pipefail

root=$(cd "$(dirname "$0")/../.." && pwd)
work=${WORK_DIR:-$root/build-appimage}
version=${HORCOM_VERSION:-0.0}
output=${OUTPUT:-horcom-linux-x86_64.AppImage}
: "${QT_ROOT_DIR:?set QT_ROOT_DIR to the Qt 6 prefix}"

linuxdeploy_tag=1-alpha-20251107-1
qt_plugin_tag=1-alpha-20250213-1

mkdir -p "$work/tools"
cmake -S "$root" -B "$work/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_PREFIX_PATH="$QT_ROOT_DIR" -DHORCOM_VERSION="$version"
cmake --build "$work/build"
ctest --test-dir "$work/build" --output-on-failure

appdir="$work/AppDir"
rm -rf "$appdir"
DESTDIR="$appdir" cmake --install "$work/build"

# Liberation Mono is the metric twin of the Courier New the sheets are
# laid out in. It rides along for hosts that have neither
fonts="$appdir/usr/share/horcom/fonts"
mkdir -p "$fonts"
for face in /usr/share/fonts/truetype/liberation2/LiberationMono-{Regular,Bold}.ttf; do
  if [ -f "$face" ]; then
    cp "$face" "$fonts/"
  fi
done
if [ -f /usr/share/doc/fonts-liberation2/copyright ]; then
  cp /usr/share/doc/fonts-liberation2/copyright "$fonts/LICENSE-Liberation"
fi

fetch() {
  if [ ! -x "$work/tools/$2" ]; then
    wget -q -O "$work/tools/$2" "https://github.com/linuxdeploy/$1/releases/download/$3/$2"
    chmod +x "$work/tools/$2"
  fi
}
fetch linuxdeploy linuxdeploy-x86_64.AppImage "$linuxdeploy_tag"
fetch linuxdeploy-plugin-qt linuxdeploy-plugin-qt-x86_64.AppImage "$qt_plugin_tag"

export APPIMAGE_EXTRACT_AND_RUN=1
export QMAKE="$QT_ROOT_DIR/bin/qmake"
export LD_LIBRARY_PATH="$QT_ROOT_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
# offscreen lets CI start the packed GUI without a display
export EXTRA_PLATFORM_PLUGINS="libqwayland-egl.so;libqwayland-generic.so;libqoffscreen.so"
export LDAI_OUTPUT="$output"
export LDAI_UPDATE_INFORMATION="gh-releases-zsync|horcom-rr|HORCOM|latest|horcom-linux-x86_64.AppImage.zsync"
export LINUXDEPLOY_OUTPUT_VERSION="$version"
PATH="$work/tools:$PATH" "$work/tools/linuxdeploy-x86_64.AppImage" \
  --appdir "$appdir" \
  --executable "$appdir/usr/bin/horcom_gui" \
  --desktop-file "$appdir/usr/share/applications/io.github.horcom_rr.horcom.desktop" \
  --icon-file "$appdir/usr/share/icons/hicolor/scalable/apps/io.github.horcom_rr.horcom.svg" \
  --plugin qt \
  --output appimage
