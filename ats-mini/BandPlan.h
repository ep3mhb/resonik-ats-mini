#ifndef BAND_PLAN_H
#define BAND_PLAN_H

// IARU Region 1 HF band plan (simplified, receive-only guidance).
// Source: IARU Region 1 HF Managers' Handbook band plan (2020 revision).
// Pure data and lookups, with host tests in tests/TestBandPlan.cpp.

#include <stdint.h>

enum BandPlanUse : uint8_t
{
  BP_NONE = 0,
  BP_CW,       // CW only
  BP_NARROW,   // Narrow band modes: CW and digital (FT8, PSK, RTTY...)
  BP_BEACON,   // IBP beacons (NCDXF/IARU); listen only
  BP_ALL,      // All modes: SSB phone, also CW and digital
  BP_SAT,      // Satellite service
  BP_FM,       // FM simplex and repeaters (10 m)
  BP_BLOCKED,  // Outside the national allocation: not allowed
};

typedef struct
{
  uint32_t start;   // Hz, inclusive
  uint32_t end;     // Hz, exclusive
  BandPlanUse use;
} BandPlanSegment;

static const BandPlanSegment bandPlanR1[] =
{
  // 160 m
  {  1810000,  1838000, BP_CW     },
  {  1838000,  1843000, BP_NARROW },
  {  1843000,  2000000, BP_ALL    },
  // 80 m
  {  3500000,  3570000, BP_CW     },
  {  3570000,  3600000, BP_NARROW },
  {  3600000,  3800000, BP_ALL    },
  // 60 m (WRC-15 secondary allocation)
  {  5351500,  5354000, BP_NARROW },
  {  5354000,  5366000, BP_ALL    },
  {  5366000,  5366500, BP_NARROW },
  // 40 m
  {  7000000,  7040000, BP_CW     },
  {  7040000,  7053000, BP_NARROW },
  {  7053000,  7200000, BP_ALL    },
  // 30 m (no phone)
  { 10100000, 10130000, BP_CW     },
  { 10130000, 10150000, BP_NARROW },
  // 20 m
  { 14000000, 14070000, BP_CW     },
  { 14070000, 14099000, BP_NARROW },
  { 14099000, 14101000, BP_BEACON },
  { 14101000, 14112000, BP_NARROW },
  { 14112000, 14350000, BP_ALL    },
  // 17 m
  { 18068000, 18095000, BP_CW     },
  { 18095000, 18109000, BP_NARROW },
  { 18109000, 18111000, BP_BEACON },
  { 18111000, 18168000, BP_ALL    },
  // 15 m
  { 21000000, 21070000, BP_CW     },
  { 21070000, 21149000, BP_NARROW },
  { 21149000, 21151000, BP_BEACON },
  { 21151000, 21450000, BP_ALL    },
  // 12 m
  { 24890000, 24915000, BP_CW     },
  { 24915000, 24929000, BP_NARROW },
  { 24929000, 24931000, BP_BEACON },
  { 24931000, 24990000, BP_ALL    },
  // 10 m
  { 28000000, 28070000, BP_CW     },
  { 28070000, 28190000, BP_NARROW },
  { 28190000, 28225000, BP_BEACON },
  { 28225000, 29300000, BP_ALL    },
  { 29300000, 29510000, BP_SAT    },
  { 29520000, 29700000, BP_FM     },
};

// Segment containing freq, or nullptr outside the amateur bands
static inline const BandPlanSegment *bandPlanFind(uint32_t freq)
{
  for(unsigned i = 0 ; i < sizeof(bandPlanR1) / sizeof(bandPlanR1[0]) ; i++)
    if(freq >= bandPlanR1[i].start && freq < bandPlanR1[i].end) return(&bandPlanR1[i]);
  return(nullptr);
}

// Plan areas: the IARU Region 1 plan, narrowed where a country's licence
// allows less. UAE: TDRA Regulations for Amateur Radio v3.0 (Dec 2020),
// art. 4.1 (160 m is 1810-1850 kHz only). Iran: Region 1 until the CRA
// table is confirmed.
enum BandPlanArea : uint8_t { BP_AREA_R1 = 0, BP_AREA_UAE, BP_AREA_IRAN, BP_AREA_COUNT };

static const BandPlanSegment bandPlanBlockedUAE[] =
{
  {  1850000,  2000000, BP_BLOCKED },
};

static inline const BandPlanSegment *bandPlanBlocked(uint8_t area, unsigned *count)
{
  if(area == BP_AREA_UAE) { *count = sizeof(bandPlanBlockedUAE) / sizeof(bandPlanBlockedUAE[0]); return(bandPlanBlockedUAE); }
  *count = 0;
  return(nullptr);
}

// Segment for an area: a blocked part wins over the Region 1 segment
static inline const BandPlanSegment *bandPlanFindArea(uint32_t freq, uint8_t area)
{
  unsigned n;
  const BandPlanSegment *b = bandPlanBlocked(area, &n);
  for(unsigned i = 0 ; i < n ; i++)
    if(freq >= b[i].start && freq < b[i].end) return(&b[i]);
  return(bandPlanFind(freq));
}

// Short label for the screen
static inline const char *bandPlanLabel(BandPlanUse use)
{
  switch(use)
  {
    case BP_CW:     return("CW");
    case BP_NARROW: return("DIGI");
    case BP_BEACON: return("BCN");
    case BP_ALL:    return("SSB");
    case BP_SAT:    return("SAT");
    case BP_FM:     return("FM");
    case BP_BLOCKED: return("NOT ALLOWED");
    default:        return("");
  }
}

// Conventional sideband for phone: LSB below 10 MHz, USB above (60 m uses USB)
static inline bool bandPlanPhoneIsLSB(uint32_t freq)
{
  return(freq < 10000000 && !(freq >= 5351500 && freq < 5366500));
}

#endif // BAND_PLAN_H
