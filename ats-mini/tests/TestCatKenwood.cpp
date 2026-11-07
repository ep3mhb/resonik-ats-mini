#include "../CatKenwood.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint32_t freq = 7074000;
static uint8_t mode = KW_USB, vol = 100;
static uint32_t getF() { return freq; }
static bool setF(uint32_t f) { if(f < 150000 || f > 30000000) return false; freq = f; return true; }
static uint8_t getM() { return mode; }
static bool setM(uint8_t m) { if(m < 1 || m > 5) return false; mode = m; return true; }
static uint8_t getS() { return 15; }
static uint8_t getV() { return vol; }
static void setV(uint8_t v) { vol = v; }

static const CatRadio radio = { getF, setF, getM, setM, getS, getV, setV };

static const char *send(CatKenwood &k, const char *s)
{
  static char out[64];
  out[0] = 0;
  bool done = false;
  for( ; *s ; s++) done = k.feed(*s, radio, out);
  assert(done);
  return out;
}

int main()
{
  CatKenwood k;
  assert(!strcmp(send(k, "ID;"), "ID020;"));
  assert(!strcmp(send(k, "FA;"), "FA00007074000;"));
  assert(!strcmp(send(k, "FA00014074000;"), "") && freq == 14074000);
  assert(!strcmp(send(k, "fa;"), "FA00014074000;"));
  assert(!strcmp(send(k, "FA00099999999;"), "?;") && freq == 14074000);
  assert(!strcmp(send(k, "MD;"), "MD2;"));
  assert(!strcmp(send(k, "MD3;"), "") && mode == KW_CW);
  assert(!strcmp(send(k, "MD8;"), "?;"));
  const char *ifr = send(k, "IF;");
  assert(strlen(ifr) == 38);
  assert(!strncmp(ifr, "IF00014074000     +0000", 23));
  assert(ifr[29] == '3');          // Mode position (P9)
  assert(ifr[37] == ';' && ifr[36] == ' ');
  assert(!strcmp(send(k, "SM0;"), "SM00015;"));
  assert(!strcmp(send(k, "AG0;"), "AG0100;"));
  assert(!strcmp(send(k, "AG0200;"), "") && vol == 200);
  assert(!strcmp(send(k, "PS;"), "PS1;"));
  assert(!strcmp(send(k, "TX;"), "?;"));
  assert(!strcmp(send(k, "ZZ;"), "?;"));
  assert(!strcmp(send(k, "\r\nID;"), "ID020;"));
  assert(!strcmp(send(k, "AI2;"), "") && !strcmp(send(k, "AI;"), "AI2;"));
  // Two commands in one write
  char out[64];
  int replies = 0;
  for(const char *s = "FA;MD;" ; *s ; s++) if(k.feed(*s, radio, out)) replies++;
  assert(replies == 2 && !strcmp(out, "MD3;"));

  assert(catSmeterFromRssi(0) == 0);
  assert(catSmeterFromRssi(34) == 15);
  assert(catSmeterFromRssi(94) == 30);
  assert(catSmeterFromRssi(127) == 30);
  printf("TestCatKenwood: all tests passed\n");
  return 0;
}
