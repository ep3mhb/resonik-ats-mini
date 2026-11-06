#include <cassert>
#include <cstdio>
#include "../OtaVersion.h"

static uint32_t parse(const char *tag)
{
  uint32_t v = 0;
  return otaParseReleaseTag(tag, &v)? v : 0;
}

int main()
{
  // Valid tags
  assert(parse("v2.45") == 24500);
  assert(parse("v2.45-r1") == 24501);
  assert(parse("v2.45-r12") == 24512);
  assert(parse("v3.00-r2") == 30002);
  assert(parse("v2.05-r1") == 20501);

  // Ordering: a RESONIK release is newer than its upstream base and older than the next upstream version
  assert(parse("v2.45-r1") > parse("v2.45"));
  assert(parse("v2.45-r2") > parse("v2.45-r1"));
  assert(parse("v2.46") > parse("v2.45-r9"));

  // Rejected tags
  assert(parse("") == 0);
  assert(parse("2.45") == 0);
  assert(parse("v2.4") == 0);
  assert(parse("v2.45-r") == 0);
  assert(parse("v2.45-r0") == 0);
  assert(parse("v2.45-r01") == 0);
  assert(parse("v2.45-r1-beta") == 0);
  assert(parse("v2.45-beta") == 0);
  assert(parse("v2.45x") == 0);
  assert(parse("v2.450") == 0);
  assert(parse("v2.45-r100") == 0);
  assert(parse("v0.99") == 0);

  printf("TestOtaVersion: all tests passed\n");
  return 0;
}
