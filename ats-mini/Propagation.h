#ifndef PROPAGATION_H
#define PROPAGATION_H

// Solar and HF propagation data from N0NBH's hamqsl.com feed (RESONIK).
// Pure parser with host tests; fetching and drawing are in Propagation.cpp.

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define PROP_URL "https://www.hamqsl.com/solarxml.php"

enum PropCond : uint8_t { PROP_UNKNOWN = 0, PROP_POOR, PROP_FAIR, PROP_GOOD };

// Band groups as published: 80m-40m, 30m-20m, 17m-15m, 12m-10m
static const char *const propBandNames[4] = { "80m-40m", "30m-20m", "17m-15m", "12m-10m" };

typedef struct
{
  bool     valid;
  int16_t  sfi;            // Solar flux index
  int16_t  ssn;            // Sunspot number
  int16_t  aIndex;
  int16_t  kIndex;
  char     xray[8];        // e.g. "B5.6"
  char     updated[24];    // As published, e.g. "07 Nov 2026 0745 GMT"
  char     geomag[16];     // e.g. "Quiet"
  PropCond day[4];
  PropCond night[4];
} PropData;

// Copy the text of <tag>...</tag> (first occurrence) into out; trims spaces
static inline bool propTag(const char *xml, const char *tag, char *out, size_t size)
{
  char open[24];
  size_t tl = strlen(tag);
  if(tl + 3 > sizeof(open)) return(false);
  open[0] = '<'; memcpy(open + 1, tag, tl); open[tl + 1] = '>'; open[tl + 2] = '\0';
  const char *p = strstr(xml, open);
  if(!p) return(false);
  p += tl + 2;
  const char *e = strchr(p, '<');
  if(!e) return(false);
  while(p < e && isspace((unsigned char)*p)) p++;
  while(e > p && isspace((unsigned char)e[-1])) e--;
  size_t n = (size_t)(e - p);
  if(n >= size) n = size - 1;
  memcpy(out, p, n);
  out[n] = '\0';
  return(true);
}

static inline PropCond propCondFrom(const char *s)
{
  while(isspace((unsigned char)*s)) s++;
  if(!strncmp(s, "Good", 4)) return(PROP_GOOD);
  if(!strncmp(s, "Fair", 4)) return(PROP_FAIR);
  if(!strncmp(s, "Poor", 4)) return(PROP_POOR);
  return(PROP_UNKNOWN);
}

static inline bool propParse(const char *xml, PropData *d)
{
  char buf[32];
  memset(d, 0, sizeof(*d));
  if(!strstr(xml, "<solardata>")) return(false);

  if(propTag(xml, "solarflux", buf, sizeof(buf))) d->sfi = (int16_t)atoi(buf); else return(false);
  if(propTag(xml, "sunspots", buf, sizeof(buf)))  d->ssn = (int16_t)atoi(buf);
  if(propTag(xml, "aindex", buf, sizeof(buf)))    d->aIndex = (int16_t)atoi(buf);
  if(propTag(xml, "kindex", buf, sizeof(buf)))    d->kIndex = (int16_t)atoi(buf);
  propTag(xml, "xray", d->xray, sizeof(d->xray));
  propTag(xml, "updated", d->updated, sizeof(d->updated));
  propTag(xml, "geomagfield", d->geomag, sizeof(d->geomag));

  // <band name="80m-40m" time="day">Good</band> inside <calculatedconditions>
  const char *cc = strstr(xml, "<calculatedconditions>");
  if(cc)
  {
    for(const char *p = strstr(cc, "<band ") ; p ; p = strstr(p + 1, "<band "))
    {
      const char *end = strchr(p, '>');
      if(!end) break;
      for(int i = 0 ; i < 4 ; i++)
      {
        char key[24];
        size_t kl = strlen(propBandNames[i]);
        memcpy(key, "name=\"", 6); memcpy(key + 6, propBandNames[i], kl); key[6 + kl] = '"'; key[7 + kl] = '\0';
        const char *k = strstr(p, key);
        if(!k || k > end) continue;
        const char *t = strstr(p, "time=\"");
        if(!t || t > end) continue;
        PropCond c = propCondFrom(end + 1);
        if(!strncmp(t + 6, "day", 3)) d->day[i] = c;
        else if(!strncmp(t + 6, "night", 5)) d->night[i] = c;
      }
      if(strstr(p, "</calculatedconditions>") == p) break;
    }
  }
  d->valid = true;
  return(true);
}

#endif // PROPAGATION_H
