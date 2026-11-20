#ifndef PAGE_STATUS_H
#define PAGE_STATUS_H

#include <Arduino.h>

static const char pageStatusTitle[] PROGMEM = "ATS-Mini Status";

static const char pageStatus[] PROGMEM = R"HTML(<H1>{{title}}</H1>
{{{navigation}}}
<STYLE>.v-good{color:#06D7A0}.v-fair{color:#FFD167}.v-bad{color:#FF6B6B}</STYLE>
<TABLE COLUMNS="2">
  <TR><TD CLASS="LABEL">IP Address</TD><TD><A HREF="http://{{ip}}">{{ip}}</A> ({{ssid}})</TD></TR>
  <TR><TD CLASS="LABEL">MAC Address</TD><TD>{{mac}}</TD></TR>
  <TR><TD CLASS="LABEL">Firmware</TD><TD>{{version}}</TD></TR>
  <TR><TD CLASS="LABEL">Date/Time</TD><TD>{{time}}</TD></TR>
  <TR><TD CLASS="LABEL">Band</TD><TD>{{band}}</TD></TR>
  <TR><TD CLASS="LABEL">Frequency</TD><TD>{{frequency}}{{mode}}</TD></TR>
  <TR><TD CLASS="LABEL">Signal Strength</TD><TD>{{rssi}} dB&micro;V &mdash; <SPAN CLASS="v-{{rssi_c}}">{{rssi_v}}</SPAN></TD></TR>
  <TR><TD CLASS="LABEL">Signal to Noise</TD><TD>{{snr}} dB &mdash; <SPAN CLASS="v-{{snr_c}}">{{snr_v}}</SPAN></TD></TR>
  <TR><TD CLASS="LABEL">Battery Voltage</TD><TD>{{battery}} V &mdash; <SPAN CLASS="v-{{bat_c}}">{{bat_v}}</SPAN></TD></TR>
  <TR><TD CLASS="LABEL">Running for</TD><TD>{{uptime}}</TD></TR>
  <TR><TD CLASS="LABEL">Last restart</TD><TD>{{reset}} &mdash; <SPAN CLASS="v-{{rst_c}}">{{rst_v}}</SPAN></TD></TR>
  <TR><TD CLASS="LABEL">Processor</TD><TD>{{temp}} &deg;C, {{cpu}} MHz &mdash; <SPAN CLASS="v-{{tmp_c}}">{{tmp_v}}</SPAN></TD></TR>
  <TR><TD CLASS="LABEL">Memory free</TD><TD>{{heap}} KiB internal, {{psram}} KiB PSRAM (lowest {{heapmin}} KiB) &mdash; <SPAN CLASS="v-{{mem_c}}">{{mem_v}}</SPAN></TD></TR>
  <TR><TD CLASS="LABEL">Wi-Fi signal</TD><TD>{{wrssi}} &mdash; <SPAN CLASS="v-{{wifi_c}}">{{wifi_v}}</SPAN></TD></TR>
</TABLE>
<P CLASS="CENTER"><A HREF="/screenshot.bmp">Download screenshot</A></P>
)HTML";

#endif // PAGE_STATUS_H
