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
    BODY { margin: 0; padding: 0; }
    H1 { text-align: center; }
    TABLE { width: 100%; max-width: 768px; border: 0px; margin-left: auto; margin-right: auto; }
    TH, TD { padding: 0.5em; }
    .HEADING { background-color: #80A0FF; column-span: all; text-align: center; }
    TD.LABEL { text-align: right; }
    INPUT[type=text], INPUT[type=password], INPUT[type=number], SELECT { width: 95%; padding: 0.5em; }
    INPUT[type=submit] { width: 50%; padding: 0.5em 0; }
    .CENTER { text-align: center; }
    .HEAP { color: #888; font-size: 0.75em; text-align: center; margin: 1.5em 0.5em; }
    .BRAND { background: #0F2F3C; color: #FFF; display: flex; align-items: center; justify-content: center; gap: 12px; padding: 12px 16px; }
    .BRAND B { font-size: 1.35em; letter-spacing: 0.18em; }
    .BRAND SMALL { display: block; color: #9FC1CE; font-size: 0.8em; letter-spacing: 0.02em; }
    .BRAND A { color: inherit; text-decoration: none; display: flex; align-items: center; gap: 12px; }
  </STYLE>
</HEAD>
<BODY STYLE="font-family: sans-serif;">
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
  out += R"HTML(<P ALIGN="CENTER">)HTML";
  for(size_t i = 0; i < sizeof(pages) / sizeof(pages[0]); i++)
  {
    if(i) out += "&nbsp;|&nbsp;";
    if(!strcmp(activePage, pages[i].path)) out += pages[i].name;
    else pageAppend(out, R"HTML(<A HREF="{{path}}">{{label}}</A>)HTML", {{"path", pages[i].path}, {"label", pages[i].name}});
  }
  out += "</P>";
}

#endif // PAGE_COMMON_H
