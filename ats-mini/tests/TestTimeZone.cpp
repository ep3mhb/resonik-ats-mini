#include "../TimeZone.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const TimeZone london   = {    0, TZ_EU,   "London",   "GMT",  "BST",  "Europe/London" };
static const TimeZone berlin   = {   60, TZ_EU,   "Berlin",   "CET",  "CEST", "Europe/Berlin" };
static const TimeZone newYork  = { -300, TZ_US,   "New York", "EST",  "EDT",  "America/New_York" };
static const TimeZone sydney   = {  600, TZ_AU,   "Sydney",   "AEST", "AEDT", "Australia/Sydney" };
static const TimeZone auckland = {  720, TZ_NZ,   "Auckland", "NZST", "NZDT", "Pacific/Auckland" };
static const TimeZone dubai    = {  240, TZ_NONE, "Dubai",    "GST",  nullptr, "Asia/Dubai" };

// UTC epoch of a civil date and time
static int64_t at(int y, int m, int d, int hh, int mm)
{
  return(tzDaysFromCivil(y, m, d) * 86400LL + hh * 3600 + mm * 60);
}

int main()
{
  assert(tzDaysFromCivil(1970, 1, 1) == 0);
  assert(tzDaysFromCivil(2026, 11, 6) == 20763);
  assert(tzYearFromDays(20763) == 2026);
  assert(tzYearFromDays(tzDaysFromCivil(2028, 12, 31)) == 2028);
  assert(tzSunday(2026, 3, 0) == tzDaysFromCivil(2026, 3, 29));
  assert(tzSunday(2026, 3, 2) == tzDaysFromCivil(2026, 3, 8));
  assert(tzSunday(2026, 11, 1) == tzDaysFromCivil(2026, 11, 1));
  assert(tzSunday(2026, 9, 0) == tzDaysFromCivil(2026, 9, 27));

  // Europe 2026: 29 Mar 01:00 UTC .. 25 Oct 01:00 UTC
  assert(tzOffsetMinutes(&london, at(2026, 3, 29, 0, 59)) == 0);
  assert(tzOffsetMinutes(&london, at(2026, 3, 29, 1, 0)) == 60);
  assert(tzOffsetMinutes(&berlin, at(2026, 10, 25, 0, 59)) == 120);
  assert(tzOffsetMinutes(&berlin, at(2026, 10, 25, 1, 0)) == 60);

  // US 2026: 8 Mar 07:00 UTC .. 1 Nov 06:00 UTC (New York)
  assert(tzOffsetMinutes(&newYork, at(2026, 3, 8, 6, 59)) == -300);
  assert(tzOffsetMinutes(&newYork, at(2026, 3, 8, 7, 0)) == -240);
  assert(tzOffsetMinutes(&newYork, at(2026, 11, 1, 5, 59)) == -240);
  assert(tzOffsetMinutes(&newYork, at(2026, 11, 1, 6, 0)) == -300);

  // Sydney 2026: summer time ends 4 Apr 16:00 UTC, starts 3 Oct 16:00 UTC
  assert(tzOffsetMinutes(&sydney, at(2026, 1, 15, 0, 0)) == 660);
  assert(tzOffsetMinutes(&sydney, at(2026, 4, 4, 15, 59)) == 660);
  assert(tzOffsetMinutes(&sydney, at(2026, 4, 4, 16, 0)) == 600);
  assert(tzOffsetMinutes(&sydney, at(2026, 10, 3, 15, 59)) == 600);
  assert(tzOffsetMinutes(&sydney, at(2026, 10, 3, 16, 0)) == 660);

  // Auckland 2026: ends 4 Apr 14:00 UTC, starts 26 Sep 14:00 UTC
  assert(tzOffsetMinutes(&auckland, at(2026, 4, 4, 13, 59)) == 780);
  assert(tzOffsetMinutes(&auckland, at(2026, 4, 4, 14, 0)) == 720);
  assert(tzOffsetMinutes(&auckland, at(2026, 9, 26, 13, 59)) == 720);
  assert(tzOffsetMinutes(&auckland, at(2026, 9, 26, 14, 0)) == 780);

  // Local New Year's Eve in Auckland is already next year in local terms
  assert(tzOffsetMinutes(&auckland, at(2026, 12, 31, 12, 0)) == 780);

  // No summer time
  assert(tzOffsetMinutes(&dubai, at(2026, 7, 1, 0, 0)) == 240);
  assert(tzOffsetMinutes(nullptr, at(2026, 7, 1, 0, 0)) == 0);

  assert(!strcmp(tzAbbr(&london, at(2026, 7, 1, 0, 0)), "BST"));
  assert(!strcmp(tzAbbr(&london, at(2026, 12, 1, 0, 0)), "GMT"));
  assert(!strcmp(tzAbbr(&dubai, at(2026, 7, 1, 0, 0)), "GST"));

  printf("TestTimeZone: all tests passed\n");
  return 0;
}
