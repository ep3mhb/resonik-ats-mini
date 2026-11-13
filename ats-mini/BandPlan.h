#ifndef BAND_PLAN_H
#define BAND_PLAN_H

// IARU HF band plans (simplified, receive-only guidance): Region 1 in full,
// Regions 2 and 3 where they differ, and national limits for Iran and UAE.
// Region 1:
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

// IARU Region 2 (the Americas), simplified from the IARU R2 HF band plan
// (2019): wider 160, 80 and 40 m; the higher bands match Region 1.
static const BandPlanSegment bandPlanR2[] =
{
  {  1800000,  1810000, BP_NARROW },
  {  1810000,  1840000, BP_CW     },
  {  1840000,  1850000, BP_NARROW },
  {  1850000,  2000000, BP_ALL    },
  {  3500000,  3570000, BP_CW     },
  {  3570000,  3600000, BP_NARROW },
  {  3600000,  4000000, BP_ALL    },
  {  5330500,  5351500, BP_ALL    },   // US channels below the WRC-15 band
  {  5351500,  5354000, BP_NARROW },
  {  5354000,  5366000, BP_ALL    },
  {  5366000,  5366500, BP_NARROW },
  {  5366500,  5406500, BP_ALL    },   // US channels above it
  {  7000000,  7040000, BP_CW     },
  {  7040000,  7060000, BP_NARROW },
  {  7060000,  7300000, BP_ALL    },
  { 10100000, 10130000, BP_CW     },
  { 10130000, 10150000, BP_NARROW },
  { 14000000, 14070000, BP_CW     },
  { 14070000, 14099000, BP_NARROW },
  { 14099000, 14101000, BP_BEACON },
  { 14101000, 14112000, BP_NARROW },
  { 14112000, 14350000, BP_ALL    },
  { 18068000, 18095000, BP_CW     },
  { 18095000, 18109000, BP_NARROW },
  { 18109000, 18111000, BP_BEACON },
  { 18111000, 18168000, BP_ALL    },
  { 21000000, 21070000, BP_CW     },
  { 21070000, 21149000, BP_NARROW },
  { 21149000, 21151000, BP_BEACON },
  { 21151000, 21450000, BP_ALL    },
  { 24890000, 24915000, BP_CW     },
  { 24915000, 24929000, BP_NARROW },
  { 24929000, 24931000, BP_BEACON },
  { 24931000, 24990000, BP_ALL    },
  { 28000000, 28070000, BP_CW     },
  { 28070000, 28190000, BP_NARROW },
  { 28190000, 28225000, BP_BEACON },
  { 28225000, 29300000, BP_ALL    },
  { 29300000, 29510000, BP_SAT    },
  { 29520000, 29700000, BP_FM     },
};

// IARU Region 3 (Asia-Pacific), simplified from the IARU R3 band plan:
// 160 m from 1800, 80 m to 3900, 40 m to 7300; the higher bands match Region 1.
static const BandPlanSegment bandPlanR3[] =
{
  {  1800000,  1830000, BP_CW     },
  {  1830000,  1843000, BP_NARROW },
  {  1843000,  2000000, BP_ALL    },
  {  3500000,  3535000, BP_CW     },
  {  3535000,  3600000, BP_NARROW },
  {  3600000,  3900000, BP_ALL    },
  {  5351500,  5354000, BP_NARROW },
  {  5354000,  5366000, BP_ALL    },
  {  5366000,  5366500, BP_NARROW },
  {  7000000,  7030000, BP_CW     },
  {  7030000,  7060000, BP_NARROW },
  {  7060000,  7300000, BP_ALL    },
  { 10100000, 10130000, BP_CW     },
  { 10130000, 10150000, BP_NARROW },
  { 14000000, 14070000, BP_CW     },
  { 14070000, 14099000, BP_NARROW },
  { 14099000, 14101000, BP_BEACON },
  { 14101000, 14112000, BP_NARROW },
  { 14112000, 14350000, BP_ALL    },
  { 18068000, 18095000, BP_CW     },
  { 18095000, 18109000, BP_NARROW },
  { 18109000, 18111000, BP_BEACON },
  { 18111000, 18168000, BP_ALL    },
  { 21000000, 21070000, BP_CW     },
  { 21070000, 21149000, BP_NARROW },
  { 21149000, 21151000, BP_BEACON },
  { 21151000, 21450000, BP_ALL    },
  { 24890000, 24915000, BP_CW     },
  { 24915000, 24929000, BP_NARROW },
  { 24929000, 24931000, BP_BEACON },
  { 24931000, 24990000, BP_ALL    },
  { 28000000, 28070000, BP_CW     },
  { 28070000, 28190000, BP_NARROW },
  { 28190000, 28225000, BP_BEACON },
  { 28225000, 29300000, BP_ALL    },
  { 29300000, 29510000, BP_SAT    },
  { 29520000, 29700000, BP_FM     },
};

