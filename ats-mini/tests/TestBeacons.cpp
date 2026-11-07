#include "../Beacons.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main()
{
  // At the start of each 3-minute cycle 4U1UN is on 14100, YV5B on 18110 ...
  uint32_t t0 = 1794000000u - (1794000000u % 180);
  assert(!strcmp(ibpBeacons[ibpBeaconOn(0, t0)].call, "4U1UN"));
  assert(!strcmp(ibpBeacons[ibpBeaconOn(1, t0)].call, "YV5B"));
  assert(!strcmp(ibpBeacons[ibpBeaconOn(2, t0)].call, "OA4B"));
  assert(!strcmp(ibpBeacons[ibpBeaconOn(4, t0)].call, "CS3B"));
  // ... and 10 s later 4U1UN moves to 18110 and VE8AT takes 14100
  assert(!strcmp(ibpBeacons[ibpBeaconOn(0, t0 + 10)].call, "VE8AT"));
  assert(!strcmp(ibpBeacons[ibpBeaconOn(1, t0 + 10)].call, "4U1UN"));
  // 4X6TU (index 12) starts on 20 m two minutes into the cycle
  assert(ibpBandOf(12, t0 + 120) == 0);
  assert(ibpBandOf(12, t0 + 160) == 4);
  assert(ibpBandOf(12, t0 + 170) == -1);
  assert(ibpBandOf(12, t0 + 119) == -1);
  assert(ibpSecondsToNext(12, t0 + 115) == 5);
  assert(ibpSecondsLeft(t0 + 3) == 7);

  assert(ibpBandAt(14100000) == 0);
  assert(ibpBandAt(14100800) == 0);
  assert(ibpBandAt(28199500) == 4);
  assert(ibpBandAt(14102000) == -1);
  assert(ibpBandAt(7000000) == -1);

  // Every beacon is on each band exactly once per cycle
  for(int b = 0 ; b < 5 ; b++)
  {
    int seen[18] = {0};
    for(int s = 0 ; s < 18 ; s++) seen[ibpBeaconOn(b, t0 + s * 10)]++;
    for(int i = 0 ; i < 18 ; i++) assert(seen[i] == 1);
  }
  printf("TestBeacons: all tests passed\n");
  return 0;
}
