#!/usr/bin/env python3
"""Build the roadmap PDF (A4, English and Persian pages) from docs/roadmap/roadmap.html.

Usage: build_roadmap.py [out.pdf]   (default: website/resonik-roadmap.pdf)

Update docs/roadmap/roadmap.html when a release ships or the plans change
(move a release from Planned to In testing to Released, with its date), then
run this script and rebuild the website. The date in the header is today's.
"""
import sys
import tempfile
from datetime import date
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from build_manual_fa import EN_MONTHS, FA_DIGITS, FA_MONTHS, ROOT, jalali  # noqa: E402

SRC = ROOT / "docs" / "roadmap" / "roadmap.html"


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "website" / "resonik-roadmap.pdf"
    d = date.today()
    jy, jm, jd = jalali(d)
    html = SRC.read_text()
    html = html.replace("{{DATE_EN}}", f"{d.day} {EN_MONTHS[d.month - 1]} {d.year}")
    html = html.replace("{{DATE_FA}}", f"{jd} {FA_MONTHS[jm - 1]} {jy}".translate(FA_DIGITS))

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
            out.parent.mkdir(parents=True, exist_ok=True)
            page.pdf(path=str(out), format="A4", print_background=True, prefer_css_page_size=True)
            browser.close()
    finally:
        Path(tmp.name).unlink()
    print(out)


if __name__ == "__main__":
    main()
