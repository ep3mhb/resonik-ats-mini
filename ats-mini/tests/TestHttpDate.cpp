#include "../HttpDate.h"
#include <assert.h>
#include <stdio.h>

int main()
{
  uint32_t e = 0;
  assert(httpDateToEpoch("Thu, 01 Jan 1970 00:00:00 GMT", &e) == false); // before 2000
  assert(httpDateToEpoch("Sat, 01 Jan 2000 00:00:00 GMT", &e) && e == 946684800u);
  assert(httpDateToEpoch("Fri, 06 Nov 2026 14:05:09 GMT", &e) && e == 1793973909u);
  assert(httpDateToEpoch("Tue, 29 Feb 2028 23:59:59 GMT", &e) && e == 1835481599u);
  assert(!httpDateToEpoch("Fri, 06 Xyz 2026 14:05:09 GMT", &e));
  assert(!httpDateToEpoch("Fri, 06 Nov 2026 14:05:09 UTC", &e));
  assert(!httpDateToEpoch("garbage", &e));
  assert(!httpDateToEpoch(nullptr, &e));
  printf("HttpDate tests passed\n");
  return 0;
}
