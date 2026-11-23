#ifndef PAGE_LOG_H
#define PAGE_LOG_H

#include <Arduino.h>

// RESONIK R5: the Log page of the web portal. It was "Log (CSV)", a download
// only; 23 Nov: "logger shall have more options ... activation, download,
// view and delete". The logger itself is in Resonik.cpp.
static const char pageLogTitle[] PROGMEM = "Log";

#define PAGE_LOG_LINES 50    // lines shown, newest first
#define PAGE_LOG_TAIL  6144  // bytes read from the end of the file for them

static const char pageLog[] PROGMEM = R"HTML(<STYLE>
.LOGWRAP { max-width: 768px; margin: 0 auto; overflow-x: auto; }
.LOG { width: 100%; border-collapse: collapse; font-size: 13px; }
.LOG TH, .LOG TD { padding: 0.35em 0.6em; white-space: nowrap; text-align: left; border-bottom: 1px solid var(--line); }
.LOG TH { color: var(--muted); font-weight: 600; }
.LOG .N { text-align: right; font-variant-numeric: tabular-nums; }
.LOG TR.DAY TD { color: var(--muted); font-weight: 600; padding-top: 0.9em; border-bottom: 1px solid var(--blue); }
.LOGBTN INPUT[type=submit] { width: auto; margin: 0.25em; }
@media (max-width: 480px) { .LOG TH, .LOG TD { padding: 0.35em 0.4em; } .LOG .B { display: none; } }
</STYLE>
<FORM ACTION="/log" METHOD="POST">
  <TABLE COLUMNS="2">
    <TR><TH COLSPAN="2" CLASS="HEADING">Signal logger</TH></TR>
    <TR><TD CLASS="LABEL">Logger</TD><TD><SELECT NAME="interval">{{{interval_options}}}</SELECT><BR><SMALL>One line each time: UTC time, frequency, mode, band, signal (dB&micro;V), SNR and station name. On the radio: Menu &rarr; Memory &rarr; Logger.</SMALL></TD></TR>
    <TR><TD CLASS="LABEL">Log&nbsp;file</TD><TD>{{file_state}}</TD></TR>
    <TR><TH COLSPAN="2" CLASS="HEADING"><INPUT TYPE="SUBMIT" VALUE="Save"></TH></TR>
  </TABLE>
</FORM>
<TABLE COLUMNS="1">
  <TR><TH CLASS="HEADING">The log</TH></TR>
  <TR><TD CLASS="CENTER LOGBTN"><INPUT TYPE="SUBMIT" VALUE="Download (CSV)" FORM="logget" {{disabled}}> <INPUT TYPE="SUBMIT" VALUE="Delete log" FORM="logclear" {{disabled}}></TD></TR>
</TABLE>
{{{lines}}}
<FORM ID="logget" ACTION="/log.csv" METHOD="GET"></FORM>
<FORM ID="logclear" ACTION="/log" METHOD="POST" ONSUBMIT="return confirm('Delete the signal log? It cannot be brought back.')"><INPUT TYPE="HIDDEN" NAME="clear" VALUE="1"></FORM>
)HTML";

static void pageLogText(String &out, const char *s)
{
  for( ; *s ; s++)
    switch(*s)
    {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      default:  out += *s; break;
    }
}

