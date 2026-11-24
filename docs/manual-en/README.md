# English user manual (PDF)

`manual-en.html` is the source of the manuals: the Persian
(`docs/manual-fa/manual-fa.html`) and Arabic (`docs/manual-ar/manual-ar.html`)
manuals are translated from it, chapter for chapter. All three use the screenshots in
`docs/manual-fa/img`. `tools/build_manual_en.py` prints it to A4 as
`website/manual/resonik-manual-en.pdf`, linked from the manual page and the home page.

The version and date on the cover and in the footer come from `ats-mini/Common.h`
and the version's CHANGELOG heading.

At every release:

1. Add the new features to `manual-en.html` first (new sections get the `tag` label
   with the release, e.g. `R5`, and a line in the contents list). Menu names exactly
   as on the radio.
2. If screens changed, replace the screenshots in `docs/manual-fa/img` (640×340 PNG,
   taken in the host simulator at 2×).
3. `python3 tools/build_manual_en.py`, check that no figure splits across pages,
   commit the PDF.
4. Translate the same sections into the Persian and Arabic manuals (see their
   READMEs), build both, and rebuild the website zip with `tools/build_website.py`.
