#include "../BatteryTime.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main()
{
  assert(batterySocFromVolts(3.0f) == 0.0f);
  assert(batterySocFromVolts(4.2f) == 100.0f);
  assert(fabsf(batterySocFromVolts(3.80f) - 50.0f) < 0.01f);
  assert(fabsf(batterySocFromVolts(3.82f) - 55.0f) < 0.01f);
  // Monotonic
  for(float v = 3.3f ; v < 4.15f ; v += 0.01f)
    assert(batterySocFromVolts(v + 0.01f) >= batterySocFromVolts(v));

  BatteryEstimator e;
  batteryEstimatorReset(&e);
  assert(batteryEstimatorMinutes(&e) == 0);

  // Full battery, default rate: 100 % / 12 %/h = 500 minutes
  batteryEstimatorUpdate(&e, 0, 4.2f);
  assert(batteryEstimatorMinutes(&e) == 500);
  assert(!e.learned);

  // Simulate 20 %/h: 90 % -> 80 % (4.05 V -> 3.97 V) over 30 minutes, 1 sample/s
  batteryEstimatorReset(&e);
  uint32_t t = 0;
  for(int s = 0 ; s <= 30 * 60 ; s++, t += 1000)
  {
    float soc = 90.0f - 10.0f * s / 1800.0f;
    // Invert the curve in the 80..90 % segment
    float v = 3.97f + (soc - 80.0f) / 10.0f * (4.05f - 3.97f);
    batteryEstimatorUpdate(&e, t, v);
  }
  assert(e.learned);
  // The smoothed SOC lags a little, so the learned rate is close to 20 %/h
  assert(e.rate > 15.0f && e.rate < 25.0f);
  uint16_t m = batteryEstimatorMinutes(&e);
  assert(m > 180 && m < 330);   // About 80 % / 20 %/h = 4 h

  printf("TestBatteryTime: all tests passed (rate %.1f %%/h, %u min)\n", e.rate, m);
  return 0;
}
