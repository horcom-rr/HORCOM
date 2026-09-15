# SPDX-License-Identifier: GPL-3.0-or-later
# horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
# Copyright (c) 2026 Dominik Schwimmbeck
"""Regenerates assets/symb from Robert Rettig's original symbol bitmaps.

Each 24 pixel *_G.BMP from the symbbmp folder of the local archive is
traced into bezier outlines with potrace (pip install potracer) and
rasterized supersampled to a 384 pixel RGBA sprite, black ink in the
alpha channel. Corners stay corners, curves become smooth strokes, so
the wheel zoom and the SVG export stay clean at any size.

Usage: python tools/trace_sprites.py <path to symbbmp>
"""
import glob
import os
import sys

import numpy as np
import potrace
from PIL import Image, ImageDraw

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "symb")
OUT_SIZE = 384
SUPERSAMPLE = 4


def flatten_curve(curve, steps=32):
    pts = [(curve.start_point.x, curve.start_point.y)]
    for seg in curve:
        p0 = pts[-1]
        if seg.is_corner:
            pts += [(seg.c.x, seg.c.y), (seg.end_point.x, seg.end_point.y)]
        else:
            c1, c2, p3 = seg.c1, seg.c2, seg.end_point
            for i in range(1, steps + 1):
                t = i / steps
                mt = 1 - t
                pts.append((mt**3 * p0[0] + 3 * mt * mt * t * c1.x + 3 * mt * t * t * c2.x + t**3 * p3.x,
                            mt**3 * p0[1] + 3 * mt * mt * t * c1.y + 3 * mt * t * t * c2.y + t**3 * p3.y))
    return pts


def signed_area(pts):
    return sum(pts[i][0] * pts[(i + 1) % len(pts)][1] - pts[(i + 1) % len(pts)][0] * pts[i][1]
               for i in range(len(pts))) / 2


def vectorize(src):
    ink = np.array(Image.open(src).convert("L")) < 128
    path = potrace.Bitmap(ink).trace(turdsize=1, alphamax=1.0, opttolerance=0.2)
    scale = OUT_SIZE * SUPERSAMPLE / ink.shape[0]
    img = Image.new("L", (OUT_SIZE * SUPERSAMPLE, OUT_SIZE * SUPERSAMPLE), 0)
    draw = ImageDraw.Draw(img)
    for curve in path:
        pts = [(x * scale, y * scale) for x, y in flatten_curve(curve)]
        # potrace runs holes against the outline orientation, the y down
        # raster flips the sign
        draw.polygon(pts, fill=0 if signed_area(pts) > 0 else 255)
    mask = img.resize((OUT_SIZE, OUT_SIZE), Image.LANCZOS)
    out = Image.new("RGBA", (OUT_SIZE, OUT_SIZE), (0, 0, 0, 0))
    out.putalpha(mask)
    return out


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 1
    src_dir = sys.argv[1]
    stems = [os.path.splitext(os.path.basename(f))[0]
             for f in sorted(glob.glob(os.path.join(OUT_DIR, "*.png")))]
    done = 0
    for stem in stems:
        src = os.path.join(src_dir, stem + "_G.BMP")
        if not os.path.exists(src):
            print("missing source for", stem)
            continue
        vectorize(src).save(os.path.join(OUT_DIR, stem + ".png"))
        done += 1
    print("traced", done, "of", len(stems))
    return 0


if __name__ == "__main__":
    sys.exit(main())
