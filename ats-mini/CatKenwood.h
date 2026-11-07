#ifndef CAT_KENWOOD_H
#define CAT_KENWOOD_H

// Kenwood TS-480 compatible CAT, receive-only subset (RESONIK).
// Enough for Hamlib (rig 2028 "TS-480"), flrig, WSJT-X, fldigi, OmniRig and
// logging programs to read and set frequency and mode and read the S-meter.
// Pure logic: the radio is reached through the CatRadio callbacks, so the
// parser runs unchanged in host tests.

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Kenwood mode numbers
enum : uint8_t { KW_LSB = 1, KW_USB = 2, KW_CW = 3, KW_FM = 4, KW_AM = 5, KW_FSK = 6, KW_CWR = 7, KW_FSKR = 9 };

struct CatRadio
{
  uint32_t (*getFreq)();               // Hz
  bool     (*setFreq)(uint32_t hz);    // false if it cannot be tuned
  uint8_t  (*getMode)();               // Kenwood mode number
  bool     (*setMode)(uint8_t kwMode);
  uint8_t  (*getSmeter)();             // 0..30
  uint8_t  (*getVolume)();             // 0..255
  void     (*setVolume)(uint8_t v);    // 0..255
};

struct CatKenwood
{
  char cmd[40];
  uint8_t len = 0;
  bool overflow = false;
  uint8_t autoInfo = 0;

  // Feed one byte; when a command ends with ';', the reply (possibly empty)
  // is written to out (at least 48 bytes) and true is returned.
  bool feed(char c, const CatRadio &r, char *out)
  {
    if(c == '\r' || c == '\n' || c == ' ') return(false);
    if(c != ';')
    {
      if(len < sizeof(cmd) - 1) cmd[len++] = c; else overflow = true;
      return(false);
    }
    cmd[len] = '\0';
    bool bad = overflow;
    len = 0;
    overflow = false;
    out[0] = '\0';
    if(bad) { strcpy(out, "?;"); return(true); }
    execute(r, out);
    return(true);
  }

  void execute(const CatRadio &r, char *out)
  {
    if(strlen(cmd) < 2) { strcpy(out, "?;"); return; }
    char a = toupper(cmd[0]), b = toupper(cmd[1]);
    const char *arg = cmd + 2;
    size_t n = strlen(arg);

    if(a == 'I' && b == 'D' && !n) { strcpy(out, "ID020;"); return; }
    if(a == 'P' && b == 'S') { strcpy(out, n ? "" : "PS1;"); return; }
    if(a == 'A' && b == 'I')
    {
      if(n) autoInfo = arg[0] - '0'; else sprintf(out, "AI%u;", autoInfo);
      return;
    }
    if((a == 'F' && (b == 'A' || b == 'B')))
    {
      if(!n) { sprintf(out, "F%c%011lu;", b, (unsigned long)r.getFreq()); return; }
      if(b == 'B') return;                       // Single receiver: VFO B is ignored
      if(!allDigits(arg)) { strcpy(out, "?;"); return; }
      if(!r.setFreq((uint32_t)strtoul(arg, nullptr, 10))) strcpy(out, "?;");
      return;
    }
    if(a == 'F' && (b == 'R' || b == 'T'))
    {
      if(!n) sprintf(out, "F%c0;", b);
      return;
    }
    if(a == 'M' && b == 'D')
    {
      if(!n) { sprintf(out, "MD%u;", r.getMode()); return; }
      if(n != 1 || !isdigit((unsigned char)arg[0]) || !r.setMode(arg[0] - '0')) strcpy(out, "?;");
      return;
    }
    if(a == 'I' && b == 'F' && !n)
    {
      // IF: freq(11) step(5 blanks) RIT(+0000) RIT XIT bank ch(2) TX mode VFO scan split tone tone#(2) blank
      sprintf(out, "IF%011lu     +0000000000%u000000 ;", (unsigned long)r.getFreq(), r.getMode());
      return;
    }
    if(a == 'S' && b == 'M')
    {
      sprintf(out, "SM0%04u;", r.getSmeter());
      return;
    }
    if(a == 'A' && b == 'G')
    {
      // AG0; read, AG0nnn; set (main receiver)
      if(n <= 1) { sprintf(out, "AG0%03u;", r.getVolume()); return; }
      if(n == 4 && allDigits(arg)) { r.setVolume((uint8_t)atoi(arg + 1)); return; }
      strcpy(out, "?;");
      return;
    }
    if(a == 'R' && b == 'X') { strcpy(out, "RX0;"); return; }
    if(a == 'T' && b == 'X') { strcpy(out, "?;"); return; }   // Receiver only
    if(a == 'R' && b == 'S' && !n) { strcpy(out, "RS0;"); return; }
    if(a == 'K' && b == 'S' && !n) { strcpy(out, "KS020;"); return; }
    if(a == 'S' && b == 'H' && !n) { strcpy(out, "SH00;"); return; }
    if(a == 'S' && b == 'L' && !n) { strcpy(out, "SL00;"); return; }
    if(a == 'R' && b == 'T' && !n) { strcpy(out, "RT0;"); return; }
    if(a == 'X' && b == 'T' && !n) { strcpy(out, "XT0;"); return; }
    if(a == 'R' && b == 'F' && !n) { strcpy(out, "RF0;"); return; }   // TS-2000 style; harmless
    strcpy(out, "?;");
  }

  static bool allDigits(const char *s)
  {
    if(!*s) return(false);
    for( ; *s ; s++) if(!isdigit((unsigned char)*s)) return(false);
    return(true);
  }
};

// S-meter: SI4732 RSSI (dBuV) to the TS-480 scale 0..30 (S9 = 15, S9+60 = 30)
static inline uint8_t catSmeterFromRssi(uint8_t rssi)
{
  // About S1 at 1 dBuV, S9 at 34 dBuV (50 uV / 50 ohm on HF), 6 dB per S-unit
  int s;
  if(rssi < 1) s = 0;
  else if(rssi <= 34) s = 1 + (rssi - 1) * 14 / 33;   // S1..S9 -> 1..15
  else s = 15 + (rssi - 34) / 4;                       // +4 dB per step above S9
  return((uint8_t)(s > 30 ? 30 : s));
}

#endif // CAT_KENWOOD_H
