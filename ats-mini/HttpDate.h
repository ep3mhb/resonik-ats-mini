#ifndef HTTP_DATE_H
#define HTTP_DATE_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// Parse an HTTP Date header ("Fri, 06 Nov 2026 14:05:09 GMT", RFC 9110 IMF-fixdate)
// into a UTC epoch. Used as a fallback clock source when NTP (UDP 123) is blocked.
static inline bool httpDateToEpoch(const char *text, uint32_t *epoch)
{
  static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char wday[4], mon[4], zone[4];
  int day, year, hour, minute, second;

  if(!text || !epoch) return(false);
  if(sscanf(text, "%3s, %d %3s %d %d:%d:%d %3s",
            wday, &day, mon, &year, &hour, &minute, &second, zone) != 8)
    return(false);
  if(strcmp(zone, "GMT")) return(false);

  const char *m = strstr(months, mon);
  if(!m || strlen(mon) != 3 || (m - months) % 3) return(false);
  int month = (m - months) / 3 + 1;

  if(year < 2000 || year > 2105 || day < 1 || day > 31 ||
     hour > 23 || minute > 59 || second > 60 || hour < 0 || minute < 0 || second < 0)
    return(false);

  // Days from civil (Howard Hinnant), no time zone or libc dependency
  int y = year - (month <= 2);
  int era = y / 400;
  int yoe = y - era * 400;
  int doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  int64_t days = (int64_t)era * 146097 + doe - 719468;

  int64_t value = days * 86400 + hour * 3600 + minute * 60 + (second > 59 ? 59 : second);
  if(value < 0 || value > 0xFFFFFFFFLL) return(false);
  *epoch = (uint32_t)value;
  return(true);
}

#endif // HTTP_DATE_H
