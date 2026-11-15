#!/usr/bin/env python3
"""Build the English user manual PDF (A4) from docs/manual-en/manual-en.html.

Usage: build_manual_en.py [out.pdf]   (default: website/manual/resonik-manual-en.pdf)

The English manual follows the Persian one chapter by chapter and uses its
screenshots (docs/manual-fa/img). Update both at every release; the version
and date come from Common.h and CHANGELOG.md as for the Persian manual.
"""
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from build_manual_fa import EN_MONTHS, ROOT, release_date, version  # noqa: E402

SRC = ROOT / "docs" / "manual-en" / "manual-en.html"


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "website" / "manual" / "resonik-manual-en.pdf"
    ver = version()
    d = release_date(ver)
    html = SRC.read_text()
    html = html.replace("{{VERSION}}", "v" + ver)
    html = html.replace("{{DATE_EN}}", f"{d.day} {EN_MONTHS[d.month - 1]} {d.year}")

    from playwright.sync_api import sync_playwright
    footer = ('<div style="width:100%;font-family:Sora,sans-serif;font-size:8pt;color:#5b6670;'
              'padding:0 17mm;display:flex;justify-content:space-between">'
              f'<span>RESONIK user manual · v{ver}</span><span class="pageNumber"></span></div>')
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
            page.pdf(path=str(out), format="A4", print_background=True, prefer_css_page_size=True,
                     display_header_footer=True, header_template="<span></span>", footer_template=footer)
            browser.close()
    finally:
        Path(tmp.name).unlink()
    print(out)


if __name__ == "__main__":
    main()
