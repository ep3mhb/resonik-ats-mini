#!/usr/bin/env python3
"""Build the resonik.mikroutik.com site into an upload-ready zip.

Usage: build_website.py <release-tag> <esp-web-tools dist/web dir> <release zip dir> <out dir>

<release zip dir> holds the release's resonik-ats-mini-<tag>-ospi.zip and -qspi.zip.
The firmware goes to firmware/<tag>/<variant>/, so each version has its own URLs and
can be cached for ever; only the two manifests change between releases.
"""
import json
import shutil
import sys
import zipfile
from datetime import date
from pathlib import Path

PARTS = [  # (file in the release zip, flash offset)
    ("ats-mini.ino.bootloader.bin", 0x0),
    ("ats-mini.ino.partitions.bin", 0x8000),
    ("boot_app0.bin", 0xE000),
    ("ats-mini.ino.bin", 0x10000),
]
VARIANTS = {"ospi": "8 MB PSRAM (OSPI)", "qspi": "2 MB PSRAM (QSPI)"}


def main():
    tag, ewt, rel, out = sys.argv[1], Path(sys.argv[2]), Path(sys.argv[3]), Path(sys.argv[4])
    root = Path(__file__).resolve().parent.parent / "website"
    site = out / "site"
    if site.exists():
        shutil.rmtree(site)
    shutil.copytree(root, site, ignore=shutil.ignore_patterns(".*", "*.md"))
    shutil.copytree(ewt, site / "esp-web-tools")

    for variant, label in VARIANTS.items():
        dest = site / "firmware" / tag / variant
        dest.mkdir(parents=True)
        with zipfile.ZipFile(rel / f"resonik-ats-mini-{tag}-{variant}.zip") as z:
            for name, _ in PARTS:
                member = next(m for m in z.namelist() if m.endswith("/" + name))
                (dest / name).write_bytes(z.read(member))
        manifest = {
            "name": f"RESONIK ATS Mini firmware, {label}",
            "version": tag,
            "new_install_prompt_erase": True,  # False would erase everything without asking
            "builds": [{
                "chipFamily": "ESP32-S3",
                "parts": [{"path": f"{tag}/{variant}/{name}", "offset": off} for name, off in PARTS],
            }],
        }
        (site / "firmware" / f"manifest-{variant}.json").write_text(json.dumps(manifest, indent=2) + "\n")

    today = date.today().isoformat()
    (site / "robots.txt").write_text("User-agent: *\nAllow: /\n\nSitemap: https://resonik.mikroutik.com/sitemap.xml\n")
    (site / "sitemap.xml").write_text(
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">\n'
        f"  <url><loc>https://resonik.mikroutik.com/</loc><lastmod>{today}</lastmod></url>\n"
        f"  <url><loc>https://resonik.mikroutik.com/manual/</loc><lastmod>{today}</lastmod></url>\n"
        f"  <url><loc>https://resonik.mikroutik.com/manual/resonik-manual-fa.pdf</loc><lastmod>{today}</lastmod></url>\n"
        f"  <url><loc>https://resonik.mikroutik.com/resonik-roadmap.pdf</loc><lastmod>{today}</lastmod></url>\n"
        f"  <url><loc>https://resonik.mikroutik.com/resonik-roadmap-fa.pdf</loc><lastmod>{today}</lastmod></url>\n"
        "</urlset>\n")

    zpath = out / f"resonik-site-{tag}.zip"
    with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as z:
        for f in sorted(site.rglob("*")):
            if f.is_file():
                z.write(f, f.relative_to(site))
    print(zpath)


if __name__ == "__main__":
    main()
