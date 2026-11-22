#!/usr/bin/env python3
"""Draw the picture of the Ham & SWL menu tile and write it into ats-mini/TileDraw.h.

The picture is the drawing inside the international amateur radio symbol:
aerial, coil and earth. It is taken as it is, line for line, from
tools/ham-symbol.svg ("International amateur radio symbol.svg" from
Wikimedia Commons, by Denelson83, released into the public domain), without
the diamond around it, and reduced to the height of a tile. Nothing is
redrawn: the script reads the paths of the SVG, draws them at eight times the
size and reduces them, which keeps the soft edges that make thin lines look
right on a small screen. One thing differs from the symbol: the two straight
wires, from the aerial to the coil and from the coil to earth, are shortened
(LEAD), so that the rest can be larger in the same height. The firmware gets
a table of how much ink each pixel has (0 to 255) and blends it into the tile
in yellow, the symbol's own colour, with a dark edge around it.

    tools/ham_icon.py              write the table
    tools/ham_icon.py show x.png   also save a picture, six times enlarged
Needs Pillow.
"""
import math
import re
import sys
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent
SVG = ROOT / "tools" / "ham-symbol.svg"
HEADER = ROOT / "ats-mini" / "TileDraw.h"
BEGIN = "// tools/ham_icon.py writes the next table"
END = "// End of the table of tools/ham_icon.py"
HEIGHT = 57                 # Pixels: the tile is 69 high, and the picture keeps clear of its edges
WEIGHT = 1.0                # 1: the lines as heavy as in the symbol
LEAD = 16.0                 # Length kept of each straight wire, in the SVG's units (55 and 58 in the symbol)
SCALE = 8


def bezier(p0, p1, p2, p3, steps=24):
    out = []
    for i in range(1, steps + 1):
        t = i / steps
        u = 1 - t
        out.append(tuple(u ** 3 * p0[k] + 3 * u * u * t * p1[k] + 3 * u * t * t * p2[k] + t ** 3 * p3[k] for k in (0, 1)))
    return out


def polyline(d):
    """The points of one path of the SVG (it uses M, L, C and z only)."""
    tok = re.findall(r"[MLCz]|-?[\d.]+", d)
    pts, i, cmd = [], 0, None
    while i < len(tok):
        if tok[i] in "MLCz":
            cmd = tok[i]
            i += 1
            if cmd == "z":
                pts.append(pts[0])
            continue
        if cmd in "ML":
            pts.append((float(tok[i]), float(tok[i + 1])))
            i += 2
        else:
            c = [(float(tok[i + k]), float(tok[i + k + 1])) for k in (0, 2, 4)]
            pts += bezier(pts[-1], *c)
            i += 6
    return pts


def picture():
    svg = SVG.read_text()
    group = svg[svg.index('id="g3177"'):]               # The drawing inside the diamond
    mirror = float(re.search(r"matrix\(-1,0,0,1,([\d.]+),0\)", group).group(1))
    width = float(re.search(r"stroke-width:([\d.]+)", group).group(1))
    lines = [[(mirror - x, y) for x, y in polyline(d)] for d in re.findall(r'\sd="([^"]+)"', group)]
    # The wire's four stations, top to bottom: the aerial's point, the coil's
    # two ends, the first earth bar. The stretches between them are the leads.
    wire = max(lines, key=len)
    turns = [y for x, y in wire if abs(x - wire[0][0]) > 1]
    coil_top, coil_bottom = min(turns), max(turns)
    coil_top = max(y for x, y in wire if y < coil_top + 12 and abs(x - wire[0][0]) < 0.1)
    coil_bottom = min(y for x, y in wire if y > coil_bottom - 12 and abs(x - wire[0][0]) < 0.1)
    aerial = min((line for line in lines if line is not wire), key=lambda line: min(y for _, y in line))   # The triangle: the highest shape
    apex = max(y for _, y in aerial)
    earth = max(y for _, y in wire)
    up, down = (coil_top - apex) - LEAD, (earth - coil_bottom) - LEAD

    def shorter(y):
        if y <= apex:
            return y + up
        if y < coil_top:
            return coil_top - (coil_top - y) * LEAD / (coil_top - apex)
        if y <= coil_bottom:
            return y
        if y < earth:
            return coil_bottom + (y - coil_bottom) * LEAD / (earth - coil_bottom)
        return y - down

    lines = [[(x, shorter(y)) for x, y in line] for line in lines]
    xs = [x for line in lines for x, _ in line]
    ys = [y for line in lines for _, y in line]
    x0, x1, y0, y1 = min(xs) - width / 2, max(xs) + width / 2, min(ys) - width / 2, max(ys) + width / 2
    k = HEIGHT / (y1 - y0)
    w = math.ceil((x1 - x0) * k)
    w += (w + 1) % 2                                   # Odd, so that the wire stands on a pixel column
    off = (w - (x1 - x0) * k) / 2
    big = Image.new("L", (w * SCALE, HEIGHT * SCALE), 0)
    draw = ImageDraw.Draw(big)
    for line in lines:
        draw.line([(((x - x0) * k + off) * SCALE, (y - y0) * k * SCALE) for x, y in line],
                  fill=255, width=max(1, round(width * WEIGHT * k * SCALE)), joint="curve")
    return big.resize((w, HEIGHT), Image.BOX)


def main():
    im = picture()
    w, h = im.size
    px = list(im.tobytes())
    rows = ["  " + ", ".join("%3d" % v for v in px[r * w:(r + 1) * w]) + "," for r in range(h)]
    table = "#define TILE_HAM_W %d\n#define TILE_HAM_H %d\nstatic const uint8_t tileHamInk[TILE_HAM_W * TILE_HAM_H] =\n{\n%s\n};\n" % (w, h, "\n".join(rows))
    text = HEADER.read_text()
    a = text.index(BEGIN) + len(BEGIN) + 1
    b = text.index(END)
    HEADER.write_text(text[:a] + table + text[b:])
    if len(sys.argv) == 3 and sys.argv[1] == "show":
        out = Image.new("RGB", im.size, (0, 214, 165))
        out.paste(Image.new("RGB", im.size, (255, 255, 0)), (0, 0), im)
        out.resize((w * 6, h * 6), Image.NEAREST).save(sys.argv[2])
    print("TileDraw.h: Ham & SWL picture, %d x %d" % (w, h))


if __name__ == "__main__":
    main()
