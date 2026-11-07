#include "../EibiParse.h"
#include <assert.h>
#include <stdio.h>

int main()
{
  assert(eibiParseDays("") == 0x7F);
  assert(eibiParseDays("Mo-Fr") == 0x1F);
  assert(eibiParseDays("Sa") == 0x20);
  assert(eibiParseDays("Sa,Su") == 0x60);
  assert(eibiParseDays("Sa-Mo") == 0x61);
  assert(eibiParseDays("1245") == 0x1B);
  assert(eibiParseDays("1-5") == 0x1F);
  assert(eibiParseDays("7") == 0x40);
  assert(eibiParseDays("irr") == 0x7F);
  assert(eibiParseDays("24Dec") == 0x7F);
  assert(eibiParseDays("alt") == 0x7F);

  StationSchedule e;
  assert(!eibiParseCsvLine("kHz:75;Time(UTC):93;Days:59;ITU:49;Station:201;Lng:49;Target:62;Remarks:135;P:35;Start:60;Stop:60;", e));
  assert(eibiParseCsvLine("153;0000-2400;;ROU;R Antena Satelor;R;EEu;;1;;", e));
  assert(e.freq == 153 && e.start_h == 0 && e.end_h == 24 && e.days == 0x7F);
  assert(!strcmp(e.name, "R Antena Satelor") && !strcmp(e.lang, "R") && !strcmp(e.target, "EEu"));
  assert(eibiParseCsvLine("6005.5;0700-0900;Sa;D;Radio Mi Amigo Int.;E;WEu;kw;1;;\r", e));
  assert(e.freq == 6005 && e.start_h == 7 && e.end_m == 0 && e.days == 0x20 && !strcmp(e.target, "WEu"));
  assert(eibiParseCsvLine("9665;1800-1900;Mo-Fr;IRN;VIRI Voice of Islamic Republic of Iran;FA;ME;;1;;", e));
  assert(strlen(e.name) == 31 && !strcmp(e.lang, "FA"));
  assert(!eibiParseCsvLine("5000;0000-2400;;CHN;Chinese Jammer;-CW;;;;;", e));
  assert(!eibiParseCsvLine("abc;def", e));

  char c[16], p[16];
  eibiSeasonFiles(2026, 11, c, p);  assert(!strcmp(c, "sked-b26.csv") && !strcmp(p, "sked-a26.csv"));
  eibiSeasonFiles(2027, 1, c, p);   assert(!strcmp(c, "sked-b26.csv") && !strcmp(p, "sked-a26.csv"));
  eibiSeasonFiles(2027, 5, c, p);   assert(!strcmp(c, "sked-a27.csv") && !strcmp(p, "sked-b26.csv"));

  // 2026-11-07 was a Saturday
  assert(eibiDayBit(20764u * 86400u + 3600) == 0x20);
  // 1970-01-01 Thursday
  assert(eibiDayBit(0) == 0x08);
  printf("TestEibiParse: all tests passed\n");
  return 0;
}
