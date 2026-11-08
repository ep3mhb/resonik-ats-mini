#!/usr/bin/env python3
"""Build the Persian user manual PDF (A4) from docs/manual-fa/manual-fa.html.

Usage: build_manual_fa.py [out.pdf]   (default: website/manual/resonik-manual-fa.pdf)

The version comes from ats-mini/Common.h (VER_APP, VER_RESONIK) and the date
from that version's section in CHANGELOG.md ("## 2.45-r2 (2026-11-08)"), so
after a release only the text needs reviewing: update manual-fa.html for the
new features (and the screenshots in docs/manual-fa/img if the screens
changed), then run this script and rebuild the website.

Needs Python Playwright with Chromium.
"""
import re
import sys
import tempfile
from datetime import date
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "docs" / "manual-fa" / "manual-fa.html"
FA_DIGITS = str.maketrans("0123456789", "۰۱۲۳۴۵۶۷۸۹")
FA_MONTHS = ["فروردین", "اردیبهشت", "خرداد", "تیر", "مرداد", "شهریور",
             "مهر", "آبان", "آذر", "دی", "بهمن", "اسفند"]
EN_MONTHS = ["January", "February", "March", "April", "May", "June", "July",
             "August", "September", "October", "November", "December"]


def version():
    common = (ROOT / "ats-mini" / "Common.h").read_text()
    app = int(re.search(r"#define VER_APP\s+(\d+)", common).group(1))
    res = int(re.search(r"#define VER_RESONIK\s+(\d+)", common).group(1))
    return f"{app // 100}.{app % 100:02d}-r{res}"


def release_date(ver):
    log = (ROOT / "CHANGELOG.md").read_text()
    m = re.search(rf"^## {re.escape(ver)} \((\d{{4}})-(\d\d)-(\d\d)\)", log, re.M)
    return date(*map(int, m.groups())) if m else date.today()


def jalali(d):
    """Gregorian date to the Solar Hijri calendar (year, month, day)."""
    gy, gm, gd = d.year, d.month, d.day
    g_d_m = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334]
    gy2 = gy + 1 if gm > 2 else gy
    days = 355666 + 365 * gy + (gy2 + 3) // 4 - (gy2 + 99) // 100 + (gy2 + 399) // 400 + gd + g_d_m[gm - 1]
    jy = -1595 + 33 * (days // 12053)
    days %= 12053
    jy += 4 * (days // 1461)
    days %= 1461
    if days > 365:
        jy += (days - 1) // 365
        days = (days - 1) % 365
    if days < 186:
        jm, jd = 1 + days // 31, 1 + days % 31
    else:
        jm, jd = 7 + (days - 186) // 30, 1 + (days - 186) % 30
    return jy, jm, jd


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "website" / "manual" / "resonik-manual-fa.pdf"
    ver = version()
    d = release_date(ver)
    jy, jm, jd = jalali(d)
    html = SRC.read_text()
    html = html.replace("{{VERSION}}", "v" + ver)
    html = html.replace("{{DATE_FA}}", f"{jd} {FA_MONTHS[jm - 1]} {jy}".translate(FA_DIGITS))
    html = html.replace("{{DATE_EN}}", f"{d.day} {EN_MONTHS[d.month - 1]} {d.year}")

    from playwright.sync_api import sync_playwright
    footer = ('<div style="width:100%;font-family:Vazirmatn,sans-serif;font-size:8pt;color:#5b6670;'
              'padding:0 17mm;display:flex;justify-content:space-between;direction:rtl">'
              f'<span>راهنمای کاربری RESONIK · v{ver}</span><span class="pageNumber"></span></div>')
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