// The latest lines of the log as a table, newest first, under a line for each
// day (so the time column stays narrow enough for a phone; Band is hidden there
// too). buf holds the end of the file (n bytes and a NUL after them) and is cut
// up in place; partial says that its first line is cut short and that the file
// has more before it.
static void pageLogRows(String &out, char *buf, size_t n, bool partial)
{
  char *line[PAGE_LOG_LINES];
  size_t count = 0;
  char *p = buf, *end = buf + n;

  if(partial)
  {
    char *nl = (char *)memchr(p, '\n', end - p);
    p = nl ? nl + 1 : end;
  }
  while(p < end)
  {
    char *nl = (char *)memchr(p, '\n', end - p);
    char *e = nl ? nl : end;
    *e = '\0';
    if(e > p && e[-1] == '\r') e[-1] = '\0';
    // Skip the header line and empty lines
    if(*p && strncmp(p, "utc,", 4)) line[count++ % PAGE_LOG_LINES] = p;
    p = nl ? nl + 1 : end;
  }

  if(!count)
  {
    out += "<P CLASS=\"CENTER\"><SMALL>No lines yet.</SMALL></P>";
    return;
  }

  size_t shown = count < PAGE_LOG_LINES ? count : PAGE_LOG_LINES;
  out += "<DIV CLASS=\"LOGWRAP\"><TABLE CLASS=\"LOG\"><TR><TH>UTC</TH><TH CLASS=\"N\">Frequency</TH><TH>Mode</TH>"
         "<TH CLASS=\"B\">Band</TH><TH CLASS=\"N\">dB&micro;V</TH><TH CLASS=\"N\">SNR</TH><TH>Station</TH></TR>";
  char prevDay[11] = "";
  for(size_t i = 0 ; i < shown ; i++)
  {
    // utc,frequency_hz,mode,band,rssi_dbuv,snr_db,station
    char *f[7];
    size_t k = 0;
    char *q = line[(count - 1 - i) % PAGE_LOG_LINES];
    f[k++] = q;
    for( ; *q && k < 7 ; q++)
      if(*q == ',') { *q = '\0'; f[k++] = q + 1; }
    for( ; k < 7 ; k++) f[k] = q;

    // 2026-11-23T05:16:22Z: the day on a line of its own when it changes, then
    // 05:16:22. "+123s": seconds after power-on, written while the clock was not set
    bool dated = strlen(f[0]) == 20 && f[0][10] == 'T';
    char day[11];
    if(dated) { memcpy(day, f[0], 10); day[10] = '\0'; }
    else strcpy(day, "-");
    if(strcmp(day, prevDay))
    {
      out += "<TR CLASS=\"DAY\"><TD COLSPAN=\"7\">";
      out += dated ? day : "No clock: seconds after power-on";
      out += "</TD></TR>";
      strcpy(prevDay, day);
    }
    out += "<TR><TD>";
    if(dated) out.concat(f[0] + 11, 8);
    else pageLogText(out, f[0]);

    char freq[24];
    unsigned long hz = strtoul(f[1], nullptr, 10);
    if(!strcmp(f[2], "FM"))
      snprintf(freq, sizeof(freq), "%lu.%02lu MHz", hz / 1000000UL, (hz / 10000UL) % 100UL);
    else if(hz % 1000UL == 0)
      snprintf(freq, sizeof(freq), "%lu kHz", hz / 1000UL);
    else
    {
      int len = snprintf(freq, sizeof(freq), "%lu.%03lu", hz / 1000UL, hz % 1000UL);
      while(len > 0 && freq[len - 1] == '0') freq[--len] = '\0';
      snprintf(freq + len, sizeof(freq) - len, " kHz");
    }
    out += "</TD><TD CLASS=\"N\">";
    out += freq;
    out += "</TD><TD>";
    pageLogText(out, f[2]);
    out += "</TD><TD CLASS=\"B\">";
    pageLogText(out, f[3]);
    out += "</TD><TD CLASS=\"N\">";
    pageLogText(out, f[4]);
    out += "</TD><TD CLASS=\"N\">";
    pageLogText(out, f[5]);
    out += "</TD><TD>";
    pageLogText(out, f[6]);
    out += "</TD></TR>";
  }

  char note[96];
  if(partial || count > PAGE_LOG_LINES)
    snprintf(note, sizeof(note), "The latest %u lines, newest first. Download the file for all of them.", (unsigned)shown);
  else
    snprintf(note, sizeof(note), "All %u lines, newest first.", (unsigned)shown);
  out += "</TABLE></DIV><P CLASS=\"CENTER\"><SMALL>";
  out += note;
  out += "</SMALL></P>";
}

#endif // PAGE_LOG_H
