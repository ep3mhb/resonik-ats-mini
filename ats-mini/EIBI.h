#ifndef EIBI_H
#define EIBI_H

struct BandLabel
{
  uint16_t freq_start;  // Starting frequency
  uint16_t freq_end;    // Ending frequency
  const char *name;     // Band name
};

// StationSchedule and the CSV parser live in EibiParse.h (host-testable)
#include "EibiParse.h"

bool eibiAvailable();
bool eibiLoadSchedule();
const StationSchedule *eibiLookup(uint16_t freq, uint8_t hour, uint8_t minute, size_t *offset=NULL);
const StationSchedule *eibiPrev(uint16_t freq, uint8_t hour, uint8_t minute, size_t *offset);
const StationSchedule *eibiNext(uint16_t freq, uint8_t hour, uint8_t minute, size_t *offset);
const StationSchedule *eibiAtSameFreq(uint8_t hour, uint8_t minute, size_t *offset, bool same);

#endif // EIBI_H
