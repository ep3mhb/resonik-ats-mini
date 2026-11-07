#ifndef EIBI_PARSE_H
#define EIBI_PARSE_H

// EiBi schedule parsing (RESONIK): the season's CSV file with days of the
// week, language and target area. Pure functions with host tests.
// CSV fields: kHz;Time(UTC);Days;ITU;Station;Lng;Target;Remarks;P;Start;Stop

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define EIBI_ALL_DAYS 0x7F   // Bit 0 = Monday ... bit 6 = Sunday

struct StationSchedule
{
  uint16_t freq;        // Frequency in kHz
  int8_t   start_h;     // Starting hour (0..23, -1 = any)
  int8_t   start_m;     // Starting minute
  int8_t   end_h;       // Ending hour
  int8_t   end_m;       // Ending minute
  uint8_t  days;        // Days on air, EIBI_ALL_DAYS for daily or irregular
  char     lang[4];     // Language code, e.g. "E", "A", "F"
  char     target[5];   // Target area, e.g. "ME", "WEu"
  char     name[32];    // Station name
};

// Days field: "" (daily), "Mo-Fr", "Sa", "Sa,Su", "1245" or "1-5" (1 = Monday).
// Anything else ("irr", dates, "alt"...) is treated as daily.
static inline uint8_t eibiParseDays(const char *s)
{
  static const char *names[7] = { "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };
  while(*s == ' ') s++;
  if(!*s) return(EIBI_ALL_DAYS);

  uint8_t mask = 0;
  int prev = -1;
  bool range = false;
  for(const char *p = s ; *p && *p != ' ' ; )
  {
    int d = -1;
    if(*p >= '1' && *p <= '7') { d = *p - '1'; p++; }
    else
    {
      for(int i = 0 ; i < 7 ; i++)
        if(p[0] == names[i][0] && p[1] == names[i][1]) { d = i; break; }
      if(d < 0) return(EIBI_ALL_DAYS);
      p += 2;
    }
    if(range && prev >= 0)
    {
      for(int i = prev ; ; i = (i + 1) % 7) { mask |= 1 << i; if(i == d) break; }
      range = false;
    }
    else mask |= 1 << d;
    prev = d;
    if(*p == '-') { range = true; p++; }
    else if(*p == ',') p++;
    else if(*p && *p != ' ' && !(*p >= '1' && *p <= '7') && !isupper((unsigned char)*p)) return(EIBI_ALL_DAYS);
  }
  return(mask ? mask : EIBI_ALL_DAYS);
}

// Copy a field, trimmed, dropping characters outside printable ASCII
static inline void eibiCopyField(char *dst, size_t size, const char *src, size_t len)
{
  while(len && (*src == ' ' || *src == '\t')) { src++; len--; }
  while(len && (src[len - 1] == ' ' || src[len - 1] == '\t' || src[len - 1] == '\r')) len--;
  size_t j = 0;
  for(size_t i = 0 ; i < len && j + 1 < size ; i++)
  {
    unsigned char c = (unsigned char)src[i];
    if(c >= 0x20 && c < 0x7F) dst[j++] = (char)c;
  }
  dst[j] = '\0';
}

// One CSV line into entry; false for headers, jammers and malformed lines
static inline bool eibiParseCsvLine(const char *line, StationSchedule &e)
{
  const char *f[8];
  size_t l[8];
  const char *p = line;
  int n = 0;
  while(n < 8)
  {
    const char *q = strchr(p, ';');
    f[n] = p;
    l[n] = q ? (size_t)(q - p) : strlen(p);
    n++;
    if(!q) break;
    p = q + 1;
  }
  if(n < 7) return(false);

  memset(&e, 0, sizeof(e));
  char buf[16];
  eibiCopyField(buf, sizeof(buf), f[0], l[0]);
  if(!isdigit((unsigned char)buf[0])) return(false);
  double khz = atof(buf);
  if(khz < 1 || khz > 65535) return(false);
  e.freq = (uint16_t)khz;

  eibiCopyField(buf, sizeof(buf), f[1], l[1]);
  int sh, sm, eh, em;
  if(sscanf(buf, "%2d%2d-%2d%2d", &sh, &sm, &eh, &em) != 4) return(false);
  e.start_h = sh; e.start_m = sm; e.end_h = eh; e.end_m = em;

  eibiCopyField(buf, sizeof(buf), f[2], l[2]);
  e.days = eibiParseDays(buf);

  eibiCopyField(e.name, sizeof(e.name), f[4], l[4]);
  if(!e.name[0] || strstr(e.name, "Jammer")) return(false);
  eibiCopyField(e.lang, sizeof(e.lang), f[5], l[5]);
  eibiCopyField(e.target, sizeof(e.target), f[6], l[6]);
  return(true);
}

// EiBi season file names: A (summer, last Sunday of March to last Sunday of
// October) and B (winter). B runs into the next year: "b26" lasts until March 2027.
static inline void eibiSeasonFiles(int year, int month, char cur[16], char prev[16])
{
  // Months 4..10 are season A; November..December are B of this year; January..March
  // are B of the previous year. The switchover weeks are covered by the fallback.
  if(month >= 4 && month <= 10)
  {
    snprintf(cur, 16, "sked-a%02d.csv", year % 100);
    snprintf(prev, 16, "sked-b%02d.csv", (year + 99) % 100);
  }
  else if(month >= 11)
  {
    snprintf(cur, 16, "sked-b%02d.csv", year % 100);
    snprintf(prev, 16, "sked-a%02d.csv", year % 100);
  }
  else
  {
    snprintf(cur, 16, "sked-b%02d.csv", (year + 99) % 100);
    snprintf(prev, 16, "sked-a%02d.csv", (year + 99) % 100);
  }
}

// Weekday bit for a UTC epoch (bit 0 = Monday)
static inline uint8_t eibiDayBit(uint32_t utc)
{
  int wd = (int)((utc / 86400 + 3) % 7);   // 1970-01-01 was a Thursday: 0 = Monday
  return((uint8_t)(1 << wd));
}

#endif // EIBI_PARSE_H
