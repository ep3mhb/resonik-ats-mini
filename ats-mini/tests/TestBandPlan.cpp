#include "../BandPlan.h"
#include <assert.h>
#include <stdio.h>

int main()
{
  // Tables are sorted and non-overlapping
  for(uint8_t area = 0 ; area < BP_AREA_COUNT ; area++)
  {
    BandPlanTable t[3];
    unsigned nt = bandPlanTables(area, t);
    for(unsigned k = 0 ; k < nt ; k++)
      for(unsigned i = 0 ; i < t[k].count ; i++)
      {
        assert(t[k].seg[i].start < t[k].seg[i].end);
        if(i) assert(t[k].seg[i - 1].end <= t[k].seg[i].start);
      }
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

  // Regions: 80 m phone to 4000 in Region 2, 3900 in Region 3, 3800 in Region 1
  assert(bandPlanFindArea(3950000, BP_AREA_R2)->use == BP_ALL);
  assert(!bandPlanFindArea(3950000, BP_AREA_R3));
  assert(bandPlanFindArea(3850000, BP_AREA_R3)->use == BP_ALL);
  assert(!bandPlanFindArea(3850000, BP_AREA_R1));
  assert(bandPlanFindArea(7250000, BP_AREA_R2)->use == BP_ALL);
  assert(!bandPlanFindArea(7250000, BP_AREA_R1));
  assert(bandPlanFindArea(1805000, BP_AREA_R3)->use == BP_CW);
  assert(bandPlanFindArea(5340000, BP_AREA_R2)->use == BP_ALL);
  // All: Region 1 details where it has the band, the others beyond
  assert(bandPlanFindArea(7045000, BP_AREA_ALL)->use == BP_NARROW);   // Region 1 7040-7053
  assert(bandPlanFindArea(7055000, BP_AREA_ALL)->use == BP_ALL);      // Region 1, not R2/R3 narrow
  assert(bandPlanFindArea(7250000, BP_AREA_ALL)->use == BP_ALL);      // Region 2/3 only
  assert(bandPlanFindArea(3950000, BP_AREA_ALL)->use == BP_ALL);      // Region 2 only
  assert(bandPlanFindArea(1805000, BP_AREA_ALL)->use == BP_NARROW);   // Region 2 before Region 3
  assert(!bandPlanFindArea(9650000, BP_AREA_ALL));
  // Saved values from before the region choice
  assert(bandPlanAreaFromOld(0) == BP_AREA_R1);
  assert(bandPlanAreaFromOld(1) == BP_AREA_UAE);
  assert(bandPlanAreaFromOld(2) == BP_AREA_IRAN);

  assert(bandPlanPhoneIsLSB(3700000));
  assert(bandPlanPhoneIsLSB(7100000));
  assert(!bandPlanPhoneIsLSB(5360000));
  assert(!bandPlanPhoneIsLSB(14200000));

  printf("TestBandPlan: all tests passed\n");
  return 0;
}