#define BP_TABLE(t) { t, sizeof(t) / sizeof(t[0]) }
typedef struct { const BandPlanSegment *seg; unsigned count; } BandPlanTable;

// Segment of one table containing freq, or nullptr
static inline const BandPlanSegment *bandPlanFindIn(const BandPlanTable &t, uint32_t freq)
{
  for(unsigned i = 0 ; i < t.count ; i++)
    if(freq >= t.seg[i].start && freq < t.seg[i].end) return(&t.seg[i]);
  return(nullptr);
}

// Segment containing freq in the Region 1 plan, or nullptr outside the amateur bands
static inline const BandPlanSegment *bandPlanFind(uint32_t freq)
{
  static const BandPlanTable r1 = BP_TABLE(bandPlanR1);
  return(bandPlanFindIn(r1, freq));
}

// Plan areas. All: every region's bands, Region 1 details first, then 2 and
// 3 for the parts only they have. Iran and UAE: the Region 1 plan, narrowed
// where the country's licence allows less. UAE: TDRA Regulations for Amateur
// Radio v3.0 (Dec 2020), art. 4.1 (160 m is 1810-1850 kHz only). Iran: CRA
// National Table of Frequency Allocations, 6th edition (2019), national
// column: no amateur allocation on 160 m, 80 m or 30 m (60 m is secondary,
// 40 m is 7000-7200). Region 2 and 3 plans are simplified guidance: check
// your own licence.
enum BandPlanArea : uint8_t
{
  BP_AREA_ALL = 0, BP_AREA_R1, BP_AREA_R2, BP_AREA_R3, BP_AREA_IRAN, BP_AREA_UAE, BP_AREA_COUNT
};

// Tables of an area, highest priority first; returns how many
static inline unsigned bandPlanTables(uint8_t area, BandPlanTable *t)
{
  static const BandPlanTable r1 = BP_TABLE(bandPlanR1), r2 = BP_TABLE(bandPlanR2), r3 = BP_TABLE(bandPlanR3);
  switch(area)
  {
    case BP_AREA_ALL: t[0] = r1; t[1] = r2; t[2] = r3; return(3);
    case BP_AREA_R2:  t[0] = r2; return(1);
    case BP_AREA_R3:  t[0] = r3; return(1);
    default:          t[0] = r1; return(1);
  }
}

static const BandPlanSegment bandPlanBlockedUAE[] =
{
  {  1850000,  2000000, BP_BLOCKED },
};

static const BandPlanSegment bandPlanBlockedIran[] =
{
  {  1810000,  2000000, BP_BLOCKED },
  {  3500000,  3800000, BP_BLOCKED },
  { 10100000, 10150000, BP_BLOCKED },
};

static inline const BandPlanSegment *bandPlanBlocked(uint8_t area, unsigned *count)
{
  if(area == BP_AREA_UAE) { *count = sizeof(bandPlanBlockedUAE) / sizeof(bandPlanBlockedUAE[0]); return(bandPlanBlockedUAE); }
  if(area == BP_AREA_IRAN) { *count = sizeof(bandPlanBlockedIran) / sizeof(bandPlanBlockedIran[0]); return(bandPlanBlockedIran); }
  *count = 0;
  return(nullptr);
}

// Segment for an area: a blocked part wins over the plan
static inline const BandPlanSegment *bandPlanFindArea(uint32_t freq, uint8_t area)
{
  unsigned n;
  const BandPlanSegment *b = bandPlanBlocked(area, &n);
  for(unsigned i = 0 ; i < n ; i++)
    if(freq >= b[i].start && freq < b[i].end) return(&b[i]);
  BandPlanTable t[3];
  unsigned nt = bandPlanTables(area, t);
  for(unsigned i = 0 ; i < nt ; i++)
  {
    const BandPlanSegment *seg = bandPlanFindIn(t[i], freq);
    if(seg) return(seg);
  }
  return(nullptr);
}

// Area saved by builds before the region choice (0 Region 1, 1 UAE, 2 Iran)
static inline uint8_t bandPlanAreaFromOld(uint8_t old)
{
  return(old == 1 ? BP_AREA_UAE : old == 2 ? BP_AREA_IRAN : BP_AREA_R1);
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
