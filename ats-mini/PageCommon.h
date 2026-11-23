#ifndef PAGE_COMMON_H
#define PAGE_COMMON_H

#include "PageTemplate.h"
#include "Common.h"

// R5: one frame for every page of the radio's web portal, Web Control
// included (PageControl.h): the RESONIK bar, the page menu and a footer with
// the site, the manual and GitHub (Mohammad, 22 Nov: the pages should look
// the same, and the portal named neither GitHub nor the RESONIK site).
// html: a scrollbar on every page, so the frame does not shift sideways between a
// short page and a long one (23 Nov: "opening each tab shakes the screen")
#define PAGE_FRAME_CSS \
  "html{overflow-y:scroll;-webkit-text-size-adjust:100%;text-size-adjust:100%}" \
  ".rk-brand{box-sizing:border-box;position:relative;display:flex;align-items:center;justify-content:center;gap:12px;" \
  "padding:10px 16px;background:#0F2F3C;border-bottom:1px solid #1D4A5C;color:#fff;" \
  "font:15px/1.4 system-ui,-apple-system,'Segoe UI',Roboto,sans-serif}" \
  ".rk-brand a{display:flex;align-items:center;gap:12px;color:inherit;text-decoration:none}" \
  ".rk-brand b{display:block;font-size:20px;letter-spacing:.18em;line-height:1.15}" \
  ".rk-brand small{display:block;color:#9FC1CE;font-size:12px;letter-spacing:.02em}" \
  ".rk-right{position:absolute;right:16px;top:50%;transform:translateY(-50%)}" \
  ".rk-nav{box-sizing:border-box;display:flex;flex-wrap:wrap;justify-content:center;gap:6px;margin:12px 8px}" \
  ".rk-nav a,.rk-nav span{padding:6px 12px;border-radius:9px;background:#0F2F3C;color:#8FB0BD;" \
  "text-decoration:none;font:600 13.5px/1.4 system-ui,-apple-system,'Segoe UI',Roboto,sans-serif}" \
  ".rk-nav span{background:#108AB1;color:#fff}" \
  ".rk-foot{box-sizing:border-box;max-width:768px;margin:24px auto 0;padding:12px 8px 16px;border-top:1px solid #1D4A5C;" \
  "text-align:center;color:#8FB0BD;font:13px/1.6 system-ui,-apple-system,'Segoe UI',Roboto,sans-serif}" \
  ".rk-foot a{color:#06D7A0;text-decoration:none}" \
  "@media(max-width:420px){.rk-brand{justify-content:flex-start}}"

#define PAGE_FRAME_FOOT \
  "<footer class=\"rk-foot\"><a href=\"https://" FIRMWARE_SITE "\" target=\"_blank\" rel=\"noopener\">" FIRMWARE_SITE "</a>" \
  " &middot; <a href=\"" MANUAL_URL "\" target=\"_blank\" rel=\"noopener\">User manual</a>" \
  " &middot; <a href=\"" FIRMWARE_URL "\" target=\"_blank\" rel=\"noopener\">GitHub</a></footer>\n"

static const char pageFrameBrand[] PROGMEM = R"HTML(<header class="rk-brand"><a href="/" title="Status"><svg viewBox="0 0 53 48" width="34" height="31" aria-hidden="true"><rect x="0" y="40" width="5" height="8" rx="2" fill="#DCE6EA"/><rect x="8" y="36" width="5" height="12" rx="2" fill="#DCE6EA"/><rect x="16" y="28" width="5" height="20" rx="2" fill="#DCE6EA"/><rect x="24" y="0" width="5" height="48" rx="2" fill="#4DABF7"/><rect x="32" y="28" width="5" height="20" rx="2" fill="#DCE6EA"/><rect x="40" y="36" width="5" height="12" rx="2" fill="#DCE6EA"/><rect x="48" y="40" width="5" height="8" rx="2" fill="#DCE6EA"/></svg><span><b>RESONIK</b><small>ATS Mini firmware {{version}}</small></span></a>{{{right}}}</header>
)HTML";

static const char pageStart[] PROGMEM = R"HTML(<!DOCTYPE HTML>
<HTML>
<HEAD>
  <META CHARSET="UTF-8">
  <META NAME="viewport" CONTENT="width=device-width, initial-scale=1.0">
  <TITLE>RESONIK {{title}}</TITLE>
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
    INPUT[type=file] { color: var(--muted); width: 100%; max-width: 100%; box-sizing: border-box; }
    .CENTER { text-align: center; }
    .HEAP { color: var(--muted); font-size: 0.75em; text-align: center; margin: 1.5em 0.5em; }
  )HTML" PAGE_FRAME_CSS R"HTML(
  </STYLE>
</HEAD>
<BODY>
)HTML";

static const char pageEnd[] PROGMEM = PAGE_FRAME_FOOT R"HTML(
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
  out += R"HTML(<nav class="rk-nav">)HTML";
  for(size_t i = 0; i < sizeof(pages) / sizeof(pages[0]); i++)
  {
    if(!strcmp(activePage, pages[i].path)) { out += "<span>"; out += pages[i].name; out += "</span>"; }
    else pageAppend(out, R"HTML(<a href="{{path}}">{{label}}</a>)HTML", {{"path", pages[i].path}, {"label", pages[i].name}});
  }
  out += "</nav>\n";
}

// "v2.46-r5", as in the RESONIK bar of every page
static String pageVersion()
{
  char text[16];
  snprintf(text, sizeof(text), "v%d.%02d-r%d", VER_APP / 100, VER_APP % 100, VER_RESONIK);
  return(String(text));
}

// The top of every page: the RESONIK bar, then the page menu with this page
// lit. right: HTML for the bar's right end (Web Control's live/offline).
static void webFrameTop(String &out, const char *activePage, const char *right = "")
{
  pageAppend(out, pageFrameBrand, {{"version", pageVersion()}, {"right", String(right)}});
  webNavigation(out, activePage);
}

#endif // PAGE_COMMON_H
