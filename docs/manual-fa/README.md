# Persian user manual (PDF)

`manual-fa.html` is the source of the Persian manual, printed to A4 by
`tools/build_manual_fa.py` into `website/manual/resonik-manual-fa.pdf`, which the
website links from the manual page and the Persian section.

The version and date on the cover and in the footer are filled in from
`ats-mini/Common.h` and the version's CHANGELOG heading.

At every release:

1. Add the new features to `manual-fa.html` (new sections get the `tag` label with
   the release, e.g. `R3`, and a line in the contents list). Keep menu names in
   English inside `<bdi>`, as on the radio; the knob is «ولوم»; avoid «کاشی».
2. If screens changed, replace the screenshots in `img/` (640×340 PNG, taken in
   the host simulator at 2×).
3. `python3 tools/build_manual_fa.py`, check the PDF, commit it, rebuild the
   website zip with `tools/build_website.py`.
