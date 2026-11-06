#ifndef OTA_VERSION_H
#define OTA_VERSION_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

//
// RESONIK release tags look like "v2.45-r1": the upstream ats-mini version
// followed by the RESONIK release number. A plain upstream-style "v2.45" is
// accepted as release 0. Anything else (prerelease suffixes such as
// "v2.45-r2-beta", missing digits, extra characters) is rejected.
//
// On success *version receives major*10000 + minor*100 + release, so that
// later releases always compare greater.
//
static inline bool otaParseReleaseTag(const char *tag, uint32_t *version)
{
  if(!tag || !version) return(false);
  size_t len = strlen(tag);
  if(len < 5 || len > 12 || tag[0] != 'v') return(false);

  unsigned major, minor, release = 0;
  char trailing;

  if(sscanf(tag, "v%u.%u-r%u%c", &major, &minor, &release, &trailing) == 3)
  {
    // Release numbers are written without leading zeros or signs
    const char *r = strstr(tag, "-r");
    if(!r || r[2] < '1' || r[2] > '9') return(false);
  }
  else if(sscanf(tag, "v%u.%u%c", &major, &minor, &trailing) == 2)
    release = 0;
  else
    return(false);

  // Minor version is always two digits, as in "v2.05"
  const char *dot = strchr(tag, '.');
  if(!dot || dot[1] < '0' || dot[1] > '9' || dot[2] < '0' || dot[2] > '9') return(false);
  if(dot[3] != '\0' && dot[3] != '-') return(false);

  if(major < 1 || major > 99 || minor > 99 || release > 99) return(false);
  *version = major * 10000u + minor * 100u + release;
  return(true);
}

#endif // OTA_VERSION_H
