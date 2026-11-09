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

  // Plan areas: UAE blocks 160 m above 1850 kHz; Iran blocks 160, 80 and 30 m
  assert(bandPlanFindArea(1840000, BP_AREA_UAE)->use == BP_NARROW);
  assert(bandPlanFindArea(1900000, BP_AREA_UAE)->use == BP_BLOCKED);
  assert(bandPlanFindArea(1900000, BP_AREA_R1)->use == BP_ALL);
  assert(bandPlanFindArea(1900000, BP_AREA_IRAN)->use == BP_BLOCKED);
  assert(bandPlanFindArea(1820000, BP_AREA_IRAN)->use == BP_BLOCKED);
  assert(bandPlanFindArea(3550000, BP_AREA_IRAN)->use == BP_BLOCKED);
  assert(bandPlanFindArea(3790000, BP_AREA_IRAN)->use == BP_BLOCKED);
  assert(bandPlanFindArea(10120000, BP_AREA_IRAN)->use == BP_BLOCKED);
  assert(bandPlanFindArea(5357000, BP_AREA_IRAN)->use == BP_ALL);
  assert(bandPlanFindArea(7150000, BP_AREA_IRAN)->use == BP_ALL);
  assert(bandPlanFindArea(14200000, BP_AREA_IRAN)->use == BP_ALL);
  assert(bandPlanFindArea(28074000, BP_AREA_IRAN)->use == BP_NARROW);
  assert(bandPlanFindArea(7010000, BP_AREA_UAE)->use == BP_CW);
  assert(!bandPlanFindArea(9650000, BP_AREA_UAE));

  assert(bandPlanPhoneIsLSB(3700000));
  assert(bandPlanPhoneIsLSB(7100000));
  assert(!bandPlanPhoneIsLSB(5360000));
  assert(!bandPlanPhoneIsLSB(14200000));

  printf("TestBandPlan: all tests passed\n");
  return 0;
}
