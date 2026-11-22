#!/usr/bin/env python3
"""Write the NAVTEX station table of the Decodes tab into ats-mini/PageControl.h.

Reads tools/navtex-stations.txt (see its head for the format) and replaces
the line that begins with "const NVD='" in the page's script.

Usage: navtex_table.py [check]   (check: only say whether the page is up to date)
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "tools" / "navtex-stations.txt"
PAGE = ROOT / "ats-mini" / "PageControl.h"
CHANNELS = ["518", "490", "4209.5"]          # The order of navCh[] in DigiPage.cpp


def tenths(text):
    deg, minutes, hemi = text.split()
    value = round((int(deg) + int(minutes) / 60) * 10)
    return -value if hemi in "SW" else value


def table():
    rows = []
    seen = set()
    for number, line in enumerate(SRC.read_text(encoding="utf-8").splitlines(), 1):
        if not line.strip() or line.startswith("#"):
            continue
        part = [p.strip() for p in line.split("|")]
        khz, area, letter = part[0].split()
        name, country = part[1], part[2]
        extra = part[5] if len(part) > 5 else ""
        if khz not in CHANNELS or not re.fullmatch(r"[A-X]", letter):
            sys.exit(f"{SRC.name}:{number}: channel or letter")
        if re.search(r"[,;'\\<>&]", name + country + extra.replace(",", "")) or not (name + country).isascii():
            sys.exit(f"{SRC.name}:{number}: plain ASCII without , ; ' \\ < > & please")
        if extra and not re.fullmatch(r"t=\d{4}(,\d{4})*|f=\d{4}|n=[\w .-]+", extra):
            sys.exit(f"{SRC.name}:{number}: extra")
        key = (khz, area, letter, name)
        if key in seen:
            sys.exit(f"{SRC.name}:{number}: twice")
        seen.add(key)
        row = [str(CHANNELS.index(khz)), area, letter, name, country, str(tenths(part[3])), str(tenths(part[4]))]
        if extra:
            row.append(extra[0] + extra[2:].replace(",", "/"))
        rows.append(",".join(row))
    return ";".join(rows), len(rows)


def main():
    data, count = table()
    page = PAGE.read_text(encoding="utf-8")
    line = f"const NVD='{data}';"
    found = re.search(r"^const NVD='[^'\n]*';$", page, re.M)
    if not found:
        sys.exit("PageControl.h: no line beginning with const NVD='")
    if len(sys.argv) > 1 and sys.argv[1] == "check":
        sys.exit(0 if found.group(0) == line else f"PageControl.h is not up to date with {SRC.name}")
    PAGE.write_text(page[:found.start()] + line + page[found.end():], encoding="utf-8")
    print(f"{count} stations, {len(data)} characters")


if __name__ == "__main__":
    main()
