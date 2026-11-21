#include "../KenwoodExtra.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static KenwoodExtra ask(const char *s, const char *&text)
{
  return kenwoodExtra((const uint8_t *)s, strlen(s), text);
}

int main()
{
  const char *t;
  // CW and data modes
  assert(ask("MD3;", t) == KenwoodExtra::ModeCw && !t);
  assert(ask("md7;", t) == KenwoodExtra::ModeCw);
  assert(ask("MD6;", t) == KenwoodExtra::ModeUsb);
  assert(ask("MD9;", t) == KenwoodExtra::ModeUsb);
  // Everything upstream handles is left to it
  assert(ask("MD;", t) == KenwoodExtra::None);
  assert(ask("MD1;", t) == KenwoodExtra::None);
  assert(ask("MD2;", t) == KenwoodExtra::None);
  assert(ask("MD4;", t) == KenwoodExtra::None);
  assert(ask("MD5;", t) == KenwoodExtra::None);
  assert(ask("MD8;", t) == KenwoodExtra::None);
  assert(ask("MD33;", t) == KenwoodExtra::None);
  assert(ask("FA;", t) == KenwoodExtra::None);
  assert(ask("FA00014074000;", t) == KenwoodExtra::None);
  assert(ask("IF;", t) == KenwoodExtra::None);
  assert(ask("TX;", t) == KenwoodExtra::None);
  assert(ask(";", t) == KenwoodExtra::None);
  assert(ask("M;", t) == KenwoodExtra::None);
  assert(ask("MD3", t) == KenwoodExtra::None);     // No ';'
  // Fixed answers, reads only
  assert(ask("RS;", t) == KenwoodExtra::Reply && !strcmp(t, "RS0;"));
  assert(ask("ks;", t) == KenwoodExtra::Reply && !strcmp(t, "KS020;"));
  assert(ask("SH;", t) == KenwoodExtra::Reply && !strcmp(t, "SH00;"));
  assert(ask("SL;", t) == KenwoodExtra::Reply && !strcmp(t, "SL00;"));
  assert(ask("RT;", t) == KenwoodExtra::Reply && !strcmp(t, "RT0;"));
  assert(ask("XT;", t) == KenwoodExtra::Reply && !strcmp(t, "XT0;"));
  assert(ask("RF;", t) == KenwoodExtra::Reply && !strcmp(t, "RF0;"));
  assert(ask("SH05;", t) == KenwoodExtra::None && !t);
  assert(ask("RT1;", t) == KenwoodExtra::None);
  printf("TestKenwoodExtra: all tests passed\n");
  return 0;
}
