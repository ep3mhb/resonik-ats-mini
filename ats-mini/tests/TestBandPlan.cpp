#include "../BandPlan.h"
#include <assert.h>
#include <stdio.h>

int main()
{
  // Table is sorted and non-overlapping
  for(unsigned i = 0 ; i < sizeof(bandPlanR1) / sizeof(bandPlanR1[0]) ; i++)
  {
    assert(bandPlanR1[i].start < bandPlanR1[i].end);
    if(i) assert(bandPlanR1[i - 1].end <= bandPlanR1[i].start);
  }

  assert(!bandPlanFind(1809999));
  assert(bandPlanFind(1810000)->use == BP_CW);
  assert(bandPlanFind(3699000)->use == BP_ALL);
  assert(bandPlanFind(7039999)->use == BP_CW);
  assert(bandPlanFind(7074000)->use == BP_ALL);   // FT8 sits in the all-mode part on 40 m
  assert(bandPlanFind(10136000)->use == BP_NARROW);
  assert(bandPlanFind(14074000)->use == BP_NARROW);
  assert(bandPlanFind(14100000)->use == BP_BEACON);
  assert(bandPlanFind(14200000)->use == BP_ALL);
  assert(!bandPlanFind(14350000));
  assert(bandPlanFind(28200000)->use == BP_BEACON);
  assert(bandPlanFind(29600000)->use == BP_FM);
  assert(!bandPlanFind(9650000));                 // 31 m broadcast

  assert(bandPlanPhoneIsLSB(3700000));
  assert(bandPlanPhoneIsLSB(7100000));
  assert(!bandPlanPhoneIsLSB(5360000));
  assert(!bandPlanPhoneIsLSB(14200000));

  printf("TestBandPlan: all tests passed\n");
  return 0;
}
