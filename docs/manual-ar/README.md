# Arabic user manual (PDF)

`manual-ar.html` is the Arabic translation of the English manual
(`docs/manual-en/manual-en.html`, the source), chapter for chapter, and uses the
screenshots in `docs/manual-fa/img`. `tools/build_manual_ar.py` prints it to A4 as
`website/manual/resonik-manual-ar.pdf`, linked from the manual page and the home page.

The version and date on the cover and in the footer come from `ats-mini/Common.h`
and the version's CHANGELOG heading; the date is written with Arabic-Indic digits
and the month names used in the Gulf (٢٤ نوفمبر ٢٠٢٦).

At every release, after updating the English manual:

1. Translate the new sections from `manual-en.html` only, with the same `tag`
   labels and a line in the contents list.
2. Keep menu names in English inside `<bdi>`, as on the radio. A menu path runs
   right to left, each name in its own `<bdi>`:
   `<bdi>Menu</bdi> ← <bdi>System</bdi> ← <bdi>Update FW</bdi>` (the build stops
   on a `→`). A value with its unit goes in one `<bdi>` with Latin digits
   (`<bdi>14.074&nbsp;MHz</bdi>`); counts, section numbers and dates use
   Arabic-Indic digits (القسم ٢٥).
3. Words used: المقبض (the knob), النقرة، الضغطة القصيرة، الضغطة الطويلة (click,
   short press, long press), مفكّك الترميز (decoder), الشلال (waterfall), إشارة
   النداء (callsign), التحكم عبر المتصفح (Web Control). Appendix A is «أ», its
   sections «أ-١» to «أ-٩».
4. Do not stack a shadda with a vowel or tanwin on a tall letter (كلٌّ، خطًّا):
   when such a line starts a page, Chromium paints the top of the marks at the foot
   of the page before. Write كلّ, or reword.
5. `python3 tools/build_manual_ar.py`, look at the foot of every page and check
   that no figure splits across pages, commit the PDF, rebuild the website zip with
   `tools/build_website.py`.
