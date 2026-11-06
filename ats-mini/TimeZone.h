#ifndef TIME_ZONE_H
#define TIME_ZONE_H

// Named time zones with automatic daylight saving time.
// Pure functions without libc time zone support: the C library clock stays in UTC
// (see ats-mini.ino), and web handlers on other tasks rely on that.

#include <stdint.h>

enum TzRule : uint8_t
{
  TZ_NONE = 0, // No daylight saving time
  TZ_EU,       // Last Sun Mar 01:00 UTC .. last Sun Oct 01:00 UTC
  TZ_US,       // 2nd Sun Mar 02:00 local .. 1st Sun Nov 02:00 local
  TZ_AU,       // 1st Sun Oct 02:00 local .. 1st Sun Apr 03:00 local (south)
  TZ_NZ,       // Last Sun Sep 02:00 local .. 1st Sun Apr 03:00 local
};

typedef struct
{
  int16_t stdMinutes;     // Standard offset from UTC in minutes
  TzRule rule;            // Daylight saving rule
  const char *desc;       // Menu label (short)
  const char *abbr;       // Standard time abbreviation
  const char *abbrDst;    // Summer time abbreviation, or nullptr
  const char *iana;       // IANA name used by browsers, or nullptr for plain offsets
} TimeZone;

// Days since 1970-01-01 for a civil date (Howard Hinnant's algorithm)
static inline int32_t tzDaysFromCivil(int32_t y, uint32_t m, uint32_t d)
{
  y -= m <= 2;
  const int32_t era = (y >= 0 ? y : y - 399) / 400;
  const uint32_t yoe = (uint32_t)(y - era * 400);
  const uint32_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return(era * 146097 + (int32_t)doe - 719468);
}

static inline int32_t tzYearFromDays(int32_t z)
{
  z += 719468;
  const int32_t era = (z >= 0 ? z : z - 146096) / 146097;
  const uint32_t doe = (uint32_t)(z - era * 146097);
  const uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const uint32_t mp = (5 * doy + 2) / 153;
  const uint32_t m = mp < 10 ? mp + 3 : mp - 9;
  return((int32_t)yoe + era * 400 + (m <= 2));
}

// Day number of the n-th Sunday (n = 1..4) or, with n = 0, the last Sunday of a month
static inline int32_t tzSunday(int32_t year, uint32_t month, int n)
{
  if(n)
  {
    int32_t first = tzDaysFromCivil(year, month, 1);
    int wd = (int)((first % 7 + 11) % 7);  // 0 = Sunday (1970-01-01 was a Thursday)
    return(first + (7 - wd) % 7 + (n - 1) * 7);
  }
  int32_t next = month == 12 ? tzDaysFromCivil(year + 1, 1, 1) : tzDaysFromCivil(year, month + 1, 1);
  int32_t last = next - 1;
  int wd = (int)((last % 7 + 11) % 7);
  return(last - wd);
}

// Current offset from UTC in minutes, including summer time
static inline int16_t tzOffsetMinutes(const TimeZone *tz, int64_t utc, bool *dst = nullptr)
{
  bool on = false;
  if(tz && tz->rule != TZ_NONE)
  {
    const int64_t s = tz->stdMinutes * 60LL;
    const int32_t year = tzYearFromDays((int32_t)((utc + s) / 86400));
    int64_t start = 0, end = 0;

    switch(tz->rule)
    {
      case TZ_EU:
        start = tzSunday(year, 3, 0) * 86400LL + 3600;
        end   = tzSunday(year, 10, 0) * 86400LL + 3600;
        break;
      case TZ_US:
        start = tzSunday(year, 3, 2) * 86400LL + 2 * 3600 - s;
        end   = tzSunday(year, 11, 1) * 86400LL + 2 * 3600 - (s + 3600);
        break;
      case TZ_AU:
        start = tzSunday(year, 10, 1) * 86400LL + 2 * 3600 - s;
        end   = tzSunday(year, 4, 1) * 86400LL + 3 * 3600 - (s + 3600);
        break;
      case TZ_NZ:
        start = tzSunday(year, 9, 0) * 86400LL + 2 * 3600 - s;
        end   = tzSunday(year, 4, 1) * 86400LL + 3 * 3600 - (s + 3600);
        break;
      default:
        break;
    }

    // Northern rules run start..end inside the year; southern ones wrap around New Year
    on = start < end ? (utc >= start && utc < end) : (utc >= start || utc < end);
  }

  if(dst) *dst = on;
  return((int16_t)((tz ? tz->stdMinutes : 0) + (on ? 60 : 0)));
}

static inline const char *tzAbbr(const TimeZone *tz, int64_t utc)
{
  bool dst;
  tzOffsetMinutes(tz, utc, &dst);
  return(dst && tz->abbrDst ? tz->abbrDst : tz->abbr);
}

#endif // TIME_ZONE_H
