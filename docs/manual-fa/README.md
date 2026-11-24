# Persian user manual (PDF)

`manual-fa.html` is the Persian translation of the English manual
(`docs/manual-en/manual-en.html`, the source), chapter for chapter, printed to A4 by
`tools/build_manual_fa.py` into `website/manual/resonik-manual-fa.pdf`, which the
website links from the manual page and the Persian section. The screenshots in
`img/` are shared by the English, Persian and Arabic manuals.

The version and date on the cover and in the footer are filled in from
`ats-mini/Common.h` and the version's CHANGELOG heading.

At every release, after updating the English manual:

1. Translate the new sections from `manual-en.html` into `manual-fa.html` (with the
   same `tag` label, e.g. `R5`, and a line in the contents list). Keep menu names in
   English inside `<bdi>`, as on the radio; the knob is «ولوم»; avoid «کاشی».
   A menu path runs right to left, each name in its own `<bdi>`:
   `<bdi>Menu</bdi> ← <bdi>System</bdi> ← <bdi>Update FW</bdi>` (the build stops
   on a `→`). A line that is wholly English, like the sign-off
   `73 de resonik.mikroutik.com`, goes in one `<bdi>`. A callsign is «نشانه».
2. `python3 tools/build_manual_fa.py`, check the PDF, commit it, rebuild the
   website zip with `tools/build_website.py`.
