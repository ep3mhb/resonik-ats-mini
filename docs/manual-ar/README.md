# Arabic user manual (PDF)

`manual-ar.html` is the Arabic translation of the English manual
(`docs/manual-en/manual-en.html`, the source), chapter for chapter, and uses the
screenshots in `docs/manual-fa/img`. `tools/build_manual_ar.py` prints it to A4 as
`website/manual/resonik-manual-ar.pdf`, linked from the manual page and the home page.

The version and date on the cover and in the footer come from `ats-mini/Common.h`
and the version's CHANGELOG heading; the date uses the month names of the Gulf
(24 نوفمبر 2026).

It is set like the RESONIK CW Trainer's Arabic manual: the Arabic in Vazirmatn
(400, 500 and 700), Latin in Vazirmatn and then Inter, and Western digits (0–9)
throughout, as Gulf technical writing does. The fonts are in `fonts/` (SIL Open
Font License, from the npm packages `@fontsource/vazirmatn` and `@fontsource/inter`);
the build puts them into the page and the footer as data.

At every release, after updating the English manual:

1. Translate the new sections from `manual-en.html` only, with the same `tag`
   labels and a line in the contents list.
2. Keep menu names in English inside `<bdi>`, as on the radio. A menu path runs
   right to left, each name in its own `<bdi>`:
   `<bdi>Menu</bdi> ← <bdi>System</bdi> ← <bdi>Update FW</bdi>` (the build stops
   on a `→`). A value with its unit goes in one `<bdi>` (`<bdi>14.074&nbsp;MHz</bdi>`);
   counts and section numbers stand in the Arabic text as they are (القسم 25).
3. Words used: المقبض (the knob), النقرة، الضغطة القصيرة، الضغطة الطويلة (click,
   short press, long press), مفكّك الترميز (decoder), الشلال (waterfall), إشارة
   النداء (callsign), التحكم عبر المتصفح (Web Control). Appendix A is «أ», its
   sections «أ-1» to «أ-9».
4. Do not stack a shadda with a vowel or tanwin on a tall letter (كلٌّ، خطًّا):
   when such a line starts a page, Chromium paints the top of the marks at the foot
   of the page before. Write كلّ, or reword.
5. `python3 tools/build_manual_ar.py`, look at the foot of every page and check
   that no figure splits across pages, commit the PDF, rebuild the website zip with
   `tools/build_website.py`.
