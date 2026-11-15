# English user manual (PDF)

`manual-en.html` is the English translation of the Persian manual
(`docs/manual-fa/manual-fa.html`), chapter for chapter, and uses its screenshots in
`docs/manual-fa/img`. `tools/build_manual_en.py` prints it to A4 as
`website/manual/resonik-manual-en.pdf`, linked from the manual page and the home page.

The version and date on the cover and in the footer come from `ats-mini/Common.h`
and the version's CHANGELOG heading.

At every release, after updating the Persian manual:

1. Add the same sections to `manual-en.html`, with the same `tag` labels and
   contents lines. Menu names exactly as on the radio.
2. `python3 tools/build_manual_en.py`, check that no figure splits across pages,
   commit the PDF, rebuild the website zip with `tools/build_website.py`.
