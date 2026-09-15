# SPDX-License-Identifier: GPL-3.0-or-later
# horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
# Copyright (c) 2026 Dominik Schwimmbeck
"""Regenerates assets/symb from Robert Rettig's original symbol bitmaps.

Each 24 pixel *_G.BMP from the symbbmp folder of the local archive is
upscaled with one scale3x pass, which straightens single pixel stairs
without inventing curves, then filtered down to a 384 pixel RGBA
sprite with black ink in the alpha channel. The pixel character of his
drawings stays untouched.

Usage: python tools/upscale_sprites.py <path to symbbmp>
"""
import glob
import os
import sys

import numpy as np
from PIL import Image

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "symb")
OUT_SIZE = 384


def scale3x(a):
    h, w = a.shape
    out = np.zeros((h * 3, w * 3), dtype=a.dtype)
    p = np.pad(a, 1, mode="edge")
    A = p[0:h, 0:w]; B = p[0:h, 1:w + 1]; C = p[0:h, 2:w + 2]
    D = p[1:h + 1, 0:w]; E = p[1:h + 1, 1:w + 1]; F = p[1:h + 1, 2:w + 2]
    G = p[2:h + 2, 0:w]; H = p[2:h + 2, 1:w + 1]; I = p[2:h + 2, 2:w + 2]
    keep = (B == H) | (D == F)
    out[0::3, 0::3] = np.where((D == B) & ~keep, D, E)
    out[0::3, 1::3] = np.where(((D == B) & ~keep & (E != C)) | ((B == F) & ~keep & (E != A)), B, E)
    out[0::3, 2::3] = np.where((B == F) & ~keep, F, E)
    out[1::3, 0::3] = np.where(((D == B) & ~keep & (E != G)) | ((D == H) & ~keep & (E != A)), D, E)
    out[1::3, 1::3] = E
    out[1::3, 2::3] = np.where(((B == F) & ~keep & (E != I)) | ((H == F) & ~keep & (E != C)), F, E)
    out[2::3, 0::3] = np.where((D == H) & ~keep, D, E)
    out[2::3, 1::3] = np.where(((D == H) & ~keep & (E != I)) | ((H == F) & ~keep & (E != G)), H, E)
    out[2::3, 2::3] = np.where((H == F) & ~keep, F, E)
    return out


def upscale(src):
    ink = (np.array(Image.open(src).convert("L")) < 128).astype(np.uint8)
    mask = Image.fromarray(scale3x(ink) * 255, "L").resize((OUT_SIZE, OUT_SIZE), Image.LANCZOS)
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
        upscale(src).save(os.path.join(OUT_DIR, stem + ".png"))
        done += 1
    print("upscaled", done, "of", len(stems))
    return 0


if __name__ == "__main__":
    sys.exit(main())
