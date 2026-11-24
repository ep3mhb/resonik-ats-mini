#!/usr/bin/env python3
"""Build the roadmap PDFs (A4) from docs/roadmap/roadmap.html: pages 1-2 are
the English roadmap, 3-4 the Persian one and 5-6 the Arabic one, printed as
three separate files. English is the source; Persian and Arabic follow it.

Usage: build_roadmap.py [out_dir]   (default: website/)
  -> resonik-roadmap.pdf (English), resonik-roadmap-fa.pdf (Persian)
     and resonik-roadmap-ar.pdf (Arabic)

Update docs/roadmap/roadmap.html when a release ships or the plans change
(move a release from Planned to In testing to Released, with its date), then
run this script and rebuild the website. The date in the header is today's.
"""
import sys
import tempfile
from datetime import date
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from build_manual_ar import AR_MONTHS  # noqa: E402
from build_manual_fa import EN_MONTHS, FA_DIGITS, FA_MONTHS, ROOT, jalali  # noqa: E402

SRC = ROOT / "docs" / "roadmap" / "roadmap.html"


def main():
    out_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "website"
    d = date.today()
    jy, jm, jd = jalali(d)
    html = SRC.read_text()
    html = html.replace("{{DATE_EN}}", f"{d.day} {EN_MONTHS[d.month - 1]} {d.year}")
    html = html.replace("{{DATE_FA}}", f"{jd} {FA_MONTHS[jm - 1]} {jy}".translate(FA_DIGITS))
    html = html.replace("{{DATE_AR}}", f"{d.day} {AR_MONTHS[d.month - 1]} {d.year}")

    from playwright.sync_api import sync_playwright
    with tempfile.NamedTemporaryFile("w", suffix=".html", dir=SRC.parent, delete=False) as tmp:
        tmp.write(html)
    try:
        with sync_playwright() as p:
            browser = p.chromium.launch()
            page = browser.new_page()
            page.goto(Path(tmp.name).as_uri())
            page.wait_for_load_state("networkidle")
            page.evaluate("document.fonts.ready")
            out_dir.mkdir(parents=True, exist_ok=True)
            for name, pages in (("resonik-roadmap.pdf", "1-2"), ("resonik-roadmap-fa.pdf", "3-4"),
                                ("resonik-roadmap-ar.pdf", "5-6")):
                page.pdf(path=str(out_dir / name), format="A4", print_background=True,
                         prefer_css_page_size=True, page_ranges=pages)
                print(out_dir / name)
            browser.close()
    finally:
        Path(tmp.name).unlink()


if __name__ == "__main__":
    main()
