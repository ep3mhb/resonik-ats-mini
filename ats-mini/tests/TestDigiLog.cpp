#include "../DigiDecoders.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string>

// The whole-text log of the Digital page (Digi::Log) and its feed from Text
static std::string all(const Digi::Log &l)
{
  std::string s;
  for(uint32_t i = l.first(); i < l.total; i++) s += l.at(i);
  return s;
}

static void feed(Digi::Text &t, const char *s) { for(; *s; s++) t.put(*s); }

int main()
{
  char buf[64];
  Digi::Log log;
  Digi::Text text;

  // Without a ring nothing is kept and nothing breaks
  log.put('A');
  assert(log.total == 0);

  log.buf = buf; log.size = sizeof(buf);
  text.log = &log;
  text.clear();

  // The screen line turns breaks into spaces; the log keeps them, CR dropped
  feed(text, "ZCZC FA12\r\nGALE WARNING\r\n");
  assert(!strcmp(text.buf, "ZCZC FA12 GALE WARNING "));
  assert(all(log) == "ZCZC FA12\nGALE WARNING\n");

  // One empty line at most, and nothing but printable characters
  feed(text, "\n\n\n\aNNNN\x01\x7f");
  assert(all(log) == "ZCZC FA12\nGALE WARNING\n\nNNNN");

  // A head line waits for the first character, skips spaces and breaks
  // before it, and stands on a line of its own after an empty one
  strcpy(log.head, "== NAVTEX 518 kHz ==");
  feed(text, "\n \n");
  assert(all(log) == "ZCZC FA12\nGALE WARNING\n\nNNNN");
  feed(text, "ZC");
  assert(all(log) == "ZCZC FA12\nGALE WARNING\n\nNNNN\n\n== NAVTEX 518 kHz ==\nZC");
  assert(!log.head[0]);

  // A head on an empty log starts it, with no empty line above
  char b2[32];
  Digi::Log l2;
  l2.buf = b2; l2.size = sizeof(b2);
  strcpy(l2.head, "== RTTY ==");
  l2.put('\n'); l2.put('R'); l2.put('Y');
  assert(all(l2) == "== RTTY ==\nRY");

  // The ring keeps the last `size` characters; offsets go on counting
  for(int i = 0; i < 100; i++) l2.put((char)('0' + i % 10));
  assert(l2.total == 13 + 100);
  assert(l2.first() == l2.total - sizeof(b2));
  std::string tail = all(l2);
  assert(tail.size() == sizeof(b2));
  assert(tail.back() == '9' && tail[0] == '8');

  puts("TestDigiLog: ok");
  return 0;
}
