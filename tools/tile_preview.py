#!/usr/bin/env python3
"""Draw tile menu pages and value screens on a PC with the firmware's own code.

    tools/tile_preview.py check
    tools/tile_preview.py page out.png "Capacitor"
    tools/tile_preview.py value out.png "USB Port" 2 Off "Ad hoc" "CAT TS-480"
    tools/tile_preview.py groups out.png 3        (the first page; 3: the fourth group chosen)

The first run fetches LovyanGFX (the version in ats-mini/sketch.yaml) and
builds it for the PC; that and the preview program are kept in the cache
directory (TILE_PREVIEW_CACHE, default ~/.cache/resonik-tile-preview).
Needs git, gcc, g++ and, for PNG output, Pillow. Output is 320x170, the
radio's screen; add --scale N for a larger picture.
"""
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "ats-mini"
CACHE = Path(os.environ.get("TILE_PREVIEW_CACHE", Path.home() / ".cache" / "resonik-tile-preview"))
FLAGS = ["-O1", "-w", "-DLGFX_LINUX_FB", "-DLGFX_USE_V1"]


def run(*cmd, **kw):
    subprocess.run([str(c) for c in cmd], check=True, **kw)


def lgfx():
    """LovyanGFX for the PC: sprites and fonts only, no display."""
    ver = re.search(r"LovyanGFX \(([\d.]+)\)", (SRC / "sketch.yaml").read_text()).group(1)
    src = CACHE / f"LovyanGFX-{ver}"
    lib = CACHE / f"liblgfx-{ver}.a"
    if not src.exists():
        run("git", "clone", "-q", "--depth", "1", "--branch", ver, "https://github.com/lovyan03/LovyanGFX", src)
    if not lib.exists():
        obj = CACHE / f"obj-{ver}"
        obj.mkdir(parents=True, exist_ok=True)
        inc = f"-I{src / 'src'}"
        files = sorted((src / "src" / "lgfx").rglob("*.c")) + sorted((src / "src" / "lgfx" / "v1").glob("*.cpp")) \
            + sorted((src / "src" / "lgfx" / "v1" / "misc").glob("*.cpp"))
        objs = []
        for i, f in enumerate(files):
            o = obj / f"{i:03d}-{f.stem}.o"
            cc = ["gcc"] if f.suffix == ".c" else ["g++", "-std=c++17"]
            run(*cc, *FLAGS, inc, "-c", f, "-o", o)
            objs.append(o)
        run("ar", "rcs", lib, *objs)
    return src / "src", lib


def gen():
    """gen.h: the parts of Menu.cpp and Menu.h the preview draws with."""
    m = (SRC / "Menu.cpp").read_text()
    h = (SRC / "Menu.h").read_text()

    def cut(start, end):
        a = m.index(start)
        return m[a:m.index(end, a)]

    out = ["#define ITEM_COUNT(x) (sizeof(x)/sizeof((x)[0]))"]
    out += re.findall(r"^#define CMD_\w+\s+0x[0-9A-Fa-f]+", h, flags=re.M)
    out.append(cut("#define MENU_MODE         0", "// Opening the menu comes back to the page used last"))
    out.append(cut("typedef struct\n{\n  bool list;          // A list of values, else a number", "static const uint8_t tileAmModes[]"))
    out.append(cut("static void tileDrawValue(uint16_t cmd, const TileValue &v)", "bool drawTileMenu()"))
    (CACHE / "gen.h").write_text("\n".join(out) + "\n")


def main():
    args = sys.argv[1:]
    scale = 1
    if "--scale" in args:
        i = args.index("--scale")
        scale = int(args[i + 1])
        del args[i:i + 2]
    if not args:
        sys.exit(__doc__)
    CACHE.mkdir(parents=True, exist_ok=True)
    inc, lib = lgfx()
    gen()
    exe = CACHE / "preview"
    run("g++", "-std=c++17", *FLAGS, f"-I{inc}", f"-I{SRC}", f"-I{CACHE}", ROOT / "tools" / "tile_preview.cpp", lib, "-o", exe)
    png = None
    if args[0] in ("page", "value", "groups") and len(args) > 1 and args[1].lower().endswith(".png"):
        png = Path(args[1])
        args[1] = str(CACHE / "out.ppm")
    code = subprocess.run([str(exe), *args]).returncode
    if png and code == 0:
        from PIL import Image
        im = Image.open(CACHE / "out.ppm")
        if scale > 1:
            im = im.resize((320 * scale, 170 * scale), Image.NEAREST)
        im.save(png)
    sys.exit(code)


if __name__ == "__main__":
    main()
