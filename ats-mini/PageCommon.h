#ifndef PAGE_COMMON_H
#define PAGE_COMMON_H

#include "PageTemplate.h"

static const char pageStart[] PROGMEM = R"HTML(<!DOCTYPE HTML>
<HTML>
<HEAD>
  <META CHARSET="UTF-8">
  <META NAME="viewport" CONTENT="width=device-width, initial-scale=1.0">
  <TITLE>{{title}}</TITLE>
  <STYLE>
    /* RESONIK R4: dark, as the Control page */
    :root { color-scheme: dark; --bg: #09202A; --card: #0F2F3C; --line: #1D4A5C; --text: #EEF5F7; --muted: #8FB0BD; --accent: #06D7A0; --blue: #108AB1; }
    BODY { margin: 0; padding: 0 0 1em; background: var(--bg); color: var(--text); font: 15px/1.45 system-ui, -apple-system, "Segoe UI", Roboto, sans-serif; }
    A { color: var(--accent); }
    H1, H2, H3 { text-align: center; }
    TABLE { width: 100%; max-width: 768px; border: 0px; margin-left: auto; margin-right: auto; border-collapse: separate; border-spacing: 0; }
    TH, TD { padding: 0.5em; }
    .HEADING { background-color: var(--card); color: var(--text); column-span: all; text-align: center; border-top: 2px solid var(--blue); font-weight: 700; }
    TD.LABEL { text-align: right; color: var(--muted); }
    INPUT[type=text], INPUT[type=password], INPUT[type=number], SELECT, TEXTAREA { width: 95%; padding: 0.5em; background: #06171E; color: var(--text); border: 1px solid var(--line); border-radius: 8px; font: inherit; }
    INPUT[type=checkbox], INPUT[type=radio] { accent-color: var(--accent); }
    INPUT[type=submit], BUTTON { padding: 0.6em 1.2em; background: var(--card); color: var(--text); border: 1px solid var(--line); border-radius: 10px; font: 600 15px system-ui, sans-serif; cursor: pointer; }
    INPUT[type=submit] { width: 50%; }
    INPUT[type=submit]:hover, BUTTON:hover { background: var(--blue); }
    BUTTON:disabled, INPUT:disabled { opacity: 0.45; cursor: default; }
    INPUT[type=file] { color: var(--muted); }
    .CENTER { text-align: center; }
    .HEAP { color: var(--muted); font-size: 0.75em; text-align: center; margin: 1.5em 0.5em; }
    .BRAND { background: var(--card); color: #FFF; display: flex; align-items: center; justify-content: center; gap: 12px; padding: 12px 16px; border-bottom: 1px solid var(--line); }
    .BRAND B { font-size: 1.35em; letter-spacing: 0.18em; }
    .BRAND SMALL { display: block; color: #9FC1CE; font-size: 0.8em; letter-spacing: 0.02em; }
    .BRAND A { color: inherit; text-decoration: none; display: flex; align-items: center; gap: 12px; }
    .NAV { display: flex; flex-wrap: wrap; justify-content: center; gap: 6px; margin: 12px 8px; }
    .NAV A, .NAV SPAN { padding: 6px 12px; border-radius: 9px; background: var(--card); color: var(--muted); text-decoration: none; font-size: 0.9em; font-weight: 600; }
    .NAV SPAN { background: var(--blue); color: #FFF; }
  </STYLE>
</HEAD>
<BODY>
<DIV CLASS="BRAND"><A HREF="/"><SVG CLASS="SYM" VIEWBOX="0 0 53 48" WIDTH="34" HEIGHT="31" ARIA-HIDDEN="true"><RECT X="0" Y="40" WIDTH="5" HEIGHT="8" RX="2" FILL="#DCE6EA"/><RECT X="8" Y="36" WIDTH="5" HEIGHT="12" RX="2" FILL="#DCE6EA"/><RECT X="16" Y="28" WIDTH="5" HEIGHT="20" RX="2" FILL="#DCE6EA"/><RECT X="24" Y="0" WIDTH="5" HEIGHT="48" RX="2" FILL="#4DABF7"/><RECT X="32" Y="28" WIDTH="5" HEIGHT="20" RX="2" FILL="#DCE6EA"/><RECT X="40" Y="36" WIDTH="5" HEIGHT="12" RX="2" FILL="#DCE6EA"/><RECT X="48" Y="40" WIDTH="5" HEIGHT="8" RX="2" FILL="#DCE6EA"/></SVG><SPAN><B>RESONIK</B><SMALL>ATS Mini firmware {{version}}</SMALL></SPAN></A></DIV>
)HTML";

static const char pageEnd[] PROGMEM = R"HTML(
<FOOTER CLASS="HEAP">
  Internal heap: {{heap_used}} KiB used / {{heap_free}} KiB free &middot;
  PSRAM: {{psram_used}} KiB used / {{psram_free}} KiB free (at page render)
</FOOTER>
</BODY>
</HTML>
)HTML";

static void webNavigation(String &out, const char *activePage)
{
  static const struct { const char *name; const char *path; } pages[] =
  {
    {"Status", "/"},
    {"Control", "/control"},
    {"Memory", "/memory"},
    {"Config", "/config"},
    {"Wi-Fi", "/wifi"},
    {"Update", "/update"},
    {"Patches", "/patches"},
    {"Log (CSV)", "/log.csv"},
  };
  out += R"HTML(<NAV CLASS="NAV">)HTML";
  for(size_t i = 0; i < sizeof(pages) / sizeof(pages[0]); i++)
  {
    if(!strcmp(activePage, pages[i].path)) { out += "<SPAN>"; out += pages[i].name; out += "</SPAN>"; }
    else pageAppend(out, R"HTML(<A HREF="{{path}}">{{label}}</A>)HTML", {{"path", pages[i].path}, {"label", pages[i].name}});
  }
  out += "</NAV>";
}

#endif // PAGE_COMMON_H
