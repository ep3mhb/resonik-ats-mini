#ifndef BEACONS_H
#define BEACONS_H

// NCDXF/IARU International Beacon Project (RESONIK).
// 18 beacons take turns on five frequencies: each transmits for 10 seconds on
// 14100 kHz, then on 18110, 21150, 24930 and 28200 kHz. A full cycle takes
// three minutes and starts at 00:00:00 UTC every third minute.
// https://www.ncdxf.org/beacon/

#include <stdint.h>

typedef struct
{
  const char *call;
  const char *place;
} IbpBeacon;

static const IbpBeacon ibpBeacons[18] =
{
  { "4U1UN",  "New York"     },
  { "VE8AT",  "Canada"       },
  { "W6WX",   "California"   },
  { "KH6RS",  "Hawaii"       },
  { "ZL6B",   "New Zealand"  },
  { "VK6RBP", "Australia"    },
  { "JA2IGY", "Japan"        },
  { "RR9O",   "Siberia"      },
  { "VR2B",   "Hong Kong"    },
  { "4S7B",   "Sri Lanka"    },
  { "ZS6DN",  "South Africa" },
  { "5Z4B",   "Kenya"        },
  { "4X6TU",  "Israel"       },
  { "OH2B",   "Finland"      },
  { "CS3B",   "Madeira"      },
  { "LU4AA",  "Argentina"    },
  { "OA4B",   "Peru"         },
  { "YV5B",   "Venezuela"    },
};

static const uint32_t ibpFreqs[5] = { 14100000, 18110000, 21150000, 24930000, 28200000 };

// Band index (0..4) of an IBP frequency within +-1 kHz, or -1
static inline int ibpBandAt(uint32_t freqHz)
{
  for(int b = 0 ; b < 5 ; b++)
    if(freqHz + 1000 >= ibpFreqs[b] && freqHz <= ibpFreqs[b] + 1000) return(b);
  return(-1);
}

// Beacon (0..17) transmitting on band b at the given UTC time
static inline int ibpBeaconOn(int band, uint32_t utc)
{
  int slot = (int)((utc % 180) / 10);
  return((slot - band + 18) % 18);
}

// Band (0..4) the given beacon uses at this time, or -1 while it is silent
static inline int ibpBandOf(int beacon, uint32_t utc)
{
  int slot = (int)((utc % 180) / 10);
  int band = (slot - beacon + 18) % 18;
  return(band < 5 ? band : -1);
}

// Seconds left in the current 10-second slot
static inline int ibpSecondsLeft(uint32_t utc)
{
  return(10 - (int)(utc % 10));
}

// Seconds until the given beacon next starts on any band
static inline int ibpSecondsToNext(int beacon, uint32_t utc)
{
  for(int s = 0 ; s < 180 ; s += 10)
  {
    uint32_t t = utc - (utc % 10) + s;
    if(ibpBandOf(beacon, t) >= 0 && t > utc) return((int)(t - utc));
  }
  return(0);
}

#endif // BEACONS_H
