#!/usr/bin/env python3
"""Build the Arabic user manual PDF (A4) from docs/manual-ar/manual-ar.html.

Usage: build_manual_ar.py [out.pdf]   (default: website/manual/resonik-manual-ar.pdf)

The English manual is the source: the Arabic manual is translated from it,
chapter by chapter, and uses the same screenshots (docs/manual-fa/img). The
version and date come from Common.h and CHANGELOG.md as for the other manuals.
The text is set in Vazirmatn, Latin in Vazirmatn and Inter, with Western
digits, as in the RESONIK CW Trainer's Arabic manual; the fonts are in
docs/manual-ar/fonts (SIL Open Font License).

Needs Python Playwright with Chromium.
"""
import base64
import re
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from build_manual_fa import ROOT, release_date, version  # noqa: E402

SRC = ROOT / "docs" / "manual-ar" / "manual-ar.html"
FONTS = SRC.parent / "fonts"
AR_MONTHS = ["يناير", "فبراير", "مارس", "أبريل", "مايو", "يونيو",
             "يوليو", "أغسطس", "سبتمبر", "أكتوبر", "نوفمبر", "ديسمبر"]


def font_data(name):
    return "data:font/woff2;base64," + base64.b64encode((FONTS / name).read_bytes()).decode()


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "website" / "manual" / "resonik-manual-ar.pdf"
    src = Path(sys.argv[2]) if len(sys.argv) > 2 else SRC
    ver = version()
    d = release_date(ver)
    html = src.read_text()
    # A menu path runs right to left in Arabic, each name in its own <bdi>:
    # <bdi>Menu</bdi> ← <bdi>System</bdi>. One left-to-right run "A → B" inside
    # an Arabic line reads in the wrong order.
    bad = [n for n, line in enumerate(html.splitlines(), 1) if "→" in line]
    if bad:
        sys.exit(f"{src.name}: '→' on line(s) {', '.join(map(str, bad))}: "
                 "write menu paths right to left, <bdi>Menu</bdi> ← <bdi>System</bdi>")
    html = html.replace("{{VERSION}}", "v" + ver)
    html = html.replace("{{DATE_AR}}", f"{d.day} {AR_MONTHS[d.month - 1]} {d.year}")
    # The fonts go into the page as data, so the footer below (drawn apart from the page)
    # finds the same fonts already loaded.
    html = re.sub(r'url\("fonts/([\w.-]+\.woff2)"\)', lambda m: f"url({font_data(m.group(1))})", html)

    from playwright.sync_api import sync_playwright
    # The footer is drawn apart from the page and cannot load its fonts: they go in as data.
    def face(name, ranges):
        return ("@font-face{font-family:'Vazirmatn';font-weight:400;"
                f"src:url({font_data(f'vazirmatn-{name}-400-normal.woff2')}) format('woff2');unicode-range:{ranges};}}")
    footer = (f'<style>{face("arabic", "U+0600-06FF,U+200C-200F,U+FB50-FDFF,U+FE70-FEFC")}'
              f'{face("latin", "U+0000-00FF,U+2000-206F")}</style>'
              '<div style="width:100%;font-family:Vazirmatn,sans-serif;font-size:8pt;color:#5b6670;'
              'padding:0 17mm;display:flex;justify-content:space-between;direction:rtl">'
              f'<span>دليل مستخدم RESONIK · v{ver}</span><span class="pageNumber"></span></div>')
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
