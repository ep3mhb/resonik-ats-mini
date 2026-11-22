#!/usr/bin/env python3
"""Draw the picture of the Ham & SWL menu tile and write it into ats-mini/TileDraw.h.

The picture follows the international amateur radio symbol (Wikimedia Commons,
"International amateur radio symbol.svg" by Denelson83, released into the
public domain): an aerial as a triangle, point down, on the wire; a coil; and
earth as bars that get shorter. The coil's turns are the symbol's own curves,
two of them where the symbol has four, and everything is made wider and
heavier than in the symbol so that it reads on a screen 170 pixels high.

It is kept in the firmware as two masks of one bit per pixel, 31 wide and 59
high: the wire (aerial, leads, earth) and the coil. This script draws them at
eight times the size, reduces them to whole pixels, and replaces the two
tables in TileDraw.h between the lines that name this script.

    tools/ham_icon.py            write the tables
    tools/ham_icon.py show x.png also save a picture, six times enlarged
Needs Pillow.
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw
S = 8; W, H = 31, 59; CX, CY = 15, 29
TILE = (0, 214, 165); DEEP = (0, 60, 78); WHITE = (255, 255, 255); ORANGE = (255, 159, 28)
def layer(): im = Image.new("L", (W * S, H * S), 0); return im, ImageDraw.Draw(im)
def P(x, y): return ((CX + x + 0.5) * S, (CY + y + 0.5) * S)
def stroke(d, pts, w, caps=True):
    q = [P(x, y) for x, y in pts]
    d.line(q, fill=255, width=max(1, int(round(w * S))), joint="curve")
    if caps:
        r = w * S / 2
        for x, y in (q[0], q[-1]): d.ellipse([x - r, y - r, x + r, y + r], fill=255)
def bez(p0, p1, p2, p3, n=40):
    out = []
    for i in range(n + 1):
        t = i / n; u = 1 - t
        out.append((u**3 * p0[0] + 3 * u * u * t * p1[0] + 3 * u * t * t * p2[0] + t**3 * p3[0], u**3 * p0[1] + 3 * u * u * t * p1[1] + 3 * u * t * t * p2[1] + t**3 * p3[1]))
    return out
def coil(top, n, a, p, q, side=-1):
    """Points of the coil, downward from the wire at (0, top). side -1: first turn goes left, as the symbol shows it."""
    R = side * a; L = -side * a
    pts = bez((0, top), (R * 0.713, top), (R, top + q * 0.395), (R, top + q))          # quarter turn out to the first extreme
    y = top + q
    for i in range(n):
        yl = y + p / 2; yr = y + p
        pts += bez((R, y), (R, y + 0.49 * p), (L, yl + 0.63 * p), (L, yl))             # across the front, down and back up
        pts += bez((L, yl), (L, yl - 0.585 * p), (R, yr - 0.50 * p), (R, yr))          # across the back, up and down
        y = yr
    pts += bez((R, y), (R, y + q * 0.605), (R * 0.713, y + q), (0, y + q))             # quarter turn back to the wire
    return pts, y + q
def build(n, a, p, q, wc, ww=2.0, bars=4, top=-13, tri=(9, 10)):
    wi, dw = layer(); co, dc = layer()
    pts, bot = coil(top, n, a, p, q)
    stroke(dc, pts, wc, caps=False)
    hw, th = tri
    stroke(dw, [(-hw, -27), (hw, -27), (0, -27 + th), (-hw, -27)], ww - 0.2)            # the aerial: a triangle, point down
    stroke(dw, [(0, -27), (0, top)], ww); stroke(dw, [(0, bot), (0, 16)], ww)
    g = 16; half = 11
    for b in range(bars):
        dw.rectangle([(CX - half) * S, (CY + g) * S, (CX + half + 1) * S - 1, (CY + g + 2) * S - 1], fill=255)
        g += 3; half -= 3
    f = lambda im: im.resize((W, H), Image.BOX).point(lambda v: 255 if v >= 112 else 0)
    return f(wi), f(co), bot
def paint(wi, co, scale=1, one_colour=False):
    im = Image.new("RGB", (W, H), TILE); px = im.load(); a = wi.load(); b = co.load()
    solid = [[a[x, y] or b[x, y] for x in range(W)] for y in range(H)]
    for y in range(H):
        for x in range(W):
            if not solid[y][x] and any(0 <= x + dx < W and 0 <= y + dy < H and solid[y + dy][x + dx] for dx in (-1, 0, 1) for dy in (-1, 0, 1)): px[x, y] = DEEP
            elif b[x, y] and not a[x, y]: px[x, y] = WHITE if one_colour else ORANGE
            elif a[x, y]: px[x, y] = WHITE
    return im.resize((W * scale, H * scale), Image.NEAREST)

TURNS = dict(n=2, a=9.5, p=9.0, q=3.6, wc=2.0)
ROOT = Path(__file__).resolve().parent.parent
HEADER = ROOT / "ats-mini" / "TileDraw.h"
BEGIN = "// tools/ham_icon.py writes the next two tables"
END = "// End of the tables of tools/ham_icon.py"


def table(name, mask):
    px = mask.load()
    rows = []
    for y in range(H):
        v = 0
        for x in range(W):
            if px[x, y]:
                v |= 1 << x
        rows.append("0x%08X" % v)
    lines = ["  " + ", ".join(rows[i:i + 6]) + "," for i in range(0, H, 6)]
    return "static const uint32_t %s[%d] =\n{\n%s\n};\n" % (name, H, "\n".join(lines))


def main():
    wire, coil_mask, _ = build(**TURNS)
    text = HEADER.read_text()
    a = text.index(BEGIN) + len(BEGIN) + 1
    b = text.index(END)
    HEADER.write_text(text[:a] + table("tileHamWire", wire) + table("tileHamCoil", coil_mask) + text[b:])
    if len(sys.argv) == 3 and sys.argv[1] == "show":
        paint(wire, coil_mask, 6).save(sys.argv[2])
    print("TileDraw.h: Ham & SWL picture, %d x %d" % (W, H))


if __name__ == "__main__":
    main()
