#ifndef BATTERY_TIME_H
#define BATTERY_TIME_H

// Battery state of charge and time remaining (RESONIK).
// Pure functions with host tests in tests/TestBatteryTime.cpp.

#include <stdint.h>

// Typical 1S Li-ion/Li-Po voltage under the receiver's light load (~0.1C)
static inline float batterySocFromVolts(float v)
{
  static const float volts[] = { 3.30f, 3.50f, 3.65f, 3.70f, 3.74f, 3.77f, 3.80f, 3.84f, 3.90f, 3.97f, 4.05f, 4.15f };
  static const float soc[]   = {  0.0f,  5.0f, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f,100.0f };
  const int n = sizeof(volts) / sizeof(volts[0]);

  if(v <= volts[0]) return(0.0f);
  if(v >= volts[n - 1]) return(100.0f);
  for(int i = 1 ; i < n ; i++)
    if(v < volts[i])
      return(soc[i - 1] + (soc[i] - soc[i - 1]) * (v - volts[i - 1]) / (volts[i] - volts[i - 1]));
  return(100.0f);
}

// Learns the discharge rate from the smoothed state of charge.
// Until enough data is in, a typical rate is assumed.
#define BATT_DEFAULT_RATE   12.0f   // %/hour (about 8 hours from full)
#define BATT_RATE_MIN        2.0f
#define BATT_RATE_MAX       60.0f
#define BATT_WINDOW_MS  (10UL * 60UL * 1000UL) // Rate measured over 10-minute windows

typedef struct
{
  bool     started;
  bool     learned;       // At least one full window measured
  float    socEma;        // Smoothed state of charge, %
  float    rate;          // Discharge rate, %/hour
  uint32_t windowStart;   // ms
  float    windowSoc;     // socEma at windowStart
  uint32_t lastSample;    // ms
} BatteryEstimator;

static inline void batteryEstimatorReset(BatteryEstimator *e)
{
  e->started = false;
  e->learned = false;
  e->rate = BATT_DEFAULT_RATE;
}

// Feed a voltage reading (on battery power only). Samples closer than 1 s are ignored.
static inline void batteryEstimatorUpdate(BatteryEstimator *e, uint32_t nowMs, float volts)
{
  float soc = batterySocFromVolts(volts);
  if(!e->started)
  {
    e->started = true;
    e->socEma = soc;
    e->windowStart = e->lastSample = nowMs;
    e->windowSoc = soc;
    return;
  }
  if(nowMs - e->lastSample < 1000) return;
  e->lastSample = nowMs;

  // Smooth ADC noise and load changes (time constant about 1 minute at 1 sample/s)
  e->socEma += (soc - e->socEma) * 0.02f;

  uint32_t elapsed = nowMs - e->windowStart;
  if(elapsed >= BATT_WINDOW_MS)
  {
    float hours = elapsed / 3600000.0f;
    float measured = (e->windowSoc - e->socEma) / hours;
    if(measured < BATT_RATE_MIN) measured = BATT_RATE_MIN;
    if(measured > BATT_RATE_MAX) measured = BATT_RATE_MAX;
    // First window replaces the default; later ones are averaged
    e->rate = e->learned ? e->rate * 0.7f + measured * 0.3f : measured;
    e->learned = true;
    e->windowStart = nowMs;
    e->windowSoc = e->socEma;
  }
}

// Minutes of listening left
static inline uint16_t batteryEstimatorMinutes(const BatteryEstimator *e)
{
  if(!e->started) return(0);
  float minutes = e->socEma / e->rate * 60.0f;
  return(minutes < 0 ? 0 : minutes > 6000 ? 6000 : (uint16_t)(minutes + 0.5f));
}

#endif // BATTERY_TIME_H
